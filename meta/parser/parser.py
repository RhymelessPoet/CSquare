import os
import sys

import argparse
from clang.cindex import Config
import clang.cindex as CX

from meta_info import Class, Namespace, Filed, Method, AttributeParser
from code_generator import CodeGenerator
import json

# Auto-detect script directory and project root from script location: meta/parser/ -> ../../
SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
PROJECT_ROOT = os.path.abspath(os.path.join(SCRIPT_DIR, '..', '..'))

# Configure libclang path using absolute path
LIBCLANG_PATH = os.path.abspath(os.path.join(
    SCRIPT_DIR, '..', 'bin', 'x64', 'libclang.dll'))
Config.set_library_file(LIBCLANG_PATH)

# Standard include paths for the project
STD_INCLUDE_ARGS = [
    '-x', 'c++',
    '-std=c++20',
    '-I' + os.path.join(PROJECT_ROOT, 'meta'),
    '-I' + os.path.join(PROJECT_ROOT, 'source', 'render'),
    '-I' + os.path.join(PROJECT_ROOT, 'source', 'engine'),
    '-I' + os.path.join(PROJECT_ROOT, 'thirdparty'),
]


def collect_source_files(path: str):
    """Recursively collect .h/.hpp/.cpp/.cc/.cxx files from a directory."""
    files = []
    for root, dirs, filenames in os.walk(path):
        # Skip generated directory to avoid recursive parsing of generated files
        dirs[:] = [d for d in dirs if d != 'generated']
        for fname in filenames:
            if fname.endswith(('.h', '.hpp', '.cpp', '.cc', '.cxx')):
                files.append(os.path.join(root, fname))
    return sorted(files)


def get_output_name(input_file: str) -> str:
    """Generate output file name from input file: Foo.h -> CSMetaRegister_Foo.cpp"""
    basename = os.path.splitext(os.path.basename(input_file))[0]
    return f"CSMetaRegister_{basename}.cpp"


class Parser:
    def __init__(self, input_file: str, cx_index: CX.Index, args):
        self._input_file = input_file
        self._cx_index = cx_index
        self._args = args
        self._is_debug = False
        self._namespaces = {'::': Namespace('::')}
        self._classes = {}
        self._current_namespace = Namespace('::')
        self._code_generator = None

    def __is_from_input_file(self, node: CX.Cursor) -> bool:
        """Check if a cursor's definition originates from the input file."""
        loc = node.location
        if not loc or not loc.file:
            return False
        return loc.file.name == self._input_file

    def __traverse__(self, node: CX.Cursor):
        is_from_input = self.__is_from_input_file(node)

        prev_namespace = self._current_namespace
        if node.kind == CX.CursorKind.NAMESPACE and is_from_input:
            self._current_namespace = Namespace(node.spelling)
            if node.spelling not in self._namespaces:
                self._namespaces[node.spelling] = self._current_namespace

        elif node.kind == CX.CursorKind.CLASS_DECL and is_from_input:
            class_ = Class(node, self._current_namespace,
                           self._attribute_parser)
            if class_.identify_name() not in self._classes:
                self._classes[class_.identify_name()] = class_
            if self._code_generator:
                self._code_generator.add_class(node, self._current_namespace)

        for child in node.get_children():
            self.__traverse__(child)

        if node.kind == CX.CursorKind.NAMESPACE and is_from_input:
            self._current_namespace = prev_namespace

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
        self._attribute_parser = AttributeParser(
            config['Attributes'], config['Tags'])

    def parse(self):
        translation_unit = self._cx_index.parse(self._input_file, self._args)

        if self._is_debug:
            self.__debug_traverse__(translation_unit.cursor)

        self.__traverse__(translation_unit.cursor)

    def namespaces(self):
        return self._namespaces.values()

    def classes(self):
        return self._classes.values()

    def generate_code(self, header_include: str = None) -> str:
        if self._code_generator:
            return self._code_generator.generate(header_include)
        return ''


