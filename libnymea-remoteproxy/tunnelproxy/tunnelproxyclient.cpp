#include "tunnelproxyclient.h"
#include "server/transportinterface.h"

namespace remoteproxy {

TunnelProxyClient::TunnelProxyClient(TransportInterface *interface, const QUuid &clientId, const QHostAddress &address, QObject *parent) :
    TransportClient(interface, clientId, address, parent)
{

}

TunnelProxyClient::Type TunnelProxyClient::type() const
{
    return m_type;
}

void TunnelProxyClient::setType(Type type)
{
    if (m_type == type)
        return;

    m_type = type;
    emit typeChanged(m_type);
}

bool TunnelProxyClient::slipEnabled() const
{
    return m_slipEnabled;
}

void TunnelProxyClient::setSlipEnabled(bool slipEnabled)
{
    m_slipEnabled = slipEnabled;
}

QList<QByteArray> TunnelProxyClient::processData(const QByteArray &data)
{
    // TODO: unescape if this data is for the json handler
    QList<QByteArray> packages;

    if (m_slipEnabled) {

    } else {
        // Handle json packet fragmentation
        m_dataBuffers.append(data);
        int splitIndex = m_dataBuffers.indexOf("}\n{");
        while (splitIndex > -1) {
            packages.append(m_dataBuffers.left(splitIndex + 1));
            m_dataBuffers = m_dataBuffers.right(m_dataBuffers.length() - splitIndex - 2);
            splitIndex = m_dataBuffers.indexOf("}\n{");
        }
        if (m_dataBuffers.trimmed().endsWith("}")) {
            packages.append(m_dataBuffers);
            m_dataBuffers.clear();
        }
    }

    return packages;
}

QDebug operator<<(QDebug debug, TunnelProxyClient *tunnelProxyClient)
{
    debug.nospace() << "TunnelProxyClient(";
    debug.nospace() << tunnelProxyClient->name() << ", ";
    debug.nospace() << tunnelProxyClient->interface()->serverName()<< ", ";
    debug.nospace() << tunnelProxyClient->clientId().toString()<< ", ";
    debug.nospace() << tunnelProxyClient->peerAddress().toString() << ", ";
    debug.nospace() << tunnelProxyClient->creationTimeString() << ")";
    return debug.space();
}

}
