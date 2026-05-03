import os
import sys
import subprocess
import json
import platform
import re
import argparse
import shutil
from pathlib import Path
from datetime import datetime

# ANSI constants
CLR_RED = "\033[91m"
CLR_YLW = "\033[93m"
CLR_CYN = "\033[96m"
CLR_GRN = "\033[92m"
CLR_RST = "\033[0m"
CLR_BLD = "\033[1m"

class Logger:
    """Utility class for consistent, colored logging."""
    
    @staticmethod
    def info(msg):
        print(f"{CLR_CYN}[INFO]{CLR_RST} {msg}")

    @staticmethod
    def warn(msg):
        print(f"{CLR_YLW}[WARN]{CLR_RST} {msg}")

    @staticmethod
    def error(msg):
        print(f"{CLR_RED}[ERROR]{CLR_RST} {msg}")

    @staticmethod
    def success(msg):
        print(f"{CLR_GRN}[SUCCESS]{CLR_RST} {msg}")

    @staticmethod
    def critical(msg):
        print(f"{CLR_RED}{CLR_BLD}[CRITICAL]{CLR_RST} {msg}")

    @staticmethod
    def header(msg):
        print(f"\n{CLR_BLD}{CLR_CYN}>>> {msg}{CLR_RST}")

    @staticmethod
    def summary_header(msg):
        print(f"\n{CLR_BLD}--- {msg} ---{CLR_RST}")

def get_tool_root(explicit_path=None) -> Path:
    """
    determines the root directory of the nsec-auditor tool.
    if explicit_path is provided, use that.
    otherwise, default to the parent of this script's directory.
    """
    if explicit_path:
        return Path(explicit_path).resolve()
    
    # default: script is in [root]/scripts/nsec_wrapper.py
    return Path(__file__).resolve().parent.parent

def get_core_engine_dir(tool_root: Path) -> Path:
    """returns the core-engine directory relative to the tool root"""
    return tool_root / "core-engine"

def get_project_name(tool_root: Path):
    """Parses CMakeLists.txt to find the project name."""
    cmake_path = get_core_engine_dir(tool_root) / "CMakeLists.txt"
    if not cmake_path.exists():
        return None
    
    with open(cmake_path, "r") as f:
        content = f.read()
        match = re.search(r"project\(\s*\"?([\w-]+)\"?[\s\)]", content, re.IGNORECASE)
        if match:
            return match.group(1)
    return None

def os_is_windows():
    """Returns True if the current OS is Windows."""
    return platform.system() == "Windows"

def find_binary(tool_root: Path, project_name):
    """Recursively searches core-engine/ for the OS-appropriate binary."""
    ext = ".exe" if os_is_windows() else ""
    target_name = f"{project_name}{ext}"
    
    search_dir = get_core_engine_dir(tool_root)
    if not search_dir.exists():
        return None
        
    for path in search_dir.rglob(target_name):
        if path.is_file():
            # for windows, we check if it exists; for unix, we check if executable
            if not os_is_windows() and not os.access(path, os.X_OK):
                continue
            return str(path)
    return None

def build_binary(tool_root: Path):
    """attempts to build the core-engine using CMake"""
    engine_dir = get_core_engine_dir(tool_root)
    build_dir = engine_dir / "build"
    
    if not engine_dir.exists():
        Logger.critical(f"Source directory {engine_dir} does not exist. Cannot build.")
        sys.exit(1)

    Logger.warn("Auditor binary missing. Attempting to build core-engine...")
    build_dir.mkdir(exist_ok=True)

    # clear CMake cache
    cache_file = build_dir / "CMakeCache.txt"
    if cache_file.exists():
        Logger.info("Detecting stale CMake cache... clearing for new environment.")
        try:
            os.remove(cache_file)
            cmake_files_dir = build_dir / "CMakeFiles"
            if cmake_files_dir.exists():
                shutil.rmtree(cmake_files_dir)
        except Exception as e:
            Logger.warn(f"Could not fully clear cache: {e}")
    
    try:
        # configure
        Logger.info("Configuring project with CMake...")
        subprocess.run(["cmake", "..", "-DCMAKE_BUILD_TYPE=Release"], cwd=build_dir, check=True, capture_output=True, text=True)
        
        # build
        Logger.info("Building project (Release mode)...")
        subprocess.run(["cmake", "--build", ".", "--config", "Release"], cwd=build_dir, check=True, capture_output=True, text=True)
        Logger.success("Build completed successfully.")
    except subprocess.CalledProcessError as e:
        Logger.critical("CMake build failed!")
        Logger.error(f"STDOUT: {e.stdout}")
        Logger.error(f"STDERR: {e.stderr}")
        sys.exit(1)
    except FileNotFoundError:
        Logger.critical("CMake command not found. Please ensure CMake is installed and in your PATH.")
        sys.exit(1)

def check_binary_out_of_date(tool_root: Path, binary_path):
    """compares binary timestamp against source file timestamps"""
    try:
        binary_mtime = os.path.getmtime(binary_path)
        src_dir = get_core_engine_dir(tool_root) / "src"
        
        if not src_dir.exists():
            return

        latest_src_mtime = 0
        extensions = {'.cpp', '.h', '.hpp'}
        
        for path in src_dir.rglob('*'):
            if path.suffix.lower() in extensions:
                mtime = os.path.getmtime(path)
                if mtime > latest_src_mtime:
                    latest_src_mtime = mtime
        
        if latest_src_mtime > binary_mtime:
            Logger.info("Auditor binary might be out of date. Consider rebuilding core-engine.")
    except Exception:
        pass # dont crash the script

