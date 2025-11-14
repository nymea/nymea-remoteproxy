// SPDX-License-Identifier: GPL-3.0-or-later

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright (C) 2013 - 2024, nymea GmbH
* Copyright (C) 2024 - 2025, chargebyte austria GmbH
*
* This file is part of nymea-remoteproxy.
*
* nymea-remoteproxy is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* nymea-remoteproxy is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with nymea-remoteproxy. If not, see <https://www.gnu.org/licenses/>.
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "serverconnection.h"

ServerConnection::ServerConnection(const QUrl &serverUrl, const QString &name, const QUuid &uuid, bool insecure, bool echo, QObject *parent) :
    QObject(parent),
    m_serverUrl(serverUrl),
    m_name(name),
    m_uuid(uuid),
    m_insecure(insecure),
    m_echo(echo)
{

    m_socketServer = new TunnelProxySocketServer(m_uuid, m_name, this);

    connect(m_socketServer, &TunnelProxySocketServer::clientConnected, this, [this](TunnelProxySocket *tunnelProxySocket){
        qDebug() << "[+] Client connected" << tunnelProxySocket;
        if (m_echo) {
            connect(tunnelProxySocket, &TunnelProxySocket::dataReceived, m_socketServer, [tunnelProxySocket](const QByteArray &data){
                tunnelProxySocket->writeData(data);
            });
        }
    });

    connect(m_socketServer, &TunnelProxySocketServer::clientDisconnected, this, [](TunnelProxySocket *tunnelProxySocket){
        qDebug() << "[-] Client disconnected" << tunnelProxySocket;
    });

    connect(m_socketServer, &TunnelProxySocketServer::runningChanged, this, [this](bool running){
        if (running) {
            qDebug() << "Connected with" << m_socketServer->remoteProxyServer() << m_socketServer->remoteProxyServerName() << m_socketServer->remoteProxyServerVersion() << m_socketServer->remoteProxyApiVersion();
        }
        qDebug() << "--> The tunnel proxy server is" << (running ? "running and listening for incoming connections" : "not running any more");
    });

    connect(m_socketServer, &TunnelProxySocketServer::sslErrors, this, [this](const QList<QSslError> &errors){
        if (m_insecure) {
            m_socketServer->ignoreSslErrors(errors);
        } else {
            qWarning() << "SSL errors occurred:";
            foreach (const QSslError &sslError, errors) {
                qWarning() << "  --> " << sslError.errorString();
            }
        }
    });
}

void ServerConnection::startServer()
{
    m_socketServer->startServer(m_serverUrl);
}
