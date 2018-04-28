/****************************************************************************
** This is an auto-generated file.
** Do not edit! All changes made to it will be lost.
****************************************************************************/

#pragma once

#include "FaceliftModel.h"

{% for struct in module.structs %}
#include "{{struct|fullyQualifiedPath}}.h"
{% endfor %}

{% for enum in module.enums %}
#include "{{enum|fullyQualifiedPath}}.h"
{% endfor %}

{% for interface in module.interfaces %}
#include "{{interface|fullyQualifiedPath}}.h"
#include "{{interface|fullyQualifiedPath}}QMLFrontend.h"
{% endfor %}

{{module|namespaceOpen}}

/**
* \brief {{module.name}} module singleton object
* \ingroup {{module.name|toValidId}}
* \inqmlmodule {{module.name}}
* This singleton can be used to create instances of the types defined in the {{module.name}} module/package.
*/
class Module : public facelift::ModuleBase {

public:
	Module();

    /**
     * Register all QML types provided by this module
     */
    static void registerQmlTypes(const char* uri, int majorVersion = {{module.majorVersion}}, int minorVersion = {{module.minorVersion}});

    /**
     * Register all interfaces as uncreatable types.
     * An interface called "MyInterface" in the QFace file is registered as uncreatable under the name "MyInterface"
     */
    static void registerUncreatableQmlTypes(const char* uri, int majorVersion = {{module.majorVersion}}, int minorVersion = {{module.minorVersion}});

private:
    /**
     * Register metatypes of all types defined by this module. This method should not be called by any hand-written code
     */
    static void registerTypes();

};

{{module|namespaceClose}}
