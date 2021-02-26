#include "../include/sockio.h"

SockIO::SockIO(QTcpSocket* a_socket, QObject* parent)
	: QObject(parent)
	, m_socket{ a_socket }
{
	connect(m_socket, &QTcpSocket::readyRead, this, &SockIO::onReadyRead);
}

bool SockIO::hasMessages()
{
	return !m_messageQueue.isEmpty();
}

Message SockIO::nextMessage()
{
	if (!hasMessages()) qFatal("No mesages in queue!");

	Message const MESSAGE{ m_messageQueue[0] };
	m_messageQueue.pop_front();
	return MESSAGE;
}

bool SockIO::sendMessage(Message const& a_message)
{
	Logger->trace("SockIO::sendMessage()");
	if (m_socket->write(a_message.rawData()) < 0) {
		Logger->warn("Failed to send message to host", m_socket->peerAddress().toString().toStdString());
		return false;
	}

	return true;
}

void SockIO::onReadyRead()
{
	m_bufer += m_socket->readAll();
	Logger->trace("Recived data from ip:{}, bufSize:{}", m_socket->peerAddress().toString().toStdString(), m_bufer.size());
	Logger->trace("while({} >= {})", m_bufer.size() ,static_cast<int>(sizeof(Message::Header)) );
	while (m_bufer.size() >= static_cast<int>(sizeof(Message::Header))) {
		Logger->trace("checkPrefix:{}", Message::checkPrefix(m_bufer));
		if (Message::checkPrefix(m_bufer)) {
			auto messageSize = Message::validate(m_bufer);
			Logger->trace("validate:{}", messageSize);
			if (messageSize > 0) {
				m_messageQueue.push_back(Message{ m_bufer });
				m_bufer.remove(0, messageSize);
				Logger->trace("emit new message:");
				emit(newMessage());
			}
			else {
				Logger->trace("messageSize = 0");
				break;
			}
		}
		else {
			Logger->warn("Buffer out of order {}", m_socket->peerAddress().toString().toStdString());
			m_bufer.remove(0, 1);
		}
	}
}
