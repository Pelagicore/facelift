/*
 *   This is part of the FaceLift project
 *   Copyright (C) 2017 Pelagicore AB
 *   SPDX-License-Identifier: LGPL-2.1
 *   This file is subject to the terms of the LGPL 2.1 license.
 */

#pragma once

#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>

#include "Model.h"
#include "Property.h"

#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>

#include <tuple>

#include <QFileSystemWatcher>
#include "ControlWidgets.h"

#include "QMLFrontend.h"

class Ui_DummyModelPanel;

namespace Ui {
class DummyModelsMainWindow;
}

namespace facelift {

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




/*
template<typename Type>
inline void assignRandomValue(Type &t)
{
    t = {};
}

#include <random>
template<>
inline void assignRandomValue(int &t)
{
    static ::std::normal_distribution<double> normal_dist(653, 10);
    typedef ::std::mt19937 MyRNG;  // the Mersenne Twister with a popular choice of parameters
    MyRNG rng;
    t = normal_dist(rng);
}

template<>
inline void assignRandomValue(QString &t)
{
    t = "Random string";
}

template<::std::size_t I = 0, typename ... Tp>
inline typename ::std::enable_if<I == sizeof ... (Tp), void>::type
random_tuple(::std::tuple<Tp ...> &t)
{
    Q_UNUSED(t);
}

template<::std::size_t I = 0, typename ... Tp>
inline typename ::std::enable_if < I<sizeof ... (Tp), void>::type
random_tuple(::std::tuple<Tp ...> &t)
{
    assignRandomValue(::std::get<I>(t));
    random_tuple<I + 1, Tp ...>(t);
}
*/

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
struct DummyModelTypeHandler<Type, typename ::std::enable_if<::std::is_base_of<StructureBase, Type>::value>::type>
{

    static void writeJSON(QJsonValue &json, const Type &value)
    {
        QJsonObject subObject;
        writeFieldsToJson(value.asTuple(), subObject);
        json = subObject;
    }

    template<::std::size_t I = 0, typename ... Tp>
    static typename ::std::enable_if<I == sizeof ... (Tp), void>::type
    writeFieldsToJson(const ::std::tuple<Tp ...> &value, QJsonObject &jsonObject)
    {
        Q_UNUSED(value);
        Q_UNUSED(jsonObject);
    }

    template<::std::size_t I = 0, typename ... Tp>
    static typename ::std::enable_if < I<sizeof ... (Tp), void>::type
    writeFieldsToJson(const ::std::tuple<Tp ...> &value, QJsonObject &jsonObject)
    {
        QJsonValue v;
        typedef typename ::std::tuple_element<I, typename Type::FieldTupleTypes>::type FieldType;
        DummyModelTypeHandler<FieldType>::writeJSON(v, ::std::get<I>(value));
        jsonObject[Type::FIELD_NAMES[I]] = v;
        writeFieldsToJson<I + 1, Tp ...>(value, jsonObject);
    }

    static void readJSON(const QJsonValue &json, Type &value)
    {
        QJsonObject subObject = json.toObject();
        readFieldsFromJson(value.asTuple(), subObject);
    }

    template<::std::size_t I = 0, typename ... Tp>
    static typename ::std::enable_if<I == sizeof ... (Tp), void>::type
    readFieldsFromJson(::std::tuple<Tp ...> &value, QJsonObject &jsonObject)
    {
        Q_UNUSED(value);
        Q_UNUSED(jsonObject);
    }

    template<::std::size_t I = 0, typename ... Tp>
    static typename ::std::enable_if < I<sizeof ... (Tp), void>::type
    readFieldsFromJson(::std::tuple<Tp ...> &value, QJsonObject &jsonObject)
    {
        typedef typename ::std::tuple_element<I, typename Type::FieldTupleTypes>::type FieldType;
        DummyModelTypeHandler<FieldType>::readJSON(jsonObject[Type::FIELD_NAMES[I]], ::std::get<I>(value));
        readFieldsFromJson<I + 1, Tp ...>(value, jsonObject);
    }

};

template<typename Type>
struct DummyModelTypeHandler<Type, typename ::std::enable_if<::std::is_enum<Type>::value>::type>
{
    static void readJSON(const QJsonValue &json, Type &value)
    {
        QString s;
        readJSONSimple(json, s);
        assignFromString(s, value);
    }

