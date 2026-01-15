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

#include "tunnelproxysocket.h"
#include "proxyconnection.h"
#include "tunnelproxysocketserver.h"
#include "../common/slipdataprocessor.h"

namespace remoteproxyclient {

namespace {
const int kMaxPendingBytes = 1024 * 1024;
}

TunnelProxySocket::TunnelProxySocket(ProxyConnection *connection, TunnelProxySocketServer *socketServer, const QString &clientName, const QUuid &clientUuid, const QHostAddress &clientPeerAddress, quint16 socketAddress, QObject *parent) :
    QObject(parent),
    m_connection(connection),
    m_socketServer(socketServer),
    m_clientName(clientName),
    m_clientUuid(clientUuid),
    m_clientPeerAddress(clientPeerAddress),
    m_socketAddress(socketAddress),
    m_e2ee(TunnelProxyE2ee::RoleServer)
{

}

QUuid TunnelProxySocket::clientUuid() const
{
    return m_clientUuid;
}

QString TunnelProxySocket::clientName() const
{
    return m_clientName;
}

QHostAddress TunnelProxySocket::clientPeerAddress() const
{
    return m_clientPeerAddress;
}

quint16 TunnelProxySocket::socketAddress() const
{
    return m_socketAddress;
}

bool TunnelProxySocket::connected() const
{
    return m_connected;
}

void TunnelProxySocket::writeData(const QByteArray &data)
{
    if (!m_e2ee.established()) {
        queuePendingData(data);
        return;
    }

    sendEncryptedData(data);
}

void TunnelProxySocket::disconnectSocket()
{
    m_socketServer->requestSocketDisconnect(m_socketAddress);
}

void TunnelProxySocket::processIncomingData(const QByteArray &data)
{
    QList<QByteArray> plaintexts;
    QList<QByteArray> responseFrames;
    QString error;
    bool wasEstablished = m_e2ee.established();

    if (!m_e2ee.processIncoming(data, &plaintexts, &responseFrames, &error)) {
        qCWarning(dcTunnelProxySocketServer()) << "E2EE processing failed:" << error << "Disconnecting socket" << m_socketAddress;
        disconnectSocket();
        return;
    }

    foreach (const QByteArray &frame, responseFrames) {
        sendFrame(frame);
    }

    if (!wasEstablished && m_e2ee.established()) {
        flushPendingData();
    }

    foreach (const QByteArray &plaintext, plaintexts) {
        emit dataReceived(plaintext);
    }
}

void TunnelProxySocket::sendFrame(const QByteArray &payload)
{
    SlipDataProcessor::Frame frame;
    frame.socketAddress = m_socketAddress;
    frame.data = payload;
    m_connection->sendData(SlipDataProcessor::serializeData(SlipDataProcessor::buildFrame(frame)));
}

bool TunnelProxySocket::sendEncryptedData(const QByteArray &data)
{
    QList<QByteArray> frames;
    QString error;
    if (!m_e2ee.buildDataFrames(data, &frames, &error)) {
        qCWarning(dcTunnelProxySocketServer()) << "Failed to encrypt data:" << error << "Disconnecting socket" << m_socketAddress;
        disconnectSocket();
        return false;
    }

    foreach (const QByteArray &frame, frames) {
        sendFrame(frame);
    }

    return true;
}

void TunnelProxySocket::queuePendingData(const QByteArray &data)
{
    if (m_pendingBytes + data.size() > kMaxPendingBytes) {
        qCWarning(dcTunnelProxySocketServer()) << "Pending E2EE data limit exceeded. Disconnecting socket" << m_socketAddress;
        disconnectSocket();
        return;
    }

    m_pendingData.append(data);
    m_pendingBytes += data.size();
}

void TunnelProxySocket::flushPendingData()
{
    if (m_pendingData.isEmpty())
        return;

    QList<QByteArray> pending = m_pendingData;
    m_pendingData.clear();
    m_pendingBytes = 0;

    foreach (const QByteArray &data, pending) {
        if (!sendEncryptedData(data))
            return;
    }
}

void TunnelProxySocket::setDisconnected()
{
    m_connected = false;
    emit connectedChanged(false);
    emit disconnected();
}

QDebug operator<<(QDebug debug, TunnelProxySocket *tunnelProxySocket)
{
    QDebugStateSaver saver(debug);
    debug.nospace() << "TunnelProxySocket(";
    debug.nospace() << tunnelProxySocket->clientName() << ", ";
    debug.nospace() << tunnelProxySocket->clientUuid().toString() << ", ";
    debug.nospace() << tunnelProxySocket->clientPeerAddress().toString() << ", ";
    debug.nospace() << tunnelProxySocket->socketAddress() << ")";
    return debug;
}

}
