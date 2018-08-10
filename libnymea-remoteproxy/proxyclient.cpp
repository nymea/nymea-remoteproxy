#include "proxyclient.h"

namespace remoteproxy {

ProxyClient::ProxyClient(TransportInterface *interface, const QUuid &clientId, QObject *parent) :
    QObject(parent),
    m_interface(interface),
    m_clientId(clientId)
{

}

QUuid ProxyClient::clientId() const
{
    return m_clientId;
}

bool ProxyClient::isAuthenticated() const
{
    return m_authenticated;
}

void ProxyClient::setAuthenticated(bool isAuthenticated)
{
    // TODO: start the timeout counter and disconnect if no tunnel established
    m_authenticated = isAuthenticated;
    if (m_authenticated){
        emit authenticated();
    }
}

bool ProxyClient::isTunnelConnected() const
{
    return m_tunnelConnected;
}

void ProxyClient::setTunnelConnected(bool isTunnelConnected)
{
    // TODO: reset the timeout counter and disconnect if no tunnel established
    m_tunnelConnected = isTunnelConnected;
    if (m_tunnelConnected){
        emit tunnelConnected();
    }
}

TransportInterface *ProxyClient::interface() const
{
    return m_interface;
}

QString ProxyClient::uuid() const
{
    return m_uuid;
}

void ProxyClient::setUuid(const QString &uuid)
{
    m_uuid = uuid;
}

QString ProxyClient::name() const
{
    return m_name;
}

void ProxyClient::setName(const QString &name)
{
    m_name = name;
}

QString ProxyClient::token() const
{
    return m_token;
}

void ProxyClient::setToken(const QString &token)
{
    m_token = token;
}

QDebug operator<<(QDebug debug, ProxyClient *proxyClient)
{
    debug.nospace() << "ProxyClient(" << proxyClient->interface()->serverName();
    debug.nospace() << ", " << proxyClient->clientId().toString() << ") ";
    return debug;
}

}
