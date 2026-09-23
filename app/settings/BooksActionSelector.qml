import QtQuick 2.0
import Sailfish.Silica 1.0
import org.nemomobile.configuration 1.0

ComboBox {
    id: root

    property alias key: configuration.key
    property alias defaultValue: configuration.defaultValue

    menu: ContextMenu {
        x: 0
        width: root.width

        onActivated: configuration.value = index

        //: Combo box value for no action
        //% "No action"
        MenuItem { text: qsTrId("harbour-books-settings-page-action-none") }
        //: Combo box value for previous page action
        //% "Previous page"
        MenuItem { text: qsTrId("harbour-books-settings-page-action-previous_page") }
        //: Combo box value for next page action
        //% "Next page"
        MenuItem { text: qsTrId("harbour-books-settings-page-action-next_page") }
    }

    ConfigurationValue {
        id: configuration

        defaultValue: 0
        // For some reason, the currentIndex: configuration.value binding doesn't always work
        onValueChanged: root.currentIndex = value
    }
}