def get_staged_files():
    """queries git for staged C++ files in the CURRENT WORKING DIRECTORY"""
    try:
        cmd = ["git", "diff", "--cached", "--name-only", "--diff-filter=ACM"]
        result = subprocess.run(cmd, capture_output=True, text=True, check=True)
        files = result.stdout.splitlines()
        
        cpp_extensions = {'.cpp', '.h', '.hpp', '.cc', '.cxx'}
        return [f for f in files if os.path.splitext(f)[1].lower() in cpp_extensions]
    except subprocess.CalledProcessError:
        Logger.error("Failed to query Git staged files.")
        return []

def run_auditor(binary_path, files, args):
    """executes the auditor, injects metadata, and parses results"""
    report_dir = Path("reports")
    report_dir.mkdir(exist_ok=True)
    
    timestamp_str = datetime.now().strftime("%Y%m%d_%H%M%S")
    report_path = report_dir / f"report_{timestamp_str}.json"

    try:
        # execute core engine
        cmd = [binary_path] + files + ["--output", str(report_path)]
        subprocess.run(cmd, capture_output=True, check=True, text=True, env=os.environ)
        
        # inject metadata if report exists
        if report_path.exists():
            with open(report_path, "r") as f:
                data = json.load(f)
            
            # Create Enterprise Metadata Block
            data["metadata"] = {
                "job_name": args.job_name if args.job_name else "local-dev",
                "build_id": args.build_id if args.build_id else "manual",
                "timestamp": datetime.now().strftime("%Y-%m-%d %H:%M:%S"),
            }

            # write updated report back to disk
            with open(report_path, "w") as f:
                json.dump(data, f, indent=4)

            return data.get("issues", [])
            
    except subprocess.CalledProcessError as e:
        Logger.critical(f"Auditor Engine Crashed (Exit Code: {e.returncode})")
        Logger.error(f"Engine Output: {e.stderr}")
        sys.exit(1)
    except Exception as e:
        Logger.critical(f"System Error during execution: {e}")
        sys.exit(1)
    
    return []

def display_results(findings):
    """Prints findings and returns the count of critical issues."""
    Logger.summary_header("Analysis Summary")
    
    critical_count = 0
    warning_count = 0

    if not findings:
        Logger.success("No security issues detected in staged changes.")
        return 0

    for issue in findings:
        sev = issue.get("severity", "Info")
        msg = issue.get("message", "N/A")
        loc = f"{issue.get('file', 'unknown')}:{issue.get('line', '?')}"
        
        if sev == "Critical":
            print(f"{CLR_RED}[CRITICAL]{CLR_RST} {loc} | {msg}")
            critical_count += 1
        elif sev == "Warning":
            print(f"{CLR_YLW}[WARNING]{CLR_RST}  {loc} | {msg}")
            warning_count += 1
        else:
            print(f"{CLR_CYN}[INFO]{CLR_RST}     {loc} | {msg}")

    return critical_count

def main():
    parser = argparse.ArgumentParser(description="nsec-auditor Enterprise Orchestrator")
    # parser.add_argument("--path", help="Explicit source dir for the auditor")
    parser.add_argument("--build-id", help="Jenkins/CI Build ID")
    parser.add_argument("--job-name", help="Jenkins/CI Job Name")
    parser.add_argument("--engine-path", help="Explicit path to the auditor binary")
    args = parser.parse_args()

    # # the current directory is the project being scanned
    # # tool_root is where the core-engine and logic reside
    # tool_root = get_tool_root(args.path)
    tool_root = get_tool_root() # get tool root regardless of current working dir

    Logger.header("nsec-auditor: Surgical Guardrail")
    Logger.info(f"Auditor Tool Root: {tool_root}")
    Logger.info(f"Scanning Project:  {Path.cwd()}")

    # binary discovery logic based on tool_root
    if args.engine_path:
        binary_path = args.engine_path
        if not os.path.exists(binary_path):
            Logger.critical(f"Explicit engine path not found: {binary_path}")
            sys.exit(1)
        Logger.info(f"Using enterprise binary: {binary_path}")
    else:
        proj_name = get_project_name(tool_root)
        if not proj_name:
            Logger.error("Could not find project() name in core-engine/CMakeLists.txt")
            sys.exit(0)

        # attempt to find binary
        binary_path = find_binary(tool_root, proj_name)
        
        # auto build if not found
        if not binary_path:
            build_binary(tool_root, )
            # reattempt after build
            binary_path = find_binary(tool_root, proj_name)
            if not binary_path:
                Logger.critical(f"Failed to find binary '{proj_name}' even after successful build attempt.")
                sys.exit(1)
            Logger.success(f"Discovered binary after build: {binary_path}")
        
        check_binary_out_of_date(tool_root, binary_path)

    # get staged files
    staged_files = get_staged_files()
    if not staged_files:
        Logger.success("No staged C++ files to analyze.")
        sys.exit(0)

    Logger.info(f"Analyzing {len(staged_files)} staged file(s)...")
    if args.job_name: Logger.info(f"Context: {args.job_name} (Build #{args.build_id})")
    
    # run core engine and add metadata
    findings = run_auditor(binary_path, staged_files, args)
    criticals = display_results(findings)

    # guardrail enforcement
    if criticals > 0:
        Logger.critical("GUARDRAIL FAILURE: High-severity violations detected.")
        sys.exit(1)
    
    Logger.success("GUARDRAIL PASSED: All checks successful.")
    sys.exit(0)

if __name__ == "__main__":
    main()
