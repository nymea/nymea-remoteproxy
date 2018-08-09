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

void Engine::start()
{
    if (!m_running)
        qCDebug(dcEngine()) << "Start server engine";

    // Init proxy server
    if (m_proxyServer) {
        delete m_proxyServer;
        m_proxyServer = nullptr;
    }

    m_proxyServer = new ProxyServer(this);

    // Init WebSocketServer
    if (m_webSocketServer) {
        delete m_webSocketServer;
        m_webSocketServer = nullptr;
    }

    QUrl websocketServerUrl;
    websocketServerUrl.setScheme("wss");
    websocketServerUrl.setHost(m_webSocketServerHostAddress.toString());
    websocketServerUrl.setPort(m_webSocketServerPort);

    m_webSocketServer = new WebSocketServer(m_sslConfiguration, this);
    m_webSocketServer->setServerUrl(websocketServerUrl);

    m_proxyServer->registerTransportInterface(m_webSocketServer);

    // Make sure an authenticator was registered
    Q_ASSERT_X(m_authenticator != nullptr, "Engine", "There is no authenticator registerd.");

    qCDebug(dcEngine()) << "Starting proxy server";
    m_proxyServer->startServer();

    setRunning(true);
}

void Engine::stop()
{
    if (m_running)
        qCDebug(dcEngine()) << "Stop server engine";

    if (m_proxyServer) {
        m_proxyServer->stopServer();
        delete m_proxyServer;
        m_proxyServer = nullptr;
    }

    if (m_webSocketServer) {
        delete m_webSocketServer;
        m_webSocketServer = nullptr;
    }

    setRunning(false);
}

bool Engine::running() const
{
    return m_running;
}

QString Engine::serverName() const
{
    return m_serverName;
}

void Engine::setServerName(const QString &serverName)
{
    m_serverName = serverName;
}

void Engine::setWebSocketServerHostAddress(const QHostAddress &hostAddress)
{
    qCDebug(dcEngine()) << "Websocket server host address:" << hostAddress;
    m_webSocketServerHostAddress = hostAddress;
}

void Engine::setWebSocketServerPort(const quint16 &port)
{
    qCDebug(dcEngine()) << "Websocket server port:" << port;
    m_webSocketServerPort = port;
}

void Engine::setSslConfiguration(const QSslConfiguration &configuration)
{
    qCDebug(dcEngine()) << "SSL certificate information:";
    qCDebug(dcEngine()) << "    Common name:" << configuration.localCertificate().issuerInfo(QSslCertificate::CommonName);
    qCDebug(dcEngine()) << "    Organisation:" << configuration.localCertificate().issuerInfo(QSslCertificate::Organization);
    qCDebug(dcEngine()) << "    Organisation unit name:" << configuration.localCertificate().issuerInfo(QSslCertificate::OrganizationalUnitName);
    qCDebug(dcEngine()) << "    Country name:" << configuration.localCertificate().issuerInfo(QSslCertificate::CountryName);
    qCDebug(dcEngine()) << "    Locality name:" << configuration.localCertificate().issuerInfo(QSslCertificate::LocalityName);
    qCDebug(dcEngine()) << "    State/Province:" << configuration.localCertificate().issuerInfo(QSslCertificate::StateOrProvinceName);
    qCDebug(dcEngine()) << "    Email address:" << configuration.localCertificate().issuerInfo(QSslCertificate::EmailAddress);

    m_sslConfiguration = configuration;
}

void Engine::setAuthenticationServerUrl(const QUrl &url)
{
    qCDebug(dcEngine()) << "Authentication server URL" << url.toString();
    m_authenticationServerUrl = url;
}

void Engine::setAuthenticator(Authenticator *authenticator)
{
    if (m_authenticator == authenticator)
        return;

    if (m_authenticator) {
        qCDebug(dcEngine()) << "There is already an authenticator registered. Unregister default authenticator";
        m_authenticator = nullptr;
        // FIXME: do unconnect
    }

    m_authenticator = authenticator;

    // FIXME: connect
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

void Engine::setRunning(bool running)
{
    if (m_running == running)
        return;

    qCDebug(dcEngine()) << "Engine is" << (running ? "now running." : "not running any more.");
    m_running = running;
    emit runningChanged(m_running);
}

}
