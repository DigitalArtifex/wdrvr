import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtGraphs

Item {
    anchors.fill: parent
    RowLayout
    {
        spacing: 0
        anchors.fill: parent
        GraphsView
        {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.preferredWidth: parent.width / 2
            theme: GraphsTheme
            {
                colorScheme: GraphsTheme.ColorScheme.Dark
                theme: GraphsTheme.Theme.BlueSeries
            }
            PieSeries
            {
                id: pieSeries
                PieSlice
                {
                    id: wifiSlice
                    value: locationModel.wifiStats
                    label: "<h3>WIFI</h3>"
                    labelVisible: (locationModel.wifiStats > 0)

                    property HoverHandler hoverHandler: HoverHandler
                    {
                        target: wifiSlice
                        acceptedDevices: PointerDevice.Mouse | PointerDevice.TouchPad
                        cursorShape: Qt.PointingHandCursor

                        onHoveredChanged: {
                            wifiSlice.exploded = hovered
                        }
                    }
                }
                PieSlice
                {
                    id: btSlice
                    value: locationModel.bluetoothStats
                    label: "<h3>Bluetooth</h3>"
                    labelVisible: (locationModel.bluetoothStats > 0)
                }
                PieSlice
                {
                    id: btLESlice
                    value: locationModel.bluetoothLEStats
                    label: "<h3>BluetoothLE</h3>"
                    labelVisible: (locationModel.bluetoothLEStats > 0)
                }
                PieSlice
                {
                    id: gsmSlice
                    value: locationModel.gsmStats
                    label: "<h3>GSM</h3>"
                    labelVisible: (locationModel.gsmStats > 0)
                }
                PieSlice
                {
                    id: cdmaSlice
                    value: locationModel.cdmaStats
                    label: "<h3>CDMA</h3>"
                    labelVisible: (locationModel.cdmaStats > 0)
                }
                PieSlice
                {
                    id: wcdmaSlice
                    value: locationModel.wcdmaStats
                    label: "<h3>WCDMA</h3>"
                    labelVisible: (locationModel.wcdmaStats > 0)
                }
                PieSlice
                {
                    id: nrSlice
                    value: locationModel.nrStats
                    label: "<h3>5G</h3>"
                    labelVisible: (locationModel.nrStats > 0)
                }
                PieSlice
                {
                    id: lteSlice
                    value: locationModel.lteStats
                    label: "<h3>4G LTE</h3>"
                    labelVisible: (locationModel.lteStats > 0)
                }
            }
        }

        Rectangle
        {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.preferredWidth: parent.width / 6
            Layout.minimumWidth: 200
            color: "#333333"
            ColumnLayout
            {
                anchors.fill: parent
                Text
                {
                    id: headerText
                    Layout.topMargin: 100
                    Layout.bottomMargin: 20
                    Layout.alignment: Qt.AlignTop
                    Layout.fillWidth: true
                    Layout.fillHeight: false
                    text:"<h2><u>Radio Frequency Stats</u></h2>"
                    horizontalAlignment: Text.AlignHCenter
                    color: "#ffffff"
                }
                Text
                {
                    Layout.topMargin: 12
                    Layout.alignment: Qt.AlignTop
                    Layout.leftMargin: 12
                    Layout.fillWidth: true
                    text:"<h3>WiFi Locations: </h3>" + locationModel.wifiStats
                    visible: (locationModel.wifiStats > 0)
                    color: "#ffffff"
                }
                Text
                {
                    Layout.topMargin: 12
                    Layout.alignment: Qt.AlignTop
                    Layout.leftMargin: 12
                    Layout.fillWidth: true
                    text:"<h3>Bluetooth Locations: </h3>" + locationModel.bluetoothStats
                    visible: (locationModel.bluetoothStats > 0)
                    color: "#ffffff"
                }
                Text
                {
                    Layout.topMargin: 12
                    Layout.alignment: Qt.AlignTop
                    Layout.leftMargin: 12
                    Layout.fillWidth: true
                    text:"<h3>BluetoothLE Locations: </h3>" + locationModel.bluetoothLEStats
                    visible: (locationModel.bluetoothLEStats > 0)
                    color: "#ffffff"
                }
                Text
                {
                    Layout.topMargin: 12
                    Layout.alignment: Qt.AlignTop
                    Layout.leftMargin: 12
                    Layout.fillWidth: true
                    text:"<h3>GSM Locations: </h3>" + locationModel.gsmStats
                    visible: (locationModel.gsmStats > 0)
                    color: "#ffffff"
                }
                Text
                {
                    Layout.topMargin: 12
                    Layout.alignment: Qt.AlignTop
                    Layout.leftMargin: 12
                    Layout.fillWidth: true
                    text:"<h3>CDMA Locations: </h3>" + locationModel.cdmaStats
                    visible: (locationModel.cdmaStats > 0)
                    color: "#ffffff"
                }
                Text
                {
                    Layout.topMargin: 12
                    Layout.alignment: Qt.AlignTop
                    Layout.leftMargin: 12
                    Layout.fillWidth: true
                    text:"<h3>WCDMA Locations: </h3>" + locationModel.wcdmaStats
                    visible: (locationModel.wcdmaStats > 0)
                    color: "#ffffff"
                }
                Text
                {
                    Layout.topMargin: 12
                    Layout.alignment: Qt.AlignTop
                    Layout.leftMargin: 12
                    Layout.fillWidth: true
                    text:"<h3>4G LTE Locations: </h3>" + locationModel.lteStats
                    visible: (locationModel.lteStats > 0)
                    color: "#ffffff"
                }
                Text
                {
                    Layout.topMargin: 12
                    Layout.alignment: Qt.AlignTop
                    Layout.leftMargin: 12
                    Layout.fillWidth: true
                    text:"<h3>5G Locations: </h3>" + locationModel.nrStats
                    visible: (locationModel.nrStats > 0)
                    color: "#ffffff"
                }
                Text
                {
                    Layout.topMargin: 12
                    Layout.alignment: Qt.AlignTop
                    Layout.leftMargin: 12
                    Layout.fillWidth: true
                    text:"<h3>MACs Per Second Average: </h3>" + locationModel.mpsAverage
                    color: "#ffffff"
                }

                Rectangle
                {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    color: "transparent"
                }
            }
        }

        // HoverHandler
        // {
        //     target: wifiSlice
        //     acceptedDevices: PointerDevice.Mouse | PointerDevice.TouchPad
        //     cursorShape: Qt.PointingHandCursor

        //     onHoveredChanged: {
        //         wifiSlice.exploded = hovered
        //     }
        // }

        // HoverHandler
        // {
        //     target: btSlice
        //     acceptedDevices: PointerDevice.Mouse | PointerDevice.TouchPad
        //     cursorShape: Qt.PointingHandCursor

        //     onHoveredChanged: {
        //         btSlice.exploded = hovered
        //         btSlice.explodeDistanceFactor = 0.5
        //         btLESlice.exploded = hovered
        //         btLESlice.explodeDistanceFactor = 0.25
        //     }
        // }

        // HoverHandler
        // {
        //     target: btLESlice
        //     acceptedDevices: PointerDevice.Mouse | PointerDevice.TouchPad
        //     cursorShape: Qt.PointingHandCursor

        //     onHoveredChanged: {
        //         btSlice.exploded = hovered
        //         btSlice.explodeDistanceFactor = 0.25
        //         btLESlice.exploded = hovered
        //         btLESlice.explodeDistanceFactor = 0.5
        //     }
        // }

        // HoverHandler
        // {
        //     target: gsmSlice
        //     acceptedDevices: PointerDevice.Mouse | PointerDevice.TouchPad
        //     cursorShape: Qt.PointingHandCursor

        //     onHoveredChanged: {
        //         gsmSlice.exploded = hovered
        //         gsmSlice.explodeDistanceFactor = 0.5
        //         cdmaSlice.exploded = hovered
        //         cdmaSlice.explodeDistanceFactor = 0.25
        //         wcdmaSlice.exploded = hovered
        //         wcdmaSlice.explodeDistanceFactor = 0.25
        //     }
        // }
    }

    Button
    {
        width: 50
        height: 50
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.topMargin: 12
        anchors.rightMargin: 12
        icon.name: "cancel"
        icon.width: 32
        icon.height: 32

        onClicked:
        {
            locationModel.currentPage = "map"
        }
    }
}
