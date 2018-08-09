#ifndef AUTHENTICATOR_H
#define AUTHENTICATOR_H

#include <QUuid>
#include <QObject>

namespace remoteproxy {

class ProxyClient;
class AuthenticationReply;

class Authenticator : public QObject
{
    Q_OBJECT
public:
    enum AuthenticationError {
        AuthenticationErrorNoError,
        AuthenticationErrorUnknown,
        AuthenticationErrorTimeout,
        AuthenticationErrorAborted,
        AuthenticationErrorAuthenticationFailed,
        AuthenticationErrorAuthenticationServerNotResponding
    };
    Q_ENUM(AuthenticationError)

    explicit Authenticator(QObject *parent = nullptr);
    virtual ~Authenticator() = 0;

    virtual QString name() const = 0;

public slots:
    virtual AuthenticationReply *authenticate(ProxyClient *proxyClient) = 0;

protected:
    void setReplyError(AuthenticationReply *reply, AuthenticationError error);
    void setReplyFinished(AuthenticationReply *reply);

    AuthenticationReply *createAuthenticationReply(ProxyClient *proxyClient, QObject *parent = nullptr);
};

}

#endif // AUTHENTICATOR_H
