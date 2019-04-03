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

#include "DummyModel.h"
#include "ui_dummymodelsmainwindow.h"
#include "ui_dummymodelpanel.h"

#include <private/qqmlmetatype_p.h>

namespace facelift {

Q_LOGGING_CATEGORY(LogTools, "facelift.tools")

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

void DummyModelBase::appendLog(QString textToAppend) const
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

bool isDefined(QQmlType *p)
{
    return (p != nullptr);
}

bool isDefined(QQmlType p)
{
    return (p.isCreatable());
}

bool DummyModuleBase::isTypeRegistered(const QString &fullyQualifiedTypeName, int majorVersion, int minorVersion)
{
    return isDefined(QQmlMetaType::qmlType(fullyQualifiedTypeName, majorVersion, minorVersion));
}

}
