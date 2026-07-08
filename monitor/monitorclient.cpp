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
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
    connect(m_socket, &QLocalSocket::errorOccurred, this, &MonitorClient::onErrorOccurred);
#else
    connect(m_socket, SIGNAL(error(QLocalSocket::LocalSocketError)), this, SLOT(onErrorOccurred(QLocalSocket::LocalSocketError)));
#endif
}

bool MonitorClient::printAll() const
{
    return m_printAll;
}

void MonitorClient::setPrintAll(bool printAll)
{
    m_printAll = printAll;
}

void MonitorClient::processBufferData(const QByteArray &message)
{
    QJsonParseError error;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(message, &error);
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
    m_dataBuffer.append(m_socket->readAll());

    // Drain every complete message currently in the buffer, not just the first one,
    // in case more than one refresh reply arrived in a single read.
    int index;
    while ((index = m_dataBuffer.indexOf("}\n")) >= 0) {
        QByteArray message = m_dataBuffer.left(index + 1);
        m_dataBuffer.remove(0, index + 2);
        processBufferData(message);
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
