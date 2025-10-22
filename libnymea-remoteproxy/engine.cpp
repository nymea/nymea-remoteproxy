/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
*  Copyright 2013 - 2023, nymea GmbH
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

#include "engine.h"
#include "loggingcategories.h"
#include "version.h"

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
    qCDebug(dcEngine()) << "Using configuration" << m_configuration;


    // Tunnel proxy
    // -------------------------------------
    m_tunnelProxyServer = new TunnelProxyServer(this);
    m_webSocketServerTunnelProxy = new WebSocketServer(m_configuration->sslEnabled(), m_configuration->sslConfiguration(), this);
    m_tcpSocketServerTunnelProxy = new TcpSocketServer(m_configuration->sslEnabled(), m_configuration->sslConfiguration(), this);
    m_unixSocketServerTunnelProxy = new UnixSocketServer(m_configuration->unixSocketFileName(), this);

    // Configure websocket server
    QUrl websocketServerTunnelProxyUrl;
    websocketServerTunnelProxyUrl.setScheme(m_configuration->sslEnabled() ? "wss" : "ws");
    websocketServerTunnelProxyUrl.setHost(m_configuration->webSocketServerTunnelProxyHost().toString());
    websocketServerTunnelProxyUrl.setPort(m_configuration->webSocketServerTunnelProxyPort());
    m_webSocketServerTunnelProxy->setServerUrl(websocketServerTunnelProxyUrl);

    // Configure tcp socket server
    QUrl tcpSocketServerTunnelProxyUrl;
    tcpSocketServerTunnelProxyUrl.setScheme(m_configuration->sslEnabled() ? "ssl" : "tcp");
    tcpSocketServerTunnelProxyUrl.setHost(m_configuration->tcpServerTunnelProxyHost().toString());
    tcpSocketServerTunnelProxyUrl.setPort(m_configuration->tcpServerTunnelProxyPort());
    m_tcpSocketServerTunnelProxy->setServerUrl(tcpSocketServerTunnelProxyUrl);

    // Register the transport interfaces in the proxy server
    m_tunnelProxyServer->registerTransportInterface(m_webSocketServerTunnelProxy);
    m_tunnelProxyServer->registerTransportInterface(m_tcpSocketServerTunnelProxy);
    m_tunnelProxyServer->registerTransportInterface(m_unixSocketServerTunnelProxy);

    // Start the server
    qCDebug(dcEngine()) << "Starting the tunnel proxy manager...";
    m_tunnelProxyServer->startServer();

    // Monitor server
    // -------------------------------------
    m_monitorServer = new MonitorServer(configuration->monitorSocketFileName(), this);
    m_monitorServer->startServer();

    if (configuration->logEngineEnabled())
        m_logEngine->enable();

    // Set running true in the next event loop
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

QString Engine::serverName() const
{
    return m_configuration->serverName();
}

ProxyConfiguration *Engine::configuration() const
{
    return m_configuration;
}

TunnelProxyServer *Engine::tunnelProxyServer() const
{
    return m_tunnelProxyServer;
}

UnixSocketServer *Engine::unixSocketServerTunnelProxy() const
{
    return m_unixSocketServerTunnelProxy;
}

TcpSocketServer *Engine::tcpSocketServerTunnelProxy() const
{
    return m_tcpSocketServerTunnelProxy;
}

WebSocketServer *Engine::webSocketServerTunnelProxy() const
{
    return m_webSocketServerTunnelProxy;
}

MonitorServer *Engine::monitorServer() const
{
    return m_monitorServer;
}

LogEngine *Engine::logEngine() const
{
    return m_logEngine;
}

QVariantMap Engine::buildMonitorData(bool printAll)
{
    QVariantMap monitorData;
    monitorData.insert("serverName", m_configuration->serverName());
    monitorData.insert("serverVersion", SERVER_VERSION_STRING);
    monitorData.insert("apiVersion", API_VERSION_STRING);
    monitorData.insert("tunnelProxyStatistic", tunnelProxyServer()->currentStatistics(printAll));
    return monitorData;
}

Engine::Engine(QObject *parent) :
    QObject(parent)
{
    m_lastTimeStamp = QDateTime::currentDateTimeUtc().toMSecsSinceEpoch();

    m_timer = new QTimer(this);
    m_timer->setSingleShot(false);
    m_timer->setInterval(50);

    connect(m_timer, &QTimer::timeout, this, &Engine::onTimerTick);
    m_logEngine = new LogEngine(this);
}

Engine::~Engine()
{
    stop();
}

void Engine::onTimerTick()
{
    qint64 timestamp = QDateTime::currentDateTimeUtc().toMSecsSinceEpoch();
    qint64 deltaTime = timestamp - m_lastTimeStamp;
    m_lastTimeStamp = timestamp;

    m_currentTimeCounter += deltaTime;
    if (m_currentTimeCounter >= 1000) {
        // One second passed, do second tick
        m_currentTimeCounter = 0;
        if (m_tunnelProxyServer) {
            m_tunnelProxyServer->tick();
        }
    }
}

void Engine::clean()
{
    if (m_monitorServer) {
        m_monitorServer->stopServer();
        delete m_monitorServer;
        m_monitorServer = nullptr;
    }

    if (m_tunnelProxyServer) {
        m_tunnelProxyServer->stopServer();
        delete m_tunnelProxyServer;
        m_tunnelProxyServer = nullptr;
    }

    if (m_tcpSocketServerTunnelProxy) {
        delete m_tcpSocketServerTunnelProxy;
        m_tcpSocketServerTunnelProxy = nullptr;
    }

    if (m_webSocketServerTunnelProxy) {
        delete m_webSocketServerTunnelProxy;
        m_webSocketServerTunnelProxy = nullptr;
    }

    if (m_unixSocketServerTunnelProxy) {
        delete m_unixSocketServerTunnelProxy;
        m_unixSocketServerTunnelProxy = nullptr;
    }

    if (m_configuration) {
        delete m_configuration;
        m_configuration = nullptr;
    }
}

void Engine::setRunning(bool running)
{
    if (m_running == running)
        return;

    if (m_tunnelProxyServer)
        m_tunnelProxyServer->setRunning(running);

    qCDebug(dcEngine()) << "Engine is" << (running ? "now running." : "not running any more.");

    if (running) {
        m_timer->start();
    } else {
        m_timer->stop();
    }

    m_running = running;
    emit runningChanged(m_running);
}


}
