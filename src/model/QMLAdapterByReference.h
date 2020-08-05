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

#include <QObject>
#include <QQmlEngine>

namespace facelift {

/*!
 * This is the class which is registered when calling registerQmlComponent()
 * It is an actual instance of the QMLAdapterType and wraps an instance of the provider
 */
template<typename ProviderImplementationType>
class QMLAdapterByReference : public ProviderImplementationType::QMLAdapterType
{

public:
    QMLAdapterByReference(ProviderImplementationType& provider, QObject *parent = nullptr)
        : ProviderImplementationType::QMLAdapterType(parent), m_provider(provider)
    {
    }

    QMLAdapterByReference(ProviderImplementationType& provider, QQmlEngine *engine)
        : ProviderImplementationType::QMLAdapterType(engine), m_provider(provider)
    {
    }

    const QString &implementationID() override
    {
        return m_provider.implementationID();
    }

    void componentComplete() override
    {
        // notify anyone interested that we are ready (such as an IPC attached property)
        m_provider.setComponentCompleted();
    }

    void connectProvider() {
        ProviderImplementationType::QMLAdapterType::connectProvider(m_provider);
    }

private:
    ProviderImplementationType& m_provider;

};


}
