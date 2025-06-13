// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtCore
import QtQuick
import QtQuick.Controls.Material
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

        id: mapComponent
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

            FileDialog
            {
                id: fileDialog
                currentFolder: StandardPaths.standardLocations(StandardPaths.HomeLocation)[0]
                nameFilters: ["WiGLE KML files (*.kml)", "WiGLE CSV files (*.csv)"]

                onAccepted:
                {
                    locationModel.openFile(selectedFile)
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

    Component
    {
        id: chartComponent

        ChartPage {}
    }

    Loader {
        id: pageLoader
        anchors.fill: parent
        active: permission.status === Qt.PermissionStatus.Granted
        sourceComponent: mapComponent
    }

    Settings {
        id: settings
        property string iconTheme: "win11"
        property string database: "default";
    }

    Component.onCompleted: {
        locationModel.load(settings.database)
    }

    Connections
    {
        target: locationModel

        function onCurrentPageChanged()
        {
            if(locationModel.currentPage === "map")
                pageLoader.sourceComponent = mapComponent
            else if(locationModel.currentPage === "chart")
                pageLoader.sourceComponent = chartComponent
            if(locationModel.currentPage === "graph")
                pageLoader.sourceComponent = graphComponent
        }
    }
}
