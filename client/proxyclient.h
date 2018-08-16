#ifndef PROXYCLIENT_H
#define PROXYCLIENT_H

#include <QObject>
#include <QLoggingCategory>

#include "remoteproxyconnection.h"

using namespace remoteproxyclient;

Q_DECLARE_LOGGING_CATEGORY(dcProxyClient)

class ProxyClient : public QObject
{
    Q_OBJECT
public:
    explicit ProxyClient(const QString &name, const QUuid &uuid, QObject *parent = nullptr);

    void setInsecure(bool insecure);

private:
    QString m_name;
    QUuid m_uuid;
    QString m_token;
    bool m_insecure = false;

    RemoteProxyConnection *m_connection = nullptr;

private slots:
    void onErrorOccured(RemoteProxyConnection::Error error);
    void onClientReady();
    void onAuthenticationFinished();
    void onRemoteConnectionEstablished();
    void onClientDisconnected();
    void onSslErrors(const QList<QSslError> errors);

public slots:
    void start(const QUrl &url, const QString &token);

};

#endif // PROXYCLIENT_H
