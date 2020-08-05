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
#include <QWidget>
#include <QComboBox>
#include <QScrollArea>
#include <QMap>
#include <QList>

#include "FaceliftUtils.h"
#include "FaceliftModel.h"
#include "Structure.h"

namespace facelift {

Q_DECLARE_LOGGING_CATEGORY(LogTools)

class PropertyWidgetBase : public QWidget
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

    virtual void enableEdition()
    {
        Q_ASSERT(false);
    }

    Q_SIGNAL void valueChanged();

    virtual void refreshWidgetFromValue()
    {
        Q_ASSERT(false);
    }

private:
    QHBoxLayout *m_layout;
    QLabel *m_propertyNameLabel;
    QString m_propertyName;
};


template<typename PropertyType>
class PropertyWidget : public PropertyWidgetBase
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

    void init()
    {
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



template<typename EnumType>
class EnumerationPropertyWidget : public PropertyWidget<EnumType>
{

public:
    EnumerationPropertyWidget(EnumType &value, const QString &propertyName, QWidget *parent = nullptr) :
        PropertyWidget<EnumType>(value, propertyName, parent)
    {
        widget = new QComboBox();
        this->addWidget(widget);
        widget->setEnabled(false);
        auto values = validValues<EnumType>();
        for (auto &v : values) {
            widget->addItem(facelift::toString(v), static_cast<int>(v));
        }
    }

    void enableEdition() override
    {
        widget->setEnabled(true);
    }

    void refreshWidgetFromValue() override
    {
        int i = 0;
        for (auto e : validValues<EnumType>()) {
            if (this->value() == e) {
                widget->setCurrentIndex(i);
            }
            i++;
        }
    }

    void init()
    {
        QObject::connect(widget, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, [this](int) {
                int index = widget->currentIndex();
                this->updateValue(*(validValues<EnumType>().begin() + index));
            });
    }

private:
    QComboBox *widget = nullptr;
};


class BooleanPropertyWidget : public PropertyWidget<bool>
{

public:
    BooleanPropertyWidget(bool &value, const QString &propertyName, QWidget *parent = nullptr) :
        PropertyWidget(value, propertyName, parent)
    {
        widget = new QCheckBox();
        widget->setEnabled(false);
        addWidget(widget);
    }

    void enableEdition() override
    {
        widget->setEnabled(true);
    }

    void refreshWidgetFromValue() override
    {
        widget->setChecked(value());
    }

    void init()
    {
        QObject::connect(widget, &QCheckBox::stateChanged, this, [this]() {
                updateValue(widget->isChecked());
            });
    }

private:
    QCheckBox *widget = nullptr;
};


class IntegerPropertyWidget : public PropertyWidget<int>
{

public:
    IntegerPropertyWidget(int &value, const QString &propertyName, QWidget *parent = nullptr) :
        PropertyWidget(value, propertyName, parent)
    {
        widget = new QSpinBox();
        widget->setEnabled(false);
        widget->setMaximum(5000);
        this->addWidget(widget);
    }

    void enableEdition() override
    {
        widget->setEnabled(true);
    }

    void refreshWidgetFromValue() override
    {
        widget->setValue(value());
    }

    void init()
    {
        QObject::connect(widget, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), [this]() {
                updateValue(widget->value());
            });
    }

private:
    QSpinBox *widget = nullptr;
};

class DoublePropertyWidget : public PropertyWidget<double>
{

public:
    DoublePropertyWidget(double &value, const QString &propertyName, QWidget *parent = nullptr) :
        PropertyWidget(value, propertyName, parent)
    {
        widget = new QDoubleSpinBox();
        widget->setEnabled(false);
        widget->setMaximum(5000);
        widget->setSingleStep(0.1);
        addWidget(widget);
    }

    void enableEdition() override
    {
        widget->setEnabled(true);
    }

    void refreshWidgetFromValue() override
    {
        widget->setValue(value());
    }

    void init()
    {
        QObject::connect(widget, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), [this]() {
                updateValue(widget->value());
            });
    }

private:
    QDoubleSpinBox *widget = nullptr;
};


class StringPropertyWidget : public PropertyWidget<QString>
{

public:
    StringPropertyWidget(QString &value, const QString &propertyName, QWidget *parent = nullptr) :
        PropertyWidget(value, propertyName, parent)
    {
        widget = new QTextEdit();
        widget->setReadOnly(true);
        addWidget(widget);
    }

    void enableEdition() override
    {
        widget->setReadOnly(false);
    }

    void refreshWidgetFromValue() override
    {
        widget->setText(value());
    }

    void init()
    {
        QObject::connect(widget, &QTextEdit::textChanged, [this]() {
                updateValue(widget->toPlainText());
            });
    }

private:
    QTextEdit *widget = nullptr;
};



template<typename ElementType>
class MapPropertyWidget : public PropertyWidget<QMap<QString, ElementType> >
{

public:
    MapPropertyWidget(QMap<QString, ElementType> &value, const QString &propertyName, QWidget *parent = nullptr) :
        PropertyWidget<QMap<QString, ElementType> >(value, propertyName, parent)
    {
    }

    void enableEdition() override
    {
    }

    void refreshWidgetFromValue() override
    {
    }

    void setValueWidget(QWidget *widget)
    {
        this->addWidget(widget);
    }

    void init()
    {
        Q_ASSERT(false);
    }

};


