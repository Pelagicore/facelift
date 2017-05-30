import QtQuick 2.5

import QtQuick.Controls 1.4

/**
 * Tuner application UI
 */

Item {

    id: root

    width: 700
    height: 600

    property var viewModel

    Column {

        Row {
            Label {
                text: "Frequency: "
            }
            Text {
                text: viewModel.currentStation.frequency
            }
        }

        Row {
            Label {
                text: "Name: "
            }
            Text {
                text: viewModel.currentStation.name
            }
        }

        Row {
            Button {
                text: "Next"
                onClicked: viewModel.nextStation()
            }
            Button {
                text: "Previous"
                onClicked: viewModel.previousStation()
            }
        }


    }


}
