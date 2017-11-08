/*
 *   This is part of the FaceLift project
 *   Copyright (C) 2017 Pelagicore AB
 *   SPDX-License-Identifier: LGPL-2.1
 *   This file is subject to the terms of the LGPL 2.1 license.
 */


#include "ServiceMonitor.h"
#include "ui_servicemonitorpanel.h"
#include "ui_servicemonitormanagerwindow.h"

#include <QRegExp>

namespace facelift {

void ServiceMonitorBase::init(InterfaceBase &service, const QString &interfaceName)
{
    m_interfaceName = interfaceName;

    m_window = new QWidget();
    ui = new Ui_ServiceMonitorPanel();
    ui->setupUi(m_window);

    m_window->setWindowTitle(interfaceName + " - Monitor - " + service.implementationID());

    QObject::connect(ui->clearLogButton, &QPushButton::clicked, this, [this]() {
            ui->logLabel->setText("");
        });

    ui->readyCheckBox->setChecked(service.ready());
    QObject::connect(&service, &InterfaceBase::readyChanged, this, [this, &service]() {
            ui->readyCheckBox->setChecked(service.ready());
        });

}

ServiceMonitorBase::~ServiceMonitorBase()
{
    delete(m_window);
}


void ServiceMonitorBase::addWidget(PropertyWidgetBase &widget)
{
    QPalette pal;
    m_oddWidget = !m_oddWidget;
    pal.setColor(QPalette::Background, m_oddWidget ? Qt::lightGray : Qt::gray);
    widget.setPalette(pal);
    ui->controlsLayout->addWidget(&widget);
    m_widgets.append(&widget);
}

void ServiceMonitorBase::appendLog(QString textToAppend)
{
    QString text = ui->logLabel->toPlainText() + "\n" + textToAppend;
    ui->logLabel->setPlainText(text);
}

bool ModuleMonitorBase::isEnabled()
{
    return true;
    auto v = getenv("FACELIFT_ENABLE_MONITOR");
    return ((v != nullptr) && (strcmp(v, "1") == 0));
}

ServiceMonitorManager::ServiceMonitorManager()
{
    // Our object might be instantiated from another thread so we move it back to the main thread
    moveToThread(QApplication::instance()->thread());
}

void ServiceMonitorManager::show()
{
    QTimer::singleShot(0, Qt::TimerType::CoarseTimer, this, &ServiceMonitorManager::doShow);
}

void ServiceMonitorManager::doShow()
{
    m_window = new QMainWindow();
    ui = new Ui_ServiceMonitorManagerWindow();
    ui->setupUi(m_window);

    m_window->setWindowTitle("Service monitor manager");
    m_window->resize(600, 800);
    m_window->show();

    connect(ui->interfaceList, &QListView::activated, this, &ServiceMonitorManager::onItemActivated);

    connect(&facelift::ServiceRegistry::instance(), &facelift::ServiceRegistry::objectRegistered, this,
            &ServiceMonitorManager::refreshList, Qt::DirectConnection);
    connect(&facelift::ServiceRegistry::instance(), &facelift::ServiceRegistry::objectDeregistered, this,
            &ServiceMonitorManager::refreshList, Qt::DirectConnection);

    ui->interfaceList->setModel(this);

}


void ServiceMonitorManager::onItemActivated(const QModelIndex &index)
{
    int rowIdex = index.row();
    auto s = facelift::ServiceRegistry::instance().objects()[rowIdex];
    createMonitor(s);
}


void ServiceMonitorManager::refreshList()
{
    beginResetModel();
    endResetModel();
}


ServiceMonitorManager &ServiceMonitorManager::instance()
{
    static ServiceMonitorManager manager;
    return manager;
}


}
