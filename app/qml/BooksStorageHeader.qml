import QtQuick 2.0
import Sailfish.Silica 1.0

import "harbour"
import "Books.js" as Books

Column {
    id: root

    property bool isPortrait: true
    property bool needed
    property bool removable
    property int count
    property bool showCount: true
    property alias animationEnabled: yBehavior.enabled

    signal clicked()

    spacing: 0
    y: needed ? Theme.paddingMedium : -height

    Behavior on y {
        id: yBehavior

        enabled: false
        NumberAnimation { duration: 200  }
    }

    Item {
        width: parent.width
        height: Math.max(storageLabel.y + storageLabel.height, bookCount.y + bookCount.height)

        HarbourHighlightIcon {
            id: icon

            anchors {
                left: parent.left
                leftMargin: Theme.paddingMedium
                verticalCenter: parent.verticalCenter
            }
            visible: removableStorage
            height: storageLabel.height*3/4
            sourceSize.height: height
            fillMode: Image.PreserveAspectFit
            source: "images/sdcard.svg"
            highlightColor: storageLabel.color
        }

        Label {
            id: storageLabel

            x: (icon.visible ? (icon.x + icon.width) : 0) + Theme.paddingMedium
            width: ((isPortrait && Books.topNotchHeight > 0) ? (Books.topNotchLeft - root.x) :
                ((bookCount.visible ? bookCount.x : parent.width) - Theme.paddingMedium)) - x
            color: (root.enabled && !mouseArea.pressed) ? Theme.primaryColor : Theme.highlightColor
            truncationMode: TruncationMode.Fade
            text: removable ?
                //: Header label for the memory card
                //% "Memory card"
                qsTrId("harbour-books-storage-removable") :
                //: Header label for the internal storage
                //% "Internal storage"
                qsTrId("harbour-books-storage-internal")

            MouseArea {
                id: mouseArea

                anchors.fill: parent
                onClicked: root.clicked()
            }

            Behavior on color { ColorAnimation { duration: 100 } }

            // The label overlaps with the Sailfish 2.0 pulley menu which
            // doesn't look great. Hide it when it's not needed. The book
            // count can be left there, it doesn't overlap with anything
            opacity: needed ? 1 : 0
            visible: opacity > 0
            Behavior on opacity { FadeAnimation {} }
        }

        Label {
            id: bookCount

            y: Theme.paddingSmall
            anchors {
                right: parent.right
                rightMargin: Theme.paddingMedium
            }
            //: Number of books in the storage header
            //% "%0 book(s)"
            text: qsTrId("harbour-books-storage-book_count",count).arg(count)
            font.pixelSize: Theme.fontSizeExtraSmall
            color: Theme.highlightColor
            visible: (showCount && needed && count > 0) ? 1 : 0
        }
    }

    Item {
        height: Theme.paddingSmall
        width: 1
    }
}
