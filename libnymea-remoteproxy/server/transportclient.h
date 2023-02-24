/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
*  Copyright 2013 - 2022, nymea GmbH
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
    void trafficOccurred();

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
