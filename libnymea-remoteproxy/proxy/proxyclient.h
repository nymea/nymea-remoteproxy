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

#include "server/transportclient.h"

namespace remoteproxy {

class ProxyClient : public TransportClient
{
    Q_OBJECT

public:
    enum TimerWaitState {
        TimerWaitStateInactive,
        TimerWaitStateAlone
    };
    Q_ENUM(TimerWaitState)

    explicit ProxyClient(TransportInterface *interface, const QUuid &clientId, const QHostAddress &address, QObject *parent = nullptr);

    bool isAuthenticated() const;
    void setAuthenticated(bool isAuthenticated);

    bool isTunnelConnected() const;
    void setTunnelConnected(bool isTunnelConnected);

    QString userName() const;
    void setUserName(const QString &userName);

    QString tunnelIdentifier() const;

    QString token() const;
    void setToken(const QString &token);

    QString nonce() const;
    void setNonce(const QString &nonce);

    // Actions for this client
    TimerWaitState timerWaitState() const;
    void resetTimer();

    // Json server methods    
    QList<QByteArray> processData(const QByteArray &data) override;

private:
    QTimer *m_timer = nullptr;
    TimerWaitState m_timerWaitState = TimerWaitStateInactive;

    bool m_authenticated = false;
    bool m_tunnelConnected = false;

    QString m_token;
    QString m_nonce;

    QString m_userName;

signals:
    void authenticated();
    void tunnelConnected();
    void timeoutOccurred();

};

QDebug operator<< (QDebug debug, ProxyClient *proxyClient);

}

#endif // PROXYCLIENT_H
