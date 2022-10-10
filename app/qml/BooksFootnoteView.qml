/*
  Copyright (C) 2015-2021 Jolla Ltd.
  Copyright (C) 2015-2021 Slava Monich <slava.monich@jolla.com>

  You may use this file under the terms of BSD license as follows:

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions
  are met:

    1. Redistributions of source code must retain the above copyright
       notice, this list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright
       notice, this list of conditions and the following disclaimer
       in the documentation and/or other materials provided with the
       distribution.
    3. Neither the names of the copyright holders nor the names of its
       contributors may be used to endorse or promote products derived
       from this software without specific prior written permission.

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
import harbour.books 1.0

import "Books.js" as Books

Rectangle {
    id: root

    visible: opacity > 0
    opacity: 0.0
    anchors.fill: parent
    color: Theme.rgba(Theme.highlightDimmerColor, 0.9)

    readonly property real footnoteX: Math.round((root.width-footnoteItem.width)/2)
    readonly property real footnoteY: Theme.paddingMedium

    Behavior on opacity { FadeAnimation {} }

    function show(startX,startY,text,url) {
        flickable.scrollToTop()
        content.source = url
        if (state !== "show") {
            footnoteItem.scale = 0
            footnoteItem.x = startX
            footnoteItem.y = startY
            footnoteLabel.text = text
            state = "show"
        }
    }

    function cancel() {
        state = "cancel"
    }

    function hide() {
        state = "hide"
    }

    Connections {
        target: Settings
        onNightModeChanged: cancel()
    }

    MouseArea {
        anchors.fill: parent
        onPressed: root.hide()
    }

    Item {
        id: footnoteItem

        x: footnoteX
        y: footnoteY
        width: footnote.width
        height: Math.round(root.height + footnote.height)/2 - Theme.paddingMedium
        transformOrigin: Item.TopLeft

        Label {
            id: footnoteLabel

            anchors {
                top: parent.top
                bottom: footnote.top
                bottomMargin: Theme.paddingMedium
            }
            width: parent.width
            height: Math.round(root.height - footnote.height/2) - 2*Theme.paddingMedium
            color: Theme.highlightColor
            verticalAlignment: Text.AlignBottom
            maximumLineCount: 4
            visible: opacity > 0

            Behavior on opacity { FadeAnimation {} }
        }

        Rectangle {
            id: footnote

            radius: Theme.paddingMedium/2
            border.color: Settings.invertedPageBackgroundColor
            color: Settings.pageBackgroundColor
            width: content.width + 2*Theme.paddingMedium
            height: Math.min(root.height/2, content.height + 2*Theme.paddingMedium)
            anchors.bottom: parent.bottom

            SilicaFlickable {
                id: flickable

                anchors {
                    fill: parent
                    margins: Theme.paddingMedium
                }
                clip: true
                contentWidth: content.width
                contentHeight: content.height

                Image {
                    id: content

                    opacity: Books.contentOpacity(Settings.brightness)
                    cache: false
                }

                VerticalScrollDecorator {}
            }
        }
    }

    states: [
        State {
            name: "invisible"
            PropertyChanges {
                target: root
                opacity: 0
            }
            PropertyChanges {
                target: footnoteItem
                scale: 0
            }
        },
        State {
            name: "hide"
            extend: "invisible"
        },
        State {
            name: "cancel"
            extend: "invisible"
            PropertyChanges {
                target: footnoteItem
                transformOrigin: Item.Center
            }
        },
        State {
            name: "show"
            PropertyChanges {
                target: root
                opacity: 1
            }
            PropertyChanges {
                target: footnoteItem
                x: footnoteX
                y: footnoteY
                scale: 1
                transformOrigin: Item.TopLeft
            }
        }
    ]

    transitions: [
        Transition {
            from: "*"
            to: "show"
            NumberAnimation {
                properties: "opacity,x,y,scale"
                duration: 200
                easing.type: Easing.InOutQuad
            }
        },
        Transition {
            from: "show"
            to: "hide"
            NumberAnimation {
                properties: "opacity,x,y,scale"
                duration: 200
                easing.type: Easing.InOutQuad
            }
        },
        Transition {
            from: "show"
            to: "cancel"
            NumberAnimation {
                properties: "opacity,scale"
                duration: 200
                easing.type: Easing.InOutQuad
            }
        }
    ]
}
