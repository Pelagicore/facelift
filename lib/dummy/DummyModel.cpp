/*
 *   This is part of the QMLCppAPI project
 *   Copyright (C) 2017 Pelagicore AB
 *   SPDX-License-Identifier: LGPL-2.1
 *   This file is subject to the terms of the LGPL 2.1 license.
 *   Please see the LICENSE file for details.
 */

#include "DummyModel.h"


void DummyModelBase::init(const QString &interfaceName)
{
    m_interfaceName = interfaceName;

    m_window = new QWidget();
    ui = new Ui_DummyModelPanel();
    ui->setupUi(m_window);

    m_window->setWindowTitle(interfaceName);
    m_window->resize(600, 800);
    m_window->show();

    m_fileSystemWatcher.addPath(interfaceName);

    bool success;
    auto settingsDoc = loadJSONFile(getSettingsFilePath(), success);
    if (success) {
        auto settingsObject = settingsDoc.object();
        m_autoSaveEnabled = settingsObject[AUTOSAVE_JSON_FIELD].toBool();
    }

    ui->autoSaveCheckBox->setChecked(m_autoSaveEnabled);

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
