// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

//! [Imports]
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import QtQuick.Effects
import QtCore
import QtPositioning
import QtLocation
//! [Imports]

Item
{
    property bool append: false;
    property alias map: view.map
    property string apikey;

    anchors.fill: parent
    z: 0

    Rectangle
    {
        anchors.fill: parent

        //! [Initialize Plugin]
        Plugin
        {
            id: mapPlugin
            name: "osm"
            PluginParameter { name: "osm.useragent"; value: "wdrvr 0.1" }
            PluginParameter { name: "osm.mapping.copyright"; value: "wdrvr" }
            PluginParameter {
                name: "osm.mapping.providersrepository.disabled"
                value: "true"
            }
            PluginParameter {
                name: "osm.mapping.providersrepository.address"
                value: "http://maps-redirect.qt.io/osm/5.6/"
            }

            //! [Loaded From Settings]
            PluginParameter {
                name: "osm.mapping.highdpi_tiles"
                value: settings.highDPIMapTiles
            }
            //! [Loaded From Settings]


            // PluginParameter { name: "osm.routing.host"; value: "http://osrm.server.address/viaroute" }
            // PluginParameter { name: "osm.geocoding.host"; value: "http://geocoding.server.address" }
        }
        //! [Initialize Plugin]

        //! [Current Location]
        PositionSource
        {
            id: positionSource
            property variant lastSearchPosition: QtPositioning.coordinate(settings.latitude, settings.longitude) //Initialized/Fallback to Oslo
            active: true
            updateInterval: 1000 // 2 mins
            onPositionChanged:  {
            }
        }
        //! [Current Location]

        //! [Places MapItemView]
        MapView
        {
            id: view
            anchors.fill: parent
            map.plugin: mapPlugin;
            map.center: positionSource.lastSearchPosition
            map.zoomLevel: 13
            map.color: "#444444"
            map.copyrightsVisible: false

            // //! [Loaded From Settings]
            map.activeMapType: map.supportedMapTypes[settings.mapType]
            // //! [Loaded From Settings]

            map.onZoomLevelChanged:
            {
                locationModel.getPointsInRect(view.map.visibleRegion, view.map.zoomLevel / view.map.maximumZoomLevel);
            }

            map.onActiveMapTypeChanged: settings.mapType = map.supportedMapTypes.indexOf(map.activeMapType)

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
                        var distance = positionSource.lastSearchPosition.distanceTo(view.map.center)
                        if (distance > 500) {
                            // 500m from last performed food search
                            positionSource.lastSearchPosition = view.map.center
                            locationModel.getPointsInRect(view.map.visibleRegion, view.map.zoomLevel / view.map.maximumZoomLevel);
                        }
                    }
                }
                delegate: MapQuickItem
                {
                    coordinate: QtPositioning.coordinate(location.latitude, location.longitude)

                    anchorPoint.x: marker.width * 0.5
                    anchorPoint.y: marker.height

                    sourceItem: ColumnLayout
                    {
                        Image
                        {
                            id: marker
                            source: "icons/win11/scalable/place-marker.svg"

                            ToolTip.visible: mouseArea.containsMouse
                            ToolTip.delay: 100
                            ToolTip.text: description

                            MouseArea
                            {
                               id: mouseArea
                               anchors.fill: parent
                               hoverEnabled: true
                            }

                            // states:
                            // [
                            //     State
                            //     {
                            //         when: style === '#highConfidence'
                            //         PropertyChanges {
                            //             //marker.source: ""
                            //         }
                            //     },
                            //     State
                            //     {
                            //         when: type === 'wifi'
                            //         PropertyChanges {
                            //             marker.source: 'icons/wifi.png'
                            //         }
                            //     }

                            // ]
                        }

                        Text
                        {
                            text: name
                            font.bold: true
                            anchors.leftMargin: name.width * 0.5
                        }
                    }
                }
            }
        }

        //! [Places MapItemView]
    }

    Connections {
        target: locationModel
        function onDataChanged() {
            console.log("Updated")
            update();
            view.map.update()
        }
    }

    MultiEffect
    {
        id: effect
        source: view
        anchors.fill: parent
        blurEnabled: false
        blurMax: 32
        blur: 1.0
    }

    states:
    [
        State
        {
            name: "blocked"

            PropertyChanges
            {
                effect.blurEnabled: true
                view.enabled: false
            }
        }
    ]

    transitions: Transition {
        PropertyAnimation { easing.type: Easing.InOutQuad }
    }


    Settings
    {
        id: settings

        property string osmApiKey;
        property string iconTheme;
        property bool highDPIMapTiles;
        property int mapType;
        property string database: "default";
        property double latitude: 59.93
        property double longitude: 10.76
    }
}
