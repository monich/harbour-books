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

SilicaFlickable {
    id: root

    property variant book

    signal closeBook()
    signal pageClicked(var page)

    property int _currentPage: bookListWatcher.currentIndex
    property bool _loading: minLoadingDelay.running || bookModel.loading
    property var _currentState: _visibilityStates[globalSettings.pageDetails % _visibilityStates.length]
    readonly property var _visibilityStates: [
        { pager: false, page: false, title: false, tools: false },
        { pager: false, page: true,  title: true,  tools: false },
        { pager: true,  page: true,  title: true,  tools: true  }
    ]

    // NOTE: These have to match ResetReason in BooksBookModel
    readonly property var _loadingTextLabel: [
        //% "Loading..."
        qsTrId("book-view-loading"),
        //% "Applying larger fonts..."
        qsTrId("book-view-applying-larger-fonts"),
        //% "Applying smaller fonts..."
        qsTrId("book-view-applying-smaller-fonts")
    ]

    PullDownMenu {
        MenuItem {
            //% "Back to library"
            text: qsTrId("book-view-back")
            onClicked: root.closeBook()
        }
    }

    Timer {
        id: minLoadingDelay
        interval: 1000
    }

    Timer {
        id: resetPager
        interval: 0
        onTriggered: {
            if (_currentPage >= 0) {
                console.log("resetting pager to", _currentPage)
                pager.currentPage = _currentPage
            }
        }
    }

    BookModel {
        id: bookModel
        book: root.book ? root.book : null
        size: bookListWatcher.size
        currentPage: _currentPage
        leftMargin: Theme.horizontalPageMargin
        rightMargin: Theme.horizontalPageMargin
        topMargin: Theme.itemSizeSmall
        bottomMargin: Theme.itemSizeSmall
        settings: globalSettings
        onJumpToPage: bookView.jumpTo(index)
        onCurrentPageChanged: {
            if (currentPage >= 0 && bookView._jumpingTo < 0) {
                pager.currentPage = currentPage
            }
        }
        onLoadingChanged: {
            if (loading && !pageCount) {
                minLoadingDelay.start()
                bookView._jumpingTo = -1
            }
        }
    }

    ListWatcher {
        id: bookListWatcher
        listView: bookView
    }

    SilicaListView {
        id: bookView
        model: bookModel
        anchors.fill: parent
        flickDeceleration: maximumFlickVelocity
        orientation: ListView.Horizontal
        snapMode: ListView.SnapOneItem
        spacing: Theme.paddingMedium
        opacity: _loading ? 0 : 1
        visible: opacity > 0
        delegate: BooksPageView {
            width: bookView.width
            height: bookView.height
            model: bookModel
            page: index
            leftMargin: bookModel.leftMargin
            rightMargin: bookModel.rightMargin
            topMargin: bookModel.topMargin
            bottomMargin: bookModel.bottomMargin
            leftSpaceReserved: pageTools.visible ? pageTools.leftSpaceUsed : 0
            rightSpaceReserved: pageTools.visible ? pageTools.rightSpaceUsed: 0
            titleVisible: _currentState.title
            pageNumberVisible: _currentState.page
            title: bookModel.title
            onPageClicked: {
                root.pageClicked(index)
                globalSettings.pageDetails = (globalSettings.pageDetails+ 1) % _visibilityStates.length
            }
        }

        property int _jumpingTo: -1
        function jumpTo(page) {
            if (page >=0 && page !== _currentPage) {
                _jumpingTo = page
                positionViewAtIndex(page, ListView.Center)
                _jumpingTo = -1
                if (_currentPage !== page) {
                    console.log("oops, still at", _currentPage)
                    resetPager.restart()
                }
            }
        }

        Behavior on opacity { FadeAnimation {} }

        BooksPageTools {
            id: pageTools
            anchors {
                top: parent.top
                left: parent.left
                right: parent.right
            }
            leftMargin: bookModel.leftMargin
            rightMargin: bookModel.rightMargin
            opacity: _currentState.tools ? 1 : 0
            visible: opacity > 0 && book && bookModel.pageCount && !_loading
            Behavior on opacity { FadeAnimation {} }
        }

        BooksPager {
            id: pager
            anchors {
                bottom: parent.bottom
                bottomMargin: (Theme.itemSizeExtraSmall + 2*(bookModel.bottomMargin - height))/4
            }
            pageCount: bookModel.pageCount
            width: parent.width
            opacity: (_currentState.pager && book && bookModel.pageCount) ? 0.75 : 0
            visible: opacity > 0
            onPageChanged: bookView.jumpTo(page)
            Behavior on opacity { FadeAnimation {} }
        }
    }

    BooksTitleLabel {
        id: titleLabel
        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
            leftMargin: bookModel.leftMargin
            rightMargin: bookModel.rightMargin
        }
        text: bookModel.title
        height: Theme.itemSizeExtraSmall
        color: Theme.highlightColor
        opacity: _loading ? 0.6 : 0
    }

    BusyIndicator {
        id: busyIndicator
        anchors.centerIn: parent
        size: BusyIndicatorSize.Large
        running: _loading
    }

    BooksFitLabel {
        anchors.fill: busyIndicator
        text: bookModel.progress > 0 ? bookModel.progress : ""
        opacity: (_loading && bookModel.progress > 0) ? 1 : 0
    }

    Label {
        anchors {
            top: busyIndicator.bottom
            topMargin: Theme.paddingLarge
            horizontalCenter: busyIndicator.horizontalCenter

        }
        horizontalAlignment: Text.AlignHCenter
        color: Theme.highlightColor
        opacity: _loading ? 1 : 0
        visible: opacity > 0
        Behavior on opacity { FadeAnimation {} }
        text: bookModel ? _loadingTextLabel[bookModel.resetReason] : ""
    }

    Button {
        //% "Cancel"
        text: qsTrId("book-view-cancel-loading")
        height: Theme.itemSizeLarge
        anchors {
            bottom: parent.bottom
            horizontalCenter: parent.horizontalCenter
        }
        onClicked: root.closeBook()
        enabled: _loading
        visible: opacity > 0
        opacity: enabled ? 1.0 : 0.0
        Behavior on opacity { FadeAnimation { } }
    }
}
