/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright 2013 - 2020, nymea GmbH
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
    qCDebug(dcRemoteProxyClientWebSocket()) << "Socket error occured" << error << m_webSocket->errorString();
    emit errorOccured(error);
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
    setServerUrl(serverUrl);

    qCDebug(dcRemoteProxyClientWebSocket()) << "Connecting to" << this->serverUrl().toString();
    m_webSocket->open(this->serverUrl());
}

void WebSocketConnection::disconnectServer()
{
    qCDebug(dcRemoteProxyClientWebSocket()) << "Disconnecting from" << serverUrl().toString();
    m_webSocket->close();
}

}
