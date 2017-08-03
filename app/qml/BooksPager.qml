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
    id: root
    height: slider.height

    property var stack
    property int pageCount
    property real leftMargin: Theme.horizontalPageMargin
    property real rightMargin: Theme.horizontalPageMargin
    property alias currentPage: slider.value
    property alias pressed: slider.pressed

    signal pageChanged(var page)

    MouseArea {
        id: navigateBackArea
        property bool down: pressed && containsMouse
        width: navigateBack.width + root.leftMargin
        height: navigateBack.height
        anchors {
            left: parent.left
            verticalCenter: parent.verticalCenter
        }
        onClicked: stack.back()
    }


    IconButton {
        id: navigateBack
        icon.source: "image://theme/icon-m-left?" + Settings.primaryPageToolColor
        down: navigateBackArea.down || (pressed && containsMouse)
        anchors {
            left: parent.left
            leftMargin: root.leftMargin
            verticalCenter: parent.verticalCenter
        }
        onClicked: stack.back()
    }

    BooksPageSlider {
        id: slider
        anchors {
            left: navigateBack.right
            right: navigateForwardArea.left
            bottom: parent.bottom
        }
        stepSize: 1
        minimumValue: 0
        maximumValue: pageCount > 0 ? pageCount - 1 : 0
        valueText: ""
        label: ""
        leftMargin: Theme.horizontalPageMargin
        rightMargin: Theme.horizontalPageMargin
        primaryColor: Settings.primaryPageToolColor
        secondaryColor: Settings.primaryPageToolColor
        highlightColor: Settings.highlightPageToolColor
        secondaryHighlightColor: Settings.highlightPageToolColor
        onSliderValueChanged: root.pageChanged(value)
    }

    MouseArea {
        id: navigateForwardArea
        property bool down: pressed && containsMouse
        width: navigateForward.width + root.rightMargin
        height: navigateForward.height
        anchors {
            right: parent.right
            verticalCenter: parent.verticalCenter
        }
        onClicked: stack.forward()
    }

    IconButton {
        id: navigateForward
        icon.source: "image://theme/icon-m-right?" + Settings.primaryPageToolColor
        down: navigateForwardArea.down || (pressed && containsMouse)
        anchors {
            right: parent.right
            rightMargin: root.rightMargin
            verticalCenter: parent.verticalCenter
        }
        onClicked: stack.forward()
    }
}
