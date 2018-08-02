#ifndef REMOTEPROXYCONNECTOR_H
#define REMOTEPROXYCONNECTOR_H

#include <QObject>

class RemoteProxyConnector : public QObject
{
    Q_OBJECT
public:
    explicit RemoteProxyConnector(QObject *parent = nullptr);

signals:

public slots:

};

#endif // REMOTEPROXYCONNECTOR_H
