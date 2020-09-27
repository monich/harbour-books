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
import harbour.books 1.0

SilicaFlickable {
    id: storageView
    interactive: !dragInProgress

    property bool editMode: false

    signal openBook(var book)

    property real _cellWidth
    property real _cellHeight: Math.ceil(_cellWidth*8/5)
    property var draggedItem
    property var currentShelf
    property var currentShelfView
    property int currentShelfIndex: storageListWatcher.currentIndex
    readonly property bool dragInProgress: draggedItem ? true : false
    readonly property real maxContentY: currentShelfView ? Math.max(0, currentShelfView.contentHeight - currentShelfView.height) -
                            (currentShelfView.headerItem ? currentShelfView.headerItem.height : 0) : 0
    readonly property real verticalScrollThreshold: _cellHeight/2
    readonly property real horizontalScrollThreshold: _cellWidth/2

    readonly property real _minGridCellWidth: 10*Theme.paddingMedium
    property var _settingsComponent

    // Books in the library shouldn't be too small or too big.
    // At least 3 (or 5 in landscape) should fit in the horizontal direction.
    // The width shouldn't be smaller than 10*Theme.paddingMedium or 0.88 inch
    function calculateCellWidth2(viewWidth, minCount) {
        var result = 0
        if (viewWidth > 0) {
            // At least 3 books in portrait, 5 in landscape
            var n = minCount + 1
            var cellSize = viewWidth/minCount
            while (cellSize > _minGridCellWidth && (cellSize/PointsPerInch) > 0.88 && n < 11) {
                cellSize = viewWidth/(++n)
            }
            result = Math.floor(viewWidth/(n-1))
        }
        return result
    }

    function calculateCellWidth() {
        // At least 3 books in portrait, 5 in landscape
        var result2 = calculateCellWidth2(Math.min(window.width, window.height), 3)
        var result1 = calculateCellWidth2(Math.max(window.width, window.height), 5)
        var result = Math.min(result1, result2)
        return result
    }

    Component.onCompleted: _cellWidth = calculateCellWidth()

    onCurrentShelfChanged: {
        if (storageList.completed && currentShelf) {
            Settings.currentFolder = currentShelf.path
        }
    }

    PullDownMenu {
        MenuItem {
            //: Pulley menu item
            //% "Settings"
            text: qsTrId("harbour-books-storage-menu-settings")
            visible: !editMode && BooksSettingsMenu
            onClicked: {
                if (!_settingsComponent) {
                    _settingsComponent = Qt.createComponent("../settings/BooksSettings.qml")
                    if (_settingsComponent.status !== Component.Ready) {
                        console.log(_settingsComponent.errorString())
                    }
                }
                pageStack.push(_settingsComponent, {
                    "title" : text,
                    "allowedOrientations": window.allowedOrientations,
                    "followOrientationChanges": true
                })
            }
        }
        MenuItem {
            //: Pulley menu item
            //% "Scan downloads"
            text: qsTrId("harbour-books-storage-menu-scan_downloads")
            visible: !editMode
            onClicked: pageStack.push(importComponent)
        }
        MenuItem {
            //: Pulley menu item
            //% "Delete all books"
            text: qsTrId("harbour-books-storage-menu-delete_everything")
            visible: editMode
            enabled: currentShelf && (currentShelf.count > 0)
            onClicked: storageModel.setDeleteAllRequest(storageListWatcher.currentIndex, true)
        }
    }

    onEditModeChanged: {
        storageModel.cancelDeleteAllRequests()
        dragScrollAnimation.stop()
    }

    Component {
        id: importComponent
        BooksImport {
            destination: currentShelf ? currentShelf.path : ""
            onAccepted: {
                var count = selectedCount
                for (var i=0; i<count; i++) {
                    currentShelf.importBook(selectedBook(i))
                }
            }
        }
    }

    Connections {
        target: Qt.application
        onActiveChanged: if (!Qt.application.active) editMode = false
    }

    BookStorage {
        id: storageModel
        // Show the contents of SD-card and let use know that he can switch
        // between the internal memory and the removable storage by swiping
        // the list horizontally
        onNewStorage: storageList.scrollToPage(index)
    }

    ListWatcher {
        id: storageListWatcher
        listView: storageList
        onSizeChanged: _cellWidth = calculateCellWidth()
    }

    SilicaListView {
        id: storageList
        anchors.fill: parent
        model: storageModel
        flickDeceleration: maximumFlickVelocity
        orientation: ListView.Horizontal
        snapMode: ListView.SnapOneItem
        highlightRangeMode: ListView.StrictlyEnforceRange
        spacing: Theme.paddingMedium
        interactive: !dragInProgress && !dragScrollAnimation.running

        property bool completed
        readonly property real maxContentX: Math.max(0, contentWidth - width)

        onMaxContentXChanged: {
            // SD-card can be removed while scroll animation is still active
            if (dragScrollAnimation.running && dragScrollAnimation.to > maxContentX) {
                dragScrollAnimation.to = maxContentX
                dragScrollAnimation.restart()
            }
        }

        Component.onCompleted: {
            var index = model.deviceIndex(Settings.currentStorage)
            // positionViewAtIndex doesn't work here, update contentX directly
            if (index >= 0) {
                contentX = (width + spacing) * index
            } else {
                // Most likely, removable storage is gone
                console.log(Settings.currentFolder, "is gone")
                Settings.currentFolder = currentShelf ? currentShelf.path : ""
            }
            completed = true
        }

        delegate: BooksShelfView {
            width: storageList.width
            height: storageList.height
            cellWidth: storageView._cellWidth
            cellHeight: storageView._cellHeight
            singleStorage: storageModel.count < 2
            editMode: storageView.editMode
            deleteAllRequest: model.deleteAllRequest
            device: model.device
            removableStorage: model.removable
            shelfIndex: model.index
            onStartEditing: storageView.editMode = true
            onStopEditing: storageView.editMode = false
            onScrollLeft: storageList.scrollOnePageLeft()
            onScrollRight: storageList.scrollOnePageRight()
            onCancelDeleteAll: storageModel.cancelDeleteAllRequests()
            onDropItem: storageView.dropItem()

            property bool current: model.index === currentShelfIndex
            Component.onCompleted: updateCurrentShelf()
            onCurrentChanged: updateCurrentShelf()
            function updateCurrentShelf() {
                if (current) {
                    storageView.currentShelf = shelf
                    storageView.currentShelfView = view
                } else {
                    // no need for dummy item anymore
                    shelf.hasDummyItem = false
                }
            }
            onOpenBook: storageView.openBook(book)
        }

        function scrollOnePageLeft() {
            if (contentX > 0) {
                dragScrollAnimation.from = contentX
                dragScrollAnimation.to = Math.max(0, contentX - width - spacing)
                dragScrollAnimation.start()
            }
        }

        function scrollOnePageRight() {
            if (contentX < maxContentX) {
                dragScrollAnimation.from = contentX
                dragScrollAnimation.to = Math.min(maxContentX, contentX + width + spacing)
                dragScrollAnimation.start()
            }
        }

        function scrollToPage(index) {
            dragScrollAnimation.from = contentX
            dragScrollAnimation.to = (width + spacing) * index
            dragScrollAnimation.start()
        }
    }

    function dropItem() {
        if (draggedItem && dragItem.shelfIndex !== currentShelfIndex && currentShelf) {
            var targetIndex = currentShelf.dummyItemIndex
            if (targetIndex >= 0 && currentShelf.drop(draggedItem)) {
                console.log("drop accepted")
                // Update coordinates of the drag item to make it move toward the drop target
                var cellsPerRow = Math.floor(currentShelfView.width/_cellWidth)
                var delta = currentShelfView.mapToItem(storageView,0,0)
                var x = _cellWidth * (targetIndex % cellsPerRow) - currentShelfView.contentX
                var y = _cellHeight * Math.floor(targetIndex / cellsPerRow) - currentShelfView.contentY
                dragItem.x = x + delta.x
                dragItem.y = y + delta.y
                // This hides the target item until the grag item makes it to the destination:
                dragItem.dropShelfIndex = currentShelfIndex
                dragItem.dropItemIndex = targetIndex
            }
        }
    }

    BooksShelfItem {
        id: dragItem
        visible: false
        width: storageView._cellWidth
        height: storageView._cellHeight
        pressed: false
        editMode: false
        scaledDown: false
        scaleAnimationEnabled: false
        synchronous: true
        book: draggedItem ? draggedItem.book : null
        name: draggedItem ? draggedItem.name : ""
        property int shelfIndex: -1
        property int dropShelfIndex: -1
        property int dropItemIndex: -1
    }

    NumberAnimation {
        id: dragScrollAnimation
        target: storageList
        property: "contentX"
        duration: 500
        easing.type: Easing.InOutQuad
    }
}
