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

#if defined(FaceliftIPCLibDBus_LIBRARY)
#  define FaceliftIPCLibDBus_EXPORT Q_DECL_EXPORT
#else
#  define FaceliftIPCLibDBus_EXPORT Q_DECL_IMPORT
#endif

#include "ipc-dbus.h"
#include "ipc-serialization.h"
#include "DBusIPCProxy.h"
#include "DBusIPCServiceAdapter.h"

namespace facelift {

namespace dbus {



template<size_t I = 0, typename ... Ts>
typename std::enable_if<I == sizeof ... (Ts)>::type
appendDBUSMethodArgumentsSignature(QTextStream &s, std::tuple<Ts ...> &t, const std::array<const char *,
        sizeof ... (Ts)> &argNames)
{
    Q_UNUSED(s);
    Q_UNUSED(t);
    Q_UNUSED(argNames);
}

template<size_t I = 0, typename ... Ts>
typename std::enable_if < I<sizeof ... (Ts)>::type
appendDBUSMethodArgumentsSignature(QTextStream &s, std::tuple<Ts ...> &t, const std::array<const char *,
        sizeof ... (Ts)> &argNames)
{
    using Type = decltype(std::get<I>(t));
    s << "<arg name=\"" << argNames[I] << "\" type=\"";
    IPCTypeHandler<Type>::writeDBUSSignature(s);
    s << "\" direction=\"in\"/>";
    appendDBUSMethodArgumentsSignature<I + 1>(s, t, argNames);
}


template<size_t I = 0, typename ... Ts>
typename std::enable_if<I == sizeof ... (Ts)>::type
appendDBUSSignalArgumentsSignature(QTextStream &s, std::tuple<Ts ...> &t, const std::array<const char *,
        sizeof ... (Ts)> &argNames)
{
    Q_UNUSED(s);
    Q_UNUSED(t);
    Q_UNUSED(argNames);
}

template<size_t I = 0, typename ... Ts>
typename std::enable_if < I<sizeof ... (Ts)>::type
appendDBUSSignalArgumentsSignature(QTextStream &s, std::tuple<Ts ...> &t, const std::array<const char *,
        sizeof ... (Ts)> &argNames)
{
    using Type = decltype(std::get<I>(t));
    s << "<arg name=\"" << argNames[I] << "\" type=\"";
    IPCTypeHandler<Type>::writeDBUSSignature(s);
    s << "\"/>";
    appendDBUSSignalArgumentsSignature<I + 1>(s, t, argNames);
}



template<size_t I = 0, typename ... Ts>
typename std::enable_if<I == sizeof ... (Ts)>::type
appendDBUSTypeSignature(QTextStream &s, std::tuple<Ts ...> &t)
{
    Q_UNUSED(s);
    Q_UNUSED(t);
}

template<size_t I = 0, typename ... Ts>
typename std::enable_if < I<sizeof ... (Ts)>::type
appendDBUSTypeSignature(QTextStream &s, std::tuple<Ts ...> &t)
{
    using Type = decltype(std::get<I>(t));
    IPCTypeHandler<Type>::writeDBUSSignature(s);
    appendDBUSTypeSignature<I + 1>(s, t);
}

template<typename Type>
void addPropertySignature(QTextStream &s, const char *propertyName, bool isReadonly)
{
    s << "<property name=\"" << propertyName << "\" type=\"";
    std::tuple<Type> dummyTuple;
    appendDBUSTypeSignature(s, dummyTuple);
    s << "\" access=\"" << (isReadonly ? "read" : "readwrite") << "\"/>";
}

template<typename ... Args>
void addMethodSignature(QTextStream &s, const char *methodName,
        const std::array<const char *, sizeof ... (Args)> &argNames)
{
    s << "<method name=\"" << methodName << "\">";
    std::tuple<Args ...> t;  // TODO : get rid of the tuple
    appendDBUSMethodArgumentsSignature(s, t, argNames);
    s << "</method>";
}

template<typename ... Args>
void addSignalSignature(QTextStream &s, const char *methodName,
        const std::array<const char *, sizeof ... (Args)> &argNames)
{
    s << "<signal name=\"" << methodName << "\">";
    std::tuple<Args ...> t;  // TODO : get rid of the tuple
    appendDBUSSignalArgumentsSignature(s, t, argNames);
    s << "</signal>";
}


template<typename Type>
inline void DBusIPCServiceAdapterBase::serializeValue(DBusIPCMessage &msg, const Type &v)
{
    typedef typename IPCTypeRegisterHandler<Type>::SerializedType SerializedType;
    IPCTypeHandler<SerializedType>::write(msg.outputPayLoad(), IPCTypeRegisterHandler<Type>::convertToSerializedType(v, *this));
}

template<typename Type>
inline void DBusIPCServiceAdapterBase::deserializeValue(DBusIPCMessage &msg, Type &v)
{
    typedef typename IPCTypeRegisterHandler<Type>::SerializedType SerializedType;
    SerializedType serializedValue;
    IPCTypeHandler<Type>::read(msg.inputPayLoad(), serializedValue);
    IPCTypeRegisterHandler<Type>::convertToDeserializedType(v, serializedValue, *this);
}


template<typename Type>
inline void DBusIPCProxyBinder::serializeValue(DBusIPCMessage &msg, const Type &v)
{
    typedef typename IPCTypeRegisterHandler<Type>::SerializedType SerializedType;
    IPCTypeHandler<SerializedType>::write(msg.outputPayLoad(), IPCTypeRegisterHandler<Type>::convertToSerializedType(v, *this));
}

template<typename Type>
inline void DBusIPCProxyBinder::deserializeValue(DBusIPCMessage &msg, Type &v)
{
    typedef typename IPCTypeRegisterHandler<Type>::SerializedType SerializedType;
    SerializedType serializedValue;
    IPCTypeHandler<SerializedType>::read(msg.inputPayLoad(), serializedValue);
    IPCTypeRegisterHandler<Type>::convertToDeserializedType(v, serializedValue, *this);
}


template<typename ... Args>
inline DBusIPCMessage DBusIPCProxyBinder::sendMethodCall(const char *methodName, const Args & ... args) const
{
    DBusIPCMessage msg(m_serviceName, objectPath(), m_interfaceName, methodName);
    auto argTuple = std::make_tuple(args ...);
    for_each_in_tuple(argTuple, SerializeParameterFunction<DBusIPCProxyBinder>(msg.outputPayLoad(), *this));
    auto replyMessage = msg.call(connection());
    if (replyMessage.isErrorMessage()) {
        onServerNotAvailableError(methodName);
    }
    return replyMessage;
}

template<typename ReturnType, typename ... Args>
inline void DBusIPCProxyBinder::sendAsyncMethodCall(const char *methodName, facelift::AsyncAnswer<ReturnType> answer, const Args & ... args)
{
    DBusIPCMessage msg(m_serviceName, objectPath(), m_interfaceName, methodName);
    auto argTuple = std::make_tuple(args ...);
    for_each_in_tuple(argTuple, SerializeParameterFunction<DBusIPCProxyBinder>(msg.outputPayLoad(), *this));
    msg.asyncCall(connection(), this, [this, answer](DBusIPCMessage &msg) {
        ReturnType returnValue;
        if (msg.isReplyMessage()) {
            deserializeValue(msg, returnValue);
            answer(returnValue);
        } else {
            qCWarning(LogIpc) << "Error received" << msg.toString();
        }
    });
}

template<typename ... Args>
inline void DBusIPCProxyBinder::sendAsyncMethodCall(const char *methodName, facelift::AsyncAnswer<void> answer, const Args & ... args)
{
    DBusIPCMessage msg(m_serviceName, objectPath(), m_interfaceName, methodName);
    auto argTuple = std::make_tuple(args ...);
    for_each_in_tuple(argTuple, SerializeParameterFunction<DBusIPCProxyBinder>(msg.outputPayLoad(), *this));
    msg.asyncCall(connection(), this, [answer](DBusIPCMessage &msg) {
        Q_UNUSED(msg);
        answer();
    });
}

template<typename ReturnType, typename ... Args>
inline void DBusIPCProxyBinder::sendMethodCallWithReturn(const char *methodName, ReturnType &returnValue, const Args & ... args) const
{
    DBusIPCMessage msg = sendMethodCall(methodName, args ...);
    if (msg.isReplyMessage()) {
        const_cast<DBusIPCProxyBinder *>(this)->deserializeValue(msg, returnValue);
    } else {
        assignDefaultValue(returnValue);
    }
}


template<typename PropertyType>
inline void DBusIPCProxyBinder::sendSetterCall(const char *methodName, const PropertyType &value)
{
    DBusIPCMessage msg(m_serviceName, objectPath(), m_interfaceName, methodName);
    serializeValue(msg, value);
    if (isSynchronous()) {
        auto replyMessage = msg.call(connection());
        if (replyMessage.isErrorMessage()) {
            onServerNotAvailableError(methodName);
        }
    } else {
        msg.asyncCall(connection(), this, [this, methodName](const DBusIPCMessage &replyMessage) {
            if (replyMessage.isErrorMessage()) {
                onServerNotAvailableError(methodName);
            }
        });
    }
}


template<typename MemberID, typename ... Args>
inline void DBusIPCServiceAdapterBase::sendSignal(MemberID signalID, const Args & ... args)
{
    if (m_pendingOutgoingMessage == nullptr) {
        initOutgoingSignalMessage();
        auto argTuple = std::make_tuple(signalID, args ...);
        for_each_in_tuple(argTuple, SerializeParameterFunction<DBusIPCServiceAdapterBase>(m_pendingOutgoingMessage->outputPayLoad(), *this));
        flush();
    }
}

template<typename ReturnType>
inline void DBusIPCServiceAdapterBase::sendAsyncCallAnswer(DBusIPCMessage &replyMessage, const ReturnType returnValue)
{
    serializeValue(replyMessage, returnValue);
    replyMessage.send(dbusManager().connection());
}

template<typename Type>
inline void DBusIPCServiceAdapterBase::serializeOptionalValue(DBusIPCMessage &msg, const Type &currentValue, Type &previousValue, bool isCompleteSnapshot)
{
    if (isCompleteSnapshot) {
        serializeValue(msg, currentValue);
    } else {
        if (previousValue == currentValue) {
            msg.outputPayLoad().writeSimple(false);
        } else {
            msg.outputPayLoad().writeSimple(true);
            serializeValue(msg, currentValue);
            previousValue = currentValue;
        }
    }
}

template<typename Type>
inline void DBusIPCServiceAdapterBase::serializeOptionalValue(DBusIPCMessage &msg, const Type &currentValue, bool isCompleteSnapshot)
{
    msg.outputPayLoad().writeSimple(isCompleteSnapshot);
    if (isCompleteSnapshot) {
        serializeValue(msg, currentValue);
    }
}
}

}
