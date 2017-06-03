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

#include "DummyModelCommon.h"

#include "ui_dummymodelsmainwindow.h"
#include "ui_dummymodelpanel.h"


class BooleanPropertyWidget :
    public PropertyWidget
{

public:
    BooleanPropertyWidget(const QString &propertyName, QWidget *parent = nullptr) :
        PropertyWidget(propertyName, parent)
    {
        widget = new QCheckBox();
        setWidget(widget);
    }

    bool value() const
    {
        return widget->isChecked();
    }

    void init(bool initialValue)
    {
        widget->setChecked(initialValue);
        QObject::connect(widget, &QCheckBox::stateChanged, this, [this]() {
                    m_listener();
                });
    }

    QCheckBox *widget = nullptr;
};


class IntegerPropertyWidget :
    public PropertyWidget
{

public:
    IntegerPropertyWidget(const QString &propertyName, QWidget *parent = nullptr) :
        PropertyWidget(propertyName, parent)
    {
        widget = new QSpinBox();
        setWidget(widget);
    }

    int value() const
    {
        return widget->value();
    }

    void init(int initialValue)
    {
        widget->setValue(initialValue);
        QObject::connect(widget, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), [this]() {
                    m_listener();
                });
    }

    QSpinBox *widget = nullptr;
};


class StringPropertyWidget :
    public PropertyWidget
{

public:
    StringPropertyWidget(const QString &propertyName, QWidget *parent = nullptr) :
        PropertyWidget(propertyName, parent)
    {
        widget = new QTextEdit();
        setWidget(widget);
    }

    QString value() const
    {
        return widget->toPlainText();
    }

    void init(const QString &initialValue)
    {
        widget->setText(initialValue);
        QObject::connect(widget, &QTextEdit::textChanged, [this]() {
                    m_listener();
                });
    }

    QTextEdit *widget = nullptr;
};


template<typename ElementType>
class ListPropertyWidget :
    public PropertyWidget
{

public:
    ListPropertyWidget(const QString &propertyName, QWidget *parent = nullptr) :
        PropertyWidget(propertyName, parent)
    {
        createNewElementButton = new QPushButton("Create new element");
        setWidget(createNewElementButton);
    }

    void setValueWidget(QWidget *widget)
    {
        m_layout->addWidget(widget);
    }

    QPushButton *createNewElementButton;

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

    void init(const QList<ElementType> &initialValue)
    {
        //        Q_ASSERT(false);
        Q_UNUSED(initialValue);
    }

    QList<ElementType> value() const
    {
        Q_ASSERT(false);
        return QList<ElementType>();
    }

};

template<typename ListElementType>
struct DummyUIDesc<QList<ListElementType> >
{
    typedef SimpleListPropertyWidget<ListElementType> PanelType;
};

template<>
struct DummyUIDesc<bool>
{
    typedef BooleanPropertyWidget PanelType;
};

template<>
struct DummyUIDesc<int>
{
    typedef IntegerPropertyWidget PanelType;
};

template<>
struct DummyUIDesc<float>
{
    typedef IntegerPropertyWidget PanelType;
};

template<>
struct DummyUIDesc<QString>
{
    typedef StringPropertyWidget PanelType;
};

template<typename StructType>
class StructurePropertyWidget :
    public PropertyWidget
{

    typedef std::array<const char *, StructType::FieldCount> FieldNames;

public:
    StructurePropertyWidget(const QString &propertyName, QWidget *parent = nullptr) :
        PropertyWidget(propertyName, parent)
    {
        auto widget = new QWidget();
        m_layout = new QVBoxLayout();
        widget->setLayout(m_layout);
        setWidget(widget);
    }

    template<std::size_t I = 0, typename ... Tp>
    inline typename std::enable_if<I == sizeof ... (Tp), void>::type
    create_widget_panel(std::tuple<Tp ...> &t, const FieldNames &fieldNames)
    {
        Q_UNUSED(t);
        Q_UNUSED(fieldNames);
    }

    template<std::size_t I = 0, typename ... Tp>
    inline typename std::enable_if < I<sizeof ... (Tp), void>::type
    create_widget_panel(std::tuple<Tp ...> &t, const FieldNames &fieldNames)
    {
        createPanelForField(std::get<I>(t), fieldNames[I]);
        create_widget_panel<I + 1, Tp ...>(t, fieldNames);
    }

    template<typename FieldType>
    void createPanelForField(FieldType &v, const char *fieldName)
    {

        typedef typename DummyUIDesc<FieldType>::PanelType PanelType;
        auto widget = new PanelType(fieldName);

        widget->init(v);
        widget->setListener([this, widget, &v]() {
                    v = widget->value();
                    if (m_listener) {
                        m_listener();
                    }
                });

        if (widget != nullptr) {
            m_layout->addWidget(widget);
        }
        Q_UNUSED(v);
    }

    void init(const StructType &initialValue = StructType(), FieldNames fieldNames = StructType::FIELD_NAMES)
    {
        m_fieldValues = initialValue.asTuple();
        create_widget_panel(m_fieldValues, fieldNames);
    }

    void add(PropertyWidget *child)
    {
        m_layout->addWidget(child);
    }

    const StructType &value()
    {
        m_value.setValue(m_fieldValues);
        return m_value;
    }

    typename StructType::FieldTupleTypes m_fieldValues;
    StructType m_value;

    QVBoxLayout *m_layout;
};


