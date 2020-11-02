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

#include "LocalIPCMessage.h"
#include "ipc-serialization.h"
#include "LocalIPCProxy.h"
#include "LocalIPCServiceAdapter.h"

namespace facelift {

namespace local {


template<typename Type>
inline void LocalIPCServiceAdapterBase::serializeValue(LocalIPCMessage &msg, const Type &v)
{
    typedef typename IPCTypeRegisterHandler<Type>::SerializedType SerializedType;
    IPCTypeHandler<SerializedType>::write(msg.outputPayLoad(), IPCTypeRegisterHandler<Type>::convertToSerializedType(v, *this));
}

template<typename Type>
inline void LocalIPCServiceAdapterBase::deserializeValue(LocalIPCMessage &msg, Type &v)
{
    typedef typename IPCTypeRegisterHandler<Type>::SerializedType SerializedType;
    SerializedType serializedValue;
    IPCTypeHandler<Type>::read(msg.inputPayLoad(), serializedValue);
    IPCTypeRegisterHandler<Type>::convertToDeserializedType(v, serializedValue, *this);
}


template<typename Type>
inline void LocalIPCProxyBinder::serializeValue(LocalIPCMessage &msg, const Type &v)
{
    typedef typename IPCTypeRegisterHandler<Type>::SerializedType SerializedType;
    IPCTypeHandler<SerializedType>::write(msg.outputPayLoad(), IPCTypeRegisterHandler<Type>::convertToSerializedType(v, *this));
}

template<typename Type>
inline void LocalIPCProxyBinder::deserializeValue(LocalIPCMessage &msg, Type &v)
{
    typedef typename IPCTypeRegisterHandler<Type>::SerializedType SerializedType;
    SerializedType serializedValue;
    IPCTypeHandler<SerializedType>::read(msg.inputPayLoad(), serializedValue);
    IPCTypeRegisterHandler<Type>::convertToDeserializedType(v, serializedValue, *this);
}


template<typename ... Args>
inline LocalIPCMessage LocalIPCProxyBinder::sendMethodCall(const char *methodName, const Args & ... args) const
{
    LocalIPCMessage msg(methodName);
    auto argTuple = std::make_tuple(args ...);
    for_each_in_tuple(argTuple, SerializeParameterFunction<LocalIPCProxyBinder>(msg.outputPayLoad(), *this));
    auto replyMessage = call(msg);
    if (replyMessage.isErrorMessage()) {
        onServerNotAvailableError(methodName);
    }
    return replyMessage;
}

template<typename ReturnType, typename ... Args>
inline void LocalIPCProxyBinder::sendAsyncMethodCall(const char *methodName, facelift::AsyncAnswer<ReturnType> answer, const Args & ... args)
{
    LocalIPCMessage msg(methodName);
    auto argTuple = std::make_tuple(args ...);
    for_each_in_tuple(argTuple, SerializeParameterFunction<LocalIPCProxyBinder>(msg.outputPayLoad(), *this));
    asyncCall(msg, this, [this, answer](LocalIPCMessage &msg) {
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
inline void LocalIPCProxyBinder::sendAsyncMethodCall(const char *methodName, facelift::AsyncAnswer<void> answer, const Args & ... args)
{
    LocalIPCMessage msg(methodName);
    auto argTuple = std::make_tuple(args ...);
    for_each_in_tuple(argTuple, SerializeParameterFunction<LocalIPCProxyBinder>(msg.outputPayLoad(), *this));
    asyncCall(msg, this, [answer](LocalIPCMessage &msg) {
                Q_UNUSED(msg);
                answer();
            });
}

template<typename ReturnType, typename ... Args>
inline void LocalIPCProxyBinder::sendMethodCallWithReturn(const char *methodName, ReturnType &returnValue, const Args & ... args) const
{
    LocalIPCMessage msg = sendMethodCall(methodName, args ...);
    if (msg.isReplyMessage()) {
        const_cast<LocalIPCProxyBinder *>(this)->deserializeValue(msg, returnValue);
    } else {
        assignDefaultValue(returnValue);
    }
}


template<typename PropertyType>
inline void LocalIPCProxyBinder::sendSetterCall(const char *methodName, const PropertyType &value)
{
    LocalIPCMessage msg(methodName);
    serializeValue(msg, value);
    if (isSynchronous()) {
        auto replyMessage = call(msg);
        if (replyMessage.isErrorMessage()) {
            onServerNotAvailableError(methodName);
        }
    } else {
        asyncCall(msg, this, [this, methodName](const LocalIPCMessage &replyMessage) {
                    if (replyMessage.isErrorMessage()) {
                        onServerNotAvailableError(methodName);
                    }
                });
    }
}


template<typename MemberID, typename ... Args>
inline void LocalIPCServiceAdapterBase::sendSignal(MemberID signalID, const Args & ... args)
{
    if (m_pendingOutgoingMessage == nullptr) {
        initOutgoingSignalMessage();
        auto argTuple = std::make_tuple(signalID, args ...);
        for_each_in_tuple(argTuple, SerializeParameterFunction<LocalIPCServiceAdapterBase>(m_pendingOutgoingMessage->outputPayLoad(), *this));
        flush();
    }
}

template<typename ReturnType>
inline void LocalIPCServiceAdapterBase::sendAsyncCallAnswer(LocalIPCMessage &replyMessage, const ReturnType returnValue)
{
    serializeValue(replyMessage, returnValue);
    sendReply(replyMessage);
}

template<typename Type>
inline void LocalIPCServiceAdapterBase::serializeOptionalValue(LocalIPCMessage &msg, const Type &currentValue, Type &previousValue,
        bool isCompleteSnapshot)
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
inline void LocalIPCServiceAdapterBase::serializeOptionalValue(LocalIPCMessage &msg, const Type &currentValue, bool isCompleteSnapshot)
{
    msg.outputPayLoad().writeSimple(isCompleteSnapshot);
    if (isCompleteSnapshot) {
        serializeValue(msg, currentValue);
    }
}
}

}
