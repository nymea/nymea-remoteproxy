// SPDX-License-Identifier: LGPL-3.0-or-later

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* nymea-remoteproxy
* Tunnel proxy server for the nymea remote access
*
* Copyright (C) 2013 - 2024, nymea GmbH
* Copyright (C) 2024 - 2025, chargebyte austria GmbH
*
* This file is part of nymea-remoteproxy.
*
* nymea-remoteproxy is free software: you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public License
* as published by the Free Software Foundation, either version 3
* of the License, or (at your option) any later version.
*
* nymea-remoteproxy is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with nymea-remoteproxy. If not, see <https://www.gnu.org/licenses/>.
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "loggingcategories.h"

Q_LOGGING_CATEGORY(dcApplication, "Application")
Q_LOGGING_CATEGORY(dcEngine, "Engine")
Q_LOGGING_CATEGORY(dcJsonRpc, "JsonRpc")
Q_LOGGING_CATEGORY(dcJsonRpcTraffic, "JsonRpcTraffic")
Q_LOGGING_CATEGORY(dcTcpSocketServer, "TcpSocketServer")
Q_LOGGING_CATEGORY(dcTcpSocketServerTraffic, "TcpSocketServerTraffic")
Q_LOGGING_CATEGORY(dcWebSocketServer, "WebSocketServer")
Q_LOGGING_CATEGORY(dcWebSocketServerTraffic, "WebSocketServerTraffic")
Q_LOGGING_CATEGORY(dcTunnelProxyServer, "TunnelProxyServer")
Q_LOGGING_CATEGORY(dcTunnelProxyServerTraffic, "TunnelProxyServerTraffic")
Q_LOGGING_CATEGORY(dcMonitorServer, "MonitorServer")
Q_LOGGING_CATEGORY(dcUnixSocketServer, "UnixSocketServer")
Q_LOGGING_CATEGORY(dcUnixSocketServerTraffic, "UnixSocketServerTraffic")

