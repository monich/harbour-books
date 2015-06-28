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
    id: shelfView

    property variant settings
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

    signal openBook(var book)
    signal dropItem(var mouseX, var mouseY)
    signal startEditing()
    signal stopEditing()
    signal cancelDeleteAll()
    signal scrollRight()
    signal scrollLeft()

    property bool _haveBooks: shelf && shelf.count
    property int _cellsPerRow: Math.floor(width/cellWidth)
    readonly property int _remorseTimeout: 5000
    property bool _loading: !shelf || shelf.loading || startAnimationTimer.running
    property var _remorse

    on_HaveBooksChanged: if (!_haveBooks) shelfView.stopEditing()

    Shelf {
        id: shelfModel
        property bool needDummyItem: dragInProgress && dragItem.shelfIndex !== shelfView.shelfIndex
        onNeedDummyItemChanged: if (needDummyItem) hasDummyItem = true
        editMode: shelfView.editMode
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
            _remorse.execute(qsTrId("shelf-view-about-to-delete-all"),
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
        count: shelfModel.count
        showCount: !_loading
    }

    SilicaGridView {
        id: grid
        anchors {
            top: singleStorage ? parent.top : storageHeader.bottom
            left: parent.left
            right: parent.right
            bottom: parent.bottom
            leftMargin: Math.floor((shelfView.width - _cellsPerRow * shelfView.cellWidth)/_cellsPerRow/2)
        }
        model: shelfModel
        interactive: !dragInProgress
        clip: true
        opacity: (!_loading && _haveBooks) ? 1 : 0
        visible: opacity > 0
        cellWidth: shelfView.cellWidth
        cellHeight: shelfView.cellHeight
        flickableDirection: Flickable.VerticalFlick
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

        property real itemOpacity: 1

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
        }

        Behavior on y { SpringAnimation {} }
        Behavior on opacity { FadeAnimation {} }
        VerticalScrollDecorator {}
    }

    ViewPlaceholder {
        //% "No books"
        text: qsTrId("shelf-view-no-books")
        enabled: !_loading && !_haveBooks
        PulleyAnimationHint {
            id: pulleyAnimationHint
            flickable: storageView
            anchors.fill: parent
            enabled: parent.enabled && !editMode
        }
    }

    property Item _busyIndicator

    Component {
        id: busyIndicatorComponent
        BusyIndicator {
            visible: opacity > 0
            anchors.centerIn: parent
            size: BusyIndicatorSize.Large
            running: _loading && !longStartTimer.running
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
                if (!_busyIndicator) _busyIndicator = busyIndicatorComponent.createObject(shelfView)
            }
        }
    }

    Timer {
        id: startAnimationTimer
        interval: 2000
    }
}
