import QtQuick 2.0
import tuner 1.0

import QtQuick.Controls 1.2


/**
 * Tuner application UI
 */
Item {

    id: root

    height: mainPanel.height
    width: mainPanel.width

    TunerAppMainPanel {
        id: mainPanel
    	anchors.fill: parent
    	viewModel: viewModel
    }

    TunerViewModel {
        id: viewModel
    }

}
