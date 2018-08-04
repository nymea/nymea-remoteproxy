#include "proxyclient.h"

ProxyClient::ProxyClient(const QUuid &clientId, QObject *parent) :
    QObject(parent),
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

bool ProxyClient::tunnelConnected() const
{
    return m_tunnelConnected;
}
