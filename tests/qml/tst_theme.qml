import QtQuick
import QtQuick.Controls
import QtTest
import "../theme-fixtures"

TestCase {
    id: test
    name: "RoTheme"
    width: 820
    height: 600
    when: windowShown
    visible: true

    RoButton { id: button; text: "Save"; variant: "primary"; x: 20; y: 20; width: 150 }
    RoCaptureCard { id: card; title: "Region"; description: "Select an area"; x: 20; y: 80; width: 300; height: 118 }
    SignalSpy { id: clicked; target: card; signalName: "clicked" }
    BusyOverlay { id: busy; text: "Loading screenshots"; z: 10 }
    InlineStatus { id: status; x: 20; y: 220; width: 300; status: "error"; text: "Unable to save" }
    RoMotion { id: motion }
    RoIcon { id: icon; x: 340; y: 20; iconSource: "qrc:/qt/qml/ro_screenshot/assets/icon-folder.svg"; width: 24; height: 24 }

    function init() {
        UiPreferencesManager.themeMode = "light";
        UiPreferencesManager.animationDurationFactor = 1;
        busy.isBusy = false;
        clicked.clear();
    }

    function test_softwareIcon() {
        tryCompare(icon, "status", Image.Ready);
        const pixels = grabImage(icon);
        let tinted = false;
        for (let x = 0; x < pixels.width; ++x)
            for (let y = 0; y < pixels.height; ++y)
                if (Math.abs(pixels.red(x, y) / 255 - RoPalette.accent.r) < 0.25
                        && Math.abs(pixels.green(x, y) / 255 - RoPalette.accent.g) < 0.25
                        && Math.abs(pixels.blue(x, y) / 255 - RoPalette.accent.b) < 0.25
                        && pixels.blue(x, y) - pixels.red(x, y) > 50)
                    tinted = true;
        verify(tinted, "The icon must be visible and tinted without a GPU");
    }

    function test_paletteSwitch() {
        const lightSurface = RoPalette.card.toString();
        UiPreferencesManager.themeMode = "dark";
        verify(RoPalette.card.toString() !== lightSurface);
        compare(button._fg, RoPalette.selectionText);
        compare(RoPalette.selectionText.toString(), RoPalette.activePalette.selectionText.toLowerCase());
        compare(status.statusColor, RoPalette.danger);
        UiPreferencesManager.themeMode = "light";
        compare(RoPalette.card.toString(), lightSurface);
        compare(button._fg.toString(), RoPalette.activePalette.selectionText.toLowerCase());
    }

    function test_keyboardActivation() {
        card.forceActiveFocus();
        verify(card.activeFocus);
        keyClick(Qt.Key_Space);
        compare(clicked.count, 1);
        keyClick(Qt.Key_Return);
        compare(clicked.count, 2);
        button.forceActiveFocus();
        verify(button.activeFocus);
    }

    function test_busyAndErrorStates() {
        busy.isBusy = true;
        tryCompare(busy, "opacity", 1);
        tryCompare(busy, "visible", true);
        busy.isBusy = false;
        tryCompare(busy, "opacity", 0);
        tryCompare(busy, "visible", false);
        tryCompare(status, "visible", true);
        compare(status.statusIconText, "✕");
        status.text = "";
        tryCompare(status, "visible", false);
        status.text = "Unable to save";
    }

    function test_reducedMotion() {
        UiPreferencesManager.animationDurationFactor = 0;
        compare(motion.fast, 0);
        compare(motion.standard, 0);
        busy.isBusy = true;
        compare(busy.opacity, 1);
        busy.isBusy = false;
        compare(busy.opacity, 0);
        UiPreferencesManager.animationDurationFactor = 2;
        compare(motion.fast, 240);
    }

    function test_disabledAndResponsive() {
        button.enabled = false;
        verify(!button.enabled);
        compare(button.opacity, RoTokens.opacityDisabled);
        button.enabled = true;
        compare(button.opacity, 1);
        for (const width of [240, 300, 480]) {
            card.width = width;
            compare(card.width, width);
            verify(card.implicitHeight > 0);
        }
    }
}
