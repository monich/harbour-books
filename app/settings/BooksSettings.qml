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
import org.nemomobile.configuration 1.0

Page {
    id: page
    property alias title: pageHeader.title
    readonly property string rootPath: "/apps/" + appName() + "/"

    // Deduce package name from the path
    function appName() {
        var parts = Qt.resolvedUrl("dummy").split('/')
        if (parts.length > 2) {
            var name = parts[parts.length-3]
            if (name.indexOf("swissclock") >= 0) {
                return name
            }
        }
        return "harbour-books"
    }

    SilicaFlickable {
        anchors.fill: parent
        contentHeight: content.height

        Column {
            id: content
            width: parent.width

            PageHeader {
                id: pageHeader
                //: Settings page header
                //% "Books"
                title: qsTrId("harbour-books-settings-page-header")
            }

            Slider {
                id: fontSizeSlider
                minimumValue: -5
                maximumValue: 15
                stepSize: 1
                //: Slider value label for the standard font size
                //% "Default"
                readonly property string normal: qsTrId("harbour-books-settings-page-font_size_label-default")
                //: Slider label
                //% "Font size"
                label: qsTrId("harbour-books-settings-page-font_size_label")
                valueText: (value === 0) ? normal : ((value > 0) ? ("+" + value) : value)
                width: page.width
                anchors.horizontalCenter: parent.horizontalCenter
                onSliderValueChanged: fontSize.value = value
                Component.onCompleted: value = fontSize.value

                ConfigurationValue {
                    id: fontSize
                    key: rootPath + "fontSize"
                    defaultValue: 0
                    onValueChanged: fontSizeSlider.value = value
                }
            }
        }
    }
}
