
import QtQuick 2.0
import tests.asynctests 1.0


AsyncTestsInterfaceQMLImplementation {
    id: root
    value:0

    setvalue: function(a){
        value = a;
    }

    calculateSum: function (a,b,c){
        return a+b+c;
    }

    compareEnums: function(arg1, arg2)
    {
        if(arg1===arg2)
        {
            return AsyncEnums.EASYNC0;
        }
        return AsyncEnums.EASYNC1;
    }


    Component.onCompleted: console.log("QML implementation is used.");
}
