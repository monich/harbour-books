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

Dialog {
    id: root
    allowedOrientations: window.allowedOrientations
    canAccept: importModel.selectedCount > 0

    property alias selectedCount: importModel.selectedCount
    property alias destination: importModel.destination

    property bool _loading: importModel.busy || startTimer.running

    function selectedBook(index) { return importModel.selectedBook(index) }

    BooksImportModel {
        id: importModel
    }

    SilicaFlickable {
        anchors.fill: parent

        DialogHeader {
            id: dialogHeader
            spacing: 0
            acceptText: (_loading || !importModel.count) ? "" :
                //% "Import %0 book(s)"
                importModel.selectedCount ?  qsTrId("import-view-import-n-books",importModel.selectedCount).arg(importModel.selectedCount) :
                //% "Select books"
                qsTrId("import-view-select-books")
        }

        SilicaListView {
            anchors {
                top: dialogHeader.bottom
                bottom: parent.bottom
                left: parent.left
                right: parent.right
            }

            model: importModel
            clip: true

            opacity: (importModel.count > 0) ? 1 : 0
            visible: opacity > 0
            Behavior on opacity { FadeAnimation {} }

            delegate: BooksImportItem {
                width: root.width
                highlighted: down || model.selected
                contentHeight: Theme.itemSizeExtraLarge
                onClicked: importModel.setSelected(model.index, !model.selected)
                book: model.book
            }
            VerticalScrollDecorator {}
        }

        ViewPlaceholder {
            //% "No new books found"
            text: qsTrId("import-view-no-new-books-found")
            enabled: !_loading && !importModel.count
        }
    }

    BusyIndicator {
        visible: opacity > 0
        anchors.centerIn: parent
        size: BusyIndicatorSize.Large
        running: _loading && !importModel.count
    }

    // Give the dialog 1 second to initialize and finish the transition
    // then ask the model to actually examine the downloads
    Timer {
        interval: 1000
        running: true
        onTriggered: importModel.refresh()
    }

    // Then show the spinner for at least one more second
    Timer {
        id: startTimer
        interval: 2000
        running: true
    }
}
