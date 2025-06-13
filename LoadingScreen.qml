import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Effects

Item
{
    anchors.fill: parent

    Rectangle
    {
        anchors.fill: parent
        color: "transparent"

        Text
        {
            id: loadingText
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.verticalCenter
            anchors.horizontalCenter: parent.horizontalCenter

            text: "<h1>" + locationModel.loadingTitle + "</h1>"
            color: "black"
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter

            MultiEffect
            {
                source: loadingText
                anchors.fill: loadingText

                shadowEnabled: true
                shadowHorizontalOffset: 2
                shadowVerticalOffset: 2
                shadowColor: "#ffffff"
            }
        }

        ProgressBar
        {
            id: loadingProgress
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.verticalCenter
            anchors.horizontalCenter: parent.horizontalCenter

            leftInset: 12
            rightInset: 12

            from: 0
            to: 1

            Connections
            {
                target: locationModel

                function onProgressChanged()
                {
                    if(locationModel.progress === -1)
                        loadingProgress.indeterminate = true;
                    else
                    {
                        loadingProgress.indeterminate = false;
                        loadingProgress.value = locationModel.progress;
                    }
                }
            }
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
}
