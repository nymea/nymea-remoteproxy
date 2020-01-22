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
