#ifndef LAUNCHCONTROLLER_H
#define LAUNCHCONTROLLER_H

#include <QObject>

#include <QtMqtt/QtMqtt>
#include <QtMqtt/QMqttMessage>

#include <QDateTime>

#define NYE_EPOCH_TIME 1546297199

class LaunchController : public QObject
{
	Q_OBJECT

	Q_PROPERTY(int  connCode READ getConnectionCode NOTIFY connectionChanged)
	Q_PROPERTY(bool armed	 READ isArmed WRITE setArmed NOTIFY connectionChanged)

	Q_PROPERTY(QDateTime countdownTarget READ getCountdownTarget NOTIFY countdownChanged)

	Q_PROPERTY(int currentSelection READ getCurrentSelection WRITE setSelection NOTIFY selectionChanged)

	Q_PROPERTY(int standbyBits READ getStandbyBits NOTIFY standbyChanged)
	Q_PROPERTY(int usedSlots	READ getUsedSlots	  NOTIFY slotsChanged)

private:
	QMqttClient mqtt_client;
	QTimer	mqtt_reconnect;

	enum ApolloConnection : uint8_t {
		DISCONNECTED = 1,
		UNPOWERED = 2,
		DISARMED = 3,
		ARMED = 4,
		FIRING = 5
	} apolloStatus;

	QDateTime countdownTarget;

	int		currentSelection;
	quint8	standbyBits;

	quint16	usedSlots;

public:
	explicit LaunchController(QObject *parent = nullptr);

	int  getConnectionCode();

	bool isArmed();
	void setArmed(bool armStatus);

	int  getCurrentSelection();
	void setSelection(int nSelection);

	int  getStandbyBits();
	int  getUsedSlots();

	QDateTime getCountdownTarget();
	Q_INVOKABLE QString getCountdownString();
	Q_INVOKABLE int	  getRemainingSecs();

	Q_INVOKABLE void fire();

signals:
	void connectionChanged();
	void countdownChanged();

	void selectionChanged();
	void standbyChanged();
	void slotsChanged();

	void gotData(const QString &key, const QByteArray &data);


public slots:
	void mqtt_handleData(const QMqttMessage &msg);
};

#endif // LAUNCHCONTROLLER_H
