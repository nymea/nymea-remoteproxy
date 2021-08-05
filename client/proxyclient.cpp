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

#include "proxyclient.h"

#include <QTimer>

Q_LOGGING_CATEGORY(dcProxyClient, "ProxyClient")

ProxyClient::ProxyClient(const QString &name, const QUuid &uuid, RemoteProxyConnection::ConnectionType connectionType, QObject *parent) :
    QObject(parent),
    m_name(name),
    m_uuid(uuid)
{
    m_connection = new RemoteProxyConnection(m_uuid, m_name, connectionType, this);
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
    m_connection->authenticate(m_token, m_nonce);
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

void ProxyClient::start(const QUrl &url, const QString &token, const QString &nonce)
{
    m_token = token;
    m_nonce = nonce;
    m_connection->connectServer(url);
}
