/*
  Copyright (C) 2015-2017 Jolla Ltd.
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
    * Neither the name of Jolla Ltd nor the names of its contributors may
      be used to endorse or promote products derived from this software
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
import harbour.books 1.0

Item {
    id: view

    property alias model: widget.model
    property alias page: widget.page
    property alias leftMargin: widget.leftMargin
    property alias rightMargin: widget.rightMargin
    property alias topMargin: widget.topMargin
    property alias bottomMargin: widget.bottomMargin
    property alias selecting: widget.selecting
    property alias currentPage: widget.currentPage
    property alias title: titleLabel.text
    property real leftSpaceReserved
    property real rightSpaceReserved
    property bool titleVisible
    property bool pageNumberVisible

    signal pageClicked()
    signal imagePressed(var url, var rect)
    signal footnotePressed(var touchX, var touchY, var text, var url)
    signal browserLinkPressed(var url)
    signal jumpToPage(var page)
    signal pushPosition(var position)

    PageWidget {
        id: widget
        anchors.fill: parent
        model: bookModel
        pressed: mouseArea.pressed
        onBrowserLinkPressed: view.browserLinkPressed(url)
        onImagePressed: view.imagePressed(imageId, rect)
        onActiveTouch: pressImage.animate(touchX, touchY)
        onJumpToPage: view.jumpToPage(page)
        onPushPosition: view.pushPosition(position)
        onShowFootnote: view.footnotePressed(touchX,touchY,text,imageId)
    }

    BooksTitleLabel {
        id: titleLabel
        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
            leftMargin: Math.max(view.leftMargin, leftSpaceReserved)
            rightMargin: Math.max(view.rightMargin, rightSpaceReserved)
        }
        centerX: Math.floor(view.width/2) - anchors.leftMargin
        height: Theme.itemSizeExtraSmall
        color: Settings.primaryPageToolColor
        opacity: titleVisible ? 1 : 0
    }

    BusyIndicator {
        anchors.centerIn: parent
        size: BusyIndicatorSize.Large
        running: widget.loading
        opacity: running ? 1 : 0
        visible: opacity > 0
        Behavior on opacity {}
    }

    Image {
        id: pressImage
        source: Settings.invertColors ?  "images/press-invert.svg" : "images/press.svg"
        visible: opacity > 0
        opacity: 0
        readonly property int maxsize: Math.max(view.width, view.height)
        ParallelAnimation {
            id: pressAnimation
            NumberAnimation {
                target: pressImage
                easing.type: Easing.InOutQuad
                properties: "width,height"
                from: pressImage.sourceSize.width
                to: pressImage.maxsize
            }
            NumberAnimation {
                id: pressAnimationX
                target: pressImage
                easing.type: Easing.InOutQuad
                properties: "x"
            }
            NumberAnimation {
                id: pressAnimationY
                target: pressImage
                easing.type: Easing.InOutQuad
                properties: "y"
            }
            NumberAnimation {
                target: pressImage
                easing.type: Easing.InOutQuad
                properties: "opacity"
                from: 0.5
                to: 0
            }
        }
        function animate(x0, y0) {
            pressAnimation.stop();
            opacity = 0
            width = pressImage.sourceSize.width
            height = pressImage.sourceSize.height
            pressAnimationX.from = x = x0 - Math.round(width/2)
            pressAnimationY.from = y = y0 - Math.round(height/2)
            pressAnimationX.to = x0 - maxsize/2
            pressAnimationY.to = y0 - maxsize/2
            pressAnimation.start()
        }
    }

    Label {
        anchors {
            horizontalCenter: parent.horizontalCenter
            bottom: parent.bottom
        }
        text: widget.page + 1
        height: Theme.itemSizeExtraSmall
        font.pixelSize: Theme.fontSizeSmall
        verticalAlignment: Text.AlignVCenter
        color: Settings.primaryPageToolColor
        opacity: pageNumberVisible ? 1 : 0
        visible: opacity > 0
        Behavior on opacity { FadeAnimation {} }
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        onClicked: {
            if (widget.selectionEmpty) {
                view.pageClicked()
            } else {
                widget.clearSelection()
            }
        }
        onPressed: widget.handlePress(mouseX, mouseY)
        onPressAndHold: widget.handleLongPress(mouseX, mouseY)
        onPositionChanged: widget.handlePositionChanged(mouseX, mouseY)
    }
}
