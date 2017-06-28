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
	    color: "yellow"
    }

    AddressBookMainPanel {
        id: mainPanel
    	anchors.fill: parent
    	viewModel: viewModel
    }

    AddressBookIPCProxy {
         id: viewModel
    }
    
}
