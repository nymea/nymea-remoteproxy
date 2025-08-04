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

#include "engine.h"
#include "monitorserver.h"
#include "loggingcategories.h"

#include <QFile>
#include <QJsonDocument>

namespace remoteproxy {

MonitorServer::MonitorServer(const QString &serverName, QObject *parent) :
    QObject(parent),
    m_serverName(serverName)
{

}

MonitorServer::~MonitorServer()
{
    stopServer();
}

bool MonitorServer::running() const
{
    if (!m_server)
        return false;

    return m_server->isListening();
}

void MonitorServer::sendMonitorData(QLocalSocket *clientConnection, const QVariantMap &dataMap)
{
    QByteArray data = QJsonDocument::fromVariant(dataMap).toJson(QJsonDocument::Compact) + "\n";
    qCDebug(dcMonitorServer()) << "Sending monitor data" << qUtf8Printable(data);
    clientConnection->write(data);
    clientConnection->flush();
}

void MonitorServer::onMonitorConnected()
{
    QLocalSocket *clientConnection = m_server->nextPendingConnection();
    connect(clientConnection, &QLocalSocket::disconnected, this, &MonitorServer::onMonitorDisconnected);
    connect(clientConnection, &QLocalSocket::readyRead, this, &MonitorServer::onMonitorReadyRead);
    m_clients.append(clientConnection);
    qCDebug(dcMonitorServer()) << "New monitor connected.";
}

void MonitorServer::onMonitorDisconnected()
{
    qCDebug(dcMonitorServer()) << "Monitor disconnected.";
    QLocalSocket *clientConnection = static_cast<QLocalSocket *>(sender());
    m_clients.removeAll(clientConnection);
    clientConnection->deleteLater();
}

void MonitorServer::onMonitorReadyRead()
{
    QLocalSocket *clientConnection = qobject_cast<QLocalSocket*>(sender());
    QByteArray data = clientConnection->readAll();
    QJsonParseError error;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(data, &error);
    if(error.error != QJsonParseError::NoError) {
        qCWarning(dcMonitorServer()) << "Failed to parse JSON data from monitor:" << error.errorString();
        clientConnection->close();
        return;
    }

    processRequest(clientConnection, jsonDoc.toVariant().toMap());
}

void MonitorServer::processRequest(QLocalSocket *clientConnection, const QVariantMap &request)
{
    /* Refresh method. If no params, it will return the active list of connections

         TODO: filter for uuid's, ip's, names etc...

         {
            "method": "refresh",
            "params": {
                "printAll": bool
            }
         }

     */

    // Note: as simple as possible...no error handling, either you know what you do, or you see nothing here.
    if (request.contains("method")) {
        if (request.value("method").toString() == "refresh") {
            bool printAll = false;
            if (request.contains("params")) {
                QVariantMap params = request.value("params").toMap();
                if (params.contains("printAll")) {
                    printAll = params.value("printAll").toBool();
                }
            }

            sendMonitorData(clientConnection, Engine::instance()->buildMonitorData(printAll));
            return;
        }
    }

    // Not a valid request. Close the connection
    qCWarning(dcMonitorServer()) << "Could not understand the request. Closing connection.";
    clientConnection->close();
}

void MonitorServer::startServer()
{    
    qCDebug(dcMonitorServer()) << "Starting server on" << m_serverName;

    // Make sure the monitor can be created
    if (QFile::exists(m_serverName)) {
        qCDebug(dcMonitorServer()) << "Clean up old monitor socket";
        QFile::remove(m_serverName);
    }

    m_server = new QLocalServer(this);
    m_server->setSocketOptions(QLocalServer::UserAccessOption | QLocalServer::GroupAccessOption | QLocalServer::OtherAccessOption);

    if (!m_server->listen(m_serverName)) {
        qCWarning(dcMonitorServer()) << "Could not start local server for monitor on" << m_serverName << m_server->errorString();
        delete m_server;
        m_server = nullptr;
        return;
    }

    connect(m_server, &QLocalServer::newConnection, this, &MonitorServer::onMonitorConnected);
    qCDebug(dcMonitorServer()) << "Started successfully on" << m_serverName;
}

void MonitorServer::stopServer()
{
    if (!m_server)
        return;

    qCDebug(dcMonitorServer()) << "Stop server" << m_serverName;
    foreach (QLocalSocket *clientConnection, m_clients) {
        clientConnection->close();
    }

    m_server->close();
    delete m_server;
    m_server = nullptr;
}

void MonitorServer::updateClients(const QVariantMap &dataMap)
{
    // Send each monitor the current data
    foreach (QLocalSocket *clientConnection, m_clients) {
        sendMonitorData(clientConnection, dataMap);
    }
}

}
