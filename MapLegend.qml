import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Effects
import QtPositioning
import QtLocation

Item {
    property QtObject shaderSource
    z: 11

    anchors.fill: parent

    Rectangle
    {
        id: legendPanel

        width: 320
        height: 100
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.rightMargin: 12
        anchors.bottomMargin: 12
        radius: 12
        border.color: "#88000000"
        border.pixelAligned: true
        border.width: 2

        ShaderEffectSource
        {
            id: sourceItem
            sourceItem: map
            hideSource: false
            live: true
            anchors.fill: parent
            sourceRect: Qt.rect(parent.x, parent.y, parent.width, parent.height)
            clip: true
        }

        // layer.enabled: true
        // layer.effect:
        MultiEffect
        {
            source: sourceItem
            id: effect
            anchors.fill: legendPanel
            blurEnabled: true
            blurMax: 16
            blur: 1
            blurMultiplier: 0
            colorizationColor: "#000000"
            colorization: 0.175
            autoPaddingEnabled: false
            clip: true
        }


        Image
        {
            id: background
            anchors.fill: parent
            fillMode: Image.PreserveAspectCrop
            opacity: 0.03
            clip: true
            source: "images/bernard-hermant-qi-H70ga93s-unsplash.jpg"
            MultiEffect
            {
                source: background
                anchors.fill: background
                blurEnabled: true
                blurMax: 64
                blur: 1.0
                colorizationColor: "#000000"
                colorization: 0.125
            }
        }

        ColumnLayout
        {
            anchors.fill: parent
            Rectangle
            {
                Layout.fillHeight: false;
                Layout.fillWidth: true;
                color: "transparent"
                Layout.preferredHeight: 30
                Layout.maximumHeight: 30
                Layout.rightMargin: 12
                Layout.leftMargin: 12
                Layout.topMargin: 12
                radius: 12

                RowLayout
                {

                    anchors.fill: parent
                    Rectangle
                    {
                        id: poiTotalRect
                        Layout.fillHeight: true;
                        Layout.fillWidth: true;
                        Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                        color: "transparent"
                        Image
                        {
                            id: locationImage
                            anchors.left: parent.left
                            width: 24
                            height: 24
                            smooth: true
                            fillMode: Image.PreserveAspectFit
                            antialiasing: true
                            verticalAlignment: Image.AlignVCenter

                            Connections {
                                target: Icon

                                function onThemeNameChanged()
                                {
                                    locationImage.source = Icon.fromTheme("scalable/place-marker.svg");
                                }
                            }

                            //satisfy pre-load conditions
                            Component.onCompleted:
                            {
                                locationImage.source = Icon.fromTheme("scalable/place-marker.svg")
                            }

                            layer.enabled: true
                            layer.effect: MultiEffect
                            {
                                shadowEnabled: true
                                shadowColor: "#000000"
                                shadowOpacity: 1
                                shadowBlur: 0.25
                            }
                        }

                        Text
                        {
                            text: "<h4>Total</h4>"
                            anchors.left: locationImage.right
                            color: "white"
                            leftPadding: 12
                            height: 24
                            verticalAlignment: Text.AlignVCenter

                            layer.enabled: true
                            layer.effect: MultiEffect
                            {
                                shadowEnabled: true
                                shadowColor: "#000000"
                                shadowOpacity: 1
                                shadowBlur: 0.25
                            }
                        }

                        Text
                        {
                            anchors.right: parent.right
                            text: locationModel.totalPointsOfInterest
                            color: "white"
                            height: 24
                            verticalAlignment: Text.AlignVCenter
                        }
                    }

                    Rectangle
                    {
                        Layout.fillHeight: true;
                        Layout.fillWidth: true;
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        color: "transparent"

                        Image
                        {
                            id: wifiImage
                            width: 24
                            height: 24
                            smooth: true
                            fillMode: Image.PreserveAspectFit
                            antialiasing: true
                            verticalAlignment: Image.AlignVCenter

                            Connections {
                                target: Icon

                                function onThemeNameChanged()
                                {
                                    wifiImage.source = Icon.fromTheme("scalable/wifi.svg");
                                }
                            }

                            //satisfy pre-load conditions
                            Component.onCompleted:
                            {
                                wifiImage.source = Icon.fromTheme("scalable/wifi.svg")
                            }

                            layer.enabled: true
                            layer.effect: MultiEffect
                            {
                                shadowEnabled: true
                                shadowColor: "#000000"
                                shadowOpacity: 1
                                shadowBlur: 0.25
                            }
                        }

                        Text
                        {
                            text: "<h4>WiFi</h4>"
                            color: "white"
                            anchors.left: wifiImage.right
                            height: 24
                            verticalAlignment: Text.AlignVCenter
                            leftPadding: 6

                            layer.enabled: true
                            layer.effect: MultiEffect
                            {
                                shadowEnabled: true
                                shadowColor: "#000000"
                                shadowOpacity: 1
                                shadowBlur: 0.25
                            }
                        }

                        Text
                        {
                            anchors.right: parent.right
                            text: locationModel.wifiPointsOfInterest
                            color: "white"
                            height: 24
                            verticalAlignment: Text.AlignVCenter
                        }
                    }
                }
            }
            Rectangle
            {
                Layout.fillHeight: false;
                Layout.fillWidth: true;
                color: "transparent"
                Layout.preferredHeight: 35
                Layout.maximumHeight: 35
                Layout.rightMargin: 12
                Layout.leftMargin: 12
                Layout.topMargin: 12

                RowLayout
                {

                    anchors.fill: parent
                    Rectangle
                    {
                        Layout.fillHeight: true;
                        Layout.fillWidth: true;
                        Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                        color: "transparent"
                        Image
                        {
                            id: bluetoothImage
                            width: 24
                            height: 24
                            smooth: true
                            fillMode: Image.PreserveAspectFit
                            antialiasing: true
                            verticalAlignment: Image.AlignVCenter

                            Connections {
                                target: Icon

                                function onThemeNameChanged()
                                {
                                    bluetoothImage.source = Icon.fromTheme("scalable/bluetooth.svg");
                                }
                            }

                            //satisfy pre-load conditions
                            Component.onCompleted:
                            {
                                bluetoothImage.source = Icon.fromTheme("scalable/bluetooth.svg")
                            }

                            layer.enabled: true
                            layer.effect: MultiEffect
                            {
                                shadowEnabled: true
                                shadowColor: "#000000"
                                shadowOpacity: 1
                                shadowBlur: 0.25
                            }
                        }

                        Text
                        {
                            text: "<h4>Bluetooth</h4>"
                            anchors.left: bluetoothImage.right
                            color: "white"
                            height: 24
                            verticalAlignment: Text.AlignVCenter
                            leftPadding: 6

                            layer.enabled: true
                            layer.effect: MultiEffect
                            {
                                shadowEnabled: true
                                shadowColor: "#000000"
                                shadowOpacity: 1
                                shadowBlur: 0.25
                            }
                        }

                        Text
                        {
                            id: bluetoothPOIValue
                            anchors.right: parent.right
                            text: locationModel.bluetoothPointsOfInterest
                            color: "white"
                            height: 24
                            verticalAlignment: Text.AlignVCenter

                            Connections {
                                target: locationModel

                                function onBluetoothPointsOfInterestChanged()
                                {
                                    bluetoothPOIValue.text = locationModel.bluetoothPointsOfInterest
                                }
                            }

                            Component.onCompleted:
                            {
                                text = locationModel.bluetoothPointsOfInterest
                            }
                        }
                    }

                    Rectangle
                    {
                        Layout.fillHeight: true;
                        Layout.fillWidth: true;
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        color: "transparent"
                        Image
                        {
                            id: cellularImage
                            width: 24
                            height: 24
                            smooth: true
                            fillMode: Image.PreserveAspectFit
                            antialiasing: true
                            verticalAlignment: Image.AlignVCenter

                            Connections {
                                target: Icon

                                function onThemeNameChanged()
                                {
                                    cellularImage.source = Icon.fromTheme("scalable/cellular-network.svg");
                                }
                            }

                            //satisfy pre-load conditions
                            Component.onCompleted:
                            {
                                cellularImage.source = Icon.fromTheme("scalable/cellular-network.svg")
                            }

                            layer.enabled: true
                            layer.effect: MultiEffect
                            {
                                shadowEnabled: true
                                shadowColor: "#000000"
                                shadowOpacity: 1
                                shadowBlur: 0.25
                            }
                        }

                        Text
                        {
                            text: "<h4>Cellular</h4>"
                            anchors.left: cellularImage.right
                            color: "white"
                            height: 24
                            verticalAlignment: Text.AlignVCenter
                            leftPadding: 6

                            layer.enabled: true
                            layer.effect: MultiEffect
                            {
                                shadowEnabled: true
                                shadowColor: "#000000"
                                shadowOpacity: 1
                                shadowBlur: 0.25
                            }
                        }

                        Text
                        {
                            id: cellularPOIValue
                            anchors.right: parent.right
                            color: "white"
                            height: 24
                            verticalAlignment: Text.AlignVCenter
                            text: locationModel.cellularPointsOfInterest

                            Connections {
                                target: locationModel

                                function onCellularPointsOfInterestChanged()
                                {
                                    cellularPOIValue.text = locationModel.cellularPointsOfInterest
                                }
                            }

                            Component.onCompleted:
                            {
                                text = locationModel.cellularPointsOfInterest
                            }
                        }
                    }
                }
            }
        }
    }
}
