import QtQuick 2.0
import advanced 1.0

/**
 * Application UI
 */
Item {

    id: root

    height: mainPanel.height
    width: mainPanel.width

    AdvancedAppMainPanel {
        id: mainPanel
        anchors.fill: parent
        advancedModel: AdvancedModel
    }

}
