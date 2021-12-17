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

#include "tunnelproxyremoteconnection.h"
#include "proxyconnection.h"
#include "tcpsocketconnection.h"
#include "websocketconnection.h"
#include "proxyjsonrpcclient.h"

Q_LOGGING_CATEGORY(dcTunnelProxyRemoteConnection, "TunnelProxyRemoteConnection")

namespace remoteproxyclient {

TunnelProxyRemoteConnection::TunnelProxyRemoteConnection(const QUuid &clientUuid, const QString &clientName, QObject *parent) :
    QObject(parent),
    m_clientUuid(clientUuid),
    m_clientName(clientName)
{

}

TunnelProxyRemoteConnection::TunnelProxyRemoteConnection(const QUuid &clientUuid, const QString &clientName, ConnectionType connectionType, QObject *parent) :
    QObject(parent),
    m_clientUuid(clientUuid),
    m_clientName(clientName),
    m_connectionType(connectionType)
{

}

TunnelProxyRemoteConnection::~TunnelProxyRemoteConnection()
{

}

bool TunnelProxyRemoteConnection::remoteConnected() const
{
    return m_state == StateRemoteConnected;
}

TunnelProxyRemoteConnection::State TunnelProxyRemoteConnection::state() const
{
    return m_state;
}

QAbstractSocket::SocketError TunnelProxyRemoteConnection::error() const
{
    return m_error;
}

void TunnelProxyRemoteConnection::ignoreSslErrors()
{
    m_connection->ignoreSslErrors();
}

void TunnelProxyRemoteConnection::ignoreSslErrors(const QList<QSslError> &errors)
{
    m_connection->ignoreSslErrors(errors);
}

QUrl TunnelProxyRemoteConnection::serverUrl() const
{
    return m_serverUrl;
}

QString TunnelProxyRemoteConnection::remoteProxyServer() const
{
    return m_remoteProxyServer;
}

QString TunnelProxyRemoteConnection::remoteProxyServerName() const
{
    return m_remoteProxyServerName;
}

QString TunnelProxyRemoteConnection::remoteProxyServerVersion() const
{
    return m_remoteProxyServerVersion;
}

QString TunnelProxyRemoteConnection::remoteProxyApiVersion() const
{
    return m_remoteProxyApiVersion;
}

bool TunnelProxyRemoteConnection::connectServer(const QUrl &url, const QUuid &serverUuid)
{
    m_serverUrl = url;
    m_serverUuid = serverUuid;
    m_error = QAbstractSocket::UnknownSocketError;

    cleanUp();

    switch (m_connectionType) {
    case ConnectionTypeWebSocket:
        qCDebug(dcTunnelProxyRemoteConnection()) << "Creating a web socket connection to" << url.toString();
        m_connection = qobject_cast<ProxyConnection *>(new WebSocketConnection(this));
        break;
    case ConnectionTypeTcpSocket:
        qCDebug(dcTunnelProxyRemoteConnection()) << "Creating a TCP socket connection to" << url.toString();
        m_connection = qobject_cast<ProxyConnection *>(new TcpSocketConnection(this));
        break;
    }

    connect(m_connection, &ProxyConnection::connectedChanged, this, &TunnelProxyRemoteConnection::onConnectionChanged);
    connect(m_connection, &ProxyConnection::dataReceived, this, &TunnelProxyRemoteConnection::onConnectionDataAvailable);
    connect(m_connection, &ProxyConnection::errorOccurred, this, &TunnelProxyRemoteConnection::onConnectionSocketError);
    connect(m_connection, &ProxyConnection::stateChanged, this, &TunnelProxyRemoteConnection::onConnectionStateChanged);
    connect(m_connection, &ProxyConnection::sslErrors, this, &TunnelProxyRemoteConnection::sslErrors);

    m_jsonClient = new JsonRpcClient(m_connection, this);

    qCDebug(dcTunnelProxyRemoteConnection()) << "Connecting to" << m_serverUrl.toString();
    m_connection->connectServer(m_serverUrl);

    return true;
}

void TunnelProxyRemoteConnection::disconnectServer()
{
    if (m_connection) {
        qCDebug(dcTunnelProxyRemoteConnection()) << "Disconnecting from" << m_connection->serverUrl().toString();
        m_connection->disconnectServer();
    }
}

bool TunnelProxyRemoteConnection::sendData(const QByteArray &data)
{
    if (!remoteConnected()) {
        qCWarning(dcTunnelProxyRemoteConnection()) << "Could not send data. Not connected.";
        return false;
    }

    m_connection->sendData(data);
    return true;
}

void TunnelProxyRemoteConnection::onConnectionChanged(bool connected)
{
    if (connected) {
        qCDebug(dcTunnelProxyRemoteConnection()) << "Connected to remote proxy server.";
        setState(StateConnected);
        setState(StateInitializing);
        JsonReply *reply = m_jsonClient->callHello();
        connect(reply, &JsonReply::finished, this, &TunnelProxyRemoteConnection::onHelloFinished);
    } else {
        qCDebug(dcTunnelProxyRemoteConnection()) << "Disconnected from remote proxy server.";
        setState(StateDisconnected);
        cleanUp();
    }
}

void TunnelProxyRemoteConnection::onConnectionDataAvailable(const QByteArray &data)
{
    if (m_state != StateRemoteConnected) {
        m_jsonClient->processData(data);
        return;
    }

    emit dataReady(data);
}

void TunnelProxyRemoteConnection::onConnectionSocketError(QAbstractSocket::SocketError error)
{
    setError(error);
}

void TunnelProxyRemoteConnection::onConnectionStateChanged(QAbstractSocket::SocketState state)
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

void TunnelProxyRemoteConnection::onHelloFinished()
{
    JsonReply *reply = static_cast<JsonReply *>(sender());
    reply->deleteLater();

    QVariantMap response = reply->response();
    qCDebug(dcTunnelProxyRemoteConnection()) << "Hello response ready" << reply->commandId() << response;

    if (response.value("status").toString() != "success") {
        qCWarning(dcTunnelProxyRemoteConnection()) << "Could not get initial information from proxy server.";
        m_connection->disconnectServer();
        return;
    }

    QVariantMap responseParams = response.value("params").toMap();
    m_remoteProxyServer = responseParams.value("server").toString();
    m_remoteProxyServerName = responseParams.value("name").toString();
    m_remoteProxyServerVersion = responseParams.value("version").toString();
    m_remoteProxyApiVersion = responseParams.value("apiVersion").toString();

    setState(StateRegister);

    JsonReply *registerReply = m_jsonClient->callRegisterClient(m_clientUuid, m_clientName, m_serverUuid);
    connect(registerReply, &JsonReply::finished, this, &TunnelProxyRemoteConnection::onClientRegistrationFinished);
}

void TunnelProxyRemoteConnection::onClientRegistrationFinished()
{
    JsonReply *reply = static_cast<JsonReply *>(sender());
    reply->deleteLater();

    QVariantMap response = reply->response();
    qCDebug(dcTunnelProxyRemoteConnection()) << "RegisterClient response ready" << reply->commandId() << response;

    if (response.value("status").toString() != "success") {
        qCWarning(dcTunnelProxyRemoteConnection()) << "JSON RPC error. Failed to register as tunnel client on the remote proxy server.";
        m_connection->disconnectServer();
        return;
    }

    QVariantMap responseParams = response.value("params").toMap();
    if (responseParams.value("tunnelProxyError").toString() != "TunnelProxyErrorNoError") {
        qCWarning(dcTunnelProxyRemoteConnection()) << "Failed to register as tunnel client on the remote proxy server:" << responseParams.value("tunnelProxyError").toString();
        m_connection->disconnectServer();
        return;
    }

    qCDebug(dcTunnelProxyRemoteConnection()) << "Registered successfully as tunnel client on the remote proxy server.";
    setState(StateRemoteConnected);
}

void TunnelProxyRemoteConnection::setState(State state)
{
    if (m_state == state)
        return;

    m_state = state;
    qCDebug(dcTunnelProxyRemoteConnection()) << "State changed" << m_state;
    emit stateChanged(m_state);
    emit remoteConnectedChanged(m_state == StateRemoteConnected);
}


void TunnelProxyRemoteConnection::setError(QAbstractSocket::SocketError error)
{
    if (m_error == error)
        return;

    m_error = error;
    qCWarning(dcTunnelProxyRemoteConnection()) << "Error occurred" << m_error;
    emit errorOccurred(m_error);
}

void TunnelProxyRemoteConnection::cleanUp()
{
    if (m_jsonClient) {
        m_jsonClient->deleteLater();
        m_jsonClient = nullptr;
    }

    if (m_connection) {
        m_connection->deleteLater();
        m_connection = nullptr;
    }

    m_remoteProxyServer.clear();
    m_remoteProxyServerName.clear();
    m_remoteProxyServerVersion.clear();
    m_remoteProxyApiVersion.clear();

    setState(StateDisconnected);
}

}
