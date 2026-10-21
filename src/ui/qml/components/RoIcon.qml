import QtQuick

Image {
    id: root
    property url iconSource: ""
    property color color: RoPalette.accent
    source: iconSource.toString().length > 0
            ? "image://ro-theme-icon/" + color.toString().replace("#", "") + "/" + iconSource.toString().split("/").pop()
            : ""
    sourceSize: Qt.size(20, 20)
    fillMode: Image.PreserveAspectFit
}
