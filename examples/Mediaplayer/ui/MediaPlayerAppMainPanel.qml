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
import QtQuick.Layouts 1.2
import QtWebKit 3.0

/**
 * Media player application UI
 */

Item {

    id: root

    width: 700
    height: 600

    property var mediaIndexerModel

    ListView {
        id: mediaList
        Layout.fillHeight: true
        width: root.width / 3
        height: 400

        model: mediaIndexerModel.files

        delegate: Rectangle {
            height: 30
            width: parent.width
            color: (index % 2) == 0 ? "gray" : "lightgray"

            Label {
                id: trackNameLabel
                text: modelData.title + ( modelData.artist ? (" - " + modelData.artist) : "")
            }

            MouseArea {
                anchors.fill: trackNameLabel
                onClicked: {
                    print("Clicked " + modelData.title)
                    webview.url = modelData.url;
                }
            }
        }
    }

    WebView {
        id: webview
        anchors.left: mediaList.right
        anchors.right: root.right

        width: 400
        height: 400
        z: -1
    }

}
