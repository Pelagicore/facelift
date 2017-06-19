import QtQuick 2.5

import QtQuick.Controls 1.4
import tuner 1.0

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
                text: "Information program"
            }
            CheckBox {
                checked: viewModel.currentStation.programType === ProgramType.Info
                enabled: false
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
                text: "<<"
                onClicked: viewModel.previousStation()
            }
            Repeater {
                model: viewModel.stationList

                delegate: Button {
                    text: modelData.name + (modelData.isPlaying ? "* " : "")
                    onClicked: {
                        viewModel.setCurrentStation(modelData)
                        print("Clicked " + modelData.name + " id " + modelData.stationId)
                    }
                }

            }
            Button {
                text: ">>"
                onClicked: viewModel.nextStation()
            }
        }

    }

}
