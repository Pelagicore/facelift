/*
 *   Copyright (C) 2017 Pelagicore AG
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


class PropertyWidgetBase :
    public QWidget
{
    Q_OBJECT

public:
    PropertyWidgetBase(const QString &propertyName, QWidget *parent = nullptr) :
        QWidget(parent)
    {
        m_propertyName = propertyName;

        m_layout = new QHBoxLayout();
        setLayout(m_layout);

        m_propertyNameLabel = new QLabel();
        m_layout->addWidget(m_propertyNameLabel);
        setPropertyName(propertyName);
        setAutoFillBackground(true);
    }

    void addWidget(QWidget *widget)
    {
        m_layout->addWidget(widget);
    }

    void setPropertyName(const QString &propertyName)
    {
        m_propertyNameLabel->setText(propertyName);
    }

    const QString &propertyName() const
    {
        return m_propertyName;
    }

    Q_SIGNAL void valueChanged();

    virtual void refreshWidgetFromValue() = 0;

private:
    QHBoxLayout *m_layout;
    QLabel *m_propertyNameLabel;
    QString m_propertyName;
};


template<typename PropertyType>
class PropertyWidget :
    public PropertyWidgetBase
{

public:
    using PropertyWidgetBase::addWidget;
    using PropertyWidgetBase::valueChanged;

    PropertyWidget(PropertyType &value, const QString &propertyName, QWidget *parent = nullptr) : PropertyWidgetBase(propertyName,
                parent),
        m_value(value)
    {
    }

    void setValue(PropertyType value)
    {
        m_value = value;
        refreshWidgetFromValue();
    }

    PropertyType &value()
    {
        return m_value;
    }

protected:
    // called to update the value and trigger change signal
    void updateValue(PropertyType value)
    {
        m_value = value;
        valueChanged();
    }

private:
    PropertyType &m_value;

};

