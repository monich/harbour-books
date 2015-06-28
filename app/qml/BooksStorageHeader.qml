/*
  Copyright (C) 2015 Jolla Ltd.
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
    anchors {
        top: parent.top
        left: parent.left
        right: parent.right
        topMargin: Theme.paddingMedium
    }
    spacing: 0
    visible: opacity > 0
    opacity: singleStorage ? 0 : 1
    Behavior on opacity { FadeAnimation {} }

    property bool removable
    property int count
    property bool showCount: true

    Item {
        width: parent.width
        height: Math.max(left.height, right.height)
        Row {
            id: left
            anchors {
                left: parent.left
                verticalCenter: parent.verticalCenter
            }
            spacing: 0
            Item {
                width: Theme.paddingMedium
                height: parent.height
            }
            BooksSDCardIcon {
                visible: removableStorage
                anchors.bottom: parent.bottom
                height: storageLabel.height*3/4
            }
            Item {
                visible: removableStorage
                width: Theme.paddingMedium
                height: parent.height
            }
            Label {
                id: storageLabel
                anchors.bottom: parent.bottom
                color: Theme.highlightColor
                text: removable ?
                    //% "Memory card"
                    qsTrId("storage-removable") :
                    //% "Internal storage"
                    qsTrId("storage-internal")
            }
        }
        Label {
            id: right
            anchors {
                bottom: parent.bottom
                right: parent.right
                rightMargin: Theme.paddingMedium
            }
            //% "%0 book(s)"
            text: qsTrId("storage-book-count",count).arg(count)
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
