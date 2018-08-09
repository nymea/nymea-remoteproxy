#ifndef AWSAUTHENTICATOR_H
#define AWSAUTHENTICATOR_H

#include <QObject>

#include "authenticator.h"
#include "authenticationreply.h"

namespace remoteproxy {

class AwsAuthenticator : public Authenticator
{
    Q_OBJECT
public:
    explicit AwsAuthenticator(QObject *parent = nullptr);
    ~AwsAuthenticator() override = default;

    QString name() const override;

public slots:
    AuthenticationReply *authenticate(ProxyClient *proxyClient) override;

};

}

#endif // AWSAUTHENTICATOR_H
