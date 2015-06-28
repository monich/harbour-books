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

CoverBackground {
    id: root
    transparent: !_haveBook || grid.count < 6

    property variant book
    property variant shelf

    property bool _haveBook: book ? true : false

    CoverModel {
        id: coverModel
        source: shelf
    }

    GridView {
        id: grid
        visible: !_haveBook
        anchors.fill: parent
        interactive: false
        cellWidth: Math.floor(width/2.0)
        cellHeight: Math.ceil(height/((grid.count > 4) ? 3.0 : 2.0))
        model: coverModel
        delegate: BookCover {
            book: model.book
            width: grid.cellWidth
            height: grid.cellHeight
            borderWidth: 0
            stretch: true
        }
    }

    Rectangle {
        anchors.centerIn: parent
        visible: !_haveBook && !coverModel.count
        color: "#cc0000"
        height: Math.ceil(parent.height*3/10)
        width: height
        radius: height/2

        Image {
            anchors.centerIn: parent
            anchors.horizontalCenter:  parent.horizontalCenter
            source: "images/cover-image.svg"
            width: parent.width - Math.max(Math.ceil(parent.width/20),2)
            sourceSize.width: width
            sourceSize.height: height
            fillMode: Image.PreserveAspectFit
        }
    }

    BookCover {
        id: bookCover
        visible: _haveBook
        anchors.fill: parent
        anchors.centerIn: parent
        width: Theme.coverSizeLarge.width
        height: Theme.coverSizeLarge.height
        borderRadius: Theme.paddingSmall
        borderColor: Theme.primaryColor
        synchronous: true
        book: root.book ? root.book : null
        defaultCover: "images/default-cover.jpg"
        stretch: true
    }

    BooksTitleText {
        anchors {
            margins: Theme.paddingMedium
            fill: parent
        }
        text: book ? book.title : ""
        visible: bookCover.empty
    }
}
