import QtQuick 2.0
import facelift.example.mypackage 1.0

/**
 * This file contains the UI code of the application.
 */
Item {

    id: root

    // Add a typed property 
    readonly property MyInterface model: _myInterfaceImplementation

    MouseArea {
    	anchors.fill: parent
    	onClicked: model.resetCounter();
    }

    Text {
       text: model.counter
    }

    // Create an instance of the create QML type registered under the name "MyInterfaceImplementation".
    // The exact type depends on what has been registered during the plugin initialization, but the interface implemented by any type 
    // registered under the name "MyInterface" should be the same. That enables the UI code to be reused whatever implementation is registered.
    MyInterfaceImplementation {
        id: _myInterfaceImplementation
        onCounterReset: print("Counter has been reset")
    }

}
