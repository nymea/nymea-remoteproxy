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

#include "engine.h"
#include "loggingcategories.h"

namespace remoteproxy {

Engine *Engine::s_instance = nullptr;

Engine *Engine::instance()
{
    if (!s_instance) {
        qCDebug(dcEngine()) << "Create server engine";
        s_instance = new Engine();
    }

    return s_instance;
}

void Engine::destroy()
{
    qCDebug(dcEngine()) << "Destroy server engine";
    if (s_instance) {
        delete s_instance;
    }

    s_instance = nullptr;
}

bool Engine::exists()
{
    return s_instance != nullptr;
}

void Engine::start(ProxyConfiguration *configuration)
{
    if (!m_running)
        qCDebug(dcEngine()) << "Start server engine";

    // Clean up
    clean();

    m_configuration = configuration;
    qCDebug(dcApplication()) << "Using configuration" << m_configuration;

    // Make sure an authenticator was registered
    Q_ASSERT_X(m_authenticator != nullptr, "Engine", "There is no authenticator registerd.");

    m_proxyServer = new ProxyServer(this);
    m_webSocketServer = new WebSocketServer(m_configuration->sslConfiguration(), this);

    QUrl websocketServerUrl;
    websocketServerUrl.setScheme("wss");
    websocketServerUrl.setHost(m_configuration->webSocketServerHost().toString());
    websocketServerUrl.setPort(m_configuration->webSocketServerPort());

    m_webSocketServer->setServerUrl(websocketServerUrl);

    m_proxyServer->registerTransportInterface(m_webSocketServer);

    qCDebug(dcEngine()) << "Starting proxy server";
    m_proxyServer->startServer();

    m_monitorServer = new MonitorServer(configuration->monitorSocketFileName(), this);
    m_monitorServer->startServer();

    // Set tunning true in the next event loop
    QMetaObject::invokeMethod(this, QString("setRunning").toLatin1().data(), Qt::QueuedConnection, Q_ARG(bool, true));
}

void Engine::stop()
{
    if (m_running)
        qCDebug(dcEngine()) << "Stop server engine";

    clean();
    setRunning(false);
}

bool Engine::running() const
{
    return m_running;
}

bool Engine::developerMode() const
{
    return m_developerMode;
}

QString Engine::serverName() const
{
    return m_configuration->serverName();
}

void Engine::setAuthenticator(Authenticator *authenticator)
{
    if (m_authenticator == authenticator)
        return;

    if (m_authenticator) {
        qCDebug(dcEngine()) << "There is already an authenticator registered. Unregister default authenticator";
        m_authenticator = nullptr;
    }

    m_authenticator = authenticator;
}

void Engine::setDeveloperModeEnabled(bool enabled)
{
    m_developerMode = enabled;
}

ProxyConfiguration *Engine::configuration() const
{
    return m_configuration;
}

Authenticator *Engine::authenticator() const
{
    return m_authenticator;
}

ProxyServer *Engine::proxyServer() const
{
    return m_proxyServer;
}

WebSocketServer *Engine::webSocketServer() const
{
    return m_webSocketServer;
}

MonitorServer *Engine::monitorServer() const
{
    return m_monitorServer;
}

Engine::Engine(QObject *parent) :
    QObject(parent)
{

}

Engine::~Engine()
{
    stop();
}

void Engine::clean()
{
    if (m_monitorServer) {
        m_monitorServer->stopServer();
        delete m_monitorServer;
        m_monitorServer = nullptr;
    }

    if (m_proxyServer) {
        m_proxyServer->stopServer();
        delete m_proxyServer;
        m_proxyServer = nullptr;
    }

    if (m_webSocketServer) {
        delete m_webSocketServer;
        m_webSocketServer = nullptr;
    }

    if (m_configuration) {
        m_configuration = nullptr;
    }
}


void Engine::setRunning(bool running)
{
    if (m_running == running)
        return;

    //qCDebug(dcEngine()) << "----------------------------------------------------------";
    qCDebug(dcEngine()) << "Engine is" << (running ? "now running." : "not running any more.");
    //qCDebug(dcEngine()) << "----------------------------------------------------------";
    m_running = running;
    emit runningChanged(m_running);
}


}
