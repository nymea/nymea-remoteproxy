#ifndef MOCKAUTHENTICATOR_H
#define MOCKAUTHENTICATOR_H

#include <QTimer>
#include <QObject>

#include "authentication/authenticator.h"

class MockAuthenticator : public Authenticator
{
    Q_OBJECT
public:
    explicit MockAuthenticator(QObject *parent = nullptr);

    void setTimeoutDuration(int timeout);
    void setExpectedAuthenticationError(Authenticator::AuthenticationError error);

private:
    QTimer * m_timer = nullptr;
    int m_timeoutDuration = 1000;
    Authenticator::AuthenticationError m_expectedError;

private slots:
    void onTimeout();

public slots:
    AuthenticationReply *authenticate(const QUuid &clientId, const QString &token) override;
};

#endif // MOCKAUTHENTICATOR_H
