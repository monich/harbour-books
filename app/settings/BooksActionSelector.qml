/*
  Copyright (C) 2017 Jolla Ltd.
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

ComboBox {
    id: actionComboBox
    property alias key: configuration.key
    property alias defaultValue: configuration.defaultValue
    property bool ready

    value: currentItem ? currentItem.text : ""
    menu: ContextMenu {
        id: actionMenu
        readonly property int defaultIndex: 0
        MenuItem {
            //: Combo box value for no action
            //% "No action"
            text: qsTrId("harbour-books-settings-page-action-none")
            readonly property int value: 0
        }
        MenuItem {
            //: Combo box value for previous page action
            //% "Previous page"
            text: qsTrId("harbour-books-settings-page-action-previous_page")
            readonly property int value: 1
        }
        MenuItem {
            //: Combo box value for next page action
            //% "Next page"
            text: qsTrId("harbour-books-settings-page-action-next_page")
            readonly property int value: 2
        }
    }
    onCurrentItemChanged: {
        if (ready && currentItem) {
            configuration.value = currentItem.value
        }
    }
    Component.onCompleted: {
        configuration.updateControls()
        ready = true
    }

    ConfigurationValue {
        id: configuration
        defaultValue: 0
        onValueChanged: updateControls()
        function updateControls() {
            var n = actionMenu.children.length
            var index = actionMenu.defaultIndex
            for (var i=0; i<n; i++) {
                if (actionMenu.children[i].value === value) {
                    index = i
                    break
                }
            }
            actionComboBox.currentIndex = index
        }
    }
}
