import QtQuick 2.11
import QtQuick.Window 2.11

import QtQuick.Controls 2.4
import QtQuick.Layouts 1.3

import QtQuick.Controls.Material 2.3

ApplicationWindow {
	id: rootWindow

	visible: true
	width: 360
	height: 480
	title: qsTr("Hello World")

	Material.theme: Material.Dark

	property color bColor: Material.color(Material.Grey, Material.Shade900);
	Behavior on bColor {
		ColorAnimation { duration: 500 }
	}

	Connections {
		target: houston
		onConnectionChanged: {
			if(houston.connCode === 5)
				bColor = Qt.tint(Material.color(Material.Grey, Material.Shade900), "#10FF0000");
			else
				bColor = Material.color(Material.Grey, Material.Shade900)
		}
	}

	Material.background: bColor

	property color  connColor: Material.color([Material.Red, Material.Red, Material.Purple,
	Material.Indigo, Material.Green, Material.Cyan][houston.connCode])
	property string connString: ["Disconnected", "Apollo disconnected",
	"Unpowered", "Disarmed", "ARMED", "FIRING"][houston.connCode];

	Behavior on connColor {
		ColorAnimation {
			duration: 100
		}
	}

	Rectangle {
		id: topLabelBox

		anchors.top: parent.top;
		anchors.left: parent.left;
		anchors.right: parent.right;

		height: 50

		color: Material.primary

		Label {
			text: "Apollo Control"

			verticalAlignment: Text.AlignVCenter
			horizontalAlignment: Text.AlignHCenter
			anchors.fill: parent

			fontSizeMode: Text.Fit
			font.pixelSize: 400
		}
	}

	Flickable {
		anchors.left: parent.left;
		anchors.right: parent.right;
		anchors.top: topLabelBox.bottom;
		anchors.bottom: parent.bottom;

		flickableDirection: Flickable.VerticalFlick

		clip: true

		ColumnLayout {
			width: rootWindow.width

			Layout.margins: 5

			StatusBox {
				Layout.fillWidth: true
				Layout.preferredHeight: 60

				Layout.margins: 5
				Material.elevation: 4
			}

			RowLayout {
				Layout.fillWidth: true
				Layout.preferredHeight: 60

				Layout.margins: 5

				DelayButton {
					text: qsTr("ARM")

					Layout.fillWidth: true
					Layout.fillHeight: true

					enabled: houston.connCode > 2 || checked

					checked: houston.armed

					onCheckedChanged: houston.armed = checked;

					delay: 5000
				}

				DelayButton {
					text: qsTr("FIRE")

					Layout.fillWidth: true
					Layout.fillHeight: true

					enabled: houston.connCode > 3

					onCheckedChanged: {
						if(checked) {
							checked = false;

							console.log("Firing!");

							houston.fire();
						}
					}

					delay: 2000
				}
			}

			Frame {
				Layout.margins: 5
				Layout.fillWidth: true
				Layout.preferredHeight: 60

				Material.elevation: 4

				padding: 2

				Timer {
					interval: 16
					running: true
					repeat: true

					onTriggered: {
						countdownLabel.text = houston.getCountdownString();


						var nC = Material.Grey;
						if(houston.getRemainingSecs() > 30*60)
							nC = Material.Purple
						else if(houston.getRemainingSecs() > 0)
							nC = Material.DeepOrange
						else if(houston.getRemainingSecs() > -30)
							nC = Material.Red
						else if(houston.getRemainingSecs() > -15*60)
							nC = Material.DeepOrange
						else if(houston.getRemainingSecs() > -60*60)
							nC = Material.Orange
						else if(houston.getRemainingSecs() > -3*60*60)
							nC = Material.Amber
						else if(houston.getRemainingSecs() > -6*60*60)
							nC = Material.Yellow
						else
							nC = Material.Green

						countdownLabel.color = Material.color(nC, Material.Shade300)
					}
				}

				Label {
					id: countdownLabel
					anchors.fill: parent;

					text: "COUNTDOWN: "

					Behavior on color {
						ColorAnimation {duration: 200 }
					}

					verticalAlignment: Text.AlignVCenter
					horizontalAlignment: Text.AlignHCenter

					fontSizeMode: Text.Fit
					font.pixelSize: 500

					Timer {
						interval: 250
						repeat: true
						running: true

						onTriggered: {

						}
					}
				}
			}

			Frame {
				padding: 3
				Material.elevation: 3

				Layout.fillWidth: true
				Layout.preferredHeight: ignitionSelection.height + 2*padding

				Layout.margins: 5

				IgnSelector {
					id: ignitionSelection
					anchors.left: parent.left
					anchors.right: parent.right
				}
			}
		}
	}
}
