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

#include "websocketconnection.h"

Q_LOGGING_CATEGORY(dcRemoteProxyClientWebSocket, "RemoteProxyClientWebSocket")

namespace remoteproxyclient {

WebSocketConnection::WebSocketConnection(QObject *parent) :
    ProxyConnection(parent)
{
    m_webSocket = new QWebSocket("libnymea-remoteproxyclient", QWebSocketProtocol::Version13, this);

    connect(m_webSocket, &QWebSocket::disconnected, this, &WebSocketConnection::onDisconnected);
    connect(m_webSocket, &QWebSocket::textMessageReceived, this, &WebSocketConnection::onTextMessageReceived);

    connect(m_webSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(onError(QAbstractSocket::SocketError)));
    connect(m_webSocket, SIGNAL(stateChanged(QAbstractSocket::SocketState)), this, SLOT(onStateChanged(QAbstractSocket::SocketState)));
    connect(m_webSocket, SIGNAL(sslErrors(QList<QSslError>)), this, SIGNAL(sslErrors(QList<QSslError>)));
}

WebSocketConnection::~WebSocketConnection()
{
    m_webSocket->close();
}

void WebSocketConnection::sendData(const QByteArray &data)
{
    m_webSocket->sendTextMessage(QString::fromUtf8(data));
}

void WebSocketConnection::ignoreSslErrors()
{
    m_webSocket->ignoreSslErrors();
}

void WebSocketConnection::ignoreSslErrors(const QList<QSslError> &errors)
{
    m_webSocket->ignoreSslErrors(errors);
}

void WebSocketConnection::onDisconnected()
{
    qCDebug(dcRemoteProxyClientWebSocket()) << "Disconnected from" << m_webSocket->requestUrl().toString() << m_webSocket->closeReason();
    setConnected(false);
}

void WebSocketConnection::onError(QAbstractSocket::SocketError error)
{
    qCDebug(dcRemoteProxyClientWebSocket()) << "Socket error occurred" << error << m_webSocket->errorString();
    emit errorOccurred(error);
}

void WebSocketConnection::onStateChanged(QAbstractSocket::SocketState state)
{
    qCDebug(dcRemoteProxyClientWebSocket()) << "Socket state changed" << state;

    switch (state) {
    case QAbstractSocket::ConnectedState:
        qCDebug(dcRemoteProxyClientWebSocket()) << "Connected with" << m_webSocket->requestUrl().toString();
        setConnected(true);
        break;
    default:
        setConnected(false);
        break;
    }

    emit stateChanged(state);
}

void WebSocketConnection::onTextMessageReceived(const QString &message)
{
    emit dataReceived(message.toUtf8());
}

void WebSocketConnection::connectServer(const QUrl &serverUrl)
{
    if (connected()) {
        m_webSocket->close();
    }
    m_serverUrl = serverUrl;

    qCDebug(dcRemoteProxyClientWebSocket()) << "Connecting to" << m_serverUrl.toString();
    m_webSocket->open(this->serverUrl());
}

void WebSocketConnection::disconnectServer()
{
    qCDebug(dcRemoteProxyClientWebSocket()) << "Disconnecting from" << m_serverUrl.toString();
    m_webSocket->close();
}

}
