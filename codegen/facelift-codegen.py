#!/usr/bin/env python3

# This is part of the FaceLift project
# Copyright (C) 2017 Pelagicore AB
# SPDX-License-Identifier: LGPL-2.1
# This file is subject to the terms of the LGPL 2.1 license.
# Please see the LICENSE file for details. 

import click
from qface.generator import FileSystem, Generator
from path import Path
import logging
import logging.config
import yaml
import qface
import pdb

here = Path(__file__).dirname()

logging.config.dictConfig(yaml.load(open(here / 'facelift/facelift-log.yaml')))
log = logging.getLogger(__name__)


def parameterType(symbol):
    return symbol

def fullyQualifiedPath(symbol):
    return symbol.qualified_name.replace('.', '/')

def toValidId(name):
    return name.replace('.', '_')

def fullyQualifiedName(symbol):
    return symbol.qualified_name

def getPrimitiveCppType(symbol):
    if symbol.name == 'string':
        return 'QString'
    if symbol.name == 'real':
        return 'float'
    return symbol;

def qmlCompatibleType(symbol):
    if symbol.type.is_interface:
        return "QObject*"
    if symbol.type.is_enum:
        return returnType(symbol.type) + "Gadget::Type"
    if symbol.type.is_list:
        return "QVariantList"
    return returnType(symbol.type)

def fullyQualifiedCppName(type):
    try:
        if type.is_primitive:
            return getPrimitiveCppType(type)
    except AttributeError:
        pass
    s = '{0}'.format(fullyQualifiedName(type)).replace(".", "::")
    return s

def namespaceOpen(symbol):
    parts = symbol.qualified_name.split('.')
    ns = ' '.join(['namespace %s {' % x for x in parts])
    return ns

def namespaceClose(symbol):
    parts = symbol.qualified_name.split('.')
    ns = '} ' * len(parts)
    return ns

def returnTypeFromSymbol(type):
    if type.is_void or type.is_primitive:
        if type.name == 'string':
            return 'QString'
        if type.name == 'real':
            return 'float'
        return type
    elif type.is_list:
        return 'QList<{0}>'.format(returnTypeFromSymbol(type.nested))
    elif type.is_interface:
        return '{0}*'.format(fullyQualifiedCppName(type))
    else:
        return fullyQualifiedCppName(type)

def returnType(symbol):
    return returnTypeFromSymbol(symbol.type)

def cppBool(b):
    if b:
        return "true"
    else:
        return "false"

def nestedType(symbol):
    return symbol.type.nested

def requiredIncludeFromType(symbol, suffix):
    typeName = ''
    if not symbol.is_primitive:
        symbol = symbol.nested if symbol.nested else symbol
        typeName = fullyQualifiedCppName(symbol)
        if typeName.startswith("::"):
            typeName = typeName[2:]
        return '#include "' + typeName.replace('::', '/') + suffix + '"'
    else:
        return ""

def requiredInclude(symbol):
    if not symbol.type.is_primitive:
        type = symbol.type.nested if symbol.type.nested else symbol.type
        return requiredIncludeFromType(type, ".h")
    return ""

def requiredQMLInclude(symbol):
    if symbol.type.is_interface:
        type = symbol.type.nested if symbol.type.nested else symbol.type
        return requiredIncludeFromType(type, "QMLFrontend.h")
    else:
        return ""

def hasListParameter(parameters):
    for param in parameters:
        if param.type.is_list:
            return True
    return False

def hasReturnValue(self):
    return not self.type.name == 'void'

setattr(qface.idl.domain.Operation, 'hasReturnValue', property(hasReturnValue))

def run_generation(input, output, dependency):
    FileSystem.strict = True
    Generator.strict = True

    # Build the list of modules to be generated
    modulesToGenerate = []
    for module in FileSystem.parse(list(input)).modules:
        modulesToGenerate.append(module.name)

    system = FileSystem.parse(list(input) + list(dependency))
    generator = Generator(search_path=Path(here / 'facelift/templates'))
    generator.register_filter('returnType', returnType)
    generator.register_filter('cppBool', cppBool)
    generator.register_filter('parameterType', parameterType)
    generator.register_filter('nestedType', nestedType)
    generator.register_filter('requiredInclude', requiredInclude)
    generator.register_filter('requiredQMLInclude', requiredQMLInclude)
    generator.register_filter('namespaceOpen', namespaceOpen)
    generator.register_filter('namespaceClose', namespaceClose)
    generator.register_filter('fullyQualifiedName', fullyQualifiedName)
    generator.register_filter('fullyQualifiedCppName', fullyQualifiedCppName)
    generator.register_filter('fullyQualifiedPath', fullyQualifiedPath)
    generator.register_filter('toValidId', toValidId)
    generator.register_filter('hasListParameter', hasListParameter)
    generator.register_filter('qmlCompatibleType', qmlCompatibleType)
    generator.destination = output

    ctx = {'output': output}
    for module in system.modules:
        if module.name in modulesToGenerate:
            ctx.update({'module': module})
            module_path = '/'.join(module.name_parts)
            log.debug('process module %s' % module.module_name)
            ctx.update({'path': module_path})
            generator.write('module/{{path}}/Module.h', 'Module.template.h', ctx)
            generator.write('module/{{path}}/Module.cpp', 'Module.template.cpp', ctx)
            generator.write('ipc/{{path}}/ModuleIPC.h', 'ModuleIPC.template.h', ctx)
            generator.write('devtools/{{path}}/ModuleMonitor.h', 'ModuleMonitor.template.h', ctx)
            generator.write('devtools/{{path}}/ModuleMonitor.cpp', 'ModuleMonitor.template.cpp', ctx)
            generator.write('devtools/{{path}}/ModuleDummy.h', 'DummyModule.template.h', ctx)
            for interface in module.interfaces:
                log.debug('process interface %s' % interface)
                ctx.update({'interface': interface})
                generator.write('types/{{path}}/{{interface}}.h', 'Service.template.h', ctx)
                generator.write('types/{{path}}/{{interface}}.cpp', 'Service.template.cpp', ctx)
                generator.write('types/{{path}}/{{interface}}PropertyAdapter.h', 'ServicePropertyAdapter.template.h', ctx)
                generator.write('types/{{path}}/{{interface}}QMLImplementation.h', 'QMLImplementation.template.h', ctx)
                generator.write('types/{{path}}/{{interface}}QMLFrontend.h', 'QMLFrontend.template.h', ctx)
                generator.write('devtools/{{path}}/{{interface}}Dummy.h', 'DummyService.template.h', ctx)
                generator.write('devtools/{{path}}/{{interface}}Monitor.h', 'ServiceMonitor.template.h', ctx)
                generator.write('ipc/{{path}}/{{interface}}IPC.h', 'ServiceIPC.template.h', ctx)
            for enum in module.enums:
                ctx.update({'enum': enum})
                generator.write('types/{{path}}/{{enum}}.h', 'Enum.template.h', ctx)
                generator.write('types/{{path}}/{{enum}}.cpp', 'Enum.template.cpp', ctx)
            for struct in module.structs:
                ctx.update({'struct': struct})
                generator.write('types/{{path}}/{{struct}}.h', 'Struct.template.h', ctx)
                generator.write('types/{{path}}/{{struct}}.cpp', 'Struct.template.cpp', ctx)


@click.command()
@click.option('--output', default=".")
@click.option('--input', '-i', multiple=True)
@click.option('--dependency', '-d', multiple=True)
def generate(input, output, dependency):
    """Takes several files or directories as input and generates the code
    in the given output directory."""
    run_generation(input, output, dependency)


if __name__ == '__main__':
    generate()
