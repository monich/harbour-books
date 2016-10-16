/*
  Copyright (C) 2015-2016 Jolla Ltd.
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
    id: pageTools
    height: Theme.itemSizeExtraSmall
    property real leftMargin: Theme.horizontalPageMargin
    property real rightMargin: Theme.horizontalPageMargin
    property real spacing: Theme.paddingLarge
    property real leftSpaceUsed: dayNightModeSwitch.x + dayNightModeSwitch.width
    property real rightSpaceUsed: width - decreaseFontSizeButton.x

    signal increaseFontSize();
    signal decreaseFontSize();

    property real _spacingBy2: Math.ceil(spacing/2)

    // Left side

    MouseArea {
        id: dayNightModeSwitch
        height: parent.height
        width: leftMargin + dayModeImage.width + spacing
        anchors {
            left: parent.left
            verticalCenter: parent.verticalCenter
        }
        onClicked: Settings.invertColors = !Settings.invertColors

        Image {
            id: dayModeImage
            source: "images/day-mode.svg"
            anchors {
                left: parent.left
                leftMargin: pageTools.leftMargin
                verticalCenter: parent.verticalCenter
            }
            height: Math.ceil(parent.height/2)
            sourceSize.height: height
            opacity: Settings.invertColors ? 0.5 : 0
            visible: opacity > 0
            Behavior on opacity { FadeAnimation {} }
        }

        Image {
            source: "images/night-mode.svg"
            anchors.fill: dayModeImage
            sourceSize.height: height
            opacity: Settings.invertColors ? 0 : 0.25
            visible: opacity > 0
            Behavior on opacity { FadeAnimation {} }
        }
    }

    // Right side

    MouseArea {
        id: increaseFontSizeButton
        width: leftMargin + increaseFontSizeButtonImage.width + _spacingBy2
        height: parent.height
        anchors {
            right: parent.right
            verticalCenter: parent.verticalCenter
        }
        Image {
            id: increaseFontSizeButtonImage
            source: "images/font-larger.svg"
            anchors {
                left: parent.left
                leftMargin: _spacingBy2
                verticalCenter: parent.verticalCenter
            }
            height: Math.ceil(parent.height/2)
            sourceSize.height: height
            opacity: Settings.invertColors ? 1 : 0.5
            Behavior on opacity { FadeAnimation {} }
        }
        onClicked: pageTools.increaseFontSize()
    }

    MouseArea {
        id: decreaseFontSizeButton
        width: decreaseFontSizeButtonImage.width + spacing + _spacingBy2
        height: parent.height
        anchors {
            right: increaseFontSizeButton.left
            verticalCenter: parent.verticalCenter
        }
        Image {
            id: decreaseFontSizeButtonImage
            source: "images/font-smaller.svg"
            anchors {
                right: parent.right
                rightMargin: _spacingBy2
                verticalCenter: parent.verticalCenter
            }
            height: Math.ceil(parent.height/2)
            sourceSize.height: height
            opacity: Settings.invertColors ? 1 : 0.5
            Behavior on opacity { FadeAnimation {} }
        }
        onClicked: pageTools.decreaseFontSize()
    }
}