template<typename StructType>
struct DummyUIDesc<StructType, typename std::enable_if<std::is_base_of<ModelStructure, StructType>::value>::type>
{
    typedef StructurePropertyWidget<StructType> PanelType;
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
inline QWidget *createWidget(StructListProperty<ListElementType> &t, const QString &propertyName)
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


class DummyModelBase :
    public QObject
{

    Q_OBJECT

protected:
    DummyModelBase(QObject *parent) :
        QObject(parent)
    {
    }

    virtual ~DummyModelBase();

    void initUi();

    Ui_DummyModelPanel *ui = nullptr;

    QWidget *m_window;
    bool m_autoSaveEnabled = true;

    bool m_oddWidget = true;

    QList<PropertyWidget *> m_widgets;

};


class DummyModelControlWindow :
    public QMainWindow
{

public:
    DummyModelControlWindow();

    static void addModel(DummyModelBase &model)
    {
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
        initUi();
        m_window->setWindowTitle(TypeName::INTERFACE_NAME);

        loadSettings();
        if (m_autoSaveEnabled) {
            loadJSONSnapshot();
        }

        ui->autoSaveCheckBox->setChecked(m_autoSaveEnabled);

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

    void finishInit()
    {
        DummyModelControlWindow::instance().addModel(*this);
    }

    void addWidget(PropertyWidget &widget)
    {
        QPalette pal;
        m_oddWidget = !m_oddWidget;
        pal.setColor(QPalette::Background, m_oddWidget ? Qt::lightGray : Qt::gray);
        widget.setPalette(pal);
        ui->controlsLayout->addWidget(&widget);
        m_widgets.append(&widget);
    }

    QString getModelPersistenceFolder() const
    {
        auto path = QDir::currentPath() + QStringLiteral("/models/");
        return path;
    }

    QString getJSONSnapshotFilePath() const
    {
        QString path = getModelPersistenceFolder() + TypeName::INTERFACE_NAME + ".snapshot";
        return path;
    }

    QString getSettingsFilePath() const
    {
        QString path = getModelPersistenceFolder() + TypeName::INTERFACE_NAME + ".settings";
        return path;
    }

    virtual void writeJsonValues(QJsonObject &jsonObject) const = 0;

    virtual void loadJsonValues(const QJsonObject &jsonObject) = 0;

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
            qDebug() << "JSON snapshot written to " << jsonFile.fileName() << " : " << doc.toJson();
        } else {
            qWarning() << "Can't save JSON snapshot to file " << jsonFile.fileName();
        }
    }

    void saveJSONSnapshot() const
    {
        QJsonObject jsonObject;
        writeJsonValues(jsonObject);
        writeToFile(jsonObject, getJSONSnapshotFilePath());
    }

    void loadJSONSnapshot()
    {
        bool success;
        auto jsonDoc = loadJSONFile(getJSONSnapshotFilePath(), success);
        if (success) {
            loadJsonValues(jsonDoc.object());
        }
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
            qWarning() << "Can't load JSON snapshot to file " << jsonFile.fileName();
        }

