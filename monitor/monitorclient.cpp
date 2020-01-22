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

#include "monitorclient.h"

#include <QJsonDocument>

MonitorClient::MonitorClient(const QString &serverName, QObject *parent) :
    QObject(parent),
    m_serverName(serverName)
{
    m_socket = new QLocalSocket(this);

    connect(m_socket, &QLocalSocket::connected, this, &MonitorClient::onConnected);
    connect(m_socket, &QLocalSocket::disconnected, this, &MonitorClient::onDisconnected);
    connect(m_socket, &QLocalSocket::readyRead, this, &MonitorClient::onReadyRead);
    connect(m_socket, SIGNAL(error(QLocalSocket::LocalSocketError)), this, SLOT(onErrorOccured(QLocalSocket::LocalSocketError)));
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
    QByteArray data = m_socket->readAll();

    QJsonParseError error;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(data, &error);

    if(error.error != QJsonParseError::NoError) {
        qWarning() << "Failed to parse JSON data" << data << ":" << error.errorString();
        return;
    }

    //qDebug() << qUtf8Printable(jsonDoc.toJson(QJsonDocument::Indented));

    QVariantMap dataMap = jsonDoc.toVariant().toMap();
    emit dataReady(dataMap);
}

void MonitorClient::onErrorOccured(QLocalSocket::LocalSocketError socketError)
{
    Q_UNUSED(socketError)
    qWarning() << "Local socket error occured" << m_socket->errorString();
}

void MonitorClient::connectMonitor()
{    
    m_socket->connectToServer(m_serverName, QLocalSocket::ReadOnly);
}

void MonitorClient::disconnectMonitor()
{
    m_socket->close();
}
