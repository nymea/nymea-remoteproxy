#ifndef JSONRPCSERVER_H
#define JSONRPCSERVER_H

#include <QObject>

#include "transportinterface.h"
#include "jsonrpc/jsonhandler.h"
#include "jsonrpc/authenticationhandler.h"

class JsonRpcServer : public JsonHandler
{
    Q_OBJECT
public:
    explicit JsonRpcServer(QObject *parent = nullptr);
    ~JsonRpcServer() override;

    QString name() const override;

    QHash<QString, JsonHandler *> handlers() const;

    void registerTransportInterface(TransportInterface *interface);
    void unregisterTransportInterface(TransportInterface *interface);

    Q_INVOKABLE JsonReply *Hello(const QVariantMap &params) const;
    Q_INVOKABLE JsonReply *Introspect(const QVariantMap &params) const;

private:
    QList<TransportInterface *> m_interfaces;
    QHash<QString, JsonHandler *> m_handlers;
    QHash<JsonReply *, TransportInterface *> m_asyncReplies;
    QHash<QUuid, TransportInterface*> m_clientTransports;
    QHash<QString, JsonReply*> m_pairingRequests;

    int m_notificationId;

    void sendResponse(TransportInterface *interface, const QUuid &clientId, int commandId, const QVariantMap &params = QVariantMap());
    void sendErrorResponse(TransportInterface *interface, const QUuid &clientId, int commandId, const QString &error);
    QString formatAssertion(const QString &targetNamespace, const QString &method, JsonHandler *handler, const QVariantMap &data) const;

    void registerHandler(JsonHandler *handler);



private slots:
    void setup();
    void clientConnected(const QUuid &clientId);
    void clientDisconnected(const QUuid &clientId);
    void processData(const QUuid &clientId, const QByteArray &data);
//    void sendNotification(const QVariantMap &params);
//    void asyncReplyFinished();

};

#endif // JSONRPCSERVER_H
