#ifndef AUTHENTICATIONPROCESS_H
#define AUTHENTICATIONPROCESS_H

#include <QObject>
#include <QProcess>
#include <QNetworkAccessManager>

#include "authenticator.h"

namespace remoteproxy {

class AuthenticationProcess : public QObject
{
    Q_OBJECT
public:
    explicit AuthenticationProcess(QNetworkAccessManager *manager, QObject *parent = nullptr);

    void useDynamicCredentials(bool dynamicCredentials);

private:
    QString m_token;
    QString m_resultFileName;

    bool m_dynamicCredentials = true;
    QString m_awsAccessKeyId;
    QString m_awsSecretAccessKey;
    QString m_awsSessionToken;

    QNetworkAccessManager *m_manager = nullptr;
    QProcess *m_process = nullptr;

    void requestDynamicCredentials();
    void startVerificationProcess();

signals:
    void authenticationFinished(Authenticator::AuthenticationError error);

private slots:
    void onDynamicCredentialsReady();
    void onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);

public slots:
    void authenticate(const QString &token);

};

}

#endif // AUTHENTICATIONPROCESS_H
