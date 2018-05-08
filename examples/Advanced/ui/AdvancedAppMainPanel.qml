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
import QtQuick.Layouts 1.2

/**
 * Media player application UI
 */

Item {

    id: root

    width: 700
    height: 600

    property var advancedModel

    ListView {
        id: mediaList
        Layout.fillHeight: true
        width: root.width / 3
        height: 400

        model: advancedModel.theModel

        delegate: Rectangle {
            height: 30
            width: parent.width
            color: (index % 2) == 0 ? "gray" : "lightgray"

            Row {
            
                spacing: 10
                anchors.fill: parent
            
                Rectangle {
                    width: 50
                    height: parent.height 
                    color: "blue"
                    
                    Text {
                        text: "Remove"
                    }

                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            advancedModel.deleteModelItem(modelData)
                        }
                    }
                    
                }
    
                Text {
                    id: trackNameLabel
                    text: modelData.name
                }

                Rectangle {
                    width: 50
                    height: parent.height 
                    color: "red"

                    Text {
                        text: "Rename"
                    }

                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            advancedModel.renameModelItem(modelData, "renamed item")
                        }
                    }

                }

                Rectangle {
                    width: 50
                    height: parent.height
                    color: "green"
                    
                    Text {
                        text: "Insert"
                    }

                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            advancedModel.insertNewModelItemAfter(modelData)
                        }
                    }

                }

            }

        }
    }
}
