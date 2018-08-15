#include "monitorserver.h"
#include "loggingcategories.h"

#include <QJsonDocument>

namespace remoteproxy {

MonitorServer::MonitorServer(const QString &serverName, QObject *parent) :
    QObject(parent),
    m_serverName(serverName)
{
    m_timer = new QTimer(this);
    m_timer->setInterval(1000);
    m_timer->setSingleShot(false);
    connect(m_timer, &QTimer::timeout, this, &MonitorServer::onTimeout);
}

QVariantMap MonitorServer::createMonitorData()
{
    QVariantMap monitorData;

    return monitorData;
}

void MonitorServer::sendMonitorData(QLocalSocket *clientConnection, const QVariantMap &dataMap)
{
    clientConnection->write(QJsonDocument::fromVariant(dataMap).toJson(QJsonDocument::Compact) + '\n');
    clientConnection->flush();
}

void MonitorServer::onTimeout()
{
    // If no client left or no server running, stop the timer
    if (m_clients.isEmpty() || !m_server)
        m_timer->stop();

    QVariantMap dataMap = createMonitorData();

    // Send each monitor the current data
    foreach (QLocalSocket *clientConnection, m_clients) {
        sendMonitorData(clientConnection, dataMap);
    }
}

void MonitorServer::onMonitorConnected()
{
    QLocalSocket *clientConnection = m_server->nextPendingConnection();
    connect(clientConnection, &QLocalSocket::disconnected, this, &MonitorServer::onMonitorDisconnected);
    connect(clientConnection, &QLocalSocket::readyRead, this, &MonitorServer::onMonitorDisconnected);
    m_clients.append(clientConnection);

    if (!m_timer->isActive()) {
        // Send the data right the way
        onTimeout();
        m_timer->start();
    }
}

void MonitorServer::onMonitorDisconnected()
{
    qCDebug(dcMonitorServer()) << "Monitor disconnected.";
    QLocalSocket *clientConnection = static_cast<QLocalSocket *>(sender());
    m_clients.removeAll(clientConnection);
    clientConnection->deleteLater();
}

void MonitorServer::startServer()
{
    m_server = new QLocalServer(this);
    if (!m_server->listen(m_serverName)) {
        qCWarning(dcMonitorServer()) << "Could not start local server for monitor on" << m_serverName;
        delete m_server;
        m_server = nullptr;
        return;
    }

    connect(m_server, &QLocalServer::newConnection, this, &MonitorServer::onMonitorConnected);
    qCDebug(dcMonitorServer()) << "Started successfully on" << m_serverName;
}

void MonitorServer::stopServer()
{
    if (!m_server)
        return;

    qCDebug(dcMonitorServer()) << "Stopping server" << m_serverName;
    foreach (QLocalSocket *clientConnection, m_clients) {
        clientConnection->close();
    }

    m_server->close();
    m_server->deleteLater();
    m_server = nullptr;
}

}
