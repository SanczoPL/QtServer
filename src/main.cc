#include <QCoreApplication>
#include <QJsonDocument>

#include "../include/ioserver.h"
#include "configreader.h"
#include "includespdlog.h"

constexpr auto COMM{ "server.json" };
constexpr auto LOG_LEVEL{ "LogLevel" };
constexpr auto SERVER{ "Server" };


int main(int argc, char* argv[])
{
	QCoreApplication application(argc, argv);

	QString configName{ COMM };
	if (argc > 1)
	{
		configName = argv[1];
	}
	ConfigReader* configReader = new ConfigReader();
	QJsonObject jObject;
	if (!configReader->readConfig(configName, jObject))
	{
		Logger->error("File {} not readed", configName.toStdString());
		return -66;
	}
	delete configReader;

	Logger->set_level(static_cast<spdlog::level::level_enum>(jObject[LOG_LEVEL].toInt()));
	IOServer server{ jObject[SERVER].toObject() };
	return application.exec();
}
