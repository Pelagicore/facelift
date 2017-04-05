#include "ipc.h"
#include <QDebug>
#include <QApplication>

#include <sys/types.h>
#include <unistd.h>

#include "addressbook/AddressBookDummy.h"
#include "addressbook/AddressBookIPC.h"

using namespace addressbook;

void mainClient(int &argc, char **argv) {

    QApplication app(argc, argv);
    auto sessionBus = QDBusConnection::sessionBus();

    qDebug() << "Client running";

    AddressBookIPCProxy proxy;

    QTimer timer;
    QObject::connect(&timer, &QTimer::timeout, [&] () {
        qWarning() << "IsLodaded" << proxy.isLoaded();
        qDebug() << "createNewContact ";
        proxy.createNewContact();
//            static int id = 0;
//            proxy.selectContact(id++);
        });
    timer.start(1000);

    QObject::connect(&proxy, &AddressBook::isLoadedChanged, [&] () {
        qWarning() << "IsLodaded changed " << proxy.isLoaded();
    });

    app.exec();
    qDebug() << "Client exited";

}


void mainServer(int &argc, char **argv) {

    QApplication app(argc, argv);

//    auto sessionBus = QDBusConnection::sessionBus();
//    auto success = sessionBus.registerService(SERVICE);
//    assert(success);

    AddressBookDummy abook;
    AddressBookIPCAdapter svc;
    svc.setService(&abook);

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
