/*
  Copyright (C) 2015-2019 Jolla Ltd.
  Copyright (C) 2015-2019 Slava Monich <slava.monich@jolla.com>

  You may use this file under the terms of the BSD license as follows:

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions
  are met:

    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the names of the copyright holders nor the names of its
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

Text {
    font.family: Theme.fontFamily
    font.pixelSize: Theme.fontSizeSmall
    horizontalAlignment: Text.AlignHCenter
    verticalAlignment: Text.AlignVCenter
    wrapMode: Text.Wrap
    onTextChanged: fitText()
    onWidthChanged: fitText()
    onHeightChanged: fitText()
    onPaintedHeightChanged: fitText()
    property bool _fittingText: false

    function fitText() {
        if (!_fittingText) {
            _fittingText  = true
            font.pixelSize = Theme.fontSizeSmall
            maximumLineCount = 100000
            elide = Text.ElideNone
            if (width > 0 && height > 0 && paintedHeight > height) {
                font.pixelSize = Theme.fontSizeTiny
                if (paintedHeight > height && lineHeight > 0) {
                    maximumLineCount = height/lineHeight
                    elide = Text.ElideRight
                }
            }
            _fittingText = false
        }
    }
}
