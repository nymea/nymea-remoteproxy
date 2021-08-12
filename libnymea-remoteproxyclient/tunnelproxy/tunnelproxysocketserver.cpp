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
#include "../../common/slipdataprocessor.h"

Q_LOGGING_CATEGORY(dcTunnelProxySocketServer, "TunnelProxySocketServer")
Q_LOGGING_CATEGORY(dcTunnelProxySocketServerTraffic, "TunnelProxySocketServerTraffic")

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

TunnelProxySocketServer::Error TunnelProxySocketServer::serverError() const
{
    return m_serverError;
}

void TunnelProxySocketServer::ignoreSslErrors()
{
    m_connection->ignoreSslErrors();
}

void TunnelProxySocketServer::ignoreSslErrors(const QList<QSslError> &errors)
{
    m_connection->ignoreSslErrors(errors);
}

QUrl TunnelProxySocketServer::serverUrl() const
{
    return m_serverUrl;
}

QString TunnelProxySocketServer::remoteProxyServer() const
{
    return m_remoteProxyServer;
}

QString TunnelProxySocketServer::remoteProxyServerName() const
{
    return m_remoteProxyServerName;
}

QString TunnelProxySocketServer::remoteProxyServerVersion() const
{
    return m_remoteProxyServerVersion;
}

QString TunnelProxySocketServer::remoteProxyApiVersion() const
{
    return m_remoteProxyApiVersion;
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
    connect(m_jsonClient, &JsonRpcClient::tunnelProxyClientConnected, this, &TunnelProxySocketServer::onTunnelProxyClientConnected);
    connect(m_jsonClient, &JsonRpcClient::tunnelProxyClientDisonnected, this, &TunnelProxySocketServer::onTunnelProxyClientDisconnected);

    qCDebug(dcTunnelProxySocketServer()) << "Connecting to" << m_serverUrl.toString();
    m_connection->connectServer(m_serverUrl);
}

void TunnelProxySocketServer::stopServer()
{
    if (m_connection) {
        qCDebug(dcTunnelProxySocketServer()) << "Disconnecting from" << m_connection->serverUrl().toString();
        m_connection->disconnectServer();
    }
}

void TunnelProxySocketServer::onConnectionChanged(bool connected)
{
    if (connected) {
        qCDebug(dcTunnelProxySocketServer()) << "Connected to remote proxy server.";
        setState(StateConnected);
        m_serverError = TunnelProxySocketServer::ErrorNoError;

        setState(StateInitializing);
        JsonReply *reply = m_jsonClient->callHello();
        connect(reply, &JsonReply::finished, this, &TunnelProxySocketServer::onHelloFinished);
    } else {
        qCDebug(dcTunnelProxySocketServer()) << "Disconnected from remote proxy server.";
        setState(StateDisconnected);
        setServerError(ErrorNotConnected);
        cleanUp();
    }
}

