/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                               *
 * Copyright (C) 2018 Simon Stürz <simon.stuerz@guh.io>                          *
 *                                                                               *
 * This file is part of nymea-remoteproxy.                                       *
 *                                                                               *
 * This program is free software: you can redistribute it and/or modify          *
 * it under the terms of the GNU General Public License as published by          *
 * the Free Software Foundation, either version 3 of the License, or             *
 * (at your option) any later version.                                           *
 *                                                                               *
 * This program is distributed in the hope that it will be useful,               *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of                *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                 *
 * GNU General Public License for more details.                                  *
 *                                                                               *
 * You should have received a copy of the GNU General Public License             *
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.         *
 *                                                                               *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "engine.h"
#include "monitorserver.h"
#include "loggingcategories.h"

#include <QFile>
#include <QJsonDocument>

namespace remoteproxy {

MonitorServer::MonitorServer(const QString &serverName, QObject *parent) :
    QObject(parent),
    m_serverName(serverName)
{

}

MonitorServer::~MonitorServer()
{
    stopServer();
}

bool MonitorServer::running() const
{
    if (!m_server)
        return false;

    return m_server->isListening();
}

void MonitorServer::sendMonitorData(QLocalSocket *clientConnection, const QVariantMap &dataMap)
{
    clientConnection->write(QJsonDocument::fromVariant(dataMap).toJson(QJsonDocument::Compact) + '\n');
    clientConnection->flush();
}

void MonitorServer::onMonitorConnected()
{
    QLocalSocket *clientConnection = m_server->nextPendingConnection();
    connect(clientConnection, &QLocalSocket::disconnected, this, &MonitorServer::onMonitorDisconnected);
    connect(clientConnection, &QLocalSocket::readyRead, this, &MonitorServer::onMonitorDisconnected);
    m_clients.append(clientConnection);

    qCDebug(dcMonitorServer()) << "New monitor connected.";
}

void MonitorServer::onMonitorDisconnected()
{
    qCDebug(dcMonitorServer()) << "Monitor disconnected.";
    QLocalSocket *clientConnection = static_cast<QLocalSocket *>(sender());
    m_clients.removeAll(clientConnection);
    clientConnection->deleteLater();
}

void MonitorServer::startServer()
{    
    qCDebug(dcMonitorServer()) << "Starting server on" << m_serverName;

    // Make sure the monitor can be created
    if (QFile::exists(m_serverName)) {
        qCDebug(dcMonitorServer()) << "Clean up old monitor socket";
        QFile::remove(m_serverName);
    }

    m_server = new QLocalServer(this);
    m_server->setSocketOptions(QLocalServer::UserAccessOption | QLocalServer::GroupAccessOption | QLocalServer::OtherAccessOption);

    if (!m_server->listen(m_serverName)) {
        qCWarning(dcMonitorServer()) << "Could not start local server for monitor on" << m_serverName << m_server->errorString();
        delete m_server;
        m_server = nullptr;
        return;
    }

    connect(m_server, &QLocalServer::newConnection, this, &MonitorServer::onMonitorConnected);
    qCDebug(dcMonitorServer()) << "Started successfully on" << m_serverName;
}

void MonitorServer::stopServer()
{
    if (!m_server)
        return;

    qCDebug(dcMonitorServer()) << "Stop server" << m_serverName;
    foreach (QLocalSocket *clientConnection, m_clients) {
        clientConnection->close();
    }

    m_server->close();
    delete m_server;
    m_server = nullptr;
}

void MonitorServer::updateClients(const QVariantMap &dataMap)
{
    // Send each monitor the current data
    foreach (QLocalSocket *clientConnection, m_clients) {
        sendMonitorData(clientConnection, dataMap);
    }
}

}
