#include "launchcontroller.h"

LaunchController::LaunchController(QObject *parent) : QObject(parent),
	mqtt_client(this), mqtt_reconnect(this),
	apolloStatus(DISCONNECTED),
	currentSelection(-1), standbyBits(0), usedSlots(0)
{
	connect(&mqtt_client, &QMqttClient::connected, this,
			  [this]() {
		qDebug()<<"Connected!";

		emit connectionChanged();

		auto sub = this->mqtt_client.subscribe(QString("Xasin/NYEv3/#"), 2);
		connect(sub, &QMqttSubscription::messageReceived, this,
				  &LaunchController::mqtt_handleData);
	});

	connect(&mqtt_reconnect, &QTimer::timeout, this,
			  [this]() {
		if(this->mqtt_client.state() != QMqttClient::Connected)
			this->mqtt_client.connectToHost();
	});

	mqtt_client.setCleanSession(false);
	mqtt_client.setHostname("iot.eclipse.org");
	mqtt_client.setPort(1883);

	mqtt_reconnect.setInterval(3000);
	mqtt_reconnect.setSingleShot(false);
	mqtt_reconnect.start();

	countdownTarget = QDateTime::fromSecsSinceEpoch(NYE_EPOCH_TIME);
}

int LaunchController::getConnectionCode() {
	if(mqtt_client.state() != QMqttClient::Connected)
		return 0;

	return int(apolloStatus);
}

bool LaunchController::isArmed() {
	return apolloStatus >= ARMED;
}
void LaunchController::setArmed(bool armStatus) {
	if(isArmed() == armStatus)
		return;

	auto data = QByteArray();
	data.append(armStatus);

	mqtt_client.publish(QString("Xasin/NYEv3/Arm"), data, 1);
}

int LaunchController::getCurrentSelection() {
	return currentSelection;
}
void LaunchController::setSelection(int nSelection) {
	if(nSelection == currentSelection)
		return;

	uint8_t dummy = nSelection;

	auto data = QByteArray();
	data.append(dummy);

	mqtt_client.publish(QString("Xasin/NYEv3/Selection"), data, 1, true);

	currentSelection = nSelection;
	emit selectionChanged();
}

int LaunchController::getStandbyBits() {
	return standbyBits;
}
int LaunchController::getUsedSlots() {
	return usedSlots;
}

QDateTime LaunchController::getCountdownTarget() {
	return countdownTarget;
}
QString LaunchController::getCountdownString() {
	qint64 tDiff = countdownTarget.msecsTo(QDateTime::currentDateTime());

	bool negative = tDiff < 0;
	if(negative)
		tDiff *= -1;

	int mSecs = tDiff%1000;
	tDiff /= 1000;

	int secs  = tDiff%60;
	int mins  = (tDiff/60)%60;
	int hours = (tDiff/3600)%24;
	int days  = (tDiff/(3600*24));

	QString pref = QString("T") + (negative ? "-" : "+");

	if(days != 0)
		return pref + QString().sprintf("%d:%02d:%02d:%02d", days, hours, mins, secs);
	else if(hours != 0)
		return pref + QString().sprintf("%02d:%02d:%02d.%01d", hours, mins, secs, mSecs/100);
	else
		return pref + QString().sprintf("%02d:%02d.%01d", mins, secs, mSecs/100);
}

int LaunchController::getRemainingSecs() {
	return countdownTarget.secsTo(QDateTime::currentDateTime());
}

void LaunchController::fire() {
	if(apolloStatus != ARMED)
		return;

	auto data = QByteArray();
	data.append(currentSelection);

	mqtt_client.publish(QString("Xasin/NYEv3/Fire"), data, 2);

	setSelection(currentSelection+1);
}

void LaunchController::mqtt_handleData(const QMqttMessage &msg) {
	qDebug()<<"Got data!";

	QString key = msg.topic().levels()[2];

	if(key == "Connection") {
		if(msg.payload().size() == 0)
			apolloStatus = DISCONNECTED;
		else
			apolloStatus = *(reinterpret_cast<const ApolloConnection *>(msg.payload().data()));

		emit connectionChanged();
	}
	else if(key == "Countdown") {
		int mSecs = *(reinterpret_cast<const uint16_t*>(msg.payload().data()));

		QDateTime nextCountdown = QDateTime::currentDateTime();
		if(mSecs == 0)
			nextCountdown = QDateTime::fromSecsSinceEpoch(NYE_EPOCH_TIME);
		else
			nextCountdown = nextCountdown.addMSecs(mSecs);

		if(nextCountdown != countdownTarget) {
			countdownTarget = nextCountdown;
			emit countdownChanged();
		}
	}
	else if(key == "Selection") {
		int nSelection = *(reinterpret_cast<const uint8_t *>
								 (msg.payload().data()));

		if(nSelection != currentSelection) {
			currentSelection = nSelection;
			emit selectionChanged();
		}
	}
	else if(key == "StandbyOn") {
		standbyBits = *(reinterpret_cast<const uint8_t *>
							 (msg.payload().data()));

		qDebug()<<"Standby on is:"<<standbyBits;
		emit standbyChanged();
	}
	else if(key == "UsedSlots") {
		usedSlots = *(reinterpret_cast<const uint16_t *>
						  (msg.payload().data()));
		emit slotsChanged();
	}

	emit gotData(key, msg.payload());
}
