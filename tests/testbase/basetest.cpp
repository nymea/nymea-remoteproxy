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

#include "basetest.h"

#include "engine.h"
#include "loggingcategories.h"

#include "../common/slipdataprocessor.h"

#include <QSslError>
#include <QMetaType>
#include <QSignalSpy>
#include <QSslSocket>
#include <QWebSocket>
#include <QJsonDocument>
#include <QWebSocketServer>

BaseTest::BaseTest(QObject *parent) :
    QObject(parent)
{
    resetDebugCategories();

    qRegisterMetaType<TransportClient*>("TransportClient*");
}

void BaseTest::loadConfiguration(const QString &fileName)
{
    qDebug() << "Load test configurations" << fileName;
    m_configuration->loadConfiguration(fileName);
    //restartEngine();
}

void BaseTest::resetDebugCategories()
{
    m_currentDebugCategories = m_defaultDebugCategories;
    QLoggingCategory::setFilterRules(m_currentDebugCategories);
}

void BaseTest::addDebugCategory(const QString &debugCategory)
{
    m_currentDebugCategories += debugCategory + "\n";
    //qDebug() << m_currentDebugCategories;
    QLoggingCategory::setFilterRules(m_currentDebugCategories);
}

void BaseTest::cleanUpEngine()
{
    qDebug() << "Clean up engine";
    if (Engine::exists()) {
        Engine::instance()->stop();
        Engine::instance()->destroy();
        QVERIFY(!Engine::exists());
    }

    if (m_configuration) {
        m_configuration = nullptr;
    }
}

void BaseTest::restartEngine()
{
    cleanUpEngine();
    startEngine();
}

void BaseTest::startEngine()
{
    if (!m_configuration)
        m_configuration = new ProxyConfiguration(Engine::instance());

    loadConfiguration(":/test-configuration.conf");

    QSignalSpy runningSpy(Engine::instance(), &Engine::runningChanged);
    Engine::instance()->start(m_configuration);
    runningSpy.wait();
    QVERIFY(runningSpy.count() == 1);
    QVERIFY(Engine::instance()->running());
}

void BaseTest::startServer()
{
    restartEngine();

    QVERIFY(Engine::instance()->running());
    QVERIFY(Engine::instance()->webSocketServerTunnelProxy()->running());
    QVERIFY(Engine::instance()->tcpSocketServerTunnelProxy()->running());
    QVERIFY(Engine::instance()->unixSocketServerTunnelProxy()->running());
    QVERIFY(Engine::instance()->tunnelProxyServer()->running());
    QVERIFY(Engine::instance()->monitorServer()->running());
}

void BaseTest::stopServer()
{
    if (!Engine::instance()->running())
        return;

    Engine::instance()->stop();
    QVERIFY(!Engine::instance()->running());

    cleanUpEngine();
}


QVariant BaseTest::invokeWebSocketTunnelProxyApiCall(const QString &method, const QVariantMap params)
{
    QVariantMap request;
    request.insert("id", m_commandCounter);
    request.insert("method", method);
    request.insert("params", params);
    QJsonDocument jsonDoc = QJsonDocument::fromVariant(request);

    QWebSocket *socket = new QWebSocket("tunnelproxy-testclient", QWebSocketProtocol::Version13);
    connect(socket, &QWebSocket::sslErrors, this, &BaseTest::sslErrors);
    QSignalSpy spyConnection(socket, SIGNAL(connected()));
    socket->open(Engine::instance()->webSocketServerTunnelProxy()->serverUrl());
    spyConnection.wait();
    if (spyConnection.count() == 0) {
        qWarning() << "Failed to connect websocket on tunnel proxy";
        return QVariant();
    }

    QSignalSpy dataSpy(socket, SIGNAL(textMessageReceived(QString)));
    socket->sendTextMessage(QString(jsonDoc.toJson(QJsonDocument::Compact)));
    dataSpy.wait();

    socket->close();
    socket->deleteLater();

    for (int i = 0; i < dataSpy.count(); i++) {
        // Make sure the response ends with '}\n'
        if (!dataSpy.at(i).last().toByteArray().endsWith("}\n")) {
            qWarning() << "JSON data does not end with \"}\n\"";
            return QVariant();
        }

        // Make sure the response it a valid JSON string
        QJsonParseError error;
        jsonDoc = QJsonDocument::fromJson(dataSpy.at(i).last().toByteArray(), &error);
        if (error.error != QJsonParseError::NoError) {
            qWarning() << "JSON parser error" << error.errorString();
            return QVariant();
        }
        QVariantMap response = jsonDoc.toVariant().toMap();

        // skip notifications
        if (response.contains("notification"))
            continue;

        if (response.value("id").toInt() == m_commandCounter) {
            m_commandCounter++;
            return jsonDoc.toVariant();
        }
    }

    m_commandCounter++;
    return QVariant();
}

