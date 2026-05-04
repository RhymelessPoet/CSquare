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


def _has_meta_annotation_fast(file_path: str) -> bool:
    """Quick text scan to check if a file contains [[Meta annotations.
    This avoids expensive libclang parsing for files without meta markers."""
    try:
        with open(file_path, 'r', encoding='utf-8', errors='ignore') as f:
            # Read in chunks to avoid loading huge files
            chunk_size = 8192
            while True:
                chunk = f.read(chunk_size)
                if not chunk:
                    return False
                if '[[' in chunk and 'Meta' in chunk:
                    # More precise check: look for [[Meta pattern
                    # We need to handle the case where the pattern spans chunk boundaries
                    # by reading a bit more context
                    pos = chunk.find('[[')
                    while pos != -1:
                        # Check if 'Meta' appears near this position
                        search_start = max(0, pos - 10)
                        search_end = min(len(chunk), pos + 30)
                        segment = chunk[search_start:search_end]
                        if 'Meta' in segment:
                            return True
                        pos = chunk.find('[[', pos + 1)
    except (IOError, OSError):
        return False
    return False


def collect_source_files(path: str, fast_filter: bool = True):
    """Recursively collect .h/.hpp/.cpp/.cc/.cxx files from a directory.
    If fast_filter is True, skip files that don't contain [[Meta annotations."""
    files = []
    for root, dirs, filenames in os.walk(path):
        # Skip generated directory to avoid recursive parsing of generated files
        dirs[:] = [d for d in dirs if d != 'generated']
        for fname in filenames:
            if fname.endswith(('.h', '.hpp', '.cpp', '.cc', '.cxx')):
                fpath = os.path.join(root, fname)
                if fast_filter and not _has_meta_annotation_fast(fpath):
                    continue
                files.append(fpath)
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

    def _has_meta_annotation(self, node: CX.Cursor) -> bool:
        """Fast check if a cursor has a [[Meta]] annotation."""
        for child in node.get_children():
            if child.kind == CX.CursorKind.ANNOTATE_ATTR:
                attrs = self._attribute_parser.parse_attributes(child.spelling)
                if any(a.name() == 'Meta' for a in attrs):
                    return True
        return False

    def __traverse__(self, node: CX.Cursor):
        is_from_input = self.__is_from_input_file(node)

        prev_namespace = self._current_namespace
        if node.kind == CX.CursorKind.NAMESPACE and is_from_input:
            self._current_namespace = Namespace(node.spelling)
            if node.spelling not in self._namespaces:
                self._namespaces[node.spelling] = self._current_namespace

        elif node.kind == CX.CursorKind.CLASS_DECL and is_from_input:
            # Fast path: only process classes with [[Meta]] annotation
            if self._has_meta_annotation(node):
                class_ = Class(node, self._current_namespace,
                               self._attribute_parser)
                if class_.identify_name() not in self._classes:
                    self._classes[class_.identify_name()] = class_
                if self._code_generator:
                    self._code_generator.add_class(
                        node, self._current_namespace)
            # Skip traversing children of non-meta classes entirely
            return

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


def _write_if_changed(file_path: str, content: str) -> bool:
    """Write content to file only if it differs from existing content.
    Returns True if the file was actually written or created."""
    if os.path.exists(file_path):
        with open(file_path, 'r', encoding='utf-8') as f:
            existing = f.read()
        if existing == content:
            return False
    with open(file_path, 'w', encoding='utf-8') as f:
        f.write(content)
    return True


def parse_single_file(input_file: str, config: dict, include_args: list,
                      output_file: str = None, debug: bool = False,
                      verbose: bool = False, manage_stale: bool = True,
                      cx_index: CX.Index = None) -> tuple:
    """Parse a single header/source file and generate registration code.
    Returns (generated_code, class_names).
    If cx_index is provided, reuses it instead of creating a new one.
    """
    index = cx_index if cx_index else CX.Index.create(excludeDecls=True)
    meta_parser = Parser(input_file, index, include_args)
    if debug:
        meta_parser.enable_traverse_debug()
    meta_parser.config_attribute_parser(config)
    meta_parser._code_generator = CodeGenerator(meta_parser._attribute_parser)

    meta_parser.parse()

    # Print parsed info (only in verbose mode)
    if verbose:
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
            written = _write_if_changed(output_file, generated_code)
            if written:
                print(f'Generated registration code written to: {output_file}')
            else:
                print(f'Up-to-date: {output_file}')
        else:
            # Remove stale output if it exists and input no longer has meta classes
            if manage_stale and os.path.exists(output_file):
                os.remove(output_file)
                print(f'Removed stale: {output_file}')
            elif verbose:
                print(f'Skipped: no meta-annotated classes in {input_file}')
    else:
        print('--- Generated UDRefl Registration Code ---')
        print(generated_code)

    return generated_code, class_names


