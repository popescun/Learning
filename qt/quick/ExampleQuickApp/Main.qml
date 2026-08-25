import QtQuick
import QtQuick.Controls

Window {
    width: 640
    height: 480
    visible: true
    title: qsTr("Hello World")

    Column {
        id: column
        width: 125
        height: 75
        anchors.centerIn: parent

        TextField {
            id: textField
            placeholderText: "Enter some text"
        }

        Button {
            id: buttone
            text: "Click to open popup"

            onClicked: {
                myPopup.open()
            }
        }
    }

    Popup {
        id: myPopup
        anchors.centerIn: parent
        width: 150
        height: 75
        closePolicy: "CloseOnEscape"

        Column {
            anchors.centerIn: parent
            spacing: 10

            Text {
                text: textField.text
            }

            Button {
                text: "Close"
                width: 100

                onClicked: {
                    myPopup.close()
                }
            }
        }
    }
}
