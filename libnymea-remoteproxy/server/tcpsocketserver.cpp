// SPDX-License-Identifier: LGPL-3.0-or-later

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* nymea-remoteproxy
* Tunnel proxy server for the nymea remote access
*
* Copyright (C) 2013 - 2024, nymea GmbH
* Copyright (C) 2024 - 2025, chargebyte austria GmbH
*
* This file is part of nymea-remoteproxy.
*
* nymea-remoteproxy is free software: you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public License
* as published by the Free Software Foundation, either version 3
* of the License, or (at your option) any later version.
*
* nymea-remoteproxy is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with nymea-remoteproxy. If not, see <https://www.gnu.org/licenses/>.
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "tcpsocketserver.h"
#include "loggingcategories.h"

namespace remoteproxy {

TcpSocketServer::TcpSocketServer(bool sslEnabled, const QSslConfiguration &sslConfiguration, QObject *parent) :
    TransportInterface(parent),
    m_sslEnabled(sslEnabled),
    m_sslConfiguration(sslConfiguration)
{
    m_serverName = "TCP";
}

TcpSocketServer::~TcpSocketServer()
{
    TcpSocketServer::stopServer();
}

void TcpSocketServer::sendData(const QUuid &clientId, const QByteArray &data)
{
    QSslSocket *client = m_clientList.value(clientId);
    if (!client) {
        qCWarning(dcTcpSocketServer()) << "Client" << clientId << "unknown to this transport";
        return;
    }

    qCDebug(dcTcpSocketServerTraffic()) << "Send data to" << clientId.toString() << data;
    if (client->write(data) < 0) {
        qCWarning(dcTcpSocketServer()) << "Could not write data to client socket" << clientId.toString();
    }
}

void TcpSocketServer::killClientConnection(const QUuid &clientId, const QString &killReason)
{
    QSslSocket *client = m_clientList.value(clientId);
    if (!client) {
        qCWarning(dcTcpSocketServer()) << "Could not kill connection with id" << clientId.toString() << "with reason" << killReason << "because there is no socket with this id.";
        return;
    }

    qCDebug(dcTcpSocketServer()) << "Killing client connection" << clientId.toString() << "Reason:" << killReason;
    client->flush();
    client->close();
}

uint TcpSocketServer::connectionsCount() const
{
    return m_clientList.count();
}

bool TcpSocketServer::running() const
{
    if (!m_server)
        return false;

    return m_server->isListening();
}

bool TcpSocketServer::startServer()
{
    if (m_server) {
        m_server->close();
        delete m_server;
        m_server = nullptr;
    }

    qCDebug(dcTcpSocketServer()) << "Starting TCP server" << m_serverUrl.toString();
    m_server = new SslServer(m_sslEnabled, m_sslConfiguration, this);
    m_server->setMaxPendingConnections(100);
    if(!m_server->listen(QHostAddress(m_serverUrl.host()), static_cast<quint16>(m_serverUrl.port()))) {
        qCWarning(dcTcpSocketServer()) << "Tcp server error: can not listen on" << m_serverUrl.toString();
        delete m_server;
        m_server = nullptr;
        return false;
    }

    connect(m_server, &SslServer::socketConnected, this, &TcpSocketServer::onSocketConnected);
    connect(m_server, &SslServer::socketDisconnected, this, &TcpSocketServer::onSocketDisconnected);
    connect(m_server, &SslServer::dataAvailable, this, &TcpSocketServer::onDataAvailable);

    qCDebug(dcTcpSocketServer()) << "Server started successfully.";
    qCDebug(dcTcpSocketServer()) << m_server;
    return true;
}

bool TcpSocketServer::stopServer()
{
    qCDebug(dcTcpSocketServer()) << "Stopping server" << m_serverUrl.toString() << m_server;
    if (!m_server)
        return true;

    // Clean up client connections
    foreach (const QUuid &clientId, m_clientList.keys()) {
        killClientConnection(clientId, "Stop server");
    }

    m_server->close();
    m_server->deleteLater();
    m_server = nullptr;
    return true;
}


void TcpSocketServer::onDataAvailable(QSslSocket *client, const QByteArray &data)
{
    QUuid clientId = m_clientList.key(client);
    if (clientId.isNull()) {
        qCWarning(dcTcpSocketServer()) << "Socket sent data but the uuid is null." << client << client->peerAddress().toString() << "Ignoring data...";
        return;
    }
    emit dataAvailable(clientId, data);
}

void TcpSocketServer::onSocketConnected(QSslSocket *client)
{
    QUuid clientId = QUuid::createUuid();
    qCDebug(dcTcpSocketServer()) << "New client connected:" << client << client->peerAddress().toString() << clientId.toString();
    m_clientList.insert(clientId, client);
    emit clientConnected(clientId, client->peerAddress());
}

void TcpSocketServer::onSocketDisconnected(QSslSocket *client)
{
    QUuid clientId = m_clientList.key(client);
    if (clientId.isNull()) {
        qCWarning(dcTcpSocketServer()) << "Socket disconnected but the uuid is null." << client << client->peerAddress().toString() << clientId.toString();
        return;
    }

    qCDebug(dcTcpSocketServer()) << "Client disconnected:" << client << client->peerAddress().toString() << clientId.toString();
    m_clientList.take(clientId);
    // Note: the SslServer is deleting the socket object
    emit clientDisconnected(clientId);
}

SslServer::SslServer(bool sslEnabled, const QSslConfiguration &config, QObject *parent) :
    QTcpServer(parent),
    m_sslEnabled(sslEnabled),
    m_config(config)
{
    connect(this, &QTcpServer::acceptError, this, [this](QAbstractSocket::SocketError socketError){
        qCWarning(dcTcpSocketServer()) << "Accept error occurred" << socketError << errorString();
    });

    connect(this, &QTcpServer::newConnection, this, [this](){
        while (hasPendingConnections()) {
            SslClient *sslSocket = qobject_cast<SslClient *>(nextPendingConnection());
            if (!m_sslEnabled) {
                emit socketConnected(sslSocket);
            }
        }
    });
}

void SslServer::incomingConnection(qintptr socketDescriptor)
{
    SslClient *sslSocket = new SslClient(this);
    qCDebug(dcTcpSocketServer()) << "New incomming connection. Creating" << sslSocket;
    if (!sslSocket->setSocketDescriptor(socketDescriptor)) {
        qCWarning(dcTcpSocketServer()) << "Failed to set SSL socket descriptor" << sslSocket << "Discard connection...";
        delete sslSocket;
        return;
    }

    connect(sslSocket, &SslClient::disconnected, this, [this, sslSocket](){
        qCDebug(dcTcpSocketServer()) << "Client socket disconnected:" << sslSocket << sslSocket->peerAddress().toString();;

        if (m_sslEnabled) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
            // FIXME: the SSL socket behavior seems to have changed in Qt6,
            // we need to emit this signal in any case, otherwise the upper layer assumes the
            // socket object still exists, even if we only hand over to upper layers on encypted.
            emit socketDisconnected(sslSocket);
#else
            // Only tell the upper layer the client has disconnecred if it was encrypted
            if (sslSocket->isEncrypted()) {
                emit socketDisconnected(sslSocket);
            }
#endif
        } else {
            emit socketDisconnected(sslSocket);
        }

        m_clients.removeAll(sslSocket);
        sslSocket->deleteLater();
    });

