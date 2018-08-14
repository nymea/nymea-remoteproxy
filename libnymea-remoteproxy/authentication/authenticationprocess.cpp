#include "authenticationprocess.h"
#include "loggingcategories.h"

#include <QUrl>
#include <QNetworkReply>
#include <QJsonDocument>

namespace remoteproxy {

AuthenticationProcess::AuthenticationProcess(QNetworkAccessManager *manager, QObject *parent) :
    QObject(parent),
    m_manager(manager)
{

}

void AuthenticationProcess::useDynamicCredentials(bool dynamicCredentials)
{
    m_dynamicCredentials = dynamicCredentials;
}

void AuthenticationProcess::requestDynamicCredentials()
{
    QNetworkReply *reply = m_manager->get(QNetworkRequest(QUrl("http://169.254.169.254/latest/meta-data/iam/security-credentials/EC2-Remote-Connection-Proxy-Role")));
    connect(reply, &QNetworkReply::finished, this, &AuthenticationProcess::onDynamicCredentialsReady);
}

void AuthenticationProcess::startVerificationProcess()
{
    if (m_process->state() != QProcess::NotRunning) {
        qCWarning(dcAuthenticator()) << "Authentication process already running. Killing the running process and restart.";
        m_process->kill();
        m_process->deleteLater();
        m_process = nullptr;
    }

    m_process = new QProcess(this);
    connect(m_process, static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished), this, &AuthenticationProcess::onProcessFinished);

    // Create request map
    QVariantMap request;
    request.insert("token", m_token);


    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert("AWS_DEFAULT_REGION", "eu-west-1");

    if (m_dynamicCredentials) {
        env.insert("AWS_ACCESS_KEY_ID", m_awsAccessKeyId);
        env.insert("AWS_SECRET_ACCESS_KEY", m_awsSecretAccessKey);
        env.insert("AWS_SESSION_TOKEN", m_awsSessionToken);
    }

    // Output file name
    m_resultFileName = "/tmp/" + QUuid::createUuid().toString().remove("{").remove("}").remove("-") + ".json";

    qCDebug(dcAuthenticator()) << "Start authenticator process and store result in" << m_resultFileName;

    m_process->start("aws", { "lambda", "invoke",
                              "--function-name", "system-services-authorizer-dev-checkToken",
                              "--invocation-type", "RequestResponse",
                              "--payload", QString("'%1'").arg(QString::fromUtf8(QJsonDocument::fromVariant(request).toJson())),
                              m_resultFileName });

}

void AuthenticationProcess::onDynamicCredentialsReady()
{
    QNetworkReply *reply = static_cast<QNetworkReply *>(sender());
    reply->deleteLater();

    if (reply->error()) {
        qCWarning(dcAuthenticator()) << "Dynamic credentials reply error: " << reply->errorString();
        emit authenticationFinished(Authenticator::AuthenticationErrorProxyError);
        return;
    }

    QByteArray data = reply->readAll();

    QJsonParseError error;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(data, &error);

    if(error.error != QJsonParseError::NoError) {
        qCWarning(dcAuthenticator()) << "Failed to parse dynamic credentials reply data" << data << ":" << error.errorString();
        emit authenticationFinished(Authenticator::AuthenticationErrorProxyError);
        return;
    }


    QVariantMap response = jsonDoc.toVariant().toMap();
    qCDebug(dcAuthenticator()) << "-->" << response;

    m_awsAccessKeyId = response.value("AccessKeyId").toString();
    m_awsSecretAccessKey = response.value("SecretAccessKey").toString();
    m_awsSessionToken = response.value("Token").toString();

    startVerificationProcess();
}

void AuthenticationProcess::onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    qCDebug(dcAuthenticator()) << "Authentication process finished" << exitCode << exitStatus;
}

void AuthenticationProcess::authenticate(const QString &token)
{
    qCDebug(dcAuthenticator()) << "Start authentication process for token";
    m_token = token;

    if (m_dynamicCredentials) {
        // Request the access information
        requestDynamicCredentials();
    } else {
        startVerificationProcess();
    }
}

}
