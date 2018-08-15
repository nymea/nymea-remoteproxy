#ifndef MOCKAUTHENTICATOR_H
#define MOCKAUTHENTICATOR_H

#include <QTimer>
#include <QObject>

#include "authentication/authenticator.h"

using namespace remoteproxy;

class MockAuthenticationReply : public QObject
{
    Q_OBJECT
public:
    explicit MockAuthenticationReply(int timeout, Authenticator::AuthenticationError error, AuthenticationReply *authenticationReply, QObject *parent = nullptr);

    AuthenticationReply *authenticationReply() const { return m_authenticationReply; }
    Authenticator::AuthenticationError error() const { return m_error; }

private:
    Authenticator::AuthenticationError m_error;
    AuthenticationReply *m_authenticationReply;

signals:
    void finished();

};


class MockAuthenticator : public Authenticator
{
    Q_OBJECT
public:
    explicit MockAuthenticator(QObject *parent = nullptr);

    QString name() const override;

    void setTimeoutDuration(int timeout);
    void setExpectedAuthenticationError(Authenticator::AuthenticationError error = AuthenticationErrorNoError);

private:
    int m_timeoutDuration = 1000;
    Authenticator::AuthenticationError m_expectedError;

private slots:
    void replyFinished();

public slots:
    AuthenticationReply *authenticate(ProxyClient *proxyClient) override;
};

#endif // MOCKAUTHENTICATOR_H
