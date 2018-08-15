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

    m_monitorServer = new MonitorServer("/tmp/nymea-remoteproxy-monitor.socket", this);
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
        m_monitorServer->startServer();
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

    qCDebug(dcEngine()) << "Engine is" << (running ? "now running." : "not running any more.");
    m_running = running;
    emit runningChanged(m_running);
}


}
