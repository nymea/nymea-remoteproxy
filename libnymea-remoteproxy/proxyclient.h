#ifndef PROXYCLIENT_H
#define PROXYCLIENT_H

#include <QUuid>
#include <QObject>

class ProxyClient : public QObject
{
    Q_OBJECT
public:
    explicit ProxyClient(const QUuid &clientId, QObject *parent = nullptr);

    QUuid clientId() const;

    bool authenticated() const;
    bool tunnelConnected() const;

private:
    QUuid m_clientId;
    bool m_authenticated = false;
    bool m_tunnelConnected = false;


};

#endif // PROXYCLIENT_H
