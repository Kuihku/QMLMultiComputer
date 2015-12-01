import QtQuick 2.0

MouseArea {
    id: mainBalls;
    width: 1024;
    height: 768;

    property int ballSize: 100;

    Component {
        id: ball;
        Rectangle {
            id: ballRect;
            width: mainBalls.ballSize;
            height: mainBalls.ballSize;
            radius: (0.5 * mainBalls.ballSize);
            border.width: 1;
            visible: false;
            property int closeTimeout: 1000;
            property int liveTimeout: 5000;
            onVisibleChanged: {
                if (ballRect.visible) {
                    ballXAnim.to = Math.floor(Math.random() * ((mainBalls.width - mainBalls.ballSize) + 1));
                    ballYAnim.to = Math.floor(Math.random() * ((mainBalls.height - mainBalls.ballSize) + 1));
                    ballRect.liveTimeout = Math.floor(Math.random() * (ballRect.liveTimeout - ballRect.closeTimeout + 1)) + ballRect.closeTimeout;
                    movingAnimmation.start();
                    ballTimer.start();
                }
            }

            Component.onCompleted: {
                var rr = Math.floor(Math.random() * (0xff + 1));
                var gg = Math.floor(Math.random() * (0xff + 1));
                var bb = Math.floor(Math.random() * (0xff + 1));
//                console.debug("rr: " + rr + ", gg: " + gg + ", bb: " + bb);
                var ballColor = "#" + (rr < 10 ? "0" + rr : rr.toString(16)) + (gg < 10 ? "0" + gg : gg.toString(16)) + (bb < 10 ? "0" + bb : bb.toString(16));
//                console.debug("ballColor: " + ballColor);
                ballRect.color = ballColor;
            }

            ParallelAnimation {
                id: movingAnimmation;
//                running: !closeAnimation.running;
                PropertyAnimation {
                    id: ballXAnim;
                    target: ballRect;
                    property: "x";
                    duration: ballRect.liveTimeout;
                }
                PropertyAnimation {
                    id: ballYAnim;
                    target: ballRect;
                    property: "y";
                    duration: ballRect.liveTimeout;
                }
            }

            Timer {
                id: ballTimer;
                interval: ballRect.liveTimeout;
//                running: true;
//                repeat: true;
                property int destroyBall;
                onTriggered: {
                    if (ballTimer.destroyBall <= 0) {
                        ballRect.destroy();
                    }
                    else {
                        ballTimer.destroyBall -= 1;
//                        console.debug("onTriggered ballTimer.destroyBall: " + ballTimer.destroyBall);
//                        console.debug("ballCloseXAnim.to: " + ballCloseXAnim.to + ", ballCloseYAnim.to: " + ballCloseYAnim.to);
                        movingAnimmation.stop();
                        if (ballTimer.destroyBall <= 0) {
                            ballCloseXAnim.to = (ballRect.x + (0.5 * mainBalls.ballSize));
                            ballCloseYAnim.to = (ballRect.y + (0.5 * mainBalls.ballSize));
                            ballTimer.interval = ballRect.closeTimeout;
                            closeAnimation.start();
                        }
                        else {
                            ballXAnim.to = Math.floor(Math.random() * ((mainBalls.width - mainBalls.ballSize) + 1));
                            ballYAnim.to = Math.floor(Math.random() * ((mainBalls.height - mainBalls.ballSize) + 1));
                            movingAnimmation.start();
                        }
                        ballTimer.start();
                    }
                }
                Component.onCompleted: {
                    ballTimer.destroyBall = Math.floor(Math.random() * 5) + 1;
//                    console.debug("ballTimer.destroyBall: " + ballTimer.destroyBall);
                }
            }

            ParallelAnimation {
                id: closeAnimation;
                PropertyAnimation {
                    id: ballCloseXAnim;
                    target: ballRect;
                    property: "x";
                    duration: ballRect.closeTimeout;
                    easing.type: Easing.InQuad;
                    // to: (ballRect.x + (0.5 * mainBalls.ballSize));
                }
                PropertyAnimation {
                    id: ballCloseYAnim;
                    target: ballRect;
                    property: "y";
                    duration: ballRect.closeTimeout;
                    easing.type: Easing.InQuad;
                    // to: (ballRect.y + (0.5 * mainBalls.ballSize));
                }
                PropertyAnimation {
                    target: ballRect;
                    properties: "width,height,opacity";
                    duration: ballRect.closeTimeout;
                    easing.type: Easing.InQuad;
                    to: 0;
                }
            }
        }
    }

    onClicked: {
        var object = ball.createObject(mainBalls);
        object.x = mouse.x - (0.5 * mainBalls.ballSize);
        object.y = mouse.y - (0.5 * mainBalls.ballSize);
        object.visible = true;
    }
}
