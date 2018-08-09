#ifndef JSONRPCCLIENT_H
#define JSONRPCCLIENT_H

#include <QObject>
#include <QVariantMap>
#include <QLoggingCategory>

#include "jsonreply.h"
#include "proxyconnection.h"

Q_DECLARE_LOGGING_CATEGORY(dcRemoteProxyClientJsonRpc)
Q_DECLARE_LOGGING_CATEGORY(dcRemoteProxyClientJsonRpcTraffic)

namespace remoteproxyclient {

class JsonRpcClient : public QObject
{
    Q_OBJECT
public:
    explicit JsonRpcClient(QObject *parent = nullptr);

    JsonReply *callHello();

private:
    ProxyConnection *m_connection = nullptr;

    int m_commandId = 0;

    QHash<int, JsonReply *> m_replies;

    QString m_serverName;
    QString m_proxyServerName;
    QString m_proxyServerVersion;
    QString m_proxyApiVersion;

    void sendRequest(const QVariantMap &request);

signals:

public slots:
    void onConnectedChanged(bool connected);
    void processData(const QByteArray &data);

};

}

#endif // JSONRPCCLIENT_H
