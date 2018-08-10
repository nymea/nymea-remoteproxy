#ifndef JSONRPCCLIENT_H
#define JSONRPCCLIENT_H

#include <QUuid>
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
    explicit JsonRpcClient(ProxyConnection *connection, QObject *parent = nullptr);

    JsonReply *callHello();
    JsonReply *callAuthenticate(const QUuid &clientUuid, const QString &clientName, const QString &token);

private:
    ProxyConnection *m_connection = nullptr;

    int m_commandId = 0;

    QHash<int, JsonReply *> m_replies;

    void sendRequest(const QVariantMap &request);

signals:

public slots:
    void onConnectedChanged(bool connected);
    void processData(const QByteArray &data);

};

}

#endif // JSONRPCCLIENT_H
