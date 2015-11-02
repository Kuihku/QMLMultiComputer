import QtQuick 2.5
// import QtQuick.Window 2.2

Rectangle {
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
        interval: 50;
        running: true;
        repeat: true;
        property int counter: 0;
        triggeredOnStart: true;
        onTriggered: {
//            console.debug("QML Timer counter: " + timer.counter);
            if (timer.counter++ > 100) {
                Qt.quit();
            }
//            timer.interval = 100 + (Math.random() * 1000);
//            timer.start();
        }
    }
}
