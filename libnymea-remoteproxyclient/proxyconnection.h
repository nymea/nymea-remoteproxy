// SPDX-License-Identifier: LGPL-3.0-or-later

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* nymea-remoteproxy
* Tunnel proxy server for the nymea remote access
*
* Copyright (C) 2013 - 2024, nymea GmbH
* Copyright (C) 2024 - 2025, chargebyte austria GmbH
*
* This file is part of nymea-remoteproxy.
*
* nymea-remoteproxy is free software: you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public License
* as published by the Free Software Foundation, either version 3
* of the License, or (at your option) any later version.
*
* nymea-remoteproxy is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with nymea-remoteproxy. If not, see <https://www.gnu.org/licenses/>.
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef SOCKETCONNECTOR_H
#define SOCKETCONNECTOR_H

#include <QUrl>
#include <QObject>
#include <QSslError>
#include <QSslConfiguration>
#include <QHostAddress>

namespace remoteproxyclient {

class ProxyConnection : public QObject
{
    Q_OBJECT
public:
    explicit ProxyConnection(QObject *parent = nullptr);
    virtual ~ProxyConnection() = 0;

    virtual void sendData(const QByteArray &data) = 0;

    QUrl serverUrl() const;

    virtual void ignoreSslErrors() = 0;
    virtual void ignoreSslErrors(const QList<QSslError> &errors) = 0;

    bool connected();

private:
    bool m_connected = false;

protected:
    QUrl m_serverUrl;

    void setConnected(bool connected);

signals:
    void connectedChanged(bool connected);
    void dataReceived(const QByteArray &data);

    void stateChanged(QAbstractSocket::SocketState state);
    void errorOccurred(QAbstractSocket::SocketError error);

    void sslErrors(const QList<QSslError> &errors);

public slots:
    virtual void connectServer(const QUrl &serverUrl) = 0;
    virtual void disconnectServer() = 0;

};

}

#endif // SOCKETCONNECTOR_H
