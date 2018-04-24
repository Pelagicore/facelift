{% set ampersand = joiner(" &&") %}
{% set comma = joiner(", ") %}
/****************************************************************************
** This is an auto-generated file.
** Do not edit! All changes made to it will be lost.
****************************************************************************/

#include "{{struct}}.h"

{{module|namespaceOpen}}

const {{struct}}::FieldNames {{struct}}::FIELD_NAMES = {
    {%- for field in struct.fields -%}
    {{ comma() }}
    "{{field.name}}"
    {%- endfor %}

};

{{module|namespaceClose}}
