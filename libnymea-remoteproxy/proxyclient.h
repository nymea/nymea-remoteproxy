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

#ifndef PROXYCLIENT_H
#define PROXYCLIENT_H

#include <QUuid>
#include <QDebug>
#include <QObject>
#include <QTimer>
#include <QHostAddress>

#include "transportinterface.h"

namespace remoteproxy {

class ProxyClient : public QObject
{
    Q_OBJECT

public:
    explicit ProxyClient(TransportInterface *interface, const QUuid &clientId, const QHostAddress &address, QObject *parent = nullptr);

    QUuid clientId() const;
    QHostAddress peerAddress() const;

    uint creationTime() const;
    QString creationTimeString() const;

    bool isAuthenticated() const;
    void setAuthenticated(bool isAuthenticated);

    bool isTunnelConnected() const;
    void setTunnelConnected(bool isTunnelConnected);

    QString userName() const;
    void setUserName(const QString &userName);

    TransportInterface *interface() const;

    // Properties from auth request
    QString uuid() const;
    void setUuid(const QString &uuid);

    QString name() const;
    void setName(const QString &name);

    QString token() const;
    void setToken(const QString &token);

    QString nonce() const;
    void setNonce(const QString &nonce);

    quint64 rxDataCount() const;
    void addRxDataCount(int dataCount);

    quint64 txDataCount() const;
    void addTxDataCount(int dataCount);

    // Actions for this client
    void sendData(const QByteArray &data);
    void killConnection(const QString &reason);

private:
    TransportInterface *m_interface = nullptr;
    QTimer m_timer;

    QUuid m_clientId;
    QHostAddress m_peerAddress;
    uint m_creationTimeStamp = 0;

    bool m_authenticated = false;
    bool m_tunnelConnected = false;

    QString m_uuid;
    QString m_name;
    QString m_token;
    QString m_nonce;

    QString m_userName;

    quint64 m_rxDataCount = 0;
    quint64 m_txDataCount = 0;

signals:
    void authenticated();
    void tunnelConnected();
    void timeoutOccured();

};

QDebug operator<< (QDebug debug, ProxyClient *proxyClient);

}

#endif // PROXYCLIENT_H
