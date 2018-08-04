#include "engine.h"
#include "jsonrpcserver.h"
#include "loggingcategories.h"
#include "jsonrpc/jsontypes.h"

#include <QJsonDocument>
#include <QCoreApplication>

JsonRpcServer::JsonRpcServer(QObject *parent) :
    JsonHandler(parent)
{
    // Methods
    QVariantMap params; QVariantMap returns;

    params.clear(); returns.clear();
    setDescription("Hello", "Once connected to this server, a client can get information about the server by saying Hello. The response informs the client about the server.");
    setParams("Hello", params);
    returns.insert("server", JsonTypes::basicTypeToString(JsonTypes::String));
    returns.insert("name", JsonTypes::basicTypeToString(JsonTypes::String));
    returns.insert("version", JsonTypes::basicTypeToString(JsonTypes::String));
    returns.insert("apiVersion", JsonTypes::basicTypeToString(JsonTypes::String));
    setReturns("Hello", returns);

    params.clear(); returns.clear();
    setDescription("Introspect", "Introspect this API.");
    setParams("Introspect", params);
    returns.insert("methods", JsonTypes::basicTypeToString(JsonTypes::Object));
    returns.insert("types", JsonTypes::basicTypeToString(JsonTypes::Object));
    returns.insert("notifications", JsonTypes::basicTypeToString(JsonTypes::Object));
    setReturns("Introspect", returns);

    QMetaObject::invokeMethod(this, "setup", Qt::QueuedConnection);
}

JsonRpcServer::~JsonRpcServer()
{
    qCDebug(dcJsonRpc()) << "Shutting down Json RPC server";
}

QString JsonRpcServer::name() const
{
    return "RemoteProxy";
}

QHash<QString, JsonHandler *> JsonRpcServer::handlers() const
{
    return m_handlers;
}

void JsonRpcServer::registerTransportInterface(TransportInterface *interface)
{
    if (m_interfaces.contains(interface))
        return;

    connect(interface, &TransportInterface::clientConnected, this, &JsonRpcServer::clientConnected);
    connect(interface, &TransportInterface::clientDisconnected, this, &JsonRpcServer::clientDisconnected);
    connect(interface, &TransportInterface::dataAvailable, this, &JsonRpcServer::processData);
    m_interfaces.append(interface);
}

void JsonRpcServer::unregisterTransportInterface(TransportInterface *interface)
{
    disconnect(interface, &TransportInterface::clientConnected, this, &JsonRpcServer::clientConnected);
    disconnect(interface, &TransportInterface::clientDisconnected, this, &JsonRpcServer::clientDisconnected);
    disconnect(interface, &TransportInterface::dataAvailable, this, &JsonRpcServer::processData);
    m_interfaces.removeOne(interface);
}

JsonReply *JsonRpcServer::Hello(const QVariantMap &params) const
{
    Q_UNUSED(params)

    QVariantMap data;
    data.insert("server", QCoreApplication::applicationName());
    data.insert("name", Engine::instance()->serverName());
    data.insert("version", QCoreApplication::applicationVersion());
    data.insert("apiVersion", API_VERSION_STRING);

    return createReply(data);
}

JsonReply *JsonRpcServer::Introspect(const QVariantMap &params) const
{
    Q_UNUSED(params)

    QVariantMap data;
    data.insert("types", JsonTypes::allTypes());

    QVariantMap methods;
    foreach (JsonHandler *handler, m_handlers) {
        methods.unite(handler->introspect(QMetaMethod::Method));
    }

    data.insert("methods", methods);

    QVariantMap signalsMap;
    foreach (JsonHandler *handler, m_handlers) {
        signalsMap.unite(handler->introspect(QMetaMethod::Signal));
    }

    data.insert("notifications", signalsMap);

    return createReply(data);
}

void JsonRpcServer::sendResponse(TransportInterface *interface, const QUuid &clientId, int commandId, const QVariantMap &params)
{
    QVariantMap response;
    response.insert("id", commandId);
    response.insert("status", "success");
    response.insert("params", params);

    QByteArray data = QJsonDocument::fromVariant(response).toJson(QJsonDocument::Compact);
    qCDebug(dcJsonRpcTraffic()) << "Sending data:" << data;
    interface->sendData(clientId, data);
}

void JsonRpcServer::sendErrorResponse(TransportInterface *interface, const QUuid &clientId, int commandId, const QString &error)
{
    QVariantMap errorResponse;
    errorResponse.insert("id", commandId);
    errorResponse.insert("status", "error");
    errorResponse.insert("error", error);

    QByteArray data = QJsonDocument::fromVariant(errorResponse).toJson(QJsonDocument::Compact);
    qCDebug(dcJsonRpcTraffic()) << "Sending data:" << data;
    interface->sendData(clientId, data);
}

QString JsonRpcServer::formatAssertion(const QString &targetNamespace, const QString &method, JsonHandler *handler, const QVariantMap &data) const
{
    QJsonDocument doc = QJsonDocument::fromVariant(handler->introspect(QMetaMethod::Method).value(targetNamespace + "." + method));
    QJsonDocument doc2 = QJsonDocument::fromVariant(data);
    return QString("\nMethod: %1\nTemplate: %2\nValue: %3")
            .arg(targetNamespace + "." + method)
            .arg(QString(doc.toJson(QJsonDocument::Indented)))
            .arg(QString(doc2.toJson(QJsonDocument::Indented)));
}

void JsonRpcServer::registerHandler(JsonHandler *handler)
{
    m_handlers.insert(handler->name(), handler);
    qCDebug(dcJsonRpc()) << "Register handler" << handler->name();
    for (int i = 0; i < handler->metaObject()->methodCount(); ++i) {
        QMetaMethod method = handler->metaObject()->method(i);
        if (method.methodType() == QMetaMethod::Signal && QString(method.name()).contains(QRegExp("^[A-Z]"))) {
            QObject::connect(handler, method, this, metaObject()->method(metaObject()->indexOfSlot("sendNotification(QVariantMap)")));
        }
    }
}

void JsonRpcServer::setup()
{
    registerHandler(this);
    registerHandler(new AuthenticationHandler(this));
}