def parse_single_file(input_file: str, config: dict, include_args: list, output_file: str = None, debug: bool = False) -> tuple:
    """Parse a single header/source file and generate registration code.
    Returns (generated_code, class_names).
    """
    index = CX.Index.create(excludeDecls=True)
    meta_parser = Parser(input_file, index, include_args)
    if debug:
        meta_parser.enable_traverse_debug()
    meta_parser.config_attribute_parser(config)
    meta_parser._code_generator = CodeGenerator(meta_parser._attribute_parser)

    meta_parser.parse()

    # Print parsed info
    for _class in meta_parser.classes():
        print(f'Class : {_class.identify_name()}')
        for attr in _class.attributes():
            print(f'  --- Attributes : {attr.name()} {attr.tags()}')
        for base_class in _class.base_classes():
            print(f'  --- Base Class : {base_class.name()}')
        for method in _class.methods():
            print(f'  --- Method : {method.name()}')
            for attr in method.attributes():
                print(f'      Attributes : {attr.name()} {attr.tags()}')
        for filed in _class.fileds():
            print(f'  --- Filed : {filed.name()}')
            for attr in filed.attributes():
                print(f'      Attributes : {attr.name()} {attr.tags()}')
        print('')

    # Compute header include path relative to the output file's directory
    header_include = None
    if output_file:
        out_dir = os.path.dirname(os.path.abspath(output_file))
        header_include = os.path.relpath(os.path.abspath(
            input_file), out_dir).replace('\\', '/')

    generated_code = meta_parser.generate_code(header_include)
    class_names = meta_parser._code_generator.class_names(
    ) if meta_parser._code_generator else []

    if output_file:
        if meta_parser._code_generator and meta_parser._code_generator.has_classes():
            with open(output_file, 'w') as out_file:
                out_file.write(generated_code)
            print(f'Generated registration code written to: {output_file}')
        else:
            print(f'Skipped: no meta-annotated classes in {input_file}')
    else:
        print('--- Generated UDRefl Registration Code ---')
        print(generated_code)

    return generated_code, class_names


def parse_directory(input_dir: str, config: dict, include_args: list, output_dir: str, debug: bool = False):
    """Recursively parse all source files in a directory and generate registration code."""
    source_files = collect_source_files(input_dir)
    if not source_files:
        print(f'No source files found in: {input_dir}')
        return

    os.makedirs(output_dir, exist_ok=True)
    print(f'Found {len(source_files)} source file(s) in {input_dir}')
    print(f'Output directory: {output_dir}')
    print('')

    all_class_names = []

    for input_file in source_files:
        output_name = get_output_name(input_file)
        output_file = os.path.join(output_dir, output_name)
        print(f'Processing: {input_file} -> {output_file}')
        _, class_names = parse_single_file(
            input_file, config, include_args, output_file, debug)
        all_class_names.extend(class_names)
        print('')

    # Generate aggregator file
    if all_class_names:
        aggregator_path = os.path.join(output_dir, 'CSMetaRegister_Auto.cpp')
        aggregator_code = CodeGenerator.generate_aggregator(all_class_names)
        with open(aggregator_path, 'w') as f:
            f.write(aggregator_code)
        print(
            f'Generated aggregator: {aggregator_path} ({len(all_class_names)} class(es))')
    else:
        print('No meta-annotated classes found in any file.')


def main():
    argparser = argparse.ArgumentParser(description='Meta Parser')
    argparser.add_argument('input_path', help='Input file or directory path')
    argparser.add_argument('parse_config_file',
                           help='Parse Config File Path')
    argparser.add_argument(
        '--output', '-o', help='Output file path (for single file mode)', default=None)
    argparser.add_argument(
        '--output-dir', '-d', help='Output directory (for directory mode)', default=None)
    argparser.add_argument('--debug', action='store_true',
                           help='Enable debug traverse')
    args = argparser.parse_args()

    with open(args.parse_config_file) as file:
        config = json.load(file)

    input_path = args.input_path

    # Build include args: standard project paths + input directory
    include_args = STD_INCLUDE_ARGS.copy()
    if os.path.isdir(input_path):
        include_args.append('-I' + os.path.abspath(input_path))
    else:
        include_args.append(
            '-I' + os.path.abspath(os.path.dirname(input_path)))

    if os.path.isdir(input_path):
        # Directory mode
        output_dir = args.output_dir or os.path.join(input_path, 'generated')
        parse_directory(input_path, config, include_args,
                        output_dir, args.debug)
    else:
        # Single file mode
        parse_single_file(input_path, config, include_args,
                          args.output, args.debug)


if __name__ == '__main__':
    main()
