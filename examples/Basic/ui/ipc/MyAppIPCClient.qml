import QtQuick 2.0
import mypackage 1.0


/**
 * My application UI
 */
Item {

    id: root

    MouseArea {
    	anchors.fill: parent
    	onClicked: {
    	    myInterface.resetCounter();
    	}
    }

    Text {
       text: myInterface.counter
    }

    MyInterfaceIPCProxy {
        // Create an IPC proxy for the MyInterface. This object implements the "MyInterface" interface, but uses an IPC connection
        // to interact with actual server instance. It can be used as if the server object was local.
        id: myInterface

        onCounterReset: print("The counter has been reset")

        // The object path must match the one used by the server side. The default value is suitable if the service is a singleton
//        objectPath: "/my/object/path"
    }

}
