import QtQuick 2.0
import facelift.example.addressbook 1.0

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
    	viewModel: viewModelProxy
    }
    
    AddressBookImpl {
        id: viewModel
        IPC.enabled: true
    }

    AddressBookIPCProxy {
         id: viewModelProxy
    }
    
}
