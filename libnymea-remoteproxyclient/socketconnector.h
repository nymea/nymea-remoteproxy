#ifndef SOCKETCONNECTOR_H
#define SOCKETCONNECTOR_H

#include <QObject>
#include <QHostAddress>

class SocketConnector : public QObject
{
    Q_OBJECT
public:
    explicit SocketConnector(QObject *parent = nullptr);
    virtual ~SocketConnector() = 0;

    virtual void sendData(const QByteArray &data) = 0;
    virtual bool isConnected() = 0;

signals:
    void connected();
    void disconnected();

    void dataReceived(const QByteArray &data);

public slots:
    virtual void connectServer(const QHostAddress &address, quint16 port) = 0;
    virtual void disconnectServer() = 0;

};

#endif // SOCKETCONNECTOR_H
