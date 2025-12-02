// SPDX-License-Identifier: GPL-3.0-or-later

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright (C) 2013 - 2024, nymea GmbH
* Copyright (C) 2024 - 2025, chargebyte austria GmbH
*
* This file is part of nymea-remoteproxy.
*
* nymea-remoteproxy is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* nymea-remoteproxy is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with nymea-remoteproxy. If not, see <https://www.gnu.org/licenses/>.
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "monitorclient.h"
#include "utils.h"

#include <QJsonDocument>

MonitorClient::MonitorClient(const QString &serverName, bool jsonMode, QObject *parent) :
    QObject(parent),
    m_serverName(serverName),
    m_jsonMode(jsonMode)
{
    m_socket = new QLocalSocket(this);

    connect(m_socket, &QLocalSocket::connected, this, &MonitorClient::onConnected);
    connect(m_socket, &QLocalSocket::disconnected, this, &MonitorClient::onDisconnected);
    connect(m_socket, &QLocalSocket::readyRead, this, &MonitorClient::onReadyRead);
    connect(m_socket, SIGNAL(error(QLocalSocket::LocalSocketError)), this, SLOT(onErrorOccurred(QLocalSocket::LocalSocketError)));
}

bool MonitorClient::printAll() const
{
    return m_printAll;
}

void MonitorClient::setPrintAll(bool printAll)
{
    m_printAll = printAll;
}

void MonitorClient::processBufferData()
{
    QJsonParseError error;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(m_dataBuffer, &error);
    if(error.error != QJsonParseError::NoError) {
        qWarning() << "Failed to parse JSON data:" << error.errorString();
        return;
    }

    if (m_jsonMode) {
        qStdOut() << qUtf8Printable(jsonDoc.toJson(QJsonDocument::Indented)) << "\n";
        QTextStream out(stdout);
        out.flush();
        exit(EXIT_FAILURE);
    }

    QVariantMap dataMap = jsonDoc.toVariant().toMap();
    emit dataReady(dataMap);
}

void MonitorClient::onConnected()
{
    qDebug() << "Monitor connected to" << m_serverName;
    emit connected();
}

void MonitorClient::onDisconnected()
{
    qDebug() << "Monitor disconnected from" << m_serverName;
    emit disconnected();
}

void MonitorClient::onReadyRead()
{
    // Note: the server sends the data compact with "\n" at the end
    QByteArray data = m_socket->readAll();

    int index = data.indexOf("}\n");
    if (index < 0) {
        // Append the entire data and continue
        m_dataBuffer.append(data);
        return;
    } else {
        m_dataBuffer.append(data.left(index + 1));
        processBufferData();
        m_dataBuffer.clear();
        m_dataBuffer.append(data.right(data.length() - index - 2));
    }
}

void MonitorClient::onErrorOccurred(QLocalSocket::LocalSocketError socketError)
{
    Q_UNUSED(socketError)
    qWarning() << "Local socket error occurred" << m_socket->errorString();
}

void MonitorClient::connectMonitor()
{    
    m_socket->connectToServer(m_serverName, QLocalSocket::ReadWrite);
}

void MonitorClient::disconnectMonitor()
{
    m_socket->close();
}

void MonitorClient::refresh()
{
    if (m_socket->state() != QLocalSocket::ConnectedState)
        return;

    QVariantMap request;
    request.insert("method", "refresh");
    QVariantMap params;
    if (m_printAll) {
        params.insert("printAll", m_printAll);
    }

    if (!params.isEmpty())
        request.insert("params", params);

    m_socket->write(QJsonDocument::fromVariant(request).toJson(QJsonDocument::Compact) + "\n");
}
