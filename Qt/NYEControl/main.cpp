#include <QGuiApplication>
#include <QQmlApplicationEngine>

#include <QQmlContext>

#include <QQuickStyle>

#include "launchcontroller.h"

int main(int argc, char *argv[])
{
	QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

	QGuiApplication app(argc, argv);

	QQmlApplicationEngine engine;
	QQuickStyle::setStyle("Material");

	LaunchController houston(nullptr);

	engine.rootContext()->setContextProperty("houston", &houston);

	engine.load(QUrl(QStringLiteral("qrc:/main.qml")));
	if (engine.rootObjects().isEmpty())
		return -1;

	return app.exec();
}