void TunnelProxySocketServer::onConnectionDataAvailable(const QByteArray &data)
{
    qCDebug(dcTunnelProxySocketServerTraffic()) << "Data received" << qUtf8Printable(data);

    if (m_state != StateRunning) {
        m_jsonClient->processData(data);
        m_dataBuffer.clear();
        return;
    }

    // Parse SLIP frame
    for (int i = 0; i < data.length(); i++) {
        quint8 byte = static_cast<quint8>(data.at(i));
        if (byte == SlipDataProcessor::ProtocolByteEnd) {
            // If there is no data...continue since it might be a starting END byte
            if (m_dataBuffer.isEmpty())
                continue;

            QByteArray frameData = SlipDataProcessor::deserializeData(m_dataBuffer);
            if (frameData.isNull()) {
                qCWarning(dcTunnelProxySocketServerTraffic()) << "Received inconsistant SLIP encoded message. Ignoring data...";
            } else {
                SlipDataProcessor::Frame frame = SlipDataProcessor::parseFrame(frameData);
                qCDebug(dcTunnelProxySocketServerTraffic()) << "Frame received" << frame.socketAddress << qUtf8Printable(frame.data);
                if (frame.socketAddress == 0x0000) {
                    m_jsonClient->processData(frame.data);
                } else {
                    // Find the socket and emit the data received signal
                    TunnelProxySocket *tunnlProxySocket = m_tunnelProxySockets.value(frame.socketAddress);
                    if (!tunnlProxySocket) {
                        qCWarning(dcTunnelProxySocketServer()) << "Received data from unknown tunnel proxy client with address" << frame.socketAddress << "...ignoring the data";
                    } else {
                        emit tunnlProxySocket->dataReceived(frame.data);
                    }
                }
            }

            m_dataBuffer.clear();
        } else {
            m_dataBuffer.append(data.at(i));
        }
    }
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
        setServerError(ErrorConnectionError);
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
        setServerError(ErrorConnectionError);
        return;
    }

    QVariantMap responseParams = response.value("params").toMap();
    if (responseParams.value("tunnelProxyError").toString() != "TunnelProxyErrorNoError") {
        qCWarning(dcTunnelProxySocketServer()) << "Failed to register as tunnel server on the remote proxy server:" << responseParams.value("tunnelProxyError").toString();
        m_connection->disconnectServer();
        setServerError(ErrorConnectionError);
        return;
    }

    qCDebug(dcTunnelProxySocketServer()) << "Registered successfully as tunnel server on the remote proxy server.";
    setState(StateRunning);
    m_serverError = ErrorNoError;
}

void TunnelProxySocketServer::onTunnelProxyClientConnected(const QString &clientName, const QUuid &clientUuid, const QString &clientPeerAddress, quint16 socketAddress)
{
    TunnelProxySocket *tunnelProxySocket = new TunnelProxySocket(m_connection, this, clientName, clientUuid, QHostAddress(clientPeerAddress), socketAddress, this);
    qCDebug(dcTunnelProxySocketServer()) << "--> New client connected" << tunnelProxySocket;
    m_tunnelProxySockets.insert(socketAddress, tunnelProxySocket);
    emit clientConnected(tunnelProxySocket);
}

void TunnelProxySocketServer::onTunnelProxyClientDisconnected(quint16 socketAddress)
{
    TunnelProxySocket *tunnelProxySocket = m_tunnelProxySockets.take(socketAddress);
    if (!tunnelProxySocket) {
        qCWarning(dcTunnelProxySocketServer()) << "Unknown client disconnected" << socketAddress << ". Ignoring the event.";
        return;
    }

    qCDebug(dcTunnelProxySocketServer()) << "--> Client disconnected" << tunnelProxySocket;
    tunnelProxySocket->setDisconnected();
    emit clientDisconnected(tunnelProxySocket);
    tunnelProxySocket->deleteLater();
}

void TunnelProxySocketServer::requestSocketDisconnect(quint16 socketAddress)
{
    TunnelProxySocket *socket = m_tunnelProxySockets.value(socketAddress);
    qCDebug(dcTunnelProxySocketServer()) << "Request to disconnect socket" << socket;

    JsonReply *reply = m_jsonClient->callDisconnectClient(socketAddress);
    connect(reply, &JsonReply::finished, this, [=](){
       reply->deleteLater();
       // TODO: handle errors
       qCDebug(dcTunnelProxySocketServer()) << "Request to disconnect client finished" << reply->response();
    });
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

void TunnelProxySocketServer::setServerError(Error error)
{
    if (m_serverError == error)
        return;

    qCDebug(dcTunnelProxySocketServer()) << "Server error occured" << error;
    m_serverError = error;
    emit serverErrorOccured(m_serverError);
}

void TunnelProxySocketServer::cleanUp()
{
    foreach (quint16 socketAddress, m_tunnelProxySockets.keys()) {
        onTunnelProxyClientDisconnected(socketAddress);
    }

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
