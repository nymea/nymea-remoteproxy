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

#ifndef ENGINE_H
#define ENGINE_H

#include <QUrl>
#include <QObject>
#include <QHostAddress>
#include <QSslConfiguration>

#include "proxyserver.h"
#include "monitorserver.h"
#include "websocketserver.h"
#include "proxyconfiguration.h"
#include "authentication/authenticator.h"

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
    bool developerMode() const;

    QString serverName() const;

    void setAuthenticator(Authenticator *authenticator);
    void setDeveloperModeEnabled(bool enabled);

    ProxyConfiguration *configuration() const;
    Authenticator *authenticator() const;
    ProxyServer *proxyServer() const;
    WebSocketServer *webSocketServer() const;
    MonitorServer *monitorServer() const;

private:
    explicit Engine(QObject *parent = nullptr);
    ~Engine();
    static Engine *s_instance;

    bool m_running = false;
    bool m_developerMode = false;

    ProxyConfiguration *m_configuration = nullptr;
    Authenticator *m_authenticator = nullptr;
    ProxyServer *m_proxyServer = nullptr;
    WebSocketServer *m_webSocketServer = nullptr;
    MonitorServer *m_monitorServer = nullptr;

signals:
    void runningChanged(bool running);

private slots:
    void clean();
    void setRunning(bool running);

};

}

#endif // ENGINE_H
