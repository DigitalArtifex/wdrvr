import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtCore

Item
{
    anchors.fill: parent
    Settings
    {
        id: settings

        property string osmApiKey;
        property string iconTheme;
        property bool highDPIMapTiles;
        property int mapType;
        property string database: "default";
    }

    Rectangle
    {
        id: content
        anchors.fill: parent
        color: "transparent"
        opacity: 0

        ColumnLayout
        {
            anchors.fill: parent
            spacing: 0

            RowLayout
            {
                Layout.alignment: Qt.AlignTop
                Layout.fillWidth: true
                Layout.fillHeight: false

                Text {
                    Layout.fillWidth: true
                    Layout.leftMargin: 12
                    Layout.topMargin: 24
                    text: qsTr("<h3>Database</h3>")
                    color: "white"
                }

                ComboBox
                {
                    id: databaseSelector
                    Layout.fillWidth: true
                    Layout.rightMargin: 6
                    Layout.preferredHeight: 35

                    model: locationModel.availableDatabases
                }

                Button
                {
                    Layout.preferredHeight: 35
                    icon.name: "create"
                    icon.width: 24
                    icon.height: 24
                    Layout.rightMargin: 6

                    onClicked: {
                        databaseNameDialog.visible = true
                    }
                }

                Button
                {
                    Layout.preferredHeight: 35
                    icon.name: "open"
                    icon.width: 24
                    icon.height: 24
                    Layout.rightMargin: 6

                    onClicked: {

                        settings.database = locationModel.availableDatabases[databaseSelector.currentIndex]
                        locationModel.database = settings.database
                        locationModel.load(settings.database)
                    }
                }
            }

            RowLayout
            {
                Layout.alignment: Qt.AlignTop
                Layout.fillWidth: true
                Layout.fillHeight: false

                Text {
                    Layout.fillWidth: true
                    Layout.leftMargin: 12
                    Layout.topMargin: 24
                    text: qsTr("<h3>Icon Theme</h3>")
                    color: "white"
                }

                ComboBox
                {
                    Layout.fillWidth: true
                    Layout.rightMargin: 6
                    Layout.preferredHeight: 35

                    model: Icon.availableThemes

                    onActivated:
                    {
                        Icon.themeName = Icon.availableThemes[currentIndex]
                    }
                }
            }

            RowLayout
            {
                Layout.alignment: Qt.AlignTop
                Layout.fillWidth: true
                Layout.fillHeight: false

                Text {
                    Layout.fillWidth: true
                    Layout.leftMargin: 12
                    Layout.rightMargin: 6
                    Layout.topMargin: 24
                    text: qsTr("<h3>Open Street Map API Key</h3>")
                    color: "white"
                }

                TextField
                {
                    Layout.fillWidth: true
                    Layout.rightMargin: 6
                    Layout.preferredHeight: 35

                    text: settings.osmApiKey

                    onAccepted:
                    {
                        settings.osmApiKey = text
                    }
                }
            }

            RowLayout
            {
                Layout.alignment: Qt.AlignTop
                Layout.fillWidth: true
                Layout.fillHeight: false

                Text {
                    Layout.fillWidth: true
                    Layout.leftMargin: 12
                    Layout.topMargin: 12
                    text: qsTr("<h3>High DPI Map Tiles</h3>")
                    color: "white"
                }

                Switch
                {
                    Layout.alignment: Qt.AlignRight
                    height: 50
                    width: 50
                    checked: settings.highDPIMapTiles
                    onCheckedChanged: settings.highDPIMapTiles = checked
                }
            }

            Rectangle { color:"transparent"; Layout.fillHeight: true; }

            Button
            {
                Layout.fillWidth: true
                Layout.preferredHeight: 50
                text: "Delete Database"
                icon.name: "delete-document"
                icon.width: 24
                icon.height: 24
            }
        }
    }

    Dialog
    {
        id: databaseNameDialog
        standardButtons: Dialog.Ok | Dialog.Cancel
        title: "New Database"
        visible: false

        width: 360
        height: 180

        anchors.centerIn: parent

        ColumnLayout
        {
            anchors.fill: parent

            Text
            {
                Layout.fillWidth: true
                id: databaseNameDialogText
                text: qsTr("Name for the new database")
                color: "white"
            }

            TextField
            {
                Layout.fillWidth: true
                id: databaseNameDialogInput
            }
        }

        onAccepted: {
            locationModel.createDatabase(databaseNameDialogInput.text);
            databaseNameDialogInput.text = "";
        }
    }

    states:
    [
        State
        {
            name: "open"
            PropertyChanges
            {
                content.opacity: 1
            }
        }
    ]

    transitions: Transition {
        PropertyAnimation { properties: "x,y,width,height,opacity,color"; easing.type: Easing.InOutQuad }
    }

    function onVisibleChanged() {
        databaseSelector.currentIndex = locationModel.availableDatabases.indexOf(settings.database)
    }
}
