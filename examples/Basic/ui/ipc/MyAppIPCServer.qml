import QtQuick 2.0
import mypackage 1.0

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
    }

    MyInterfaceIPCAdapter {
        // This object registers our interface implementation on the IPC so that it can be accessed by other processes
        service: myInterface
       
       // An object path can be set if needed. The default value is suitable if the service is a singleton
//        objectPath: "/my/object/path"   
    }

}
