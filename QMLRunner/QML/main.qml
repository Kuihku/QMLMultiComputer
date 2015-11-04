import QtQuick 2.5
// import QtQuick.Window 2.2

Rectangle {
    id: mainQML;
    width: 200;
    height: 200;
    visible: true

    MouseArea {
        anchors.fill: parent
        onClicked: {
            Qt.quit();
        }
    }

    Text {
        id: timoutText;
        text: timer.counter;
        anchors.centerIn: parent
    }
    Text {
        id: positionText;
        anchors {
            top: timoutText.bottom;
            topMargin: 5;
            horizontalCenter: parent.horizontalCenter;
        }
        text: "( " + parent.x + ", " + parent.y + ", " + parent.width + ", " + parent.height + " )";
    }

    Timer {
        id: timer;
        interval: 1000;
        running: true;
        repeat: true;
        property int counter: 0;
        triggeredOnStart: true;
        onTriggered: {
            mainQML.width += 10;
            mainQML.height += 10;
//            console.debug("QML Timer counter: " + timer.counter);
            if (timer.counter++ >= 10) {
                timer.counter = 0;
                Qt.quit();
            }
//            timer.interval = 100 + (Math.random() * 1000);
//            timer.start();
        }
    }
}