QVariant BaseTest::injectWebSocketTunnelProxyData(const QByteArray &data)
{
    QWebSocket *socket = new QWebSocket("tunnelproxy-testclient", QWebSocketProtocol::Version13);
    connect(socket, &QWebSocket::sslErrors, this, &BaseTest::sslErrors);

    QSignalSpy spyConnection(socket, SIGNAL(connected()));
    socket->open(Engine::instance()->webSocketServerTunnelProxy()->serverUrl());
    spyConnection.wait();
    if (spyConnection.count() == 0) {
        return QVariant();
    }

    QSignalSpy spy(socket, SIGNAL(textMessageReceived(QString)));
    socket->sendTextMessage(QString(data));
    spy.wait();

    socket->close();
    socket->deleteLater();

    for (int i = 0; i < spy.count(); i++) {
        // Make sure the response ends with '}\n'
        if (!spy.at(i).last().toByteArray().endsWith("}\n")) {
            qWarning() << "JSON data does not end with \"}\n\"";
            return QVariant();
        }

        // Make sure the response it a valid JSON string
        QJsonParseError error;
        QJsonDocument jsonDoc = QJsonDocument::fromJson(spy.at(i).last().toByteArray(), &error);
        if (error.error != QJsonParseError::NoError) {
            qWarning() << "JSON parser error" << error.errorString();
            return QVariant();
        }
        return jsonDoc.toVariant();
    }
    m_commandCounter++;
    return QVariant();
}

QVariant BaseTest::invokeTcpSocketTunnelProxyApiCall(const QString &method, const QVariantMap params)
{
    QVariantMap request;
    request.insert("id", m_commandCounter);
    request.insert("method", method);
    request.insert("params", params);
    QJsonDocument jsonDoc = QJsonDocument::fromVariant(request);

    QSslSocket *socket = new QSslSocket(this);
    typedef void (QSslSocket:: *sslErrorsSignal)(const QList<QSslError> &);
    QObject::connect(socket, static_cast<sslErrorsSignal>(&QSslSocket::sslErrors), this, &BaseTest::sslSocketSslErrors);

    QSignalSpy spyConnection(socket, &QSslSocket::connected);
    socket->connectToHostEncrypted(Engine::instance()->tcpSocketServerTunnelProxy()->serverUrl().host(),
                                   static_cast<quint16>(Engine::instance()->tcpSocketServerTunnelProxy()->serverUrl().port()));
    spyConnection.wait();
    if (spyConnection.count() == 0) {
        return QVariant();
    }

    QSignalSpy dataSpy(socket, &QSslSocket::readyRead);
    socket->write(jsonDoc.toJson(QJsonDocument::Compact) + "\n");
    dataSpy.wait();
    if (dataSpy.count() != 1) {
        qWarning() << "No data received";
        return QVariant();
    }

    QByteArray data = socket->readAll();
    socket->close();
    socket->deleteLater();

    // Make sure the response ends with '}\n'
    if (!data.endsWith("}\n")) {
        qWarning() << "JSON data does not end with \"}\n\"";
        return QVariant();
    }

    // Make sure the response it a valid JSON string
    QJsonParseError error;
    jsonDoc = QJsonDocument::fromJson(data, &error);
    if (error.error != QJsonParseError::NoError) {
        qWarning() << "JSON parser error" << error.errorString();
        return QVariant();
    }

    QVariantMap response = jsonDoc.toVariant().toMap();

    if (response.value("id").toInt() == m_commandCounter) {
        m_commandCounter++;
        return jsonDoc.toVariant();
    }

    m_commandCounter++;
    return QVariant();
}

