import QtQuick 2.0
import facelift.test 1.0
import facelift 1.0

Rectangle {
    width: 400
    height: 300
    color: "green"

    TestInterfaceImplementation {
        intProperty: 5
        stringListProperty: [ 'one', 'two' ]
        intMapProperty: { 'one':1, 'two':2 }

        onEventWithList: console.log("Received signal with list parameter: " + p);
        onEventWithMap: console.log("Received signal with map parameter, e.g.: " + p.five);
        onEventWithStructWithList: console.log("Received signal with struct parameter. list of ints: "
                                               + p.listOfInts + " / list of structs: " + p.listOfStructs);

        Component.onCompleted: {
            console.log("stringListProperty: " + stringListProperty);
            console.log("interfaceListProperty: " + interfaceListProperty[0]);
            interfaceListProperty[0].doSomething();
            console.log("intMapProperty.one: " + intMapProperty.one
                        + ", intMapProperty['two']: "+ intMapProperty['two']);

            console.log("interfaceMapProperty[\"key1\"]: " + interfaceMapProperty["key1"]);
            interfaceMapProperty["key1"].doSomething();
        }
    }

}
