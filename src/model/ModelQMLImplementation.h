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


#include "ModelQMLImplementationBase.h"

namespace facelift {

template<typename InterfaceType>
class ModelQMLImplementation : public ModelQMLImplementationBase
{

public:
    ModelQMLImplementation(QObject *parent = nullptr) :
        ModelQMLImplementationBase(parent)
    {
    }

    static void setModelImplementationFilePath(QString path)
    {
        modelImplementationFilePath() = path;
    }

    static QString &modelImplementationFilePath()
    {
        static QString s_modelImplementationFilePath;
        return s_modelImplementationFilePath;
    }

    virtual void initProvider(InterfaceType *provider) = 0;

    void retrieveFrontend()
    {
        m_interface = retrieveFrontendUnderConstruction();
        if (m_interface == nullptr) {
            m_interface = createFrontend();
        }

        Q_ASSERT(m_interface);

        initProvider(m_interface);
        setInterface(m_interface);
    }

    virtual InterfaceType *createFrontend() = 0;

    static void registerTypes(const char *theURI)
    {
        typedef typename InterfaceType::QMLImplementationModelType QMLImplementationModelType;

        // Register the component used to actually implement the model in QML
        // the QML file containing the model implementation should have this type at its root
        ::qmlRegisterType<QMLImplementationModelType>(theURI, 1, 0, QMLImplementationModelType::QML_NAME);
    }

    static InterfaceType *retrieveFrontendUnderConstruction()
    {
        auto instance = frontendUnderConstruction();
        frontendUnderConstruction() = nullptr;
        return instance;
    }

    static void setFrontendUnderConstruction(InterfaceType *instance)
    {
        Q_ASSERT(frontendUnderConstruction() == nullptr);
        frontendUnderConstruction() = instance;
    }

    void checkInterface() const
    {
        Q_ASSERT(m_interface != nullptr);
    }

private:
    static InterfaceType * &frontendUnderConstruction()
    {
        static InterfaceType *i = nullptr;
        return i;
    }

protected:
    InterfaceType *provider() const
    {
        return m_interface;
    }

    InterfaceType *m_interface = nullptr;

};


}
