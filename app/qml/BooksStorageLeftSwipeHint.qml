import QtQuick 2.0
import Sailfish.Silica 1.0

Loader {
    id: root
    anchors.fill: parent
    active: hintEnabled || hintRunning

    property bool hintEnabled
    property bool hintRunning
    signal hintShown()

    sourceComponent: Component {
        Item {
            anchors.fill: parent
            property bool cancelled
            function showHint() {
                hintRunning = true
                hintShownTimer.restart()
                touchInteractionHint.start()
            }
            Connections {
                target: root
                onHintEnabledChanged: if (root.hintEnabled) showHint();
            }
            Component.onCompleted: if (root.hintEnabled) showHint();
            InteractionHintLabel {
                //% "Swipe left to see what's on the SD-card"
                text: qsTrId("storage-view-swipe-left-hint")
                anchors.bottom: parent.bottom
                opacity: touchInteractionHint.running ? 1.0 : 0.0
                Behavior on opacity { FadeAnimation { duration: 1000 } }
            }
            TouchInteractionHint {
                id: touchInteractionHint
                direction: TouchInteraction.Left
                anchors.verticalCenter: parent.verticalCenter
                onRunningChanged: hintRunning = running
            }
            Timer {
                id: hintShownTimer
                interval: 1000
                onTriggered: root.hintShown()
            }
        }
    }
}
