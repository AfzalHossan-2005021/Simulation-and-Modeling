#!/usr/bin/env python3
"""
Validation script for M/M/1 Queue Simulator.

This script runs various checks on the mm1_simulator.py file:
- Compilation check with py_compile
- Type checking with mypy
- Linting with pylint
- Style checking with flake8

Usage:
    python validate.py
"""

import subprocess
import sys
import os


def run_command(command: str) -> bool:
    """
    Run a shell command and return True if successful.

    Args:
        command: The command to run.

    Returns:
        bool: True if the command succeeded (return code 0), False otherwise.
    """
    try:
        print(f"\n{'='*50}")
        print(f"Running: {command}")
        print('='*50)
        result = subprocess.run(command, shell=True, capture_output=True,
                                text=True, cwd=os.path.dirname(__file__))
        if result.stdout:
            print("STDOUT:")
            print(result.stdout)
        if result.stderr:
            print("STDERR:")
            print(result.stderr)
        if result.returncode == 0:
            print("✓ Command succeeded")
            return True
        else:
            print(f"✗ Command failed with return code {result.returncode}")
            return False
    except Exception as e:
        print(f"✗ Error running command: {e}")
        return False


def main() -> None:
    """Main function to run all validation checks."""
    file_path = r"mm1_simulator.py"

    # Check if file exists
    if not os.path.exists(file_path):
        print(f"Error: File {file_path} does not exist.")
        sys.exit(1)

    commands = [
        f'python -m py_compile "{file_path}"',
        f'python -m mypy "{file_path}"',
        f'python -m pylint "{file_path}"',
        f'python -m flake8 "{file_path}" --max-line-length=120'
    ]

    all_passed = True
    for cmd in commands:
        if not run_command(cmd):
            all_passed = False

    print(f"\n{'='*50}")
    if all_passed:
        print("🎉 All checks passed!")
        sys.exit(0)
    else:
        print("❌ Some checks failed. Please review the output above.")
        sys.exit(1)


if __name__ == "__main__":
    main()
