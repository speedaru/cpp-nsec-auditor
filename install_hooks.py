import os
import sys
import shutil
import platform
import stat
import argparse
from pathlib import Path
from scripts.nsec_wrapper import get_project_name, find_binary

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

def install_hook(target_repo_path: Path):
    """
    installs the pre-commit hook into the specified target repository
    """
    # the source of the hook is relative to this script
    script_root = Path(__file__).parent.absolute()
    hook_source = script_root / "git-hooks" / "pre-commit"
    
    # the destination is relative to the target path provided by the user
    git_hooks_dir = target_repo_path / ".git" / "hooks"
    hook_dest = git_hooks_dir / "pre-commit"

    # environment checks
    if not target_repo_path.exists():
        log_error(f"Target path does not exist: {target_repo_path}")
        sys.exit(1)

    if not (target_repo_path / ".git").exists():
        log_error(f"Target directory is not a Git repository: {target_repo_path}")
        sys.exit(1)

    if not hook_source.exists():
        log_error(f"Source hook not found at {hook_source}. Ensure the git-hooks/ folder exists in the script directory")
        sys.exit(1)

    # deployment
    log_info(f"Installing hook to {hook_dest}...")
    
    try:
        # create hooks directory if it doesn't exist
        git_hooks_dir.mkdir(parents=True, exist_ok=True)
        
        # copy the file
        shutil.copy2(hook_source, hook_dest)
        
        # set execution permissions (chmod +x)
        current_stat = os.stat(hook_dest)
        os.chmod(hook_dest, current_stat.st_mode | stat.S_IEXEC | stat.S_IXGRP | stat.S_IXOTH)
        
        log_success(f"Git pre-commit hook installed successfully in: {target_repo_path.name}")
    except Exception as e:
        log_error(f"Failed to install hook: {e}")
        sys.exit(1)

    # binary readiness check (check script's local core-engine)
    check_binary_readiness(script_root)

def check_binary_readiness(script_root: Path):
    """inform the user if the c++ binary is missing in the primary tool directory"""
    project_name = get_project_name()
    binary_path = find_binary(project_name)

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
    target_path = Path(args.target).absolute()

    log_bold(CLR_CYN, ">>> nsec-auditor: Pipeline Installer")
    install_hook(target_path)

if __name__ == "__main__":
    main()
