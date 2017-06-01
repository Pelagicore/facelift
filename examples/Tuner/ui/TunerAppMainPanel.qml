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
            Label {
                text: "AF: "
            }
            CheckBox {
                checked: viewModel.enable_AF
                onCheckedChanged: {
                    if (viewModel.enable_AF !== checked)
                        viewModel.enable_AF = checked
                }
            }
        }

        Row {
            Button {
                text: "Previous"
                onClicked: viewModel.previousStation()
            }
            Button {
                text: "Next"
                onClicked: viewModel.nextStation()
            }
        }

    }

}
