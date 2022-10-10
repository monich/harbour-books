/*
  Copyright (C) 2015-2022 Jolla Ltd.
  Copyright (C) 2015-2022 Slava Monich <slava.monich@jolla.com>

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
import org.nemomobile.configuration 1.0

import "../qml/Books.js" as Books
import "../qml/harbour"

Page {
    id: thisPage

    property bool inApp
    property var colorSchemeModel
    property var colorEditorModel
    property alias title: pageHeader.title

    signal resetColors()

    // jolla-settings expects these properties:
    property var applicationName
    property var applicationIcon

    // Internal properties
    readonly property string _rootPath: "/apps/" + appName() + "/"
    readonly property bool _darkOnLight: ('colorScheme' in Theme) && Theme.colorScheme === 1
    readonly property int _screenWidth: isPortrait ? Screen.width : Screen.height
    readonly property int _screenHeight: isPortrait ? Screen.height : Screen.width
    readonly property bool _landscapeLayout: (_screenWidth > _screenHeight && Screen.sizeCategory > Screen.Small) || Screen.sizeCategory > Screen.Medium

    // Deduce package name from the path
    function appName() {
        var parts = Qt.resolvedUrl("dummy").split('/')
        if (parts.length > 2) {
            var name = parts[parts.length-3]
            if (name.indexOf("-books") >= 0) {
                return name
            }
        }
        return "harbour-books"
    }

    Loader {
        active: inApp // Follow orientation changes
        Connections {
            target: orientation
            onValueChanged: allowedOrientations =
                (orientation.value === 1) ? Orientation.Portrait :
                (orientation.value === 2) ? Orientation.Landscape :
                                            Orientation.All
        }
    }

    SilicaFlickable {
        anchors.fill: parent
        contentHeight: content.height

        // Night mode example (positioned right above the slider)
        Rectangle {
            opacity: nightModeBrightnessSlider.pressed ? 1.0 : 0
            visible: opacity > 0
            radius: Theme.paddingSmall
            width: nightModeExampleLabel.width + 2 * Theme.paddingLarge
            height: nightModeExampleLabel.height + 2 * Theme.paddingMedium
            readonly property int xMin:  nightModeBrightnessSlider.leftMargin
            readonly property int xMax: content.width - nightModeBrightnessSlider.rightMargin - width
            readonly property int yOffset: _landscapeLayout ? (Theme.itemSizeExtraLarge - Theme.itemSizeMedium) : 0
            x: content.x + displayGrid.x + Math.max(Math.min(nightModeBrightnessSlider.sliderLeft + Math.round(nightModeBrightnessSlider.sliderThumbX - width/2.0), xMax), xMin)
            y: content.y + displayGrid.y + nightModeBrightnessSlider.y + yOffset - height
            z: nightModeBrightnessSlider.z + 1
            color: "black"
            border {
                color: Theme.highlightColor
                width: Math.ceil(radius/5)
            }

            Label {
                id: nightModeExampleLabel

                anchors.centerIn: parent
                color: "white"
                opacity: Books.contentOpacity(nightModeBrightness.value)
                //: Night mode example label
                //% "Night mode"
                text: qsTrId("harbour-books-settings-page-night_mode_example")
                font {
                    bold: true
                    pixelSize: Theme.fontSizeLarge
                    family: "Times"
                }
            }

            Behavior on x { SmoothedAnimation { duration: 125 } }
            Behavior on opacity { FadeAnimation { duration: 500 } }
        }

        Column {
            id: content
            width: parent.width

            PageHeader {
                id: pageHeader
                rightMargin: Theme.horizontalPageMargin + (appIcon.visible ? (height - appIcon.padding) : 0)
                title: applicationName ? applicationName :
                    //: Settings page header (app name)
                    //% "Books"
                    qsTrId("harbour-books-settings-page-header")
                description: inApp ? "" :
                    //: Settings page header description (app version)
                    //% "Version %1"
                    qsTrId("harbour-books-settings-version").arg(Books.version)

                Image {
                    id: appIcon
                    readonly property int padding: Theme.paddingLarge
                    readonly property int size: pageHeader.height - 2 * padding
                    x: pageHeader.width - width - Theme.horizontalPageMargin
                    y: padding
                    width: size
                    height: size
                    sourceSize: Qt.size(size,size)
                    source: applicationIcon ? applicationIcon : ""
                    visible: appIcon.status === Image.Ready
                }
            }

            // =============== Display ===============

            SectionHeader {
                //: Section header for display settings
                //% "Display"
                text: qsTrId("harbour-books-settings-page-display-section_header")
            }

            Grid {
                id: displayGrid

                width: parent.width
                columns: _landscapeLayout ? 2 : 1

                readonly property real columnWidth: width/columns

                Slider {
                    id: fontSizeSlider

                    minimumValue: -5
                    maximumValue: 15
                    stepSize: 1
                    width: parent.columnWidth
                    leftMargin: _landscapeLayout ? Theme.horizontalPageMargin : nightModeBrightnessSlider.leftMargin
                    rightMargin: leftMargin
                    //: Slider value label for the standard font size
                    //% "Default"
                    readonly property string normal: qsTrId("harbour-books-settings-page-font_size_label-default")
                    //: Slider label
                    //% "Font size"
                    label: qsTrId("harbour-books-settings-page-font_size_label")
                    valueText: (value === 0) ? normal : ((value > 0) ? ("+" + value) : value)
                    onSliderValueChanged: fontSize.value = value
                    Component.onCompleted: value = fontSize.value

                    ConfigurationValue {
                        id: fontSize
                        key: _rootPath + "fontSize"
                        defaultValue: 0
                        onValueChanged: fontSizeSlider.value = value
                    }
                }

                Slider {
                    id: nightModeBrightnessSlider

                    width: parent.columnWidth
                    leftMargin: keepDisplayOnSwitch.leftMargin - Theme.paddingLarge + Theme.itemSizeExtraSmall
                    rightMargin: leftMargin
                    //: Slider label
                    //% "Brightness in night mode"
                    label: qsTrId("harbour-books-settings-page-night_mode_brightness_label")
                    stepSize: (maximumValue - minimumValue) / 100.0
                    onSliderValueChanged: nightModeBrightness.value = value
                    value: nightModeBrightness.value
                    valueText: _landscapeLayout ? " " : ""

                    readonly property real sliderLeft: x + leftMargin
                    readonly property real sliderRight: x + width - rightMargin
                    readonly property real sliderWidth: width - leftMargin - rightMargin
                    readonly property real sliderThumbX: sliderLeft + (maximumValue > minimumValue) ? (sliderWidth * sliderValue / (maximumValue - minimumValue)) : 0
                    readonly property real sliderBarVerticalCenter: Math.round(height - Theme.fontSizeSmall - Theme.paddingSmall - Theme.itemSizeExtraSmall*3/8)
                    readonly property color highlightColor: highlighted ? Theme.secondaryHighlightColor : Theme.secondaryColor

                    ConfigurationValue {
                        id: nightModeBrightness
                        key: _rootPath + "nightModeBrightness"
                        defaultValue: 1
                    }

                    HarbourHighlightIcon {
                        source: "images/brightness.svg"
                        y: nightModeBrightnessSlider.sliderBarVerticalCenter - Math.round(height/2)
                        anchors {
                            left: parent.left
                            leftMargin: nightModeBrightnessSlider.leftMargin - Theme.paddingSmall - width
                        }
                        sourceSize.height: Math.round(Theme.iconSizeSmall * 0.8)
                        highlightColor: nightModeBrightnessSlider.highlightColor
                        opacity: (_darkOnLight && !nightModeBrightnessSlider.highlighted) ? 0.6 : 1.0
                    }

                    HarbourHighlightIcon {
                        source: "images/brightness.svg"
                        y: nightModeBrightnessSlider.sliderBarVerticalCenter - Math.round(height/2)
                        anchors {
                            right: parent.right
                            rightMargin: nightModeBrightnessSlider.rightMargin - Theme.paddingSmall - width
                        }
                        sourceSize.height: Math.round(Theme.iconSizeSmall * 1.2)
                        highlightColor: nightModeBrightnessSlider.highlightColor
                        opacity: (_darkOnLight && !nightModeBrightnessSlider.highlighted) ? 0.6 : 1.0
                    }
                }
            }

            Grid {
                width: parent.width
                columns: _landscapeLayout ? 2 : 1
                flow: Grid.TopToBottom

                readonly property real columnWidth: width/columns

                Column {
                    width: parent.columnWidth

                    ComboBox {
                        id: orientationComboBox

                        //: Combo box label
                        //% "Orientation"
                        label: qsTrId("harbour-books-settings-page-orientation_label")
                        value: currentItem ? currentItem.text : ""
                        menu: ContextMenu {
                            id: orientationMenu

                            x: 0
                            width: orientationComboBox.width
                            readonly property int defaultIndex: 0
                            MenuItem {
                                readonly property int value: 0
                                //: Combo box value for dynamic orientation
                                //% "Dynamic"
                                text: qsTrId("harbour-books-settings-page-orientation-dynamic")
                                onClicked: orientation.value = value
                            }
                            MenuItem {
                                readonly property int value: 1
                                //: Combo box value for portrait orientation
                                //% "Portrait"
                                text: qsTrId("harbour-books-settings-page-orientation-portrait")
                                onClicked: orientation.value = value
                            }
                            MenuItem {
                                readonly property int value: 2
                                //: Combo box value for landscape orientation
                                //% "Landscape"
                                text: qsTrId("harbour-books-settings-page-orientation-landscape")
                                onClicked: orientation.value = value
                            }
                        }
                        Component.onCompleted: orientation.updateControls()
                        ConfigurationValue {
                            id: orientation
                            key: _rootPath + "orientation"
                            defaultValue: 0
                            onValueChanged: updateControls()
                            function updateControls() {
                                var n = orientationMenu.children.length
                                var index = orientationMenu.defaultIndex
                                for (var i=0; i<n; i++) {
                                    if (orientationMenu.children[i].value === value) {
                                        index = i
                                        break
                                    }
                                }
                                orientationComboBox.currentIndex = index
                            }
                        }
                    }

                    ComboBox {
                        id: layoutComboBox

                        //: Combo box label
                        //% "Page layout"
                        label: qsTrId("harbour-books-settings-page-page_layout")
                        value: currentItem ? currentItem.valueText : ""
                        readonly property int yBottom: y + Theme.itemSizeSmall
                        menu: ContextMenu {
                            x: 0
                            width: layoutComboBox.width
                            readonly property int defaultIndex: 0
                            BooksDetailMenuItem {
                                //: Combo box value for dynamic page layout
                                //% "Dynamic"
                                valueText: qsTrId("harbour-books-settings-page-layout-dynamic")
                                //: Combo box detail for dynamic page layout
                                //% "(toggle on tap)"
                                detailText: qsTrId("harbour-books-settings-page-layout-dynamic-detail")
                                onClicked: pageDetailsFixed.value = false
                            }
                            BooksDetailMenuItem {
                                //: Combo box value for clean page layout (just the content)
                                //% "Clean"
                                valueText: qsTrId("harbour-books-settings-page-layout-clean")
                                //: Combo box detail for clean page layout (just the content)
                                //% "(just the content)"
                                detailText: qsTrId("harbour-books-settings-page-layout-clean-detail")
                                onClicked: {
                                    pageDetailsFixed.value = true
                                    pageDetails.value = 0
                                }
                            }
                            BooksDetailMenuItem {
                                //: Combo box value for minimal page layout (title + page)
                                //% "Minimal"
                                valueText: qsTrId("harbour-books-settings-page-layout-minimal")
                                //: Combo box detail for minimal page layout (title + page)
                                //% "(title, page)"
                                detailText: qsTrId("harbour-books-settings-page-layout-minimal-detail")
                                onClicked: {
                                    pageDetailsFixed.value = true
                                    pageDetails.value = 1
                                }
                            }
                            BooksDetailMenuItem {
                                //: Combo box value for normal page layout (title + page + slider)
                                //% "Regular"
                                valueText: qsTrId("harbour-books-settings-page-layout-normal")
                                //: Combo box detail for normal page layout (title + page + slider)
                                //% "(title, page, slider)"
                                detailText: qsTrId("harbour-books-settings-page-layout-normal-detail")
                                onClicked: {
                                    pageDetailsFixed.value = true
                                    pageDetails.value = 2
                                }
                            }
                            BooksDetailMenuItem {
                                //: Combo box value for full page layout (title + page + slider)
                                //% "Full"
                                valueText: qsTrId("harbour-books-settings-page-layout-full")
                                //: Combo box detail for full page layout (title + page + slider)
                                //% "(everything)"
                                detailText: qsTrId("harbour-books-settings-page-layout-full-detail")
                                onClicked: {
                                    pageDetailsFixed.value = true
                                    pageDetails.value = 3
                                }
                            }
                        }
                        Component.onCompleted: updateSelectedItem()
                        function updateSelectedItem() {
                            currentIndex = pageDetailsFixed.value ? (pageDetails.value + 1) : 0
                        }
                        ConfigurationValue {
                            id: pageDetails
                            key: _rootPath + "pageDetails"
                            defaultValue: 0
                            onValueChanged: layoutComboBox.updateSelectedItem()
                        }
                        ConfigurationValue {
                            id: pageDetailsFixed
                            key: _rootPath + "pageDetailsFixed"
                            defaultValue: false
                            onValueChanged: layoutComboBox.updateSelectedItem()
                        }
                    }
                }

                TextSwitch {
                    id: keepDisplayOnSwitch

                    width: parent.columnWidth
                    automaticCheck: false
                    checked: keepDisplayOn.value
                    //: Text switch label
                    //% "Keep display on while reading"
                    text: qsTrId("harbour-books-settings-page-keep_display_on")
                    //: Text switch description
                    //% "Prevent the display from blanking while reading the book."
                    description: qsTrId("harbour-books-settings-page-keep_display_on_description")
                    onClicked: keepDisplayOn.value = !keepDisplayOn.value
                    ConfigurationValue {
                        id: keepDisplayOn
                        key: _rootPath + "keepDisplayOn"
                        defaultValue: false
                    }
                }
            }

            // =============== Navigation ===============

            SectionHeader {
                //: Section header for media keys
                //% "Navigation"
                text: qsTrId("harbour-books-settings-page-navigation-section_header")
            }

            Grid {
                width: parent.width
                columns: _landscapeLayout ? 2 : 1
                flow: Grid.TopToBottom

                readonly property real columnWidth: width/columns

                Column {
                    width: parent.columnWidth

                    BooksActionSelector {
                        //: Combo box label
                        //% "Volume up"
                        label: qsTrId("harbour-books-settings-page-volume_up-label")
                        key: _rootPath + "volumeUpAction"
                        defaultValue: 2 // BooksSettings.ActionNextPage
                    }

                    BooksActionSelector {
                        //: Combo box label
                        //% "Volume down"
                        label: qsTrId("harbour-books-settings-page-volume_down-label")
                        key: _rootPath + "volumeDownAction"
                        defaultValue: 1 // BooksSettings.ActionPreviousPage
                        readonly property int yBottom: y + Theme.itemSizeSmall
                    }
                }

                Item {
                    width: 1
                    height: 1
                    visible: _landscapeLayout // To occupy the grid slot
                }

                TextSwitch {
                    width: parent.columnWidth
                    automaticCheck: false
                    checked: turnPageByTap.value
                    //: Text switch label
                    //% "Turn pages by tapping the screen"
                    text: qsTrId("harbour-books-settings-page-turn_pages_by_tap")
                    //: Text switch description
                    //% "Tapping near the left edge of the screen returns to the previous page, tapping near the right edge gets you to the next page."
                    description: qsTrId("harbour-books-settings-page-turn_pages_by_tap-description")
                    onClicked: turnPageByTap.value = !turnPageByTap.value
                    ConfigurationValue {
                        id: turnPageByTap
                        key: _rootPath + "turnPageByTap"
                        defaultValue: false
                    }
                }

                TextSwitch {
                    width: parent.columnWidth
                    automaticCheck: false
                    checked: bookPullDownMenu.value
                    //: Text switch label
                    //% "Show pulley menu when the book is open"
                    text: qsTrId("harbour-books-settings-page-book_pulldown_menu")
                    //: Text switch description
                    //% "Without the pulley menu, the book has to be closed by swiping it up."
                    description: qsTrId("harbour-books-settings-page-book_pulldown_menu-description")
                    onClicked: bookPullDownMenu.value = !bookPullDownMenu.value
                    ConfigurationValue {
                        id: bookPullDownMenu
                        key: _rootPath + "bookPullDownMenu"
                        defaultValue: true
                    }
                }
            }

            // =============== Colors ===============

            SectionHeader {
                //: Section header for colors
                //% "Colors"
                text: qsTrId("harbour-books-settings-page-colors-section_header")
            }

            TextSwitch {
                id: useCustomColorSchemeSwitch

                width: parent.width
                automaticCheck: false
                checked: !useCustomColorScheme.value
                //: Text switch label
                //% "Use standard colors"
                text: qsTrId("harbour-books-settings-page-standard_colors")
                //: Text switch description
                //% "Note that colors hardcoded in the book override the color scheme."
                description: qsTrId("harbour-books-settings-page-standard_colors-description")
                onClicked: {
                    customColorsGridHeightBehavior.enabled = true
                    useCustomColorScheme.value = !useCustomColorScheme.value
                    customColorsGridHeightBehavior.enabled = false
                }
                ConfigurationValue {
                    id: useCustomColorScheme

                    key: _rootPath + "useCustomColorScheme"
                    defaultValue: false
                }
            }

            Grid {
                id: customColorsGrid

                x: useCustomColorSchemeSwitch.leftMargin - Theme.paddingLarge - Theme.paddingMedium +
                    Theme.itemSizeExtraSmall + (_darkOnLight ? Theme.paddingMedium : 0)
                width: parent.width - x
                columns: _landscapeLayout ? 2 : 1
                flow: Grid.TopToBottom
                height: useCustomColorScheme.value ? implicitHeight : 0
                visible: height > 0
                clip: true

                readonly property real columnWidth: width/columns

                Repeater {
                    model: colorSchemeModel
                    delegate: BackgroundItem {
                        id: delegate

                        width: customColorsGrid.columnWidth

                        Rectangle {
                            id: colorButton

                            width: height
                            height: Theme.itemSizeSmall - 2 * Theme.paddingMedium
                            color: model.color
                            border {
                                width: 1
                                color: Theme.primaryColor
                            }
                            anchors {
                                verticalCenter: parent.verticalCenter
                                left: parent.left
                                leftMargin: Theme.paddingMedium
                            }
                            layer.enabled: delegate.highlighted
                            layer.effect: HarbourPressEffect {
                                source: colorButton
                            }
                        }

                        Label{
                            anchors {
                                verticalCenter: parent.verticalCenter
                                left: colorButton.right
                                leftMargin: Theme.paddingMedium
                                right: parent.right
                            }
                            truncationMode: TruncationMode.Fade
                            text: model.label
                        }

                        onClicked: {
                            var dialog = pageStack.push(Qt.resolvedUrl("../qml/harbour/HarbourColorPickerDialog.qml"), {
                                allowedOrientations: thisPage.allowedOrientations,
                                acceptDestinationAction: PageStackAction.Replace,
                                colorModel: colorEditorModel,
                                color: model.color,
                                //: Pulley menu item
                                //% "Reset colors"
                                resetColorsMenuText: qsTrId("harbour-books-color_picker-menu-reset_colors"),
                                //: Dialog title label
                                //% "Select color"
                                acceptText: qsTrId("harbour-books-color_picker-action-select_color"),
                                //: Dialog title label
                                //% "Add color"
                                addColorAcceptText: qsTrId("harbour-books-color_picker-action-add_color"),
                                //: Hue slider label
                                //% "Color"
                                addColorHueSliderText: qsTrId("harbour-books-color_picker-slider-hue"),
                                //: Brightness slider label
                                //% "Brightness"
                                addColorBrightnessSliderText: qsTrId("harbour-books-color_picker-slider-brightness"),
                                //: Text field description
                                //% "Hex notation"
                                addColorHexNotationText: qsTrId("harbour-books-color_picker-text-hex_notation")
                            })
                            dialog.resetColors.connect(thisPage.resetColors)
                            dialog.accepted.connect(function() {
                                delegate.setColor(dialog.color)
                            })
                        }

                        function setColor(c) {
                            model.color = c
                        }
                    }
                }

                Behavior on height {
                    id: customColorsGridHeightBehavior

                    enabled: false
                    NumberAnimation { duration: 200 }
                }
            }

            // =============== Memory card ===============

            SectionHeader {
                //: Section header for memory card settings
                //% "Memory card"
                text: qsTrId("harbour-books-settings-page-removable-section_header")
            }

            TextField {
                id: removableRootField
                width: parent.width
                labelVisible: false

                Component.onCompleted: text = removableRoot.value
                onActiveFocusChanged: removableRoot.value = text
                EnterKey.onClicked: thisPage.focus = true
                EnterKey.iconSource: "image://theme/icon-m-enter-close"

                ConfigurationValue {
                    id: removableRoot
                    key: _rootPath + "removableRoot"
                    defaultValue: "Books"
                    onValueChanged: removableRootField.text = value
                }
            }

            Label {
                id: removableRootLabel
                //: Settings field label
                //% "Books folder"
                text: qsTrId("harbour-books-settings-page-removable_root-label")
                x: removableRootField.textLeftMargin
                width: removableRootField.width - removableRootField.textLeftMargin - removableRootField.textRightMargin
                height: text.length ? (implicitHeight + Theme.paddingMedium) : 0
                anchors {
                    topMargin: -Theme.paddingSmall
                    bottomMargin: Theme.paddingMedium
                }
                color: removableRootField.activeFocus ? Theme.highlightColor : Theme.primaryColor
                opacity: removableRootField.activeFocus ? 1.0 : 0.6
                elide: Text.ElideRight
                font.pixelSize: Theme.fontSizeSmall
            }

            Label {
                //: Settings field description
                //% "Leave the folder name empty to scan the entire memory card for books."
                text: qsTrId("harbour-books-settings-page-removable_root-description")
                height: text.length ? (implicitHeight + Theme.paddingMedium) : 0
                width: removableRootLabel.width
                x: removableRootLabel.x
                font.pixelSize: Theme.fontSizeExtraSmall
                color: Theme.secondaryColor
                wrapMode: Text.Wrap
            }
        }

        VerticalScrollDecorator { }
    }
}
