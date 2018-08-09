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

bool ProxyClient::authenticated() const
{
    return m_authenticated;
}

void ProxyClient::setAuthenticated(bool authenticated)
{
    if (m_authenticated == authenticated)
        return;

    m_authenticated = authenticated;
    emit authenticatedChanged(m_authenticated);
}

bool ProxyClient::tunnelConnected() const
{
    return m_tunnelConnected;
}

void ProxyClient::setTunnelConnected(bool tunnelConnected)
{
    if (m_tunnelConnected == tunnelConnected)
        return;

    m_tunnelConnected = tunnelConnected;
    emit tunnelConnectedChanged(m_tunnelConnected);
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
    debug.nospace() << ", " << proxyClient->clientId().toString() << ") :" << endl;
    debug.nospace() << "    tunnel: " << proxyClient->tunnelConnected() << endl;
    debug.nospace() << "    authenticated: " << proxyClient->authenticated() << endl;
    if (!proxyClient->name().isEmpty() && !proxyClient->token().isEmpty() && !proxyClient->uuid().isEmpty()) {
        debug.nospace() << "    name: " << proxyClient->name() << endl;
        debug.nospace() << "    uuid: " << proxyClient->uuid() << endl;
        debug.nospace() << "    token: " << proxyClient->token() << endl;

    }
    return debug;
}

}
