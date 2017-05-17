/*
 *   This is part of the QMLCppAPI project
 *   Copyright (C) 2017 Pelagicore AB
 *   SPDX-License-Identifier: LGPL-2.1
 *   This file is subject to the terms of the LGPL 2.1 license.
 *   Please see the LICENSE file for details.
 */

#pragma once

#include <QMainWindow>
#include <QApplication>
#include <QVBoxLayout>
#include <QLabel>
#include <QCheckBox>
#include <QSpinBox>
#include <QTextEdit>
#include <QPushButton>
#include <QPalette>
#include <QDir>
#include <QComboBox>
#include <QScrollArea>

#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>

#include "common/JSON.h"
#include "model/Model.h"
#include "property/Property.h"



class PropertyWidget : public QWidget {
public:

    typedef std::function<void()> ChangeListener;

    PropertyWidget(const QString& propertyName, QWidget* parent = nullptr) : QWidget(parent) {
        m_propertyName = propertyName;

        m_layout = new QHBoxLayout();
        setLayout(m_layout);

        m_propertyNameLabel = new QLabel();
        m_layout->addWidget(m_propertyNameLabel);
        setPropertyName(propertyName);
        setAutoFillBackground(true);
    }

    void setWidget(QWidget* widget) {
        m_layout->addWidget(widget);
    }

    void setPropertyName(const QString& propertyName) {
        m_propertyNameLabel->setText(propertyName);
    }

    const QString& propertyName() const {
        return m_propertyName;
    }

    void setListener(ChangeListener listener) {
        m_listener= listener;
    }

    QHBoxLayout *m_layout;
    QLabel *m_propertyNameLabel;
    ChangeListener m_listener;
    QString m_propertyName;
};


template <typename EnumType>
class EnumerationPropertyWidget : public PropertyWidget {

public:
    EnumerationPropertyWidget(const QString& propertyName, QWidget* parent = nullptr) : PropertyWidget(propertyName, parent) {
        widget = new QComboBox();
        setWidget(widget);
        auto values = validValues<EnumType>();
        for(auto& v:values) {
            widget->addItem(toString(v), static_cast<int>(v));
        }
    }

    EnumType value() const {
        int index = widget->currentIndex();
        return validValues<EnumType>()[index];
    }

    void init(EnumType initialValue) {

        auto values = validValues<EnumType>();
        for(int i = 0; i < values.size(); i++) {
            if (initialValue == values[i])
            widget->setCurrentIndex(i);
        }

        QObject::connect(widget, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, [this](int) {
            m_listener();
        });

    }

    QComboBox* widget = nullptr;
};

template <typename Type, typename Sfinae = void> struct DummyUIDesc {
    typedef PropertyWidget PanelType;
};


