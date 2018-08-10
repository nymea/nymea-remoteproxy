#ifndef TUNNELCONNECTION_H
#define TUNNELCONNECTION_H

#include "proxyclient.h"

namespace remoteproxy {

class TunnelConnection
{
public:
    TunnelConnection(ProxyClient *clientOne = nullptr, ProxyClient *clientTwo = nullptr);

    QString token() const;

    ProxyClient *clientOne() const;
    ProxyClient *clientTwo() const;

    bool isValid() const;

private:
    ProxyClient *m_clientOne = nullptr;
    ProxyClient *m_clientTwo = nullptr;

};

}

#endif // TUNNELCONNECTION_H
