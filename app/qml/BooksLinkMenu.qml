/*
  Copyright (C) 2016 Jolla Ltd.
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
    * Neither the name of Jolla Ltd nor the names of its contributors
      may be used to endorse or promote products derived from this software
      without specific prior written permission.

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

Rectangle {
    id: root
    visible: opacity > 0.0
    opacity: 0.0
    width: parent.width
    height: parent.height
    gradient: Gradient {
        GradientStop { position: 0.0; color: Theme.highlightDimmerColor }
        GradientStop { position: 1.0; color: Theme.rgba(Theme.highlightDimmerColor, 0.9) }
    }

    readonly property bool landscape: width > height

    Behavior on opacity { FadeAnimation {} }

    Column {
        width: parent.width - 2*Theme.horizontalPageMargin
        spacing: Theme.paddingMedium
        anchors {
            top: parent.top
            topMargin: Theme.paddingLarge*2
            left: parent.left
            leftMargin: Theme.horizontalPageMargin
        }

        Label {
            id: title
            anchors.horizontalCenter: parent.horizontalCenter
            //: External link menu title
            //% "Link"
            text: qsTrId("harbour-books-book-browser_link-title")
            width: root.width
            elide: Text.ElideRight
            wrapMode: Text.Wrap
            maximumLineCount: 2
            color: Theme.highlightColor
            font.pixelSize: Theme.fontSizeExtraLarge
            horizontalAlignment: Text.AlignHCenter
            opacity: 0.6
        }

        Label {
            id: linkLabel
            anchors.horizontalCenter: parent.horizontalCenter
            color: Theme.highlightColor
            width: root.width
            wrapMode: Text.Wrap
            elide: Text.ElideRight
            maximumLineCount: landscape ? 1 : 4
            font.pixelSize: Theme.fontSizeMedium
            horizontalAlignment: Text.AlignHCenter
            opacity: 0.6
        }
    }

    MouseArea {
        anchors.fill: parent
        onPressed: {
            root.hide()
        }
    }

    HighlightBar {
        id: menuHighlight
    }

    Column {
        id: menu

        anchors.bottom: parent.bottom
        anchors.bottomMargin: landscape ? Theme.paddingLarge : Theme.itemSizeSmall
        width: parent.width

        property var highlightBar: menuHighlight

        BooksMenuItem {
            //: Open link in browser
            //% "Open in browser"
            text: qsTrId("harbour-books-book-browser_link-open_in_browser")
            onClicked: {
                console.log("opening", root.url)
                root.hide()
                Qt.openUrlExternally(root.url)
            }
        }

        BooksMenuItem {
            //: Copy link to clipboard
            //% "Copy to clipboard"
            text: qsTrId("harbour-books-book-browser_link-copy_to_clipboard")
            onClicked: {
                root.hide()
                Clipboard.text = root.url
            }
        }
    }

    function show(url) {
        linkLabel.text = url
        opacity = 1.0
    }

    function hide() {
        opacity = 0.0
        menuHighlight.clearHighlight()
    }
}