    static void writeJSON(QJsonValue &json, const Type &value)
    {
        json = facelift::toString(value);
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

    void onPropertyValueChanged()
    {
        if (m_autoSaveEnabled) {
            saveJSONSnapshot();
        }
    }

    void addWidget(PropertyWidgetBase &widget);

    void appendLog(QString textToAppend);

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
    Ui::DummyModelsMainWindow *ui = nullptr;
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
    void addSignalWidget(QString signalName,
            const ::std::array<const char *, sizeof ... (ParameterTypes)> &parameterNames, TypeName *obj,
            void (TypeName::*signalPointer)(ParameterTypes ...))
    {
        typedef Structure<ParameterTypes ...> SignalParametersStruct;

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


    template<typename PropertyType>
    void addPropertyWidget(Property<PropertyType> &property, const QString &propertyName)
    {
        Q_UNUSED(property);

        typedef typename TypeToWidget<PropertyType>::PanelType PanelType;
        auto widget = new PanelType(*new PropertyType(), propertyName);

        widget->init();
        widget->setValue(property.value());
        widget->enableEdition();

        // Update our property if the value is changed with the GUI
        connect(widget, &PanelType::valueChanged, this, [&property, widget]() {
                property = widget->value();
            });

        // Update the GUI if the value is changed in the property
        PropertyConnector<TypeName>::connect(property, this, [&property, widget]() {
                widget->setValue(property.value());
            });

        addPropertyWidget_(property, *widget);
    }

    template<typename ListElementType>
    void addPropertyWidget(ModelProperty<ListElementType> &property, const QString &propertyName)
    {
        auto widget = new ListPropertyWidget<ListElementType>(*(new QList<ListElementType>()), propertyName);
        widget->enableEdition();

        typedef typename TypeToWidget<ListElementType>::PanelType ElementPanelType;
        auto widgetForNewElement = new ElementPanelType(*new ListElementType(), "New");
        widgetForNewElement->init();
        widget->setValueWidget(widgetForNewElement);

        QObject::connect(widget->createNewElementButton, &QPushButton::clicked, [&property, widgetForNewElement]() {
                auto v = property.asList();
                ListElementType clone = TypeToWidget<ListElementType>::clone(widgetForNewElement->value());     // ensure structs are cloned to get a new ID
                v.append(clone);
                property.setElements(v);
            });

        addPropertyWidget_(property, *widget);
    }


    template<typename ListElementType>
    void addPropertyWidget(ListProperty<ListElementType> &property, const QString &propertyName)
    {
        auto widget = new ListPropertyWidget<ListElementType>(*(new QList<ListElementType>()), propertyName);
        widget->enableEdition();

        typedef typename TypeToWidget<ListElementType>::PanelType ElementPanelType;
        auto widgetForNewElement = new ElementPanelType(*new ListElementType(), "New");
        widgetForNewElement->init();
        widget->setValueWidget(widgetForNewElement);

        QObject::connect(widget->createNewElementButton, &QPushButton::clicked, [&property, widgetForNewElement]() {
                auto v = property.value();
                ListElementType clone = TypeToWidget<ListElementType>::clone(widgetForNewElement->value());     // ensure structs are cloned to get a new ID
                v.append(clone);
                property.setValue(v);
            });

        // Update the GUI if the value is changed in the property
        PropertyConnector<TypeName>::connect(property, this, [&property, widget]() {
                widget->listContentWidget->setText(facelift::toString(property.value()));
            });

        addPropertyWidget_(property, *widget);
    }

    void generateToString(QTextStream &message)
    {
        Q_UNUSED(message);
    }

    template<typename FirstParameterTypes, typename ... ParameterTypes>
    void generateToString(QTextStream &message, const FirstParameterTypes &firstParameter, const ParameterTypes & ... parameters)
    {
        message << firstParameter << ", ";
        generateToString(message, parameters ...);
    }

    template<typename ... ParameterTypes>
    void logMethodCall(const QString methodName, const ::std::array<const char *, sizeof ... (ParameterTypes)> &parameterNames,
            const ParameterTypes & ... parameters)
    {
        Q_UNUSED(parameterNames);

        QString argString;
        QTextStream s(&argString);
        generateToString(s, parameters ...);

        appendLog(methodName + " called with args : " + argString);
    }

    template<typename ParameterType>
    void logSetterCall(const QString propertyName, const ParameterType &value)
    {
        appendLog(propertyName + " setter called with value : " + facelift::toString(value));
    }

private:
    void addPropertyWidget_(PropertyBase &property, PropertyWidgetBase &widget)
    {
        qDebug() << "Added property widget" << property.name();
        addWidget(widget);

        PropertyConnector<TypeName>::connect(property, this, [this] () {
                onPropertyValueChanged();
            });
    }

};


class DummyModuleBase
{
public:
    static bool isTypeRegistered(const QString &fullyQualifiedTypeName, int majorVersion, int minorVersion);

    template<typename DummyInterfaceType>
    static void registerQmlComponentIfNotAlready(const char *uri)
    {
        QString fullyQualifiedTypeName = uri;
        fullyQualifiedTypeName += "/";
        fullyQualifiedTypeName += DummyInterfaceType::INTERFACE_NAME;

        // We register the dummy under the "real" name only if nothing is yet registered
        if (isTypeRegistered(fullyQualifiedTypeName, DummyInterfaceType::VERSION_MAJOR, DummyInterfaceType::VERSION_MINOR)) {
            qDebug() << "Registering dummy type for interface " << fullyQualifiedTypeName;
            registerQmlComponent<DummyInterfaceType>(uri, DummyInterfaceType::INTERFACE_NAME);
        } else {
            qDebug() << "QML type already registered : " << fullyQualifiedTypeName << " => not registering dummy implementation";
        }
    }

};

}
