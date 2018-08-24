/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                               *
 * Copyright (C) 2018 Simon Stürz <simon.stuerz@guh.io>                          *
 *                                                                               *
 * This file is part of nymea-remoteproxy.                                       *
 *                                                                               *
 * This program is free software: you can redistribute it and/or modify          *
 * it under the terms of the GNU General Public License as published by          *
 * the Free Software Foundation, either version 3 of the License, or             *
 * (at your option) any later version.                                           *
 *                                                                               *
 * This program is distributed in the hope that it will be useful,               *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of                *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                 *
 * GNU General Public License for more details.                                  *
 *                                                                               *
 * You should have received a copy of the GNU General Public License             *
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.         *
 *                                                                               *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "authenticationprocess.h"
#include "loggingcategories.h"

#include <QUrl>
#include <QFile>
#include <QNetworkReply>
#include <QJsonDocument>


#include "sigv4utils.h"

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

void AuthenticationProcess::invokeLambdaFunction(const QString accessKey, const QString &secretAccessKey, const QString &sessionToken)
{
    // Known configurations
    QString region = "eu-west-1";
    QString lambdaFunctionName = "system-services-authorizer-dev-checkToken";
    QString invocationType = "RequestResponse";
    QString service = "lambda";

    // {'url_path': '/2015-03-31/functions/system-services-authorizer-dev-checkToken/invocations', 'query_string': {}, 'method': 'POST', 'headers': {'X-Amz-Invocation-Type': 'RequestResponse', 'User-Agent': 'aws-cli/1.14.44 Python/3.6.5 Linux/4.15.0-1019-aws botocore/1.8.48'}, 'body': b'{"token": "...."}', 'url': 'https://lambda.eu-west-1.amazonaws.com/2015-03-31/functions/system-services-authorizer-dev-checkToken/invocations', 'context': {'client_region': 'eu-west-1', 'client_config': <botocore.config.Config object at 0x7f44560f3128>, 'has_streaming_input': True, 'auth_type': None}}

    // Create request map
    QVariantMap requestMap;
    requestMap.insert("token", m_token);
    QByteArray payload = QJsonDocument::fromVariant(requestMap).toJson(QJsonDocument::Compact);

    QUrl requestUrl;
    requestUrl.setScheme("https");
    requestUrl.setHost(QString("lambda.%1.amazonaws.com").arg(region));
    requestUrl.setPath(QString("/2015-03-31/functions/%1/invocations").arg(lambdaFunctionName));

    QNetworkRequest request(requestUrl);
    request.setRawHeader("User-Agent", QString("%1/%2 JSON-RPC/%3").arg(SERVER_NAME_STRING).arg(SERVER_VERSION_STRING).arg(API_VERSION_STRING).toUtf8());
    request.setRawHeader("Content-Type", "application/json");
    request.setRawHeader("host", requestUrl.host().toUtf8());
    request.setRawHeader("x-amz-invocation-type", invocationType.toUtf8());

    SigV4Utils::signRequest(QNetworkAccessManager::PostOperation, request, region, service, accessKey.toUtf8(), secretAccessKey.toUtf8(), sessionToken.toUtf8(), payload);

    qCDebug(dcAuthenticationProcess()) << "Invoke lambda function" << lambdaFunctionName;

    qCDebug(dcAuthenticationProcess()) << "--------------------------------------------";
    qCDebug(dcAuthenticationProcess()) << request.url().toString();

    foreach (const QByteArray &rawHeader, request.rawHeaderList()) {
        qDebug(dcAuthenticationProcess()) << request.rawHeader(rawHeader);
    }
    qCDebug(dcAuthenticationProcess()) << payload;
    qCDebug(dcAuthenticationProcess()) << "--------------------------------------------";

    m_lambdaTimer.start();

    QNetworkReply *reply = m_manager->get(request);
    connect(reply, &QNetworkReply::finished, this, &AuthenticationProcess::onLambdaInvokeFunctionFinished);

}

void AuthenticationProcess::startVerificationProcess(const QString accessKey, const QString &secretAccessKey, const QString &sessionToken)
{
    if (m_process->state() != QProcess::NotRunning) {
        qCWarning(dcAuthenticationProcess()) << "Authentication process already running. Killing the running process and restart.";
        m_process->kill();
    }

    // Create request map
    QVariantMap request;
    request.insert("token", m_token);

    // Set environment
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert("AWS_DEFAULT_REGION", "eu-west-1");
    if (m_dynamicCredentials) {
        qCDebug(dcAuthenticationProcess()) << "Using dynamic credentials" << accessKey << secretAccessKey << sessionToken;
        env.insert("AWS_ACCESS_KEY_ID", accessKey);
        env.insert("AWS_SECRET_ACCESS_KEY", secretAccessKey);
        env.insert("AWS_SESSION_TOKEN", sessionToken);
    }
    m_process->setProcessEnvironment(env);

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

    startVerificationProcess(response.value("AccessKeyId").toString(),
                             response.value("SecretAccessKey").toString(),
                             response.value("Token").toString());
}

void AuthenticationProcess::onLambdaInvokeFunctionFinished()
{
    QNetworkReply *reply = static_cast<QNetworkReply *>(sender());
    reply->deleteLater();

    qCDebug(dcAuthenticationProcess()) << "Lambda invoke  request finished (" << m_lambdaTimer.elapsed() << "[ms] )";
    if (reply->error()) {
        qCWarning(dcAuthenticationProcess()) << "Dynamic credentials reply error: " << reply->errorString();
        emit authenticationFinished(Authenticator::AuthenticationErrorProxyError);
        return;
    }

    QByteArray data = reply->readAll();

    qCDebug(dcAuthenticationProcess()) << "Invoke lambda function response ready";

    qCDebug(dcAuthenticationProcess()) << "--------------------------------------------";
    qCDebug(dcAuthenticationProcess()) << reply->request().url().toString();

    foreach (const QByteArray &rawHeader, reply->rawHeaderList()) {
        qDebug(dcAuthenticationProcess()) << reply->rawHeader(rawHeader);
    }
    qCDebug(dcAuthenticationProcess()) << data;
    qCDebug(dcAuthenticationProcess()) << "--------------------------------------------";


    QJsonParseError error;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(data, &error);

    if(error.error != QJsonParseError::NoError) {
        qCWarning(dcAuthenticationProcess()) << "Failed to parse dynamic credentials reply data" << data << ":" << error.errorString();
        emit authenticationFinished(Authenticator::AuthenticationErrorProxyError);
        return;
    }

    QVariantMap response = jsonDoc.toVariant().toMap();
    qCDebug(dcAuthentication()) << "-->" << response;

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

    qCDebug(dcAuthenticationProcess()) << "Lambda function result ready" << qUtf8Printable(resultData);

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
        QVariantMap verifiedDataMap = response.value("verifiedData").toMap();
        QString vendorId = verifiedDataMap.value("vendorId").toString();
        QString userPoolId = verifiedDataMap.value("userPoolId").toString();
        QVariantMap verifiedParsedTokenMap = verifiedDataMap.value("verifiedParsedToken").toMap();
        QString email = verifiedParsedTokenMap.value("email").toString();
        QString cognitoUsername = verifiedParsedTokenMap.value("cognito:username").toString();

        UserInformation userInformation(email, cognitoUsername, vendorId, userPoolId);

        emit authenticationFinished(Authenticator::AuthenticationErrorNoError, userInformation);
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
        // FIXME:
        // Direct call aws cli and assume the credentials will be provided static
        // startVerificationProcess();
    }
}

}
