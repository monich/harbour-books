/****************************************************************************************
**
** Copyright (C) 2013 Jolla Ltd.
** Contact: Martin Jones <martin.jones@jollamobile.com>
** All rights reserved.
**
** This file is part of Sailfish Silica UI component package.
**
** You may use this file under the terms of BSD license as follows:
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are met:
**     * Redistributions of source code must retain the above copyright
**       notice, this list of conditions and the following disclaimer.
**     * Redistributions in binary form must reproduce the above copyright
**       notice, this list of conditions and the following disclaimer in the
**       documentation and/or other materials provided with the distribution.
**     * Neither the name of the Jolla Ltd nor the
**       names of its contributors may be used to endorse or promote products
**       derived from this software without specific prior written permission.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
** ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
** DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR
** ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
** (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
** LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
** ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
** SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**
****************************************************************************************/

import QtQuick 2.0
import Sailfish.Silica 1.0

MouseArea {
    id: slider

    // Same as Silica Slider except that its colors are configurable
    property color primaryColor: Theme.primaryColor
    property color secondaryColor: Theme.secondaryColor
    property color highlightColor: Theme.highlightColor
    property color secondaryHighlightColor: Theme.secondaryHighlightColor

    property real maximumValue: 1.0
    property real minimumValue: 0.0
    property real stepSize
    property real value: 0.0
    readonly property real sliderValue: Math.max(minimumValue, Math.min(maximumValue, value))
    property bool handleVisible: true
    property string valueText
    property alias label: labelText.text
    property bool down: pressed
    property bool highlighted: down
    property real leftMargin: Math.round(Screen.width/8)
    property real rightMargin: Math.round(Screen.width/8)

    property bool _hasValueLabel: false
    property real _oldValue
    property bool _tracking: true
    property real _precFactor: 1.0

    property real _grooveWidth: Math.max(0, width - leftMargin - rightMargin)
    property bool _widthChanged
    property bool _cancel

    property bool _componentComplete

    onStepSizeChanged: {
        // Avoid rounding errors.  We assume that the range will
        // be sensibly related to stepSize
        var decimial = Math.floor(stepSize) - stepSize
        if (decimial < 0.001) {
            _precFactor = Math.pow(10, 7)
        } else if (decimial < 0.1) {
            _precFactor = Math.pow(10, 4)
        } else {
            _precFactor = 1.0
        }
    }

    height: valueText !== "" ? Theme.itemSizeExtraLarge : label !== "" ? Theme.itemSizeMedium : Theme.itemSizeSmall

    onWidthChanged: updateWidth()
    onLeftMarginChanged: updateWidth()
    onRightMarginChanged: updateWidth()

    // changing the width of the slider shouldn't animate the slider bar/handle
    function updateWidth() {
        _widthChanged = true
        _grooveWidth = Math.max(0, width - leftMargin - rightMargin)
        _widthChanged = false
    }

    function cancel() {
        _cancel = true
        value = _oldValue
        _updateHighlightToValue()
    }

    drag {
        target: draggable
        minimumX: leftMargin - highlight.width/2
        maximumX: slider.width - leftMargin - highlight.width/2
        axis: Drag.XAxis
    }

    function _updateHighlightToValue() {
        if (maximumValue > minimumValue) {
            highlight.x = (sliderValue - minimumValue) / (maximumValue - minimumValue) * _grooveWidth - highlight.width/2 + leftMargin
        } else {
            highlight.x = leftMargin - highlight.width/2
        }
    }

    function _updateValueToDraggable() {
        if (width > (leftMargin + rightMargin)) {
            highlight.x = draggable.x
            var pos = draggable.x + highlight.width/2 - leftMargin
            value = _calcValue((pos / _grooveWidth) * (maximumValue - minimumValue) + minimumValue)
        }
    }

    function _calcValue(newVal) {
        if (newVal <= minimumValue) {
            return minimumValue
        }

        if (stepSize > 0.0) {
            var offset = newVal - minimumValue
            var intervals = Math.round(offset / stepSize)
            newVal = Math.round((minimumValue + (intervals * stepSize)) * _precFactor) / _precFactor
        }

        if (newVal > maximumValue) {
            return maximumValue
        }

        return newVal
    }

    onPressed: {
        _cancel = false
        _oldValue = value
        draggable.x = Math.min(Math.max(drag.minimumX, mouseX - highlight.width/2), drag.maximumX)
    }

    onReleased: {
        if (!_cancel) {
            _tracking = false
            _updateValueToDraggable()
            if (stepSize != 0.0) {
                // on release make sure that we settle on a step boundary
                _updateHighlightToValue()
            }
            _oldValue = value
        }
    }

    onCanceled: value = _oldValue

    onValueTextChanged: {
        if (valueText && !_hasValueLabel) {
            _hasValueLabel = true
            var valueIndicatorComponent = Qt.createComponent("private/SliderValueLabel.qml")
            if (valueIndicatorComponent.status === Component.Ready) {
                valueIndicatorComponent.createObject(slider)
            } else {
                console.log(valueIndicatorComponent.errorString())
            }
        }
    }

    onSliderValueChanged: {
        if (!slider.drag.active) {
            _tracking = false
            _updateHighlightToValue()
        }
    }

    GlassItem {
        id: background
        // extra painting margins (Theme.paddingMedium on both sides) are needed,
        // because glass item doesn't visibly paint across the full width of the item
        x: slider.leftMargin-Theme.paddingMedium
        width: slider._grooveWidth + 2*Theme.paddingMedium
        height: Theme.itemSizeExtraSmall/2

        anchors.verticalCenter: parent.verticalCenter
        dimmed: true
        radius: 0.06
        falloffRadius: 0.09
        ratio: 0.0
        onWidthChanged: { _tracking = true; _updateHighlightToValue() }
        color: slider.highlighted ? highlightColor : secondaryColor
        states: State {
            name: "hasText"; when: slider.valueText !== "" || text !== ""
            AnchorChanges { target: background; anchors.verticalCenter: undefined; anchors.bottom: slider.bottom }
            PropertyChanges { target: background; anchors.bottomMargin: Theme.fontSizeSmall + Theme.paddingSmall + 1 }
        }
    }

    GlassItem {
        id: progressBar
        x: background.x // some margin at each end
        anchors.verticalCenter: background.verticalCenter
        // height added as GlassItem will not display correctly with width < height
        width: (sliderValue - minimumValue) / (maximumValue - minimumValue) * (background.width-height) + height
        height: Theme.itemSizeExtraSmall/2
        visible: sliderValue > minimumValue
        dimmed: false
        radius: 0.05
        falloffRadius: 0.14
        ratio: 0.0
        color: slider.highlighted ? highlightColor : primaryColor
        Behavior on width {
            enabled: !_widthChanged
            SmoothedAnimation { velocity: 1500 }
        }
    }

    Item {
        id: draggable
        width: highlight.width
        height: highlight.height
        onXChanged: {
            if (_cancel) {
                return
            }
            if (slider.drag.active) {
                _updateValueToDraggable()
            }
            if (!_tracking && Math.abs(highlight.x - draggable.x) < 5) {
                _tracking = true
            }
        }
    }
    GlassItem {
        id: highlight
        width: Theme.itemSizeMedium
        height: Theme.itemSizeMedium
        radius: 0.17
        falloffRadius: 0.17
        anchors.verticalCenter: background.verticalCenter
        visible: handleVisible
        color: slider.highlighted ? highlightColor : primaryColor
        Behavior on x {
            enabled: !_widthChanged
            SmoothedAnimation { velocity: 1500 }
        }
    }

    Label {
        id: labelText
        visible: text.length
        font.pixelSize: Theme.fontSizeSmall
        color: slider.highlighted ? secondaryHighlightColor : secondaryColor
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: background.verticalCenter
        anchors.topMargin: Theme.paddingMedium
        width: Math.min(paintedWidth, parent.width - 2*Theme.paddingMedium)
        truncationMode: TruncationMode.Fade
    }
    states: State {
        name: "invalidRange"
        when: _componentComplete && minimumValue >= maximumValue
        PropertyChanges {
            target: progressBar
            width: progressBar.height
        }
        PropertyChanges {
            target: slider
            enabled: false
            opacity: 0.6
        }
        StateChangeScript {
            script: console.log("Warning: Slider.maximumValue needs to be higher than Slider.minimumValue")
        }
    }

    Component.onCompleted: {
        _componentComplete = true
        _updateHighlightToValue()
    }
}
