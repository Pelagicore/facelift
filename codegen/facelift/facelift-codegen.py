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
import logging.config
import yaml
import qface

here = Path(__file__).dirname()

logging.config.dictConfig(yaml.safe_load(open(here / 'facelift-log.yaml')))
log = logging.getLogger(__name__)

generateAsyncProxy = False
generateAll = False

def generateAsync():
    global generateAsyncProxy
    return generateAsyncProxy

def interfaceNameSuffix():
    if generateAsync():
        return "Async"
    else:
        return ""

def fullyQualifiedName(symbol):
    if symbol.type.is_interface:
        return symbol.qualified_name + interfaceNameSuffix()
    return symbol.qualified_name

def getPrimitiveCppType(symbol):
    if symbol.name == 'string':
        return 'QString'
    if symbol.name == 'real':
        return 'double'
    return symbol

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
    return getPrimitiveCppType(type) if type.is_primitive else fullyQualifiedCppType(type)

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
            return 'double'
        return type
    elif type.is_list:
        return 'QList<{0}>'.format(cppTypeFromSymbol(type.nested, isInterfaceType))
    elif type.is_interface:
        return fullyQualifiedCppName(type) + ("*" if isInterfaceType else "")
    elif type.is_map:
        return 'QMap<QString, {0}>'.format(cppTypeFromSymbol(type.nested, isInterfaceType))
    else:
        return fullyQualifiedCppName(type)


def requiredIncludeFromType(symbol, suffix):
    if not symbol.is_primitive:
        symbol = symbol.nested if symbol.nested else symbol
        typeName = fullyQualifiedCppName(symbol)
        if typeName.startswith("::"):
            typeName = typeName[2:]
        return '#include "' + typeName.replace('::', '/') + suffix + '"'
    else:
        return ""

def insertUniqueType(symbol, unique_types):
    type = symbol.type.nested if symbol.type.nested else symbol.type
    if type.name not in (t.name for t in unique_types):
        unique_types.append(type)

def referencedTypes(self):
    types = []
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

def appendTypeIfStructureAndUnique(symbol, unique_list):
    type = symbol.type.nested if symbol.type.nested else symbol.type
    if type.is_struct and type.name not in (t.name for t in unique_list):
        unique_list.append(type)

def referencedStructureTypes(self):
    interfaces = []
    for property in self.properties:
        appendTypeIfStructureAndUnique(property, interfaces)
    for m in self.operations:
        for param in m.parameters:
            appendTypeIfStructureAndUnique(param, interfaces)
        if m.hasReturnValue:
            appendTypeIfStructureAndUnique(m.type, interfaces)
    for m in self.signals:
        for param in m.parameters:
            appendTypeIfStructureAndUnique(param, interfaces)
    return interfaces

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

def hasPropertyWithReadyFlag(interface):
    return any(prop.tags.get('hasReadyFlag') for prop in interface.properties)

def hasModelProperty(interface):
    return any(prop.type.is_model for prop in interface.properties)

def fullyQualifiedCppType(type):
    s = '{0}'.format(fullyQualifiedName(type)).replace(".", "::")
    return s

def isAsync(self):
    return True if self.tags.get('async') else generateAsync()

def isIPCEnabled(self):
    return self.isSynchronousIPCEnabled or self.isAsynchronousIPCEnabled

def isSynchronousIPCEnabled(self):
    return True if self.tags.get('ipc-sync') else generateAll

def isAsynchronousIPCEnabled(self):
    return True if self.tags.get('ipc-async') else generateAll

def isQMLImplementationEnabled(self):
    return True if self.tags.get('qml-implementation') else generateAll

def isSerializable(self):
    return True if self.tags.get('serializable') else generateAll

def toByteArrayOverDBus(self):
    return True if self.tags.get('toByteArrayOverDBus') else generateAll

def isQObjectWrapperEnabled(self):
    return True if self.tags.get('qml-component') else False

def isQObjectWrapperDeprecated(self):
    return False if self.tags.get('qml-component') else True

def verifyStruct(self):
    blackList = [ 'userData', 'UserData', 'serialize', 'deserialize', 'clone', 'toString' ]
    for field in self.fields:
        if field.name in blackList:
            raise Exception("Field name '{}' in struct '{}' is not allowed, "
                            "since it is used by Facelift already.".format(field.name, self.name))
    return True

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

