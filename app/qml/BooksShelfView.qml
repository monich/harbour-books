/*
  Copyright (C) 2015-2021 Jolla Ltd.
  Copyright (C) 2015-2021 Slava Monich <slava.monich@jolla.com>

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
    id: shelfView

    property int shelfIndex
    property bool singleStorage
    property bool removableStorage
    property bool editMode
    property bool deleteAllRequest
    property real cellWidth
    property real cellHeight
    property alias view: grid
    property alias shelf: shelfModel
    property alias device: shelfModel.device
    property alias dummyItemIndex: shelfModel.dummyItemIndex
    property alias bookCount: shelfModel.bookCount
    property alias bookCountVisible: storageHeader.needed

    signal openBook(var book)
    signal dropItem(var mouseX, var mouseY)
    signal startEditing()
    signal stopEditing()
    signal cancelDeleteAll()
    signal scrollRight()
    signal scrollLeft()

    readonly property bool _haveBooks: shelfModel.count > 0
    readonly property int _cellsPerRow: Math.floor(width/cellWidth)
    readonly property int _remorseTimeout: 5000
    readonly property bool _loading: shelfModel.loading || startAnimationTimer.running
    property var _remorse

    on_HaveBooksChanged: if (!_haveBooks) shelfView.stopEditing()

    Shelf {
        id: shelfModel
        property bool needDummyItem: dragInProgress && dragItem.shelfIndex !== shelfView.shelfIndex
        property bool _completed // Received Component.onCompleted
        onNeedDummyItemChanged: if (needDummyItem) hasDummyItem = true
        editMode: shelfView.editMode
        onRelativePathChanged: longStartTimer.restart()
        onPathChanged: {
            if (Settings.currentStorage === device && _completed) {
                Settings.currentFolder = path
            }
        }
        onDeviceChanged: {
            if (device === Settings.currentStorage) {
                relativePath = Settings.relativePath
            }
        }
        Component.onCompleted: _completed = true
    }

    BooksPathModel {
        id: pathModel
        path: shelfModel.relativePath
    }

    onEditModeChanged: {
        dragItem.visible = false
        shelf.cancelAllDeleteRequests()
        dragArea.draggedItemIndex = -1
        fadeAnimation.stop()
        if (!editMode) {
            dragArea.resetPressState()
            dragScrollAnimation.stop()
            if (_remorse) _remorse.cancel()
        }
    }

    function doDeleteAll() {
        if (deleteAllRequest) {
            fadeAnimation.stop()
            shelf.removeAll()
            shelf.cancelAllDeleteRequests()
        }
    }

    onDeleteAllRequestChanged: {
        if (deleteAllRequest) {
            if (!_remorse) _remorse = remorseComponent.createObject(shelfView)
            fadeAnimation.restart()
            //% "Deleting all books"
            _remorse.execute(qsTrId("harbour-books-shelf-view-about_to_delete_all"),
                doDeleteAll, _remorseTimeout)
        } else if (_remorse) {
            fadeAnimation.stop()
            _remorse.cancel()
        }
    }

    Component {
        id: remorseComponent
        RemorsePopup {
            onCanceled: {
                shelfView.cancelDeleteAll()
                shelf.cancelAllDeleteRequests()
                fadeAnimation.stop()
            }
        }
    }

    Connections {
        target: dragItem
        onAnimatingChanged: if (!dragItem.animating && !dragArea.pressed) dragArea.finishDrag()
    }

    BooksStorageHeader {
        id: storageHeader
        removable: removableStorage
        count: shelfModel.bookCount
        showCount: !_loading
        enabled: grid.contentY > grid.minContentY || pathModel.count > 0
        needed: !singleStorage || pathModel.count > 0
        onClicked: {
            if (!scrollToTopAnimation.running) {
                if (grid.contentY > grid.minContentY) {
                    scrollToTopAnimation.start()
                } else {
                    animationEnabled = true
                    shelfModel.relativePath = ""
                }
            }
        }
    }

    NumberAnimation {
        id: scrollToTopAnimation
        target: grid
        property: "contentY"
        duration: 500
        easing.type: Easing.InOutQuad
        to: grid.minContentY
    }

    SilicaGridView {
        id: grid
        anchors {
            top: storageHeader.bottom
            left: parent.left
            right: parent.right
            bottom: parent.bottom
            leftMargin: Math.floor((shelfView.width - _cellsPerRow * shelfView.cellWidth)/2)
        }
        model: shelfModel
        interactive: !dragInProgress && !scrollToTopAnimation.running
        clip: true
        cellWidth: shelfView.cellWidth
        cellHeight: shelfView.cellHeight
        flickableDirection: Flickable.VerticalFlick
        header: Column {
            Repeater {
                model: pathModel
                BooksShelfTitle {
                    width: grid.width
                    text: model.name
                    editable: editMode && currentFolder
                    currentFolder: model.index === (pathModel.count-1)
                    onClicked: {
                        if (!currentFolder) {
                            console.log("switching to", model.path)
                            shelfView.stopEditing()
                            shelfModel.relativePath = model.path
                        }
                    }
                    onRename: {
                        console.log(to)
                        shelfModel.name = to
                    }
                    onStartEdit: shelfView.startEditing()
                }
            }
        }
        delegate: BooksShelfItem {
            editMode: shelfView.editMode
            dropped: dragItem.dropShelfIndex >= 0 &&
                     dragItem.dropShelfIndex === shelfView.shelfIndex &&
                     dragItem.dropItemIndex >= 0 &&
                     dragItem.dropItemIndex === model.index
            dragged: dragItem.shelfIndex === shelfView.shelfIndex &&
                     dragArea.draggedItemIndex === model.index
            visible: !model.dummy && !dragged && !dropped
            pressed: model.accessible && (model.index === dragArea.pressedItemIndex) && (model.index !== dragArea.pressedDeleteItemIndex)
            deleting: model.deleteRequested
            deletingAll: shelfView.deleteAllRequest
            deleteAllOpacity: grid.itemOpacity
            width: shelfView.cellWidth
            height: shelfView.cellHeight
            book: model.book
            name: model.name
            copyingIn: model.copyingIn
            copyingOut: model.copyingOut
            copyProgress: model.copyProgress
            remorseTimeout: _remorseTimeout
            onScalingChanged: updateLastPressedItemScalingIndex()
            onPressedChanged: updateLastPressedItemScalingIndex()
            onDeleteMe: shelfView.shelf.remove(model.index)
            function updateLastPressedItemScalingIndex() {
                if (pressed && scaling) {
                    dragArea.lastPressedItemScalingIndex = model.index
                } else if (dragArea.lastPressedItemScalingIndex === model.index) {
                    dragArea.lastPressedItemScalingIndex = -1
                }
            }
        }

        footer: BooksShelfFooter {
            width: grid.width
            height: visible ? Math.max(implicitHeight, (grid.height > grid.headerItem.height) ? (grid.height - grid.headerItem.height) : 0) : 0
            allowBusyIndicator: !longStartTimer.running
            footerState: _haveBooks ? 0 : _loading ? 1 : 2
            visible: !_haveBooks
        }

        property real itemOpacity: 1
        property real minContentY: -headerItem.height

        moveDisplaced: Transition {
            SmoothedAnimation { properties: "x,y"; duration: 150 }
        }

        removeDisplaced: Transition {
            SmoothedAnimation { properties: "x,y"; duration: 150 }
        }

        NumberAnimation {
            id: fadeAnimation
            target: grid
            property: "itemOpacity"
            from: 1
            to: 0
            duration: _remorseTimeout
            easing.type: Easing.OutCubic
        }

        onActiveFocusChanged: console.log("BooksShelfView.grid", activeFocus)

        Behavior on y { SpringAnimation {} }
        VerticalScrollDecorator {}
    }

    BooksDragArea {
        id: dragArea
        dragParent: storageView
        gridView: grid
        onDeleteItemAt: {
            if (!shelfView.deleteAllRequest) {
                shelfView.shelf.setDeleteRequested(index, true);
            }
        }
        onDropItem: shelfView.dropItem(mouseX, mouseY)
        onActiveFocusChanged: console.log("BooksShelfView.grid.dragArea", activeFocus)
        Component.onCompleted: {
            console.log("BooksDragArea created")
            grid.focus = true
        }
    }

    Timer {
        id: longStartTimer
        interval: 500
        running: true
        onTriggered: {
            if (shelf.loading) {
                console.log(shelfModel.path, "startup is taking too long")
                startAnimationTimer.start()
            }
        }
    }

    Timer {
        id: startAnimationTimer
        interval: 2000
    }

    Loader {
        id: leftSwipeHintLoader
        anchors.fill: parent
        active: BooksHints.storageLeftSwipe < MaximumHintCount || running
        property bool running
        sourceComponent: Component {
            HarbourHorizontalSwipeHint {
                hintEnabled: !_loading &&
                    storageView.visible &&
                    shelfIndex == storageListWatcher.currentIndex &&
                    (shelfIndex+1) < storageModel.count &&
                    BooksHints.storageLeftSwipe < MaximumHintCount

                //% "Swipe left to see what's on the SD-card"
                text: qsTrId("harbour-books-storage-view-swipe_left_hint")
                swipeRight: false
                onHintShown: BooksHints.storageLeftSwipe++
                onHintRunningChanged: leftSwipeHintLoader.running = hintRunning
            }
        }
    }
}