    connect(sslSocket, &QSslSocket::readyRead, this, [this, sslSocket](){

        // Only forward data from an encrypted socket if ssl is enabled
        if (m_sslEnabled && !sslSocket->isEncrypted())
            return;

        QByteArray data = sslSocket->readAll();
        qCDebug(dcTcpSocketServerTraffic()) << "Data from socket" << sslSocket->peerAddress().toString() << data;
        emit dataAvailable(sslSocket, data);
    });

    connect(sslSocket, &SslClient::encrypted, this, [this, sslSocket](){
        qCDebug(dcTcpSocketServer()) << "SSL encryption established for" << sslSocket;
        emit socketConnected(sslSocket);
    });

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    connect(sslSocket, &QSslSocket::errorOccurred, this, [sslSocket](QAbstractSocket::SocketError error){
        qCWarning(dcTcpSocketServer()) << "Socket error occurred on" << sslSocket << error << sslSocket->errorString() << "Explicitly closing the client connection.";
        sslSocket->close();
    });

    connect(sslSocket, &QSslSocket::sslErrors, this, [sslSocket](const QList<QSslError> &errors) {
        qCWarning(dcTcpSocketServer()) << "SSL error occurred in the client connection" << sslSocket;
        foreach (const QSslError &error, errors) {
            qCWarning(dcTcpSocketServer()) << "--> SSL error:" << error.error() << error.errorString();
        }
    });
#else
    typedef void (QAbstractSocket:: *errorSignal)(QAbstractSocket::SocketError);
    connect(sslSocket, static_cast<errorSignal>(&QAbstractSocket::error), this, [sslSocket](QAbstractSocket::SocketError error){
        qCWarning(dcTcpSocketServer()) << "Socket error occurred on" << sslSocket << error << sslSocket->errorString() << "Explicitly closing the client connection.";
        sslSocket->close();
    });

    typedef void (QSslSocket:: *sslErrorsSignal)(const QList<QSslError> &);
    connect(sslSocket, static_cast<sslErrorsSignal>(&QSslSocket::sslErrors), this, [sslSocket](const QList<QSslError> &errors) {
        qCWarning(dcTcpSocketServer()) << "SSL error occurred in the client connection" << sslSocket;
        foreach (const QSslError &error, errors) {
            qCWarning(dcTcpSocketServer()) << "--> SSL error:" << error.error() << error.errorString();
        }
    });

#endif

    if (m_sslEnabled) {
        qCDebug(dcTcpSocketServer()) << "Starting SSL encryption for" << sslSocket;
        sslSocket->setSslConfiguration(m_config);
        sslSocket->startServerEncryption();
        sslSocket->startWaitingForEncrypted();
    }

    m_clients.append(sslSocket);
    addPendingConnection(sslSocket);
}

SslClient::SslClient(QObject *parent) :
    QSslSocket(parent)
{
    m_timer.setSingleShot(true);
    m_timer.setInterval(5000);
    connect(&m_timer, &QTimer::timeout, this, [this](){
        qCWarning(dcTcpSocketServer()) << "SSL socket timeout occurred. The client has not encrypted the connection within" << (m_timer.interval() / 1000) << "seconds. Terminate connection";
        close();
    });

    connect(this, &SslClient::encrypted, &m_timer, &QTimer::stop);
}

void SslClient::startWaitingForEncrypted()
{
    m_timer.start();
}

}