def cppMethodArgumentType(self):
    if self.type.is_model:
        print("We don't support models as function arguments!")
        exit()

    if self.type.is_list or self.type.is_map or self.type.is_struct or (self.type.name == "string"):
       return "const " + cppType(self.type) + "&"
    else:
        return cppType(self.type)


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
setattr(qface.idl.domain.Interface, 'referencedStructureTypes', property(referencedStructureTypes))
setattr(qface.idl.domain.Interface, 'referencedTypes', property(referencedTypes))

setattr(qface.idl.domain.Interface, 'hasPropertyWithReadyFlag', property(hasPropertyWithReadyFlag))
setattr(qface.idl.domain.Interface, 'hasModelProperty', property(hasModelProperty))
setattr(qface.idl.domain.Interface, 'isAsynchronousIPCEnabled', property(isAsynchronousIPCEnabled))
setattr(qface.idl.domain.Interface, 'isSynchronousIPCEnabled', property(isSynchronousIPCEnabled))
setattr(qface.idl.domain.Interface, 'isIPCEnabled', property(isIPCEnabled))
setattr(qface.idl.domain.Interface, 'isQMLImplementationEnabled', property(isQMLImplementationEnabled))

setattr(qface.idl.domain.Module, 'namespaceCppOpen', property(namespaceCppOpen))
setattr(qface.idl.domain.Module, 'namespaceCppClose', property(namespaceCppClose))

setattr(qface.idl.domain.Operation, 'isAsync', property(isAsync))

setattr(qface.idl.domain.Struct, 'verifyStruct', property(verifyStruct))
setattr(qface.idl.domain.Struct, 'isSerializable', property(isSerializable))
setattr(qface.idl.domain.Struct, 'toByteArrayOverDBus', property(toByteArrayOverDBus))
setattr(qface.idl.domain.Struct, 'isQObjectWrapperEnabled', property(isQObjectWrapperEnabled))
setattr(qface.idl.domain.Struct, 'isQObjectWrapperDeprecated', property(isQObjectWrapperDeprecated))

def hasReturnValue(self):
    return not self.type.name == 'void'

setattr(qface.idl.domain.Operation, 'hasReturnValue', property(hasReturnValue))
setattr(qface.idl.domain.Parameter, 'cppMethodArgumentType', property(cppMethodArgumentType))
setattr(qface.idl.domain.Property, 'cppMethodArgumentType', property(cppMethodArgumentType))

##############################

def generateFile(generator, outputPath, templatePath, context, libraryName, libraryType):
    if libraryName:
        name = libraryName
        if libraryType:
            name = libraryName + "_" + libraryType
        context.update({'classExportDefines': classExportDefines(name)})
        context.update({'classExport': classExport(name)})
    else:
        context.update({'classExportDefines': ""})
        context.update({'classExport': ""})
    generator.write(outputPath, templatePath, context)

