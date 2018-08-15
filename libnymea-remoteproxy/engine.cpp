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
    // Make sure an authenticator was registered
    Q_ASSERT_X(m_authenticator != nullptr, "Engine", "There is no authenticator registerd.");
    Q_ASSERT_X(m_configuration != nullptr, "Engine", "There is no configuration set.");

    if (!m_running)
        qCDebug(dcEngine()) << "Start server engine";

    // Clean up
    clean();

    m_configuration = configuration;
    m_proxyServer = new ProxyServer(this);
    m_webSocketServer = new WebSocketServer(m_sslConfiguration, this);

    QUrl websocketServerUrl;
    websocketServerUrl.setScheme("wss");
    websocketServerUrl.setHost(m_configuration->webSocketServerHost().toString());
    websocketServerUrl.setPort(m_configuration->webSocketServerPort());
    qDebug() << "WSS url is:" << websocketServerUrl;
    m_webSocketServer->setServerUrl(websocketServerUrl);

    m_proxyServer->registerTransportInterface(m_webSocketServer);

    qCDebug(dcEngine()) << "Starting proxy server";
    m_proxyServer->startServer();

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
    return m_serverName;
}

void Engine::setServerName(const QString &serverName)
{
    m_serverName = serverName;
}

void Engine::setConfiguration(ProxyConfiguration *configuration)
{
    m_configuration = configuration;
    qCDebug(dcApplication()) << "Set configuration" << m_configuration;
}

void Engine::setSslConfiguration(const QSslConfiguration &configuration)
{
    m_sslConfiguration = configuration;

    qCDebug(dcEngine()) << "SSL certificate information:";
    qCDebug(dcEngine()) << "    Common name:" << m_sslConfiguration.localCertificate().issuerInfo(QSslCertificate::CommonName);
    qCDebug(dcEngine()) << "    Organisation:" << m_sslConfiguration.localCertificate().issuerInfo(QSslCertificate::Organization);
    qCDebug(dcEngine()) << "    Organisation unit name:" << m_sslConfiguration.localCertificate().issuerInfo(QSslCertificate::OrganizationalUnitName);
    qCDebug(dcEngine()) << "    Country name:" << m_sslConfiguration.localCertificate().issuerInfo(QSslCertificate::CountryName);
    qCDebug(dcEngine()) << "    Locality name:" << m_sslConfiguration.localCertificate().issuerInfo(QSslCertificate::LocalityName);
    qCDebug(dcEngine()) << "    State/Province:" << m_sslConfiguration.localCertificate().issuerInfo(QSslCertificate::StateOrProvinceName);
    qCDebug(dcEngine()) << "    Email address:" << m_sslConfiguration.localCertificate().issuerInfo(QSslCertificate::EmailAddress);
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
