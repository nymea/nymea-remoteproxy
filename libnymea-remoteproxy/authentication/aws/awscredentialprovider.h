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

#ifndef AWSCREDENTIALPROVIDER_H
#define AWSCREDENTIALPROVIDER_H

#include <QUrl>
#include <QTimer>
#include <QObject>
#include <QDateTime>
#include <QElapsedTimer>
#include <QNetworkAccessManager>

namespace remoteproxy {

class AwsCredentialProvider : public QObject
{
    Q_OBJECT
public:
    explicit AwsCredentialProvider(QNetworkAccessManager *networkManager, const QUrl &awsCredentialsUrl, QObject *parent = nullptr);

    QString accessKey() const;
    QString secretAccessKey() const;
    QString sessionToken() const;

    bool isValid() const;
    bool enabled() const;

private:
    QNetworkAccessManager *m_networkManager = nullptr;
    QTimer *m_timer = nullptr;

    QElapsedTimer m_requestTimer;

    bool m_enabled = false;

    QUrl m_requestUrl;

    QString m_accessKey;
    QString m_secretAccessKey;
    QString m_sessionToken;

    QDateTime m_expirationTime;
    QDateTime m_lastUpdateTime;

    void refreshCredentials();
    void clear();

private slots:
    void onTimeout();
    void onReplyFinished();

signals:
    void credentialsChanged();

public slots:
    void enable();
    void disable();

};

}

#endif // AWSCREDENTIALPROVIDER_H