def run_generation(input, output, dependency, libraryName, all):
    global generateAsyncProxy
    global generateAll
    generateAll = all
    FileSystem.strict = True
    Generator.strict = True

    # Build the list of modules to be generated
    system = FileSystem.parse(list(input))
    modulesToGenerate = [module.name for module in system.modules]

    system = FileSystem.parse(list(input) + list(dependency))
    generator = Generator(search_path=Path(here / 'templates'))
    generator.register_filter('cppBool',
                              lambda var: "true" if var else "false")
    generator.register_filter('toValidId',
                              lambda name: name.replace('.', '_'))
    generator.register_filter('hasQMLIncompatibleParameter',
                              lambda parameters: any(p.type.is_list or p.type.is_map or p.type.is_interface
                                                     for p in parameters))
    generator.destination = output

    ctx = {'output': output}
    for module in system.modules:
        if module.name in modulesToGenerate:
            ctx.update({'module': module})
            module_path = '/'.join(module.name_parts)
            log.debug('process module %s' % module.module_name)
            ctx.update({'path': module_path})
            generateFile(generator, 'module/{{path}}/ModulePrivate.h', 'ModulePrivate.template.h', ctx, libraryName, "")
            generateFile(generator, 'module/{{path}}/Module.h', 'Module.template.h', ctx, libraryName, "")
            generateFile(generator, 'module/{{path}}/Module.cpp', 'Module.template.cpp', ctx, libraryName, "")
            generateFile(generator, 'ipc/{{path}}/ModuleIPC.h', 'ModuleIPC.template.h', ctx, libraryName, "")
            generateFile(generator, 'ipc/{{path}}/ModuleIPC.cpp', 'ModuleIPC.template.cpp', ctx, libraryName, "")

            for interface in module.interfaces:
                log.debug('process interface %s' % interface)
                ctx.update({'interface': interface})
                ctx.update({'interfaceName': interface.name})
                generateAsyncProxy = False
                ctx.update({'generateAsyncProxy': generateAsyncProxy})
                generateFile(generator, 'types/{{path}}/{{interface}}.h', 'Service.template.h', ctx, libraryName, "")
                generateFile(generator, 'types/{{path}}/{{interface}}.cpp', 'Service.template.cpp', ctx, libraryName, "")
                generateFile(generator, 'types/{{path}}/{{interface}}ImplementationBase.h', 'ImplementationBase.template.h', ctx, libraryName, "")
                generateFile(generator, 'types/{{path}}/{{interface}}ImplementationBase.cpp', 'ImplementationBase.template.cpp', ctx, libraryName, "")
                generateFile(generator, 'types/{{path}}/{{interface}}QMLAdapter.h', 'QMLAdapter.template.h', ctx, libraryName, "")
                generateFile(generator, 'types/{{path}}/{{interface}}QMLAdapter.cpp', 'QMLAdapter.template.cpp', ctx, libraryName, "")

                if isQMLImplementationEnabled(interface):
                    generateFile(generator, 'types/{{path}}/{{interface}}ImplementationBaseQML.h', 'ImplementationBaseQML.template.h', ctx, libraryName, "")

                if isIPCEnabled(interface):
                    generateFile(generator, 'ipc/{{path}}/{{interface}}IPCAdapter.h', 'IPCAdapter.template.h', ctx, libraryName, "")
                    generateFile(generator, 'ipc/{{path}}/{{interface}}IPCAdapter.cpp', 'IPCAdapter.template.cpp', ctx, libraryName, "")
                    generateFile(generator, 'types/{{path}}/{{interface}}IPCCommon.h', 'IPCCommon.template.h', ctx, libraryName, "")
                    generateFile(generator, 'ipc_dbus/{{path}}/{{interface}}IPCDBusAdapter.h', 'IPCDBusServiceAdapter.template.h', ctx, libraryName, "")
                    generateFile(generator, 'ipc_dbus/{{path}}/{{interface}}IPCDBusAdapter.cpp', 'IPCDBusServiceAdapter.template.cpp', ctx, libraryName, "")
                    generateFile(generator, 'ipc/{{path}}/{{interface}}IPCLocalServiceAdapter.h', 'IPCLocalServiceAdapter.template.h', ctx, libraryName, "")
                    generateFile(generator, 'ipc/{{path}}/{{interface}}IPCLocalServiceAdapter.cpp', 'IPCLocalServiceAdapter.template.cpp', ctx, libraryName, "")

                if isSynchronousIPCEnabled(interface):
                    generateFile(generator, 'ipc/{{path}}/{{interface}}IPCProxy.h', 'IPCProxy.template.h', ctx, libraryName, "")
                    generateFile(generator, 'ipc/{{path}}/{{interface}}IPCProxy.cpp', 'IPCProxy.template.cpp', ctx, libraryName, "")
                    generateFile(generator, 'ipc/{{path}}/{{interface}}ServiceWrapper.h', 'ServiceWrapper.template.h', ctx, libraryName, "")
                    generateFile(generator, 'ipc/{{path}}/{{interface}}ServiceWrapper.cpp', 'ServiceWrapper.template.cpp', ctx, libraryName, "")
                    generateFile(generator, 'ipc/{{path}}/{{interface}}NotAvailableImpl.h', 'ServiceNotAvailableImpl.template.h', ctx, libraryName, "")
                    generateFile(generator, 'ipc_dbus/{{path}}/{{interface}}IPCDBusProxy.h', 'IPCDBusProxyAdapter.template.h', ctx, libraryName, "")
                    generateFile(generator, 'ipc_dbus/{{path}}/{{interface}}IPCDBusProxy.cpp', 'IPCDBusProxyAdapter.template.cpp', ctx, libraryName, "")

                if isAsynchronousIPCEnabled(interface):
                    generateAsyncProxy = True
                    ctx.update({'generateAsyncProxy': generateAsyncProxy})
                    ctx.update({'interfaceName': interface.name + interfaceNameSuffix()})
                    generateFile(generator, 'types/{{path}}/{{interface}}Async.h', 'Service.template.h', ctx, libraryName, "")
                    generateFile(generator, 'types/{{path}}/{{interface}}Async.cpp', 'Service.template.cpp', ctx, libraryName, "")
                    generateFile(generator, 'types/{{path}}/{{interface}}AsyncQMLAdapter.h', 'QMLAdapter.template.h', ctx, libraryName, "")
                    generateFile(generator, 'types/{{path}}/{{interface}}AsyncQMLAdapter.cpp', 'QMLAdapter.template.cpp', ctx, libraryName, "")
                    generateFile(generator, 'ipc/{{path}}/{{interface}}AsyncIPCProxy.h', 'IPCProxy.template.h', ctx, libraryName, "")
                    generateFile(generator, 'ipc/{{path}}/{{interface}}AsyncIPCProxy.cpp', 'IPCProxy.template.cpp', ctx, libraryName, "")
                    generateFile(generator, 'ipc/{{path}}/{{interface}}AsyncServiceWrapper.h', 'ServiceWrapper.template.h', ctx, libraryName, "")
                    generateFile(generator, 'ipc/{{path}}/{{interface}}AsyncServiceWrapper.cpp', 'ServiceWrapper.template.cpp', ctx, libraryName, "")
                    generateFile(generator, 'ipc/{{path}}/{{interface}}AsyncNotAvailableImpl.h', 'ServiceNotAvailableImpl.template.h', ctx, libraryName, "")
                    generateFile(generator, 'ipc/{{path}}/{{interface}}AsyncIPCLocalProxyAdapter.h', 'IPCLocalProxyAdapter.template.h', ctx, libraryName, "")
                    generateFile(generator, 'ipc/{{path}}/{{interface}}AsyncIPCLocalProxyAdapter.cpp', 'IPCLocalProxyAdapter.template.cpp', ctx, libraryName, "")
                    generateFile(generator, 'ipc_dbus/{{path}}/{{interface}}AsyncIPCDBusProxy.h', 'IPCDBusProxyAdapter.template.h', ctx, libraryName, "")
                    generateFile(generator, 'ipc_dbus/{{path}}/{{interface}}AsyncIPCDBusProxy.cpp', 'IPCDBusProxyAdapter.template.cpp', ctx, libraryName, "")

            for enum in module.enums:
                ctx.update({'enum': enum})
                generateFile(generator, 'types/{{path}}/{{enum}}.h', 'Enum.template.h', ctx, libraryName, "")
                generateFile(generator, 'types/{{path}}/{{enum}}.cpp', 'Enum.template.cpp', ctx, libraryName, "")
            for struct in module.structs:
                ctx.update({'struct': struct})
                generateFile(generator, 'types/{{path}}/{{struct}}.h', 'Struct.template.h', ctx, libraryName, "")
                generateFile(generator, 'types/{{path}}/{{struct}}.cpp', 'Struct.template.cpp', ctx, libraryName, "")

                if isQObjectWrapperEnabled(struct):
                    generateFile(generator, 'types/{{path}}/{{struct}}QObjectWrapper.h', 'StructQObjectWrapper.template.h', ctx, libraryName, "")
                    generateFile(generator, 'types/{{path}}/{{struct}}QObjectWrapper.cpp', 'StructQObjectWrapper.template.cpp', ctx, libraryName, "")


@click.command()
@click.option('--library', default="")
@click.option('--output', default=".")
@click.option('--input', '-i', multiple=True)
@click.option('--dependency', '-d', multiple=True)
@click.option('--all', is_flag=True)
def generate(input, output, dependency, library, all):
    """Takes several files or directories as input and generates the code
    in the given output directory."""
    run_generation(input, output, dependency, library, all)


if __name__ == '__main__':
    generate()
