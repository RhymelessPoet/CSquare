# import os

import argparse
from clang.cindex import Config
import clang.cindex as CX

from meta_info import Class, Namespace, Filed, Method, AttributeParser
import json

# 配置 libclang 路径
Config.set_library_file('../meta/bin/x64/libclang.dll')

class Parser:
    def __init__(self, input_file: str, cx_index: CX.Index, args):
        self._input_file = input_file
        self._cx_index = cx_index
        self._args = args
        self._is_debug = False
        self._namespaces = { '::' : Namespace('::')}
        self._classes = {}
        self._current_namespace = Namespace('::')

    def __traverse__(self, node: CX.Cursor):
        text = f"{str(node.kind).removeprefix('CursorKind.')}: {node.spelling}"
        children = list(node.get_children())

        if node.kind == CX.CursorKind.NAMESPACE:
            self._current_namespace = Namespace(node.spelling)
            if node.spelling not in self._namespaces:
                self._namespaces[node.spelling] = self._current_namespace

        elif node.kind == CX.CursorKind.CLASS_DECL:
            class_ = Class(node, self._current_namespace, self._attribute_parser)
            if class_.identify_name() not in self._classes:
                self._classes[class_.identify_name()] = class_

        for child in children:
            self.__traverse__(child)

    def __debug_traverse__(self, node: CX.Cursor, prefix="", is_last=True):
        branch = "└──" if is_last else "├──"
        text = f"{str(node.kind).removeprefix('CursorKind.')}: {node.spelling}"

        if node.kind == CX.CursorKind.INTEGER_LITERAL:
            value = list(node.get_tokens())[0].spelling
            text = f"{text}{value}"

        print(f"{prefix}{branch} {text}")
        new_prefix = prefix + ("    " if is_last else "│   ")
        children = list(node.get_children())

        for child in children:
            self.__debug_traverse__(child, new_prefix, child is children[-1])

    def enable_traverse_debug(self):
        self._is_debug = True

    def config_attribute_parser(self, config):
        self._attribute_parser = AttributeParser(config['Attributes'], config['Tags'])

    def parse(self):
        translation_unit = self._cx_index.parse(self._input_file, self._args)

        if self._is_debug :
            self.__debug_traverse__(translation_unit.cursor)

        self.__traverse__(translation_unit.cursor)

    def namespaces(self):
        return self._namespaces.values()

    def classes(self):
        return self._classes.values()


def main():
    argparser = argparse.ArgumentParser(description='Meta Parser')
    argparser.add_argument('input_file', help='Input File Path')
    argparser.add_argument('parse_config_file', help='Parse Config File Path')
    args = argparser.parse_args()

    with open(args.parse_config_file) as file:
        config = json.load(file)

    index = CX.Index.create(excludeDecls=True)

    meta_parser = Parser(args.input_file, index, ['-std=c++20'])
    # meta_parser.enable_traverse_debug()
    meta_parser.config_attribute_parser(config)

    meta_parser.parse()

    # for namespace in meta_parser.namespaces():
    #     print('Namespace : ', namespace.identify_name())
    # print('')

    # for _class in meta_parser.classes():
    #     print('Class : ', _class.identify_name())

    #     for attr in _class.attributes():
    #         print('--- Attributes : ', attr.name())

    #     for base_class in _class.base_classes():
    #         print('--- Base Class : ', base_class.name())

    #     for method in _class.methods():
    #         print('--- Method : ', method.name())

    #     for filed in _class.fileds():
    #         print('--- Filed : ', filed.name())

    #     print('')

if __name__ == '__main__':
    main()