def _extract_class_names_from_generated(file_path: str) -> list:
    """Extract class names from an already-generated registration file
    by scanning for 'void Register_XXX(' patterns."""
    if not os.path.exists(file_path):
        return []
    try:
        with open(file_path, 'r', encoding='utf-8') as f:
            content = f.read()
        import re
        matches = re.findall(r'void\s+Register_(\w+)\s*\(', content)
        return matches
    except (IOError, OSError):
        return []


def parse_directory(input_dir: str, config: dict, include_args: list,
                    output_dir: str, debug: bool = False, verbose: bool = False):
    """Recursively parse all source files in a directory and generate registration code."""
    source_files = collect_source_files(input_dir)
    if not source_files:
        print(f'No source files found in: {input_dir}')
        return

    os.makedirs(output_dir, exist_ok=True)
    if verbose:
        print(f'Found {len(source_files)} source file(s) in {input_dir}')
        print(f'Output directory: {output_dir}')
        print('')

    all_class_names = []
    generated_outputs = set()  # Track output files that should exist

    # Reuse a single CX.Index across all files to reduce overhead
    shared_index = CX.Index.create(excludeDecls=True)

    for input_file in source_files:
        output_name = get_output_name(input_file)
        output_file = os.path.join(output_dir, output_name)
        output_file_abs = os.path.abspath(output_file)

        # Check if output is newer than input; if so, skip expensive parsing
        skip_parse = False
        if os.path.exists(output_file):
            input_mtime = os.path.getmtime(input_file)
            output_mtime = os.path.getmtime(output_file)
            if output_mtime >= input_mtime:
                class_names = _extract_class_names_from_generated(output_file)
                if class_names:
                    all_class_names.extend(class_names)
                    generated_outputs.add(output_file_abs)
                skip_parse = True
                if verbose:
                    print(f'Skipped (up-to-date): {input_file}')

        if not skip_parse:
            if verbose:
                print(f'Processing: {input_file} -> {output_file}')

            _, class_names = parse_single_file(
                input_file, config, include_args, output_file, debug, verbose,
                manage_stale=False, cx_index=shared_index)
            all_class_names.extend(class_names)

            if class_names:
                generated_outputs.add(output_file_abs)

            if verbose:
                print('')

    # Remove stale output files that are no longer generated by any source
    for fname in os.listdir(output_dir):
        if fname == 'CSMetaRegister_Auto.cpp':
            continue
        fpath = os.path.join(output_dir, fname)
        if os.path.isfile(fpath) and os.path.abspath(fpath) not in generated_outputs:
            os.remove(fpath)
            print(f'Removed stale: {fpath}')

    # Generate aggregator file
    if all_class_names:
        aggregator_path = os.path.join(output_dir, 'CSMetaRegister_Auto.cpp')
        aggregator_code = CodeGenerator.generate_aggregator(all_class_names)
        written = _write_if_changed(aggregator_path, aggregator_code)
        if written:
            print(
                f'Generated aggregator: {aggregator_path} ({len(all_class_names)} class(es))')
        elif verbose:
            print(f'Aggregator up-to-date: {aggregator_path}')
    else:
        aggregator_path = os.path.join(output_dir, 'CSMetaRegister_Auto.cpp')
        if os.path.exists(aggregator_path):
            os.remove(aggregator_path)
            print(f'Removed stale aggregator: {aggregator_path}')
        elif verbose:
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
    argparser.add_argument('--verbose', '-v', action='store_true',
                           help='Enable verbose output (print parsed class/method/field details)')
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
                        output_dir, args.debug, args.verbose)
    else:
        # Single file mode
        parse_single_file(input_path, config, include_args,
                          args.output, args.debug, args.verbose)


if __name__ == '__main__':
    main()
