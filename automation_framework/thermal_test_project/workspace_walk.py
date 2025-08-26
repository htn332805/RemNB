
#!/usr/bin/env python3
import os
import argparse

# Base output name (without .txt extension so we can add parts)
OUTPUT_FILE_BASE = "workspace_content"
OUTPUT_FILE_EXT = ".txt"

# Max size in bytes (24 MB)
MAX_SIZE = 23 * 1024 * 1024

# Folders to skip entirely
IGNORE_DIRS = {".git", ".env", "SensitiveFolder", "__pycache__", "automation", "Data", "Logs" }

def is_binary_file(filepath):
    """
    Detect if a file is binary by checking its first 1024 bytes.
    """
    try:
        with open(filepath, "rb") as f:
            chunk = f.read(1024)
        # If it contains null byte → likely binary
        return b"\0" in chunk
    except Exception:
        return True  # Treat unreadable files as binary

def open_new_output_file(part_number):
    """Helper: open a new numbered output file stream."""
    filename = f"{OUTPUT_FILE_BASE}_part{part_number}{OUTPUT_FILE_EXT}"
    return filename, open(filename, "w", encoding="utf-8")

def collect_files(root_dir):
    part_number = 1
    current_size = 0
    output_filename, out = open_new_output_file(part_number)

    for dirpath, dirnames, filenames in os.walk(root_dir):
        # Remove ignored directories in-place
        dirnames[:] = [d for d in dirnames if d not in IGNORE_DIRS]

        for filename in filenames:
            filepath = os.path.join(dirpath, filename)
            rel_path = os.path.relpath(filepath, start=os.path.dirname(root_dir))

            header = f"#-----CONTENT FOR: {rel_path}----------------------\n"
            footer = f"#-----THE END Of CONTENT FOR: {rel_path}----------------------\n\n"

            # Prepare file content
            if is_binary_file(filepath):
                body = "         Leave blank since it is a binary/non_text data\n"
            else:
                try:
                    with open(filepath, "r", encoding="utf-8", errors="ignore") as f:
                        body = f.read()
                except Exception as e:
                    body = f"         [Error reading file: {e}]\n"

            file_content = header + body + footer
            encoded_content = file_content.encode("utf-8")
            content_size = len(encoded_content)

            # If it won't fit, rotate to a new file
            if current_size + content_size > MAX_SIZE:
                out.close()
                part_number += 1
                output_filename, out = open_new_output_file(part_number)
                current_size = 0

            # Write to the current file
            out.write(file_content)
            current_size += content_size

    out.close()
    print(f"[✓] Workspace content collected into {output_filename} (possibly multiple parts)")

def main():
    parser = argparse.ArgumentParser(
        description="Recursively gather all text file contents into workspace_content.txt (split into 24MB parts if needed)"
    )
    parser.add_argument(
        "root_dir",
        help="Root directory of the workspace (e.g., thermal_test_framework/)"
    )
    args = parser.parse_args()

    collect_files(args.root_dir)

if __name__ == "__main__":
    main()
