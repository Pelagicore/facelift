import QtQuick 2.0
import QtQuick.Controls 1.2

Row {

    id: root
    property alias text: text.text
    property alias label: label.text

    Text {
        id: label
        width: 100
    }

    TextEdit {
        id: text
    }

}
