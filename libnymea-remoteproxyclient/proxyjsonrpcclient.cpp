/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright 2013 - 2023, nymea GmbH
* Contact: contact@nymea.io
*
* This file is part of nymea.
* This project including source code and documentation is protected by copyright law, and
* remains the property of nymea GmbH. All rights, including reproduction, publication,
* editing and translation, are reserved. The use of this project is subject to the terms of a
* license agreement to be concluded with nymea GmbH in accordance with the terms
* of use of nymea GmbH, available under https://nymea.io/license
*
* GNU Lesser General Public License Usage
* Alternatively, this project may be redistributed and/or modified under the terms of the GNU
* Lesser General Public License as published by the Free Software Foundation; version 3.
* this project is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
* without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
* See the GNU Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License along with this project.
* If not, see <https://www.gnu.org/licenses/>.
*
* For any further details and any questions please contact us under contact@nymea.io
* or see our FAQ/Licensing Information on https://nymea.io/license/faq
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "proxyjsonrpcclient.h"
#include "proxyconnection.h"
#include "../common/slipdataprocessor.h"

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

JsonReply *JsonRpcClient::callRegisterServer(const QUuid &serverUuid, const QString &serverName)
{
    QVariantMap params;
    params.insert("serverName", serverName);
    params.insert("serverUuid", serverUuid.toString());

    JsonReply *reply = new JsonReply(m_commandId, "TunnelProxy", "RegisterServer", params, this);
    qCDebug(dcRemoteProxyClientJsonRpc()) << "Calling" << QString("%1.%2").arg(reply->nameSpace()).arg(reply->method());
    sendRequest(reply->requestMap());
    m_replies.insert(m_commandId, reply);
    return reply;
}

JsonReply *JsonRpcClient::callRegisterClient(const QUuid &clientUuid, const QString &clientName, const QUuid &serverUuid)
{
    QVariantMap params;
    params.insert("clientUuid", clientUuid);
    params.insert("clientName", clientName);
    params.insert("serverUuid", serverUuid.toString());

    JsonReply *reply = new JsonReply(m_commandId, "TunnelProxy", "RegisterClient", params, this);
    qCDebug(dcRemoteProxyClientJsonRpc()) << "Calling" << QString("%1.%2").arg(reply->nameSpace()).arg(reply->method());
    sendRequest(reply->requestMap());
    m_replies.insert(m_commandId, reply);
    return reply;
}

JsonReply *JsonRpcClient::callDisconnectClient(quint16 socketAddress)
{
    QVariantMap params;
    params.insert("socketAddress", socketAddress);

    JsonReply *reply = new JsonReply(m_commandId, "TunnelProxy", "DisconnectClient", params, this);
    qCDebug(dcRemoteProxyClientJsonRpc()) << "Calling" << QString("%1.%2").arg(reply->nameSpace()).arg(reply->method());
    sendRequest(reply->requestMap(), true);
    m_replies.insert(m_commandId, reply);
    return reply;
}

JsonReply *JsonRpcClient::callPing(uint timestamp)
{
    QVariantMap params;
    params.insert("timestamp", timestamp);

    JsonReply *reply = new JsonReply(m_commandId, "TunnelProxy", "Ping", params, this);
    qCDebug(dcRemoteProxyClientJsonRpc()) << "Calling" << QString("%1.%2").arg(reply->nameSpace()).arg(reply->method());
    sendRequest(reply->requestMap(), true);
    m_replies.insert(m_commandId, reply);
    return reply;
}

void JsonRpcClient::sendRequest(const QVariantMap &request, bool slipEnabled)
{
    QByteArray data = QJsonDocument::fromVariant(request).toJson(QJsonDocument::Compact) + '\n';

    if (slipEnabled) {
        SlipDataProcessor::Frame frame;
        frame.socketAddress = 0x0000;
        frame.data = data;
        data = SlipDataProcessor::serializeData(SlipDataProcessor::buildFrame(frame));
    }

    qCDebug(dcRemoteProxyClientJsonRpcTraffic()) << "Sending" << qUtf8Printable(data);
    m_connection->sendData(data);
}

void JsonRpcClient::processDataPacket(const QByteArray &data)
{
    QJsonParseError error;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(data, &error);
    if (error.error != QJsonParseError::NoError) {
        qCWarning(dcRemoteProxyClientJsonRpc()) << "Invalid JSON data recived" << error.errorString();
        return;
    }

    QVariantMap dataMap = jsonDoc.toVariant().toMap();

    qCDebug(dcRemoteProxyClientJsonRpcTraffic()) << "Data received" << qUtf8Printable(data);

    // check if this is a reply to a request
    int commandId = dataMap.value("id").toInt();
    JsonReply *reply = m_replies.take(commandId);
    if (reply) {
        qCDebug(dcRemoteProxyClientJsonRpcTraffic()) << QString("Got response for %1.%2: %3").arg(reply->nameSpace(), reply->method(), QString::fromUtf8(jsonDoc.toJson(QJsonDocument::Indented)));

        if (dataMap.value("status").toString() == "error") {
            qCWarning(dcRemoteProxyClientJsonRpc()) << "Api error happend" << dataMap.value("error").toString();
            // FIMXME: handle json layer errors
        }

        reply->setResponse(dataMap);
        emit reply->finished();
        return;
    }

    // check if this is a notification
    if (dataMap.contains("notification")) {
        QStringList notification = dataMap.value("notification").toString().split(".");
        QString nameSpace = notification.first();
        QString notificationName = notification.last();
        QVariantMap notificationParams = dataMap.value("params").toMap();

        qCDebug(dcRemoteProxyClientJsonRpc()) << "Notification received" << nameSpace << notificationName;

        if (nameSpace == "RemoteProxy" && notificationName == "TunnelEstablished") {
            QString clientName = notificationParams.value("name").toString();
            QString clientUuid = notificationParams.value("uuid").toString();
            emit tunnelEstablished(clientName, clientUuid);
        } else if (nameSpace == "TunnelProxy" && notificationName == "ClientConnected") {
            QString clientName = notificationParams.value("clientName").toString();
            QUuid clientUuid = notificationParams.value("clientUuid").toUuid();
            QString clientPeerAddress = notificationParams.value("clientPeerAddress").toString();
            quint16 socketAddress = static_cast<quint16>(notificationParams.value("socketAddress").toUInt());
            emit tunnelProxyClientConnected(clientName, clientUuid, clientPeerAddress, socketAddress);
        } else if (nameSpace == "TunnelProxy" && notificationName == "ClientDisconnected") {
            quint16 socketAddress = static_cast<quint16>(notificationParams.value("socketAddress").toUInt());
            emit tunnelProxyClientDisonnected(socketAddress);
        }
    }
}

void JsonRpcClient::processData(const QByteArray &data)
{
    qCDebug(dcRemoteProxyClientJsonRpcTraffic()) << "Received data:" << data;

    // Handle packet fragmentation
    m_dataBuffer.append(data);
    int splitIndex = m_dataBuffer.indexOf("}\n{");
    while (splitIndex > -1) {
        processDataPacket(m_dataBuffer.left(splitIndex + 1));
        m_dataBuffer = m_dataBuffer.right(m_dataBuffer.length() - splitIndex - 2);
        splitIndex = m_dataBuffer.indexOf("}\n{");
    }
    if (m_dataBuffer.endsWith("}\n") || m_dataBuffer.endsWith("}")) {
        processDataPacket(m_dataBuffer);
        m_dataBuffer.clear();
    }
}

}
