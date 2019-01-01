
import QtQuick.Controls.Material 2.3

import QtQuick.Controls 2.4
import QtQuick 2.0

Frame {
	padding: 5

	Text {
		text: rootWindow.connString

		verticalAlignment: Text.AlignVCenter
		horizontalAlignment: Text.AlignHCenter
		anchors.fill: parent

		fontSizeMode: Text.Fit
		font.pixelSize: 400

		color: rootWindow.connColor
	}
}
