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
#include "loggingcategories.h"

#include <QDateTime>

namespace remoteproxy {

ProxyClient::ProxyClient(TransportInterface *interface, const QUuid &clientId, const QHostAddress &address, QObject *parent) :
    TransportClient(interface, clientId, address, parent)
{
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &ProxyClient::timeoutOccurred);
    m_timer->setSingleShot(true);
    resetTimer();
}

bool ProxyClient::isAuthenticated() const
{
    return m_authenticated;
}

void ProxyClient::setAuthenticated(bool isAuthenticated)
{
    m_authenticated = isAuthenticated;
    if (m_authenticated) {
        m_timerWaitState = TimerWaitStateAlone;
        resetTimer();
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
        m_timer->stop();
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

ProxyClient::TimerWaitState ProxyClient::timerWaitState() const
{
    return m_timerWaitState;
}

void ProxyClient::resetTimer()
{
    switch (m_timerWaitState) {
    case TimerWaitStateInactive:
        m_timer->stop();
        m_timer->start(Engine::instance()->configuration()->inactiveTimeout());
        break;
    case TimerWaitStateAlone:
        m_timer->stop();
        m_timer->start(Engine::instance()->configuration()->aloneTimeout());
        break;
    }
}

QList<QByteArray> ProxyClient::processData(const QByteArray &data)
{
    QList<QByteArray> packets;

    // Handle packet fragmentation
    m_dataBuffer.append(data);
    int splitIndex = m_dataBuffer.indexOf("}\n{");
    while (splitIndex > -1) {
        packets.append(m_dataBuffer.left(splitIndex + 1));
        m_dataBuffer = m_dataBuffer.right(m_dataBuffer.length() - splitIndex - 2);
        splitIndex = m_dataBuffer.indexOf("}\n{");
    }
    if (m_dataBuffer.trimmed().endsWith("}")) {
        packets.append(m_dataBuffer);
        m_dataBuffer.clear();
    }

    return packets;
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
    debug.nospace() << ", " << proxyClient->creationTimeString() << ")";
    return debug.space();
}

}
