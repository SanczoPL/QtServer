#include "../include/ioserver.h"
#include <QDebug>
#include <QtNetwork>

constexpr auto PORT{ "Port" };
constexpr auto IP{ "Ip" };
constexpr auto PID{ "Pid" };
constexpr auto ID{ "Id" };
constexpr auto COMMAND{ "Command" };
constexpr auto PING{ "Ping" };
constexpr auto PING_PONG{ "PingPong" };
constexpr auto MESSAGE_TYPE{ "MessageType" };
constexpr auto TIME{ "Time" };
constexpr auto ERROR_DATA{ "Error" };
constexpr auto SENDER{ "Sender" };
constexpr auto FROM{ "From" };
constexpr auto TO{ "To" };

IOServer::IOServer()
	: m_server{ new QTcpServer(this) }
{}

IOServer::IOServer(QJsonObject const& a_config)
	: m_server{ new QTcpServer(this) }
	, m_port{ static_cast<quint16>(a_config[PORT].toInt()) }
{
	IOServer::configure(a_config);
}

IOServer::~IOServer()
{
	m_server->close();
	delete m_server;

	for (auto client : m_clients) client->deleteLater();
}

void IOServer::configure(QJsonObject const& a_config)
{
	m_port = static_cast<quint16>(a_config[PORT].toInt());
	connect(m_server, &QTcpServer::newConnection, this, &IOServer::onConnection);
	m_server->listen(QHostAddress::AnyIPv4, m_port);
	Logger->info("IOServer::configure() listen on port:{} and ip:{}", m_port, QHostAddress::AnyIPv4);
}

void IOServer::onConnection()
{
	while (m_server->hasPendingConnections()) {
		Connection* client{ new Connection{ m_server->nextPendingConnection() } };
		m_clients.push_back(client);
		connect(client, &Connection::disconnected, this, &IOServer::onDisconnection);
		connect(client, &Connection::connected, this, &IOServer::onConnected);
		connect(client, &Connection::newMessage, this, &IOServer::onNewMessage);
		connect(this, &IOServer::sendMessage, client, &Connection::onSendMessage);
		Logger->info("Connected to {}", client->ip().toStdString());
	}
}

void IOServer::onDisconnection()
{
	Logger->trace("IOServer::onDisconnection()");
	Connection* client{ dynamic_cast<Connection*>(QObject::sender()) };
	m_clients.removeAll(client);
	delete client;
}

void IOServer::onConnected()
{
	Logger->trace("IOServer::onConnected()");
	emit(connected());
}

void IOServer::onNewMessage(QByteArray const a_rawMessage)
{
	Logger->trace("IOServer::onNewMessage()");
	emit(newMessage(a_rawMessage));
}
