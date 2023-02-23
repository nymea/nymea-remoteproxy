/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
*  Copyright 2013 - 2020, nymea GmbH
*  Contact: contact@nymea.io
*
*  This file is part of nymea.
*  This project including source code and documentation is protected by copyright law, and
*  remains the property of nymea GmbH. All rights, including reproduction, publication,
*  editing and translation, are reserved. The use of this project is subject to the terms of a
*  license agreement to be concluded with nymea GmbH in accordance with the terms
*  of use of nymea GmbH, available under https://nymea.io/license
*
*  GNU General Public License Usage
*  Alternatively, this project may be redistributed and/or modified under
*  the terms of the GNU General Public License as published by the Free Software Foundation,
*  GNU version 3. this project is distributed in the hope that it will be useful, but WITHOUT ANY
*  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
*  PURPOSE. See the GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License along with this project.
*  If not, see <https://www.gnu.org/licenses/>.
*
*  For any further details and any questions please contact us under contact@nymea.io
*  or see our FAQ/Licensing Information on https://nymea.io/license/faq
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "authenticationprocess.h"
#include "loggingcategories.h"

#include <QUrl>
#include <QFile>
#include <QNetworkReply>
#include <QJsonDocument>

#include "engine.h"
#include "sigv4utils.h"

namespace remoteproxy {

AuthenticationProcess::AuthenticationProcess(QNetworkAccessManager *manager, const QString &accessKey, const QString &secretAccessKey, const QString &sessionToken, QObject *parent) :
    QObject(parent),
    m_manager(manager),
    m_accessKey(accessKey),
    m_secretAccessKey(secretAccessKey),
    m_sessionToken(sessionToken)
{
    m_process = new QProcess(this);
    m_process->setProcessChannelMode(QProcess::MergedChannels);
    connect(m_process, static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished), this, &AuthenticationProcess::onProcessFinished);
}

void AuthenticationProcess::invokeLambdaFunction()
{
    // Known configurations
    QString region = Engine::instance()->configuration()->awsRegion();
    QString lambdaFunctionName = Engine::instance()->configuration()->awsAuthorizerLambdaFunctionName();

    QString invocationType = "RequestResponse";
    QString service = "lambda";

    QUrl requestUrl;
    requestUrl.setScheme("https");
    requestUrl.setHost(QString("lambda.%1.amazonaws.com").arg(region));
    requestUrl.setPath(QString("/2015-03-31/functions/%1/invocations").arg(lambdaFunctionName));

    // Create request map
    QVariantMap requestMap;
    requestMap.insert("token", m_token);
    QByteArray payload = QJsonDocument::fromVariant(requestMap).toJson(QJsonDocument::Compact);

    QNetworkRequest request(requestUrl);
    //request.setRawHeader("User-Agent", QString("%1/%2 JSON-RPC/%3").arg(SERVER_NAME_STRING).arg(SERVER_VERSION_STRING).arg(API_VERSION_STRING).toUtf8());
    //request.setRawHeader("Content-Type", "application/json");
    request.setRawHeader("host", requestUrl.host().toUtf8());
    SigV4Utils::signRequest(QNetworkAccessManager::PostOperation, request, region, service, invocationType, m_accessKey.toUtf8(), m_secretAccessKey.toUtf8(), m_sessionToken.toUtf8(), payload);

    qCDebug(dcAuthenticationProcess()) << "Invoke lambda function" << lambdaFunctionName;

    qCDebug(dcAuthenticationProcess()) << "--------------------------------------------";
    qCDebug(dcAuthenticationProcess()) << request.url().toString();

    foreach (const QByteArray &rawHeader, request.rawHeaderList()) {
        qDebug(dcAuthenticationProcess()) << rawHeader << request.rawHeader(rawHeader);
    }
    qCDebug(dcAuthenticationProcess()) << payload;
    qCDebug(dcAuthenticationProcess()) << "--------------------------------------------";

    m_lambdaTimer.start();

    QNetworkReply *reply = m_manager->post(request, payload);
    connect(reply, &QNetworkReply::finished, this, &AuthenticationProcess::onLambdaInvokeFunctionFinished);

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

    // Set environment
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert("AWS_DEFAULT_REGION", "eu-west-1");
    if (m_fallback) {
        qCDebug(dcAuthenticationProcess()) << "Using dynamic credentials" << m_accessKey << m_secretAccessKey << m_sessionToken;
        env.insert("AWS_ACCESS_KEY_ID", m_accessKey);
        env.insert("AWS_SECRET_ACCESS_KEY", m_secretAccessKey);
        env.insert("AWS_SESSION_TOKEN", m_sessionToken);
    }

    m_process->setProcessEnvironment(env);

    // FIXME: check how to clean this up properly
    m_resultFileName = "/tmp/" + QUuid::createUuid().toString().remove("{").remove("}").remove("-") + ".json";

    QStringList processParams = { "lambda", "invoke",
                                  "--function-name", "system-services-authorizer-dev-checkToken",
                                  "--invocation-type", "RequestResponse",
                                  "--payload", QString::fromUtf8(QJsonDocument::fromVariant(request).toJson()),
                                  m_resultFileName };

    qCDebug(dcAuthenticationProcess()) << "Process environment" << env.toStringList();
    qCDebug(dcAuthenticationProcess()) << "Process params" << processParams;


    qCDebug(dcAuthentication()) << "Start authenticator process and store result in" << m_resultFileName;
    m_processTimer.start();
    m_process->start("aws", processParams);
}

void AuthenticationProcess::onLambdaInvokeFunctionFinished()
{
    QNetworkReply *reply = static_cast<QNetworkReply *>(sender());
    reply->deleteLater();

    qCDebug(dcAuthenticationProcess()) << "Lambda invoke request finished (" << m_lambdaTimer.elapsed() << "[ms] )";

    QByteArray data = reply->readAll();

    qCDebug(dcAuthenticationProcess()) << "--------------------------------------------";
    qCDebug(dcAuthenticationProcess()) << reply->request().url().toString();

    foreach (const QByteArray &rawHeader, reply->rawHeaderList()) {
        qDebug(dcAuthenticationProcess()) << rawHeader << reply->rawHeader(rawHeader);
    }
    qCDebug(dcAuthenticationProcess()) << qUtf8Printable(data);
    qCDebug(dcAuthenticationProcess()) << "--------------------------------------------";

    if (reply->error()) {
        qCWarning(dcAuthenticationProcess()) << "Lambda invoke reply error: " << reply->errorString();
        m_fallback = true;
        return;
    }

    qCDebug(dcAuthenticationProcess()) << "Lambda function result ready" << qUtf8Printable(data);

    QJsonParseError error;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(data, &error);
    if(error.error != QJsonParseError::NoError) {
        qCWarning(dcAuthenticationProcess()) << "Failed to parse lambda invoke result data" << data << ":" << error.errorString();
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

void AuthenticationProcess::onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    qCDebug(dcAuthenticationProcess()) << "Authentication process finished (" << m_processTimer.elapsed() << "[ms] )";;
    if (exitStatus == QProcess::CrashExit) {
        qCWarning(dcAuthenticationProcess()) << "Authentication process crashed:" << "\n" << qUtf8Printable(m_process->readAll());
        emit authenticationFinished(Authenticator::AuthenticationErrorProxyError);
        return;
    }

    if (exitCode != 0) {
        qCWarning(dcAuthenticationProcess()) << "The authentication process finished with error" << exitCode << "\n" << qUtf8Printable(m_process->readAll());
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
    invokeLambdaFunction();
}

}
