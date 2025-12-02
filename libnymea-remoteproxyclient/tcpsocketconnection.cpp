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

#include "tcpsocketconnection.h"

Q_LOGGING_CATEGORY(dcRemoteProxyClientTcpSocket, "RemoteProxyClientTcpSocket")

namespace remoteproxyclient {

TcpSocketConnection::TcpSocketConnection(QObject *parent) :
    ProxyConnection(parent)
{
    m_tcpSocket = new QSslSocket(this);

    QObject::connect(m_tcpSocket, &QSslSocket::disconnected, this, &TcpSocketConnection::onDisconnected);
    QObject::connect(m_tcpSocket, &QSslSocket::encrypted, this, &TcpSocketConnection::onEncrypted);
    QObject::connect(m_tcpSocket, &QSslSocket::readyRead, this, &TcpSocketConnection::onReadyRead);
    QObject::connect(m_tcpSocket, &QSslSocket::stateChanged, this, &TcpSocketConnection::onStateChanged);

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    connect(m_tcpSocket, &QSslSocket::errorOccurred, this, &TcpSocketConnection::onError);
    connect(m_tcpSocket, &QSslSocket::sslErrors, this, &TcpSocketConnection::sslErrors);
#else
    typedef void (QSslSocket:: *errorSignal)(QAbstractSocket::SocketError);
    QObject::connect(m_tcpSocket, static_cast<errorSignal>(&QSslSocket::error), this, &TcpSocketConnection::onError);

    typedef void (QSslSocket:: *sslErrorsSignal)(const QList<QSslError> &);
    QObject::connect(m_tcpSocket, static_cast<sslErrorsSignal>(&QSslSocket::sslErrors), this, &TcpSocketConnection::sslErrors);
#endif

}

TcpSocketConnection::~TcpSocketConnection()
{
    qCDebug(dcRemoteProxyClientTcpSocket()) << "Delete socket connection";
    m_tcpSocket->close();
}

void TcpSocketConnection::sendData(const QByteArray &data)
{
    m_tcpSocket->write(data);
}

void TcpSocketConnection::ignoreSslErrors()
{
    m_tcpSocket->ignoreSslErrors();
}

void TcpSocketConnection::ignoreSslErrors(const QList<QSslError> &errors)
{
    m_tcpSocket->ignoreSslErrors(errors);
}

void TcpSocketConnection::onDisconnected()
{
    qCDebug(dcRemoteProxyClientTcpSocket()) << "Disconnected from" << serverUrl().toString();
    setConnected(false);
}

void TcpSocketConnection::onEncrypted()
{
    qCDebug(dcRemoteProxyClientTcpSocket()) << "Connection encrypted";
    setConnected(true);
}

void TcpSocketConnection::onError(QAbstractSocket::SocketError error)
{
    qCWarning(dcRemoteProxyClientTcpSocket()) << "Socket error occurred" << error << m_tcpSocket->errorString();
    emit errorOccurred(error);
}

void TcpSocketConnection::onStateChanged(QAbstractSocket::SocketState state)
{
    qCDebug(dcRemoteProxyClientTcpSocket()) << "Socket state changed" << state;

    switch (state) {
    case QAbstractSocket::ConnectedState:
        qCDebug(dcRemoteProxyClientTcpSocket()) << "Connected with" << serverUrl().toString();
        if (!m_ssl) {
            setConnected(true);
        }
        break;
    default:
        setConnected(false);
        break;
    }

    emit stateChanged(state);
}

void TcpSocketConnection::onReadyRead()
{
    emit dataReceived(m_tcpSocket->readAll());
}

void TcpSocketConnection::connectServer(const QUrl &serverUrl)
{
    m_serverUrl = serverUrl;

    if (m_serverUrl.scheme() == "tcp") {
        qCDebug(dcRemoteProxyClientTcpSocket()) << "Connecting to" << m_serverUrl.toString();
        m_tcpSocket->connectToHost(QHostAddress(m_serverUrl.host()), static_cast<quint16>(m_serverUrl.port()));
    } else {
        m_ssl = true;
        qCDebug(dcRemoteProxyClientTcpSocket()) << "Connecting encrypted to" << m_serverUrl.host() + ":" + QString::number(m_serverUrl.port());
        m_tcpSocket->connectToHostEncrypted(m_serverUrl.host(), static_cast<quint16>(m_serverUrl.port()));
    }
}

void TcpSocketConnection::disconnectServer()
{
    qCDebug(dcRemoteProxyClientTcpSocket()) << "Disconnecting from" << serverUrl().toString();
    m_tcpSocket->disconnectFromHost();
    m_tcpSocket->close();
    m_tcpSocket->abort();
}

}
