#include "engine.h"
#include "loggingcategories.h"

Engine *Engine::s_instance = nullptr;

Engine *Engine::instance()
{
    if (!s_instance) {
        qCDebug(dcEngine()) << "Create server engine";
        s_instance = new Engine();
    }

    return s_instance;
}

bool Engine::exists()
{
    return s_instance != nullptr;
}

void Engine::destroy()
{
    qCDebug(dcEngine()) << "Destroy server engine";
    if (s_instance) {
        delete s_instance;
    }

    s_instance = nullptr;
}

void Engine::start()
{
    if (!m_running)
        qCDebug(dcEngine()) << "Start server engine";

    qCDebug(dcEngine()) << "Starting websocket server";
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
    m_webSocketServer->startServer();

    setRunning(true);
}

void Engine::stop()
{
    if (m_running)
        qCDebug(dcEngine()) << "Stop server engine";

    if (m_webSocketServer) {
        m_webSocketServer->stopServer();
        m_webSocketServer->deleteLater();
        m_webSocketServer = nullptr;
    }

    setRunning(false);
}

bool Engine::running() const
{
    return m_running;
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
    qCDebug(dcEngine()) << "SSL Configuration:";
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
