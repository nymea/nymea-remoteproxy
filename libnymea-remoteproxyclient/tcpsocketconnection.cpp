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

#include "tcpsocketconnection.h"

Q_LOGGING_CATEGORY(dcRemoteProxyClientTcpSocket, "RemoteProxyClientTcpSocket")

namespace remoteproxyclient {

TcpSocketConnection::TcpSocketConnection(QObject *parent) :
    ProxyConnection(parent)
{
    m_tcpSocket = new QSslSocket(this);

    connect(m_tcpSocket, &QSslSocket::disconnected, this, &TcpSocketConnection::onDisconnected);
    connect(m_tcpSocket, &QSslSocket::encrypted, this, &TcpSocketConnection::onEncrypted);
    connect(m_tcpSocket, &QSslSocket::readyRead, this, &TcpSocketConnection::onReadyRead);
    connect(m_tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(onError(QAbstractSocket::SocketError)));
    connect(m_tcpSocket, SIGNAL(stateChanged(QAbstractSocket::SocketState)), this, SLOT(onStateChanged(QAbstractSocket::SocketState)));
    connect(m_tcpSocket, SIGNAL(sslErrors(QList<QSslError>)), this, SIGNAL(sslErrors(QList<QSslError>)));
}

TcpSocketConnection::~TcpSocketConnection()
{
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
    qCDebug(dcRemoteProxyClientTcpSocket()) << "Socket error occured" << error << m_tcpSocket->errorString();
    emit errorOccured(error);
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
    setServerUrl(serverUrl);
    qCDebug(dcRemoteProxyClientTcpSocket()) << "Connecting to" << this->serverUrl().toString();

    if (serverUrl.scheme() == "tcp") {
        m_tcpSocket->connectToHost(QHostAddress(this->serverUrl().host()), static_cast<quint16>(this->serverUrl().port()));
    } else {
        m_ssl = true;
        m_tcpSocket->connectToHostEncrypted(this->serverUrl().host(), static_cast<quint16>(this->serverUrl().port()));
    }
}

void TcpSocketConnection::disconnectServer()
{
    qCDebug(dcRemoteProxyClientTcpSocket()) << "Disconnecting from" << serverUrl().toString();
    m_tcpSocket->close();
}

}
