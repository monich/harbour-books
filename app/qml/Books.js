.pragma library
.import Sailfish.Silica 1.0 as Silica

var version = "1.1.6"

var topNotchHeight = ('topCutout' in Silica.Screen) ? Silica.Screen.topCutout.height : 0
var topNotchLeft = ('topCutout' in Silica.Screen && topNotchHeight > 0) ? Silica.Screen.topCutout.x : Silica.Screen.width

function contentOpacity(brightness) {
    return 0.4 + brightness * 0.6
}
