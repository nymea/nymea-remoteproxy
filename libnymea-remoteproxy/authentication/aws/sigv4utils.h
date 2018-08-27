/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                               *
 * Copyright (C) 2018 Simon Stürz <simon.stuerz@guh.io>                          *
 * Copyright (C) 2018 Michael Zanetti <michael.zanetti@guh.io>                   *
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

#ifndef SIGV4UTILS_H
#define SIGV4UTILS_H

#include <QString>
#include <QNetworkRequest>
#include <QNetworkAccessManager>

namespace remoteproxy {

class SigV4Utils
{
public:
    SigV4Utils();

    // Signes a request by adding the "X-AMZ-Date" (if not present) and "X-AMZ-Signature" headers
    static void signRequest(QNetworkAccessManager::Operation operation, QNetworkRequest &request, const QString &region, const QString &service, const QString &invocationType, const QByteArray &accessKeyId, const QByteArray &secretAccessKey, const QByteArray &sessionToken = QByteArray(), const QByteArray &payload = QByteArray());

    static QByteArray getCurrentDateTime();

    static QByteArray getCanonicalQueryString(const QNetworkRequest &request, const QByteArray &accessKeyId, const QByteArray &secretAccessKey, const QByteArray &sessionToken, const QByteArray &region, const QByteArray &service, const QByteArray &payload);
    static QByteArray getCanonicalRequest(QNetworkAccessManager::Operation operation, const QNetworkRequest &request, const QByteArray &payload);
    static QByteArray getCanonicalHeaders(const QNetworkRequest &request);
    static QByteArray getCredentialScope(const QByteArray &algorithm, const QByteArray &dateTime, const QByteArray &region, const QByteArray &service);
    static QByteArray getStringToSign(const QByteArray &canonicalRequest, const QByteArray &dateTime, const QByteArray &region, const QByteArray &service);
    static QByteArray getSignatureKey(const QByteArray &key, const QByteArray &date, const QByteArray &region, const QByteArray &service);
    static QByteArray getSignature(const QByteArray &stringToSign, const QByteArray &secretAccessKey, const QByteArray &dateTime, const QString &region, const QString &service);
    static QByteArray getAuthorizationHeader(const QByteArray &accessKeyId, const QByteArray &dateTime, const QString &region, const QString &service, const QNetworkRequest &request, const QByteArray &signature);

};

}

#endif // SIGV4UTILS_H
