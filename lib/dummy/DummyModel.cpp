/*
 *   This is part of the QMLCppAPI project
 *   Copyright (C) 2017 Pelagicore AB
 *   SPDX-License-Identifier: LGPL-2.1
 *   This file is subject to the terms of the LGPL 2.1 license.
 *   Please see the LICENSE file for details.
 */

#include "DummyModel.h"


void DummyModelBase::initUi()
{
    m_window = new QWidget();
    ui = new Ui_DummyModelPanel();
    ui->setupUi(m_window);

    m_window->resize(600, 800);
    m_window->show();

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
