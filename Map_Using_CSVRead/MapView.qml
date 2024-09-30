import QtQuick 2.15
import QtLocation 6.6
import QtPositioning 6.6
import QtQuick.Controls 6.6

Item {
    width: 800
    height: 600

    property var routePath: []
    property var routeSpeeds: []
    property var routeTimes: []
    property ListModel markerModel: ListModel {}
    property ListModel segmentModel: ListModel {}  // Define the segment model here
    property var mapCenter: QtPositioning.coordinate(19.0760, 72.8777) // Initialize with a default coordinate

       property double userSpeed1: 0.0
       property double userSpeed2: 0.0
       property double userSpeed3: 0.0
       property double userSpeed4: 0.0

    Plugin {
        id: mapPlugin
        name: "osm"
        PluginParameter {
            name: "osm.mapping.custom.host"
            value: "https://tile.openstreetmap.org/"
        }
    }

    Map {
        id: map
        anchors.fill: parent
        plugin: mapPlugin
        center: mapCenter
        zoomLevel: 9
        activeMapType: map.supportedMapTypes[map.supportedMapTypes.length - 1]

        WheelHandler {
            id: wheel
            acceptedDevices: Qt.platform.pluginName === "cocoa" || Qt.platform.pluginName === "wayland" ? PointerDevice.Mouse | PointerDevice.TouchPad : PointerDevice.Mouse
            rotationScale: 1/120
            property: "zoomLevel"
        }
        DragHandler {
            id: drag
            target: null
            onTranslationChanged: (delta) => map.pan(-delta.x, -delta.y)
        }

        MapItemView {
            model: markerModel
            delegate: MapQuickItem {
                id: marker
                coordinate: model.coordinate
                anchorPoint.x: width / 2
                anchorPoint.y: height
                sourceItem: Image {
                    height: 50
                    width: 60
                    source: model.index === 0 ? "qrc:/image/marker_start.png" : "qrc:/image/marker_stop.png"
                }
            }
        }

        Component {
            id: segmentDelegate

            MapPolyline {
                line.width: 7
                line.color: model.lineColor
                path: [
                    QtPositioning.coordinate(model.startLat, model.startLon),
                    QtPositioning.coordinate(model.endLat, model.endLon)
                ]
            }
        }

        MapItemView {
            model: segmentModel  // Use the segment model directly here
            delegate: segmentDelegate
        }

        MouseArea {
            anchors.fill: parent
            onClicked: (mouse) => {
                           var clickCoord = map.toCoordinate(Qt.point(mouse.x, mouse.y))
                           var closestIndex = findClosestPointOnPolyline(clickCoord)
                           if (closestIndex >= 0) {
                               var speed = routeSpeeds[closestIndex]
                               var time = routeTimes[closestIndex];
                               popupText.text = speed + " km/h" + " , " + time

                               speedPopup.visible = true
                               speedPopup.x = mouse.x
                               speedPopup.y = mouse.y
                           } else {
                               speedPopup.visible = false
                           }
                       }
        }
    }

    Rectangle {
        id: speedPopup
        width: 170
        height: 40
        color: "white"
        border.color: "black"
        visible: false

        Text {
            id: popupText
            anchors.centerIn: parent
            text: ""
            font.pixelSize: 16
            font.bold: true


        }
    }

    function setRouteData(path, speeds, times, startLat, startLon, endLat, endLon) {
        routePath = path;
        routeSpeeds = speeds;
        routeTimes = times;

        markerModel.clear();
        markerModel.append({ coordinate: QtPositioning.coordinate(startLat, startLon) });
        markerModel.append({ coordinate: QtPositioning.coordinate(endLat, endLon) });

        mapCenter = QtPositioning.coordinate(startLat, startLon);

        console.log("Route Path length in setRouteData:", routePath.length);
        console.log("Route speed setRouteData:", routeSpeeds);

        updatePathSegments();
    }

    function findClosestPointOnPolyline(point) {
        var minDistance = Number.MAX_VALUE;
        var closestIndex = -1;
        var thresholdDistance = 300; // Adjust this value to determine how close a click must be to a polyline segment

        for (var i = 0; i <routePath.length-1; i++) {
            var segmentStart = QtPositioning.coordinate(routePath[i].latitude, routePath[i].longitude);
            var segmentEnd = QtPositioning.coordinate(routePath[i + 1].latitude, routePath[i + 1].longitude);
            var distance = distanceToSegment(point, segmentStart, segmentEnd);
            if (distance < thresholdDistance && distance < minDistance) {
                minDistance = distance;
                closestIndex = i;
            }
        }
        return closestIndex;
    }

    function distanceToSegment(point, segmentStart, segmentEnd) {
        var l2 = segmentStart.distanceTo(segmentEnd) * segmentStart.distanceTo(segmentEnd);
        if (l2 === 0) return point.distanceTo(segmentStart);
        var t = ((point.latitude - segmentStart.latitude) * (segmentEnd.latitude - segmentStart.latitude) +
                 (point.longitude - segmentStart.longitude) * (segmentEnd.longitude - segmentStart.longitude)) / l2;
        t = Math.max(0, Math.min(1, t));
        var projection = QtPositioning.coordinate(
                    segmentStart.latitude + t * (segmentEnd.latitude - segmentStart.latitude),
                    segmentStart.longitude + t * (segmentEnd.longitude - segmentStart.longitude)
                    );
        return point.distanceTo(projection);
    }


    function setSpeedValues(speed1, speed2, speed3, speed4) {
            userSpeed1 = speed1;
            userSpeed2 = speed2;
            userSpeed3 = speed3;
            userSpeed4 = speed4;
            updatePathSegments();
        }

        function getColorForSpeed(speed) {
            if (speed >= 100 && speed <= 180)
                return "red";
            if (speed >= 70 && speed <= 99)
                return "orange";
            if (speed >= 61 && speed <= 69)
                return "yellow";
             if (speed >= 0 && speed <= 60)
                return "green";
            else
                return "blue"; // Default color for out of range
        }

        function updatePathSegments() {
            segmentModel.clear();

            var pathLength = routePath.length;

            for (var i = 0; i < pathLength - 1; ++i) {
                var startCoord = routePath[i];
                var endCoord = routePath[i + 1];
                var speed = routeSpeeds[i];
                var color = getColorForSpeed(speed);

                // Check if the speed matches any of the user inputs
                var isUserSpeed = (speed === userSpeed1 || speed === userSpeed2 || speed === userSpeed3 || speed === userSpeed4);

                // Set color to blue by default, or highlight if it matches user input
                if (isUserSpeed) {
                    segmentModel.append({
                        startLat: startCoord.latitude,
                        startLon: startCoord.longitude,
                        endLat: endCoord.latitude,
                        endLon: endCoord.longitude,
                        lineColor: color
                    });
                } else {
                    segmentModel.append({
                        startLat: startCoord.latitude,
                        startLon: startCoord.longitude,
                        endLat: endCoord.latitude,
                        endLon: endCoord.longitude,
                        lineColor: "blue" // Default color for segments not matching user inputs
                    });
                }
            }
        }
        function clearRouteData() {
            // Clear route data arrays
            routePath = [];
            routeSpeeds = [];
            routeTimes = [];

            // Clear markers
            markerModel.clear();

            // Clear path segments
            segmentModel.clear();

            // Optionally reset the map center to a default location
            mapCenter = QtPositioning.coordinate(19.0760, 72.8777); // Set to your desired default location

            console.log("Route data cleared.");
        }


}
