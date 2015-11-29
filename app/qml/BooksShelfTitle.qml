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

MouseArea {
    id: root
    implicitHeight: column.implicitHeight
    property alias text: label.text
    property bool currentFolder
    property bool _highlighted: pressed
    property color _highlightedColor: Theme.rgba(Theme.highlightBackgroundColor, Theme.highlightBackgroundOpacity)
    property bool _showPress: !currentFolder && (_highlighted || pressTimer.running)

    Column {
        id: column
        width: parent.width
        spacing: 0

        Item {
            height: Theme.paddingSmall
            width: parent.width
        }

        Item {
            id: labelItem
            width: Math.min(label.implicitWidth + icon.width + 3*Theme.paddingMedium, parent.width)
            height: label.implicitHeight

            Image {
                id: icon
                height: label.height*3/4
                sourceSize.height: height
                fillMode: Image.PreserveAspectFit
                source: "images/folder.svg"
                anchors {
                    left: parent.left
                    leftMargin: Theme.paddingMedium
                    verticalCenter: parent.verticalCenter
                }
            }

            Label {
                id: label
                truncationMode: TruncationMode.Fade
                width: Math.min(parent.width - 2*Theme.paddingMedium, implicitWidth)
                anchors {
                    left: icon.right
                    leftMargin: Theme.paddingMedium
                    verticalCenter: parent.verticalCenter
                }
                color: (currentFolder || pressed) ? Theme.highlightColor : Theme.primaryColor
                Behavior on color { ColorAnimation { duration: 100 } }
            }
        }

        Item {
            height: Theme.paddingSmall
            width: parent.width
        }

        /*
        Item {
            width: labelItem.width
            height: 3*Math.floor(PointsPerInch/100)

            Image {
                id: shelfLeft
                anchors {
                    left: parent.left
                    leftMargin: Theme.paddingMedium
                    bottom: parent.bottom
                }
                height: parent.height
                sourceSize.height: height
                fillMode: Image.PreserveAspectFit
                source: "images/shelf-left.svg"
            }

            Image {
                id: shelfRight
                anchors {
                    right: parent.right
                    rightMargin: Theme.paddingMedium
                    bottom: parent.bottom
                }
                height: parent.height
                sourceSize.height: height
                fillMode: Image.PreserveAspectFit
                source: "images/shelf-right.svg"
            }

            Image {
                anchors {
                    left: shelfLeft.right
                    right: shelfRight.left
                    bottom: parent.bottom
                }
                height: parent.height
                sourceSize.height: height
                sourceSize.width: width
                source: "images/shelf-middle.svg"
            }
        }
        */
    }

    onPressed: pressTimer.start()
    onCanceled: pressTimer.stop()

    Rectangle {
        anchors.fill: parent
        color: _showPress ? _highlightedColor : "transparent"
    }

    Timer {
        id: pressTimer
        interval: 50
    }
}
