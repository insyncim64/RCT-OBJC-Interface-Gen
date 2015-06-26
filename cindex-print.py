"""
A simple command line tool for dumping a source file using the Clang Index
Library.
"""
import sys
import os
import itertools
import clang.cindex
#from mako.template import Template

def get_annotations(node):
    return [c.displayname for c in node.get_children()
            if c.kind == clang.cindex.CursorKind.ANNOTATE_ATTR]

class Param(object):
    def __init__(self, cursor):
        self.type_name = cursor.type.spelling
        self.name = cursor.spelling
        print('{} : {}'.format(self.type_name , self.name))

class Function(object):
    def __init__(self, cursor):
        self.name = cursor.displayname
        self.spelling = cursor.spelling
        self.annotations = get_annotations(cursor)
        self.access = cursor.access_specifier
        self.params = []
        print(self.name)
        print(cursor.result_type.kind)
        for c in cursor.get_children():
            print(c.kind)
            p = Param(c)
            self.params.append(p)


class Class(object):
    def __init__(self, cursor):
        self.name = cursor.spelling
        self.functions = []
        self.annotations = get_annotations(cursor)
        
        for c in cursor.get_children():
            if (c.kind == clang.cindex.CursorKind.OBJC_INSTANCE_METHOD_DECL and not c.spelling.startswith('init')):
                f = Function(c)
                self.functions.append(f)

def build_classes(cursor):
    result = []
    for c in cursor.get_children():
        if (c.kind == clang.cindex.CursorKind.OBJC_INTERFACE_DECL
            and c.location.file.name == sys.argv[1]):
            a_class = Class(c)
            result.append(a_class)
        elif c.kind == clang.cindex.CursorKind.NAMESPACE:
            child_classes = build_classes(c)
            result.extend(child_classes)
    return result

def main():
    from clang.cindex import Index
    from pprint import pprint

    from optparse import OptionParser, OptionGroup

    global opts

# Index.Config.set_library_file('/usr/local/lib/libclang.so')
    index = Index.create()
    translation_unit = index.parse(sys.argv[1], ['-x', 'objective-c', '-D__CODE_GENERATOR__'])

    classes = build_classes(translation_unit.cursor)
    #tpl = Template(filename='bind.mako')
    #rendered = tpl.render(
    #                      classes=classes,
    #                      module_name='CodegenExample',
    #                      include_file=sys.argv[1])

#OUTPUT_DIR = 'generated'

#   if not os.path.isdir(OUTPUT_DIR): os.mkdir(OUTPUT_DIR)

#   with open("generated/{}.bind.cc".format(sys.argv[1]), "w") as f:
#       f.write(rendered)


if __name__ == '__main__':
    main()

