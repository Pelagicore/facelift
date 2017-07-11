/*
 *   This is part of the QMLCppAPI project
 *   Copyright (C) 2017 Pelagicore AB
 *   SPDX-License-Identifier: LGPL-2.1
 *   This file is subject to the terms of the LGPL 2.1 license.
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

#include "Model.h"
#include "Property.h"

#include "ui_dummymodelsmainwindow.h"
#include "ui_dummymodelpanel.h"

#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>

#include <tuple>

#include <QFileSystemWatcher>

namespace detail {
template<typename Model, typename F, typename Tuple, bool Done, int Total, int ... N>
struct call_impl
{
    static void call_method(Model *obj, F f, Tuple &&t)
    {
        call_impl<Model, F, Tuple, Total == 1 + sizeof ... (N), Total, N ..., sizeof ... (N)>::call_method(obj, f,
                std::forward<Tuple>(t));
    }
};

template<typename Model, typename F, typename Tuple, int Total, int ... N>
struct call_impl<Model, F, Tuple, true, Total, N ...>
{
    static void call_method(Model *obj, F f, Tuple &&t)
    {
        (obj->*f)(std::get<N>(std::forward<Tuple>(t)) ...);
    }
};
}

// user invokes this
template<typename Model, typename F, typename Tuple>
void call_method(Model *obj, F f, Tuple &&t)
{
    typedef typename std::decay<Tuple>::type ttype;
    detail::call_impl<Model, F, Tuple, 0 == std::tuple_size<ttype>::value, std::tuple_size<ttype>::value>::call_method(
        obj, f, std::forward<Tuple>(t));
}


template<typename Type>
inline void readJSONSimple(const QJsonValue &json, Type &value)
{
    Q_UNUSED(value);
    Q_UNUSED(json);
    Q_ASSERT(false);
}

template<typename Type>
inline Type fromString(const QString &s)
{
    Q_UNUSED(s);
    Q_ASSERT(false);
    return {};
}

template<>
inline void readJSONSimple<int>(const QJsonValue &json, int &value)
{
    if (json.isDouble()) {
        value = json.toInt();
    }
}

template<>
inline void readJSONSimple<bool>(const QJsonValue &json, bool &value)
{
    if (json.isBool()) {
        value = json.toBool();
    } else {
        qFatal("Invalid data type");
    }
}

template<>
inline void readJSONSimple<float>(const QJsonValue &json, float &value)
{
    if (json.isDouble()) {
        value = json.toDouble();
    } else {
        qFatal("Invalid data type");
    }
}

template<>
inline void readJSONSimple<QString>(const QJsonValue &json, QString &value)
{
    if (json.isString()) {
        value = json.toString();
    } else {
        qFatal("Invalid data type");
    }
}


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




template<typename EnumType>
class EnumerationPropertyWidget :
    public PropertyWidget<EnumType>
{

public:
    EnumerationPropertyWidget(EnumType &value, const QString &propertyName, QWidget *parent = nullptr) :
        PropertyWidget<EnumType>(value, propertyName, parent)
    {
        widget = new QComboBox();
        this->addWidget(widget);
        auto values = validValues<EnumType>();
        for (auto &v : values) {
            widget->addItem(toString(v), static_cast<int>(v));
        }
    }

    void refreshWidgetFromValue() override
    {
        auto values = validValues<EnumType>();
        for (int i = 0; i < values.size(); i++) {
            if (this->value() == values[i]) {
                widget->setCurrentIndex(i);
            }
        }
    }

    void init()
    {
        QObject::connect(widget, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, [this](int) {
            int index = widget->currentIndex();
            this->updateValue(validValues<EnumType>()[index]);
        });
    }

private:
    QComboBox *widget = nullptr;
};


class BooleanPropertyWidget :
    public PropertyWidget<bool>
{

public:
    BooleanPropertyWidget(bool &value, const QString &propertyName, QWidget *parent = nullptr) :
        PropertyWidget(value, propertyName, parent)
    {
        widget = new QCheckBox();
        addWidget(widget);
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


class IntegerPropertyWidget :
    public PropertyWidget<int>
{

public:
    IntegerPropertyWidget(int &value, const QString &propertyName, QWidget *parent = nullptr) :
        PropertyWidget(value, propertyName, parent)
    {
        widget = new QSpinBox();
        widget->setMaximum(5000);
        this->addWidget(widget);
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

class FloatPropertyWidget :
    public PropertyWidget<float>
{

public:
    FloatPropertyWidget(float &value, const QString &propertyName, QWidget *parent = nullptr) :
        PropertyWidget(value, propertyName, parent)
    {
        widget = new QDoubleSpinBox();
        widget->setMaximum(5000);
        widget->setSingleStep(0.1);
        addWidget(widget);
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


class StringPropertyWidget :
    public PropertyWidget<QString>
{

public:
    StringPropertyWidget(QString &value, const QString &propertyName, QWidget *parent = nullptr) :
        PropertyWidget(value, propertyName, parent)
    {
        widget = new QTextEdit();
        addWidget(widget);
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
class ListPropertyWidget :
    public PropertyWidget<QList<ElementType> >
{

public:
    ListPropertyWidget(QList<ElementType> &value, const QString &propertyName, QWidget *parent = nullptr) :
        PropertyWidget<QList<ElementType> >(value, propertyName, parent)
    {
        addCreateNewElementButton();
    }

    void refreshWidgetFromValue() override
    {
        Q_ASSERT(false);
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

};


template<typename ElementType>
class SimpleListPropertyWidget :
    public ListPropertyWidget<ElementType>
{

public:
    SimpleListPropertyWidget(const QString &propertyName, QWidget *parent = nullptr) :
        ListPropertyWidget<ElementType>(propertyName, parent)
    {
    }

    void init()
    {
    }

    QList<ElementType> value() const
    {
        Q_ASSERT(false);
        return QList<ElementType>();
    }

};

struct DummyUIDescBase
{

    template<typename Type>
    static Type clone(const Type &v)
    {
        return v;
    }

};

template<typename Type, typename Sfinae = void>
struct DummyUIDesc :
    public DummyUIDescBase
{
    typedef PropertyWidget<Type> PanelType;
};


template<typename ListElementType>
struct DummyUIDesc<QList<ListElementType> > :
    public DummyUIDescBase
{
    typedef SimpleListPropertyWidget<ListElementType> PanelType;
};

template<>
struct DummyUIDesc<bool> :
    public DummyUIDescBase
{
    typedef BooleanPropertyWidget PanelType;
};

template<>
struct DummyUIDesc<int> :
    public DummyUIDescBase
{
    typedef IntegerPropertyWidget PanelType;
};

template<>
struct DummyUIDesc<float> :
    public DummyUIDescBase
{
    typedef FloatPropertyWidget PanelType;
};

template<>
struct DummyUIDesc<QString> :
    public DummyUIDescBase
{
    typedef StringPropertyWidget PanelType;
};

template<typename EnumType>
struct DummyUIDesc<EnumType, typename std::enable_if<std::is_enum<EnumType>::value>::type>
{
    typedef EnumerationPropertyWidget<EnumType> PanelType;
};


template<typename StructType>
class StructurePropertyWidget :
    public PropertyWidget<StructType>
{
    typedef std::array<const char *, StructType::FieldCount> FieldNames;
    typedef typename StructType::FieldTupleTypes FieldTypes;

public:
    StructurePropertyWidget(StructType &value, const QString &propertyName, FieldNames fieldNames = StructType::FIELD_NAMES,
            QWidget *parent = nullptr) :
        PropertyWidget<StructType>(value, propertyName, parent)
    {
        auto widget = new QWidget();
        m_layout = new QVBoxLayout();
        widget->setLayout(m_layout);
        this->addWidget(widget);
        create_widget_panel(fieldNames);
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
        typedef typename DummyUIDesc<FieldType>::PanelType PanelType;
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

};


template<typename StructType>
struct DummyUIDesc<StructType, typename std::enable_if<std::is_base_of<ModelStructure, StructType>::value>::type>
{
    typedef StructurePropertyWidget<StructType> PanelType;

    static StructType clone(const StructType &v)
    {
        return v.clone();
    }

};

template<typename Type>
inline void assignRandomValue(Type &t)
{
    t = {};
}

#include <random>
template<>
inline void assignRandomValue(int &t)
{
    static std::normal_distribution<double> normal_dist(653, 10);
    typedef std::mt19937 MyRNG;  // the Mersenne Twister with a popular choice of parameters
    MyRNG rng;
    t = normal_dist(rng);
}

template<>
inline void assignRandomValue(QString &t)
{
    t = "Random string";
}

template<std::size_t I = 0, typename ... Tp>
inline typename std::enable_if<I == sizeof ... (Tp), void>::type
random_tuple(std::tuple<Tp ...> &t)
{
    Q_UNUSED(t);
}

template<std::size_t I = 0, typename ... Tp>
inline typename std::enable_if < I<sizeof ... (Tp), void>::type
random_tuple(std::tuple<Tp ...> &t)
{
    assignRandomValue(std::get<I>(t));
    random_tuple<I + 1, Tp ...>(t);
}

template<typename ListElementType>
inline QWidget *createWidget(ListProperty<ListElementType> &t, const QString &propertyName)
{
    auto widget = new ListPropertyWidget<ListElementType>(propertyName);

    QObject::connect(widget->createNewElementButton, &QPushButton::clicked, [&t]() {
        typename ListElementType::FieldTupleTypes values;
        random_tuple(values);
        t.addElement(ListElementType(values));
    });

    return widget;
}

template<typename ElementType>
inline QTextStream &operator<<(QTextStream &outStream, const QList<ElementType> &f)
{
    Q_UNUSED(outStream);
    Q_UNUSED(f);
    //    outStream << toString(f);
    return outStream;
}


template<typename Type, typename Sfinae = void>
struct ToStringDesc
{

    static void appendToString(QTextStream &s, const Type &value)
    {
        s << value;
    }

};



template<typename Type, typename Enable = void>
struct DummyModelTypeHandler
{
    static void writeJSON(QJsonValue &json, const Type &value)
    {
        json = value;
    }

    static void readJSON(const QJsonValue &json, Type &value)
    {
        readJSONSimple(json, value);
    }
};

template<typename Type>
struct DummyModelTypeHandler<Type, typename std::enable_if<std::is_base_of<ModelStructure, Type>::value>::type>
{

    static void writeJSON(QJsonValue &json, const Type &value)
    {
        QJsonObject subObject;
        writeFieldsToJson(value.asTuple(), subObject);
        json = subObject;
    }

    template<std::size_t I = 0, typename ... Tp>
    static typename std::enable_if<I == sizeof ... (Tp), void>::type
    writeFieldsToJson(const std::tuple<Tp ...> &value, QJsonObject &jsonObject)
    {
        Q_UNUSED(value);
        Q_UNUSED(jsonObject);
    }

    template<std::size_t I = 0, typename ... Tp>
    static typename std::enable_if < I<sizeof ... (Tp), void>::type
    writeFieldsToJson(const std::tuple<Tp ...> &value, QJsonObject &jsonObject)
    {
        QJsonValue v;
        typedef typename std::tuple_element<I, typename Type::FieldTupleTypes>::type FieldType;
        DummyModelTypeHandler<FieldType>::writeJSON(v, std::get<I>(value));
        jsonObject[Type::FIELD_NAMES[I]] = v;
        writeFieldsToJson<I + 1, Tp ...>(value, jsonObject);
    }

    static void readJSON(const QJsonValue &json, Type &value)
    {
        QJsonObject subObject = json.toObject();
        readFieldsFromJson(value.asTuple(), subObject);
    }

    template<std::size_t I = 0, typename ... Tp>
    static typename std::enable_if<I == sizeof ... (Tp), void>::type
    readFieldsFromJson(std::tuple<Tp ...> &value, QJsonObject &jsonObject)
    {
        Q_UNUSED(value);
        Q_UNUSED(jsonObject);
    }

    template<std::size_t I = 0, typename ... Tp>
    static typename std::enable_if < I<sizeof ... (Tp), void>::type
    readFieldsFromJson(std::tuple<Tp ...> &value, QJsonObject &jsonObject)
    {
        typedef typename std::tuple_element<I, typename Type::FieldTupleTypes>::type FieldType;
        DummyModelTypeHandler<FieldType>::readJSON(jsonObject[Type::FIELD_NAMES[I]], std::get<I>(value));
        readFieldsFromJson<I + 1, Tp ...>(value, jsonObject);
    }

};

template<typename Type>
struct DummyModelTypeHandler<Type, typename std::enable_if<std::is_enum<Type>::value>::type>
{
    static void readJSON(const QJsonValue &json, Type &value)
    {
        QString s;
        readJSONSimple(json, s);
        assignFromString(s, value);
    }

    static void writeJSON(QJsonValue &json, const Type &value)
    {
        json = toString(value);
    }

};



class DummyModelBase :
    public QObject
{
    Q_OBJECT

    static constexpr const char *AUTOSAVE_JSON_FIELD = "autosave";

protected:
    DummyModelBase(QObject *parent) :
        QObject(parent)
    {
    }

    virtual ~DummyModelBase();

    void init(const QString &interfaceName);

    void enableFileWatch()
    {
        m_fileSystemWatcher.addPath(getJSONSnapshotFilePath());
    }

    QString getModelPersistenceFolder() const
    {
        auto path = QDir::currentPath() + QStringLiteral("/models/");
        return path;
    }

    QString getJSONSnapshotFilePath() const
    {
        QString path = getModelPersistenceFolder() + m_interfaceName + ".snapshot";
        return path;
    }

    QString getSettingsFilePath() const
    {
        QString path = getModelPersistenceFolder() + m_interfaceName + ".settings";
        return path;
    }

    QJsonDocument loadJSONFile(const QString &filePath, bool &success)
    {
        QFile jsonFile(filePath);
        success = jsonFile.open(QFile::ReadOnly);
        if (success) {
            QJsonDocument jsonDoc = QJsonDocument().fromJson(jsonFile.readAll());
            qDebug() << "Loaded JSON file " << jsonFile.fileName();
            return jsonDoc;
        } else {
            qWarning() << "Can't load JSON file " << jsonFile.fileName();
        }

        return QJsonDocument();
    }

    void saveSettings() const
    {
        QJsonObject settings;
        settings[AUTOSAVE_JSON_FIELD] = m_autoSaveEnabled;
        writeToFile(settings, getSettingsFilePath());
    }

    void writeToFile(const QJsonObject &jsonObject, const QString &filePath) const
    {
        QJsonDocument doc(jsonObject);
        QFile jsonFile(filePath);
        QDir folder = QFileInfo(filePath).absoluteDir();
        if (!folder.mkpath(folder.path())) {
            qWarning() << "Can't create folder " << folder;
        }

        auto success = jsonFile.open(QFile::WriteOnly);
        if (success) {
            jsonFile.write(doc.toJson());
            qDebug() << "JSON file written to " << jsonFile.fileName() << " : " << doc.toJson();
        } else {
            qWarning() << "Can't save JSON file " << jsonFile.fileName();
        }
    }

    virtual void saveJSONSnapshot() const = 0;

    virtual void loadJSONSnapshot() = 0;

    void addWidget(PropertyWidgetBase &widget)
    {
        QPalette pal;
        m_oddWidget = !m_oddWidget;
        pal.setColor(QPalette::Background, m_oddWidget ? Qt::lightGray : Qt::gray);
        widget.setPalette(pal);
        ui->controlsLayout->addWidget(&widget);
        m_widgets.append(&widget);
    }

    void onPropertyValueChanged()
    {
        if (m_autoSaveEnabled) {
            saveJSONSnapshot();
        }
    }

    void appendLog(QString textToAppend)
    {
        QString text = ui->logLabel->toPlainText() + "\n" + textToAppend;
        ui->logLabel->setPlainText(text);
    }

private:
    Ui_DummyModelPanel *ui = nullptr;

    QWidget *m_window = nullptr;
    bool m_autoSaveEnabled = true;

    bool m_oddWidget = true;

    QList<PropertyWidgetBase *> m_widgets;

    QFileSystemWatcher m_fileSystemWatcher;

    QString m_interfaceName;

};


class DummyModelControlWindow :
    public QMainWindow
{

public:
    DummyModelControlWindow();

    static void addModel(DummyModelBase &model)
    {
        Q_UNUSED(model);
    }

    static DummyModelControlWindow &instance()
    {
        static DummyModelControlWindow singleton;
        return singleton;
    }


private:
    Ui::DummyModelsMainWindow *ui;
};


/**
 * Abstract class for dummy models
 */
