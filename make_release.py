import os
import shutil
import sys

def is_code(file_path: str) -> bool:
    return file_path.endswith(".cpp") or file_path.endswith(".c") or file_path.endswith(".hpp") or file_path.endswith(".h")

def get_code_files(src_dir: str, exclude: list[str] = []) -> list[str]:
    # get all the .cpp files from src recursively
    code_files: list[str] = []
    for root, _, files in os.walk(src_dir):
        # make sure the directory is not in the exclude list
        if (any(excl in root for excl in exclude)):
            continue

        # add all the code files into the list
        for file in files:
            if file in exclude:
                continue

            if is_code(file):
                code_files.append(os.path.join(root, file))
        
    return code_files

def make_release(version: str):
    os.makedirs(f"release", exist_ok=False)

    # files that have to go in a specific order
    include_files = [
        "include/Mesh.hpp",
        "include/Material.hpp",
        "include/ObjParserError.hpp",
        "include/MtlParser.hpp",
        "include/ObjParser.hpp"
    ]

    # what the user imports
    header_only_library_file: str = f"obj_parser.hpp"

    src_files = get_code_files("src")

    def dont_write_line(line: str) -> bool:
        # skip include guards and pragma once
        is_include:bool = line.startswith("#include \"") and not line.startswith("#include \"ext/")
        is_pragma:bool = line.startswith("#pragma once")
        is_guard:bool = line.startswith("#ifndef ") or line.startswith("#define ") or line.startswith("#endif")

        return is_include or is_pragma or is_guard

    # make the include file
    with open(f"release/{header_only_library_file}", "w") as incl_f:
        incl_f.write(f"// Obj Parser v{version}\n")

        # write all the include files
        for file in include_files:
            with open(file, "r") as f:
                for line in f:
                    if dont_write_line(line):
                        continue

                    incl_f.write(line)
                incl_f.write("\n")

        # if the user defines OBJ_PARSER_IMPLEMENTATION then they want the source compiled for this translation unit
        incl_f.write(f"#if defined(OBJ_PARSER_IMPLEMENTATION)\n")

        for file in src_files:
            with open(file, "r") as f:
                for line in f:
                    incl_f.write(line)
                incl_f.write("\n")

        incl_f.write(f"\n#endif\n")

def remove_releases():
    for item in os.listdir("."):
        if item == "release" and os.path.isdir(item):
            shutil.rmtree(item)
            print(f"Removed directory: {item}")

def copy_ext():
    EXT_DIR = "include/ext"
    DEST_DIR = "release/ext"

    if os.path.exists(EXT_DIR):
        shutil.copytree(EXT_DIR, DEST_DIR)
        print(f"Copied {EXT_DIR} to {DEST_DIR}")

if __name__ == "__main__":
    remove_releases()
    version = sys.argv[1]
    make_release(version)
    copy_ext()