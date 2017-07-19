/*
 *   This is part of the FaceLift project
 *   Copyright (C) 2017 Pelagicore AB
 *   SPDX-License-Identifier: LGPL-2.1
 *   This file is subject to the terms of the LGPL 2.1 license.
 */

#pragma once

#include <QObject>
#include <QMap>

#include "ControlWidgets.h"


class ServiceMonitorBase;

template<typename Type>
ServiceMonitorBase *monitorFactory(InterfaceBase *provider)
{
    typedef typename Type::ProviderType_ ProviderType;
    auto p = qobject_cast<ProviderType *>(provider);
    return new Type(*p);
}

class ServiceMonitorManager :
    public QObject
{

    Q_OBJECT

public:
    typedef ServiceMonitorBase * (*FactoryFuntion)(InterfaceBase *);

    ServiceMonitorManager()
    {
        connect(&QMLCppApi::ServiceRegistry::instance(), &QMLCppApi::ServiceRegistry::objectRegistered, this,
                &ServiceMonitorManager::onObjectRegistered, Qt::DirectConnection);
    }

    static ServiceMonitorManager &instance()
    {
        static ServiceMonitorManager manager;
        return manager;
    }

    template<typename MonitorType>
    void registerMonitorType()
    {
        FactoryFuntion f = monitorFactory<MonitorType>;
        m_factories.insert(MonitorType::ProviderType_::FULLY_QUALIFIED_INTERFACE_NAME, f);
    }

    void onObjectRegistered(InterfaceBase *object)
    {
        auto interfaceID = object->interfaceID();
        if (m_factories.count(interfaceID) != 0) {
            m_factories[interfaceID](object);
        }
    }

private:
    QMap<QString, FactoryFuntion> m_factories;

};


class Ui_ServiceMonitorPanel;

class ServiceMonitorBase :
    public QObject
{

    Q_OBJECT

public:
    ServiceMonitorBase(QObject *provider) :
        QObject(provider)
    {
    }

    void init(const QString &interfaceName);

    virtual ~ServiceMonitorBase();

    void addWidget(PropertyWidgetBase &widget);

private:
    Ui_ServiceMonitorPanel *ui = nullptr;
    QWidget *m_window = nullptr;

    bool m_oddWidget = true;

    QList<PropertyWidgetBase *> m_widgets;

    QString m_interfaceName;

};


template<typename ProviderType>
class ServiceMonitor :
    public ServiceMonitorBase
{

public:
    typedef ProviderType ProviderType_;

    ServiceMonitor(ProviderType &provider) : ServiceMonitorBase(&provider), m_provider(provider)
    {
        ServiceMonitorBase::init(ProviderType::FULLY_QUALIFIED_INTERFACE_NAME);
    }

    template<typename PropertyType>
    void addProperty(PropertyInterface<ProviderType, PropertyType> property, QString propertyName)
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

private:
    ProviderType &m_provider;

};


class ModuleMonitorBase {

public:

	static bool isEnabled();

};
