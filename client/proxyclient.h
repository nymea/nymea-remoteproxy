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

#ifndef PROXYCLIENT_H
#define PROXYCLIENT_H

#include <QObject>
#include <QLoggingCategory>

#include "remoteproxyconnection.h"

using namespace remoteproxyclient;

Q_DECLARE_LOGGING_CATEGORY(dcProxyClient)

class ProxyClient : public QObject
{
    Q_OBJECT
public:
    explicit ProxyClient(const QString &name, const QUuid &uuid, QObject *parent = nullptr);

    void setInsecure(bool insecure);

private:
    QString m_name;
    QUuid m_uuid;
    QString m_token;
    bool m_insecure = false;

    RemoteProxyConnection *m_connection = nullptr;

private slots:
    void onErrorOccured(RemoteProxyConnection::Error error);
    void onClientReady();
    void onAuthenticationFinished();
    void onRemoteConnectionEstablished();
    void onClientDisconnected();
    void onSslErrors(const QList<QSslError> errors);

public slots:
    void start(const QUrl &url, const QString &token);

};

#endif // PROXYCLIENT_H
