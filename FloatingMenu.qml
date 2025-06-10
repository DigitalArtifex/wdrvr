import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import QtQuick.Effects
import QtCore
import QtPositioning
import QtLocation

Item {
    property Map map;
    property PlacesMap mapView;

    anchors.fill: parent

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
                                places.state = ""
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
                        icon.name: "open-document"

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

                        model: map.supportedMapTypes

                        onActivated:
                        {
                            map.activeMapType = map.supportedMapTypes[currentIndex]
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
                            mapView.state === 'blocked' ? mapView.state = "" : mapView.state = 'blocked';
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
                Layout.minimumWidth: 0
                Layout.rightMargin: 6
                Layout.topMargin: 6
                Layout.bottomMargin: 6
                Layout.alignment: Qt.AlignTop
                color: "#88000000"

                SettingsPanel
                {
                    id: settingsPanelContents
                }

                states:
                [
                    State
                    {
                        name: "open"
                        PropertyChanges
                        {
                            settingsPanelContents.state: "open"
                            settingsPanel.opacity: 1
                            settingsPanel.width: 640 < settingsPanel.parent.width ? 640 : settingsPanel.parent.width
                        }
                    }
                ]

                transitions: Transition {
                    PropertyAnimation { properties: "x,y,width,height,opacity,color"; easing.type: Easing.InOutQuad }
                }
            }
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
