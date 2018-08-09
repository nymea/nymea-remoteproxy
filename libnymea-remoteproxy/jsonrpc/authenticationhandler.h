#ifndef AUTHENTICATIONHANDLER_H
#define AUTHENTICATIONHANDLER_H

#include <QObject>

#include "jsonhandler.h"
#include "authentication/authenticationreply.h"

namespace remoteproxy {

class AuthenticationHandler : public JsonHandler
{
    Q_OBJECT
public:
    explicit AuthenticationHandler(QObject *parent = nullptr);
    ~AuthenticationHandler() override = default;

    QString name() const override;

    Q_INVOKABLE JsonReply *Authenticate(const QVariantMap &params, ProxyClient *proxyClient);

private:
    QHash<AuthenticationReply *, JsonReply *> m_runningAuthentications;

private slots:
    void onAuthenticationFinished();


};

}

#endif // AUTHENTICATIONHANDLER_H
