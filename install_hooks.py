import os
import sys
import shutil
import platform
import stat
import argparse
import re
from pathlib import Path
from scripts.nsec_wrapper import get_project_name, find_binary, get_tool_root

# constants
CLR_RED = "\033[91m"
CLR_YLW = "\033[93m"
CLR_GRN = "\033[92m"
CLR_CYN = "\033[96m"
CLR_RST = "\033[0m"
CLR_BLD = "\033[1m"

def log_info(msg): print(f"{CLR_CYN}[INFO]{CLR_RST} {msg}")
def log_success(msg): print(f"{CLR_GRN}[SUCCESS]{CLR_RST} {msg}")
def log_warn(msg): print(f"{CLR_YLW}[WARN]{CLR_RST} {msg}")
def log_error(msg): print(f"{CLR_RED}[ERROR]{CLR_RST} {msg}")
def log_bold(color, msg): print(f"{CLR_BLD}{color}{msg}{CLR_RST}")

def patch_hook_content(source_content: str, wrapper_path: Path) -> str:
    """
    patches the pre-commit shell script to use absolute paths and the correct --path argument
    paths are formatted according to the current OS
    """
    # determine OS dependant path representation
    if platform.system() == "Windows":
        # for windows we escape \
        wrapper_str = str(wrapper_path).replace("\\", "\\\\")
    else:
        # standard unix paths
        wrapper_str = str(wrapper_path)

    # locate the execution line template and replace with patched version
    execution_pattern = r'\$PYTHON_EXE\s+scripts/nsec_wrapper\.py'
    patched_line = f'$PYTHON_EXE "{wrapper_str}"'
    
    new_content = re.sub(execution_pattern, patched_line, source_content)
    
    return new_content

def install_hook(target_repo_path: Path):
    """
    installs the pre-commit hook into the specified target repository
    """
    # the source of the hook is relative to this script
    auditor_tool_root = get_tool_root()
    hook_source_file = auditor_tool_root / "git-hooks" / "pre-commit"
    wrapper_path = auditor_tool_root / "scripts" / "nsec_wrapper.py"
    
    git_hooks_dir = target_repo_path / ".git" / "hooks"
    hook_dest = git_hooks_dir / "pre-commit"

    # environment checks
    if not target_repo_path.exists() or not (target_repo_path / ".git").exists():
        log_error(f"Target is not a valid Git repository: {target_repo_path}")
        sys.exit(1)

    if not hook_source_file.exists():
        log_error(f"Source hook template not found: {hook_source_file}")
        sys.exit(1)

    # patching and deployment
    log_info(f"Patching and installing hook to {target_repo_path.name}...")
    
    try:
        # read template
        with open(hook_source_file, "r") as f:
            content = f.read()
        
        # patch content with OS pecific paths
        patched_content = patch_hook_content(content, wrapper_path)
        
        # write to destination
        git_hooks_dir.mkdir(parents=True, exist_ok=True)

        # specfiy newline to ensure we don't mess up line endings
        with open(hook_dest, "w", newline='\n') as f: 
            f.write(patched_content)
        
        # set execution permissions (chmod +x)
        current_stat = os.stat(hook_dest)
        os.chmod(hook_dest, current_stat.st_mode | stat.S_IEXEC | stat.S_IXGRP | stat.S_IXOTH)
        
        log_success("Hook installed with surgical path patching.")
        log_info(f"Wrapper (OS Path): {wrapper_path}")
        log_info(f"Target (OS Path):  {target_repo_path}")
        
    except Exception as e:
        log_error(f"Failed to install/patch hook: {e}")
        sys.exit(1)

    check_binary_readiness()

def check_binary_readiness():
    """inform the user if the c++ binary is missing in the primary tool directory"""
    tool_root = get_tool_root()
    project_name = get_project_name(tool_root)
    binary_path = find_binary(tool_root, project_name)

    if binary_path is None:
        SEPARATOR = "-" * 50
        print("\n" + SEPARATOR)
        log_warn("Hook installed, but C++ binary not found in core-engine/")
        log_info("Please build the core-engine using CMake so the hooks can actually run")
        print(SEPARATOR + "\n")
    else:
        log_info(f"Verified: C++ auditor binary found at {binary_path}")

def main():
    parser = argparse.ArgumentParser(description="nsec-auditor Git Hook Installer")
    parser.add_argument(
        "target", 
        nargs="?", 
        default=".", 
        help="The path to the Git repository where you want to install the hook (default: current directory)"
    )
    
    args = parser.parse_args()
    target_path = Path(args.target)

    log_bold(CLR_CYN, ">>> nsec-auditor: Pipeline Installer")
    install_hook(target_path)

if __name__ == "__main__":
    main()
