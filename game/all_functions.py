import clang.cindex
import sys
clang.cindex.Config.set_library_path('/usr/local/clang+llvm-16.0.0-x86_64-linux-gnu-ubuntu-18.04/lib')

def extract_functions(node, filename):
    """Extract function names from the node."""
    if node.kind == clang.cindex.CursorKind.FUNCTION_DECL:
        if node.location.file is not None and node.location.file.name == filename:
            print(node.spelling)
    # Recurse for children of this node
    for c in node.get_children():
        extract_functions(c, filename)

index = clang.cindex.Index.create()
filename = 'main.c'
tu = index.parse(filename, ['-DECS_STRUCT(...)'])

with open('functions.txt', 'w') as file:
    old_stdout = sys.stdout
    sys.stdout = file

    extract_functions(tu.cursor, filename)

    sys.stdout = old_stdout

