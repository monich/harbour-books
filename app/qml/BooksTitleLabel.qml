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

Item {
    id: title
    property real centerX: Math.floor(width/2)
    property alias text: label.text
    property alias color: label.color

    visible: opacity > 0
    Behavior on opacity { FadeAnimation {} }

    Component.onCompleted: updateLayout()

    onWidthChanged: updateLayout()
    onHeightChanged: updateLayout()
    onCenterXChanged: updateLayout()
    onTextChanged: updateLayout()

    function updateLayout() {
        if (centerX > 0 && centerX < width) {
            var left = Math.ceil(centerX)
            var right = Math.floor(width - centerX)
            var preferredWidth = Math.floor(Math.min(left, right))*2
            label.anchors.leftMargin = 0
            label.anchors.rightMargin = 0
            if (label.paintedWidth <= preferredWidth) {
                if (left < right) {
                    label.anchors.leftMargin = 0
                    label.anchors.rightMargin = right - left
                } else {
                    label.anchors.leftMargin = left - right
                    label.anchors.rightMargin = 0
                }
            }
        }
    }

    Label {
        id: label
        anchors.fill: parent
        smooth: true
        height: Theme.itemSizeExtraSmall
        font.pixelSize: Theme.fontSizeSmall
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
    }
}
