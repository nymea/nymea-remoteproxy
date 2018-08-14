#ifndef DUMMYAUTHENTICATOR_H
#define DUMMYAUTHENTICATOR_H

#include <QObject>

#include "proxyclient.h"
#include "authenticator.h"

namespace remoteproxy {

class DummyAuthenticator : public Authenticator
{
    Q_OBJECT
public:
    explicit DummyAuthenticator(QObject *parent = nullptr);
    ~DummyAuthenticator() override = default;

    QString name() const override;

public slots:
    AuthenticationReply *authenticate(ProxyClient *proxyClient) override;

};

}

#endif // DUMMYAUTHENTICATOR_H
