/**********************************************************************
**
** Copyright (C) 2018 Luxoft Sweden AB
**
** This file is part of the FaceLift project
**
** Permission is hereby granted, free of charge, to any person
** obtaining a copy of this software and associated documentation files
** (the "Software"), to deal in the Software without restriction,
** including without limitation the rights to use, copy, modify, merge,
** publish, distribute, sublicense, and/or sell copies of the Software,
** and to permit persons to whom the Software is furnished to do so,
** subject to the following conditions:
**
** The above copyright notice and this permission notice shall be
** included in all copies or substantial portions of the Software.
**
** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
** EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
** MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
** NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
** BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
** ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
** CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
** SOFTWARE.
**
** SPDX-License-Identifier: MIT
**
**********************************************************************/

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
                        var station = modelData
                        viewModel.setCurrentStation(station)
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
