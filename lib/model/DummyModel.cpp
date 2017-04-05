/*
 *   This is part of the QMLCppAPI project
 *   Copyright (C) 2017 Pelagicore AB
 *   SPDX-License-Identifier: LGPL-2.1
 *   This file is subject to the terms of the LGPL 2.1 license.
 *   Please see the LICENSE file for details. 
 */

#include "DummyModel.h"

void DummyModelBase::init() {

    m_layout = new QVBoxLayout();
    m_window = new QWidget();
    m_window->setLayout(m_layout);

    m_logLabel = new QLabel(m_window);
    m_logLabel->setFixedSize(400, 150);
    m_layout->addWidget(m_logLabel);

    auto autoSaveWidget = new QWidget();
    auto autoSaveLayout = new QHBoxLayout();
    autoSaveWidget->setLayout(autoSaveLayout);

    auto autoSaveLabel = new QLabel("Auto save");
    autoSaveLayout->addWidget(autoSaveLabel);

    m_autoSaveCheckBox = new QCheckBox();
    autoSaveLayout->addWidget(m_autoSaveCheckBox);
    m_layout->addWidget(autoSaveWidget);

    m_saveSnapshotButton = new QPushButton("Save snapshot");
    m_layout->addWidget(m_saveSnapshotButton);

    m_loadSnapshotButton = new QPushButton("Load snapshot");
    m_layout->addWidget(m_loadSnapshotButton);

    m_clearLogButton = new QPushButton("Clear log");
    m_layout->addWidget(m_clearLogButton);

    m_window->resize(400, 400);
    m_window->show();
}


DummyModelBase::~DummyModelBase() {
    delete(m_window);
}
