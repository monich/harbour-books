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

Page {
    id: page

    allowedOrientations: window.allowedOrientations

    property variant currentShelf: storageView.currentShelf
    readonly property bool pageActive: status === PageStatus.Active
    readonly property Item currentView: Settings.currentBook ? bookView : storageView
    property Item bookView

    readonly property real fadeInThreshold: height/4
    readonly property real swipeAwayThreshold: 3 * height/4

    Component.onCompleted: createBookViewIfNeeded()

    onCurrentViewChanged: {
        if (currentView) {
            flickable.pullDownMenu = currentView.pullDownMenu
            flickable.pullDownMenu.flickable = flickable
        }
    }

    function createBookViewIfNeeded() {
        if (Settings.currentBook && !bookView) {
            bookView = bookViewComponent.createObject(flickable.contentItem)
        }
    }

    Connections {
        target: Settings
        onCurrentBookChanged: createBookViewIfNeeded()
    }

    SilicaFlickable {
        id: flickable

        anchors.fill: parent
        contentWidth: page.width
        contentHeight: Settings.currentBook ? (2 * page.height) : page.height
        flickableDirection: Flickable.VerticalFlick
        flickDeceleration: maximumFlickVelocity
        interactive: currentView && currentView.viewInteractive && !swipeAwayAnimation.running
        pressDelay: 0

        BooksStorageView {
            id: storageView

            width: page.width
            height: page.height
            y: Settings.currentBook ? flickable.contentY : 0
            viewScale: 0.9 + 0.1 * opacity
            pageActive: page.pageActive
            isCurrentView: currentView === storageView
            opacity: Settings.currentBook ? ((y > fadeInThreshold) ? 1 : (y > 0) ? y/fadeInThreshold : 0) : 1
            visible: opacity > 0

            onOpenBook: Settings.currentBook = book

            Behavior on opacity {
                enabled: !Settings.currentBook
                FadeAnimation { }
            }
        }

        onMovementEnded: {
            if (contentY > 0 && Settings.currentBook) {
                if (contentY > swipeAwayThreshold) {
                    swipeAwayAnimation.start()
                } else {
                    unswipeAnimation.start()
                }
            }
        }
    }

    Component {
        id: bookViewComponent

        BooksBookView {
            id: bookView

            width: page.width
            height: page.height
            z: storageView.z + 1
            visible: !!Settings.currentBook
            orientation: page.orientation
            isCurrentView: currentView === bookView
            pageActive: page.pageActive
            book: Settings.currentBook ? Settings.currentBook : null
            loadingBackgroundOpacity: 0.8 /* opacityOverlay */ * storageView.opacity

            onCloseBook: Settings.currentBook = null
            onVisibleChanged: if (visible) opacity = 1
        }
    }

    SequentialAnimation {
        id: swipeAwayAnimation

        alwaysRunToEnd: true
        NumberAnimation {
            target: flickable
            property: "contentY"
            to: page.height
            duration: 150
            easing.type: Easing.Linear
        }
        ScriptAction {
            script: {
                Settings.currentBook = null
                flickable.contentY = 0
            }
        }
    }

    SequentialAnimation {
        id: unswipeAnimation

        alwaysRunToEnd: true
        NumberAnimation {
            target: flickable
            property: "contentY"
            to: 0
            duration: 150
            easing.type: Easing.InOutQuad
        }
    }
}
