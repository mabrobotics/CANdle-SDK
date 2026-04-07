#!/usr/bin/env python3
"""
Verify Markdown Configuration
Performs a bash command on all files from a provided directory and checks their error codes.
Stops on the first error encountered.
"""

import argparse
import os
import subprocess
import sys
from pathlib import Path

glob_command = "candletool md config verify {}"
test_command = "candletool -h"


def run_command_on_files(directory: str, command: str, verbose: bool = False) -> int:
    """
    Execute a bash command on all files in the directory.

    Args:
        directory: Path to the directory containing files
        command: Bash command to execute (use {} as placeholder for filename)
        verbose: If True, print detailed output

    Returns:
        0 if all commands succeeded, 1 if any command failed
    """
    # Validate directory exists
    dir_path = Path(directory)
    if not dir_path.is_dir():
        print(f"Error: Directory '{directory}' does not exist", file=sys.stderr)
        return 1

    # Get all files in the directory (non-recursive)
    files = sorted([f for f in dir_path.iterdir() if f.is_file()])

    if not files:
        print(f"Warning: No files found in directory '{directory}'")
        return 0

    if verbose:
        print(f"Found {len(files)} file(s) in '{directory}'")
        print(f"Command template: {command}\n")

    # Execute command for each file
    for file_path in files:
        # Replace placeholder with actual file path
        actual_command = command.replace("{}", str(file_path))

        if verbose:
            print(f"Executing: {actual_command}")

        try:
            # Execute the command
            result = subprocess.run(
                actual_command, shell=True, capture_output=not verbose, text=True
            )

            # Check error code
            if result.returncode != 0:
                print(
                    f"Error: Command failed for file '{file_path.name}'\n"
                    f"Command: {actual_command}\n"
                    f"Exit code: {result.returncode}",
                    file=sys.stderr,
                )
                if result.stdout:
                    print(f"STDOUT:\n{result.stdout}", file=sys.stderr)
                if result.stderr:
                    print(f"STDERR:\n{result.stderr}", file=sys.stderr)
                return 1

            if verbose:
                print(f"✓ Success (exit code: {result.returncode})\n")

        except Exception as e:
            print(
                f"Error: Exception occurred while processing '{file_path.name}': {e}",
                file=sys.stderr,
            )
            return 1

    if verbose:
        print(f"All {len(files)} file(s) processed successfully!")

    return 0


def main():
    parser = argparse.ArgumentParser(
        description="Perform a bash command on all files in a directory and check error codes"
    )
    parser.add_argument("directory", help="Directory containing files to process")
    parser.add_argument(
        "-v", "--verbose", action="store_true", help="Print detailed output"
    )

    args = parser.parse_args()

    # Verify candletool exists
    try:
        subprocess.run(test_command, shell=True, check=True, capture_output=True)
    except subprocess.CalledProcessError:
        print(
            "Error: 'candletool' command not found. Please ensure it is installed and in your PATH.",
            file=sys.stderr,
        )
        sys.exit(1)

    exit_code = run_command_on_files(args.directory, glob_command, args.verbose)
    sys.exit(exit_code)


if __name__ == "__main__":
    main()
