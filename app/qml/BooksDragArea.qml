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
    parent: dragInProgress ? dragParent : gridView
    anchors.fill: parent
    propagateComposedEvents: true

    signal deleteItemAt(var index)
    signal dropItem(var mouseX, var mouseY)

    property var dragParent
    property var gridView
    property int draggedItemIndex: -1
    property int pressedItemIndex: -1
    property int pressedDeleteItemIndex: -1
    property int lastPressedItemScalingIndex: -1
    property int lastReleasedItemIndex: -1
    property int lastReleasedDeleteItemIndex: -1
    property bool pressedItemScaling: (pressedItemIndex >= 0 && lastPressedItemScalingIndex === pressedItemIndex)
    property bool dragPositionChanged
    property bool dragCloseToTheLeftEdge
    property bool dragCloseToTheRightEdge
    property real dragLastX
    property real dragLastY

    onDraggedItemIndexChanged: {
        draggedItem = (draggedItemIndex >= 0) ? shelf.get(draggedItemIndex) : null
    }

    function itemX(index) { return shelfView.cellWidth * (index % shelfView._cellsPerRow) - gridView.contentX }
    function itemY(index) { return shelfView.cellHeight * Math.floor(index / shelfView._cellsPerRow) - gridView.contentY }

    onCanceled: {
        stopDrag()
    }
    onClicked: {
        var index
        if (shelfView.editMode) {
            if (!dragInProgress || !dragPositionChanged) {
                index = gridView.indexAt(mouseX + gridView.contentX, mouseY + currentShelfView.contentY)
                if (index >= 0 &&
                    index === lastReleasedDeleteItemIndex &&
                    dragItem.withinDeleteButton(mouseX - itemX(index), mouseY - itemY(index))) {
                    lastReleasedDeleteItemIndex = -1
                    root.deleteItemAt(index)
                    dragScrollAnimation.stop()
                } else if (shelfView.shelf.deleteRequested(index)) {
                    shelfView.shelf.setDeleteRequested(index, false);
                } else {
                    shelfView.stopEditing()
                }
            }
        } else {
            index = gridView.indexAt(mouseX + gridView.contentX, mouseY + currentShelfView.contentY)
            if (index >= 0) {
                if (index === lastReleasedItemIndex) {
                    var item = shelf.get(index);
                    if (item.accessible) {
                        if (item.book) {
                            shelfView.openBook(item.book)
                        } else if (item.shelf) {
                            var path = shelfView.shelf.relativePath
                            shelfView.shelf.relativePath  = path ? (path + "/" + item.shelf.name) : item.shelf.name
                        }
                    }
                }
            } else if (mouseY + gridView.contentY < 0) {
                // Let the header item handle it
                mouse.accepted = false
            }
        }
        resetPressState()
        dragScrollAnimation.stop()
    }
    onPressed: {
        var index = gridView.indexAt(mouseX + gridView.contentX, mouseY + currentShelfView.contentY)
        lastReleasedItemIndex = -1
        lastReleasedDeleteItemIndex = -1
        pressedItemIndex = index
        if (pressedDeleteItemIndex < 0 &&
            dragItem.withinDeleteButton(mouseX - itemX(index), mouseY - itemY(index))) {
            pressedDeleteItemIndex = index
        } else {
            pressedDeleteItemIndex = -1
        }
        if (mouseY + gridView.contentY < 0) {
            // Let the header item handle it
            mouse.accepted = false
        }
    }
    onReleased: {
        stopDrag(mouseX, mouseY)
        if (mouseY + gridView.contentY < 0) {
            // Let the header item handle it
            mouse.accepted = false
        }
    }
    onPressAndHold: {
        if (!shelfView.editMode) {
            var index = gridView.indexAt(mouseX + gridView.contentX, mouseY + currentShelfView.contentY)
            if (index === pressedItemIndex) {
                shelfView.startEditing()
            }
        }
    }
    onPositionChanged: {
        if (shelfView.editMode) {
            if (!pressedItemScaling && !dragInProgress && pressedDeleteItemIndex < 0) {
                startDrag(mouseX, mouseY)
            } else {
                doDrag(mouseX, mouseY)
            }
        }
    }

    function startDrag(x, y) {
        var index = gridView.indexAt(x + gridView.contentX, y + currentShelfView.contentY)
        if (index >= 0) {
            var item = shelf.get(index)
            if (item.accessible) {
                var delta = gridView.mapToItem(dragParent,0,0)
                console.log(shelf.path, "dragging", item.name)
                dragLastX = x + delta.x
                dragLastY = y + delta.y
                dragItem.moveAnimationEnabled = false
                dragItem.shelfIndex = shelfView.shelfIndex
                dragItem.dropShelfIndex = -1
                dragItem.dropItemIndex = -1
                dragItem.x = itemX(index) + delta.x
                dragItem.y = itemY(index) + delta.y
                dragCloseToTheLeftEdge = (x < horizontalScrollThreshold)
                dragCloseToTheRightEdge = (x > (gridView.width - horizontalScrollThreshold))
                dragPositionChanged = false
                draggedItemIndex = index
                pressedItemIndex = -1
                dragItem.visible = true
            } else {
                console.log(item.name, "is not draggable")
            }
        }
    }

    function doDrag(x, y) {
        if (dragInProgress && !dragItem.moving) {
            var dx = x - dragLastX
            var dy = y - dragLastY
            if (dx !== 0 || dy !== 0) {
                dragPositionChanged = true
                dragItem.x += dx
                dragItem.y += dy
                dragLastX = x
                dragLastY = y
                var newIndex = currentShelfView.indexAt(x + gridView.contentX, Math.max(y + currentShelfView.contentY, 0))
                if (newIndex < 0) newIndex = currentShelf.count - 1
                if (newIndex >= 0) {
                    if (currentShelf.hasDummyItem) {
                        currentShelf.dummyItemIndex = newIndex
                    } else {
                        currentShelf.move(draggedItemIndex, newIndex)
                        draggedItemIndex = newIndex
                    }
                }
                if (y < verticalScrollThreshold && currentShelfView.contentY > 0) {
                    if (!dragScrollAnimation.running) {
                        dragScrollAnimation.from = currentShelfView.contentY
                        dragScrollAnimation.to = 0
                        dragScrollAnimation.duration = currentShelfView.contentY*1000/gridView.height
                        dragScrollAnimation.start()
                    }
                } else if (y > (gridView.height - verticalScrollThreshold) && (currentShelfView.contentY < maxContentY)) {
                    if (!dragScrollAnimation.running) {
                        dragScrollAnimation.from = currentShelfView.contentY
                        dragScrollAnimation.to = maxContentY
                        dragScrollAnimation.duration = (maxContentY - currentShelfView.contentY)*1000/gridView.height
                        dragScrollAnimation.start()
                    }
                } else {
                    if (dragScrollAnimation.running) {
                        dragScrollAnimation.stop()
                    }
                    if (x < horizontalScrollThreshold) {
                        if (!dragCloseToTheLeftEdge) {
                            dragCloseToTheLeftEdge = true
                            shelfView.scrollLeft()
                        }
                    } else {
                        dragCloseToTheLeftEdge = false
                    }
                    if (x > (gridView.width - horizontalScrollThreshold)) {
                        if (!dragCloseToTheRightEdge) {
                            dragCloseToTheRightEdge = true
                            shelfView.scrollRight()
                        }
                    } else {
                        dragCloseToTheRightEdge = false
                    }
                }
            }
        }
    }

    function stopDrag(x, y) {
        lastReleasedItemIndex = pressedItemIndex
        lastReleasedDeleteItemIndex = pressedDeleteItemIndex
        if (draggedItemIndex >= 0) {
            if (x !== undefined && y !== undefined) doDrag(x, y)
            var delta = gridView.mapToItem(dragParent,0,0)
            dragItem.moveAnimationEnabled = true
            dragItem.x = itemX(draggedItemIndex) + delta.x
            dragItem.y = itemY(draggedItemIndex) + delta.y
            // Allow the listener to modify dragItem coordinates
            if (x !== undefined && y !== undefined) root.dropItem(x,y)
            dragItem.dragged = false
            // Normally we would finish things up once all the
            // animations have completed
            if (!dragItem.animating) finishDrag()
        }
        pressedItemIndex = -1
        dragScrollAnimation.stop()
    }

    function finishDrag() {
        console.log(shelf.path, "done with drag animation")
        draggedItemIndex = -1
        dragItem.dropShelfIndex = -1
        dragItem.dropItemIndex = -1
        dragItem.moveAnimationEnabled = false
        dragItem.visible = false
    }

    function resetPressState() {
        pressedItemIndex = -1
        pressedDeleteItemIndex = -1
        lastReleasedItemIndex = -1
        lastReleasedDeleteItemIndex = -1
        dragCloseToTheLeftEdge = false
        dragCloseToTheRightEdge = false
    }

    NumberAnimation {
        id: dragScrollAnimation
        target: currentShelfView
        property: "contentY"
        easing.type: Easing.InOutQuad
    }
}
