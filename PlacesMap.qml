// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

//! [Imports]
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import QtCore
import QtPositioning
import QtLocation
//! [Imports]


Rectangle
{
    property bool append: false;
    property bool changing: false;
    anchors.fill: parent

    Rectangle
    {
        anchors.fill: parent

        //! [Initialize Plugin]
        Plugin
        {
            id: myPlugin
            name: "osm"
            PluginParameter { name: "osm.useragent"; value: "wdrvr 0.1" }
            PluginParameter { name: "osm.mapping.copyright"; value: "" }
            // PluginParameter { name: "osm.routing.host"; value: "http://osrm.server.address/viaroute" }
            // PluginParameter { name: "osm.geocoding.host"; value: "http://geocoding.server.address" }
        }
        //! [Initialize Plugin]

        //! [Current Location]
        PositionSource
        {
            id: positionSource
            property variant lastSearchPosition: QtPositioning.coordinate(59.93, 10.76) //Initialized/Fallback to Oslo
            active: true
            updateInterval: 120000 // 2 mins
            onPositionChanged:  {
                var distance = lastSearchPosition.distanceTo(position.coordinate)
                if (distance > 100) {
                    // 500m from last performed food search
                    lastSearchPosition = positionSource.position.coordinate
                    locationModel.getLocations(positionSource.position.coordinate, 500, view.map.zoomLevel / view.map.maximumZoomLevel)
                }
            }
        }
        //! [Current Location]

        //! [Places MapItemView]
        MapView
        {
            id: view
            anchors.fill: parent
            map.plugin: myPlugin;
            map.center: positionSource.lastSearchPosition
            map.zoomLevel: 13

            map.onZoomLevelChanged:
            {
                var cord = view.map.toCoordinate(Qt.point(0,0))
                var cord2 = view.map.toCoordinate(Qt.point(view.map.width / 2, view.map.height / 2))
                locationModel.getLocations(cord2, cord.distanceTo(cord2), view.map.zoomLevel / view.map.maximumZoomLevel)
            }

            MapItemView
            {
                model: locationModel
                parent: view.map

                MouseArea
                {
                    anchors.fill: parent
                    property variant startCoordinate
                    property bool panning: false

                    onPressed:
                    {
                         startCoordinate = view.map.center
                         panning = true
                    }

                    onPositionChanged:
                    {
                        if(panning)
                        {
                            var cord = view.map.toCoordinate(Qt.point(0,0))
                            var cord2 = view.map.toCoordinate(Qt.point(view.map.width / 2, view.map.height / 2))
                            locationModel.getLocations(cord2, cord.distanceTo(cord2), view.map.zoomLevel / view.map.maximumZoomLevel)
                        }
                    }
                }
                delegate: MapQuickItem
                {
                    coordinate: QtPositioning.coordinate(location.latitude, location.longitude)

                    anchorPoint.x: marker.width * 0.5
                    anchorPoint.y: marker.height

                    sourceItem: Column
                    {
                        Image
                        {
                            id: marker
                            source: "marker.png"

                            ToolTip.visible: mouseArea.containsMouse
                            ToolTip.delay: 100
                            ToolTip.text: description

                            MouseArea
                            {
                               id: mouseArea
                               anchors.fill: parent
                               hoverEnabled: true
                            }

                            states:
                            [
                                State
                                {
                                    when: style === '#highConfidence'
                                    PropertyChanges {
                                        marker.source: ""
                                    }
                                }
                            ]
                        }

                        Text
                        {
                            text: name
                            font.bold: true
                        }
                    }
                }
            }
        }

        //! [Places MapItemView]
        FileDialog {
            id: fileDialog
            currentFolder: StandardPaths.standardLocations(StandardPaths.HomeLocation)[0]
            nameFilters: ["WiGLE XML files (*.kml)"]
            onAccepted:
            {
                locationModel.openFile(selectedFile, append)

                var cord = view.map.toCoordinate(Qt.point(0, view.map.height / 2), true)
                var cord2 = view.map.toCoordinate(Qt.point(view.map.width / 2, view.map.height / 2), true)
                locationModel.getLocations(cord2, cord.distanceTo(cord2), view.map.zoomLevel / view.map.maximumZoomLevel)
            }
        }
    }

    Rectangle
    {
        anchors.fill: parent

        width: 74
        height: 74
        z: 12
        id: menuControl
        color: "transparent"

        ColumnLayout
        {
            anchors.fill: parent
            z: 12
            Rectangle
            {
                id: menuBox
                // width: 62
                // height: 62
                color: "#88000000"

                Layout.topMargin: 6
                Layout.rightMargin: 6
                Layout.alignment: Qt.AlignTop | Qt.AlignLeft
                Layout.minimumHeight: 62
                Layout.minimumWidth: 62
                Layout.preferredWidth: 62

                states:
                [
                    State {
                        name: "open"
                        PropertyChanges
                        {
                            menuBox.width: 640 < menuBox.parent.width ? 640 : menuBox.parent.width
                            menuControl.width: 652 < menuControl.parent.width ? 652 : menuControl.parent.width
                            menuButton.state: 'open'
                            importFileButton.opacity: 1
                            mapSelector.opacity: 1
                            menuBox.color: "#88000000"
                            settingsButton.opacity: 1
                        }
                    }
                ]

                RowLayout
                {
                    anchors.fill: parent
                    spacing: 0

                    Button
                    {
                        id: menuButton
                        icon.name: "menu"
                        Layout.minimumHeight: 50
                        Layout.minimumWidth: 50
                        Layout.preferredWidth: 50
                        Layout.rightMargin: 6
                        Layout.leftMargin: 6

                        opacity: 1
                        onClicked:
                        {
                            if(menuBox.state === 'open')
                            {
                                menuBox.state = ""
                                settingsPanel.state === 'open' ? settingsPanel.state = "" : 0;
                                settingsButton.checked = false
                            }
                            else
                                menuBox.state = 'open';

                        }
                        checked: false;
                        checkable: true;

                        transitions: Transition {
                            PropertyAnimation { properties: "x,y,width,height,opacity,color"; easing.type: Easing.InOutQuad }
                        }
                    }

                    Button
                    {
                        id: importFileButton
                        icon.name: "unknown-document"

                        Layout.minimumHeight: 50
                        Layout.minimumWidth: 50
                        Layout.preferredWidth: 50
                        Layout.rightMargin: 6

                        onClicked:
                        {
                            if(!fileDialog.visible)
                                fileDialog.open();

                            menuBox.state = "";
                        }
                        opacity: 0

                        transitions: Transition {
                            PropertyAnimation { properties: "x,y,width,height,opacity,color"; easing.type: Easing.InOutQuad }
                        }
                    }

                    ComboBox
                    {
                        Layout.fillWidth: true
                        Layout.minimumWidth: 100
                        Layout.preferredWidth: 200
                        Layout.minimumHeight: 50
                        Layout.rightMargin: 6

                        textRole: "description"

                        id: mapSelector
                        height: 50

                        model: view.map.supportedMapTypes

                        onActivated:
                        {
                            view.map.activeMapType = view.map.supportedMapTypes[currentIndex]
                            menuBox.state = ""
                        }
                        opacity: 0

                        transitions: Transition {
                            PropertyAnimation { properties: "x,y,width,height,opacity,color"; easing.type: Easing.InOutQuad }
                        }
                    }

                    Button
                    {
                        id: settingsButton
                        icon.name: "settings"

                        Layout.minimumHeight: 50
                        Layout.minimumWidth: 50
                        Layout.preferredWidth: 50
                        Layout.rightMargin: 6

                        checkable: true
                        checked: false

                        states:
                        [
                            State {
                                name: "open"
                                PropertyChanges
                                {
                                    settingsPanel.state: "open"
                                    settingsPanel.opacity: 1
                                }
                            }
                        ]

                        onClicked:
                        {
                            settingsPanel.state === 'open' ? settingsPanel.state = "" : settingsPanel.state = 'open';
                        }
                        opacity: 0

                        transitions: Transition {
                            PropertyAnimation { properties: "x,y,width,height,opacity,color"; easing.type: Easing.InOutQuad }
                        }
                    }
                }
                transitions: Transition {
                    PropertyAnimation { properties: "x,y,width,height,opacity,color"; easing.type: Easing.InOutQuad }
                }
            }

            Rectangle
            {
                id: settingsPanel
                Layout.fillHeight: true
                Layout.preferredWidth: 0
                Layout.minimumHeight: 0
                Layout.minimumWidth: 0
                Layout.rightMargin: 6
                Layout.topMargin: 6
                Layout.bottomMargin: 6
                Layout.alignment: Qt.AlignTop
                color: "#88000000"

                states:
                [
                    State
                    {
                        name: "open"
                        PropertyChanges
                        {
                            settingsPanel.width: 646 < settingsPanel.parent.width ? 646 : settingsPanel.parent.width
                        }
                    }
                ]

                transitions: Transition {
                    PropertyAnimation { properties: "x,y,width,height,opacity,color"; easing.type: Easing.InOutQuad }
                }
            }
        }
    }

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
            Layout.rightMargin: 6
            Layout.topMargin: 6
            Layout.bottomMargin: 6
            Layout.alignment: Qt.AlignBottom | Qt.AlignRight
            color: "#AA000000"

            ColumnLayout
            {
                anchors.fill: parent
                Rectangle
                {
                    Layout.fillHeight: true;
                    Layout.fillWidth: true;
                    color: "transparent"
                    Layout.topMargin: 12
                    Layout.leftMargin: 12
                    Layout.rightMargin: 12

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
                                source: "icons/location.png"
                                verticalAlignment: Image.AlignVCenter
                            }

                            Text
                            {
                                text: "<h3>Total</h3>"
                                anchors.left: locationImage.right
                                color: "white"
                                leftPadding: 6
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
                                source: "icons/wifi.png"
                                anchors.left: parent.left
                                verticalAlignment: Image.AlignVCenter
                            }

                            Text
                            {
                                text: "<h3>WiFi</h3>"
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
                    Layout.fillHeight: true;
                    Layout.fillWidth: true;
                    color: "transparent"
                    Layout.topMargin: 12
                    Layout.leftMargin: 12
                    Layout.rightMargin: 12

                    RowLayout
                    {
                        Layout.topMargin: 6

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
                                source: "icons/bluetooth.png"
                                anchors.left: parent.left
                                verticalAlignment: Image.AlignVCenter
                            }

                            Text
                            {
                                text: "<h3>Bluetooth</h3>"
                                anchors.left: bluetoothImage.right
                                color: "white"
                                height: 24
                                verticalAlignment: Text.AlignVCenter
                                leftPadding: 6
                            }

                            Text
                            {
                                anchors.right: parent.right
                                text: locationModel.bluetoothPointsOfInterest
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
                                id: cellularImage
                                width: 24
                                height: 24
                                smooth: true
                                fillMode: Image.PreserveAspectFit
                                antialiasing: true
                                source: "icons/radio.png"
                                anchors.left: parent.left
                                verticalAlignment: Image.AlignVCenter
                            }

                            Text
                            {
                                text: "<h3>Cellular</h3>"
                                anchors.left: cellularImage.right
                                color: "white"
                                height: 24
                                verticalAlignment: Text.AlignVCenter
                                leftPadding: 6
                            }

                            Text
                            {
                                anchors.right: parent.right
                                text: locationModel.cellularPointsOfInterest
                                color: "white"
                                height: 24
                                verticalAlignment: Text.AlignVCenter
                            }
                        }
                    }
                }
            }
        }
    }

    Connections {
        target: locationModel
        function onDataChanged() {
            console.log("Updated")
            update();
            view.map.update()
        }
    }

    onWidthChanged:
    {
        onResize()
    }

    onHeightChanged:
    {
        onResize()
    }

    function onResize()
    {
        if(!changing)
        {
            changing = true

            var menuState = menuBox.state
            var settingsState = settingsPanel.state
            menuBox.state = ""
            settingsPanel.state = ""
            menuBox.state = menuState
            settingsPanel.state = settingsState

            changing = false
        }
    }
}
