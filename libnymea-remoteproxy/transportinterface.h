#ifndef TRANSPORTINTERFACE_H
#define TRANSPORTINTERFACE_H

#include <QObject>

namespace remoteproxy {

class TransportInterface : public QObject
{
    Q_OBJECT
public:
    explicit TransportInterface(QObject *parent = nullptr);
    virtual ~TransportInterface() = 0;

    QString serverName() const;

    virtual void sendData(const QUuid &clientId, const QByteArray &data) = 0;
    virtual void sendData(const QList<QUuid> &clients, const QByteArray &data) = 0;

    virtual void killClientConnection(const QUuid &clientId, const QString &killReason) = 0;

signals:
    void clientConnected(const QUuid &clientId);
    void clientDisconnected(const QUuid &clientId);
    void dataAvailable(const QUuid &clientId, const QByteArray &data);

protected:
    QString m_serverName;

public slots:
    virtual bool startServer() = 0;
    virtual bool stopServer() = 0;

};

}

#endif // TRANSPORTINTERFACE_H
