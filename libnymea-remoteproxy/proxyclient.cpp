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

#include "engine.h"
#include "proxyclient.h"

#include <QDateTime>

namespace remoteproxy {

ProxyClient::ProxyClient(TransportInterface *interface, const QUuid &clientId, const QHostAddress &address, QObject *parent) :
    QObject(parent),
    m_interface(interface),
    m_clientId(clientId),
    m_peerAddress(address)
{
    m_creationTimeStamp = QDateTime::currentDateTime().toTime_t();

    connect(&m_timer, &QTimer::timeout, this, &ProxyClient::timeoutOccured);
    m_timer.setSingleShot(true);
    m_timer.start(Engine::instance()->configuration()->inactiveTimeout());
}

QUuid ProxyClient::clientId() const
{
    return m_clientId;
}

QHostAddress ProxyClient::peerAddress() const
{
    return m_peerAddress;
}

uint ProxyClient::creationTime() const
{
    return m_creationTimeStamp;
}

QString ProxyClient::creationTimeString() const
{
    return QDateTime::fromTime_t(creationTime()).toString("dd.MM.yyyy hh:mm:ss");
}

bool ProxyClient::isAuthenticated() const
{
    return m_authenticated;
}

void ProxyClient::setAuthenticated(bool isAuthenticated)
{
    m_authenticated = isAuthenticated;
    if (m_authenticated) {
        m_timer.stop();
        m_timer.start(Engine::instance()->configuration()->aloneTimeout());
        emit authenticated();
    }
}

bool ProxyClient::isTunnelConnected() const
{
    return m_tunnelConnected;
}

void ProxyClient::setTunnelConnected(bool isTunnelConnected)
{
    m_tunnelConnected = isTunnelConnected;
    if (m_tunnelConnected) {
        m_timer.stop();
        emit tunnelConnected();
    }
}

QString ProxyClient::userName() const
{
    return m_userName;
}

void ProxyClient::setUserName(const QString &userName)
{
    m_userName = userName;
}

TransportInterface *ProxyClient::interface() const
{
    return m_interface;
}

QString ProxyClient::uuid() const
{
    return m_uuid;
}

void ProxyClient::setUuid(const QString &uuid)
{
    m_uuid = uuid;
}

QString ProxyClient::name() const
{
    return m_name;
}

void ProxyClient::setName(const QString &name)
{
    m_name = name;
}

QString ProxyClient::tunnelIdentifier() const
{
    return m_token + m_nonce;
}

QString ProxyClient::token() const
{
    return m_token;
}

void ProxyClient::setToken(const QString &token)
{
    m_token = token;
}

QString ProxyClient::nonce() const
{
    return m_nonce;
}

void ProxyClient::setNonce(const QString &nonce)
{
    m_nonce = nonce;
}

quint64 ProxyClient::rxDataCount() const
{
    return m_rxDataCount;
}

void ProxyClient::addRxDataCount(int dataCount)
{
    m_rxDataCount += static_cast<quint64>(dataCount);
}

quint64 ProxyClient::txDataCount() const
{
    return m_txDataCount;
}

void ProxyClient::addTxDataCount(int dataCount)
{
    m_txDataCount += static_cast<quint64>(dataCount);
}

void ProxyClient::sendData(const QByteArray &data)
{
    if (!m_interface)
        return;

    m_interface->sendData(m_clientId, data);
}

void ProxyClient::killConnection(const QString &reason)
{
    if (!m_interface)
        return;

    m_interface->killClientConnection(m_clientId, reason);
}

QDebug operator<<(QDebug debug, ProxyClient *proxyClient)
{
    debug.nospace() << "ProxyClient(";
    if (!proxyClient->name().isEmpty()) {
        debug.nospace() << proxyClient->name() << ", ";
    }
    debug.nospace() << proxyClient->interface()->serverName();
    debug.nospace() << ", " << proxyClient->clientId().toString();
    debug.nospace() << ", " << proxyClient->userName();
    debug.nospace() << ", " << proxyClient->peerAddress().toString();
    debug.nospace() << ", " << proxyClient->creationTimeString() << ") ";
    return debug;
}

}
