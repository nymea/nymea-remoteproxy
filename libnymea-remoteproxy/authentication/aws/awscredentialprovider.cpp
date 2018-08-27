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

#include "loggingcategories.h"
#include "awscredentialprovider.h"

#include <QJsonDocument>
#include <QNetworkReply>
#include <QJsonParseError>
#include <QNetworkRequest>

namespace remoteproxy {

AwsCredentialProvider::AwsCredentialProvider(QNetworkAccessManager *networkManager, const QUrl &awsCredentialsUrl, QObject *parent) :
    QObject(parent),
    m_networkManager(networkManager),
    m_requestUrl(awsCredentialsUrl)
{
    m_timer = new QTimer(this);
    m_timer->setSingleShot(false);
    m_timer->setInterval(90000);

    connect(m_timer, &QTimer::timeout, this, &AwsCredentialProvider::onTimeout);
}

QString AwsCredentialProvider::accessKey() const
{
    return m_accessKey;
}

QString AwsCredentialProvider::secretAccessKey() const
{
    return m_secretAccessKey;
}

QString AwsCredentialProvider::sessionToken() const
{
    return m_sessionToken;
}

bool AwsCredentialProvider::isValid() const
{
    return !m_accessKey.isEmpty() && !m_secretAccessKey.isEmpty() && !m_sessionToken.isEmpty();
}

bool AwsCredentialProvider::enabled() const
{
    return m_enabled;
}

void AwsCredentialProvider::refreshCredentials()
{
    qCDebug(dcAwsCredentialsProvider()) << "Update dynamic credentials form" << m_requestUrl.toString();

    m_requestTimer.start();
    QNetworkReply *reply = m_networkManager->get(QNetworkRequest(m_requestUrl));
    connect(reply, &QNetworkReply::finished, this, &AwsCredentialProvider::onReplyFinished);
}

void AwsCredentialProvider::clear()
{
    m_accessKey.clear();
    m_secretAccessKey.clear();
    m_sessionToken.clear();
}


void AwsCredentialProvider::onTimeout()
{
    // TODO: check if we need to refresh
    refreshCredentials();
}

void AwsCredentialProvider::onReplyFinished()
{
    QNetworkReply *reply = static_cast<QNetworkReply *>(sender());
    reply->deleteLater();

    qCDebug(dcAwsCredentialsProvider()) << "Dynamic credentials request finished (" << m_requestTimer.elapsed() << "[ms] )";
    if (reply->error()) {
        qCWarning(dcAwsCredentialsProvider()) << "Dynamic credentials reply error: " << reply->errorString();
        return;
    }

    QByteArray data = reply->readAll();

    QJsonParseError error;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(data, &error);

    if(error.error != QJsonParseError::NoError) {
        qCWarning(dcAwsCredentialsProviderTraffic()) << "Failed to parse dynamic credentials reply data" << data << ":" << error.errorString();
        return;
    }

    QVariantMap response = jsonDoc.toVariant().toMap();

    m_accessKey = response.value("AccessKeyId").toString();
    m_secretAccessKey = response.value("SecretAccessKey").toString();
    m_sessionToken = response.value("Token").toString();

    qCDebug(dcAwsCredentialsProviderTraffic()) << "Dynamic credentials updated:" << response;

    QDateTime expirationTime = QDateTime::fromString(response.value("Expiration").toString(), "yyyy-MM-ddThh:mm:ssZ");
    QDateTime lastUpdateTime = QDateTime::fromString(response.value("LastUpdated").toString(), "yyyy-MM-ddThh:mm:ssZ");
    qCDebug(dcAwsCredentialsProviderTraffic()) << "Exipration time" << expirationTime.toString("dd.MM.yyyy hh:mm:ss");
    qCDebug(dcAwsCredentialsProviderTraffic()) << "Last update time" << lastUpdateTime.toString("dd.MM.yyyy hh:mm:ss");
}

void AwsCredentialProvider::enable()
{
    if (!m_enabled) {
        clear();
        m_enabled = true;
    }

    qCDebug(dcAwsCredentialsProvider()) << "Enable AWS dynamic credentials provider";
    m_timer->start();
    refreshCredentials();
}

void AwsCredentialProvider::disable()
{
    qCDebug(dcAwsCredentialsProvider()) << "Disable AWS dynamic credentials provider";
    m_enabled = false;
    m_timer->stop();

    clear();
}

}
