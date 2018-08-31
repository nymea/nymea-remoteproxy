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

#ifndef TUNNELCONNECTION_H
#define TUNNELCONNECTION_H

#include "proxyclient.h"

namespace remoteproxy {

class TunnelConnection
{
public:
    TunnelConnection(ProxyClient *clientOne = nullptr, ProxyClient *clientTwo = nullptr);

    QString token() const;

    uint creationTime() const;
    QString creationTimeString() const;

    ProxyClient *clientOne() const;
    ProxyClient *clientTwo() const;

    bool hasClient(ProxyClient *proxyClient) const;

    bool isValid() const;

private:
    uint m_creationTimeStamp = 0;

    ProxyClient *m_clientOne = nullptr;
    ProxyClient *m_clientTwo = nullptr;

};

QDebug operator<< (QDebug debug, const TunnelConnection &tunnel);

}

#endif // TUNNELCONNECTION_H
