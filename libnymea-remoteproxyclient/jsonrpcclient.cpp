#include "jsonrpcclient.h"
#include "proxyconnection.h"

#include <QJsonDocument>

Q_LOGGING_CATEGORY(dcRemoteProxyClientJsonRpc, "RemoteProxyClientJsonRpc")
Q_LOGGING_CATEGORY(dcRemoteProxyClientJsonRpcTraffic, "RemoteProxyClientJsonRpcTraffic")

namespace remoteproxyclient {

JsonRpcClient::JsonRpcClient(ProxyConnection *connection, QObject *parent) :
    QObject(parent),
    m_connection(connection)
{

}

JsonReply *JsonRpcClient::callHello()
{
    JsonReply *reply = new JsonReply(m_commandId, "RemoteProxy", "Hello", QVariantMap(), this);
    qCDebug(dcRemoteProxyClientJsonRpc()) << "Calling" << QString("%1.%2").arg(reply->nameSpace()).arg(reply->method());
    sendRequest(reply->requestMap());
    m_replies.insert(m_commandId, reply);
    return reply;
}

JsonReply *JsonRpcClient::callAuthenticate(const QUuid &clientUuid, const QString &clientName, const QString &token)
{
    QVariantMap params;
    params.insert("name", clientName);
    params.insert("uuid", clientUuid.toString());
    params.insert("token", token);

    JsonReply *reply = new JsonReply(m_commandId, "Authentication", "Authenticate", params, this);
    qCDebug(dcRemoteProxyClientJsonRpc()) << "Calling" << QString("%1.%2").arg(reply->nameSpace()).arg(reply->method()) << params;
    sendRequest(reply->requestMap());
    m_replies.insert(m_commandId, reply);
    return reply;
}

void JsonRpcClient::sendRequest(const QVariantMap &request)
{
    QByteArray data = QJsonDocument::fromVariant(request).toJson(QJsonDocument::Compact) + '\n';
    qCDebug(dcRemoteProxyClientJsonRpcTraffic()) << "Sending" << qUtf8Printable(data);
    m_connection->sendData(data);
}

void JsonRpcClient::onConnectedChanged(bool connected)
{
    Q_UNUSED(connected)
}

void JsonRpcClient::processData(const QByteArray &data)
{
    qCDebug(dcRemoteProxyClientJsonRpcTraffic()) << "Received data:" << qUtf8Printable(data);

    QJsonParseError error;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(data, &error);
    if (error.error != QJsonParseError::NoError) {
        qCWarning(dcRemoteProxyClientJsonRpc()) << "Invalid JSON data recived" << error.errorString();
        return;
    }

    QVariantMap dataMap = jsonDoc.toVariant().toMap();

    // check if this is a reply to a request
    int commandId = dataMap.value("id").toInt();
    JsonReply *reply = m_replies.take(commandId);
    if (reply) {
        qCDebug(dcRemoteProxyClientJsonRpcTraffic()) << QString("Got response for %1.%2: %3").arg(reply->nameSpace(), reply->method(), QString::fromUtf8(jsonDoc.toJson(QJsonDocument::Indented)));

        if (dataMap.value("status").toString() == "error") {
            qCWarning(dcRemoteProxyClientJsonRpc()) << "Api error happend" << dataMap.value("error").toString();
        }

        reply->setResponse(dataMap);
        reply->finished();
        return;
    }

    // check if this is a notification
    if (dataMap.contains("notification")) {
        QStringList notification = dataMap.value("notification").toString().split(".");
        QString nameSpace = notification.first();
        QString notificationName = notification.last();
        QVariantMap notificationParams = dataMap.value("params").toMap();

        qCDebug(dcRemoteProxyClientJsonRpc()) << "Notification received" << nameSpace << notificationName;

        if (nameSpace == "ProxyServer" && notificationName == "TunnelEstablished") {
            QString clientName = notificationParams.value("name").toString();
            QString clientUuid = notificationParams.value("uuid").toString();
            emit tunnelEstablished(clientName, clientUuid);
        }
    }
}

}
