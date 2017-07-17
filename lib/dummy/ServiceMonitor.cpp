/*
 *   This is part of the FaceLift project
 *   Copyright (C) 2017 Pelagicore AB
 *   SPDX-License-Identifier: LGPL-2.1
 *   This file is subject to the terms of the LGPL 2.1 license.
 */


#include "ServiceMonitor.h"

void ServiceMonitorBase::init(const QString &interfaceName)
{
    m_interfaceName = interfaceName;

    m_window = new QWidget();
    ui = new Ui_ServiceMonitorPanel();
    ui->setupUi(m_window);

    m_window->setWindowTitle(interfaceName + " - Monitor");
    m_window->resize(600, 800);
    m_window->show();

    QObject::connect(ui->clearLogButton, &QPushButton::clicked, [this]() {
        ui->logLabel->setText("");
    });

}

ServiceMonitorBase::~ServiceMonitorBase()
{
    delete(m_window);
}

bool ModuleMonitorBase::isEnabled() {
	return (getenv("FACELIFT_ENABLE_MONITOR") != nullptr);
}
