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

#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>

#include "FaceliftModel.h"
#include "ModelProperty.h"
#include "Structure.h"

#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>

#include <tuple>

#include <QFileSystemWatcher>
#include "ControlWidgets.h"

#include "QMLAdapter.h"

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
inline void readJSONSimple<double>(const QJsonValue &json, double &value)
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


template<typename ListElementType>
QJsonArray toJsonArray(const QList<ListElementType> &list)
{
    QJsonArray array;
    for (const auto &propertyElement : list) {
        QJsonValue jsonValue;
        DummyModelTypeHandler<ListElementType>::writeJSON(jsonValue, propertyElement);
        array.append(jsonValue);
    }
    return array;
}


template<typename ElementType>
struct DummyModelTypeHandler<QList<ElementType> >
{
    static void writeJSON(QJsonValue &json, const QList<ElementType> &value)
    {
        json = toJsonArray(value);
    }

    static void readJSON(const QJsonValue &json, QList<ElementType> &value)
    {
        value.clear();
        auto array = json.toArray();
        for (const auto &jsonObject : array) {
            Q_UNUSED(jsonObject);
        }
    }

};


template<typename ElementType>
struct DummyModelTypeHandler<QMap<QString, ElementType> >
{
    static void writeJSON(QJsonValue &json, const QMap<QString, ElementType> &value)
    {
        Q_UNUSED(json);
        Q_UNUSED(value);
        Q_ASSERT(false);
    }

    static void readJSON(const QJsonValue &json, QMap<QString, ElementType> &value)
    {
        Q_UNUSED(json);
        Q_UNUSED(value);
        Q_ASSERT(false);
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
struct DummyModelTypeHandler<Type *, typename ::std::enable_if<::std::is_base_of<InterfaceBase, Type>::value>::type>
{

    static void writeJSON(QJsonValue &json, const Type *value)
    {
        Q_UNUSED(json);
        Q_UNUSED(value);
        Q_ASSERT(false);
    }

    static void readJSON(const QJsonValue &json, Type *value)
    {
        Q_UNUSED(json);
        Q_UNUSED(value);
        Q_ASSERT(false);
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

class DummyModelBase : public QObject
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
            qCDebug(LogTools) << "Loaded JSON file " << jsonFile.fileName();
            return jsonDoc;
        } else {
            qCWarning(LogTools) << "Can't load JSON file " << jsonFile.fileName();
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
            qCWarning(LogTools) << "Can't create folder " << folder;
        }

        auto success = jsonFile.open(QFile::WriteOnly);
        if (success) {
            jsonFile.write(doc.toJson());
            qCDebug(LogTools) << "JSON file written to " << jsonFile.fileName() << " : " << doc.toJson();
        } else {
            qCWarning(LogTools) << "Can't save JSON file " << jsonFile.fileName();
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

    void appendLog(QString textToAppend) const;

private:
    Ui_DummyModelPanel *ui = nullptr;

    QWidget *m_window = nullptr;
    bool m_autoSaveEnabled = true;

    bool m_oddWidget = true;

    QList<PropertyWidgetBase *> m_widgets;

    QFileSystemWatcher m_fileSystemWatcher;

    QString m_interfaceName;

};


class DummyModelControlWindow : public QMainWindow
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
class DummyModel : public DummyModelBase
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
    void writeJSONProperty(QJsonObject &json, const ListProperty<ListElementType> &property,
            const char *propertyName) const
    {
        json[propertyName] = toJsonArray(property.value());
    }

    template<typename ListElementType>
    static QList<ListElementType> modelAsList(const ModelProperty<ListElementType> &property)
    {
        auto size = property.size();
        QList<ListElementType> list;
        for (int i = 0; i < size; i++) {
            list.append(property.elementAt(i));
        }
        return list;
    }

    template<typename ListElementType>
    void writeJSONProperty(QJsonObject &json, const ModelProperty<ListElementType> &property,
            const char *propertyName) const
    {
        auto list = modelAsList(property);
        json[propertyName] = toJsonArray(list);
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
            ListElementType e {};
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
            qCWarning(LogTools) << "Expected array in property " << propertyName;
        }
    }

    template<typename ListElementType>
    void readJSONProperty(const QJsonObject &json, ModelProperty<ListElementType> &property, const char *propertyName) const
    {
        auto jsonValue = json[propertyName];
        if (jsonValue.isArray()) {
            auto elements = readJSONArray<ListElementType>(jsonValue);
            property.reset(elements.size(), [elements] (int index) {
                return elements[index];
            });
        } else {
            qCWarning(LogTools) << "Expected array in property " << propertyName;
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
        widget->enableEdition();

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
        widget->addCreateNewElementButton();

        typedef typename TypeToWidget<ListElementType>::PanelType ElementPanelType;
        auto widgetForNewElement = new ElementPanelType(*new ListElementType(), "New");
        widgetForNewElement->init();
        widgetForNewElement->enableEdition();
        widget->setValueWidget(widgetForNewElement);

        QObject::connect(widget->createNewElementButton, &QPushButton::clicked, [&property, widgetForNewElement]() {
                auto v = modelAsList(property);
                ListElementType clone = TypeToWidget<ListElementType>::clone(widgetForNewElement->value());     // ensure structs are cloned to get a new ID
                v.append(clone);
                property.reset(v.size(), [v] (int index) {
                    return v[index];
                });
            });

        addPropertyWidget_(property, *widget);
    }

    template<typename ListElementType>
    void addPropertyWidget(ListProperty<ListElementType> &property, const QString &propertyName)
    {
        auto widget = new ListPropertyWidget<ListElementType>(*(new QList<ListElementType>()), propertyName);
        widget->enableEdition();
        widget->addCreateNewElementButton();

        typedef typename TypeToWidget<ListElementType>::PanelType ElementPanelType;
        auto widgetForNewElement = new ElementPanelType(*new ListElementType(), "New");
        widgetForNewElement->init();
        widget->setValueWidget(widgetForNewElement);
        widgetForNewElement->enableEdition();

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

    template<typename ... ParameterTypes>
    void logMethodCall(const QString methodName, const ::std::array<const char *, sizeof ... (ParameterTypes)> &parameterNames,
            const ParameterTypes & ... parameters) const
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
        qCDebug(LogTools) << "Added property widget" << property.name();
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
            qCDebug(LogTools) << "Registering dummy type for interface " << fullyQualifiedTypeName;
            registerQmlComponent<DummyInterfaceType>(uri, DummyInterfaceType::INTERFACE_NAME);
        } else {
            qCDebug(LogTools) << "QML type already registered : " << fullyQualifiedTypeName << " => not registering dummy implementation";
        }
    }

};

}
