/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
*  Copyright 2013 - 2020, nymea GmbH
*  Contact: contact@nymea.io
*
*  This file is part of nymea.
*  This project including source code and documentation is protected by copyright law, and
*  remains the property of nymea GmbH. All rights, including reproduction, publication,
*  editing and translation, are reserved. The use of this project is subject to the terms of a
*  license agreement to be concluded with nymea GmbH in accordance with the terms
*  of use of nymea GmbH, available under https://nymea.io/license
*
*  GNU General Public License Usage
*  Alternatively, this project may be redistributed and/or modified under
*  the terms of the GNU General Public License as published by the Free Software Foundation,
*  GNU version 3. this project is distributed in the hope that it will be useful, but WITHOUT ANY
*  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
*  PURPOSE. See the GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License along with this project.
*  If not, see <https://www.gnu.org/licenses/>.
*
*  For any further details and any questions please contact us under contact@nymea.io
*  or see our FAQ/Licensing Information on https://nymea.io/license/faq
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "engine.h"
#include "jsonrpcserver.h"
#include "loggingcategories.h"
#include "jsonrpc/jsontypes.h"
#include "transportclient.h"
#include "../common/slipdataprocessor.h"
#include "../version.h"

#include <QJsonDocument>
#include <QJsonParseError>
#include <QCoreApplication>

