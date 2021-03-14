#ifndef CONNECTION_H
#define CONNECTION_H

#include <QObject>
#include <QVector>
#include "sockio.h"
#include "configreader.h"
#include "includespdlog.h"


class Connection : public SockIO {
	Q_OBJECT
public:
	explicit Connection(QTcpSocket* a_socket, QObject* a_parent = nullptr);
	~Connection();

	QString ip() { return m_ip; }
	void updatePing(QByteArray const a_rawMessage);

signals:
	void disconnected();
	void connected();
	void subscribeAck();
	void newMessage(QByteArray const a_rawMessage);

public slots:
	void onSendMessage(Message const& a_message);
	void onUpdate();
	

private slots:
	void onDisconnected();
	void onNewMessage();

private:
	QString m_ip{};
};

#endif // CONNECTION_H
