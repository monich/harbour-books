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
    color: Theme.rgba(Theme.highlightDimmerColor, 0.9)

    property alias source: image.source
    readonly property real maxImageWidth: width - 2*Theme.horizontalPageMargin
    readonly property real maxImageHeight: height - 2*Theme.paddingLarge

    Behavior on opacity { FadeAnimation {} }

    Image {
        id: image
        anchors.centerIn: parent
        smooth: true
        readonly property bool landscape: sourceSize.width * parent.height > sourceSize.height * parent.width
        width: landscape ? maxImageWidth : (maxImageHeight * sourceSize.width / sourceSize.height)
        height: landscape ? (maxImageWidth * sourceSize.height / sourceSize.width) : maxImageHeight
    }

    MouseArea {
        anchors.fill: parent
        onPressed: {
            root.hide()
        }
    }

    function show() {
        opacity = 1.0
    }

    function hide() {
        opacity = 0.0
    }
}
