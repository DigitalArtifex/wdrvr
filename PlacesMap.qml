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
    anchors.fill: parent
    MenuBar
    {
        id: menu
        Menu
        {
            title: qsTr("&File")
            Action
            {
                id: importAction
                text: qsTr("&Import...")

                onTriggered:
                {
                    //qppend = false
                    fileDialog.open()
                }
            }
            Action
            {
                id: appendAction
                text: qsTr("&Append...")

                onTriggered:
                {
                    append = true
                    fileDialog.open()
                }
            }
            Action
            {
                id: saveAction
                text: qsTr("&Save")
            }
            Action
            {
                id: saveAsAction
                text: qsTr("Save &As...")
            }
            MenuSeparator { }
            Action
            {
                id: quitAction
                text: qsTr("&Quit")

                onTriggered:
                {
                    Qt.exit(0)
                }
            }
        }
        height: 35
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
    }
    Rectangle
    {
        Layout.fillWidth: true
        Layout.fillHeight: true
        anchors.top: menu.bottom
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right

        //! [Initialize Plugin]
        Plugin
        {
            id: myPlugin
            name: "osm"
            //specify plugin parameters if necessary
            //PluginParameter {...}
            //PluginParameter {...}
            //...
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

                MouseArea {
                    anchors.fill: parent
                    property variant startCoordinate
                    property bool panning: false

                    onPressed: {
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

                    onReleased: {
                    }

                }
                delegate: MapQuickItem
                {
                    coordinate: QtPositioning.coordinate(location.x, location.y)

                    anchorPoint.x: image.width * 0.5
                    anchorPoint.y: image.height

                    sourceItem: Column
                    {
                        Image
                        {
                            id: image
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
                var file = selectedFile.toLowerCase;
                var endsWithXml = /kml$/;

                //if (endsWithXml.test(file))
                    locationModel.parseKML(selectedFile, append)
                var cord = view.map.toCoordinate(Qt.point(0,0))
                var cord2 = view.map.toCoordinate(Qt.point(view.map.width / 2, view.map.height / 2))
                locationModel.getLocations(cord2, cord.distanceTo(cord2), view.map.zoomLevel / view.map.maximumZoomLevel)
            }
        }
    }

    Connections {
        // target: locationModel
        // function onDataChanged() {
        //     console.log("Updated")
        //     update();
        //     view.map.update()
        // }
    }

}
