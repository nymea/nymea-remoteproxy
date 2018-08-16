#ifndef REMOTEPROXYSERVERAPPLICATION_H
#define REMOTEPROXYSERVERAPPLICATION_H

#include <QObject>
#include <QSocketNotifier>
#include <QCoreApplication>

class RemoteProxyServerApplication : public QCoreApplication
{
    Q_OBJECT
public:
    explicit RemoteProxyServerApplication(int &argc, char **argv);

signals:

public slots:
};

#endif // REMOTEPROXYSERVERAPPLICATION_H
