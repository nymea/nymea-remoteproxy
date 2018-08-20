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

#include "proxyclient.h"

#include <QTimer>

Q_LOGGING_CATEGORY(dcProxyClient, "ProxyClient")

ProxyClient::ProxyClient(const QString &name, const QUuid &uuid, QObject *parent) :
    QObject(parent),
    m_name(name),
    m_uuid(uuid)
{
    m_connection = new RemoteProxyConnection(m_uuid, m_name, this);
    qCDebug(dcProxyClient()) << "Creating remote proxy connection" << m_name << m_uuid.toString();
    connect(m_connection, &RemoteProxyConnection::ready, this, &ProxyClient::onClientReady);
    connect(m_connection, &RemoteProxyConnection::authenticated, this, &ProxyClient::onAuthenticationFinished);
    connect(m_connection, &RemoteProxyConnection::remoteConnectionEstablished, this, &ProxyClient::onRemoteConnectionEstablished);
    connect(m_connection, &RemoteProxyConnection::dataReady, this, &ProxyClient::onDataReady);
    connect(m_connection, &RemoteProxyConnection::errorOccured, this, &ProxyClient::onErrorOccured);
    connect(m_connection, &RemoteProxyConnection::disconnected, this, &ProxyClient::onClientDisconnected);
    connect(m_connection, &RemoteProxyConnection::sslErrors, this, &ProxyClient::onSslErrors);
}

void ProxyClient::setInsecure(bool insecure)
{
    m_insecure = insecure;
}

void ProxyClient::setPingpong(bool enable)
{
    m_pingpong = enable;
}

void ProxyClient::onErrorOccured(QAbstractSocket::SocketError error)
{
    qCWarning(dcProxyClient()) << "Error occured" << error;
    exit(-1);
}

void ProxyClient::onClientReady()
{
    qCDebug(dcProxyClient()) << "Connected to proxy server" << m_connection->serverUrl().toString();
    qCDebug(dcProxyClient()) << "Start authentication";
    m_connection->authenticate(m_token);
}

void ProxyClient::onAuthenticationFinished()
{
    qCDebug(dcProxyClient()) << "Authentication finished successfully.";
}

void ProxyClient::onRemoteConnectionEstablished()
{
    qCDebug(dcProxyClient()) << "----------------------------------------------------------------------------------";
    qCDebug(dcProxyClient()) << "Remote connection established with" << m_connection->tunnelPartnerName() << m_connection->tunnelPartnerUuid();
    qCDebug(dcProxyClient()) << "----------------------------------------------------------------------------------";
    if (m_pingpong) sendPing();
}

void ProxyClient::onDataReady(const QByteArray &data)
{
    qCDebug(dcProxyClient()) << "<--" << qUtf8Printable(data);
    if (m_pingpong) {
        QTimer::singleShot(1000, this, &ProxyClient::sendPing);
    }
}

void ProxyClient::onClientDisconnected()
{
    qCDebug(dcProxyClient()) << "Disconnected from" << m_connection->serverUrl().toString();
    exit(1);
}

void ProxyClient::onSslErrors(const QList<QSslError> errors)
{
    if (m_insecure) {
        qCDebug(dcProxyClient()) << "SSL errors occured. Ignoring because explicit specified.";
        m_connection->ignoreSslErrors();
    } else {
        qCWarning(dcProxyClient()) << "SSL errors occured:";
        foreach (const QSslError &sslError, errors) {
            qCWarning(dcProxyClient()) << "  --> " << sslError.errorString();
        }
    }
}

void ProxyClient::sendPing()
{
    QByteArray data(QString("Ping from " + m_name + "!").toUtf8());
    qCDebug(dcProxyClient()) << "-->" << qUtf8Printable(data);
    m_connection->sendData(data);
}

void ProxyClient::start(const QUrl &url, const QString &token)
{
    m_token = token;
    m_connection->connectServer(url);
}
