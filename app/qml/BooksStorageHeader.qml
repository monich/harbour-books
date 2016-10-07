/*
  Copyright (C) 2015-2016 Jolla Ltd.
  Contact: Slava Monich <slava.monich@jolla.com>

  You may use this file under the terms of BSD license as follows:

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions
  are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the Jolla Ltd nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR CONTRIBUTORS
  BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
  THE POSSIBILITY OF SUCH DAMAGE.
*/

import QtQuick 2.0
import Sailfish.Silica 1.0

Column {
    id: root
    anchors {
        left: parent.left
        right: parent.right
        topMargin: Theme.paddingMedium
    }
    spacing: 0
    y: needed ? Theme.paddingMedium : -height
    property alias animationEnabled: yBehavior.enabled

    Behavior on y {
        id: yBehavior
        enabled: false
        NumberAnimation { duration: 200  }
    }

    property bool needed
    property bool removable
    property int count
    property bool showCount: true

    property int _shownCount

    signal clicked()

    function updateShownCount() {
        if (count > 0) {
            _shownCount = count
        }
    }

    onCountChanged: updateShownCount()

    Component.onCompleted: updateShownCount()

    Item {
        width: parent.width
        height: Math.max(storageLabel.height, bookCount.height)

        BooksSDCardIcon {
            id: icon
            anchors {
                left: parent.left
                leftMargin: Theme.paddingMedium
                verticalCenter: parent.verticalCenter
            }
            visible: removableStorage
            height: storageLabel.height*3/4
        }

        Label {
            id: storageLabel
            anchors {
                left: removableStorage ? icon.right : parent.left
                right: bookCount.visible ? bookCount.left : parent.right
                leftMargin: Theme.paddingMedium
                rightMargin: Theme.paddingMedium
                bottom: parent.bottom
            }
            color: (root.enabled && !mouseArea.pressed) ? Theme.primaryColor : Theme.highlightColor
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
            opacity: (needed) ? 1 : 0
            visible: opacity > 0
            Behavior on opacity { FadeAnimation {} }
        }

        Label {
            id: bookCount
            anchors {
                bottom: parent.bottom
                right: parent.right
                rightMargin: Theme.paddingMedium
            }
            //: Number of books in the storage header
            //% "%0 book(s)"
            text: qsTrId("harbour-books-storage-book_count",_shownCount).arg(_shownCount)
            font.pixelSize: Theme.fontSizeExtraSmall
            color: Theme.highlightColor
            opacity: (showCount && count > 0) ? 1 : 0
            visible: opacity > 0
            Behavior on opacity { FadeAnimation {} }
        }
    }

    Item {
        height: Theme.paddingSmall
        width: parent.width
    }

    Rectangle {
        color: Theme.secondaryHighlightColor
        height: 1
        anchors {
            left: parent.left
            right: parent.right
        }
    }

    Rectangle {
        color: Theme.highlightColor
        height: 1
        anchors {
            left: parent.left
            right: parent.right
        }
    }
}
