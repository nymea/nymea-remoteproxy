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