template<typename TypeName>
class DummyModel :
    public DummyModelBase
{

public:
    DummyModel(QObject *parent = nullptr) :
        DummyModelBase(parent)
    {
    }

    void init()
    {
        DummyModelBase::init(TypeName::FULLY_QUALIFIED_INTERFACE_NAME);
    }

    void finishInit()
    {
        DummyModelControlWindow::instance().addModel(*this);
    }

    virtual void savePropertyValues(QJsonObject &jsonObject) const = 0;

    virtual void loadPropertyValues(const QJsonObject &jsonObject) = 0;

    void saveJSONSnapshot() const override
    {
        QJsonObject jsonObject;
        savePropertyValues(jsonObject);
        writeToFile(jsonObject, getJSONSnapshotFilePath());
    }

    void loadJSONSnapshot() override
    {
        bool success;
        auto jsonDoc = loadJSONFile(getJSONSnapshotFilePath(), success);
        if (success) {
            loadPropertyValues(jsonDoc.object());
        }
    }

    template<typename ElementType>
    void writeJSONProperty(QJsonObject &json, const Property<ElementType> &property, const char *propertyName) const
    {
        QJsonValue jsonValue;
        DummyModelTypeHandler<ElementType>::writeJSON(jsonValue, property.value());
        json[propertyName] = jsonValue;
    }

    template<typename ListElementType>
    QJsonArray toJsonArray(const QList<ListElementType> &list) const
    {
        QJsonArray array;
        for (auto &propertyElement : list) {
            QJsonValue jsonValue;
            DummyModelTypeHandler<ListElementType>::writeJSON(jsonValue, propertyElement);
            array.append(jsonValue);
        }
        return array;
    }


    template<typename ListElementType>
    void writeJSONProperty(QJsonObject &json, const ListProperty<ListElementType> &property,
            const char *propertyName) const
    {
        json[propertyName] = toJsonArray(property.value());
    }


    template<typename ListElementType>
    void writeJSONProperty(QJsonObject &json, const ModelProperty<ListElementType> &property,
            const char *propertyName) const
    {
        json[propertyName] = toJsonArray(property.asList());
    }

    template<typename ElementType>
    void readJSONProperty(const QJsonObject &json, Property<ElementType> &property, const char *propertyName) const
    {
        ElementType v = {};
        DummyModelTypeHandler<ElementType>::readJSON(json[propertyName], v);
        property = v;
    }

    template<typename ListElementType>
    QList<ListElementType> readJSONArray(const QJsonValue &jsonValue) const
    {
        QList<ListElementType> elements;

        Q_ASSERT(jsonValue.isArray());
        auto jsonArray = jsonValue.toArray();
        auto size = jsonArray.size();
        for (int i = 0; i < size; i++) {
            ListElementType e;
            DummyModelTypeHandler<ListElementType>::readJSON(jsonArray[i], e);
            elements.append(e);
        }

        return elements;
    }

    template<typename ListElementType>
    void readJSONProperty(const QJsonObject &json, ListProperty<ListElementType> &property, const char *propertyName) const
    {
        auto jsonValue = json[propertyName];
        if (jsonValue.isArray()) {
            auto elements = readJSONArray<ListElementType>(jsonValue);
            property.setValue(elements);
        } else {
            qWarning() << "Expected array in property " << propertyName;
        }
    }

    template<typename ListElementType>
    void readJSONProperty(const QJsonObject &json, ModelProperty<ListElementType> &property, const char *propertyName) const
    {
        auto jsonValue = json[propertyName];
        if (jsonValue.isArray()) {
            auto elements = readJSONArray<ListElementType>(jsonValue);
            property.setElements(elements);
        } else {
            qWarning() << "Expected array in property " << propertyName;
        }
    }


    template<typename ... ParameterTypes>
    void initSignal(QString signalName,
            const std::array<const char *, sizeof ... (ParameterTypes)> &parameterNames, TypeName *obj,
            void (TypeName::*signalPointer)(ParameterTypes ...))
    {
        typedef TModelStructure<ParameterTypes ...> SignalParametersStruct;

        // Create a new structure type containing the signal arguments
        typedef StructurePropertyWidget<SignalParametersStruct> PanelType;

        // Create a panel for that structure
        auto widget = new PanelType(*(new SignalParametersStruct()), signalName, parameterNames);
        widget->init();

        // And add the trigger button
        auto triggerButton = new QPushButton("Trigger signal");
        widget->addWidget(triggerButton);

        QObject::connect(triggerButton, &QPushButton::clicked, this, [ = ]() {
            // Trigger signal using the parameters of the structure
            call_method(obj, signalPointer, widget->value().asTuple());
        });

        addWidget(*widget);

    }

    //    template <typename PropertyType>
    void addPropertyWidget(PropertyBase &property, PropertyWidgetBase &widget)
    {
        qDebug() << "Added property widget" << property.name();
        addWidget(widget);

        PropertyConnector<TypeName>::connect(property, this, [this] () {
            onPropertyValueChanged();
        });
    }


    template<typename PropertyType>
    void initWidget(Property<PropertyType> &property, const QString &propertyName)
    {
        Q_UNUSED(property);

        typedef typename DummyUIDesc<PropertyType>::PanelType PanelType;
        auto widget = new PanelType(*new PropertyType(), propertyName);

        widget->init();
        widget->setValue(property.value());

        // Update our property if the value is changed with the GUI
        connect(widget, &PanelType::valueChanged, this, [&property, widget]() {
            property = widget->value();
        });

        // Update the GUI if the value is changed in the property
        PropertyConnector<TypeName>::connect(property, this, [&property, widget]() {
            widget->setValue(property.value());
        });

        widget->setValue(property.value());

        addPropertyWidget(property, *widget);
    }

    template<typename ListElementType>
    void initWidget(ModelProperty<ListElementType> &property, const QString &propertyName)
    {
        auto widget = new ListPropertyWidget<ListElementType>(*(new QList<ListElementType>()), propertyName);

        typedef typename DummyUIDesc<ListElementType>::PanelType ElementPanelType;
        auto widgetForNewElement = new ElementPanelType(*new ListElementType(), "New");
        widgetForNewElement->init();
        widget->setValueWidget(widgetForNewElement);

        QObject::connect(widget->createNewElementButton, &QPushButton::clicked, [&property, widgetForNewElement]() {
            auto v = property.asList();
            ListElementType clone = DummyUIDesc<ListElementType>::clone(widgetForNewElement->value());         // ensure structs are cloned to get a new ID
            v.append(clone);
            property.setElements(v);
        });

        addPropertyWidget(property, *widget);
    }


    template<typename ListElementType>
    void initWidget(ListProperty<ListElementType> &property, const QString &propertyName)
    {
        auto widget = new ListPropertyWidget<ListElementType>(*(new QList<ListElementType>()), propertyName);

        typedef typename DummyUIDesc<ListElementType>::PanelType ElementPanelType;
        auto widgetForNewElement = new ElementPanelType(*new ListElementType(), "New");
        widgetForNewElement->init();
        widget->setValueWidget(widgetForNewElement);

        QObject::connect(widget->createNewElementButton, &QPushButton::clicked, [&property, widgetForNewElement]() {
            auto v = property.value();
            ListElementType clone = DummyUIDesc<ListElementType>::clone(widgetForNewElement->value());         // ensure structs are cloned to get a new ID
            v.append(clone);
            property.setValue(v);
        });

        addPropertyWidget(property, *widget);
    }

    void generateToString(QTextStream &message)
    {
        Q_UNUSED(message);
    }

    template<typename FirstParameterTypes, typename ... ParameterTypes>
    void generateToString(QTextStream &message, const FirstParameterTypes &firstParameter, const ParameterTypes & ... parameters)
    {
        ToStringDesc<FirstParameterTypes>::appendToString(message, firstParameter);
        generateToString(message, parameters ...);
    }

    template<typename ... ParameterTypes>
    void logMethodCall(const QString methodName, const ParameterTypes & ... parameters)
    {
        QString argString;
        QTextStream s(&argString);
        generateToString(s, parameters ...);

        appendLog(methodName + " called with args : " + argString);
    }

    template<typename ParameterType>
    void logSetterCall(const QString propertyName, const ParameterType &value)
    {
        appendLog(propertyName + " setter called with value : " + toString(value));
    }

};
