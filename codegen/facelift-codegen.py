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

global generateAsyncProxy
generateAsyncProxy = False

def generateAsync():
    global generateAsyncProxy
    return generateAsyncProxy

def interfaceNameSuffix():
    if generateAsync():
        return "Async"
    else:
        return ""

def parameterType(symbol):
    return symbol

def toValidId(name):
    return name.replace('.', '_')

def fullyQualifiedName(symbol):
    if symbol.type.is_interface:
        return symbol.qualified_name + interfaceNameSuffix()
    return symbol.qualified_name

def getPrimitiveCppType(symbol):
    if symbol.name == 'string':
        return 'QString'
    if symbol.name == 'real':
        return 'float'
    return symbol;

def qmlCompatibleType(self):
    if self.is_interface:
        return "QObject*"
    if self.is_enum:
        return cppTypeFromSymbol(self.type, True) + "Gadget::Type"
    if self.is_list:
        return "QVariantList"
    if self.is_map:
        return "QVariantMap"
    return cppTypeFromSymbol(self.type, True)

def fullyQualifiedCppName(type):
    if type.is_primitive:
        return getPrimitiveCppType(type)
    else:
        s = '{0}'.format(fullyQualifiedName(type)).replace(".", "::")
        return s

def namespaceCppOpen(symbol):
    parts = symbol.qualified_name.split('.')
    ns = ' '.join(['namespace %s {' % x for x in parts])
    return ns

def namespaceCppClose(symbol):
    parts = symbol.qualified_name.split('.')
    ns = '} ' * len(parts)
    return ns

def classExportDefines(libraryName):
    defines = "#if defined(" + libraryName + "_LIBRARY)\n"
    defines += "#  define " + libraryName + "_EXPORT Q_DECL_EXPORT\n"
    defines += "#else\n"
    defines += "#  define " + libraryName + "_EXPORT Q_DECL_IMPORT\n"
    defines += "#endif"
    return defines

def classExport(libraryName):
    return libraryName + "_EXPORT"

def cppTypeFromSymbol(type, isInterfaceType):
    if type.is_void or type.is_primitive:
        if type.name == 'string':
            return 'QString'
        if type.name == 'real':
            return 'float'
        return type
    elif type.is_list:
        return 'QList<{0}>'.format(cppTypeFromSymbol(type.nested, isInterfaceType))
    elif type.is_interface:
        return fullyQualifiedCppName(type) + ("*" if isInterfaceType else "")
    elif type.is_map:
        return 'QMap<QString, {0}>'.format(cppTypeFromSymbol(type.nested, isInterfaceType))
    else:
        return fullyQualifiedCppName(type)

def cppBool(b):
    if b:
        return "true"
    else:
        return "false"

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

def insertUniqueType(symbol, list):
    type = symbol.type.nested if symbol.type.nested else symbol.type
    for t in list:
        if (t.name == type.name):
            return
    list.add(type)

def referencedTypes(self):
    types = set()
    for property in self.properties:
        insertUniqueType(property, types)
    for operation in self.operations:
        for param in operation.parameters:
            insertUniqueType(param, types)
        if operation.hasReturnValue:
            insertUniqueType(operation.type, types)
    for m in self.signals:
        for param in m.parameters:
            insertUniqueType(param, types)
    return types

def appendTypeIfInterface(symbol, list):
    type = symbol.type.nested if symbol.type.nested else symbol.type
    if type.is_interface:
        list.append(type)

def referencedInterfaceTypes(self):
    interfaces = []
    for property in self.properties:
        appendTypeIfInterface(property, interfaces)
    for m in self.operations:
        for param in m.parameters:
            appendTypeIfInterface(param, interfaces)
    for m in self.signals:
        for param in m.parameters:
            appendTypeIfInterface(param, interfaces)
    return interfaces

def hasQMLIncompatibleParameter(parameters):
    for param in parameters:
        if param.type.is_list or param.type.is_map or param.type.is_interface:
            return True
    return False

