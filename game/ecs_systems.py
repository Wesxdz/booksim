import clang.cindex
import sys
clang.cindex.Config.set_library_path('/usr/local/clang+llvm-16.0.0-x86_64-linux-gnu-ubuntu-18.04/lib')

def find_macros(node):
    if node.kind == clang.cindex.CursorKind.MACRO_DEFINITION:
        print(node.spelling)
        if node.spelling == 'ECS_SYSTEM':
            macro_args = []
            for c in node.get_children():
                if c.kind == clang.cindex.CursorKind.MACRO_INSTANTIATION:
                    for arg in c.get_arguments():
                        macro_args.append(arg.spelling)
                    if len(macro_args) > 1:  # If there are more than one argument
                        return macro_args[1]  # return the second argument (function name)
    # Recurse for children of this node
    for child in node.get_children():
        res = find_macros(child)
        if res is not None:
            return res

def main():
    index = clang.cindex.Index.create()
    tu = index.parse(sys.argv[1])
    print('Translation unit:', tu.spelling)

    with open('output.txt', 'w') as file:
        for node in tu.cursor.walk_preorder():
            print(node.spelling)
            function_name = find_macros(node)
            if function_name is not None:
                file.write(function_name + '\n')

if __name__ == '__main__':
    main()
