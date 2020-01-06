#!/usr/bin/env python3

#######################################################################
##
## Copyright (C) 2018 Luxoft Sweden AB
##
## This file is part of the FaceLift project
##
## Permission is hereby granted, free of charge, to any person
## obtaining a copy of this software and associated documentation files
## (the "Software"), to deal in the Software without restriction,
## including without limitation the rights to use, copy, modify, merge,
## publish, distribute, sublicense, and/or sell copies of the Software,
## and to permit persons to whom the Software is furnished to do so,
## subject to the following conditions:
##
## The above copyright notice and this permission notice shall be
## included in all copies or substantial portions of the Software.
##
## THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
## EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
## MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
## NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
## BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
## ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
## CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
## SOFTWARE.
##
## SPDX-License-Identifier: MIT
##
#######################################################################

from qface.generator import FileSystem, Generator
from path import Path
import logging.config
import yaml
import qface
import os

here = Path(__file__).dirname()

logging.config.dictConfig(yaml.load(open(here / 'facelift-log.yaml')))
log = logging.getLogger(__name__)

def collect_qface_files(path):
    """Collect all *.qface files inside a given path
    Returns a list of absolute qface file paths"""
    qface_file_list = list()
    if path is not None and path != "":
        for file in os.listdir(path):
            if file.endswith(".qface"):
                qface_file_list.append(os.path.join(path, file))
    return qface_file_list

def is_keyword_in_file(keyword, file_path):
    """Check if keyword is in a file"""
    try:
        with open(file_path, "rb") as f:
            for line in f:
                try:
                    line = line.decode("utf-8")
                except ValueError:
                    continue
                if keyword in line:
                    return True
    except (IOError, OSError):
        return False
    return False

def run_documentation_generation(input, output, dependency):
    FileSystem.strict = True
    Generator.strict = True

    # Build the list of modules to be generated
    system = FileSystem.parse(list(input) + list(dependency))

    qface_input_list = list()
    for input_dir in list(input):
        qface_input_list.extend(collect_qface_files(input_dir))
    qface_input_list = sorted(qface_input_list)

    qface_dependency_list = list()
    for dependency_dir in list(dependency):
        qface_dependency_list.extend(collect_qface_files(dependency_dir))
    qface_dependency_list = sorted(qface_dependency_list)

    generator = Generator(search_path=Path(here / 'templates'))
    generator.destination = output

    def get_basename(path):
        ret = ""
        if path is not None and path != "":
            ret = os.path.basename(path)
        return ret

    generator.register_filter('get_basename', get_basename)

    def get_qface_document(type_symbol):
        """Attempt to find the QFace document for a given type symbol"""
        module_name = ".".join(type_symbol.qualified_name.split(".")[:-1])
        for qface_input in qface_input_list:
            if is_keyword_in_file("module %s" % module_name, qface_input):
                return qface_input
        for qface_dependency in qface_dependency_list:
            if is_keyword_in_file("module %s" % module_name, qface_dependency):
                return qface_dependency
        return ""

    generator.register_filter('get_qface_document', get_qface_document)

    def get_file_contents(file):
        if file is not None and os.path.exists(file):
            with open(file, "r") as f:
                return f.read()
        return ""

    generator.register_filter('get_file_contents', get_file_contents)

    ctx = { 'output': output}
    ctx.update( { 'input' : input })
    ctx.update({ 'system': system })
    ctx.update({ 'qface_input_list': qface_input_list })
    ctx.update({ 'qface_dependency_list': qface_dependency_list })

    sorted_system_modules = sorted(system.modules, key=lambda x: x.qualified_name)
    ctx.update({ 'sorted_system_modules': sorted_system_modules })

    generator.write('documentation/index.html', 'documentation/Index.template.html', ctx)

    for module in system.modules:
        try:
            ctx.update({'module': module})
            module_path = '.'.join(module.name_parts)
            ctx.update({'module_path': module_path})
            log.debug('process module documentation %s' % module.module_name)
            ctx.update({'path': module_path})

            # Find QFace file used define this module
            qface_file_references = [ get_qface_document(module) ]
            ctx.update({ 'qface_file_references': qface_file_references })

            generator.write('documentation/{{path}}.Module.html', 'documentation/Module.template.html', ctx)
            for interface in module.interfaces:
                log.debug('process interface documentation %s' % interface)
                ctx.update({'interface': interface})
                ctx.update({'interfaceName': interface.name})
                generator.write('documentation/{{path}}.{{interfaceName}}.Interface.html', 'documentation/Interface.template.html', ctx)

            for enum in module.enums:
                ctx.update({'enum': enum})
                generator.write('documentation/{{path}}.{{enum}}.Enum.html', 'documentation/Enum.template.html', ctx)

            for struct in module.structs:
                ctx.update({'struct': struct})
                generator.write('documentation/{{path}}.{{struct}}.Struct.html', 'documentation/Struct.template.html', ctx)
        except:
            log.warning("failed to generate documentation for module %s" % module.name)

if __name__ == '__main__':
    pass