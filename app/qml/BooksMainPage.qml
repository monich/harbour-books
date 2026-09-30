import QtQuick 2.0
import Sailfish.Silica 1.0
import harbour.books 1.0

Page {
    id: page

    allowedOrientations: window.allowedOrientations

    property variant currentShelf: storageView.currentShelf
    readonly property bool pageActive: status === PageStatus.Active
    readonly property Item currentView: Settings.currentBook ? bookView : storageView
    property Item bookView

    readonly property real fadeInThreshold: height/4
    readonly property real swipeAwayThreshold: 3 * height/4

    Component.onCompleted: createBookViewIfNeeded()

    onCurrentViewChanged: {
        if (currentView) {
            flickable.pullDownMenu = currentView.pullDownMenu
            flickable.pullDownMenu.flickable = flickable
        }
    }

    function createBookViewIfNeeded() {
        if (Settings.currentBook && !bookView) {
            bookView = bookViewComponent.createObject(flickable.contentItem)
        }
    }

    Connections {
        target: Settings
        onCurrentBookChanged: createBookViewIfNeeded()
    }

    SilicaFlickable {
        id: flickable

        anchors.fill: parent
        contentWidth: page.width
        contentHeight: Settings.currentBook ? (2 * page.height) : page.height
        flickableDirection: Flickable.VerticalFlick
        flickDeceleration: maximumFlickVelocity
        interactive: currentView && currentView.viewInteractive && !swipeAwayAnimation.running
        pressDelay: 0

        BooksStorageView {
            id: storageView

            width: page.width
            height: page.height
            y: Settings.currentBook ? flickable.contentY : 0
            viewScale: 0.9 + 0.1 * opacity
            pageActive: page.pageActive
            isPortrait: page.orientation === Orientation.Portrait
            isCurrentView: currentView === storageView
            opacity: Settings.currentBook ? ((y > fadeInThreshold) ? 1 : (y > 0) ? y/fadeInThreshold : 0) : 1
            visible: opacity > 0

            onOpenBook: Settings.currentBook = book

            Behavior on opacity {
                enabled: !Settings.currentBook
                FadeAnimation { }
            }
        }

        onMovementEnded: {
            if (contentY > 0 && Settings.currentBook) {
                if (contentY > swipeAwayThreshold) {
                    swipeAwayAnimation.start()
                } else {
                    unswipeAnimation.start()
                }
            }
        }
    }

    Component {
        id: bookViewComponent

        BooksBookView {
            id: bookView

            width: page.width
            height: page.height
            z: storageView.z + 1
            visible: !!Settings.currentBook
            orientation: page.orientation
            isPortrait: page.orientation === Orientation.Portrait
            isCurrentView: currentView === bookView
            pageActive: page.pageActive
            book: Settings.currentBook ? Settings.currentBook : null
            loadingBackgroundOpacity: 0.8 /* opacityOverlay */ * storageView.opacity

            onCloseBook: Settings.currentBook = null
            onVisibleChanged: if (visible) opacity = 1
        }
    }

    SequentialAnimation {
        id: swipeAwayAnimation

        alwaysRunToEnd: true
        NumberAnimation {
            target: flickable
            property: "contentY"
            to: page.height
            duration: 150
            easing.type: Easing.Linear
        }
        ScriptAction {
            script: {
                Settings.currentBook = null
                flickable.contentY = 0
            }
        }
    }

    SequentialAnimation {
        id: unswipeAnimation

        alwaysRunToEnd: true
        NumberAnimation {
            target: flickable
            property: "contentY"
            to: 0
            duration: 150
            easing.type: Easing.InOutQuad
        }
    }
}
