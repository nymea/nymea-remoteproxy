#include "nymea-remoteproxy-tests.h"

#include "engine.h"
#include "loggingcategories.h"
#include "remoteproxyconnection.h"

#include <QMetaType>
#include <QSignalSpy>
#include <QWebSocket>
#include <QJsonDocument>
#include <QWebSocketServer>

RemoteProxyTests::RemoteProxyTests(QObject *parent) :
    QObject(parent)
{
    QFile certificateFile(":/test-certificate.crt");
    if (!certificateFile.open(QIODevice::ReadOnly)) {
        qWarning() << "Could not open resource file" << certificateFile.fileName();
        exit(1);
    }

    QByteArray certificateData = certificateFile.readAll();
    //qDebug() << "Certificate:" << endl << qUtf8Printable(certificateData);

    QFile keyFile(":/test-certificate.key");
    if (!keyFile.open(QIODevice::ReadOnly)) {
        qWarning() << "Could not open resource file" << keyFile.fileName();
        exit(1);
    }

    QByteArray keyData = keyFile.readAll();
    //qDebug() << "Certificate key:" << endl << qUtf8Printable(keyData);
    m_authenticator = new MockAuthenticator(this);

    m_sslConfiguration.setPrivateKey(QSslKey(keyData,  QSsl::Rsa, QSsl::Pem, QSsl::PrivateKey));
    m_sslConfiguration.setLocalCertificate(QSslCertificate(certificateData, QSsl::Pem));
    m_sslConfiguration.setPeerVerifyMode(QSslSocket::VerifyNone);
    m_sslConfiguration.setProtocol(QSsl::TlsV1_2OrLater);
}

void RemoteProxyTests::cleanUpEngine()
{
    if (Engine::exists()) {
        Engine::instance()->stop();
        Engine::instance()->destroy();
        QVERIFY(!Engine::exists());
    }
}

void RemoteProxyTests::restartEngine()
{
    cleanUpEngine();
    startEngine();
}

void RemoteProxyTests::startEngine()
{
    if (!Engine::exists()) {
        QString serverName = "nymea-remoteproxy-testserver";
        Engine::instance()->setAuthenticator(m_authenticator);
        Engine::instance()->setAuthenticationServerUrl(QUrl("https://localhost"));
        Engine::instance()->setServerName(serverName);
        Engine::instance()->setWebSocketServerPort(m_port);
        Engine::instance()->setWebSocketServerHostAddress(QHostAddress::LocalHost);
        Engine::instance()->setSslConfiguration(m_sslConfiguration);

        QVERIFY(Engine::exists());
        QVERIFY(Engine::instance()->serverName() == serverName);
    }
}

void RemoteProxyTests::startServer()
{
    startEngine();

    if (!Engine::instance()->running()) {
        QSignalSpy runningSpy(Engine::instance(), &Engine::runningChanged);
        Engine::instance()->start();
        runningSpy.wait();
        QVERIFY(runningSpy.count() == 1);
    }

    QVERIFY(Engine::instance()->running());
    QVERIFY(Engine::instance()->webSocketServer()->running());
}

void RemoteProxyTests::stopServer()
{
    if (!Engine::instance()->running())
        return;

    Engine::instance()->stop();
    QVERIFY(!Engine::instance()->running());
}

