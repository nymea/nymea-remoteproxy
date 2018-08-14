#include "proxyclient.h"


ProxyClient::ProxyClient(QObject *parent) :
    QObject(parent)
{
    m_connection = new RemoteProxyConnection(QUuid::createUuid(), "nymea-remoteproxy-client", RemoteProxyConnection::ConnectionTypeWebSocket, this);
    m_connection->setInsecureConnection(true);
    connect(m_connection, &RemoteProxyConnection::ready, this, &ProxyClient::onClientReady);
    connect(m_connection, &RemoteProxyConnection::authenticated, this, &ProxyClient::onAuthenticationFinished);
    connect(m_connection, &RemoteProxyConnection::errorOccured, this, &ProxyClient::onErrorOccured);

}

void ProxyClient::onErrorOccured(RemoteProxyConnection::Error error)
{
    qDebug() << "Error occured" << error << m_connection->errorString();
}

void ProxyClient::onClientReady()
{
    m_connection->authenticate(m_token);
}

void ProxyClient::onAuthenticationFinished()
{
    qDebug() << "Authentication finished.";
}

void ProxyClient::start(const QString &token)
{
    m_token = token;
    m_connection->connectServer(m_hostAddress, static_cast<quint16>(m_port));
}
