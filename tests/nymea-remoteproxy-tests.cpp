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

    QSignalSpy dataSpy(socket, SIGNAL(textMessageReceived(QString)));
    socket->sendTextMessage(QString(jsonDoc.toJson(QJsonDocument::Compact)));
    dataSpy.wait();

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
    qRegisterMetaType<RemoteProxyConnection::State>();
    qRegisterMetaType<RemoteProxyConnection::ConnectionType>();

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

    QSignalSpy closedSpy(&dummyServer, &QWebSocketServer::closed);
    dummyServer.close();
    closedSpy.wait();
    QVERIFY(closedSpy.count() == 1);

    // Try again
    cleanUpEngine();
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

    QTest::newRow("success") << QUuid::createUuid().toString() << "Testclient, hello form the test!" << m_testToken
                             << 100 << Authenticator::AuthenticationErrorNoError;

    QTest::newRow("failed") << QUuid::createUuid().toString() << "Testclient, hello form the test!" << m_testToken
                            << 100 << Authenticator::AuthenticationErrorAuthenticationFailed;

    QTest::newRow("not responding") << QUuid::createUuid().toString() << "Testclient, hello form the test!" << m_testToken
                                    << 200 << Authenticator::AuthenticationErrorAuthenticationServerNotResponding;

    QTest::newRow("aborted") << QUuid::createUuid().toString() << "Testclient, hello form the test!" << m_testToken
                             << 100 << Authenticator::AuthenticationErrorAborted;

    QTest::newRow("unknown") << QUuid::createUuid().toString() << "Testclient, hello form the test!" << m_testToken
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

    // Configure moch authenticator
    m_authenticator->setTimeoutDuration(100);
    m_authenticator->setExpectedAuthenticationError();

    // Connect to the server (insecure disabled)
    RemoteProxyConnection *connection = new RemoteProxyConnection(QUuid::createUuid(), "Test client one", RemoteProxyConnection::ConnectionTypeWebSocket, this);
    connection->setInsecureConnection(true);

    // Connect to server (insecue enabled for testing)
    QSignalSpy readySpy(connection, &RemoteProxyConnection::ready);
    QVERIFY(connection->connectServer(QHostAddress::LocalHost, m_port));
    readySpy.wait();
    QVERIFY(readySpy.count() == 1);
    QVERIFY(connection->isConnected());
    QVERIFY(!connection->isRemoteConnected());
    QVERIFY(connection->state() == RemoteProxyConnection::StateReady);
    QVERIFY(connection->error() == RemoteProxyConnection::ErrorNoError);
    QVERIFY(connection->serverAddress() == QHostAddress::LocalHost);
    QVERIFY(connection->serverPort() == m_port);
    QVERIFY(connection->connectionType() == RemoteProxyConnection::ConnectionTypeWebSocket);
    QVERIFY(connection->insecureConnection() == true);
    QVERIFY(connection->serverName() == SERVER_NAME_STRING);
    QVERIFY(connection->proxyServerName() == Engine::instance()->serverName());
    QVERIFY(connection->proxyServerVersion() == SERVER_VERSION_STRING);
    QVERIFY(connection->proxyServerApiVersion() == API_VERSION_STRING);

    QSignalSpy authenticatedSpy(connection, &RemoteProxyConnection::authenticated);
    QVERIFY(connection->authenticate("foobar"));
    authenticatedSpy.wait();
    QVERIFY(authenticatedSpy.count() == 1);
    QVERIFY(connection->isConnected());
    QVERIFY(connection->isAuthenticated());
    QVERIFY(connection->state() == RemoteProxyConnection::StateWaitTunnel);

    // Disconnect and clean up
    QSignalSpy spyDisconnected(connection, &RemoteProxyConnection::disconnected);
    connection->disconnectServer();
    // FIXME: check why it waits the full time here
    spyDisconnected.wait(100);
    QVERIFY(spyDisconnected.count() == 1);
    QVERIFY(!connection->isConnected());

    connection->deleteLater();
    stopServer();
}

