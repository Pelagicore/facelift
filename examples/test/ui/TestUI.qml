import QtQuick 2.0
import facelift.test 1.0

Rectangle {
    width: 400
    height: 300
    color: "green"

    TestInterfaceImplementation {
        intProperty: 5
        onEventWithList: console.log("Received signal with list parameter: " + p);
    }
}
