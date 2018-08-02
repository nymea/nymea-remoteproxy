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
    qCDebug(dcEngine()) << "Start server engine";
    QUrl proxyUrl;
    proxyUrl.setScheme("wss");
    proxyUrl.setHost("0.0.0.0");
    proxyUrl.setPort(static_cast<int>(m_port));
    qCDebug(dcApplication()) << "Authentication server"  << m_authenticationServerUrl.toString();
    qCDebug(dcApplication()) << "Start server"  << proxyUrl.toString();

    // TODO: init stuff

    setRunning(true);
}

void Engine::stop()
{
    qCDebug(dcEngine()) << "Stop server engine";

    // TODO: deinit stuff

    setRunning(false);
}

bool Engine::running() const
{
    return m_running;
}

void Engine::setHost(const QHostAddress &hostAddress)
{
    m_hostAddress = hostAddress;
}

void Engine::setAuthenticationServerUrl(const QUrl &url)
{
    m_authenticationServerUrl = url;
}

void Engine::setPort(const quint16 &port)
{
    m_port = port;
}

void Engine::setSslConfiguration(const QSslConfiguration &configuration)
{
    m_sslConfiguration = configuration;
}

Engine::Engine(QObject *parent) :
    QObject(parent)
{

}

void Engine::setRunning(bool running)
{
    if (m_running == running)
        return;

    qCDebug(dcEngine()) << "Engine is" << (running ? "now running." : "not running any more.");
    m_running = running;
    emit runningChanged(m_running);
}
