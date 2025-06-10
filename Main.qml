// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtCore
import QtQuick
import QtQuick.Controls.Fusion
import QtQuick.Layouts
import QtQuick.Dialogs

Item {
    property bool changing: false;

    id: root
    anchors.fill: parent

    LocationPermission {
        id: permission
        accuracy: LocationPermission.Precise
        availability: LocationPermission.WhenInUse
    }

    PermissionsScreen {
        anchors.fill: parent
        visible: permission.status !== Qt.PermissionStatus.Granted
        requestDenied: permission.status === Qt.PermissionStatus.Denied
        onRequestPermission: permission.request()
    }

    Component {

        id: applicationComponent
        Item {
            property alias map: places.map

            id: mapPage
            LoadingScreen {
                id: loading
                z:100
                visible: false
            }
            PlacesMap
            {
                id: places
            }
            MapLegend {}
            FloatingMenu
            {
                map: places.map
                mapView: places
            }

            FileDialog {
                id: fileDialog
                currentFolder: StandardPaths.standardLocations(StandardPaths.HomeLocation)[0]
                nameFilters: ["WiGLE XML files (*.kml)"]
                onAccepted:
                {
                    locationModel.openFile(selectedFile, true)

                    // var cord = view.map.toCoordinate(Qt.point(0, view.map.height / 2), true)
                    // var cord2 = view.map.toCoordinate(Qt.point(view.map.width / 2, view.map.height / 2), true)
                    // locationModel.getLocations(cord2, cord.distanceTo(cord2), view.map.zoomLevel / view.map.maximumZoomLevel)
                }

                onRejected:
                {
                    places.state = ""
                    loading.visible = false
                }

                onVisibleChanged:
                {
                    if(visible)
                    {
                        places.state = "blocked"
                    }
                }
            }

            MessageDialog
            {
                id: errorDialog
                buttons: MessageDialog.Ok
            }

            Connections{
                target: locationModel

                function onLoadingStarted() {
                    places.state = "blocked"
                    loading.visible = true
                }

                function onLoadingFinished() {
                    loading.visible = false
                    places.state = ""
                }

                function onError()
                {
                    errorDialog.informativeText = locationModel.errorMessage
                    errorDialog.title = locationModel.errorTitle
                    errorDialog.visible = true
                }
            }
        }
    }

    Loader {
        anchors.fill: parent
        active: permission.status === Qt.PermissionStatus.Granted
        sourceComponent: applicationComponent
    }

    Settings {
        id: settings
        property string iconTheme: "win11"
        property string database: "default";
        // property alias y: parent.y
        // property alias width: parent.width
        // property alias height: parent.height
    }

    Component.onCompleted: {
        locationModel.load(settings.database)
    }

    // Component.onDestruction: {
    //     settings.iconTheme = Icon.themeName
    // }
}
