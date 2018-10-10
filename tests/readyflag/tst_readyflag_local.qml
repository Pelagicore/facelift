import QtTest 1.2
import tests.readyflag  1.0

TestCase {

    ReadyFlagInterface{
        id: iReady
    }

    function test_readyInt() {

        compare(iReady.readyFlags.intProperty,false)
        tryVerify(function() {
            iReady.intProperty = 10;
            return true;
        }, 2000);
        compare(iReady.readyFlags.intProperty,true);
        compare(iReady.intProperty,10);
    }

    function test_readyString() {

        compare(iReady.readyFlags.strProperty,false);
        tryVerify(function() {
            iReady.strProperty = "hello";
            return true;
        }, 2000);

        compare(iReady.readyFlags.strProperty,true);
        compare(iReady.strProperty,"hello");
    }

    function test_readyCombo() {

        compare(iReady.readyFlags.comboData,false);
        tryVerify(function() {
            iReady.comboData.iData = 100;
            iReady.comboData.sData = "test";
            iReady.comboData.bData = true;
            return true;
        }, 2000);

        compare(iReady.readyFlags.comboData,true);
        compare(iReady.comboData.iData,100);
        compare(iReady.comboData.sData,"test");
        compare(iReady.comboData.bData,true);
    }
}
