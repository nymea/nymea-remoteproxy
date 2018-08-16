#ifndef SOCKETCONNECTOR_H
#define SOCKETCONNECTOR_H

#include <QObject>
#include <QSslError>
#include <QHostAddress>

namespace remoteproxyclient {

class ProxyConnection : public QObject
{
    Q_OBJECT
public:
    explicit ProxyConnection(QObject *parent = nullptr);
    virtual ~ProxyConnection() = 0;

    virtual void sendData(const QByteArray &data) = 0;

    virtual QUrl serverUrl() const = 0;

    virtual void ignoreSslErrors() = 0;
    virtual void ignoreSslErrors(const QList<QSslError> &errors) = 0;

    bool connected();

private:
    bool m_connected = false;

protected:
    void setConnected(bool connected);

signals:
    void connectedChanged(bool connected);
    void dataReceived(const QByteArray &data);
    void errorOccured();
    void sslErrors(const QList<QSslError> &errors);

public slots:
    virtual void connectServer(const QUrl &serverUrl) = 0;
    virtual void disconnectServer() = 0;

};

}

#endif // SOCKETCONNECTOR_H
