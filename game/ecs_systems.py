import clang.cindex
import sys
# clang.cindex.Config.set_library_path('/usr/local/clang+llvm-16.0.0-x86_64-linux-gnu-ubuntu-18.04/lib')

def find_macros(node, systems):
    if node.spelling.startswith('FLECS__F'):  # Check if spelling matches 'FLECS__F{system_name}' pattern
        systems.add(node.spelling[len('FLECS__F'):])  # Add to the set, automatically removing duplicates
    # Recurse for children of this node
    for child in node.get_children():
        find_macros(child, systems)

def main():
    index = clang.cindex.Index.create()
    tu = index.parse(sys.argv[1])
    print('Translation unit:', tu.spelling)

    systems = set()  # Store unique system names

    for node in tu.cursor.walk_preorder():

        find_macros(node, systems)

    with open("systems.txt", "w+") as f:
        # Write each unique system name followed by a new line
        f.writelines(system + "\n" for system in systems)

if __name__ == '__main__':
    main()