def hasPropertyWithReadyFlag(interface):
    for property in interface.properties:
        if property.tags.get('hasReadyFlag'):
            return True
    return False

def hasModelProperty(interface):
    for property in interface.properties:
        if property.type.is_model:
            return True
    return False

def fullyQualifiedCppType(type):
    s = '{0}'.format(fullyQualifiedName(type)).replace(".", "::")
    return s

def isAsync(self):
    if self.tags.get('async'):
        return True
    return generateAsync()

######### Property extensions

def fullyQualifiedPath(self):
    return self.qualified_name.replace('.', '/')

def storageType(self):
    return cppTypeFromSymbol(self.type, False)

def cppType(self):
    return cppTypeFromSymbol(self.type, False)

def interfaceCppType(self):
    return cppTypeFromSymbol(self.type, True)

def nestedType(self):
    return self.type.nested

def requiredInclude(self):
    if self.is_primitive:
        return ""
    else:
        type = self.nested if self.nested else self
        return requiredIncludeFromType(type, ".h")

setattr(qface.idl.domain.Operation, 'storageType', property(storageType))

setattr(qface.idl.domain.TypeSymbol, 'cppType', property(cppType))
setattr(qface.idl.domain.Operation, 'cppType', property(cppType))
setattr(qface.idl.domain.Property, 'cppType', property(cppType))
setattr(qface.idl.domain.Parameter, 'cppType', property(cppType))
setattr(qface.idl.domain.Field, 'cppType', property(cppType))

setattr(qface.idl.domain.TypeSymbol, 'interfaceCppType', property(interfaceCppType))
setattr(qface.idl.domain.Operation, 'interfaceCppType', property(interfaceCppType))
setattr(qface.idl.domain.Property, 'interfaceCppType', property(interfaceCppType))
setattr(qface.idl.domain.Parameter, 'interfaceCppType', property(interfaceCppType))
setattr(qface.idl.domain.Field, 'interfaceCppType', property(interfaceCppType))

setattr(qface.idl.domain.Property, 'nestedType', property(nestedType))

setattr(qface.idl.domain.TypeSymbol, 'requiredInclude', property(requiredInclude))
setattr(qface.idl.domain.TypeSymbol, 'qmlCompatibleType', property(qmlCompatibleType))

setattr(qface.idl.domain.TypeSymbol, 'fullyQualifiedPath', property(fullyQualifiedPath))
setattr(qface.idl.domain.Interface, 'fullyQualifiedPath', property(fullyQualifiedPath))
setattr(qface.idl.domain.Module, 'fullyQualifiedPath', property(fullyQualifiedPath))
setattr(qface.idl.domain.Struct, 'fullyQualifiedPath', property(fullyQualifiedPath))
setattr(qface.idl.domain.Enum, 'fullyQualifiedPath', property(fullyQualifiedPath))

setattr(qface.idl.domain.Enum, 'fullyQualifiedCppType', property(fullyQualifiedCppType))
setattr(qface.idl.domain.TypeSymbol, 'fullyQualifiedCppType', property(fullyQualifiedCppType))
setattr(qface.idl.domain.Struct, 'fullyQualifiedCppType', property(fullyQualifiedCppType))
setattr(qface.idl.domain.Module, 'fullyQualifiedCppType', property(fullyQualifiedCppType))
setattr(qface.idl.domain.Interface, 'fullyQualifiedCppType', property(fullyQualifiedCppType))

setattr(qface.idl.domain.Interface, 'referencedInterfaceTypes', property(referencedInterfaceTypes))
setattr(qface.idl.domain.Interface, 'referencedTypes', property(referencedTypes))

setattr(qface.idl.domain.Interface, 'hasPropertyWithReadyFlag', property(hasPropertyWithReadyFlag))
setattr(qface.idl.domain.Interface, 'hasModelProperty', property(hasModelProperty))

setattr(qface.idl.domain.Module, 'namespaceCppOpen', property(namespaceCppOpen))
setattr(qface.idl.domain.Module, 'namespaceCppClose', property(namespaceCppClose))

