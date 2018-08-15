#include "authenticationprocess.h"
#include "loggingcategories.h"

#include <QUrl>
#include <QFile>
#include <QNetworkReply>
#include <QJsonDocument>

namespace remoteproxy {

AuthenticationProcess::AuthenticationProcess(QNetworkAccessManager *manager, QObject *parent) :
    QObject(parent),
    m_manager(manager)
{
    m_process = new QProcess(this);
    m_process->setProcessChannelMode(QProcess::MergedChannels);
    connect(m_process, static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished), this, &AuthenticationProcess::onProcessFinished);
}

void AuthenticationProcess::useDynamicCredentials(bool dynamicCredentials)
{
    m_dynamicCredentials = dynamicCredentials;
}

void AuthenticationProcess::requestDynamicCredentials()
{
    m_requestTimer.start();
    QNetworkReply *reply = m_manager->get(QNetworkRequest(QUrl("http://169.254.169.254/latest/meta-data/iam/security-credentials/EC2-Remote-Connection-Proxy-Role")));
    connect(reply, &QNetworkReply::finished, this, &AuthenticationProcess::onDynamicCredentialsReady);
}

void AuthenticationProcess::startVerificationProcess()
{
    if (m_process->state() != QProcess::NotRunning) {
        qCWarning(dcAuthenticationProcess()) << "Authentication process already running. Killing the running process and restart.";
        m_process->kill();
    }

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

    // FIXME: check how to clean this up properly
    m_resultFileName = "/tmp/" + QUuid::createUuid().toString().remove("{").remove("}").remove("-") + ".json";

    qCDebug(dcAuthentication()) << "Start authenticator process and store result in" << m_resultFileName;
    m_processTimer.start();
    m_process->start("aws", { "lambda", "invoke",
                              "--function-name", "system-services-authorizer-dev-checkToken",
                              "--invocation-type", "RequestResponse",
                              "--payload", QString::fromUtf8(QJsonDocument::fromVariant(request).toJson()),
                              m_resultFileName });

}

void AuthenticationProcess::onDynamicCredentialsReady()
{
    QNetworkReply *reply = static_cast<QNetworkReply *>(sender());
    reply->deleteLater();

    qCDebug(dcAuthenticationProcess()) << "Dynamic credentials request finished (" << m_requestTimer.elapsed() << "[ms] )";
    if (reply->error()) {
        qCWarning(dcAuthenticationProcess()) << "Dynamic credentials reply error: " << reply->errorString();
        emit authenticationFinished(Authenticator::AuthenticationErrorProxyError);
        return;
    }

    QByteArray data = reply->readAll();

    QJsonParseError error;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(data, &error);

    if(error.error != QJsonParseError::NoError) {
        qCWarning(dcAuthenticationProcess()) << "Failed to parse dynamic credentials reply data" << data << ":" << error.errorString();
        emit authenticationFinished(Authenticator::AuthenticationErrorProxyError);
        return;
    }

    QVariantMap response = jsonDoc.toVariant().toMap();
    qCDebug(dcAuthentication()) << "-->" << response;

    m_awsAccessKeyId = response.value("AccessKeyId").toString();
    m_awsSecretAccessKey = response.value("SecretAccessKey").toString();
    m_awsSessionToken = response.value("Token").toString();

    startVerificationProcess();
}

void AuthenticationProcess::onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    qCDebug(dcAuthenticationProcess()) << "Authentication process finished (" << m_processTimer.elapsed() << "[ms] )";;

    if (exitStatus == QProcess::CrashExit) {
        qCWarning(dcAuthenticationProcess()) << "Authentication process crashed:" << endl << qUtf8Printable(m_process->readAll());
        emit authenticationFinished(Authenticator::AuthenticationErrorProxyError);
        return;
    }

    if (exitCode != 0) {
        qCWarning(dcAuthenticationProcess()) << "The authentication process finished with error" << exitCode << endl << qUtf8Printable(m_process->readAll());
        emit authenticationFinished(Authenticator::AuthenticationErrorProxyError);
        return;
    }

    QFile resultFile(m_resultFileName);

    if (!resultFile.exists()) {
        qCWarning(dcAuthenticationProcess()) << "The process output file does not exist.";
        emit authenticationFinished(Authenticator::AuthenticationErrorProxyError);
        return;
    }

    if (!resultFile.open(QIODevice::ReadWrite)) {
        qCWarning(dcAuthenticationProcess()) << "Could not open result file from process:" << resultFile.errorString();
        emit authenticationFinished(Authenticator::AuthenticationErrorProxyError);
        return;
    }

    QByteArray resultData = resultFile.readAll();

    resultFile.close();
    if (!resultFile.remove()) {
        qCWarning(dcAuthenticationProcess()) << "Could not clean up result file from process:" << resultFile.errorString();
        return;
    }

    QJsonParseError error;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(resultData, &error);

    if(error.error != QJsonParseError::NoError) {
        qCWarning(dcAuthenticationProcess()) << "Failed to parse lambda invoke result data" << resultData << ":" << error.errorString();
        emit authenticationFinished(Authenticator::AuthenticationErrorProxyError);
        return;
    }

    QVariantMap response = jsonDoc.toVariant().toMap();
    qCDebug(dcAuthenticationProcess()) << "-->" << response;
    if (response.isEmpty()) {
        qCWarning(dcAuthenticationProcess()) << "Received empty lambda result.";
        emit authenticationFinished(Authenticator::AuthenticationErrorProxyError);
        return;
    }

    bool isValid = response.value("isValid").toBool();

    if (isValid) {
        emit authenticationFinished(Authenticator::AuthenticationErrorNoError);
    } else {
        emit authenticationFinished(Authenticator::AuthenticationErrorAuthenticationFailed);
    }

}

void AuthenticationProcess::authenticate(const QString &token)
{
    qCDebug(dcAuthenticationProcess()) << "Start authentication process for token" << token;
    m_token = token;

    if (m_dynamicCredentials) {
        // Request the access information
        requestDynamicCredentials();
    } else {
        // Direct call aws cli and assume the credentials will be provided static
        startVerificationProcess();
    }
}

}
