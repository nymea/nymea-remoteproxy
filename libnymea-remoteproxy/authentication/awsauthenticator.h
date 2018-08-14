#ifndef AWSAUTHENTICATOR_H
#define AWSAUTHENTICATOR_H

#include <QObject>
#include <QNetworkAccessManager>

#include "authenticator.h"
#include "authenticationreply.h"
#include "authenticationprocess.h"

namespace remoteproxy {

class AwsAuthenticator : public Authenticator
{
    Q_OBJECT
public:
    explicit AwsAuthenticator(QObject *parent = nullptr);
    ~AwsAuthenticator() override;

    QString name() const override;

private:
    QNetworkAccessManager *m_manager = nullptr;
    QHash<AuthenticationProcess *, AuthenticationReply *> m_runningProcesses;

private slots:
    void onAuthenticationProcessFinished(Authenticator::AuthenticationError error);

public slots:
    AuthenticationReply *authenticate(ProxyClient *proxyClient) override;

};

}

#endif // AWSAUTHENTICATOR_H
