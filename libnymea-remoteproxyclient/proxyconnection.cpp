#include "proxyconnection.h"

namespace remoteproxyclient {

ProxyConnection::ProxyConnection(QObject *parent) : QObject(parent)
{

}

bool ProxyConnection::connected()
{
    return m_connected;
}

void ProxyConnection::setConnected(bool connected)
{
    if (m_connected == connected)
        return;

    m_connected = connected;
    emit connectedChanged(m_connected);
}

ProxyConnection::~ProxyConnection()
{

}

}
