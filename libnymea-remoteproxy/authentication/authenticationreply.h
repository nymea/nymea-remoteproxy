#ifndef AUTHENTICATIONREPLY_H
#define AUTHENTICATIONREPLY_H

#include <QUuid>
#include <QTimer>
#include <QObject>
#include <QElapsedTimer>

#include "authenticator.h"

class AuthenticationReply : public QObject
{
    Q_OBJECT
public:
    friend class Authenticator;

    explicit AuthenticationReply(const QUuid clientId, const QString &token, QObject *parent = nullptr);

    QUuid clientId() const;
    QString token() const;

    bool isTimedOut() const;
    bool isFinished() const;

    Authenticator::AuthenticationError error() const;

private:
    QUuid m_clientId;
    QString m_token;
    QTimer m_timer;

    bool m_timedOut = false;
    bool m_finished = false;
    Authenticator::AuthenticationError m_error;

    void setError(Authenticator::AuthenticationError error);

signals:
    void finished();

private slots:
    void onTimeout();

public slots:
    void abort();

};

#endif // AUTHENTICATIONREPLY_H
