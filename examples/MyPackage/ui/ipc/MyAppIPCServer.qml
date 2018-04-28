import QtQuick 2.0
import facelift.example.mypackage 1.0

/**
 * Server side
 */
Item {

    Text {
       id: text
       text: "My interface server"
    }

    MyInterface {
        // This is an instance of the QML component registered under the name "MyInterface".
        id: myInterface

        onCounterReset: {
            text.text = text.text + "\nreset"
        }

        // register that object on the IPC
        IPC.enabled: true

        // An object path can be set if needed. The default value is suitable if the service is a singleton
//        IPC.objectPath: "/my/object/path"
    }

}
