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

#ifndef LOGGINGCATEGORIES_H
#define LOGGINGCATEGORIES_H

#include <QDebug>
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(dcApplication)
Q_DECLARE_LOGGING_CATEGORY(dcEngine)
Q_DECLARE_LOGGING_CATEGORY(dcJsonRpc)
Q_DECLARE_LOGGING_CATEGORY(dcTunnel)
Q_DECLARE_LOGGING_CATEGORY(dcJsonRpcTraffic)
Q_DECLARE_LOGGING_CATEGORY(dcWebSocketServer)
Q_DECLARE_LOGGING_CATEGORY(dcWebSocketServerTraffic)
Q_DECLARE_LOGGING_CATEGORY(dcTcpSocketServer)
Q_DECLARE_LOGGING_CATEGORY(dcTcpSocketServerTraffic)
Q_DECLARE_LOGGING_CATEGORY(dcAuthentication)
Q_DECLARE_LOGGING_CATEGORY(dcAuthenticationProcess)
Q_DECLARE_LOGGING_CATEGORY(dcProxyServer)
Q_DECLARE_LOGGING_CATEGORY(dcProxyServerTraffic)
Q_DECLARE_LOGGING_CATEGORY(dcMonitorServer)
Q_DECLARE_LOGGING_CATEGORY(dcAwsCredentialsProvider)
Q_DECLARE_LOGGING_CATEGORY(dcAwsCredentialsProviderTraffic)

#endif // LOGGINGCATEGORIES_H
