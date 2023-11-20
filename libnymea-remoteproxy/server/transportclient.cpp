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

#include "transportclient.h"
#include "server/transportinterface.h"

#include <QDateTime>

namespace remoteproxy {

TransportClient::TransportClient(TransportInterface *interface, const QUuid &clientId, const QHostAddress &address, QObject *parent) :
    QObject(parent),
    m_interface(interface),
    m_clientId(clientId),
    m_peerAddress(address)
{
    m_creationTimeStamp = QDateTime::currentDateTime().toSecsSinceEpoch();
}

QUuid TransportClient::clientId() const
{
    return m_clientId;
}

QHostAddress TransportClient::peerAddress() const
{
    return m_peerAddress;
}

quint64 TransportClient::creationTime() const
{
    return m_creationTimeStamp;
}

QString TransportClient::creationTimeString() const
{
    return QDateTime::fromMSecsSinceEpoch(creationTime() * 1000).toString("dd.MM.yyyy hh:mm:ss");
}

void TransportClient::killConnectionAfterResponse(const QString &killConnectionReason)
{
    m_killConnectionRequested = true;
    m_killConnectionReason = killConnectionReason;
}

bool TransportClient::killConnectionRequested() const
{
    return m_killConnectionRequested;
}

QString TransportClient::killConnectionReason() const
{
    return m_killConnectionReason;
}

void TransportClient::enableSlipAfterResponse()
{
    m_slipAfterResponseEnabled = true;
}

bool TransportClient::slipAfterResponseEnabled() const
{
    return m_slipAfterResponseEnabled;
}

bool TransportClient::slipEnabled() const
{
    return m_slipEnabled;
}

void TransportClient::setSlipEnabled(bool slipEnabled)
{
    m_slipEnabled = slipEnabled;
}

TransportInterface *TransportClient::interface() const
{
    return m_interface;
}

QUuid TransportClient::uuid() const
{
    return m_uuid;
}

void TransportClient::setUuid(const QUuid &uuid)
{
    m_uuid = uuid;
}

QString TransportClient::name() const
{
    return m_name;
}

void TransportClient::setName(const QString &name)
{
    m_name = name;
}

quint64 TransportClient::rxDataCount() const
{
    return m_rxDataCount;
}

void TransportClient::addRxDataCount(int dataCount)
{
    m_rxDataCount += dataCount;
    if (dataCount > 0) {
        emit rxDataCountChanged();
    }
}

quint64 TransportClient::txDataCount() const
{
    return m_txDataCount;
}

void TransportClient::addTxDataCount(int dataCount)
{
    m_txDataCount += dataCount;
    if (dataCount > 0) {
        emit txDataCountChanged();
    }
}

int TransportClient::bufferSize() const
{
    return m_dataBuffer.size();
}

int TransportClient::generateMessageId()
{
    m_messageId++;
    return m_messageId;
}

void TransportClient::sendData(const QByteArray &data)
{
    if (!m_interface)
        return;

    addTxDataCount(data.count());
    m_interface->sendData(m_clientId, data);
}

void TransportClient::killConnection(const QString &reason)
{
    if (!m_interface)
        return;

    m_interface->killClientConnection(m_clientId, reason);
}

}
