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

#include "clientconnection.h"

ClientConnection::ClientConnection(const QUrl &serverUrl, const QString &name, const QUuid &uuid, const QUuid &serverUuid, bool insecure, bool sendRandomData, QObject *parent) :
    QObject(parent),
    m_serverUrl(serverUrl),
    m_name(name),
    m_uuid(uuid),
    m_serverUuid(serverUuid),
    m_insecure(insecure),
    m_sendRandomData(sendRandomData)
{
    m_remoteConnection = new TunnelProxyRemoteConnection(m_uuid, m_name);
    m_timer.setSingleShot(true);
    connect(&m_timer, &QTimer::timeout, this, [this](){
        if (m_remoteConnection->remoteConnected()) {
            m_remoteConnection->sendData(generateRandomString(100).toUtf8());
            m_timer.start(1000);
        }
    });

    connect(m_remoteConnection, &TunnelProxyRemoteConnection::stateChanged, this, [this](TunnelProxyRemoteConnection::State state){
        qDebug() << state;
        switch (state) {
        case TunnelProxyRemoteConnection::StateRegister:
            qDebug() << "Connected with" << m_remoteConnection->remoteProxyServer() << m_remoteConnection->remoteProxyServerName() << m_remoteConnection->remoteProxyServerVersion() << m_remoteConnection->remoteProxyApiVersion();
            break;
        default:
            break;
        }
    });

    connect(m_remoteConnection, &TunnelProxyRemoteConnection::remoteConnectedChanged, this, [this](bool remoteConnected){
        qDebug() << "Remote connection" << (remoteConnected ? "established successfully" : "disconnected");
        if (remoteConnected) {
            m_timer.start(1000);
        }
    });

    connect(m_remoteConnection, &TunnelProxyRemoteConnection::dataReady, this, [this](const QByteArray &data){
        if (!m_sendRandomData)
            qDebug() << "Data received" << data;
    });

    connect(m_remoteConnection, &TunnelProxyRemoteConnection::errorOccurred, this, [](QAbstractSocket::SocketError error){
        qWarning() << "Socket error occurred" << error;
    });

    connect(m_remoteConnection, &TunnelProxyRemoteConnection::sslErrors, this, [this](const QList<QSslError> &errors){
        if (m_insecure) {
            m_remoteConnection->ignoreSslErrors(errors);
        } else {
            qWarning() << "SSL errors occurred:";
            foreach (const QSslError &sslError, errors) {
                qWarning() << "  --> " << sslError.errorString();
            }
        }
    });
}

void ClientConnection::connectToServer()
{
    m_remoteConnection->connectServer(m_serverUrl, m_serverUuid);
}

QString ClientConnection::generateRandomString(uint length) const
{
    const QString possibleCharacters("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789");
    QString randomString;
    for(uint i = 0; i < length; i++) {
        randomString.append(possibleCharacters.at(std::rand() % possibleCharacters.length()));
    }
    return randomString;
}
