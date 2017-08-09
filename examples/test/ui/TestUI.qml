import QtQuick 2.0
import facelift.test 1.0

import QtQuick.Controls 1.2

Item {

    id: root
    height: 500
    width: 500

    Rectangle {
        anchors.fill: parent
	    color: "green"
    }

    TestStruct {
        id: testStructIntance1
        aString: "This is the first item"
    }

    TestInterfaceImplementation {
        id: testInterface

        property int anInt: 0

        enumProperty: TestEnum.E2

        stringListProperty: [ "string1", "string2" ]
        intListProperty: [ 8, anInt ]
        structListProperty: [ testStructIntance1 ]

        structProperty: testStructIntance1

        Component.onCompleted: {
            enumProperty = TestEnum.E3
        }

        function addNewItem() {
            var content = testInterface.structListProperty
            var newItem = Module.createTestStruct()
            newItem.aString = "IIIIIII"
            content.push(newItem)
            testInterface.structListProperty = content
        }

        Timer {
            running: true
            interval: 1000
            repeat: true
            onTriggered: {
                testInterface.anInt++;
                testStructIntance1.aString = testStructIntance1.aString + "-"
                testInterface.addNewItem()
            }
        }

    }

}
