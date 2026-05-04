import clang.cindex as CX


class Attribute:
    def __init__(self, name: str, tags: dict[str, str]):
        self._name = name
        self._tags = tags

    def name(self):
        return self._name

    def tags(self):
        return self._tags


class AttributeParser:
    def __init__(self, attribute_keys: list[str], tags: dict[str, list[str]]):
        self._attribute_keys = attribute_keys
        self._tags = tags

    def __attributes__(self, annotation: str):
        attr_values = {}
        left_parenthesis = 0
        stack = []
        parenthesis_index = -1
        for char in annotation:
            if (char in [' ', ',']) and left_parenthesis == 0:
                continue

            stack.append(char)

            if char == '(':
                if left_parenthesis == 0:
                    parenthesis_index = len(stack)
                left_parenthesis += 1

            if char == ')':
                if left_parenthesis == 1:
                    attr = ''.join(stack[0: parenthesis_index - 1])
                    attr_values[attr] = ''.join(
                        stack[parenthesis_index - 1: len(stack)])
                    stack = []
                    left_parenthesis = 0
                    parenthesis_index = -1
                else:
                    left_parenthesis -= 1

        return {k: attr_values[k] for k in attr_values.keys() & self._attribute_keys}

    def __tags__(self, attr: str, values: str):
        tags = {}
        tag_keys = self._tags[attr]

        name_values = []

        start_index = 0
        parenthesis = 0
        for index, char in enumerate(values):
            if char == '(':
                parenthesis += 1
            if char == ')':
                parenthesis -= 1
            if char == ',' and parenthesis == 0:
                name_values.append(values[start_index: index].strip())
                start_index = index + 1

        if start_index < len(values):
            name_values.append(values[start_index: len(values)].strip())

        for name_value in name_values:
            tag = name_value.split('=')

            if len(tag) == 2:
                tags[tag[0].strip()] = tag[1].strip()
            else:
                tags[tag[0].strip()] = ''

        return {k: tags[k] for k in tags.keys() & tag_keys}

    def parse_attributes(self, annotation: str):
        attributes = []

        for name, values in self.__attributes__(annotation).items():
            tags = self.__tags__(name, values[1: len(values) - 1])
            # print(name, tags)
            attributes.append(Attribute(name, tags))

        return attributes


class Namespace:
    def __init__(self, namespace: str):
        self._namespace = namespace
        self._namespaces = namespace.split('::')

    def identify_name(self):
        return self._namespace


class MetaInfo:
    def __init__(self, name, namespace: Namespace):
        self._name = name
        self._namespace = namespace
        self._attributes = []

    def __attributes__(self, node: CX.Cursor, attr_parser: AttributeParser):
        children = list(node.get_children())

        for child in children:
            if child.kind == CX.CursorKind.ANNOTATE_ATTR:
                self._attributes = attr_parser.parse_attributes(child.spelling)

    def attributes(self):
        return self._attributes

    def name(self):
        return self._name

    def namespace(self):
        return self._namespace


class Filed(MetaInfo):
    def __init__(self, node: CX.Cursor, parent_class, attr_parser: AttributeParser):
        MetaInfo.__init__(self, node.spelling, parent_class.namespace())
        self.__attributes__(node, attr_parser)
        self._parent_class = parent_class

    def parent_class(self):
        return self._parent_class


class Method(MetaInfo):
    def __init__(self, node: CX.Cursor, parent_class, attr_parser: AttributeParser):
        MetaInfo.__init__(self, node.spelling, parent_class.namespace())
        self.__attributes__(node, attr_parser)
        self._parent_class = parent_class

    def parent_class(self):
        return self._parent_class


class Class(MetaInfo):
    def __init__(self, node: CX.Cursor, namespace: Namespace, attr_parser: AttributeParser):
        MetaInfo.__init__(self, node.spelling, namespace)
        self._methods = []
        self._fileds = []
        self._base_classes = []
        self.__parse__(node, attr_parser)

    def identify_name(self):
        return self.namespace().identify_name() + str('::') + self.name()

    def methods(self):
        return self._methods

    def fileds(self):
        return self._fileds

    def base_classes(self):
        return self._base_classes

    def __parse__(self, node: CX.Cursor, attr_parser: AttributeParser):
        children = list(node.get_children())

        for child in children:
            if child.kind == CX.CursorKind.CXX_METHOD:
                self._methods.append(Method(child, self, attr_parser))

            elif child.kind == CX.CursorKind.FIELD_DECL:
                self._fileds.append(Filed(child, self, attr_parser))

            elif child.kind == CX.CursorKind.CXX_BASE_SPECIFIER:
                self._base_classes.append(self.__base_class__(child))

            elif child.kind == CX.CursorKind.ANNOTATE_ATTR:
                self._attributes = attr_parser.parse_attributes(child.spelling)

    def __base_class__(self, node: CX.Cursor):
        namespace = self._namespace
        base_class_name = node.spelling

        children = list(node.get_children())
        for child in children:
            if child.kind == CX.CursorKind.NAMESPACE_REF:
                namespace = Namespace(child.spelling)

        return MetaInfo(base_class_name, namespace)
