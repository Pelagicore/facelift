/**********************************************************************
**
** Copyright (C) 2018 Luxoft Sweden AB
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

#include <QtDBus>
#include <mutex> // std::once_flag
#include "FaceliftUtils.h"
#include "StructureBase.h"
#include "DBusIPCCommon.h"

template<typename T> struct HelperType { };

template<typename T, typename std::enable_if_t<!std::is_base_of<facelift::StructureBase, T>::value && !std::is_enum<T>::value, int> = 0>
void registerDBusType(HelperType<T>)
{
    // DO NOTHIG FOR BUILTIN TYPES
}

template<typename T, typename std::enable_if_t<!std::is_base_of<facelift::StructureBase, T>::value && std::is_enum<T>::value, int> = 0>
void registerDBusType(HelperType<T>)
{
    // Can't register an enum to have basic DBus types (int) in QtDBus -> Bug ticket QTBUG-86867
    // workaround is to handle in castToQVariant the conversion
    //qDBusRegisterMetaType<T>();
}

template<typename T, typename std::enable_if_t<std::is_base_of<facelift::StructureBase, T>::value, int> = 0>
void registerDBusType(HelperType<T>)
{
    qDBusRegisterMetaType<T>();
    Q_ASSERT_X(strlen(QDBusMetaType::typeToSignature(qMetaTypeId<T>())) < facelift::dbus::DBusIPCCommon::FACELIFT_DBUS_MAXIMUM_SIGNATURE_LENGTH, "Signature overflow",
               "Struct's signature exceeds dbus limit, annonate @toByteArrayOverDBus to switch to byte array stream of the struct over dbus, better yet rethink your structure!");
}
template<typename T, typename std::enable_if_t<std::is_base_of<facelift::StructureBase, T>::value, int> = 0>
void registerDBusType(HelperType<QList<T>>)
{
    qDBusRegisterMetaType<T>();
    qDBusRegisterMetaType<QList<T>>();
    Q_ASSERT_X(strlen(QDBusMetaType::typeToSignature(qMetaTypeId<QList<T>>())) < facelift::dbus::DBusIPCCommon::FACELIFT_DBUS_MAXIMUM_SIGNATURE_LENGTH, "Signature overflow",
               "Struct's signature exceeds dbus limit, annonate @toByteArrayOverDBus to switch to byte array stream of the struct over dbus, better yet rethink your structure!");
}

template<typename T, typename std::enable_if_t<std::is_base_of<facelift::StructureBase, T>::value, int> = 0>
void registerDBusType(HelperType<QMap<QString, T>>)
{
    qDBusRegisterMetaType<T>();
    qDBusRegisterMetaType<QMap<QString, T>>();
    Q_ASSERT_X(strlen(QDBusMetaType::typeToSignature(qMetaTypeId<QMap<QString, T>>())) < facelift::dbus::DBusIPCCommon::FACELIFT_DBUS_MAXIMUM_SIGNATURE_LENGTH, "Signature overflow",
               "Struct's signature exceeds dbus limit, annonate @toByteArrayOverDBus to switch to byte array stream of the struct over dbus, better yet rethink your structure!");
}

struct ToQDBusArgument
{
    ToQDBusArgument(QDBusArgument& argument): m_argument(argument)
    {}
    template<typename T>
    void operator()(T &&value)
    {
        static std::once_flag registerFlag;
        std::call_once(registerFlag, [](){registerDBusType(HelperType<typename std::decay<T>::type>());});
        m_argument << value;
    }
    // Can't register an enum to have basic DBus types (int) in QtDBus -> Bug ticket QTBUG-86867
    // workaround to convert enum to int
    template<typename T, typename std::enable_if_t<std::is_enum<typename std::decay<T>::type>::value, int> = 0>
    void operator()(const QList<T> &value)
    {
        QList<int> tmp;
        std::transform(value.begin(), value.end(), std::back_inserter(tmp), [](const T entry){return static_cast<int>(entry);});
        m_argument << tmp;
    }
    template<typename T, typename std::enable_if_t<std::is_enum<typename std::decay<T>::type>::value, int> = 0>
    void operator()(const QMap<QString, T> &value)
    {
        QMap<QString, int> tmp;
        for (const QString key: value.keys()) {
            tmp[key] = static_cast<int>(value[key]);
        }
        m_argument << tmp;
    }
    QDBusArgument m_argument;
};

struct FromQDBusArgument
{
    FromQDBusArgument(const QDBusArgument& argument): m_argument(argument)
    {}
    template<typename T>
    void operator()(T &&value)
    {
        static std::once_flag registerFlag;
        std::call_once(registerFlag, [](){registerDBusType(HelperType<typename std::decay<T>::type>());});
        m_argument >> value;
    }
    // Can't register an enum to have basic DBus types (int) in QtDBus -> Bug ticket QTBUG-86867
    // workaround to convert enum to int
    template<typename T, typename std::enable_if_t<std::is_enum<typename std::decay<T>::type>::value, int> = 0>
    void operator()(const QList<T> &value)
    {
        QList<int> tmp;
        m_argument >> tmp;
        std::transform(tmp.begin(), tmp.end(), std::back_inserter(value), [](const int entry){return static_cast<T>(entry);});
    }
    template<typename T, typename std::enable_if_t<std::is_enum<typename std::decay<T>::type>::value, int> = 0>
    void operator()(const QMap<QString, T> &value)
    {
        QMap<QString, int> tmp;
        m_argument >> tmp;
        for (const QString& key: tmp.keys()) {
            value[key] = static_cast<T>(tmp[key]);
        }
    }
    const QDBusArgument& m_argument;
};

struct FromQDataStream
{
    FromQDataStream(QDataStream& dataStream): m_dataStream(dataStream)
    {}
    template<typename T>
    void operator()(T &&value)
    {
        m_dataStream >> value;
    }
    QDataStream& m_dataStream;
};

struct ToQDataStream
{
    ToQDataStream(QDataStream& dataStream): m_dataStream(dataStream)
    {}
    template<typename T>
    void operator()(T &&value)
    {
        m_dataStream << value;
    }
    QDataStream& m_dataStream;
};

template <typename T>
class ToByteArrayOverDBus
{
private:
    typedef char YesType[1];
    typedef char NoType[2];

    template <typename C> static YesType& test( decltype(&C::toByteArrayOverDBus) ) ;
    template <typename C> static NoType& test(...);
public:
    enum { value = sizeof(test<T>(0)) == sizeof(YesType) };
};


template<typename T, typename std::enable_if_t<!std::is_convertible<T, facelift::StructureBase>::value, int> = 0>
inline QDataStream& operator<<(QDataStream &dataStream, const T& source)
{
    dataStream << static_cast<int>(source);
    return dataStream;
}

template<typename T, typename std::enable_if_t<!std::is_convertible<T, facelift::StructureBase>::value, int> = 0>
inline const QDataStream& operator>>(QDataStream &dataStream, T &source)
{
    int value;
    dataStream >> value;
    source = static_cast<T>(value);
    return dataStream;
}

template<typename T, typename std::enable_if_t<std::is_convertible<T, facelift::StructureBase>::value, int> = 0>
inline QDataStream& operator<<( QDataStream& dataStream, const T &structure)
{
    facelift::for_each_in_tuple_const(structure.asTuple(), ToQDataStream(dataStream));
    return dataStream;
}

template<typename T, typename std::enable_if_t<std::is_convertible<T, facelift::StructureBase>::value, int> = 0>
inline QDataStream& operator>>( QDataStream& dataStream, T &structure)
{
    facelift::for_each_in_tuple(structure.asTuple(), FromQDataStream(dataStream));
    return dataStream;
}

// not neccessary if Qt allows defining Enum as INT
template<typename T, typename std::enable_if_t<std::is_enum<T>::value, int> = 0>
inline QDBusArgument& operator<<(QDBusArgument &argument, const T& source)
{
    argument << static_cast<int>(source);
    return argument;
}

template<typename T, typename std::enable_if_t<std::is_enum<T>::value, int> = 0>
inline const QDBusArgument& operator>>(const QDBusArgument &argument, T &source)
{
    int tmp;
    argument >> tmp;
    source = static_cast<T>(tmp);
    return argument;
}

template<typename T, typename std::enable_if_t<std::is_convertible<T, facelift::StructureBase>::value && !ToByteArrayOverDBus<T>::value, int> = 0>
inline QDBusArgument &operator<<(QDBusArgument &argument, const T &structure)
{
    argument.beginStructure();
    facelift::for_each_in_tuple_const(structure.asTuple(), ToQDBusArgument(argument));
    argument.endStructure();
    return argument;
}

template<typename T, typename std::enable_if_t<std::is_convertible<T, facelift::StructureBase>::value && ToByteArrayOverDBus<T>::value, int> = 0>
inline QDBusArgument &operator<<(QDBusArgument &argument, const T &structure)
{
    argument.beginStructure();
    QByteArray byteArray;
    QDataStream dataStream(&byteArray, QIODevice::WriteOnly);
    dataStream << structure;
    argument << byteArray;
    argument.endStructure();
    return argument;
}

template<typename T, typename std::enable_if_t<std::is_convertible<T, facelift::StructureBase>::value && !ToByteArrayOverDBus<T>::value, int> = 0>
inline const QDBusArgument &operator>>(const QDBusArgument &argument, T &structure)
{
    argument.beginStructure();
    facelift::for_each_in_tuple(structure.asTuple(), FromQDBusArgument(argument));
    argument.endStructure();
    return argument;
}

template<typename T, typename std::enable_if_t<std::is_convertible<T, facelift::StructureBase>::value && ToByteArrayOverDBus<T>::value, int> = 0>
inline const QDBusArgument &operator>>(const QDBusArgument &argument, T &structure)
{
    argument.beginStructure();
    QByteArray byteArray;
    QDataStream dataStream(&byteArray, QIODevice::ReadOnly);
    argument >> byteArray;
    dataStream >> structure;
    argument.endStructure();
    return argument;
}
