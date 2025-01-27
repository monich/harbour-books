/*
  Copyright (C) 2015-2025 Slava Monich <slava@monich.com>
  Copyright (C) 2015-2022 Jolla Ltd.

  You may use this file under the terms of the BSD license as follows:

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
import QtFeedback 5.0
import Sailfish.Silica 1.0
import org.nemomobile.notifications 1.0
import harbour.books 1.0

import "harbour"

Item {
    id: root

    property variant book
    readonly property bool selecting: bookView.currentItem && bookView.currentItem.selecting
    readonly property bool selectionEmpty: !bookView.currentItem || bookView.currentItem.selectionEmpty

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

    property bool viewInteractive: bookView.interactive && !loading
    property alias pullDownMenu: menu
    property alias loadingBackgroundOpacity: loadingBackground.opacity
    property bool isCurrentView
    property bool pageActive

    property var hapticFeedback
    property var linkMenu
    property var imageView
    property var footnoteView
    property var settingsComponent

    readonly property bool viewActive: pageActive && Qt.application.active && !!book
    readonly property bool haveVolumeUpAction: Settings.volumeUpAction !== BooksSettings.ActionNone
    readonly property bool haveVolumeDownAction: Settings.volumeDownAction !== BooksSettings.ActionNone
    readonly property bool haveKeyAction: haveVolumeUpAction || haveVolumeDownAction

    readonly property string mediaKeyQml: BooksUtil.mediaKeyQml
    readonly property var permissions: Qt.createQmlObject(BooksUtil.permissionsQml, root, "Permissions")
    readonly property var volumeUp: Qt.createQmlObject(mediaKeyQml, root, "VolumeKey")
    readonly property var volumeDown: Qt.createQmlObject(mediaKeyQml, root, "VolumeKey")

    Binding { target: permissions; property: "enabled"; value: viewActive && haveKeyAction }
    Binding { target: volumeUp; property: "enabled"; value: viewActive && haveVolumeUpAction }
    Binding { target: volumeUp;  property: "key"; value: Qt.Key_VolumeUp }
    Binding { target: volumeDown; property: "enabled"; value: viewActive && haveVolumeDownAction }
    Binding { target: volumeDown;  property: "key"; value: Qt.Key_VolumeDown }

    Connections {
        target: volumeUp
        ignoreUnknownSignals: true
        onPressed: performAction(Settings.volumeUpAction)
        onRepeat: performAction(Settings.volumeUpAction)
    }

    Connections {
        target: volumeDown
        ignoreUnknownSignals: true
        onPressed: performAction(Settings.volumeDownAction)
        onRepeat: performAction(Settings.volumeDownAction)
    }

    function hideViews() {
        if (linkMenu) linkMenu.hide()
        if (imageView) imageView.hide()
        if (footnoteView) footnoteView.hide()
    }

    function bzzz() {
        if (!hapticFeedback) {
            hapticFeedback = hapticFeedbackComponent.createObject(root)
        }
        hapticFeedback.play()
    }

    onOrientationChanged: {
        if (footnoteView) {
            footnoteView.cancel()
        }
    }

    onBookChanged: if (!book) pager.setPage(0)

    onSelectingChanged: {
        if (selecting) {
            bzzz()
        } else if (!selectionEmpty) {
            notification.publish()
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

    Component {
        id: hapticFeedbackComponent

        ThemeEffect { effect: ThemeEffect.Press }
    }

    Behavior on opacity {
        enabled: menu.backToLibrary === 0
        FadeAnimation { }
    }

    Binding on opacity {
        // Fade out the page while menu is bouncing back
        when: menu.backToLibrary < 0
        value: menu.flickable.contentY / menu.backToLibrary
    }

    PullDownMenu {
        id: menu

        visible: isCurrentView && !loading && Settings.bookPullDownMenu

        property real backToLibrary

        MenuItem {
            //: Pulley menu item
            //% "Settings"
            text: qsTrId("harbour-books-menu-settings")
            visible: BooksSettingsMenu
            onClicked: {
                if (!settingsComponent) {
                    settingsComponent = Qt.createComponent("../settings/BooksSettings.qml")
                    if (settingsComponent.status !== Component.Ready) {
                        console.log(settingsComponent.errorString())
                    }
                }
                pageStack.push(settingsComponent, {
                    "title" : text,
                    "allowedOrientations": window.allowedOrientations,
                    "inApp": true
                })
            }
        }
        MenuItem {
            //% "Back to library"
            text: qsTrId("harbour-books-book-view-back")

            // Delay the actual action until bounce back is finished
            onClicked: menu.backToLibrary = flickable.contentY
        }

        onActiveChanged: {
            if (!active && backToLibrary < 0) {
                // The actual "Back to library" action
                backToLibrary = 0
                root.closeBook()
            }
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

    Rectangle {
        id: loadingBackground
        anchors.fill: parent
        color: HarbourUtil.invertedColor(Theme.primaryColor)
        visible: loading && opacity > 0
    }

    SilicaListView {
        id: bookView

        model: bookModel
        anchors.fill: parent
        flickDeceleration: maximumFlickVelocity
        orientation: ListView.Horizontal
        snapMode: ListView.SnapOneItem
        preferredHighlightBegin: 0
        preferredHighlightEnd: width
        highlightRangeMode: ListView.StrictlyEnforceRange
        spacing: Theme.paddingMedium
        opacity: loading ? 0 : 1
        visible: opacity > 0
        interactive: !selecting && !scrollAnimation.running &&
            (!linkMenu || !linkMenu.visible) &&
            (!imageView || !imageView.visible) &&
            (!footnoteView || !footnoteView.visible)

        readonly property real maxContentX: originX + Math.max(0, contentWidth - width)
        readonly property int currentPage: stackModel.currentPage
        property bool completed

        Component.onCompleted: {
            bookViewWatcher.positionViewAtIndex(currentPage)
            pager.setPage(currentPage)
            completed = true
        }

        onCurrentPageChanged: {
            if (completed && !moving && !scrollAnimation.running) {
                bookViewWatcher.positionViewAtIndex(currentPage)
            }
            pager.setPage(currentPage)
        }

        onMovingChanged: updateModel()

        delegate: BooksPageView {
            id: pageView

            width: bookView.width
            height: bookView.height
            bookModel: bookView.model
            page: model.pageIndex
            bookPos: model.bookPos
            leftMargin: bookModel.leftMargin
            rightMargin: bookModel.rightMargin
            topMargin: bookModel.topMargin
            bottomMargin: bookModel.bottomMargin
            leftSpaceReserved: pageTools.visible ? pageTools.leftSpaceUsed : 0
            rightSpaceReserved: pageTools.visible ? pageTools.rightSpaceUsed: 0
            titleVisible: _currentState.title
            pageNumberVisible: _currentState.page
            currentPage: ListView.isCurrentItem
            title: bookModel.title

            onJumpToPage: bookView.jumpTo(page)
            onPushPosition: stackModel.pushPosition(position) // bookView.jumpTo(page)
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
                if (!imageView) {
                    imageView = imageViewComponent.createObject(root)
                }
                imageView.show(url,rect)
            }
            onBrowserLinkPressed: {
                if (!linkMenu) {
                    linkMenu = linkMenuComponent.createObject(root)
                }
                linkMenu.show(url)
            }
            onFootnotePressed: {
                if (!footnoteView) {
                    footnoteView = footnoteViewComponent.createObject(root)
                }
                footnoteView.show(touchX,touchY,text,url)
            }
        }

        function jumpTo(page) {
            if (book && page >=0) {
                console.log("showing page", page)
                bookViewWatcher.positionViewAtIndex(page)
                stackModel.currentPage = page
            }
        }

        function prevPage() {
            if (!scrollAnimation.running && contentX > originX) {
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

        function updateModel() {
            if (completed && !moving && bookViewWatcher.currentIndex >= 0 && !bookViewWatcher.updatingViewPosition) {
                hideViews()
                stackModel.currentPage = bookViewWatcher.currentIndex
            }
        }

        ListWatcher {
            id: bookViewWatcher

            listView: bookView
            onCurrentIndexChanged: bookView.updateModel()
        }

        NumberAnimation {
            id: scrollAnimation

            target: bookView
            property: "contentX"
            duration: 500
            easing.type: Easing.InOutQuad
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
            visible: opacity > 0 && !!book && bookModel.pageCount && !loading

            onIncreaseFontSize: bookModel.increaseFontSize()
            onDecreaseFontSize: bookModel.decreaseFontSize()

            Behavior on opacity { FadeAnimation {} }
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
            opacity: (_currentState.pager && pageCount) ? 0.75 : 0
            visible: opacity > 0

            onPageChanged: bookView.jumpTo(page)
            onFeedback: bzzz()

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
        enabled: loading && bookModel.resetReason === BookModel.ReasonLoading
        visible: opacity > 0
        opacity: enabled ? 1.0 : 0.0

        onClicked: root.closeBook()

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

        Behavior on opacity { FadeAnimation {} }
    }

    function performAction(action) {
        switch (action) {
        case BooksSettings.ActionPreviousPage:
            bookView.prevPage()
            return
        case BooksSettings.ActionNextPage:
            bookView.nextPage()
            return
        }
    }
}
