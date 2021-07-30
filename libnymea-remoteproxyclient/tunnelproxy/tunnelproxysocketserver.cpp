/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright 2013 - 2021, nymea GmbH
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

#include "tunnelproxysocketserver.h"
#include "proxyconnection.h"
#include "tcpsocketconnection.h"
#include "websocketconnection.h"
#include "proxyjsonrpcclient.h"

Q_LOGGING_CATEGORY(dcTunnelProxySocketServer, "dcTunnelProxySocketServer")

namespace remoteproxyclient {

TunnelProxySocketServer::TunnelProxySocketServer(const QUuid &serverUuid, const QString &serverName, QObject *parent) :
    QObject(parent),
    m_serverUuid(serverUuid),
    m_serverName(serverName)
{

}

TunnelProxySocketServer::TunnelProxySocketServer(const QUuid &serverUuid, const QString &serverName, ConnectionType connectionType, QObject *parent) :
    QObject(parent),
    m_serverUuid(serverUuid),
    m_serverName(serverName),
    m_connectionType(connectionType)
{

}

TunnelProxySocketServer::~TunnelProxySocketServer()
{

}

bool TunnelProxySocketServer::running() const
{
    return m_running;
}

QAbstractSocket::SocketError TunnelProxySocketServer::error() const
{
    return m_error;
}

void TunnelProxySocketServer::ignoreSslErrors()
{
    m_connection->ignoreSslErrors();

}

void TunnelProxySocketServer::ignoreSslErrors(const QList<QSslError> &errors)
{
    m_connection->ignoreSslErrors(errors);
}

void TunnelProxySocketServer::startServer(const QUrl &serverUrl)
{
    m_serverUrl = serverUrl;
    m_error = QAbstractSocket::UnknownSocketError;

    cleanUp();

    switch (m_connectionType) {
    case ConnectionTypeWebSocket:
        qCDebug(dcTunnelProxySocketServer()) << "Creating a web socket connection to" << m_serverUrl.toString();
        m_connection = qobject_cast<ProxyConnection *>(new WebSocketConnection(this));
        break;
    case ConnectionTypeTcpSocket:
        qCDebug(dcTunnelProxySocketServer()) << "Creating a TCP socket connection to" << m_serverUrl.toString();
        m_connection = qobject_cast<ProxyConnection *>(new TcpSocketConnection(this));
        break;
    }

    connect(m_connection, &ProxyConnection::connectedChanged, this, &TunnelProxySocketServer::onConnectionChanged);
    connect(m_connection, &ProxyConnection::dataReceived, this, &TunnelProxySocketServer::onConnectionDataAvailable);
    connect(m_connection, &ProxyConnection::errorOccured, this, &TunnelProxySocketServer::onConnectionSocketError);
    connect(m_connection, &ProxyConnection::stateChanged, this, &TunnelProxySocketServer::onConnectionStateChanged);
    connect(m_connection, &ProxyConnection::sslErrors, this, &TunnelProxySocketServer::sslErrors);

    m_jsonClient = new JsonRpcClient(m_connection, this);
//    connect(m_jsonClient, &JsonRpcClient::tunnelEstablished, this, &RemoteProxyConnection::onTunnelEstablished);

    qCDebug(dcTunnelProxySocketServer()) << "Connecting to" << m_serverUrl.toString();
    m_connection->connectServer(m_serverUrl);
}

void TunnelProxySocketServer::stopServer()
{
    // Close the server connection and all related sockets
}

void TunnelProxySocketServer::onConnectionChanged(bool connected)
{
    if (connected) {
        qCDebug(dcTunnelProxySocketServer()) << "Connected to remote proxy server.";
        setState(StateConnected);

        setState(StateInitializing);
        JsonReply *reply = m_jsonClient->callHello();
        connect(reply, &JsonReply::finished, this, &TunnelProxySocketServer::onHelloFinished);
    } else {
        qCDebug(dcTunnelProxySocketServer()) << "Disconnected from remote proxy server.";
        setState(StateDisconnected);
        cleanUp();
    }
}

void TunnelProxySocketServer::onConnectionDataAvailable(const QByteArray &data)
{
    if (m_state != StateRunning) {
        m_jsonClient->processData(data);
        return;
    }

    // Parse SLIP
}

void TunnelProxySocketServer::onConnectionSocketError(QAbstractSocket::SocketError error)
{
    setError(error);
}

void TunnelProxySocketServer::onConnectionStateChanged(QAbstractSocket::SocketState state)
{
    // Process the actuall socket state
    switch (state) {
    case QAbstractSocket::UnconnectedState:
        setState(StateDisconnected);
        break;
    case QAbstractSocket::HostLookupState:
        setState(StateHostLookup);
        break;
    case QAbstractSocket::ConnectingState:
        setState(StateConnecting);
        break;
    case QAbstractSocket::ConnectedState:
        setState(StateConnected);
        break;
    case QAbstractSocket::ClosingState:
        setState(StateDiconnecting);
        break;
    case QAbstractSocket::BoundState:
    case QAbstractSocket::ListeningState:
        break;
    }
}

void TunnelProxySocketServer::onHelloFinished()
{
    JsonReply *reply = static_cast<JsonReply *>(sender());
    reply->deleteLater();

    QVariantMap response = reply->response();
    qCDebug(dcTunnelProxySocketServer()) << "Hello response ready" << reply->commandId() << response;

    if (response.value("status").toString() != "success") {
        qCWarning(dcTunnelProxySocketServer()) << "Could not get initial information from proxy server.";
        m_connection->disconnectServer();
        return;
    }

    QVariantMap responseParams = response.value("params").toMap();
    m_remoteProxyServer = responseParams.value("server").toString();
    m_remoteProxyServerName = responseParams.value("name").toString();
    m_remoteProxyServerVersion = responseParams.value("version").toString();
    m_remoteProxyApiVersion = responseParams.value("apiVersion").toString();

    setState(StateRegister);

    JsonReply *registerReply = m_jsonClient->callRegisterServer(m_serverUuid, m_serverName);
    connect(registerReply, &JsonReply::finished, this, &TunnelProxySocketServer::onServerRegistrationFinished);
}

void TunnelProxySocketServer::onServerRegistrationFinished()
{
    JsonReply *reply = static_cast<JsonReply *>(sender());
    reply->deleteLater();

    QVariantMap response = reply->response();
    qCDebug(dcTunnelProxySocketServer()) << "RegisterServer response ready" << reply->commandId() << response;

    if (response.value("status").toString() != "success") {
        qCWarning(dcTunnelProxySocketServer()) << "JSON RPC error. Failed to register as tunnel server on the remote proxy server.";
        m_connection->disconnectServer();
        return;
    }

    QVariantMap responseParams = response.value("params").toMap();
    if (responseParams.value("tunnelProxyError").toString() != "TunnelProxyErrorNoError") {
        qCWarning(dcTunnelProxySocketServer()) << "Failed to register as tunnel server on the remote proxy server:" << responseParams.value("tunnelProxyError").toString();
        m_connection->disconnectServer();
        return;
    }

    qCDebug(dcTunnelProxySocketServer()) << "Registered successfully as tunnel server on the remote proxy server.";
    setState(StateRunning);
}

void TunnelProxySocketServer::setState(State state)
{
    if (m_state == state)
        return;

    m_state = state;
    qCDebug(dcTunnelProxySocketServer()) << "State changed" << m_state;
    emit stateChanged(m_state);

    setRunning(m_state == StateRunning);
}

void TunnelProxySocketServer::setRunning(bool running)
{
    if (m_running == running)
        return;

    qCDebug(dcTunnelProxySocketServer()) << "The TunnelProxy server" << (running ? "is now up and running. Listening for remote clients" : "not running any more.");
    m_running = running;
    emit runningChanged(m_running);
}

void TunnelProxySocketServer::setError(QAbstractSocket::SocketError error)
{
    if (m_error == error)
        return;

    m_error = error;
    qCDebug(dcTunnelProxySocketServer()) << "Error occured" << m_error;
    emit errorOccured(m_error);
}

void TunnelProxySocketServer::cleanUp()
{
    // TODO: cleanup client connections


    if (m_jsonClient) {
        m_jsonClient->deleteLater();
        m_jsonClient = nullptr;
    }

    if (m_connection) {
        m_connection->deleteLater();
        m_connection = nullptr;
    }

//    m_serverName = QString();
//    m_serverUuid = QUuid();

    setState(StateDisconnected);
}

}