namespace remoteproxy {

JsonRpcServer::JsonRpcServer(QObject *parent) :
    JsonHandler(parent)
{
    // Methods
    QVariantMap params; QVariantMap returns;

    params.clear(); returns.clear();
    setDescription("Hello", "Once connected to this server, a client can get information about the server by saying Hello. "
                            "The response informs the client about this proxy server.");
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

    // Notifications
    params.clear(); returns.clear();
    setDescription("TunnelEstablished", "Emitted whenever the tunnel has been established successfully. "
                   "This is the last message from the remote proxy server! Any following data will be from "
                   "the other tunnel client until the connection will be closed. The parameter contain some information "
                   "about the other tunnel client.");
    params.insert("uuid", JsonTypes::basicTypeToString(JsonTypes::String));
    params.insert("name", JsonTypes::basicTypeToString(JsonTypes::String));
    setParams("TunnelEstablished", params);

}

JsonRpcServer::~JsonRpcServer()
{
    qCDebug(dcJsonRpc()) << "Shutting down Json RPC server";
    foreach (JsonHandler *handler, m_handlers.values()) {
        unregisterHandler(handler);
    }
}

QString JsonRpcServer::name() const
{
    return "RemoteProxy";
}

JsonReply *JsonRpcServer::Hello(const QVariantMap &params, TransportClient *transportClient) const
{
    Q_UNUSED(params)
    Q_UNUSED(transportClient)

    QVariantMap data;
    data.insert("server", SERVER_NAME_STRING);
    data.insert("name", Engine::instance()->serverName());
    data.insert("version", SERVER_VERSION_STRING);
    data.insert("apiVersion", API_VERSION_STRING);

    return createReply("Hello", data);
}

JsonReply *JsonRpcServer::Introspect(const QVariantMap &params, TransportClient *transportClient) const
{
    Q_UNUSED(params)
    Q_UNUSED(transportClient)

    qCDebug(dcJsonRpc()) << "Introspect called.";

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

    return createReply("Introspect", data);
}

void JsonRpcServer::sendResponse(TransportClient *client, int commandId, const QVariantMap &params)
{
    QVariantMap response;
    response.insert("id", commandId);
    response.insert("status", "success");
    response.insert("params", params);

    QByteArray data = QJsonDocument::fromVariant(response).toJson(QJsonDocument::Compact);
    qCDebug(dcJsonRpcTraffic()) << "Sending data:" << data;
    if (client->slipEnabled()) {
        SlipDataProcessor::Frame frame;
        frame.socketAddress = 0x0000;
        frame.data = data + '\n';
        client->sendData(SlipDataProcessor::serializeData(SlipDataProcessor::buildFrame(frame)));
    } else {
        client->sendData(data + '\n');
    }
}

void JsonRpcServer::sendErrorResponse(TransportClient *client, int commandId, const QString &error)
{
    QVariantMap errorResponse;
    errorResponse.insert("id", commandId);
    errorResponse.insert("status", "error");
    errorResponse.insert("error", error);

    QByteArray data = QJsonDocument::fromVariant(errorResponse).toJson(QJsonDocument::Compact);
    qCDebug(dcJsonRpcTraffic()) << "Sending data:" << data;
    if (client->slipEnabled()) {
        SlipDataProcessor::Frame frame;
        frame.socketAddress = 0x0000;
        frame.data = data + '\n';
        client->sendData(SlipDataProcessor::serializeData(SlipDataProcessor::buildFrame(frame)));
    } else {
        client->sendData(data + '\n');
    }
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
    qCDebug(dcJsonRpc()) << "Register handler" << handler->name();
    m_handlers.insert(handler->name(), handler);
}

void JsonRpcServer::unregisterHandler(JsonHandler *handler)
{
    qCDebug(dcJsonRpc()) << "Unregister handler" << handler->name();
    m_handlers.remove(handler->name());
}

uint JsonRpcServer::registeredClientCount() const
{
    return m_clients.count();
}

void JsonRpcServer::processDataPacket(TransportClient *transportClient, const QByteArray &data)
{
    QJsonParseError error;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(data, &error);
    if(error.error != QJsonParseError::NoError) {
        qCWarning(dcJsonRpc) << "Failed to parse JSON data" << data << ":" << error.errorString();
        sendErrorResponse(transportClient, -1, QString("Failed to parse JSON data: %1").arg(error.errorString()));
        transportClient->killConnection("Invalid JSON data received.");
        return;
    }

    QVariantMap message = jsonDoc.toVariant().toMap();

    bool success = false;
    int commandId = message.value("id").toInt(&success);
    if (!success) {
        qCWarning(dcJsonRpc()) << "Error parsing command. Missing \"id\":" << message;
        sendErrorResponse(transportClient, -1, "Error parsing command. Missing 'id'");
        transportClient->killConnection("The id property is missing in the request.");
        return;
    }

    QStringList commandList = message.value("method").toString().split('.');
    if (commandList.count() != 2) {
        qCWarning(dcJsonRpc) << "Error parsing method.\nGot:" << message.value("method").toString() << "\nExpected: \"Namespace.method\"";
        sendErrorResponse(transportClient, commandId, QString("Error parsing method. Got: '%1'', Expected: 'Namespace.method'").arg(message.value("method").toString()));
        transportClient->killConnection("Invalid method passed.");
        return;
    }

    QString targetNamespace = commandList.first();
    QString method = commandList.last();

    JsonHandler *handler = m_handlers.value(targetNamespace);
    if (!handler) {
        sendErrorResponse(transportClient, commandId, "No such namespace");
        transportClient->killConnection("No such namespace.");
        return;
    }

    if (!handler->hasMethod(method)) {
        sendErrorResponse(transportClient, commandId, "No such method");
        transportClient->killConnection("No such method.");
        return;
    }

    QVariantMap params = message.value("params").toMap();
    QPair<bool, QString> validationResult = handler->validateParams(method, params);
    if (!validationResult.first) {
        sendErrorResponse(transportClient, commandId,  "Invalid params: " + validationResult.second);
        transportClient->killConnection("Invalid params passed.");
        return;
    }

    JsonReply *reply;
    QMetaObject::invokeMethod(handler, method.toLatin1().data(), Q_RETURN_ARG(JsonReply*, reply), Q_ARG(QVariantMap, params), Q_ARG(TransportClient*, transportClient));
    if (reply->type() == JsonReply::TypeAsync) {
        m_asyncReplies.insert(reply, transportClient);
        reply->setClientId(transportClient->clientId());
        reply->setCommandId(commandId);

        connect(reply, &JsonReply::finished, this, &JsonRpcServer::asyncReplyFinished);
        reply->startWait();
    } else {
        Q_ASSERT_X((targetNamespace == "RemoteProxy" && method == "Introspect") || handler->validateReturns(method, reply->data()).first
                   ,"validating return value", formatAssertion(targetNamespace, method, handler, reply->data()).toLatin1().data());

        reply->setClientId(transportClient->clientId());
        reply->setCommandId(commandId);
        sendResponse(transportClient, commandId, reply->data());
        reply->deleteLater();

        // If the server decided to kill the connection after the response, do it now
        if (transportClient->killConnectionRequested()) {
            transportClient->killConnection(transportClient->killConnectionReason());
            return;
        }

        // Enable SLIP from now on since requested
        if (transportClient->slipAfterResponseEnabled()) {
            transportClient->setSlipEnabled(true);
        }
    }
}

void JsonRpcServer::asyncReplyFinished()
{
    JsonReply *reply = static_cast<JsonReply *>(sender());
    reply->deleteLater();

    TransportClient *transportClient = m_asyncReplies.take(reply);
    qCDebug(dcJsonRpc()) << "Async reply finished" << reply->handler()->name() << reply->method() << reply->clientId().toString();

    if (!transportClient) {
        qCWarning(dcJsonRpc()) << "Got an async reply but the client does not exist any more.";
        return;
    }

    if (!reply->timedOut()) {
        Q_ASSERT_X(reply->handler()->validateReturns(reply->method(), reply->data()).first
                   ,"validating return value", formatAssertion(reply->handler()->name(),
                                                               reply->method(), reply->handler(), reply->data()).toLatin1().data());

        QPair<bool, QString> returnValidation = reply->handler()->validateReturns(reply->method(), reply->data());
        if (!returnValidation.first) {
            qCWarning(dcJsonRpc()) << "Return value validation failed. This should never happen. Please check the source code.";
        }

        if (!reply->success()) {
            // Disconnect this client since the request was not successfully
            transportClient->killConnectionAfterResponse("API call was not successfully.");
        }

        sendResponse(transportClient, reply->commandId(), reply->data());

    } else {
        qCWarning(dcJsonRpc()) << "The reply timeouted.";
        transportClient->killConnectionAfterResponse("API call timeouted.");
        sendErrorResponse(transportClient, reply->commandId(), "Command timed out");
        // Disconnect this client since he requested something that created a timeout

    }


    // If the server decided to kill the connection after the response, do it now
    if (transportClient->killConnectionRequested()) {
        transportClient->killConnection(transportClient->killConnectionReason());
    }
}

void JsonRpcServer::registerClient(TransportClient *transportClient)
{
    qCDebug(dcJsonRpc()) << "Register client" << transportClient;
    if (m_clients.contains(transportClient)) {
        qCWarning(dcJsonRpc()) << "Client already registered" << transportClient;
        return;
    }
    m_clients.append(transportClient);
}

void JsonRpcServer::unregisterClient(TransportClient *transportClient)
{
    qCDebug(dcJsonRpc()) << "Unregister client" << transportClient;
    if (!m_clients.contains(transportClient)) {
        qCWarning(dcJsonRpc()) << "Client was not registered" << transportClient;
        return;
    }

    m_clients.removeAll(transportClient);

    if (m_asyncReplies.values().contains(transportClient)) {
        qCWarning(dcJsonRpc()) << "Client was still waiting for a reply. Clean up reply";
        JsonReply *reply = m_asyncReplies.key(transportClient);
        m_asyncReplies.remove(reply);
        // Note: the reply will be deleted in the finished slot
    }
}

void JsonRpcServer::processData(TransportClient *transportClient, const QByteArray &data)
{
    if (!m_clients.contains(transportClient))
        return;

    qCDebug(dcJsonRpcTraffic()) << "Incoming data from" << transportClient << ": " << qUtf8Printable(data);

    // Handle packet fragmentation
    QList<QByteArray> packets = transportClient->processData(data);

    // Make sure the buffer size is in range
    if (transportClient->bufferSize() > 1024 * 10) {
        qCWarning(dcJsonRpc()) << "Data buffer size violation from" << transportClient;
        transportClient->killConnection("Data buffer size violation.");
        return;
    }

    foreach (const QByteArray &packet, packets) {
        processDataPacket(transportClient, packet);
    }
}

void JsonRpcServer::sendNotification(const QString &nameSpace, const QString &method, const QVariantMap &params, TransportClient *transportClient)
{
    QVariantMap notification;
    notification.insert("id", m_notificationId++);
    notification.insert("notification", nameSpace + "." + method);
    notification.insert("params", params);

    QByteArray data = QJsonDocument::fromVariant(notification).toJson(QJsonDocument::Compact);
    if (transportClient->slipEnabled()) {
        SlipDataProcessor::Frame frame;
        frame.socketAddress = 0x0000;
        frame.data = data + '\n';
        qCDebug(dcJsonRpcTraffic()) << "Sending notification frame:" <<frame.socketAddress << qUtf8Printable(frame.data);
        transportClient->sendData(SlipDataProcessor::serializeData(SlipDataProcessor::buildFrame(frame)));
    } else {
        qCDebug(dcJsonRpcTraffic()) << "Sending notification:" << data;
        transportClient->sendData(data + '\n');
    }
}

}
