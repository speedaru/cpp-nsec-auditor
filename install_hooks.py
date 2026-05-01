import os
import sys
import shutil
import platform
import stat
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

def install_hook():
    project_root = Path(__file__).parent.absolute()
    hook_source = project_root / "git-hooks" / "pre-commit"
    git_hooks_dir = project_root / ".git" / "hooks"
    hook_dest = git_hooks_dir / "pre-commit"

    # environment check
    if not (project_root / ".git").exists():
        log_error("Not a Git repository. Please run this script from the project root.")
        sys.exit(1)

    if not hook_source.exists():
        log_error(f"Source hook not found at {hook_source}. Ensure the git-hooks/ folder exists.")
        sys.exit(1)

    # deployment
    log_info(f"Installing hook to {hook_dest}...")
    
    try:
        # copy the file to ensure its a new script in .git/hooks
        shutil.copy2(hook_source, hook_dest)
        
        # set execution permissions (chmod +x)
        # s_ixusr: execute permission for owner
        current_stat = os.stat(hook_dest)
        os.chmod(hook_dest, current_stat.st_mode | stat.S_IEXEC | stat.S_IXGRP | stat.S_IXOTH)
        
        log_success("Git pre-commit hook installed and made executable.")
    except Exception as e:
        log_error(f"Failed to install hook: {e}")
        sys.exit(1)

    # binary readiness check (dx)
    check_binary_readiness(project_root)

def check_binary_readiness(root):
    """Inform the user if the C++ binary is missing so they know to build it."""
    engine_dir = root / "core-engine"
    
    project_name = get_project_name()
    binary_path = find_binary(project_name)

    if binary_path == None:
        SEPARATOR = "-" * 50
        print("\n" + SEPARATOR)
        log_warn("Hook installed, but C++ binary not found in core-engine/.")
        log_info("Please build the core-engine using CMake before committing code.")
        print(SEPARATOR + "\n")
    else:
        log_info("Verified: C++ auditor binary is present and ready.")

if __name__ == "__main__":
    log_bold(CLR_CYN, ">>> nsec-auditor: Pipeline Installer")
    install_hook()
