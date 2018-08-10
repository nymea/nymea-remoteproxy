#include "proxyconnection.h"

namespace remoteproxyclient {

ProxyConnection::ProxyConnection(QObject *parent) : QObject(parent)
{

}

ProxyConnection::~ProxyConnection()
{

}

bool ProxyConnection::allowSslErrors() const
{
    return m_allowSslErrors;
}

void ProxyConnection::setAllowSslErrors(bool allowSslErrors)
{
    m_allowSslErrors = allowSslErrors;
}

}
