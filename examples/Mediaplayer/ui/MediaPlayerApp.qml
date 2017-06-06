import QtQuick 2.0
import mediaplayer 1.0

import QtQuick.Controls 1.2

/**
 * Media player application UI
 */
Item {

    id: root

    height: mainPanel.height
    width: mainPanel.width

    MediaPlayerAppMainPanel {
        id: mainPanel
    	anchors.fill: parent
        mediaIndexerModel: viewModel
    }

    MediaIndexerModel {
        id: viewModel
    }

}
