import QtQuick 2.0
import addressbook 1.0

import QtQuick.Controls 1.2


/**
 * AddressBook application UI
 */
Item {

    id: root
    height: mainPanel.height
    width: mainPanel.width

    property bool serverSide: true

    AddressBookMainPanel {
        id: mainPanel
    	anchors.fill: parent
    	viewModel: viewModel
    }

    AddressBookDummy {
        id: viewModel
    }

}
