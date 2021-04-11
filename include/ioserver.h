#ifndef IOSERVER_H
#define IOSERVER_H

#include <QJsonObject>
#include <QObject>
#include <QVector>
#include "connection.h"
#include "topicmanager.h"
#include "configreader.h"
#include "includespdlog.h"


class QTcpServer;

class IOServer : public QObject {
	Q_OBJECT

public:
	IOServer();
	IOServer(QJsonObject const& a_config);
	~IOServer();
	void configure(QJsonObject const& a_config);

signals:
	void connected();
	void sendMessage(Message const& a_message);
	void newMessage(QByteArray const a_rawMessage);

private slots:
	void onConnection();
	void onDisconnection();
	void onConnected();
	void onNewMessage(QByteArray const a_rawMessage);

private:
	QTcpServer* m_server;
	quint16 m_port{};
	QVector<Connection*> m_clients{};
};

#endif // IOSERVER_H
