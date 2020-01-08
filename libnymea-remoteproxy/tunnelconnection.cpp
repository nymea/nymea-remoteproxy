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

#include "tunnelconnection.h"

#include <QDateTime>

namespace remoteproxy {

TunnelConnection::TunnelConnection(ProxyClient *clientOne, ProxyClient *clientTwo):
    m_clientOne(clientOne),
    m_clientTwo(clientTwo)
{
    m_creationTimeStamp = QDateTime::currentDateTime().toTime_t();
}

QString TunnelConnection::token() const
{
    if (!isValid())
        return QString();

    return m_clientOne->token();
}

QString TunnelConnection::nonce() const
{
    if (!isValid())
        return QString();

    return m_clientOne->nonce();
}

QString TunnelConnection::tunnelIdentifier() const
{
    if (!isValid())
        return QString();

    return m_clientOne->tunnelIdentifier();
}

uint TunnelConnection::creationTime() const
{
    return m_creationTimeStamp;
}

QString TunnelConnection::creationTimeString() const
{
    return QDateTime::fromTime_t(creationTime()).toString("dd.MM.yyyy hh:mm:ss");
}

ProxyClient *TunnelConnection::clientOne() const
{
    return m_clientOne;
}

ProxyClient *TunnelConnection::clientTwo() const
{
    return m_clientTwo;
}

bool TunnelConnection::hasClient(ProxyClient *proxyClient) const
{
    return m_clientOne == proxyClient || m_clientTwo == proxyClient;
}

bool TunnelConnection::isValid() const
{
    // Both clients have to be valid
    if (!m_clientOne || !m_clientTwo)
        return false;

    // Both clients need the same token
    if (m_clientOne->token() != m_clientTwo->token())
        return false;

    // The clients need to be different
    if (m_clientOne == m_clientTwo)
        return false;

    return true;
}

QDebug operator<<(QDebug debug, const TunnelConnection &tunnel)
{
    debug.nospace() << "TunnelConnection(" << tunnel.creationTimeString() << ")" << endl;
    debug.nospace() << "    --> " << tunnel.clientOne() << endl;
    debug.nospace() << "    --> " << tunnel.clientTwo() << endl;
    return debug;
}

}
