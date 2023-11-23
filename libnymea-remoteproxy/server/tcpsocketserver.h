/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
*  Copyright 2013 - 2023, nymea GmbH
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

#ifndef TCPSOCKETSERVER_H
#define TCPSOCKETSERVER_H

#include <QUuid>
#include <QTimer>
#include <QObject>
#include <QTcpServer>
#include <QSslConfiguration>

#include "transportinterface.h"

namespace remoteproxy {

class SslClient: public QSslSocket
{
    Q_OBJECT

public:
    explicit SslClient(QObject *parent = nullptr);

    void startWaitingForEncrypted();

private:
    QTimer m_timer;

};

class SslServer: public QTcpServer
{
    Q_OBJECT
public:
    explicit SslServer(bool sslEnabled, const QSslConfiguration &config, QObject *parent = nullptr);
    ~SslServer() override = default;

signals:
    void socketConnected(QSslSocket *socket);
    void socketDisconnected(QSslSocket *socket);
    void dataAvailable(QSslSocket *socket, const QByteArray &data);

protected:
    void incomingConnection(qintptr socketDescriptor) override;

private:
    bool m_sslEnabled = false;
    QSslConfiguration m_config;

    QVector<SslClient *> m_clients;

};


class TcpSocketServer : public TransportInterface
{
    Q_OBJECT
public:
    explicit TcpSocketServer(bool sslEnabled, const QSslConfiguration &sslConfiguration, QObject *parent = nullptr);
    ~TcpSocketServer() override;

    void sendData(const QUuid &clientId, const QByteArray &data) override;
    void killClientConnection(const QUuid &clientId, const QString &killReason) override;

    uint connectionsCount() const override;

    bool running() const override;

public slots:
    bool startServer() override;
    bool stopServer() override;

private:
    bool m_sslEnabled;
    QSslConfiguration m_sslConfiguration;

    QHash<QUuid, QSslSocket *> m_clientList;

    SslServer *m_server = nullptr;

private slots:
    void onDataAvailable(QSslSocket *client, const QByteArray &data);
    void onSocketConnected(QSslSocket *client);
    void onSocketDisconnected(QSslSocket *client);


};

}

#endif // TCPSOCKETSERVER_H
