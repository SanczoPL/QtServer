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
	m_timer = new QTimer(this);
	m_timer->start(1000);
	connect(m_timer, SIGNAL(timeout()), this, SLOT(onUpdate()));

	connect(m_server, &QTcpServer::newConnection, this, &IOServer::onConnection);
	connect(this, &IOServer::listenForConnection, this, &IOServer::onListenForConnection);

	m_server->listen(QHostAddress::AnyIPv4, m_port);
	Logger->info("IOServer::IOServer() listen on port:{} and ip:{}", m_port, QHostAddress::AnyIPv4);
	emit(listenForConnection());

}

IOServer::~IOServer()
{
	m_server->close();
	delete m_server;

	for (auto client : m_clients) client->deleteLater();
}

void IOServer::onUpdate()
{
	onSendPing();
}

void IOServer::onSendPing() {
	qint32 now = qint32(QDateTime::currentMSecsSinceEpoch());
	QJsonObject json = { {MESSAGE_TYPE, PING}, {TIME, now} , {FROM, ""}, {TO, ""} };
	QJsonObject cmd = { {COMMAND, json} };
	Message msg{};
	QByteArray stateData{ QJsonDocument{cmd}.toJson(QJsonDocument::Compact) };
	msg.fromData(stateData, Message::JSON, 1);
	Logger->trace("Broadcaster::onSendPing() from {}", 1);
	sendMessage(msg);
}

void IOServer::configure(QJsonObject const& a_config)
{
	m_port = static_cast<quint16>(a_config[PORT].toInt());
	connect(m_server, &QTcpServer::newConnection, this, &IOServer::onConnection);
	connect(this, &IOServer::listenForConnection, this, &IOServer::onListenForConnection);
	m_server->listen(QHostAddress::AnyIPv4, m_port);
	Logger->info("IOServer::configure() listen on port:{} and ip:{}", m_port, QHostAddress::AnyIPv4);
	emit(listenForConnection());
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
		connect(m_timer, SIGNAL(timeout()), client, SLOT(onUpdate()));
		Logger->info("Connected to {}", client->ip().toStdString());
	}
}

void IOServer::onDisconnection()
{
	Connection* client{ dynamic_cast<Connection*>(QObject::sender()) };
	m_clients.removeAll(client);

	delete client;
}

void IOServer::onConnected()
{
	emit(connected());
}

void IOServer::onListenForConnection()
{
	Logger->trace("IOServer::onListenForConnection()");
}

void IOServer::onNewMessage(QByteArray const a_rawMessage)
{
	emit(newMessage(a_rawMessage));
}