QVariant BaseTest::injectTcpSocketTunnelProxyData(const QByteArray &data)
{
    QSslSocket *socket = new QSslSocket(this);
    typedef void (QSslSocket:: *sslErrorsSignal)(const QList<QSslError> &);
    QObject::connect(socket, static_cast<sslErrorsSignal>(&QSslSocket::sslErrors), this, &BaseTest::sslSocketSslErrors);

    QSignalSpy spyConnection(socket, &QSslSocket::connected);
    socket->connectToHostEncrypted(Engine::instance()->tcpSocketServerTunnelProxy()->serverUrl().host(),
                                   static_cast<quint16>(Engine::instance()->tcpSocketServerTunnelProxy()->serverUrl().port()));
    spyConnection.wait();
    if (spyConnection.count() == 0) {
        return QVariant();
    }

    QSignalSpy dataSpy(socket, &QSslSocket::readyRead);
    socket->write(data + "\n");
    dataSpy.wait();
    if (dataSpy.count() != 1) {
        qWarning() << "No data received";
        return QVariant();
    }

    QByteArray socketData = socket->readAll();
    socket->close();
    socket->deleteLater();

    // Make sure the response ends with '}\n'
    if (!socketData.endsWith("}\n")) {
        qWarning() << "JSON data does not end with \"}\n\"";
        return QVariant();
    }

    // Make sure the response it a valid JSON string
    QJsonParseError error;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(socketData, &error);
    if (error.error != QJsonParseError::NoError) {
        qWarning() << "JSON parser error" << error.errorString();
        return QVariant();
    }

    m_commandCounter++;
    return jsonDoc.toVariant();
}

QPair<QVariant, QSslSocket *> BaseTest::invokeTcpSocketTunnelProxyApiCallPersistant(const QString &method, const QVariantMap params,  bool slipEnabled, QSslSocket *existingSocket)
{
    QSslSocket *socket = nullptr;
    if (!existingSocket) {
        socket = new QSslSocket(this);
        typedef void (QSslSocket:: *sslErrorsSignal)(const QList<QSslError> &);
        QObject::connect(socket, static_cast<sslErrorsSignal>(&QSslSocket::sslErrors), this, &BaseTest::sslSocketSslErrors);

        QSignalSpy spyConnection(socket, &QSslSocket::connected);
        socket->connectToHostEncrypted(Engine::instance()->tcpSocketServerTunnelProxy()->serverUrl().host(),
                                       static_cast<quint16>(Engine::instance()->tcpSocketServerTunnelProxy()->serverUrl().port()));
        spyConnection.wait();
        if (spyConnection.count() == 0) {
            return QPair<QVariant, QSslSocket *>(QVariant(), socket);
        }
    } else {
        socket = existingSocket;
    }

    QVariantMap request;
    request.insert("id", m_commandCounter);
    request.insert("method", method);
    request.insert("params", params);
    QJsonDocument jsonDoc = QJsonDocument::fromVariant(request);
    QByteArray payload = jsonDoc.toJson(QJsonDocument::Compact)  + "\n";

    QSignalSpy dataSpy(socket, &QSslSocket::readyRead);

    if (slipEnabled) {
        SlipDataProcessor::Frame frame;
        frame.socketAddress = 0x0000;
        frame.data = payload;
        socket->write(SlipDataProcessor::serializeData(SlipDataProcessor::buildFrame(frame)));
    } else {
        socket->write(payload);
    }

    dataSpy.wait();
    if (dataSpy.count() < 1) {
        qWarning() << "No data received";
        return QPair<QVariant, QSslSocket *>(QVariant(), socket);
    }

    QByteArray data;
    if (slipEnabled) {
        QByteArray rawData = socket->readAll();
        QByteArray messageData;
        // Parse SLIP frame
        for (int i = 0; i < rawData.length(); i++) {
            quint8 byte = static_cast<quint8>(rawData.at(i));
            if (byte == SlipDataProcessor::ProtocolByteEnd) {
                // If there is no data...continue since it might be a starting END byte
                if (messageData.isEmpty())
                    continue;

                break;
            } else {
                messageData.append(rawData.at(i));
            }
        }

        QByteArray deserializedData = SlipDataProcessor::deserializeData(messageData);
        SlipDataProcessor::Frame frame = SlipDataProcessor::parseFrame(deserializedData);
        if (frame.data.isEmpty()) {
            qWarning() << "Could not deserialize SLIP data";
            return QPair<QVariant, QSslSocket *>(QVariant(), socket);
        }
        data = frame.data;
    } else {
        data = socket->readAll();
        // Make sure the response ends with '}\n'
        if (!data.endsWith("}\n")) {
            qWarning() << "JSON data does not end with \"}\n\"";
            return QPair<QVariant, QSslSocket *>(QVariant(), socket);
        }
    }

    // Make sure the response it a valid JSON string
    QJsonParseError error;
    jsonDoc = QJsonDocument::fromJson(data, &error);
    if (error.error != QJsonParseError::NoError) {
        qWarning() << "JSON parser error" << error.errorString() << qUtf8Printable(data);
        return QPair<QVariant, QSslSocket *>(QVariant(), socket);
    }

    QVariantMap response = jsonDoc.toVariant().toMap();

    if (response.value("id").toInt() == m_commandCounter) {
        m_commandCounter++;
        return QPair<QVariant, QSslSocket *>(jsonDoc.toVariant(), socket);
    }

    m_commandCounter++;
    return QPair<QVariant, QSslSocket *>(QVariant(), socket);
}

