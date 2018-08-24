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

#ifndef AUTHENTICATIONPROCESS_H
#define AUTHENTICATIONPROCESS_H

#include <QObject>
#include <QProcess>
#include <QElapsedTimer>
#include <QNetworkAccessManager>

#include "authenticator.h"
#include "userinformation.h"

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

    QNetworkAccessManager *m_manager = nullptr;
    QProcess *m_process = nullptr;
    QElapsedTimer m_requestTimer;
    QElapsedTimer m_lambdaTimer;
    QElapsedTimer m_processTimer;

    void requestDynamicCredentials();
    void invokeLambdaFunction(const QString accessKey, const QString &secretAccessKey, const QString &sessionToken);
    void startVerificationProcess(const QString accessKey, const QString &secretAccessKey, const QString &sessionToken);
    void cleanUp();

signals:
    void authenticationFinished(Authenticator::AuthenticationError error, const UserInformation &userInformation = UserInformation());

private slots:
    void onDynamicCredentialsReady();
    void onLambdaInvokeFunctionFinished();
    void onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);

public slots:
    void authenticate(const QString &token);

};

}

#endif // AUTHENTICATIONPROCESS_H
