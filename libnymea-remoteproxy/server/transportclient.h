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

#ifndef TRANSPORTCLIENT_H
#define TRANSPORTCLIENT_H

#include <QObject>
#include <QUuid>
#include <QDebug>
#include <QHostAddress>

namespace remoteproxy {

class TransportInterface;

class TransportClient : public QObject
{
    Q_OBJECT
public:
    explicit TransportClient(TransportInterface *interface, const QUuid &clientId, const QHostAddress &address, QObject *parent = nullptr);
    virtual ~TransportClient() = default;

    QUuid clientId() const;
    QHostAddress peerAddress() const;

    quint64 creationTime() const;
    QString creationTimeString() const;

    // Schedule a disconnect after the response
    void killConnectionAfterResponse(const QString &killConnectionReason);
    bool killConnectionRequested() const;
    QString killConnectionReason() const;

    // Schedule SLIP enable after response
    void enableSlipAfterResponse();
    bool slipAfterResponseEnabled() const;

    bool slipEnabled() const;
    void setSlipEnabled(bool slipEnabled);

    TransportInterface *interface() const;

    // Properties from auth request
    QUuid uuid() const;
    void setUuid(const QUuid &uuid);

    QString name() const;
    void setName(const QString &name);

    quint64 rxDataCount() const;
    void addRxDataCount(int dataCount);

    quint64 txDataCount() const;
    void addTxDataCount(int dataCount);

    int bufferSize() const;

    int generateMessageId();

    virtual void sendData(const QByteArray &data);
    virtual void killConnection(const QString &reason);

    virtual QList<QByteArray> processData(const QByteArray &data) = 0;

signals:
    void rxDataCountChanged();
    void txDataCountChanged();

protected:
    TransportInterface *m_interface = nullptr;

    QUuid m_clientId;
    QHostAddress m_peerAddress;
    quint64 m_creationTimeStamp = 0;

    // Eveyone has to register him self everywhere with a name and a uuid
    QString m_name;
    QUuid m_uuid;

    QByteArray m_dataBuffer;

    bool m_killConnectionRequested = false;
    QString m_killConnectionReason;

    bool m_slipAfterResponseEnabled = false;
    bool m_slipEnabled = false;

    // Json data information
    int m_messageId = 0;

private:
    // Statistics info
    quint64 m_rxDataCount = 0;
    quint64 m_txDataCount = 0;

};

}

#endif // TRANSPORTCLIENT_H
