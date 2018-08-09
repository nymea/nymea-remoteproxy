#ifndef JSONRPCSERVER_H
#define JSONRPCSERVER_H

#include <QObject>
#include <QVariant>

#include "proxyclient.h"
#include "jsonrpc/jsonreply.h"
#include "transportinterface.h"
#include "jsonrpc/jsonhandler.h"
#include "jsonrpc/authenticationhandler.h"

namespace remoteproxy {

class JsonRpcServer : public JsonHandler
{
    Q_OBJECT
public:
    explicit JsonRpcServer(QObject *parent = nullptr);
    ~JsonRpcServer() override;

    QString name() const override;

    Q_INVOKABLE JsonReply *Hello(const QVariantMap &params, ProxyClient *proxyClient = nullptr) const;
    Q_INVOKABLE JsonReply *Introspect(const QVariantMap &params, ProxyClient *proxyClient = nullptr) const;

    void sendNotification(const QString &nameSpace, const QString &method, const QVariantMap &params, ProxyClient *proxyClient = nullptr);

signals:
    void TunnelEstablished(const QVariantMap &params);

private:
    QHash<QString, JsonHandler *> m_handlers;
    QHash<JsonReply *, ProxyClient *> m_asyncReplies;
    QList<ProxyClient *> m_clients;
    int m_notificationId;

    void sendResponse(ProxyClient *client, int commandId, const QVariantMap &params = QVariantMap());
    void sendErrorResponse(ProxyClient *client, int commandId, const QString &error);

    QString formatAssertion(const QString &targetNamespace, const QString &method, JsonHandler *handler, const QVariantMap &data) const;

    void registerHandler(JsonHandler *handler);
    void unregisterHandler(JsonHandler *handler);

private slots:
    void setup();
    void asyncReplyFinished();

public slots:
    // Client registration for JSON RPC traffic
    void registerClient(ProxyClient *proxyClient);
    void unregisterClient(ProxyClient *proxyClient);

    // Process data from client
    void processData(ProxyClient *proxyClient, const QByteArray &data);

};

}

#endif // JSONRPCSERVER_H
