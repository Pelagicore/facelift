/**********************************************************************
**
** Copyright (C) 2020 Luxoft Sweden AB
**
** This file is part of the FaceLift project
**
** Permission is hereby granted, free of charge, to any person
** obtaining a copy of this software and associated documentation files
** (the "Software"), to deal in the Software without restriction,
** including without limitation the rights to use, copy, modify, merge,
** publish, distribute, sublicense, and/or sell copies of the Software,
** and to permit persons to whom the Software is furnished to do so,
** subject to the following conditions:
**
** The above copyright notice and this permission notice shall be
** included in all copies or substantial portions of the Software.
**
** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
** EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
** MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
** NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
** BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
** ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
** CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
** SOFTWARE.
**
** SPDX-License-Identifier: MIT
**
**********************************************************************/

#pragma once

#include <QTextStream>
#include <array>

#include "StructureBase.h"
#include "StringConversionHandler.h"

namespace facelift {

template<typename ... FieldTypes>
class Structure : public StructureBase
{

public:
    virtual ~Structure() = default;

    typedef std::tuple<FieldTypes ...> FieldTupleTypes;
    static constexpr size_t FieldCount = sizeof ... (FieldTypes);

    typedef std::array<const char *, FieldCount> FieldNames;

    const FieldTupleTypes &asTuple() const
    {
        return m_values;
    }

    FieldTupleTypes &asTuple()
    {
        return m_values;
    }

    void setValue(FieldTupleTypes value)
    {
        m_values = value;
    }

    void copyFrom(const Structure &other)
    {
        setValue(other.m_values);
        m_userData = other.m_userData;
    }

    bool operator==(const Structure &right) const
    {
        return (m_values == right.m_values);
    }

    bool operator!=(const Structure &right) const
    {
        return !(this->operator==(right));
    }

protected:
    template<class Tuple, std::size_t... Is>
    static void toStringWithFields(const Tuple &t, std::index_sequence<Is...>, const FieldNames &names,
                                   QTextStream &outStream)
    {
        using expander = int[]; // workaround because fold expression is only available in C++17
        (void)expander{0, (void(outStream << (Is == 0 ? "" : ", ") << names[Is] << "="
                                          << StringConversionHandler<typename std::tuple_element<Is, Tuple>::type>::toString(std::get< Is >(t))), 0)...};
    }

    QString toStringWithFields(const QString &structName, const FieldNames &names) const
    {
        QString s;
        QTextStream outStream(&s);
        outStream << structName << " { ";

        toStringWithFields(m_values, std::make_index_sequence<FieldCount>{}, names, outStream);
        outStream << " }";

        return s;
    }

    FieldTupleTypes m_values = {};
};


}

