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
    anchors.fill: parent
    color: Theme.rgba(Theme.highlightDimmerColor, 0.9)

    property alias imageBackground: background.color
    readonly property real maxImageWidth: width - 2*Theme.horizontalPageMargin
    readonly property real maxImageHeight: height - 2*Theme.paddingLarge
    readonly property real finalImageWidth: Math.ceil(image.landscape ? maxImageWidth : (maxImageHeight * image.sourceSize.width / image.sourceSize.height))
    readonly property real finalImageHeight: Math.ceil(image.landscape ? (maxImageWidth * image.sourceSize.height / image.sourceSize.width) : maxImageHeight)
    readonly property real finalImageX: Math.floor((width - finalImageWidth)/2)
    readonly property real finalImageY: Math.floor((height - finalImageHeight)/2)

    property bool shown

    Rectangle {
        id: background
        anchors.fill: image
    }

    Image {
        id: image
        smooth: true
        readonly property bool landscape: sourceSize.width * parent.height > sourceSize.height * parent.width
    }

    MouseArea {
        anchors.fill: parent
        onPressed: root.hide()
    }

    function show(url,rect) {
        image.source = url
        if (!shown) {
            image.x = rect.x
            image.y = rect.y
            image.width = rect.width
            image.height = rect.height
            shown = true
        }
    }

    function hide() {
        shown = false
    }

    states: [
        State {
            name: "shown"
            when: shown
            PropertyChanges {
                target: root
                opacity: 1
            }
            PropertyChanges {
                target: background
                opacity: 1
            }
            PropertyChanges {
                target: image
                x: finalImageX
                y: finalImageY
                width: finalImageWidth
                height: finalImageHeight
            }
        }
    ]

    transitions: [
        Transition {
            from: "*"
            to: "*"
            NumberAnimation {
                properties: "opacity,x,y,width,height"
                duration: 200
                easing.type: Easing.InOutQuad
            }
        }
    ]
}
