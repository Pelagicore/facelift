/*
 *   This is part of the FaceLift project
 *   Copyright (C) 2017 Pelagicore AB
 *   SPDX-License-Identifier: LGPL-2.1
 *   This file is subject to the terms of the LGPL 2.1 license.
 *   Please see the LICENSE file for details.
 */

#include "DummyModel.h"
#include "ui_dummymodelsmainwindow.h"
#include "ui_dummymodelpanel.h"

#include <private/qqmlmetatype_p.h>


void DummyModelBase::init(const QString &interfaceName)
{
    m_interfaceName = interfaceName;

    m_window = new QWidget();
    ui = new Ui_DummyModelPanel();
    ui->setupUi(m_window);

    m_window->setWindowTitle(interfaceName);
    m_window->resize(600, 800);
    m_window->show();

    enableFileWatch();

    connect(&m_fileSystemWatcher, &QFileSystemWatcher::fileChanged, this,
            [this] () {
        loadJSONSnapshot();
        enableFileWatch();
    });

    bool success;
    auto settingsDoc = loadJSONFile(getSettingsFilePath(), success);
    if (success) {
        auto settingsObject = settingsDoc.object();
        m_autoSaveEnabled = settingsObject[AUTOSAVE_JSON_FIELD].toBool();
    }

    ui->autoSaveCheckBox->setChecked(m_autoSaveEnabled);

    ui->snapshotPath->setText(getJSONSnapshotFilePath());

    if (m_autoSaveEnabled) {
        loadJSONSnapshot();
    }

    QObject::connect(ui->saveSnapshotButton, &QPushButton::clicked, [this]() {
        saveJSONSnapshot();
    });

    QObject::connect(ui->loadSnapshotButton, &QPushButton::clicked, [this]() {
        loadJSONSnapshot();
    });

    QObject::connect(ui->clearLogButton, &QPushButton::clicked, [this]() {
        ui->logLabel->setText("");
    });

    QObject::connect(ui->autoSaveCheckBox, &QCheckBox::stateChanged, [this]() {
        m_autoSaveEnabled = ui->autoSaveCheckBox->isChecked();
        saveSettings();
    });

}


void DummyModelBase::addWidget(PropertyWidgetBase &widget)
{
    QPalette pal;
    m_oddWidget = !m_oddWidget;
    pal.setColor(QPalette::Background, m_oddWidget ? Qt::lightGray : Qt::gray);
    widget.setPalette(pal);
    ui->controlsLayout->addWidget(&widget);
    m_widgets.append(&widget);
}

void DummyModelBase::appendLog(QString textToAppend)
{
    QString text = ui->logLabel->toPlainText() + "\n" + textToAppend;
    ui->logLabel->setPlainText(text);
}


DummyModelBase::~DummyModelBase()
{
    delete(m_window);
}


DummyModelControlWindow::DummyModelControlWindow()
{
    ui = new Ui::DummyModelsMainWindow();
    ui->setupUi(this);
    resize(300, 400);
    show();
}

bool DummyModuleBase::isTypeRegistered(const QString &fullyQualifiedTypeName, int majorVersion, int minorVersion)
{
    return (QQmlMetaType::qmlType(fullyQualifiedTypeName, majorVersion, minorVersion) == nullptr);
}
