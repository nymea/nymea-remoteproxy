/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright 2013 - 2023, nymea GmbH
* Contact: contact@nymea.io
*
* This file is part of nymea.
* This project including source code and documentation is protected by copyright law, and
* remains the property of nymea GmbH. All rights, including reproduction, publication,
* editing and translation, are reserved. The use of this project is subject to the terms of a
* license agreement to be concluded with nymea GmbH in accordance with the terms
* of use of nymea GmbH, available under https://nymea.io/license
*
* GNU Lesser General Public License Usage
* Alternatively, this project may be redistributed and/or modified under the terms of the GNU
* Lesser General Public License as published by the Free Software Foundation; version 3.
* this project is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
* without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
* See the GNU Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License along with this project.
* If not, see <https://www.gnu.org/licenses/>.
*
* For any further details and any questions please contact us under contact@nymea.io
* or see our FAQ/Licensing Information on https://nymea.io/license/faq
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

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
