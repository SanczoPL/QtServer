#include "../include/connection.h"

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
		case Message::BINARY: emit(newMessage(message.rawData())); break;
		case Message::JSON: emit(newMessage(message.rawData())); break;
		default: Logger->warn("Recived unknown message type from:{}", ip().toStdString()); break;
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