        return QJsonDocument();
    }

    static constexpr const char *AUTOSAVE_JSON_FIELD = "autosave";

    void loadSettings()
    {
        bool success;
        auto settingsDoc = loadJSONFile(getSettingsFilePath(), success);
        if (success) {
            auto settingsObject = settingsDoc.object();
            m_autoSaveEnabled = settingsObject[AUTOSAVE_JSON_FIELD].toBool();
        }
    }

    void saveSettings() const
    {
        QJsonObject settings;
        settings[AUTOSAVE_JSON_FIELD] = m_autoSaveEnabled;
        writeToFile(settings, getSettingsFilePath());
    }

    template<typename ElementType>
    void writeJSONProperty(QJsonObject &json, const Property<ElementType> &property, const char *propertyName) const
    {
        QJsonValue jsonValue;
        writeJSON(jsonValue, property.value());
        json[propertyName] = jsonValue;
    }

    template<typename ListElementType>
    void writeJSONProperty(QJsonObject &json, const StructListProperty<ListElementType> &property,
            const char *propertyName) const
    {
        QJsonArray array;

        for (auto &propertyElement : property.value()) {
            QJsonValue jsonValue;
            writeJSON(jsonValue, propertyElement);
            array.append(jsonValue);
        }

        json[propertyName] = array;
    }


    template<typename ListElementType>
    void writeJSONProperty(QJsonObject &json, const ModelProperty<ListElementType> &property,
            const char *propertyName) const
    {
        QJsonArray array;

        Q_ASSERT(false);
        // TODO


    }

    template<typename ListElementType>
    void writeJSONProperty(QJsonObject &json, const SimpleTypeListProperty<ListElementType> &property,
            const char *propertyName) const
    {
        QJsonArray array;

        for (auto &propertyElement : property.value()) {
            QJsonValue jsonValue;
            writeJSON(jsonValue, propertyElement);
            array.append(jsonValue);
        }

        json[propertyName] = array;
    }

    template<typename ElementType>
    void readJSONProperty(const QJsonObject &json, Property<ElementType> &property, const char *propertyName) const
    {
        ElementType v = {};
        readJSON(json[propertyName], v);
        property = v;
    }

    template<typename ListElementType>
    void readJSONProperty(const QJsonObject &json, StructListProperty<ListElementType> &property, const char *propertyName) const
    {
        if (json[propertyName].isArray()) {
            QList<ListElementType> elements;
            auto jsonArray = json[propertyName].toArray();
            auto size = jsonArray.size();

            for (int i = 0; i < size; i++) {
                ListElementType e;
                readJSON(jsonArray[i], e);
                elements.append(e);
            }

            property.setValue(elements);
        } else {
            qWarning() << "Expected array in property " << propertyName;
        }
    }

    template<typename ListElementType>
    void readJSONProperty(const QJsonObject &json, ModelProperty<ListElementType> &property, const char *propertyName) const
    {
        Q_ASSERT(false);
        // TODO
    }

    template<typename ListElementType>
    void readJSONProperty(const QJsonObject &json, SimpleTypeListProperty<ListElementType> &property,
            const char *propertyName) const
    {
        if (json[propertyName].isArray()) {
            QList<ListElementType> elements;
            auto jsonArray = json[propertyName].toArray();
            auto size = jsonArray.size();

            for (int i = 0; i < size; i++) {
                ListElementType e = {};
                readJSON(jsonArray[i], e);
                elements.append(e);
            }

            property.setValue(elements);
        } else {
            qWarning() << "Expected array in property " << propertyName;
        }
    }

    template<typename ... ParameterTypes>
    void initSignal(QString signalName, const std::array<const char *, sizeof ... (ParameterTypes)> &parameterNames,
            void (TypeName::*signalPointer)(ParameterTypes ...))
    {
        typedef TModelStructure<ParameterTypes ...> SignalParametersStruct;
        SignalParametersStruct s;

        typedef typename DummyUIDesc<SignalParametersStruct>::PanelType PanelType;
        auto widget = new PanelType(signalName);

        widget->init(s, parameterNames);
        addWidget(*widget);

    }

    template<typename PropertyType>
    void initWidget(Property<PropertyType> &property, const QString &propertyName)
    {

        Q_UNUSED(property);

        typedef typename DummyUIDesc<PropertyType>::PanelType PanelType;
        auto widget = new PanelType(propertyName);

        widget->init(property.value());
        widget->setListener([&property, widget]() {
                    property = widget->value();
                });

        connect(property.owner(), property.signal(), this, [this]() {
                    if (m_autoSaveEnabled) {
                        saveJSONSnapshot();
                    }
                });

        addWidget(*widget);
    }

    template<typename ListElementType>
    void initWidget(ModelProperty<ListElementType> &property, const QString &propertyName)
    {
        Q_ASSERT(false);
        // TODO
    }

    template<typename ListElementType>
    void initWidget(StructListProperty<ListElementType> &property, const QString &propertyName)
    {

        auto widget = new ListPropertyWidget<ListElementType>(propertyName);

        typedef typename DummyUIDesc<ListElementType>::PanelType ElementPanelType;
        auto widgetForNewElement = new ElementPanelType("New");
        widgetForNewElement->init();
        widget->setValueWidget(widgetForNewElement);

        QObject::connect(widget->createNewElementButton, &QPushButton::clicked, [&property, widgetForNewElement]() {
                    auto v = property.value();
                    v.append(widgetForNewElement->value().clone());
                    property.setValue(v);
                });

        addWidget(*widget);

        connect(property.owner(), property.signal(), this, [this]() {
                    if (m_autoSaveEnabled) {
                        saveJSONSnapshot();
                    }
                });

    }

    template<typename ListElementType>
    void initWidget(SimpleTypeListProperty<ListElementType> &property, const QString &propertyName)
    {

        //        Q_ASSERT(false);
        qWarning() << "TODO : implement";
        auto widget = new ListPropertyWidget<ListElementType>(propertyName);

        typedef typename DummyUIDesc<ListElementType>::PanelType ElementPanelType;
        auto widgetForNewElement = new ElementPanelType("New");
        //        widgetForNewElement->init();
        widget->setValueWidget(widgetForNewElement);

        //        QObject::connect(widget->createNewElementButton, &QPushButton::clicked, [&property, widgetForNewElement]() {
        //            property.addElement(widgetForNewElement->value().clone());
        //        });

        addWidget(*widget);

        connect(property.owner(), property.signal(), this, [this]() {
                    if (m_autoSaveEnabled) {
                        saveJSONSnapshot();
                    }
                });
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

    void appendLog(QString textToAppend)
    {
        QString text = ui->logLabel->toPlainText() + "\n" + textToAppend;
        ui->logLabel->setPlainText(text);
    }

    template<typename ParameterType>
    void logSetterCall(const QString propertyName, const ParameterType &value)
    {
        appendLog(propertyName + " setter called with value : " + toString(value));
    }

};