QVariant RemoteProxyTests::invokeApiCall(const QString &method, const QVariantMap params, bool remainsConnected)
{
    Q_UNUSED(remainsConnected)

    QVariantMap request;
    request.insert("id", m_commandCounter);
    request.insert("method", method);
    request.insert("params", params);
    QJsonDocument jsonDoc = QJsonDocument::fromVariant(request);

    QWebSocket *socket = new QWebSocket("proxy-testclient", QWebSocketProtocol::Version13);
    connect(socket, &QWebSocket::sslErrors, this, &RemoteProxyTests::sslErrors);
    QSignalSpy spyConnection(socket, SIGNAL(connected()));
    socket->open(Engine::instance()->webSocketServer()->serverUrl());
    spyConnection.wait();
    if (spyConnection.count() == 0) {
        return QVariant();
    }

    //    QSignalSpy disconnectedSpy(socket, SIGNAL(disconnected()));
    QSignalSpy dataSpy(socket, SIGNAL(textMessageReceived(QString)));
    socket->sendTextMessage(QString(jsonDoc.toJson(QJsonDocument::Compact)));
    dataSpy.wait();
    //    if (remainsConnected) {
    //        disconnectedSpy.wait(1000);
    //        if (socket->state() != QAbstractSocket::UnconnectedState) {
    //            qWarning() << "!!!!!!!!!!!!! socket still connected but should be disconnected!";
    //        }
    //    } else {
    //        disconnectedSpy.wait();
    //        if (socket->state() != QAbstractSocket::ConnectedState) {
    //            qWarning() << "!!!!!!!!!!!!! socket not connected but should be!";
    //        }
    //    }

    socket->close();
    socket->deleteLater();

    for (int i = 0; i < dataSpy.count(); i++) {
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

QVariant RemoteProxyTests::injectSocketData(const QByteArray &data)
{
    QWebSocket *socket = new QWebSocket("proxy-testclient", QWebSocketProtocol::Version13);
    connect(socket, &QWebSocket::sslErrors, this, &RemoteProxyTests::sslErrors);
    QSignalSpy spyConnection(socket, SIGNAL(connected()));
    socket->open(Engine::instance()->webSocketServer()->serverUrl());
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

void RemoteProxyTests::initTestCase()
{
    qRegisterMetaType<RemoteProxyConnection::Error>();

    qCDebug(dcApplication()) << "Init test case.";
    restartEngine();
}

void RemoteProxyTests::cleanupTestCase()
{
    qCDebug(dcApplication()) << "Clean up test case.";
    cleanUpEngine();
}

void RemoteProxyTests::startStopServer()
{
    startServer();
    stopServer();
}

void RemoteProxyTests::webserverConnectionBlocked()
{
    // Create a dummy server which blocks the port
    QWebSocketServer dummyServer("dummy-server", QWebSocketServer::NonSecureMode);
    dummyServer.listen(QHostAddress::LocalHost, m_port);

    // Start proxy webserver
    Engine::instance()->setWebSocketServerPort(m_port);
    Engine::instance()->setAuthenticator(m_authenticator);
    Engine::instance()->setWebSocketServerHostAddress(QHostAddress::LocalHost);
    Engine::instance()->setSslConfiguration(m_sslConfiguration);

    QSignalSpy runningSpy(Engine::instance(), &Engine::runningChanged);
    Engine::instance()->start();
    runningSpy.wait();

    QVERIFY(runningSpy.count() == 1);

    // Make sure the server is not running
    QVERIFY(Engine::instance()->running());

    // Make sure the websocket server is not running
    QVERIFY(!Engine::instance()->webSocketServer()->running());

    dummyServer.close();

    // Try again
    startServer();

    // Clean up
    stopServer();
}

void RemoteProxyTests::webserverConnection()
{

}

void RemoteProxyTests::getIntrospect()
{
    // Start the server
    startServer();

    QVariant response = invokeApiCall("RemoteProxy.Introspect");
    qDebug() << qUtf8Printable(QJsonDocument::fromVariant(response).toJson(QJsonDocument::Indented));

    // Clean up
    stopServer();
}

void RemoteProxyTests::getHello()
{
    // Start the server
    startServer();

    QVariant response = invokeApiCall("RemoteProxy.Hello");
    qDebug() << qUtf8Printable(QJsonDocument::fromVariant(response).toJson(QJsonDocument::Indented));

    // Clean up
    stopServer();
}

void RemoteProxyTests::apiBasicCalls_data()
{
    QTest::addColumn<QByteArray>("data");
    QTest::addColumn<bool>("valid");

    QTest::newRow("valid call") << QByteArray("{\"id\":42, \"method\":\"RemoteProxy.Hello\"}") << true;
    QTest::newRow("missing id") << QByteArray("{\"method\":\"RemoteProxy.Hello\"}")<< false;
    QTest::newRow("missing method") << QByteArray("{\"id\":42}") << false;
    QTest::newRow("borked") << QByteArray("{\"id\":42, \"method\":\"RemoteProx")<< false;
    QTest::newRow("invalid function") << QByteArray("{\"id\":42, \"method\":\"RemoteProxy.Explode\"}") << false;
    QTest::newRow("invalid namespace") << QByteArray("{\"id\":42, \"method\":\"ProxyRemote.Hello\"}") << false;
    QTest::newRow("missing dot") << QByteArray("{\"id\":42, \"method\":\"RemoteProxyHello\"}") << false;
    QTest::newRow("invalid params") << QByteArray("{\"id\":42, \"method\":\"RemoteProxy.Hello\", \"params\":{\"törööö\":\"chooo-chooo\"}}") << false;
}

void RemoteProxyTests::apiBasicCalls()
{
    QFETCH(QByteArray, data);
    QFETCH(bool, valid);

    // Start the server
    startServer();

    QVariant response = injectSocketData(data);
    if (valid) {
        QVERIFY2(response.toMap().value("status").toString() == "success", "Call wasn't parsed correctly by nymea.");
    }

    // Clean up
    stopServer();
}

void RemoteProxyTests::authenticate_data()
{
    QTest::addColumn<QString>("uuid");
    QTest::addColumn<QString>("name");
    QTest::addColumn<QString>("token");

    QTest::addColumn<int>("timeout");
    QTest::addColumn<Authenticator::AuthenticationError>("expectedError");

    QTest::newRow("success") << QUuid::createUuid().toString() << "Testclient, hello form the test!" << "Hh5JrkdFVstVVyCnfE3vVT3zG_JTacXEhPLZhCei"
                             << 100 << Authenticator::AuthenticationErrorNoError;

    QTest::newRow("failed") << QUuid::createUuid().toString() << "Testclient, hello form the test!" << "invalid_token_42"
                            << 100 << Authenticator::AuthenticationErrorAuthenticationFailed;

    QTest::newRow("not responding") << QUuid::createUuid().toString() << "Testclient, hello form the test!" << "invalid_token_42"
                                    << 200 << Authenticator::AuthenticationErrorAuthenticationServerNotResponding;

    QTest::newRow("aborted") << QUuid::createUuid().toString() << "Testclient, hello form the test!" << "invalid_token_42"
                             << 100 << Authenticator::AuthenticationErrorAborted;

    QTest::newRow("unknown") << QUuid::createUuid().toString() << "Testclient, hello form the test!" << "invalid_token_42"
                             << 100 << Authenticator::AuthenticationErrorUnknown;

}

void RemoteProxyTests::authenticate()
{
    QFETCH(QString, uuid);
    QFETCH(QString, name);
    QFETCH(QString, token);
    QFETCH(int, timeout);
    QFETCH(Authenticator::AuthenticationError, expectedError);

    // Start the server
    startServer();

    // Configure result
    m_authenticator->setExpectedAuthenticationError(expectedError);
    m_authenticator->setTimeoutDuration(timeout);

    // Create request
    QVariantMap params;
    params.insert("uuid", uuid);
    params.insert("name", name);
    params.insert("token", token);

    QVariant response = invokeApiCall("Authentication.Authenticate", params);
    qDebug() << qUtf8Printable(QJsonDocument::fromVariant(response).toJson(QJsonDocument::Indented));
    verifyAuthenticationError(response, expectedError);

    // Clean up
    stopServer();
}

void RemoteProxyTests::clientConnection()
{
    // Start the server
    startServer();

    // Connect to the server (insecure disabled)
    RemoteProxyConnection *connectorOne = new RemoteProxyConnection(RemoteProxyConnection::ConnectionTypeWebSocket, this);
    connectorOne->setInsecureConnection(true);

    // Connect to server (insecue enabled for testing)
    QSignalSpy spyConnected(connectorOne, &RemoteProxyConnection::connected);
    connectorOne->connectServer(QHostAddress::LocalHost, m_port);
    spyConnected.wait();
    //QVERIFY(connectorOne->isConnected());

    // Disconnect and clean up
    QSignalSpy spyDisconnected(connectorOne, &RemoteProxyConnection::disconnected);
    connectorOne->disconnectServer();
    spyDisconnected.wait();
    QVERIFY(!connectorOne->isConnected());

    connectorOne->deleteLater();
    stopServer();
}

void RemoteProxyTests::sslConfigurations()
{
//    // Start the server
//    startServer();

//    // Connect to the server (insecure disabled)
//    RemoteProxyConnection *connector = new RemoteProxyConnection(RemoteProxyConnection::ConnectionTypeWebSocket, this);
//    connector->setInsecureConnection(false);

//    QSignalSpy spyError(connector, &RemoteProxyConnection::errorOccured);
//    connector->connectServer(QHostAddress::LocalHost, m_port);
//    spyError.wait();

//    QCOMPARE(connector->error(), RemoteProxyConnection::ErrorSslError);
//    QCOMPARE(connector->state(), RemoteProxyConnection::StateDisconnected);

//    // Connect to server (insecue enabled)
//    QSignalSpy spyConnected(connector, &RemoteProxyConnection::connected);
//    connector->setInsecureConnection(true);
//    connector->connectServer(QHostAddress::LocalHost, m_port);
//    spyConnected.wait();

//    QVERIFY(connector->isConnected());

//    // Disconnect and clean up
//    connector->disconnectServer();
//    QVERIFY(!connector->isConnected());

//    connector->deleteLater();
//    stopServer();
}

void RemoteProxyTests::timeout()
{
    // Start the server
    startServer();

    // Configure result
    // Start the server
    startServer();

    // Configure result
    m_authenticator->setExpectedAuthenticationError();
    m_authenticator->setTimeoutDuration(6000);

    // Create request
    QVariantMap params;
    params.insert("uuid", "uuid");
    params.insert("name", "name");
    params.insert("token", "token");

    QVariant response = invokeApiCall("Authentication.Authenticate", params);
    qDebug() << qUtf8Printable(QJsonDocument::fromVariant(response).toJson(QJsonDocument::Indented));
    // Clean up
    stopServer();
}

QTEST_MAIN(RemoteProxyTests)
