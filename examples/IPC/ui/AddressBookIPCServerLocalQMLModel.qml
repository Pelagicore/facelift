import QtQuick 2.0
import addressbook 1.0

import QtQuick.Controls 1.2

import "../../AddressBook/ui"

Item {

    id: root
    height: mainPanel.height
    width: mainPanel.width

    Rectangle {
        anchors.fill: parent
	    color: "green"
    }

    AddressBookMainPanel {
        id: mainPanel
    	anchors.fill: parent
    	viewModel: proxy
    }

    AddressBookIPCProxy {
        id: proxy
    }

    AddressBookImplementation {
        id: viewModel

        isLoaded: true

        testProperty: ContactType.Family

        // This object is used to register our model object on the bus
        AddressBookIPCAdapter {
            service: viewModel.provider
        }
    }

}
