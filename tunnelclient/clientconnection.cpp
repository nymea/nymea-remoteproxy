#include "clientconnection.h"

ClientConnection::ClientConnection(const QUrl &serverUrl, const QString &name, const QUuid &uuid, const QUuid &serverUuid, bool insecure, QObject *parent) :
    QObject(parent),
    m_serverUrl(serverUrl),
    m_name(name),
    m_uuid(uuid),
    m_serverUuid(serverUuid),
    m_insecure(insecure)
{

    m_remoteConnection = new TunnelProxyRemoteConnection(m_uuid, m_name);

    connect(m_remoteConnection, &TunnelProxyRemoteConnection::stateChanged, this, [this](TunnelProxyRemoteConnection::State state){
        qDebug() << state;
        switch (state) {
        case TunnelProxyRemoteConnection::StateRegister:
            qDebug() << "Connected with" << m_remoteConnection->remoteProxyServer() << m_remoteConnection->remoteProxyServerName() << m_remoteConnection->remoteProxyServerVersion() << m_remoteConnection->remoteProxyApiVersion();
            break;
        default:
            break;
        }
    });

    connect(m_remoteConnection, &TunnelProxyRemoteConnection::remoteConnectedChanged, this, [](bool remoteConnected){
        qDebug() << "Remote connection" << (remoteConnected ? "established successfully" : "disconnected");
    });

    connect(m_remoteConnection, &TunnelProxyRemoteConnection::dataReady, this, [](const QByteArray &data){
        qDebug() << "Data received" << data;
    });

    connect(m_remoteConnection, &TunnelProxyRemoteConnection::errorOccured, this, [](QAbstractSocket::SocketError error){
        qWarning() << "Socket error occured" << error;
    });

    connect(m_remoteConnection, &TunnelProxyRemoteConnection::sslErrors, this, [=](const QList<QSslError> &errors){
        if (m_insecure) {
            m_remoteConnection->ignoreSslErrors(errors);
        } else {
            qWarning() << "SSL errors occured:";
            foreach (const QSslError &sslError, errors) {
                qWarning() << "  --> " << sslError.errorString();
            }
        }
    });
}

void ClientConnection::connectToServer()
{
    m_remoteConnection->connectServer(m_serverUrl, m_serverUuid);
}
