/*
  Copyright (C) 2015-2020 Jolla Ltd.
  Copyright (C) 2015-2020 Slava Monich <slava.monich@jolla.com>

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
import org.nemomobile.notifications 1.0
import harbour.books 1.0

//import Sailfish.Media 1.0         // Not allowed
//import org.nemomobile.policy 1.0  // Not allowed

import "harbour"

SilicaFlickable {
    id: root

    property variant book
    property bool selecting

    signal closeBook()
    signal pageClicked(var page)

    property int orientation: Orientation.Portrait
    property alias stackModel: bookModel.pageStack
    property bool loading: bookModel.loading
    property var _currentState: _visibilityStates[Settings.pageDetails % _visibilityStates.length]
    readonly property var _visibilityStates: [
        { pager: false, page: false, title: false, tools: false },
        { pager: false, page: true,  title: true,  tools: false },
        { pager: true,  page: true,  title: true,  tools: false },
        { pager: true,  page: true,  title: true,  tools: true  }
    ]

    interactive: !selecting && !scrollAnimation.running &&
        (!linkMenu || !linkMenu.visible) &&
        (!imageView || !imageView.visible) &&
        (!footnoteView || !footnoteView.visible)

    readonly property bool viewActive: Qt.application.active && book
    readonly property bool haveVolumeUpAction: Settings.volumeUpAction !== BooksSettings.ActionNone
    readonly property bool haveVolumeDownAction: Settings.volumeDownAction !== BooksSettings.ActionNone
    readonly property bool haveKeyAction: haveVolumeUpAction || haveVolumeDownAction

    property var linkMenu
    property var imageView
    property var footnoteView

    function hideViews() {
        if (linkMenu) linkMenu.hide()
        if (imageView) imageView.hide()
        if (footnoteView) footnoteView.hide()
    }

    onOrientationChanged: {
        if (footnoteView) {
            footnoteView.cancel()
        }
    }

    Component {
        id: linkMenuComponent
        BooksLinkMenu { }
    }

    Component {
        id: imageViewComponent
        BooksImageView { }
    }

    Component {
        id: footnoteViewComponent
        BooksFootnoteView { }
    }

    PullDownMenu {
        MenuItem {
            //% "Back to library"
            text: qsTrId("harbour-books-book-view-back")
            onClicked: root.closeBook()
        }
    }

    BookModel {
        id: bookModel
        book: root.book ? root.book : null
        size: bookViewWatcher.size
        leftMargin: Theme.horizontalPageMargin
        rightMargin: Theme.horizontalPageMargin
        topMargin: Theme.itemSizeSmall
        bottomMargin: Theme.itemSizeSmall
        onJumpToPage: bookView.jumpTo(page)
    }

    Notification {
        id: notification
        //: Pop-up notification
        //% "Copied to clipboard"
        previewBody: qsTrId("harbour-books-book-view-copied_to_clipboard")
        expireTimeout: 2000
        Component.onCompleted: {
            if ("icon" in notification) {
                notification.icon = "icon-s-clipboard"
            }
        }
    }

    SilicaListView {
        id: bookView
        model: bookModel
        anchors.fill: parent
        flickDeceleration: maximumFlickVelocity
        orientation: ListView.Horizontal
        snapMode: ListView.SnapOneItem
        spacing: Theme.paddingMedium
        opacity: loading ? 0 : 1
        visible: opacity > 0
        interactive: root.interactive
        readonly property real maxContentX: Math.max(0, contentWidth - width)
        readonly property int currentPage: stackModel.currentPage
        property bool completed

        Component.onCompleted: {
            bookViewWatcher.positionViewAtIndex(currentPage)
            completed = true
        }

        onCurrentPageChanged: {
            if (completed && !flicking && !scrollAnimation.running) {
                bookViewWatcher.positionViewAtIndex(currentPage)
            }
        }

        onFlickingChanged: {
            if (!flicking) {
                bookViewWatcher.updateModel()
            }
        }

        ListWatcher {
            id: bookViewWatcher
            listView: bookView
            onCurrentIndexChanged: {
                if (listView.completed && !listView.flicking && currentIndex >= 0) {
                    updateModel()
                }
            }
            function updateModel() {
                hideViews()
                stackModel.currentPage = currentIndex
                if (!pager.pressed) {
                    pager.currentPage = currentIndex
                }
            }
        }

        delegate: BooksPageView {
            id: pageView
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
            currentPage: bookViewWatcher.currentIndex == index
            title: bookModel.title
            onJumpToPage: bookView.jumpTo(page)
            onPushPosition: stackModel.pushPosition(position) // bookView.jumpTo(page)
            onCurrentPageChanged: {
                if (currentPage) {
                    root.selecting = pageView.selecting
                }
            }
            onSelectingChanged: {
                if (currentPage) {
                    globalFeedback.start("push_gesture")
                    if (!pageView.selecting) {
                        notification.publish()
                    }
                    root.selecting = pageView.selecting
                }
            }
            onPageClicked: {
                root.pageClicked(index)
                if (Settings.turnPageByTap && mouseY > bookModel.topMargin && mouseY < (pageView.height - bookModel.topMargin)) {
                    if (mouseX < pageView.width/4) {
                        bookView.prevPage()
                        return
                    }
                    if (mouseX > pageView.width*3/4) {
                        bookView.nextPage()
                        return
                    }
                }
                if (!Settings.pageDetailsFixed) {
                    Settings.pageDetails = (Settings.pageDetails + 1) % _visibilityStates.length
                }
            }
            onImagePressed: {
                if (currentPage) {
                    if (!imageView) {
                        imageView = imageViewComponent.createObject(root)
                    }
                    imageView.show(url,rect)
                }
            }
            onBrowserLinkPressed: {
                if (currentPage) {
                    if (!linkMenu) {
                        linkMenu = linkMenuComponent.createObject(root)
                    }
                    linkMenu.show(url)
                }
            }
            onFootnotePressed: {
                if (bookViewWatcher.currentIndex == index) {
                    if (!footnoteView) {
                        footnoteView = footnoteViewComponent.createObject(root)
                    }
                    footnoteView.show(touchX,touchY,text,url)
                }
            }
        }

        property int jumpingTo: -1
        function jumpTo(page) {
            if (page >=0 && page !== bookViewWatcher.currentIndex) {
                jumpingTo = page
                bookViewWatcher.positionViewAtIndex(page)
                pager.currentPage = page
                jumpingTo = -1
                if (bookViewWatcher.currentIndex !== page) {
                    console.log("oops, still at", currentPage)
                    resetPager.restart()
                }
            }
        }

        function prevPage() {
            if (!scrollAnimation.running && contentX > 0) {
                hideViews();
                scrollAnimation.from = contentX
                scrollAnimation.to = Math.max(0, contentX - width - spacing)
                scrollAnimation.start()
            }
        }

        function nextPage() {
            if (!scrollAnimation.running && contentX < maxContentX) {
                hideViews();
                scrollAnimation.from = contentX
                scrollAnimation.to = Math.min(maxContentX, contentX + width + spacing)
                scrollAnimation.start()
            }
        }

        NumberAnimation {
            id: scrollAnimation
            target: bookView
            property: "contentX"
            duration: 500
            easing.type: Easing.InOutQuad
        }

        Behavior on opacity { FadeAnimation {} }

        Timer {
            id: resetPager
            interval: 0
            onTriggered: {
                console.log("resetting pager to", bookViewWatcher.currentIndex)
                pager.currentPage = bookViewWatcher.currentIndex
            }
        }

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
            visible: opacity > 0 && book && bookModel.pageCount && !loading
            Behavior on opacity { FadeAnimation {} }
            onIncreaseFontSize: bookModel.increaseFontSize()
            onDecreaseFontSize: bookModel.decreaseFontSize()
        }

        BooksPager {
            id: pager
            anchors {
                left: parent.left
                right: parent.right
                bottom: parent.bottom
                bottomMargin: (Theme.itemSizeExtraSmall + 2*(bookModel.bottomMargin - height))/4
            }
            leftMargin: bookModel.leftMargin
            rightMargin: bookModel.rightMargin
            stack: stackModel
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
        opacity: loading ? 0.6 : 0
    }

    BusyIndicator {
        id: busyIndicator
        anchors.centerIn: parent
        size: BusyIndicatorSize.Large
        running: loading
    }

    HarbourFitLabel {
        anchors.fill: busyIndicator
        text: bookModel.progress > 0 ? bookModel.progress : ""
        opacity: (loading && bookModel.progress > 0) ? 1 : 0
    }

    Button {
        //% "Cancel"
        text: qsTrId("harbour-books-book-view-cancel_loading")
        height: Theme.itemSizeLarge
        anchors {
            bottom: parent.bottom
            horizontalCenter: parent.horizontalCenter
        }
        onClicked: root.closeBook()
        enabled: loading && bookModel.resetReason === BookModel.ReasonLoading
        visible: opacity > 0
        opacity: enabled ? 1.0 : 0.0
        Behavior on opacity { FadeAnimation { } }
    }

    Label {
        anchors {
            top: busyIndicator.bottom
            topMargin: Theme.paddingLarge
            horizontalCenter: busyIndicator.horizontalCenter

        }
        horizontalAlignment: Text.AlignHCenter
        color: Theme.highlightColor
        opacity: loading ? 1 : 0
        visible: opacity > 0
        Behavior on opacity { FadeAnimation {} }
        text: bookModel ? (bookModel.resetReason == BookModel.ReasonLoading ?
            //% "Loading..."
            qsTrId("harbour-books-book-view-loading") :
            bookModel.resetReason == BookModel.ReasonIncreasingFontSize ?
            //% "Applying larger fonts..."
            qsTrId("harbour-books-book-view-applying_larger_fonts") :
            bookModel.resetReason == BookModel.ReasonDecreasingFontSize ?
            //% "Applying smaller fonts..."
            qsTrId("harbour-books-book-view-applying_smaller_fonts") :
            //% "Formatting..."
            qsTrId("harbour-books-book-view-formatting")) : ""
    }

    function performAction(action)
    {
        switch (action) {
        case BooksSettings.ActionPreviousPage:
            bookView.prevPage()
            return
        case BooksSettings.ActionNextPage:
            bookView.nextPage()
            return
        }
    }

    MediaKey {
        enabled: viewActive && haveVolumeUpAction
        key: Qt.Key_VolumeUp
        onPressed: volumeUpAction()
        onRepeat: volumeUpAction()
        function volumeUpAction() {
            performAction(Settings.volumeUpAction)
        }
    }

    MediaKey {
        enabled: viewActive && haveVolumeDownAction
        key: Qt.Key_VolumeDown
        onPressed: volumeDownAction()
        onRepeat: volumeDownAction()
        function volumeDownAction() {
            performAction(Settings.volumeDownAction)
        }
    }

    Permissions {
        enabled: viewActive && haveKeyAction
        autoRelease: true
        applicationClass: "camera"

        Resource {
            id: volumeKeysResource
            type: Resource.ScaleButton
            optional: true
        }
    }
}