template<typename ElementType>
class ListPropertyWidget : public PropertyWidget<QList<ElementType> >
{

public:
    ListPropertyWidget(QList<ElementType> &value, const QString &propertyName, QWidget *parent = nullptr) :
        PropertyWidget<QList<ElementType> >(value, propertyName, parent)
    {
        listContentWidget = new QTextEdit();
        this->addWidget(listContentWidget);
    }

    void enableEdition() override
    {
    }

    void refreshWidgetFromValue() override
    {
        listContentWidget->setText(facelift::toString(this->value()));
    }

    void setValueWidget(QWidget *widget)
    {
        this->addWidget(widget);
    }

    void addCreateNewElementButton()
    {
        createNewElementButton = new QPushButton("Create new element");
        this->addWidget(createNewElementButton);
    }

    QPushButton *createNewElementButton = nullptr;

    QTextEdit *listContentWidget = nullptr;

};


template<typename ElementType>
class SimpleListPropertyWidget : public ListPropertyWidget<ElementType>
{

public:
    SimpleListPropertyWidget(QList<ElementType> &value, const QString &propertyName, QWidget *parent = nullptr) :
        ListPropertyWidget<ElementType>(value, propertyName, parent)
    {
    }

    void init()
    {
    }

};


struct TypeToWidgetBase
{
    template<typename Type>
    static Type clone(const Type &v)
    {
        return v;
    }
};

template<typename Type, typename Sfinae = void>
struct TypeToWidget : public TypeToWidgetBase
{
    typedef PropertyWidget<Type> PanelType;
};


template<typename ListElementType>
struct TypeToWidget<QList<ListElementType> > : public TypeToWidgetBase
{
    typedef SimpleListPropertyWidget<ListElementType> PanelType;
};

template<typename ElementType>
struct TypeToWidget<QMap<QString, ElementType> > : public TypeToWidgetBase
{
    typedef MapPropertyWidget<ElementType> PanelType;
};

template<>
struct TypeToWidget<bool> : public TypeToWidgetBase
{
    typedef BooleanPropertyWidget PanelType;
};

template<>
struct TypeToWidget<int> : public TypeToWidgetBase
{
    typedef IntegerPropertyWidget PanelType;
};

template<>
struct TypeToWidget<double> : public TypeToWidgetBase
{
    typedef DoublePropertyWidget PanelType;
};

template<>
struct TypeToWidget<QString> : public TypeToWidgetBase
{
    typedef StringPropertyWidget PanelType;
};

template<typename EnumType>
struct TypeToWidget<EnumType, typename std::enable_if<std::is_enum<EnumType>::value>::type>
{
    typedef EnumerationPropertyWidget<EnumType> PanelType;

    static EnumType clone(const EnumType &v)
    {
        return v;
    }

};


template<typename StructType>
class StructurePropertyWidget : public PropertyWidget<StructType>
{
    typedef std::array<const char *, StructType::FieldCount> FieldNames;
    typedef typename StructType::FieldTupleTypes FieldTypes;

public:
    StructurePropertyWidget(StructType &value, const QString &propertyName, FieldNames fieldNames = StructType::FIELD_NAMES,
            QWidget *parent = nullptr) :
        PropertyWidget<StructType>(value, propertyName, parent)
    {
        widget = new QWidget();
        widget->setEnabled(false);
        m_layout = new QVBoxLayout();
        widget->setLayout(m_layout);
        this->addWidget(widget);
        create_widget_panel(fieldNames);
    }

    void enableEdition() override
    {
        widget->setEnabled(true);
        for (auto &panel : m_childPanels) {
            panel->enableEdition();
        }
    }

    template<std::size_t I = 0>
    inline typename std::enable_if<I == std::tuple_size<FieldTypes>::value>::type
    create_widget_panel(const FieldNames &fieldNames)
    {
        Q_UNUSED(fieldNames);
    }

    template<std::size_t I = 0>
    inline typename std::enable_if < I<std::tuple_size<FieldTypes>::value>::type
    create_widget_panel(const FieldNames &fieldNames)
    {
        typedef typename std::tuple_element<I, FieldTypes>::type FieldType;
        createPanelForField<FieldType, I>(fieldNames[I]);
        create_widget_panel<I + 1>(fieldNames);
    }

    template<typename FieldType, std::size_t I>
    void createPanelForField(const char *fieldName)
    {
        typedef typename TypeToWidget<FieldType>::PanelType PanelType;
        auto &fieldValue = std::get<I>(this->value().asTuple());
        auto widget = new PanelType(fieldValue, fieldName);
        widget->init();

        // forward value change signal to parent
        QObject::connect(widget, &PanelType::valueChanged, this, &PropertyWidgetBase::valueChanged);

        m_layout->addWidget(widget);
        m_childPanels.push_back(widget);
    }

    void refreshWidgetFromValue() override
    {
        for (const auto &widget: m_childPanels) {
            widget->refreshWidgetFromValue();
        }
    }

    void init()
    {
    }

    void add(PropertyWidgetBase *child)
    {
        m_layout->addWidget(child);
    }

private:
    QList<PropertyWidgetBase *> m_childPanels;
    QVBoxLayout *m_layout;

    QWidget *widget = nullptr;

};


template<typename StructType>
struct TypeToWidget<StructType, typename std::enable_if<std::is_base_of<StructureBase, StructType>::value>::type>
{
    typedef StructurePropertyWidget<StructType> PanelType;

    static StructType clone(const StructType &v)
    {
        return v.clone();
    }

};

}
