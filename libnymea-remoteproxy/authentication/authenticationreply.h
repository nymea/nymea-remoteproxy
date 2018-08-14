#ifndef AUTHENTICATIONREPLY_H
#define AUTHENTICATIONREPLY_H

#include <QUuid>
#include <QTimer>
#include <QObject>
#include <QProcess>
#include <QElapsedTimer>

#include "authenticator.h"

namespace remoteproxy {

class AuthenticationReply : public QObject
{
    Q_OBJECT
public:
    friend class Authenticator;

    ProxyClient *proxyClient() const;

    bool isTimedOut() const;
    bool isFinished() const;

    Authenticator::AuthenticationError error() const;

private:
    explicit AuthenticationReply(ProxyClient *proxyClient, QObject *parent = nullptr);
    ProxyClient *m_proxyClient = nullptr;
    QTimer *m_timer = nullptr;
    QProcess *m_process = nullptr;

    bool m_timedOut = false;
    bool m_finished = false;

    Authenticator::AuthenticationError m_error = Authenticator::AuthenticationErrorUnknown;

    void setError(Authenticator::AuthenticationError error);
    void setFinished();

signals:
    void finished();

private slots:
    void onTimeout();

public slots:
    void abort();

};

}

#endif // AUTHENTICATIONREPLY_H
