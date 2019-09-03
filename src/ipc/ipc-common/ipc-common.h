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

#if defined(FaceliftIPCCommonLib_LIBRARY)
#  define FaceliftIPCCommonLib_EXPORT Q_DECL_EXPORT
#else
#  define FaceliftIPCCommonLib_EXPORT Q_DECL_IMPORT
#endif

#include <QDataStream>

#include "FaceliftModel.h"

namespace facelift {

FaceliftIPCCommonLib_EXPORT Q_DECLARE_LOGGING_CATEGORY(LogIpc)

enum class CommonSignalID {
    readyChanged,
    firstSpecific
};

typedef int ASyncRequestID;

enum class IPCHandlingResult {
    OK,          // Message is successfully handled
    OK_ASYNC,    // Message is handled but it is an asynchronous request, so no reply should be sent for now
    INVALID,     // Message is invalid and could not be handled
};

enum class ModelUpdateEvent {
    DataChanged,
    Insert,
    Remove,
    Move,
    Reset
};

template<typename Type>
inline void assignDefaultValue(Type &v)
{
    v = Type {};
}



template<typename Type, typename Enable = void>
struct IPCTypeRegisterHandler
{
    typedef Type SerializedType;

    template<typename OwnerType>
    static const Type &convertToSerializedType(const Type &v, OwnerType &adapter)
    {
        Q_UNUSED(adapter);
        return v;
    }

    template<typename OwnerType>
    static void convertToDeserializedType(Type &v, const SerializedType &serializedValue, OwnerType &adapter)
    {
        Q_UNUSED(adapter);
        v = serializedValue;
    }

};


template<typename Type>
struct IPCTypeRegisterHandler<QList<Type> >
{
    typedef QList<typename IPCTypeRegisterHandler<Type>::SerializedType> SerializedType;

    template<typename OwnerType>
    static SerializedType convertToSerializedType(const QList<Type> &v, OwnerType &adapter)
    {
        Q_UNUSED(v);
        Q_UNUSED(adapter);
        SerializedType convertedValue;
        for (const auto &e : v) {
            convertedValue.append(IPCTypeRegisterHandler<Type>::convertToSerializedType(e, adapter));
        }
        return convertedValue;
    }

    template<typename OwnerType>
    static void convertToDeserializedType(QList<Type> &v, const SerializedType &serializedValue, OwnerType &adapter)
    {
        v.clear();
        for (const auto &e : serializedValue) {
            Type c;
            IPCTypeRegisterHandler<Type>::convertToDeserializedType(c, e, adapter);
            v.append(c);
        }
    }

};


template<typename Type>
struct IPCTypeRegisterHandler<QMap<QString, Type> >
{
    typedef QMap<QString, typename IPCTypeRegisterHandler<Type>::SerializedType> SerializedType;

    template<typename OwnerType>
    static SerializedType convertToSerializedType(const QMap<QString, Type> &v, OwnerType &adapter)
    {
        SerializedType convertedValue;
        for (const auto &key : v.keys()) {
            convertedValue.insert(key, IPCTypeRegisterHandler<Type>::convertToSerializedType(v[key], adapter));
        }
        return convertedValue;
    }

    template<typename OwnerType>
    static void convertToDeserializedType(QMap<QString, Type> &v, const SerializedType &serializedValue, OwnerType &adapter)
    {
        v.clear();
        for (const auto &key : serializedValue.keys()) {
            Type c;
            IPCTypeRegisterHandler<Type>::convertToDeserializedType(c, serializedValue[key], adapter);
            v.insert(key, c);
        }
    }

};


template<typename Type>
struct IPCTypeRegisterHandler<Type *, typename std::enable_if<std::is_base_of<InterfaceBase, Type>::value>::type>
{
    typedef QString SerializedType;

    template<typename OwnerType>
    static SerializedType convertToSerializedType(Type *const &v, OwnerType &adapter)
    {
        using IPCAdapterType = typename OwnerType::template IPCAdapterType<Type>;
        return adapter.template getOrCreateAdapter< IPCAdapterType >(v)->objectPath();
    }

    template<typename OwnerType>
    static void convertToDeserializedType(Type * &v, const SerializedType &serializedValue, OwnerType &owner)
    {
        using IPCProxyType = typename OwnerType::template IPCProxyType<Type>;
        v = owner.template getOrCreateSubProxy<IPCProxyType>(serializedValue);
    }

};


class FaceliftIPCCommonLib_EXPORT OutputPayLoad
{

public:
    OutputPayLoad(QByteArray &payloadArray) : m_payloadArray(payloadArray), m_dataStream(&m_payloadArray, QIODevice::WriteOnly)
    {
    }

    template<typename Type>
    void writeSimple(const Type &v)
    {
        //        qCDebug(LogIpc) << "Writing to message : " << v;
        m_dataStream << v;
    }

    const QByteArray &getContent() const
    {
        return m_payloadArray;
    }

private:
    QByteArray& m_payloadArray;
    QDataStream m_dataStream;
};


class FaceliftIPCCommonLib_EXPORT InputPayLoad
{

public:
    InputPayLoad(const QByteArray &payloadArray) :
        m_payloadArray(payloadArray),
        m_dataStream(m_payloadArray)
    {
    }

    ~InputPayLoad()
    {
    }

    template<typename Type>
    void readNextParameter(Type &v)
    {
        m_dataStream >> v;
        //        qCDebug(LogIpc) << "Read from message : " << v;
    }

    const QByteArray &getContent() const
    {
        return m_payloadArray;
    }

private:
    const QByteArray& m_payloadArray;
    QDataStream m_dataStream;
};


template<typename Type, typename Enable = void>
struct IPCTypeHandler
{
    static void writeDBUSSignature(QTextStream &s)
    {
        s << "i";
    }

    static void write(OutputPayLoad &msg, const Type &v)
    {
        msg.writeSimple(v);
    }

    static void read(InputPayLoad &msg, Type &v)
    {
        msg.readNextParameter(v);
    }

};


}

