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

namespace remoteproxy {

ProxyClient::ProxyClient(TransportInterface *interface, const QUuid &clientId, QObject *parent) :
    QObject(parent),
    m_interface(interface),
    m_clientId(clientId)
{

}

QUuid ProxyClient::clientId() const
{
    return m_clientId;
}

bool ProxyClient::isAuthenticated() const
{
    return m_authenticated;
}

void ProxyClient::setAuthenticated(bool isAuthenticated)
{
    // TODO: start the timeout counter and disconnect if no tunnel established
    m_authenticated = isAuthenticated;
    if (m_authenticated){
        emit authenticated();
    }
}

bool ProxyClient::isTunnelConnected() const
{
    return m_tunnelConnected;
}

void ProxyClient::setTunnelConnected(bool isTunnelConnected)
{
    // TODO: reset the timeout counter and disconnect if no tunnel established
    m_tunnelConnected = isTunnelConnected;
    if (m_tunnelConnected){
        emit tunnelConnected();
    }
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

QString ProxyClient::token() const
{
    return m_token;
}

void ProxyClient::setToken(const QString &token)
{
    m_token = token;
}

QDebug operator<<(QDebug debug, ProxyClient *proxyClient)
{
    debug.nospace() << "ProxyClient(" << proxyClient->interface()->serverName();
    debug.nospace() << ", " << proxyClient->clientId().toString() << ") ";
    return debug;
}

}
