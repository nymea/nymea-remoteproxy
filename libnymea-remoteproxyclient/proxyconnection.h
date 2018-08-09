#ifndef SOCKETCONNECTOR_H
#define SOCKETCONNECTOR_H

#include <QObject>
#include <QHostAddress>

namespace remoteproxyclient {

class ProxyConnection : public QObject
{
    Q_OBJECT
public:
    explicit ProxyConnection(QObject *parent = nullptr);
    virtual ~ProxyConnection() = 0;

    virtual void sendData(const QByteArray &data) = 0;
    virtual bool isConnected() = 0;

    virtual QUrl serverUrl() const = 0;

    bool allowSslErrors() const;
    void setAllowSslErrors(bool allowSslErrors);

private:
    bool m_allowSslErrors = false;

signals:
    void connectedChanged(bool connected);
    void dataReceived(const QByteArray &data);
    void errorOccured();
    void sslErrorOccured();

public slots:
    virtual void connectServer(const QHostAddress &address, quint16 port) = 0;
    virtual void disconnectServer() = 0;

};

}

#endif // SOCKETCONNECTOR_H
