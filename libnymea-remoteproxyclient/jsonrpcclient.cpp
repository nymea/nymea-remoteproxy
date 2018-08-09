#include "jsonrpcclient.h"
#include "proxyconnection.h"

#include <QJsonDocument>

Q_LOGGING_CATEGORY(dcRemoteProxyClientJsonRpc, "RemoteProxyClientJsonRpc")
Q_LOGGING_CATEGORY(dcRemoteProxyClientJsonRpcTraffic, "RemoteProxyClientJsonRpcTraffic")

namespace remoteproxyclient {

JsonRpcClient::JsonRpcClient(QObject *parent) :
    QObject(parent)
{

}

JsonReply *JsonRpcClient::callHello()
{

    JsonReply *reply = new JsonReply(m_commandId, "RemoteProxy", "Hello", QVariantMap(), this);

    return reply;
}

void JsonRpcClient::sendRequest(const QVariantMap &request)
{
    m_connection->sendData(QJsonDocument::fromVariant(request).toJson(QJsonDocument::Compact) + '\n');
}

void JsonRpcClient::onConnectedChanged(bool connected)
{
    if (!connected) {
        m_serverName = QString();
        m_proxyServerName = QString();
        m_proxyServerVersion = QString();
        m_proxyApiVersion = QString();
    }
}

void JsonRpcClient::processData(const QByteArray &data)
{
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
        reply->deleteLater();

        qCDebug(dcRemoteProxyClientJsonRpcTraffic()) << QString("Got response for %1.%2: %3").arg(reply->nameSpace(),
                                                                                            reply->method(),
                                                                                            QString::fromUtf8(jsonDoc.toJson(QJsonDocument::Indented)));

        if (dataMap.value("status").toString() == "error") {
            qCWarning(dcRemoteProxyClientJsonRpc()) << "Api error happend" << dataMap.value("error").toString();
            return;
        }

        reply->setResponse(dataMap);
        reply->finished();
        return;
    }

    // check if this is a notification
    if (dataMap.contains("notification")) {
        QStringList notification = dataMap.value("notification").toString().split(".");
        QString nameSpace = notification.first();

        qCDebug(dcRemoteProxyClientJsonRpc()) << "Notification received" << nameSpace << notification;
    }

}

}
