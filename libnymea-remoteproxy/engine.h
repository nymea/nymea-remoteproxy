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

#ifndef ENGINE_H
#define ENGINE_H

#include <QUrl>
#include <QTimer>
#include <QObject>
#include <QDateTime>
#include <QHostAddress>
#include <QSslConfiguration>

#include "logengine.h"
#include "proxyconfiguration.h"
#include "server/monitorserver.h"
#include "server/jsonrpcserver.h"
#include "server/tcpsocketserver.h"
#include "server/websocketserver.h"
#include "server/unixsocketserver.h"
#include "tunnelproxy/tunnelproxyserver.h"

namespace remoteproxy {

class Engine : public QObject
{
    Q_OBJECT
public:
    static Engine *instance();
    void destroy();

    static bool exists();

    void start(ProxyConfiguration *configuration);
    void stop();

    bool running() const;

    QString serverName() const;

    ProxyConfiguration *configuration() const;

    TunnelProxyServer *tunnelProxyServer() const;

    UnixSocketServer *unixSocketServerTunnelProxy() const;
    TcpSocketServer *tcpSocketServerTunnelProxy() const;
    WebSocketServer *webSocketServerTunnelProxy() const;

    MonitorServer *monitorServer() const;
    LogEngine *logEngine() const;

    QVariantMap buildMonitorData(bool printAll = false);

private:
    explicit Engine(QObject *parent = nullptr);
    ~Engine();
    static Engine *s_instance;

    QTimer *m_timer = nullptr;
    qint64 m_lastTimeStamp = 0;
    int m_currentTimeCounter = 0;
    qint64 m_runTime = 0;

    bool m_running = false;

    ProxyConfiguration *m_configuration = nullptr;
    TunnelProxyServer *m_tunnelProxyServer = nullptr;

    UnixSocketServer *m_unixSocketServerTunnelProxy = nullptr;
    TcpSocketServer *m_tcpSocketServerTunnelProxy = nullptr;
    WebSocketServer *m_webSocketServerTunnelProxy = nullptr;

    MonitorServer *m_monitorServer = nullptr;
    LogEngine *m_logEngine = nullptr;

signals:
    void runningChanged(bool running);

private slots:
    void onTimerTick();
    void clean();
    void setRunning(bool running);

};

}

#endif // ENGINE_H
