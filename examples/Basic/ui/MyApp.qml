import QtQuick 2.0
import facelift.example.mypackage 1.0

/**
 * This file contains the UI code of the application.
 */
Item {

    id: root

    MouseArea {
    	anchors.fill: parent
    	onClicked: myInterface.resetCounter();
    }

    Text {
       text: myInterface.counter
    }

    MyInterface {
        // This is an instance of the QML component registered under the name "MyInterface".
        // The exact type depends on what has been registered during the plugin initialization, but the interface implemented by any type 
        // registered under the name "MyInterface" should be the same. That enables the UI code to be reused whatever implementation is registered.
        id: myInterface
        onCounterReset: print("Counter has been reset")
    }

}
