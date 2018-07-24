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

import QtQuick 2.0
import advanced 1.0


Item {
    id: root
    height: 386
    width: 360

    property AdvancedModel advancedModel

    Component {
        id: delegate

        Rectangle {
            height: 30
            width: parent.width
            color: (index % 2) == 0 ? "gray" : "lightgray"

            Row {
                anchors.fill: parent
                anchors.margins: 4
                spacing: 4

                Text {
                    width: 80
                    text: modelData.name
                }

                Rectangle {
                    width: 60
                    height: parent.height
                    color: "lightsteelblue"

                    Text {
                        text: "Rename"
                    }

                    MouseArea {
                        anchors.fill: parent
                        onClicked: advancedModel.renameModelItem(modelData, "x" + modelData.name);
                    }
                }

                Rectangle {
                    width: 60
                    height: parent.height
                    color: "green"

                    Text {
                        text: "Insert"
                    }

                    MouseArea {
                        anchors.fill: parent
                        onClicked: advancedModel.insertNewModelItemAfter(modelData);
                    }
                }

                Rectangle {
                    width: 60
                    height: parent.height
                    color: "red"

                    Text {
                        text: "Remove"
                    }

                    MouseArea {
                        anchors.fill: parent
                        onClicked: advancedModel.deleteModelItem(modelData);
                    }
                }
            }
        }
    }

    ListView {
        id: mediaList
        width: 280
        height: 286
        anchors.centerIn: parent

        model: advancedModel.theModel
        delegate: delegate
    }
}
