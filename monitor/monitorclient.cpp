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
    // Note: the server sends the data compact with '\n' at the end
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