void RemoteProxyTests::remoteConnection()
{
    // Start the server
    startServer();

    // Configure moch authenticator
    m_authenticator->setTimeoutDuration(100);
    m_authenticator->setExpectedAuthenticationError();

    // Create two connection
    RemoteProxyConnection *connectionOne = new RemoteProxyConnection(QUuid::createUuid(), "Test client one", RemoteProxyConnection::ConnectionTypeWebSocket, this);
    connectionOne->setInsecureConnection(true);

    RemoteProxyConnection *connectionTwo = new RemoteProxyConnection(QUuid::createUuid(), "Test client two", RemoteProxyConnection::ConnectionTypeWebSocket, this);
    connectionTwo->setInsecureConnection(true);

    // Connect one
    QSignalSpy connectionOneReadySpy(connectionOne, &RemoteProxyConnection::ready);
    QVERIFY(connectionOne->connectServer(QHostAddress::LocalHost, m_port));
    connectionOneReadySpy.wait();
    QVERIFY(connectionOneReadySpy.count() == 1);
    QVERIFY(connectionOne->isConnected());

    // Connect two
    QSignalSpy connectionTwoReadySpy(connectionTwo, &RemoteProxyConnection::ready);
    QVERIFY(connectionTwo->connectServer(QHostAddress::LocalHost, m_port));
    connectionTwoReadySpy.wait();
    QVERIFY(connectionTwoReadySpy.count() == 1);
    QVERIFY(connectionTwo->isConnected());

    // Authenticate one
    QSignalSpy remoteConnectionEstablishedOne(connectionOne, &RemoteProxyConnection::remoteConnectionEstablished);
    QSignalSpy connectionOneAuthenticatedSpy(connectionOne, &RemoteProxyConnection::authenticated);
    QVERIFY(connectionOne->authenticate(m_testToken));
    connectionOneAuthenticatedSpy.wait();
    QVERIFY(connectionOneAuthenticatedSpy.count() == 1);
    QVERIFY(connectionOne->isConnected());
    QVERIFY(connectionOne->isAuthenticated());
    QVERIFY(connectionOne->state() == RemoteProxyConnection::StateWaitTunnel);

    // Authenticate two
    QSignalSpy remoteConnectionEstablishedTwo(connectionTwo, &RemoteProxyConnection::remoteConnectionEstablished);
    QSignalSpy connectionTwoAuthenticatedSpy(connectionTwo, &RemoteProxyConnection::authenticated);
    QVERIFY(connectionTwo->authenticate(m_testToken));
    connectionTwoAuthenticatedSpy.wait();
    QVERIFY(connectionTwoAuthenticatedSpy.count() == 1);
    QVERIFY(connectionTwo->isConnected());
    QVERIFY(connectionTwo->isAuthenticated());

    // Wait for both to be connected
    remoteConnectionEstablishedOne.wait();
    remoteConnectionEstablishedTwo.wait();

    QVERIFY(remoteConnectionEstablishedOne.count() == 1);
    QVERIFY(remoteConnectionEstablishedTwo.count() == 1);
    QVERIFY(connectionOne->state() == RemoteProxyConnection::StateRemoteConnected);
    QVERIFY(connectionTwo->state() == RemoteProxyConnection::StateRemoteConnected);

    // Pipe data trought the tunnel
    QSignalSpy remoteConnectionDataOne(connectionOne, &RemoteProxyConnection::dataReady);
    QSignalSpy remoteConnectionDataTwo(connectionTwo, &RemoteProxyConnection::dataReady);

    QByteArray dataOne = "Hello from client one :-)";
    QByteArray dataTwo = "Hello from client two :-)";

    connectionOne->sendData(dataOne);
    remoteConnectionDataTwo.wait();
    QVERIFY(remoteConnectionDataTwo.count() == 1);

    // verify if data is the same

    connectionTwo->sendData(dataTwo);
    remoteConnectionDataOne.wait();
    QVERIFY(remoteConnectionDataOne.count() == 1);

    // verify if data is the same

    connectionOne->deleteLater();
    connectionTwo->deleteLater();

    // Clean up
    stopServer();
}

void RemoteProxyTests::sslConfigurations()
{
    //    // Start the server
    //    startServer();

    //    // Connect to the server (insecure disabled)
    //    RemoteProxyConnection *connector = new RemoteProxyConnection(QUuid::createUuid(), "Test client one", RemoteProxyConnection::ConnectionTypeWebSocket, this);
    //    QSignalSpy spyError(connector, &RemoteProxyConnection::errorOccured);
    //    QVERIFY(connector->connectServer(QHostAddress::LocalHost, m_port));
    //    spyError.wait();
    //    QVERIFY(spyError.count() == 1);
    //    QVERIFY(connector->error() == RemoteProxyConnection::ErrorSslError);
    //    QVERIFY(connector->state() == RemoteProxyConnection::StateDisconnected);

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
    //    // Start the server
    //    startServer();

    //    // Configure result
    //    // Start the server
    //    startServer();

    //    // Configure result
    //    m_authenticator->setExpectedAuthenticationError();
    //    m_authenticator->setTimeoutDuration(6000);

    //    // Create request
    //    QVariantMap params;
    //    params.insert("uuid", "uuid");
    //    params.insert("name", "name");
    //    params.insert("token", "token");

    //    QVariant response = invokeApiCall("Authentication.Authenticate", params);
    //    qDebug() << qUtf8Printable(QJsonDocument::fromVariant(response).toJson(QJsonDocument::Indented));
    //    // Clean up
    //    stopServer();
}

QTEST_MAIN(RemoteProxyTests)