QPair<QVariant, QWebSocket *> BaseTest::invokeWebSocketTunnelProxyApiCallPersistant(const QString &method, const QVariantMap params,  bool slipEnabled, QWebSocket *existingSocket)
{
    QWebSocket *socket = nullptr;
    if (!existingSocket) {
        socket = new QWebSocket("tunnelproxy-testclient", QWebSocketProtocol::Version13);
        connect(socket, &QWebSocket::sslErrors, this, &BaseTest::sslErrors);
        QSignalSpy spyConnection(socket, SIGNAL(connected()));
        socket->open(Engine::instance()->webSocketServerTunnelProxy()->serverUrl());
        spyConnection.wait();
        if (spyConnection.count() == 0) {
            qWarning() << "Failed to connect websocket on tunnel proxy";
            return QPair<QVariant, QWebSocket *>(QVariant(), socket);
        }
    } else {
        socket = existingSocket;
    }

    QVariantMap request;
    request.insert("id", m_commandCounter);
    request.insert("method", method);
    request.insert("params", params);
    QJsonDocument jsonDoc = QJsonDocument::fromVariant(request);
    QByteArray payload = jsonDoc.toJson(QJsonDocument::Compact)  + "\n";


    if (socket->state() != QAbstractSocket::ConnectedState) {
        qWarning() << "Socket not connected";
        return QPair<QVariant, QWebSocket *>(QVariant(), socket);
    }

    QSignalSpy dataSpy(socket, SIGNAL(textMessageReceived(QString)));

    if (slipEnabled) {
        SlipDataProcessor::Frame frame;
        frame.socketAddress = 0x0000;
        frame.data = payload;
        socket->sendTextMessage(QString::fromUtf8(SlipDataProcessor::serializeData(SlipDataProcessor::buildFrame(frame))));
    } else {
        socket->sendTextMessage(QString(payload));
    }

    dataSpy.wait();

    for (int i = 0; i < dataSpy.count(); i++) {
        QByteArray data;
        if (slipEnabled) {
            QByteArray rawData = dataSpy.at(i).last().toByteArray();
            QByteArray messageData;
            // Parse SLIP frame
            for (int i = 0; i < rawData.length(); i++) {
                quint8 byte = static_cast<quint8>(rawData.at(i));
                if (byte == SlipDataProcessor::ProtocolByteEnd) {
                    // If there is no data...continue since it might be a starting END byte
                    if (messageData.isEmpty())
                        continue;

                    break;
                    messageData.clear();
                } else {
                    messageData.append(rawData.at(i));
                }
            }

            QByteArray deserializedData = SlipDataProcessor::deserializeData(messageData);
            SlipDataProcessor::Frame frame = SlipDataProcessor::parseFrame(deserializedData);
            if (frame.data.isEmpty()) {
                qWarning() << "Could not deserialize SLIP data";
                return QPair<QVariant, QWebSocket *>(QVariant(), socket);
            }
            data = frame.data;
        } else {
            data = dataSpy.at(i).last().toByteArray();
            // Make sure the response ends with '}\n'
            if (!data.endsWith("}\n")) {
                qWarning() << "JSON data does not end with \"}\n\"";
                return QPair<QVariant, QWebSocket *>(QVariant(), socket);
            }
        }

        // Make sure the response it a valid JSON string
        QJsonParseError error;
        jsonDoc = QJsonDocument::fromJson(dataSpy.at(i).last().toByteArray(), &error);
        if (error.error != QJsonParseError::NoError) {
            qWarning() << "JSON parser error" << error.errorString();
            return QPair<QVariant, QWebSocket *>(QVariant(), socket);
        }
        QVariantMap response = jsonDoc.toVariant().toMap();

        // skip notifications
        if (response.contains("notification"))
            continue;

        if (response.value("id").toInt() == m_commandCounter) {
            m_commandCounter++;
            return QPair<QVariant, QWebSocket *>(jsonDoc.toVariant(), socket);
        }
    }

    qWarning() << "No websocket data received...";
    m_commandCounter++;
    return QPair<QVariant, QWebSocket *>(QVariant(), socket);
}

void BaseTest::initTestCase()
{
    qCDebug(dcApplication()) << "Init test case done.";
}

void BaseTest::cleanupTestCase()
{
    qCDebug(dcApplication()) << "Clean up test case.";
    cleanUpEngine();
}

