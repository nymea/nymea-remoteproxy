// SPDX-License-Identifier: GPL-3.0-or-later

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright (C) 2013 - 2024, nymea GmbH
* Copyright (C) 2024 - 2025, chargebyte austria GmbH
*
* This file is part of nymea-remoteproxy.
*
* nymea-remoteproxy is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* nymea-remoteproxy is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with nymea-remoteproxy. If not, see <https://www.gnu.org/licenses/>.
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef REMOTEPROXYTESTSTUNNELPROXY_H
#define REMOTEPROXYTESTSTUNNELPROXY_H

#include "basetest.h"

using namespace remoteproxy;

class RemoteProxyTestsTunnelProxy : public BaseTest
{
    Q_OBJECT
public:
    explicit RemoteProxyTestsTunnelProxy(QObject *parent = nullptr);
    ~RemoteProxyTestsTunnelProxy() = default;

private slots:
    // Basic stuff
    void startStopServer();

    // Raw api calls
    void getIntrospect();
    void getHello();

    void monitorServer();

    void configuration_data();
    void configuration();

    void serverPortBlocked();

    void websocketBinaryData();
    void websocketPing();

    void apiBasicCalls_data();
    void apiBasicCalls();

    void apiBasicCallsTcp_data();
    void apiBasicCallsTcp();

    void registerServer_data();
    void registerServer();

    void registerClient_data();
    void registerClient();

    void testSlip_data();
    void testSlip();

    void registerServerDuplicated();
    void registerClientDuplicated();
    void crossRegisterServerClient();

    // Client classes
    void testTunnelProxyServer();
    void testTunnelProxyClient();
    void testTunnelProxyServerSocketDisconnect();

    void tunnelProxyEndToEndTest();


};

#endif // REMOTEPROXYTESTSTUNNELPROXY_H
