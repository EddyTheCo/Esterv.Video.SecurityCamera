import QtQuick
import QtQuick.Controls
import SecurityCamera
import Esterv.Styles.Simple
import Esterv.CustomControls
import Esterv.AboutEstervSecurityCamera
import QtQuick.Layouts

ApplicationWindow {
    id: window
    visible: true
    background: Rectangle {
        color: Style.backColor1
    }

    Client {
        id: tcpclient
        address: address.text
    }
    ThemeSwitch {
        anchors.right: parent.right
        anchors.top: parent.top
    }

    GridLayout {
        id: grid
        property bool iscolumn: width < 800

        anchors.fill: parent
        flow: grid.iscolumn ? GridLayout.TopToBottom : GridLayout.LeftToRight
        columns: 2

        ColumnLayout {
            id: player
            Layout.fillHeight: true
            Layout.fillWidth: true
            Layout.minimumWidth: (grid.iscolumn)?300:600
            Layout.minimumHeight: 300
            Rectangle {
                id: showIsConected

                //topRightRadius:Math.max(width,height)*0.5
                //bottomRightRadius:topRightRadius

                Layout.minimumHeight: 60
                Layout.minimumWidth: 180

                color: Style.backColor2

                MouseArea {
                    anchors.fill: showIsConected
                    hoverEnabled: true
                    ToolTip.text: connectedLabel.text
                    ToolTip.visible: containsMouse
                }

                RowLayout {
                    anchors.fill: showIsConected
                    Label {
                        id: connectedLabel

                        text: qsTr(tcpclient.state ? "Connected" : "Not Connected")
                        elide: Text.ElideRight
                    }
                    Rectangle {
                        Layout.minimumWidth: 20
                        Layout.preferredHeight: width
                        radius: width * 0.5
                        color: tcpclient.state ? "green" : "red"
                    }
                }
            }
            Image {
                id: framedisplay
                Layout.fillHeight: true
                Layout.fillWidth: true
                cache: false
                source: "image://wasm/" + tcpclient.frameSource
                fillMode: Image.PreserveAspectFit
            }
        }
        Frame {
            id: serverConf
            Layout.alignment: (grid.iscolumn)?Qt.AlignBottom|Qt.AlignHCenter:Qt.AlignVCenter|Qt.AlignRight
            Layout.fillHeight: true
            Layout.fillWidth: true
            Layout.maximumWidth: 400
            Layout.maximumHeight: 250

            GridLayout {
                anchors.fill: parent
                anchors.margins: 10
                columns: 2
                flow: (width < 300) ? GridLayout.TopToBottom : GridLayout.LeftToRight
                Label {
                    text: qsTr("Server:")
                }
                TextField {
                    id: address
                    Layout.fillWidth: true
                    text: "ws://localhost:9999"
                }
                Item {
                    id: name
                }
                Button {
                    text: qsTr("Connect")
                    onClicked: tcpclient.tryConnect()
                    Layout.alignment: Qt.AlignRight
                }
            }
        }
    }
    Label {
        id: aboutLabel
        text: AboutThis.name + " " + AboutThis.version
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter

        MouseArea {
            anchors.fill: parent
            hoverEnabled: true

            Popup {
                id: aboutpop
                visible: parent.containsMouse
                y: -height
                width: 300
                height: 400
                About {
                    anchors.fill: parent
                    anchors.margins: 10
                    description: "Made with \u2764 by Esterv"
                    logo: "https://eddytheco.github.io/Esterv.Crypto.NFTWallet/img/esterlogo.png"
                }
            }
        }
    }
}
