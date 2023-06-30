from gettext import translation
from tokenize import Token
import clang.cindex
import typing
from pprint import pprint

flecs_systems = []
flecs_system_declarations = []

def get_flecs_system_name(system):
    for token in system.get_tokens():
        if token.kind == clang.cindex.TokenKind.IDENTIFIER:
            return token.spelling
    return "INVALID SYSTEM"


def is_flecs_system_declaration(line_tokens):
    return len([token for token in line_tokens if token.kind == clang.cindex.TokenKind.IDENTIFIER and token.spelling == "ECS_SYSTEM"])

def is_flecs_system_definition(node):
    return node.kind == clang.cindex.CursorKind.FUNCTION_DECL and len([token for token in node.get_tokens() if token.kind == clang.cindex.TokenKind.IDENTIFIER and token.spelling == 'ecs_iter_t'])

def populate_flecs_data():

    flecs_systems.clear()
    flecs_system_declarations.clear()

    index = clang.cindex.Index.create()
    file = 'src/game.c'
    translation_unit = index.parse(file, args=[''])
    for node in translation_unit.cursor.get_children():
        if node.location.file.name == file:
            if is_flecs_system_definition(node):
                flecs_systems.append(node)
            else:
                i = 0
                last_line = 0
                line_tokens = []
                for token in node.get_tokens():
                    if i == 0 or last_line == token.location.line:
                        line_tokens.append(token)
                    else:
                        if is_flecs_system_declaration(line_tokens):
                            flecs_system_declarations.append(line_tokens)
                        line_tokens = []
                        line_tokens.append(token)
                        i = 0
                    last_line = token.location.line
                    i += 1
# for i in translation_unit.get_tokens(extent=translation_unit.cursor.extent):
#     print("{} is {} range {}".format(i.kind, i.spelling, i.extent))

# Identify all components within a file
components_file = "game.c"

components = []

def populate_components():
    index = clang.cindex.Index.create()
    translation_unit = index.parse(components_file, args=[''])
    for cursor in translation_unit.cursor.get_children():
        if cursor.location.file.name == components_file:
            if cursor.kind == clang.cindex.CursorKind.TYPEDEF_DECL:
                idens = []
                for token in cursor.get_tokens():
                    if token.kind == clang.cindex.TokenKind.IDENTIFIER:
                        idens.append(token.spelling)
                components.append(idens[-1])