#ifndef AUTHENTICATOR_H
#define AUTHENTICATOR_H

#include <QUuid>
#include <QObject>

class AuthenticationReply;

class Authenticator : public QObject
{
    Q_OBJECT
public:
    enum AuthenticationError {
        AuthenticationErrorNoError,
        AuthenticationErrorTimeout,
        AuthenticationErrorAborted,
        AuthenticationErrorAuthenticationFailed,
        AuthenticationErrorAuthenticationServerNotResponding
    };
    Q_ENUM(AuthenticationError)

    explicit Authenticator(QObject *parent = nullptr);
    virtual ~Authenticator() = 0;

public slots:
    virtual AuthenticationReply *authenticate(const QUuid &clientId, const QString &token) = 0;
};

#endif // AUTHENTICATOR_H
