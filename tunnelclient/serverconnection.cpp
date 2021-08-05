#include "serverconnection.h"

ServerConnection::ServerConnection(const QUrl &serverUrl, const QString &name, const QUuid &uuid, bool insecure, QObject *parent) :
    QObject(parent),
    m_serverUrl(serverUrl),
    m_name(name),
    m_uuid(uuid),
    m_insecure(insecure)
{

    m_socketServer = new TunnelProxySocketServer(m_uuid, m_name, this);

    connect(m_socketServer, &TunnelProxySocketServer::clientConnected, this, [=](TunnelProxySocket *tunnelProxySocket){
        qDebug() << "[+] Client connected" << tunnelProxySocket;
    });

    connect(m_socketServer, &TunnelProxySocketServer::clientDisconnected, this, [=](TunnelProxySocket *tunnelProxySocket){
        qDebug() << "[-] Client disconnected" << tunnelProxySocket;
    });

    connect(m_socketServer, &TunnelProxySocketServer::runningChanged, this, [=](bool running){
        qDebug() << "--> Server is" << (running ? "running" : "not running any more");
        if (running) {
            qDebug() << "--> Connected with" << m_socketServer->remoteProxyServer() << m_socketServer->remoteProxyServerName() << m_socketServer->remoteProxyServerVersion() << m_socketServer->remoteProxyApiVersion();
        }
    });

    connect(m_socketServer, &TunnelProxySocketServer::sslErrors, this, [=](const QList<QSslError> &errors){
        if (m_insecure) {
            qDebug() << "SSL errors occured. Ignoring because explicit specified.";
            m_socketServer->ignoreSslErrors();
        } else {
            qWarning() << "SSL errors occured:";
            foreach (const QSslError &sslError, errors) {
                qWarning() << "  --> " << sslError.errorString();
            }
        }
    });

}

void ServerConnection::startServer()
{
    m_socketServer->startServer(m_serverUrl);
}
