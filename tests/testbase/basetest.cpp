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

#include "basetest.h"

#include "engine.h"
#include "loggingcategories.h"
#include "remoteproxyconnection.h"

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

    if (m_mockAuthenticator) {
        delete m_mockAuthenticator;
        m_mockAuthenticator = nullptr;
    }

    if (m_dummyAuthenticator) {
        delete  m_dummyAuthenticator;
        m_dummyAuthenticator = nullptr;
    }

    m_authenticator = nullptr;

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
    m_configuration = new ProxyConfiguration(this);
    loadConfiguration(":/test-configuration.conf");

    m_dummyAuthenticator = new DummyAuthenticator(this);
    m_mockAuthenticator = new MockAuthenticator(this);
    m_authenticator = qobject_cast<Authenticator *>(m_mockAuthenticator);
    if (!Engine::exists()) {
        Engine::instance()->setAuthenticator(m_authenticator);
        Engine::instance()->setDeveloperModeEnabled(true);
        QVERIFY(Engine::exists());
        QVERIFY(!Engine::instance()->running());
    }
}

void BaseTest::startServer()
{
    restartEngine();

    if (!Engine::instance()->running()) {
        QSignalSpy runningSpy(Engine::instance(), &Engine::runningChanged);
        Engine::instance()->setDeveloperModeEnabled(true);
        Engine::instance()->start(m_configuration);
        runningSpy.wait();
        QVERIFY(runningSpy.count() == 1);
    }

    QVERIFY(Engine::instance()->running());
    QVERIFY(Engine::instance()->developerMode());
    QVERIFY(Engine::instance()->webSocketServerProxy()->running());
    QVERIFY(Engine::instance()->tcpSocketServerProxy()->running());
    QVERIFY(Engine::instance()->webSocketServerTunnelProxy()->running());
    QVERIFY(Engine::instance()->tcpSocketServerTunnelProxy()->running());
    QVERIFY(Engine::instance()->proxyServer()->running());
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

QVariant BaseTest::invokeWebSocketProxyApiCall(const QString &method, const QVariantMap params)
{
    QVariantMap request;
    request.insert("id", m_commandCounter);
    request.insert("method", method);
    request.insert("params", params);
    QJsonDocument jsonDoc = QJsonDocument::fromVariant(request);

    QWebSocket *socket = new QWebSocket("proxy-testclient", QWebSocketProtocol::Version13);
    connect(socket, &QWebSocket::sslErrors, this, &BaseTest::sslErrors);
    QSignalSpy spyConnection(socket, SIGNAL(connected()));
    socket->open(Engine::instance()->webSocketServerProxy()->serverUrl());
    spyConnection.wait();
    if (spyConnection.count() == 0) {
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

QVariant BaseTest::injectWebSocketProxyData(const QByteArray &data)
{
    QWebSocket *socket = new QWebSocket("proxy-testclient", QWebSocketProtocol::Version13);
    connect(socket, &QWebSocket::sslErrors, this, &BaseTest::sslErrors);

    QSignalSpy spyConnection(socket, SIGNAL(connected()));
    socket->open(Engine::instance()->webSocketServerProxy()->serverUrl());
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

QVariant BaseTest::invokeTcpSocketProxyApiCall(const QString &method, const QVariantMap params)
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
    socket->connectToHostEncrypted(Engine::instance()->tcpSocketServerProxy()->serverUrl().host(),
                                   static_cast<quint16>(Engine::instance()->tcpSocketServerProxy()->serverUrl().port()));
    spyConnection.wait();
    if (spyConnection.count() == 0) {
        return QVariant();
    }

    QSignalSpy dataSpy(socket, &QSslSocket::readyRead);
    socket->write(jsonDoc.toJson(QJsonDocument::Compact) + '\n');
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

bool BaseTest::createRemoteConnection(const QString &token, const QString &nonce, QObject *parent)
{
    // Configure mock authenticator
    m_mockAuthenticator->setTimeoutDuration(100);
    m_mockAuthenticator->setExpectedAuthenticationError();

    QString nameConnectionOne = "Test client one";
    QUuid uuidConnectionOne = QUuid::createUuid();

    QString nameConnectionTwo = "Test client two";
    QUuid uuidConnectionTwo = QUuid::createUuid();

    //    QByteArray dataOne = "Hello from client one :-)";
    //    QByteArray dataTwo = "Hello from client two :-)";

    // Create two connection
    RemoteProxyConnection *connectionOne = new RemoteProxyConnection(uuidConnectionOne, nameConnectionOne, parent);
    connect(connectionOne, &RemoteProxyConnection::sslErrors, this, &BaseTest::ignoreConnectionSslError);

    RemoteProxyConnection *connectionTwo = new RemoteProxyConnection(uuidConnectionTwo, nameConnectionTwo, parent);
    connect(connectionTwo, &RemoteProxyConnection::sslErrors, this, &BaseTest::ignoreConnectionSslError);

    // Connect one
    QSignalSpy connectionOneReadySpy(connectionOne, &RemoteProxyConnection::ready);
    if (!connectionOne->connectServer(m_serverUrlProxyTcp)) {
        qWarning() << "Could not connect client one";
        return false;
    }

    connectionOneReadySpy.wait();
    if (connectionOneReadySpy.count() != 1) {
        qWarning() << "Could not connect client one";
        return false;
    }

    if (!connectionOne->isConnected()) {
        qWarning() << "Could not connect client one";
        return false;
    }

    // Connect two
    QSignalSpy connectionTwoReadySpy(connectionTwo, &RemoteProxyConnection::ready);
    if (!connectionTwo->connectServer(m_serverUrlProxyTcp)) {
        qWarning() << "Could not connect client two";
        return false;
    }

    connectionTwoReadySpy.wait();
    if (connectionTwoReadySpy.count() != 1) {
        qWarning() << "Could not connect client two";
        return false;
    }

    if (!connectionTwo->isConnected()) {
        qWarning() << "Could not connect client two";
        return false;
    }

    // Authenticate one
    QSignalSpy remoteConnectionEstablishedOne(connectionOne, &RemoteProxyConnection::remoteConnectionEstablished);
    QSignalSpy connectionOneAuthenticatedSpy(connectionOne, &RemoteProxyConnection::authenticated);
    if (!connectionOne->authenticate(token, nonce)) {
        qWarning() << "Could not authenticate client one";
        return false;
    }

    connectionOneAuthenticatedSpy.wait();
    if (connectionOneAuthenticatedSpy.count() != 1) {
        qWarning() << "Could not authenticate client one";
        return false;
    }

    if (connectionOne->state() != RemoteProxyConnection::StateAuthenticated) {
        qWarning() << "Could not authenticate client one";
        return false;
    }

    // Authenticate two
    QSignalSpy remoteConnectionEstablishedTwo(connectionTwo, &RemoteProxyConnection::remoteConnectionEstablished);
    QSignalSpy connectionTwoAuthenticatedSpy(connectionTwo, &RemoteProxyConnection::authenticated);
    if (!connectionTwo->authenticate(token, nonce)) {
        qWarning() << "Could not authenticate client two";
        return false;
    }

    connectionTwoAuthenticatedSpy.wait();
    if (connectionTwoAuthenticatedSpy.count() != 1) {
        qWarning() << "Could not authenticate client two";
        return false;
    }

    if (connectionTwo->state() != RemoteProxyConnection::StateAuthenticated && connectionTwo->state() != RemoteProxyConnection::StateRemoteConnected) {
        qWarning() << "Could not authenticate client two";
        return false;
    }

    // Wait for both to be connected
    if (remoteConnectionEstablishedOne.count() < 1)
        remoteConnectionEstablishedOne.wait();

    if (remoteConnectionEstablishedTwo.count() < 1)
        remoteConnectionEstablishedTwo.wait();

    if (remoteConnectionEstablishedOne.count() != 1 || remoteConnectionEstablishedTwo.count() != 1) {
        qWarning() << "Could not establish remote connection";
        return false;
    }

    if (connectionOne->state() != RemoteProxyConnection::StateRemoteConnected || connectionTwo->state() != RemoteProxyConnection::StateRemoteConnected) {
        qWarning() << "Could not establish remote connection";
        return false;
    }

    return true;
}


QVariant BaseTest::injectTcpSocketProxyData(const QByteArray &data)
{
    QSslSocket *socket = new QSslSocket(this);
    typedef void (QSslSocket:: *sslErrorsSignal)(const QList<QSslError> &);
    QObject::connect(socket, static_cast<sslErrorsSignal>(&QSslSocket::sslErrors), this, &BaseTest::sslSocketSslErrors);

    QSignalSpy spyConnection(socket, &QSslSocket::connected);
    socket->connectToHostEncrypted(Engine::instance()->tcpSocketServerProxy()->serverUrl().host(),
                                   static_cast<quint16>(Engine::instance()->tcpSocketServerProxy()->serverUrl().port()));
    spyConnection.wait();
    if (spyConnection.count() == 0) {
        return QVariant();
    }

    QSignalSpy dataSpy(socket, &QSslSocket::readyRead);
    socket->write(data + '\n');
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
    socket->write(jsonDoc.toJson(QJsonDocument::Compact) + '\n');
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
    socket->write(data + '\n');
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
    QByteArray payload = jsonDoc.toJson(QJsonDocument::Compact)  + '\n';

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
    QByteArray payload = jsonDoc.toJson(QJsonDocument::Compact)  + '\n';


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
    qRegisterMetaType<RemoteProxyConnection::State>();
    qRegisterMetaType<RemoteProxyConnection::ConnectionType>();

    m_testToken = "eyJraWQiOiJXdnFFT3prVVh5VDlINzFyRUpoNWdxRnkxNFhnR2l3SFAzVEIzUFQ1V3ZrPSIsImFsZyI6IlJT"
                  "MjU2In0.eyJzdWIiOiJmZTJmZDNlNC1hMGJhLTQ1OTUtOWRiZS00ZDkxYjRiMjFlMzUiLCJhdWQiOiI4cmpoZ"
                  "mRsZjlqZjFzdW9rMmpjcmx0ZDZ2IiwiZW1haWxfdmVyaWZpZWQiOnRydWUsImV2ZW50X2lkIjoiN2Y5NTRiNm"
                  "ItOTYyZi0xMWU4LWI0ZjItMzU5NWRiZmRiODVmIiwidG9rZW5fdXNlIjoiaWQiLCJhdXRoX3RpbWUiOjE1MzM"
                  "xOTkxMzgsImlzcyI6Imh0dHBzOlwvXC9jb2duaXRvLWlkcC5ldS13ZXN0LTEuYW1hem9uYXdzLmNvbVwvZXUt"
                  "d2VzdC0xXzZlWDZZam1YciIsImNvZ25pdG86dXNlcm5hbWUiOiIyYzE3YzUwZC0xZDFlLTRhM2UtYmFjOS0zZ"
                  "DI0YjQ1MTFiYWEiLCJleHAiOjE1MzMyMDI3MzgsImlhdCI6MTUzMzE5OTEzOCwiZW1haWwiOiJqZW5raW5zQG"
                  "d1aC5pbyJ9.hMMSvZMx7pMvV70PaUmTZOZgdez5WGX5yagRFPZojBm8jNWZND1lUmi0RFkybeD4HonDiKHxTF"
                  "_psyJoBVndgHbxYBBl3Np4gn0MxECWjvLxYzGxVBBkN24SqNUyAGkr0uFcZKkBecdtJlqNQnZN8Uk49twmODf"
                  "raRaRmGmKmRBAK1qDITpUgP6AWqH9xoJWaoDzt0kwJ3EtPxS7vL1PHqOaN8ggXA8Eq4iTCSfXU1HAXhIWJH9Y"
                  "pQbj58v1vktaAEATdmKmlzmcix-HJK9wWHRSuv3TsNa8DGxvcPOoeTu8Vql7krZ-y7Zu-s2WsgeP4VxyT80VE"
                  "T_xh6pMkOhE6g";

    qCDebug(dcApplication()) << "Init test case done.";
}

void BaseTest::cleanupTestCase()
{
    qCDebug(dcApplication()) << "Clean up test case.";
    cleanUpEngine();
}

