import os
import sys
import json
import zipfile
import shutil
import pathlib

# Use the directory containing this script as the root.
ROOT_DIR = pathlib.Path(__file__).parent.parent.resolve()

def download_and_extract_dependencies(cxx_standard: int = 20):
    """Download and extract nlohmann/json and the SQLite amalgamation for the given C++ standard."""
    if cxx_standard >= 20:
        json_url = "https://github.com/nlohmann/json/releases/download/v3.11.3/json.hpp"
        sqlite_url = "https://www.sqlite.org/2024/sqlite-amalgamation-3450100.zip"
    else:
        json_url = "https://github.com/nlohmann/json/releases/download/v3.10.5/json.hpp"
        sqlite_url = "https://www.sqlite.org/2023/sqlite-amalgamation-3410200.zip"

    # 1. Download nlohmann/json.
    json_dest_dir = ROOT_DIR / "deps" / "json"
    json_dest_dir.mkdir(parents=True, exist_ok=True)
    json_header = json_dest_dir / "json.hpp"

    if not json_header.exists():
        print(f"Downloading json.hpp for C++{cxx_standard}...")
        urllib.request.urlretrieve(json_url, json_header)

    # 2. Download and extract the SQLite amalgamation.
    sqlite_dest_dir = ROOT_DIR / "deps" / "sqlite"
    sqlite_dest_dir.mkdir(parents=True, exist_ok=True)
    sqlite_c_file = sqlite_dest_dir / "sqlite3.c"

    if not sqlite_c_file.exists():
        print("Downloading SQLite amalgamation...")
        temp_dir = ROOT_DIR / "build_temp"
        temp_dir.mkdir(exist_ok=True)
        sqlite_zip = temp_dir / "sqlite_temp.zip"
        sqlite_tmp_extract = temp_dir / "sqlite_tmp"

        try:
            urllib.request.urlretrieve(sqlite_url, sqlite_zip)
            with zipfile.ZipFile(sqlite_zip, 'r') as zip_ref:
                zip_ref.extractall(sqlite_tmp_extract)

            extracted_dirs = list(sqlite_tmp_extract.glob("sqlite-amalgamation-*"))
            if extracted_dirs:
                src_extracted = extracted_dirs[0]
                shutil.copy(src_extracted / "sqlite3.c", sqlite_dest_dir / "sqlite3.c")
                shutil.copy(src_extracted / "sqlite3.h", sqlite_dest_dir / "sqlite3.h")
        finally:
            if temp_dir.exists():
                shutil.rmtree(temp_dir)

if __name__ == "__main__":
    cxx_std = int(sys.argv[1]) if len(sys.argv) > 1 else 20
    print(f"[*] Processing for C++{cxx_std}...")

    download_and_extract_dependencies(cxx_std)

    print("[+] All dependencies downloaded successfully!")