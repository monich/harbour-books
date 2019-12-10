/*
  Copyright (C) 2015-2019 Jolla Ltd.
  Copyright (C) 2015-2019 Slava Monich <slava.monich@jolla.com>

  You may use this file under the terms of the BSD license as follows:

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions
  are met:

    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the names of the copyright holders nor the names of its
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
    property real copyProgress
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
    readonly property bool copying: copyingIn || copyingOut

    readonly property bool _deleting: deleting && !deletingAll
    readonly property real _borderRadius: Theme.paddingSmall
    readonly property color _borderColor: Theme.primaryColor
    readonly property real _borderWidth: 2
    readonly property bool _haveProgress: copyProgress > 0 && copyProgress < 1
    readonly property real _coverCenterX: cover.x + (cover.width - _deleteButtonSize)/2
    readonly property real _coverCenterY: cover.y + cover.height - _deleteButtonSize/2
    readonly property real _deleteButtonSize: 3*margins
    readonly property real _deleteButtonMargin: Theme.paddingMedium

    property bool scaledDown: (editMode && !dragged && !pressed && !dropped)

    Loader {
        active: !cover.book
        anchors {
            margins: root.margins
            fill: parent
        }
        sourceComponent: Image {
            visible: !cover.book
            source: "images/bookshelf.svg"
            sourceSize.width: width
            sourceSize.height: height
        }
    }

    BookCover {
        id: cover
        anchors {
            margins: root.margins
            fill: parent
        }
        borderRadius: _borderRadius
        borderWidth: book ? _borderWidth : 0
        borderColor: _borderColor
        opacity: (copyingIn || copyingOut) ? 0.1 : 1
        Behavior on opacity { FadeAnimation { } }

        BooksTitleText {
            id: title

            color: cover.book ? Theme.primaryColor : "#ffe798"
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

    Loader {
        id: deleteButton
        active: editMode
        sourceComponent: BooksDeleteButton {
            x: _coverCenterX
            y: _coverCenterY
            size: _deleteButtonSize
            margin: _deleteButtonMargin
            visible: !pressed && !deleting && !deletingAll && !copyingIn && !copyingOut
            onClicked: root.deleteRequested()
        }
    }

    Loader {
        active: copying && !longCopyTimer.running && _haveProgress
        anchors.centerIn: busyIndicator
        sourceComponent:  ProgressCircle {
            value: copyProgress
            width: busyIndicator.width
            height: width
            inAlternateCycle: true
        }
    }

    BusyIndicator {
        id: busyIndicator
        size: BusyIndicatorSize.Medium
        x: cover.x + cover.centerX - width/2
        y: cover.y + cover.centerY - height/2
        visible: opacity > 0
        running: copying && !longCopyTimer.running && !_haveProgress
        Behavior on opacity { enabled: false }
    }

    function withinDeleteButton(x, y) {
        return x >= _coverCenterX - _deleteButtonMargin &&
           x < _coverCenterX + _deleteButtonSize + _deleteButtonMargin &&
           y >= _coverCenterY - _deleteButtonMargin &&
           y < _coverCenterY + _deleteButtonSize + _deleteButtonMargin
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

    onCopyingChanged: {
        if (copying) {
            longCopyTimer.restart()
        } else {
            longCopyTimer.stop()
        }
    }

    Timer {
        id: longCopyTimer
        interval: 500
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
