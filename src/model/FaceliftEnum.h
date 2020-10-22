/**********************************************************************
**
** Copyright (C) 2019 Luxoft Sweden AB
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

#include "FaceliftCommon.h"
#include <type_traits>
#include <memory>
#include <QString>
#include <QMetaEnum>
#include <QByteArray>

namespace facelift {

namespace Enum {

void raiseFatalError(const QString &string);

// Returns the string that is used as the name of the given enumeration value,
// or an empty string if value is not defined
template<class T, std::enable_if_t<QtPrivate::IsQEnumHelper<T>::Value, T>* = nullptr>
QString toString(T value)
{
    return QMetaEnum::fromType<T>().valueToKey(static_cast<int>(value));
}

// Returns the enumaration value of the given enumeration key, or nullptr if key is not defined.
// TODO change std::unique_ptr to std::optional when it will be possible
template<typename T>
std::unique_ptr<T> fromString(const QString &string)
{
    QByteArray byteArray = string.toLocal8Bit();
    bool ok = false;
    int value = QMetaEnum::fromType<T>().keyToValue(byteArray.data(), &ok);
    
    T result = static_cast<T>(value);

    return ok ? std::make_unique<T>(result) : nullptr;
}


} // end namespace Enum
} // end namespace facelift