setattr(qface.idl.domain.Operation, 'isAsync', property(isAsync))

def hasReturnValue(self):
    return not self.type.name == 'void'

setattr(qface.idl.domain.Operation, 'hasReturnValue', property(hasReturnValue))

##############################

def generateFile(generator, outputPath, templatePath, context, libraryName, libraryType):
    if libraryName:
        context.update({'classExportDefines': classExportDefines(libraryName + "_" + libraryType)})
        context.update({'classExport': classExport(libraryName + "_" + libraryType)})
    else:
        context.update({'classExportDefines': ""})
        context.update({'classExport': ""})
    generator.write(outputPath, templatePath, context)

def run_generation(input, output, dependency, libraryName):
    global generateAsyncProxy
    FileSystem.strict = True
    Generator.strict = True

    # Build the list of modules to be generated
    modulesToGenerate = []
    for module in FileSystem.parse(list(input)).modules:
        modulesToGenerate.append(module.name)

    system = FileSystem.parse(list(input) + list(dependency))
    generator = Generator(search_path=Path(here / 'facelift/templates'))
    generator.register_filter('cppBool', cppBool)
    generator.register_filter('toValidId', toValidId)
    generator.register_filter('hasQMLIncompatibleParameter', hasQMLIncompatibleParameter)
    generator.destination = output

    ctx = {'output': output}
    for module in system.modules:
        if module.name in modulesToGenerate:
            ctx.update({'module': module})
            module_path = '/'.join(module.name_parts)
            log.debug('process module %s' % module.module_name)
            ctx.update({'path': module_path})
            generateFile(generator, 'module/{{path}}/ModulePrivate.h', 'ModulePrivate.template.h', ctx, libraryName, "types")
            generateFile(generator, 'module/{{path}}/Module.h', 'Module.template.h', ctx, libraryName, "types")
            generateFile(generator, 'module/{{path}}/Module.cpp', 'Module.template.cpp', ctx, libraryName, "types")
            generateFile(generator, 'ipc/{{path}}/ModuleIPC.h', 'ModuleIPC.template.h', ctx, libraryName, "ipc")
            generateFile(generator, 'ipc/{{path}}/ModuleIPC.cpp', 'ModuleIPC.template.cpp', ctx, libraryName, "ipc")
            generateFile(generator, 'devtools/{{path}}/ModuleMonitor.h', 'ModuleMonitor.template.h', ctx, libraryName, "desktop_dev_tools")
            generateFile(generator, 'devtools/{{path}}/ModuleMonitor.cpp', 'ModuleMonitor.template.cpp', ctx, libraryName, "desktop_dev_tools")
            generateFile(generator, 'devtools/{{path}}/ModuleDummy.h', 'DummyModule.template.h', ctx, libraryName, "desktop_dev_tools")
            for interface in module.interfaces:
                log.debug('process interface %s' % interface)
                ctx.update({'interface': interface})
                ctx.update({'interfaceName': interface.name})
                generateAsyncProxy = False
                ctx.update({'generateAsyncProxy': generateAsyncProxy})
                generateFile(generator, 'types/{{path}}/{{interface}}.h', 'Service.template.h', ctx, libraryName, "types")
                generateFile(generator, 'types/{{path}}/{{interface}}.cpp', 'Service.template.cpp', ctx, libraryName, "types")
                generateFile(generator, 'types/{{path}}/{{interface}}PropertyAdapter.h', 'ServicePropertyAdapter.template.h', ctx, libraryName, "types")
                generateFile(generator, 'types/{{path}}/{{interface}}PropertyAdapter.cpp', 'ServicePropertyAdapter.template.cpp', ctx, libraryName, "types")
                generateFile(generator, 'types/{{path}}/{{interface}}QMLImplementation.h', 'QMLImplementation.template.h', ctx, libraryName, "types")
                generateFile(generator, 'types/{{path}}/{{interface}}QMLFrontend.h', 'QMLFrontend.template.h', ctx, libraryName, "types")
                generateFile(generator, 'types/{{path}}/{{interface}}QMLFrontend.cpp', 'QMLFrontend.template.cpp', ctx, libraryName, "types")
                generateFile(generator, 'devtools/{{path}}/{{interface}}Dummy.h', 'DummyService.template.h', ctx, libraryName, "desktop_dev_tools")
                generateFile(generator, 'devtools/{{path}}/{{interface}}Monitor.h', 'ServiceMonitor.template.h', ctx, libraryName, "desktop_dev_tools")
                generateFile(generator, 'ipc/{{path}}/{{interface}}IPCAdapter.h', 'IPCAdapter.template.h', ctx, libraryName, "ipc")
                generateFile(generator, 'ipc/{{path}}/{{interface}}IPCAdapter.cpp', 'IPCAdapter.template.cpp', ctx, libraryName, "ipc")
                generateFile(generator, 'ipc/{{path}}/{{interface}}IPCProxy.h', 'IPCProxy.template.h', ctx, libraryName, "ipc")
                generateFile(generator, 'ipc/{{path}}/{{interface}}IPCProxy.cpp', 'IPCProxy.template.cpp', ctx, libraryName, "ipc")
                generateFile(generator, 'ipc/{{path}}/{{interface}}IPC.h', 'IPC.template.h', ctx, libraryName, "ipc")
                generateAsyncProxy = True
                ctx.update({'generateAsyncProxy': generateAsyncProxy})
                ctx.update({'interfaceName': interface.name + interfaceNameSuffix()})
                generateFile(generator, 'types/{{path}}/{{interface}}Async.h', 'Service.template.h', ctx, libraryName, "types")
                generateFile(generator, 'types/{{path}}/{{interface}}Async.cpp', 'Service.template.cpp', ctx, libraryName, "types")
                generateFile(generator, 'types/{{path}}/{{interface}}AsyncQMLFrontend.h', 'QMLFrontend.template.h', ctx, libraryName, "types")
                generateFile(generator, 'types/{{path}}/{{interface}}AsyncQMLFrontend.cpp', 'QMLFrontend.template.cpp', ctx, libraryName, "types")
                generateFile(generator, 'ipc/{{path}}/{{interface}}AsyncIPCAdapter.h', 'IPCAdapter.template.h', ctx, libraryName, "ipc")
                generateFile(generator, 'ipc/{{path}}/{{interface}}AsyncIPCAdapter.cpp', 'IPCAdapter.template.cpp', ctx, libraryName, "ipc")
                generateFile(generator, 'ipc/{{path}}/{{interface}}AsyncIPCProxy.h', 'IPCProxy.template.h', ctx, libraryName, "ipc")
                generateFile(generator, 'ipc/{{path}}/{{interface}}AsyncIPCProxy.cpp', 'IPCProxy.template.cpp', ctx, libraryName, "ipc")
            for enum in module.enums:
                ctx.update({'enum': enum})
                generateFile(generator, 'types/{{path}}/{{enum}}.h', 'Enum.template.h', ctx, libraryName, "types")
                generateFile(generator, 'types/{{path}}/{{enum}}.cpp', 'Enum.template.cpp', ctx, libraryName, "types")
            for struct in module.structs:
                ctx.update({'struct': struct})
                generateFile(generator, 'types/{{path}}/{{struct}}.h', 'Struct.template.h', ctx, libraryName, "types")
                generateFile(generator, 'types/{{path}}/{{struct}}.cpp', 'Struct.template.cpp', ctx, libraryName, "types")


@click.command()
@click.option('--library', default="")
@click.option('--output', default=".")
@click.option('--input', '-i', multiple=True)
@click.option('--dependency', '-d', multiple=True)
def generate(input, output, dependency, library):
    """Takes several files or directories as input and generates the code
    in the given output directory."""
    run_generation(input, output, dependency, library)


if __name__ == '__main__':
    generate()
