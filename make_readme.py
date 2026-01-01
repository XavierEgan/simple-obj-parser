# automatically generate readme
import argparse
import os

README_PATH = "obj_parser/README.txt"

def main():
    parser = argparse.ArgumentParser("Generate README.txt")
    parser.add_argument("tag", type=str, help="The tag for the release")
    args = parser.parse_args()

    # if readme.txt exists, remove it
    if os.path.exists(README_PATH):
        os.remove(README_PATH)
    
    with open(README_PATH, "w") as f:
        f.write(f"OBJ Parser Release {args.tag}\nFor more info and documentation, visit: https://github.com/XavierEgan/simple-obj-parser")
    

if __name__ == "__main__":
    main()