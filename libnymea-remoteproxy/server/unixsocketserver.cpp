/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
*  Copyright 2013 - 2022, nymea GmbH
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

#include "unixsocketserver.h"

#include <QFile>

#include "loggingcategories.h"

namespace remoteproxy {

UnixSocketServer::UnixSocketServer(QString socketFileName, QObject *parent) :
    TransportInterface(parent),
    m_socketFileName(socketFileName)
{
    m_serverName = "unix";
}

UnixSocketServer::~UnixSocketServer()
{
    stopServer();
}

void UnixSocketServer::sendData(const QUuid &clientId, const QByteArray &data)
{
    QLocalSocket *client = nullptr;
    client = m_clientList.value(clientId);
    if (!client) {
        qCWarning(dcUnixSocketServer()) << "Client" << clientId << "unknown to this transport";
        return;
    }

    qCDebug(dcUnixSocketServerTraffic()) << "Send data to" << clientId.toString() << data;
    if (client->write(data) < 0) {
        qCWarning(dcUnixSocketServer()) << "Could not write data to client socket" << clientId.toString();
    }
    client->flush();
}

void UnixSocketServer::killClientConnection(const QUuid &clientId, const QString &killReason)
{
    QLocalSocket *client = m_clientList.value(clientId);
    if (!client)
        return;

    if (client->state() == QLocalSocket::ConnectedState) {
        qCWarning(dcUnixSocketServer()) << "Killing client connection" << clientId.toString() << "Reason:" << killReason;
        client->close();
    }
}

bool UnixSocketServer::running() const
{
    if (!m_server)
        return false;

    return m_server->isListening();
}

bool UnixSocketServer::startServer()
{
    qCDebug(dcUnixSocketServer()) << "Starting server on" << m_socketFileName;

    // Make sure the monitor can be created
    if (QFile::exists(m_socketFileName)) {
        qCDebug(dcUnixSocketServer()) << "Clean up old unix socket";
        QFile::remove(m_socketFileName);
    }

    m_server = new QLocalServer(this);
    m_server->setSocketOptions(QLocalServer::UserAccessOption | QLocalServer::GroupAccessOption | QLocalServer::OtherAccessOption);
    if (!m_server->listen(m_socketFileName)) {
        qCWarning(dcUnixSocketServer()) << "Could not start local server for monitor on" << m_socketFileName << m_server->errorString();
        delete m_server;
        m_server = nullptr;
        return false;
    }

    connect(m_server, &QLocalServer::newConnection, this, &UnixSocketServer::onClientConnected);
    qCDebug(dcUnixSocketServer()) << "Started successfully on" << m_serverName << m_socketFileName;
    return true;
}

bool UnixSocketServer::stopServer()
{
    if (!m_server)
        return true;

    qCDebug(dcUnixSocketServer()) << "Stopping server" << m_socketFileName;
    foreach (QLocalSocket *clientConnection, m_clientList) {
        clientConnection->close();
    }

    m_server->close();
    delete m_server;
    m_server = nullptr;

    return true;
}

void UnixSocketServer::onClientConnected()
{
    QLocalSocket *client = m_server->nextPendingConnection();
    QUuid clientId = QUuid::createUuid();
    qCDebug(dcUnixSocketServer()) << "New client connected" << clientId.toString();
    m_clientList.insert(clientId, client);

    connect(client, &QLocalSocket::disconnected, this, [=](){
        QUuid clientId = m_clientList.key(client);
        qCDebug(dcUnixSocketServer()) << "Client disconnected:" << clientId.toString();
        if (m_clientList.take(clientId)) {
            emit clientDisconnected(clientId);
        }
    });
    connect(client, &QLocalSocket::readyRead, this, [=](){
        QByteArray data = client->readAll();
        qCDebug(dcUnixSocketServerTraffic()) << "Incomming data from" << clientId.toString() << data;
        emit dataAvailable(clientId, data);
    });

    emit clientConnected(clientId, QHostAddress::LocalHost);
}

}
