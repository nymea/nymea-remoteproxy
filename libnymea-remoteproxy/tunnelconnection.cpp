#include "tunnelconnection.h"

namespace remoteproxy {

TunnelConnection::TunnelConnection(ProxyClient *clientOne, ProxyClient *clientTwo):
    m_clientOne(clientOne),
    m_clientTwo(clientTwo)
{

}

QString TunnelConnection::token() const
{
    if (!isValid())
        return QString();

    return m_clientOne->token();
}

ProxyClient *TunnelConnection::clientOne() const
{
    return m_clientOne;
}

ProxyClient *TunnelConnection::clientTwo() const
{
    return m_clientTwo;
}

bool TunnelConnection::isValid() const
{
    // Both clients have to be valid
    if (!m_clientOne || !m_clientTwo)
        return false;

    // Both clients need the same token
    if (m_clientOne->token() != m_clientTwo->token())
        return false;

    // The clients need to be different
    if (m_clientOne == m_clientTwo)
        return false;

    return true;
}

}
