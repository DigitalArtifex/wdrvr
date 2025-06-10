import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Effects
import Qt5Compat.GraphicalEffects

Item {
    anchors.fill: parent
    ColumnLayout
    {
        anchors.fill: parent
        z: 11

        Rectangle
        {
            id: legendPanel

            Layout.preferredWidth: 320
            Layout.preferredHeight: 180
            Layout.minimumHeight: 160
            Layout.minimumWidth: 90
            Layout.alignment: Qt.AlignBottom | Qt.AlignRight
            Layout.rightMargin: 6
            Layout.topMargin: 6
            Layout.leftMargin: 6
            Layout.bottomMargin: 6
            color: "#00444444"
            radius: 12

            Image
            {
                id: background
                anchors.fill: parent
                fillMode: Image.PreserveAspectCrop
                opacity: 0
                clip: true
                source: "images/white_textured_wallpaper_small.png"
            }

            MultiEffect
            {
                source: background
                id: effect
                anchors.fill: background
                blurEnabled: true
                blurMax: 32
                blur: 1.0
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
                            }

                            Text
                            {
                                text: "<h4>Total</h4>"
                                anchors.left: locationImage.right
                                color: "white"
                                leftPadding: 12
                                height: 24
                                verticalAlignment: Text.AlignVCenter
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
                            }

                            Text
                            {
                                text: "<h4>WiFi</h4>"
                                color: "white"
                                anchors.left: wifiImage.right
                                height: 24
                                verticalAlignment: Text.AlignVCenter
                                leftPadding: 6
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
                            }

                            Text
                            {
                                text: "<h4>Bluetooth</h4>"
                                anchors.left: bluetoothImage.right
                                color: "white"
                                height: 24
                                verticalAlignment: Text.AlignVCenter
                                leftPadding: 6
                            }

                            Text
                            {
                                anchors.right: parent.right
                                text: "0"
                                color: "white"
                                height: 24
                                verticalAlignment: Text.AlignVCenter

                                Connections {
                                    target: locationModel

                                    function onBluetoothPointsOfInterestChanged()
                                    {
                                        text = locationModel.bluetoothPointsOfInterest
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
                            }

                            Text
                            {
                                text: "<h4>Cellular</h4>"
                                anchors.left: cellularImage.right
                                color: "white"
                                height: 24
                                verticalAlignment: Text.AlignVCenter
                                leftPadding: 6
                            }

                            Text
                            {
                                anchors.right: parent.right
                                color: "white"
                                height: 24
                                verticalAlignment: Text.AlignVCenter
                                text: "0"

                                Connections {
                                    target: locationModel

                                    function onCellularPointsOfInterestChanged()
                                    {
                                        text = locationModel.cellularPointsOfInterest
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
                Rectangle
                {
                    Layout.fillHeight: true;
                    Layout.fillWidth: true;
                    color: "transparent"
                    Layout.rightMargin: 6
                    Layout.topMargin: 6
                    Layout.leftMargin: 6
                    Layout.bottomMargin: 6

                    RowLayout
                    {
                        // anchors.fill: parent
                        // Rectangle
                        // {
                        //     Layout.fillHeight: true;
                        //     Layout.fillWidth: false;
                        //     Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                        //     color: "transparent"
                        //     Image
                        //     {
                        //         id: fourGImage
                        //         width: 24
                        //         height: 24
                        //         smooth: true
                        //         fillMode: Image.PreserveAspectFit
                        //         antialiasing: true
                        //         verticalAlignment: Image.AlignVCenter

                        //         Connections {
                        //             target: Icon

                        //             function onThemeNameChanged()
                        //             {
                        //                 fourGImage.source = Icon.fromTheme("scalable/4g.svg");
                        //             }
                        //         }

                        //         //satisfy pre-load conditions
                        //         Component.onCompleted:
                        //         {
                        //             fourGImage.source = Icon.fromTheme("scalable/4g.svg")
                        //         }
                        //     }

                        //     Text
                        //     {
                        //         text: "<h4>4G</h4>"
                        //         anchors.left: fourGImage.right
                        //         color: "white"
                        //         height: 24
                        //         verticalAlignment: Text.AlignVCenter
                        //     }
                        // }
                        // Rectangle
                        // {
                        //     Layout.fillHeight: true;
                        //     Layout.fillWidth: false;
                        //     Layout.preferredHeight: 15
                        // }
                    }
                }
            }
        }
    }
}
