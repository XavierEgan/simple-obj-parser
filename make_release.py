import os
import shutil

release_text = "release_"

# release format = major_minor_patch
# example release_v_1_0_0

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
            if is_code(file):
                code_files.append(os.path.join(root, file))
        
    return code_files

def make_release(version):
    try:
        os.makedirs(f"release", exist_ok=False)
    except FileExistsError:
        print(f"Release directory release already exists. Aborting.")
        return

    # what the user imports
    include_file: str = f"obj_parser.hpp"

    src_files = get_code_files("src")
    incl_files = get_code_files("include", ["ext"])

    def dont_write_line(line: str) -> bool:
        # skip include guards and pragma once
        is_include:bool = line.startswith("#include \"") and not line.startswith("#include \"ext/")
        is_pragma:bool = line.startswith("#pragma once")
        is_guard:bool = line.startswith("#ifndef ") or line.startswith("#define ") or line.startswith("#endif")

        return is_include or is_pragma or is_guard

    # make the include file
    with open(f"release/{include_file}", "w") as incl_f:
        # write all the include files
        for file in incl_files:
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
    
def test():
    for root, _, files in os.walk("include"):
        if (any(excl in root for excl in include_dir_exclude_dir)):
            continue

        for file in files:
            if file.endswith(".hpp"):
                print(f"root: {root}, file: {file}")

def remove_releases():
    for item in os.listdir("."):
        if item == "release" and os.path.isdir(item):
            shutil.rmtree(item)
            print(f"Removed directory: {item}")

def get_release() -> str:
    with open("release.txt", "r") as f:
        version: str = f.read().strip()
    return version

if __name__ == "__main__":
    remove_releases()
    release: str = get_release()
    make_release(release)