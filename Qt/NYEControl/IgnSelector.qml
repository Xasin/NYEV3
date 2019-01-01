
import QtQuick 2.0
import QtQuick.Controls 2.4
import QtQuick.Layouts 1.3

import QtQuick.Controls.Material 2.3

Item {
	Layout.fillWidth: true
	height: width/8*2

	GridLayout {
		columns: 8
		anchors.fill: parent;

		Layout.margins: 0
		rowSpacing: 3
		columnSpacing: 3

		Repeater {
			model: 16

			Rectangle {
				Layout.preferredHeight: 2
				Layout.preferredWidth:  2

				Layout.fillHeight: true
				Layout.fillWidth:  true

				radius: 0.06*width

				border.color: Material.color(Material.Grey);
				border.width: 1

				Behavior on color {
					ColorAnimation {
						duration: 300
					}
				}

				color: if(houston.currentSelection == index)
							 Material.color(Material.Red)
						 else if((houston.standbyBits >> index) & 1 != 0)
							 Material.color(Material.Blue)
						 else if((houston.usedSlots >> index) & 1 != 0)
							 Material.color(Material.Grey, Mateiral.Shade700)
						 else
							 Material.color(Material.Green)

				MouseArea {
					anchors.fill: parent;

					onClicked: houston.currentSelection = index;
				}
			}
		}
	}
}
