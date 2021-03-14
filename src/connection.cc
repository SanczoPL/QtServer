#include "../include/connection.h"

constexpr auto COMMAND{ "Command" };
constexpr auto PING{ "Ping" };
constexpr auto PING_PONG{ "PingPong" };
constexpr auto MESSAGE_TYPE{ "MessageType" };
constexpr auto TIME{ "Time" };


Connection::Connection(QTcpSocket* a_socket, QObject* a_parent)
	: SockIO{ a_socket, a_parent }
	, m_ip{ a_socket->peerAddress().toString() }
{
	connect(this, &SockIO::newMessage, this, &Connection::onNewMessage);
	connect(a_socket, &QTcpSocket::disconnected, this, &Connection::onDisconnected);
	emit(connected());
	Logger->trace("Connection::Connection() connected()");
}

Connection::~Connection() {}

void Connection::onUpdate()
{
	Logger->trace("Connection::onUpdate()");

}

void Connection::onNewMessage()
{
	Logger->trace("Connection::onNewMessage() Recived full message from host: {}", ip().toStdString());
	while (hasMessages()) {
		Message message{ nextMessage() };
		Logger->trace("message.type():{}", message.type());
		switch (message.type()) {
		case Message::PING: updatePing(message.rawData());  emit(newMessage(message.rawData())); break;
		case Message::BINARY: emit(newMessage(message.rawData())); break;
		case Message::JSON: emit(newMessage(message.rawData())); break;
		default: Logger->warn("Recived unknown message type from:{}", ip().toStdString()); break;
		}
	}
}

void Connection::updatePing(QByteArray const a_rawMessage)
{
	Message message{};
	message.parse(a_rawMessage);
	QJsonObject jOut{ {"none", "none"} };
	bool ret = message.parse(a_rawMessage);
	if (ret <= 0) {
		Logger->error("Broadcaster::onNewMessage() msg not correct");
	}
	else
	{
		Message::Header m_header = message.header();
		Logger->trace("Broadcaster::onNewMessage() a_message:{} m_header:{}", a_rawMessage.size() - 20, m_header.size);
		Logger->trace("Broadcaster::onNewMessage() sender:{}", message.sender());
		if (message.type() == Message::PING)
		{
			Logger->debug("Recived message that is Message::PING");
			if (message.isValid())
			{
				const QJsonDocument jDoc{ QJsonDocument::fromJson(message.content()) };
				if (!jDoc.isObject())
				{
					Logger->error("Broadcaster::onNewMessage() Recived invalid  Message::JSON");
				}
				jOut = jDoc.object()[COMMAND].toObject();
				if (jOut[MESSAGE_TYPE].toString() == PING_PONG)
				{
					qint32  old = jOut[TIME].toInt();
					qint32 now = int(QDateTime::currentMSecsSinceEpoch());

					QDateTime deltaOld = QDateTime::fromMSecsSinceEpoch(old);
					QDateTime deltaNew = QDateTime::fromMSecsSinceEpoch(now);

					QDateTime nowData = QDateTime::currentDateTime();
					qint64 delta = deltaOld.msecsTo(deltaNew);
					qint64 m_ping = delta;
					Logger->trace("m_ping:{}", m_ping);
				}
			}
		}
	}
}

void Connection::onDisconnected()
{
	Logger->warn("Connection::onDisconnected() Disconnected from:{}", ip().toStdString());
	emit(disconnected());
}

void Connection::onSendMessage(Message const& a_message)
{
	Logger->trace("Connection::onSendMessage()");
	Message message{};
	bool ret = message.parse(a_message.rawData());
	if (ret <= 0) {
		Logger->error("Connection::onSendMessage() msg not correct");
	}
	if (sendMessage(a_message))
	{
		Logger->trace("Connection::onSendMessage() correct");
	}
}