#ifndef AWSAUTHENTICATOR_H
#define AWSAUTHENTICATOR_H

#include <QObject>

#include "authenticator.h"
#include "authenticationreply.h"

class AwsAuthenticator : public Authenticator
{
    Q_OBJECT
public:
    explicit AwsAuthenticator(QObject *parent = nullptr);
    ~AwsAuthenticator() override = default;

public slots:
    AuthenticationReply *authenticate(const QUuid &clientId, const QString &token) override;

};

#endif // AWSAUTHENTICATOR_H
