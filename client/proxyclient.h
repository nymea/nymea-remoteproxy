#ifndef PROXYCLIENT_H
#define PROXYCLIENT_H

#include <QObject>

#include "remoteproxyconnection.h"

using namespace remoteproxyclient;

class ProxyClient : public QObject
{
    Q_OBJECT
public:
    explicit ProxyClient(QObject *parent = nullptr);

    void setHostAddress(const QHostAddress &hostAddress);
    void setPort(const int &port);

private:
    QString m_token;
    QHostAddress m_hostAddress = QHostAddress::LocalHost;
    int m_port = 1212;

    RemoteProxyConnection *m_connection = nullptr;

signals:


private slots:
    void onErrorOccured(RemoteProxyConnection::Error error);
    void onClientReady();
    void onAuthenticationFinished();

public slots:
    void start(const QString &token);

};

#endif // PROXYCLIENT_H
