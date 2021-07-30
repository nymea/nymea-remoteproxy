#ifndef TUNNELPROXYREMOTECONNECTION_H
#define TUNNELPROXYREMOTECONNECTION_H

#include <QObject>

class TunnelProxyRemoteConnection : public QObject
{
    Q_OBJECT
public:
    explicit TunnelProxyRemoteConnection(QObject *parent = nullptr);

signals:

};

#endif // TUNNELPROXYREMOTECONNECTION_H
