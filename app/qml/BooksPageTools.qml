/*
  Copyright (C) 2015-2022 Jolla Ltd.
  Copyright (C) 2015-2022 Slava Monich <slava.monich@jolla.com>

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

import "harbour"

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

    // Left side

    MouseArea {
        id: dayNightModeSwitch
        height: parent.height
        width: leftMargin + Theme.iconSizeSmall + spacing
        anchors {
            left: parent.left
            verticalCenter: parent.verticalCenter
        }
        onClicked: Settings.nightMode = !Settings.nightMode

        HarbourHighlightIcon {
            id: dayModeImage
            source: "images/day-mode.svg"
            anchors {
                left: parent.left
                leftMargin: pageTools.leftMargin
                verticalCenter: parent.verticalCenter
            }
            sourceSize.height: Theme.iconSizeSmall
            highlightColor: Settings.primaryPageToolColor
            opacity: Settings.nightMode ? 1 : 0
            visible: opacity > 0
            Behavior on opacity { FadeAnimation {} }
        }

        HarbourHighlightIcon {
            source: "images/night-mode.svg"
            anchors.centerIn: dayModeImage
            sourceSize.height: Theme.iconSizeSmall
            highlightColor: Settings.primaryPageToolColor
            opacity: Settings.nightMode ? 0 : 1
            visible: opacity > 0
            Behavior on opacity { FadeAnimation {} }
        }
    }

    // Right side

    MouseArea {
        id: increaseFontSizeButton
        width: rightMargin + increaseFontSizeButtonImage.width
        height: parent.height
        anchors {
            right: parent.right
            verticalCenter: parent.verticalCenter
        }
        HarbourHighlightIcon {
            id: increaseFontSizeButtonImage
            source: "images/font-larger.svg"
            anchors {
                left: parent.left
                verticalCenter: parent.verticalCenter
            }
            sourceSize.height: Theme.iconSizeSmall
            highlightColor: Settings.primaryPageToolColor
        }
        onClicked: pageTools.increaseFontSize()
    }

    MouseArea {
        id: decreaseFontSizeButton
        width: decreaseFontSizeButtonImage.width + spacing
        height: parent.height
        anchors {
            right: increaseFontSizeButton.left
            verticalCenter: parent.verticalCenter
        }
        HarbourHighlightIcon {
            id: decreaseFontSizeButtonImage
            source: "images/font-smaller.svg"
            anchors {
                right: parent.right
                verticalCenter: parent.verticalCenter
            }
            sourceSize.height: increaseFontSizeButtonImage.height
            highlightColor: Settings.primaryPageToolColor
        }
        onClicked: pageTools.decreaseFontSize()
    }
}
