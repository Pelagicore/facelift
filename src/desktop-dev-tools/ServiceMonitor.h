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

#include <QObject>
#include <QMap>
#include <QListWidgetItem>

#include "ControlWidgets.h"


class Ui_ServiceMonitorPanel;
class Ui_ServiceMonitorManagerWindow;

namespace facelift {

class ServiceMonitorBase;

template<typename Type>
ServiceMonitorBase *monitorFactory(InterfaceBase *provider)
{
    typedef typename Type::Provider_Type ProviderType;
    auto p = qobject_cast<ProviderType *>(provider);
    return new Type(*p);
}


class ServiceMonitorManager : public QAbstractTableModel
{
    Q_OBJECT

public:
    typedef ServiceMonitorBase * (*FactoryFuntion)(InterfaceBase *);

    ServiceMonitorManager();

    int rowCount(const QModelIndex &parent) const override
    {
        if (parent.column() > 0) {
            return 0;
        }
        auto count = facelift::ServiceRegistry::instance().objects().count();
        return count;
    }

    int columnCount(const QModelIndex &parent) const override
    {
        Q_UNUSED(parent);
        return 2;
    }

    QVariant data(const QModelIndex &index, int role) const override
    {
        Q_UNUSED(role);
        auto i = facelift::ServiceRegistry::instance().objects()[index.row()];
        if (index.column() == 1) {
            return i->implementationID();
        } else {
            return i->interfaceID();
        }
    }

    static ServiceMonitorManager &instance();

    void show();

    template<typename MonitorType>
    void registerMonitorType()
    {
        FactoryFuntion f = monitorFactory<MonitorType>;
        m_factories.insert(MonitorType::Provider_Type::FULLY_QUALIFIED_INTERFACE_NAME, f);
    }

    ServiceMonitorBase *createMonitor(InterfaceBase *object)
    {
        auto &interfaceID = object->interfaceID();
        if (m_factories.count(interfaceID) != 0) {
            return m_factories[interfaceID](object);
        } else {
            return nullptr;
        }
    }

    void refreshList();

    void onItemActivated(const QModelIndex &index);

private:
    void doShow();

    QMap<QString, FactoryFuntion> m_factories;
    Ui_ServiceMonitorManagerWindow *ui = nullptr;
    QMainWindow *m_window = nullptr;
};


class ServiceMonitorBase : public QObject
{
    Q_OBJECT

public:
    ServiceMonitorBase(QObject *provider) :
        QObject(provider)
    {
    }

    void init(InterfaceBase &service, const QString &interfaceName);

    virtual ~ServiceMonitorBase();

    void addWidget(PropertyWidgetBase &widget);

    void appendLog(QString textToAppend);

    QWidget *mainWidget() const
    {
        return m_window;
    }

private:
    Ui_ServiceMonitorPanel *ui = nullptr;
    QWidget *m_window = nullptr;
    bool m_oddWidget = true;
    QList<PropertyWidgetBase *> m_widgets;
    QString m_interfaceName;

};


template<typename ProviderType>
class ServiceMonitor : public ServiceMonitorBase
{

public:
    typedef ProviderType Provider_Type;

    ServiceMonitor(ProviderType &provider) : ServiceMonitorBase(&provider), m_provider(provider)
    {
        ServiceMonitorBase::init(provider, ProviderType::FULLY_QUALIFIED_INTERFACE_NAME);
    }

    template<typename PropertyType>
    typename TypeToWidget<PropertyType>::PanelType *addProperty(PropertyInterface<ProviderType, PropertyType> property,
            QString propertyName)
    {
        typedef typename TypeToWidget<PropertyType>::PanelType PanelType;
        auto widget = new PanelType(*new PropertyType(), propertyName);

        widget->init();
        widget->setValue(property.value());

        // Update the GUI if the value is changed in the property
        connect(&m_provider, property.signal, this, [property, widget]() {
                widget->setValue(property.value());
            });

        addWidget(*widget);

        return widget;
    }

    template<typename PropertyType, typename SetterFunction>
    typename TypeToWidget<PropertyType>::PanelType *addProperty(PropertyInterface<ProviderType, PropertyType> property, QString propertyName,
            SetterFunction setter)
    {
        auto widget = addProperty(property, propertyName);

        widget->enableEdition();
        connect(widget, &PropertyWidgetBase::valueChanged, this, [property, this, widget, propertyName, setter]() {
                qDebug() << "Value changed " << propertyName << " : " << facelift::toString(widget->value());
                if (!(property.value() == widget->value())) {
                    (m_provider.*setter)(widget->value());
                }
            });

        return widget;
    }

    template<typename PropertyType>
    void addProperty(ModelPropertyInterface<ProviderType, PropertyType> property, QString propertyName)
    {
        Q_UNUSED(property);
        Q_UNUSED(propertyName);
    }

    template<typename PropertyType>
    void addProperty(ServicePropertyInterface<ProviderType, PropertyType> property, QString propertyName)
    {
        Q_UNUSED(property);
        Q_UNUSED(propertyName);
    }

    template<typename PropertyType, typename SetterFunction>
    void addProperty(ServicePropertyInterface<ProviderType, PropertyType> property, QString propertyName, SetterFunction)
    {
        Q_UNUSED(property);
        Q_UNUSED(propertyName);
    }

    void addSignal();

    template<typename ... ParameterTypes>
    void logSignal(const QString signalName, void (ProviderType::*signal)(ParameterTypes ...))
    {
        QObject::connect(&m_provider, signal, this, [this, signalName] (ParameterTypes ... parameters) {
                QString argString;
                QTextStream s(&argString);
                generateToString(s, parameters ...);
                appendLog(QStringLiteral("signal: ") + signalName + "(" + argString + ")");
            });
    }

private:
    ProviderType &m_provider;

};


class ModuleMonitorBase
{
public:
    static bool isEnabled();

};

}
