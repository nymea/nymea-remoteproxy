#include "proxyclient.h"


ProxyClient::ProxyClient(QObject *parent) :
    QObject(parent)
{
    m_connection = new RemoteProxyConnection(QUuid::createUuid(), "nymea-remoteproxy-client", this);
    m_connection->setInsecureConnection(true);
    connect(m_connection, &RemoteProxyConnection::ready, this, &ProxyClient::onClientReady);
    connect(m_connection, &RemoteProxyConnection::authenticated, this, &ProxyClient::onAuthenticationFinished);
    connect(m_connection, &RemoteProxyConnection::errorOccured, this, &ProxyClient::onErrorOccured);
    connect(m_connection, &RemoteProxyConnection::disconnected, this, &ProxyClient::onClientDisconnected);

}

void ProxyClient::setHostAddress(const QHostAddress &hostAddress)
{
    m_hostAddress = hostAddress;
}

void ProxyClient::setPort(int port)
{
    m_port = port;
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

void ProxyClient::onClientDisconnected()
{
    qDebug() << "Disconnected from" << m_connection;
    exit(1);
}

void ProxyClient::start(const QString &token)
{
    m_token = token;
    m_connection->connectServer(m_hostAddress, static_cast<quint16>(m_port));
}
