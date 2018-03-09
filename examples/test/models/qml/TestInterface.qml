/*
 *   This file is part of the FaceLift project
 *   Copyright (C) 2017 Pelagicore AB
 *   SPDX-License-Identifier: LGPL-2.1
 *   This file is subject to the terms of the <license name> licence.
 *   Please see the LICENSE file for details.
 */

import QtQuick 2.0
import facelift.test 1.0


TestInterfaceQMLImplementation {
    id: root

    setintProperty: function(i) {
        console.error("new intProperty: ", i);
    }

    Timer {
        running: true
        interval: 1000
        onTriggered: root.eventWithList([1, 2, 3], false);
    }

    Component.onCompleted: console.log("QML implementation of TestInterface is used");
}
