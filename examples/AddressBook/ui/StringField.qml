import QtQuick 2.0
import QtQuick.Controls 1.4

Row {
    id: root

    property alias label: label.text
    property string text: ""
    property alias editedText: textEdit.text
    property bool isDirty: text !== editedText

    function reset() {
        editedText = Qt.binding(function() { return root.text })
    }

    Text {
        id: label
        width: 100
    }

    TextField {
        id: textEdit
        width: 300
        text: root.text
    }

}
