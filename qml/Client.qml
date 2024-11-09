import QtQuick
import QtQuick.Controls
import SecurityCamera
import Esterv.Styles.Simple
import Esterv.CustomControls
import QtQuick.Layouts

ApplicationWindow {
    id: window
    visible: true
    background: Rectangle {
        color: Style.backColor1
    }

    Client{
        id:tcpclient
        address:address.text
        port: Number(port.text)
    }
    ThemeSwitch
    {
        anchors.right: parent.right
        anchors.top: parent.top
    }

    GridLayout
    {
        id:grid
        property bool iscolumn:width < 750

        anchors.fill: parent
        flow: grid.iscolumn ? GridLayout.TopToBottom : GridLayout.LeftToRight
        columns:2

        ColumnLayout {
            id: player
            Layout.fillHeight: true
            Layout.fillWidth: true
            Layout.minimumWidth: 400
            Layout.minimumHeight:  300
            Rectangle
            {
                id:showIsConected

                topRightRadius:Math.max(width,height)*0.5
                bottomRightRadius:topRightRadius

                Layout.minimumHeight: 60
                Layout.minimumWidth: 180

                color: Style.backColor2

                MouseArea
                {
                    anchors.fill: showIsConected
                    hoverEnabled: true
                    ToolTip.text: connectedLabel.text
                    ToolTip.visible: containsMouse
                }



                RowLayout
                {
                    anchors.fill: showIsConected
                    Label
                    {
                        id:connectedLabel

                        text:qsTr(tcpclient.state?"Connected":"Not Connected")
                        elide:Text.ElideRight
                    }
                    Rectangle
                    {
                        Layout.minimumWidth: 10
                        Layout.preferredHeight: width
                        radius: width*0.5
                        color: tcpclient.state ? "green" : "red"
                    }
                }

            }
            Image {
                Layout.fillHeight: true
                Layout.fillWidth: true
                id: framedisplay
                cache: false
                source: "image://wasm/" + tcpclient.frameSource
                fillMode: Image.PreserveAspectFit
            }
        }
        Frame
        {
            id:serverConf
            Layout.alignment: Qt.AlignCenter
            Layout.fillHeight: true
            Layout.fillWidth: true
            Layout.maximumWidth: 400
            Layout.maximumHeight:  250
            GridLayout
            {
                anchors.fill: parent
                columns: 2
                flow: (width < 350) ? GridLayout.TopToBottom : GridLayout.LeftToRight
                Label {
                    text: qsTr("Server:")
                }
                TextField {
                    id: address
                    text:"localhost"
                }
                Label {
                    text: qsTr("Port:")
                }
                TextField {
                    id: port
                    validator: IntValidator { bottom: 1000; top: 1000000 }
                    text:"9999"
                }
                Button
                {
                    text:qsTr("Connect")
                    onClicked: tcpclient.tryConnect();
                    Layout.alignment: Qt.AlignRight
                }
            }
        }
    }
}
