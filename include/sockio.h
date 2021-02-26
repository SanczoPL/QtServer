#ifndef SOCKIO_H
#define SOCKIO_H

#include <QObject>
#include <QtNetwork>
#include "message.h"
#include "configreader.h"
#include "includespdlog.h"


class SockIO : public QObject {
	Q_OBJECT
public:
	explicit SockIO(QTcpSocket* a_socket, QObject* a_parent = nullptr);

	bool hasMessages();
	Message nextMessage();
	bool sendMessage(Message const& a_message);

signals:
	void newMessage();

private slots:
	void onReadyRead();
	

private:
	QTcpSocket* m_socket;
	QByteArray m_bufer{};
	QVector<Message> m_messageQueue{};
};

#endif // SOCKIO_H
