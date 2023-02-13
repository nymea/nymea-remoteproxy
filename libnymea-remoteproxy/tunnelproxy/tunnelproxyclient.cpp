#include "tunnelproxyclient.h"
#include "loggingcategories.h"
#include "server/transportinterface.h"
#include "../engine.h"
#include "../common/slipdataprocessor.h"

namespace remoteproxy {

TunnelProxyClient::TunnelProxyClient(TransportInterface *interface, const QUuid &clientId, const QHostAddress &address, QObject *parent) :
    TransportClient(interface, clientId, address, parent)
{
    // Note: a client is not inactive any more once registered successfully as client or server.
    // This makes sure we have not any inactive sockets connected to the proxy blocking resources.
    // The tunnelproxy server will call makeClientActive once registered successfuly to stop this timer.
    m_inactiveTimer.setInterval(Engine::instance()->configuration()->inactiveTimeout());
    connect(&m_inactiveTimer, &QTimer::timeout, this, [this](){
        m_interface->killClientConnection(m_clientId, "Tunnelproxy timeout occurred. The socket was inactive.");
    });
    m_inactiveTimer.setSingleShot(true);
    m_inactiveTimer.start();
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

QList<QByteArray> TunnelProxyClient::processData(const QByteArray &data)
{
    QList<QByteArray> packets;

    // Parse packets depending on the encoded
    if (m_slipEnabled) {
        // Read each byte until we get END byte, then unescape the packet
        for (int i = 0; i < data.length(); i++) {
            quint8 byte = static_cast<quint8>(data.at(i));
            if (byte == SlipDataProcessor::ProtocolByteEnd) {
                // If there is no data...continue since it might be a starting END byte
                if (m_dataBuffer.isEmpty())
                    continue;

                QByteArray frame = SlipDataProcessor::deserializeData(m_dataBuffer);
                if (frame.isNull()) {
                    qCWarning(dcTunnelProxyServerTraffic()) << "Received inconsistant SLIP encoded message. Ignoring data...";
                } else {
                    qCDebug(dcTunnelProxyServerTraffic()) << "Frame received";
                    packets.append(m_dataBuffer);
                    m_dataBuffer.clear();
                }
            } else {
                m_dataBuffer.append(data.at(i));
            }
        }
    } else {
        // Handle json packet fragmentation
        m_dataBuffer.append(data);
        int splitIndex = m_dataBuffer.indexOf("}\n{");
        while (splitIndex > -1) {
            packets.append(m_dataBuffer.left(splitIndex + 1));
            m_dataBuffer = m_dataBuffer.right(m_dataBuffer.length() - splitIndex - 2);
            splitIndex = m_dataBuffer.indexOf("}\n{");
        }
        if (m_dataBuffer.endsWith("}\n") || m_dataBuffer.endsWith("}")) {
            packets.append(m_dataBuffer);
            m_dataBuffer.clear();
        }
    }

    return packets;
}

void TunnelProxyClient::makeClientActive()
{
    m_inactiveTimer.stop();
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
