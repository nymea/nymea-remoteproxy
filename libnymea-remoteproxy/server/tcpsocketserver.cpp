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
    stopServer();
}

void TcpSocketServer::sendData(const QUuid &clientId, const QByteArray &data)
{
    QTcpSocket *client = m_clientList.value(clientId);
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
    QTcpSocket *client = m_clientList.value(clientId);
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
    //qCDebug(dcTcpSocketServerTraffic()) << "Emitting data available internal.";
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
            QSslSocket *sslSocket = qobject_cast<QSslSocket *>(nextPendingConnection());
            if (m_sslEnabled) {
                if (sslSocket->isEncrypted()) {
                    qCDebug(dcTcpSocketServer()) << "SSL encryption established for" << sslSocket;
                    emit socketConnected(sslSocket);
                }
            } else {
                emit socketConnected(sslSocket);
            }
        }
    });
}

void SslServer::incomingConnection(qintptr socketDescriptor)
{
    QSslSocket *sslSocket = new QSslSocket(this);
    qCDebug(dcTcpSocketServer()) << "New incomming connection. Creating" << sslSocket;
    if (!sslSocket->setSocketDescriptor(socketDescriptor)) {
        qCWarning(dcTcpSocketServer()) << "Failed to set SSL socket descriptor" << sslSocket << "Discard connection...";
        delete sslSocket;
        return;
    }

    connect(sslSocket, &QSslSocket::disconnected, this, [this, sslSocket](){
        qCDebug(dcTcpSocketServer()) << "Client socket disconnected:" << sslSocket << sslSocket->peerAddress().toString();;
        emit socketDisconnected(sslSocket);
        sslSocket->deleteLater();
    });


    typedef void (QAbstractSocket:: *errorSignal)(QAbstractSocket::SocketError);
    connect(sslSocket, static_cast<errorSignal>(&QAbstractSocket::error), this, [sslSocket](QAbstractSocket::SocketError error){
        qCWarning(dcTcpSocketServer()) << "Socket error occurred on" << sslSocket << error << sslSocket->errorString() << "Explicitly closing the socket connection.";
        sslSocket->abort();
    });

    connect(sslSocket, &QSslSocket::encrypted, this, [this, sslSocket](){
        qCDebug(dcTcpSocketServer()) << "SSL encryption established for" << sslSocket;
        emit socketConnected(sslSocket);
    });

    connect(sslSocket, QOverload<const QList<QSslError> &>::of(&QSslSocket::sslErrors), this, [=](const QList<QSslError> &errors){
        qCWarning(dcTcpSocketServer()) << "SSL error occurred in the client connection" << sslSocket;
        foreach (const QSslError &error, errors) {
            qCWarning(dcTcpSocketServer()) << "SSL error:" << error.error() << error.errorString();
        }
    });

    if (m_sslEnabled) {
        qCDebug(dcTcpSocketServer()) << "Start SSL encryption for" << sslSocket;
        sslSocket->setSslConfiguration(m_config);
        sslSocket->startServerEncryption();
    }

    addPendingConnection(sslSocket);
}

}

