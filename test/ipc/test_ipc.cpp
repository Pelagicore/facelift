#include "ipc/ipc.h"
#include <QDebug>
#include <QApplication>

#include <sys/types.h>
#include <unistd.h>

#include "test/TestInterfaceDummy.h"
#include "test/TestInterfaceIPC.h"

using namespace test;

void mainClient(int &argc, char **argv) {

    QApplication app(argc, argv);
    auto sessionBus = QDBusConnection::sessionBus();

    qDebug() << "Client running";

    TestInterfaceIPCProxy proxy;

    QTimer timer;
    QObject::connect(&timer, &QTimer::timeout, [&] () {
        qWarning() << "boolProperty" << proxy.boolProperty();
        qDebug() << "createNewContact ";
        proxy.method1();
//            static int id = 0;
//            proxy.selectContact(id++);
        });
    timer.start(1000);

    QObject::connect(&proxy, &TestInterface::boolPropertyChanged, [&] () {
        qWarning() << "boolProperty changed " << proxy.boolProperty();
    });

    app.exec();
    qDebug() << "Client exited";

}


void mainServer(int &argc, char **argv) {

    QApplication app(argc, argv);

    TestInterfaceDummy testInterface;
    TestInterfaceIPCAdapter svc;
    svc.setService(&testInterface);

    qDebug() << "Server running";
    app.exec();
    qDebug() << "Server exited";

}

int main(int argc, char **argv) {

    if (argc == 1) {
        mainServer(argc, argv);
    } else {
        mainClient(argc, argv);
    }

}
