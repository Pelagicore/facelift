#include <QtCore>
#include <QDebug>

#include "FaceliftProperty.h"
#include "TestProperty.h"

#define EXPECT_TRUE(cond) if (!(cond)) { qFatal("Expectation wrong:" #cond "%s", qPrintable(__PRETTY_FUNCTION__)); };

int main(int argc, char *argv[])
{
    TestPropertyClass c;
    facelift::Property<int> intProperty = 7;
    bool signalTriggered = false;
    QObject::connect(&c, &TestPropertyClass::aSignal, [&] () {
        signalTriggered = true;
    });
    intProperty.init(&c, &TestPropertyClass::aSignal);

    intProperty = 9;  // We expect the signal to be triggered here
    EXPECT_TRUE(signalTriggered);

    return 0;
}
