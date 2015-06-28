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
import harbour.books 1.0

Item {
    id: root

    signal clicked()
    signal deleteRequested()
    signal deleteMe()

    opacity: deletingAll ? deleteAllOpacity : deleting ? scale : 1

    property alias name: title.text
    property alias book: cover.book
    property alias synchronous: cover.synchronous
    property alias remorseTimeout: deleteAnimation.duration
    property real margins: Theme.paddingMedium
    property real deleteAllOpacity: 1
    property bool editMode
    property bool dropped
    property bool dragged
    property bool pressed
    property bool deletingAll
    property bool deleting
    property bool copyingOut
    property bool copyingIn
    property bool scaleAnimationEnabled: true
    property bool moveAnimationEnabled: false

    readonly property bool scaling: scaleUpAnimation.running || scaleDownAnimation.running || deleteAnimation.running
    readonly property bool moving: moveAnimationX.running || moveAnimationY.running
    readonly property bool animating: scaling || moving

    property bool _deleting: deleting && !deletingAll
    property real _borderRadius: Theme.paddingSmall
    property color _borderColor: Theme.primaryColor

    property bool scaledDown: (editMode && !dragged && !pressed && !dropped)

    BookCover {
        id: cover
        anchors {
            margins: root.margins
            fill: parent
        }
        borderWidth: 2
        borderRadius: _borderRadius
        borderColor: _borderColor
        opacity: (copyingIn || copyingOut) ? 0.1 : 1
        Behavior on opacity { FadeAnimation { } }

        BooksTitleText {
            id: title
            anchors.centerIn: parent
            width: parent.width - 2*Theme.paddingMedium
            height: parent.height - 2*Theme.paddingMedium
            visible: parent.empty
        }

        layer.enabled: root.pressed
        layer.effect: ShaderEffect { // i.e. PressEffect from Lipstick
            property variant source
            property color color: Theme.rgba(Theme.highlightBackgroundColor, 0.4)
            fragmentShader: "
            uniform sampler2D source;
            uniform highp vec4 color;
            uniform lowp float qt_Opacity;
            varying highp vec2 qt_TexCoord0;
            void main(void) {
                highp vec4 pixelColor = texture2D(source, qt_TexCoord0);
                gl_FragColor = vec4(mix(pixelColor.rgb/max(pixelColor.a, 0.00390625), color.rgb/max(color.a, 0.00390625), color.a) * pixelColor.a, pixelColor.a) * qt_Opacity;
            }"
        }
    }

    MouseArea {
        anchors {
            fill: parent
            margins: -Theme.paddingSmall
        }
        onClicked: root.clicked()
    }

    BooksDeleteButton {
        id: deleteButton
        x: cover.x + (cover.width - width)/2
        y: cover.y + cover.height - height/2
        size: 3*margins
        enabled: editMode && !pressed && !deleting && !deletingAll && !copyingIn && !copyingOut
        onClicked: root.deleteRequested()
    }

    BusyIndicator {
        id: busyIndicator
        anchors.centerIn: parent
        size: BusyIndicatorSize.Large
        running: copyingIn || copyingOut
    }

    function withinDeleteButton(x, y) {
        return x >= deleteButton.x - deleteButton.margin &&
               x < deleteButton.x + deleteButton.width + deleteButton.margin &&
               y >= deleteButton.y - deleteButton.margin &&
               y < deleteButton.y + deleteButton.height + deleteButton.margin;
    }

    function updateScaledDown() {
        if (scaleAnimationEnabled) {
            if (scaledDown) {
                scaleUpAnimation.stop()
                scaleDownAnimation.restart()
            } else {
                scaleDownAnimation.stop()
                scaleUpAnimation.restart()
            }
        } else {
            scale = scaledDown ? scaleDownAnimation.to : scaleUpAnimation.to
        }
    }

    onScaledDownChanged: if (!_deleting) updateScaledDown()

    onScaleAnimationEnabledChanged: {
        if (!_deleting && !scaleAnimationEnabled) {
            scale = scaledDown ? scaleDownAnimation.to : scaleUpAnimation.to
        }
    }

    on_DeletingChanged: {
        if (_deleting) {
            scaleUpAnimation.stop()
            scaleDownAnimation.stop()
            deleteAnimation.start()
        } else {
            deleteAnimation.stop()
            updateScaledDown()
        }
    }

    NumberAnimation {
        id: scaleUpAnimation
        target: root
        property: "scale"
        to: 1
        duration: 250
        easing.type: Easing.InOutQuad
    }

    NumberAnimation {
        id: scaleDownAnimation
        target: root
        property: "scale"
        to: 0.9
        duration: 250
        easing.type: Easing.InOutQuad
    }

    NumberAnimation {
        id: deleteAnimation
        target: root
        property: "scale"
        to: 0
        duration: 2000
        easing.type: Easing.OutSine
        onRunningChanged: if (!running && _deleting) root.deleteMe()
    }

    NumberAnimation { id: moveAnimationX; duration: 150; easing.type: Easing.InOutQuad }
    NumberAnimation { id: moveAnimationY; duration: 150; easing.type: Easing.InOutQuad }

    Behavior on x { animation: moveAnimationX; enabled: moveAnimationEnabled }
    Behavior on y { animation: moveAnimationY; enabled: moveAnimationEnabled }
}
