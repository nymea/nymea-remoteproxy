#include "engine.h"
#include "loggingcategories.h"
#include "remoteproxyconnection.h"
#include "nymea-remoteproxy-tests-online.h"

#include <QMetaType>
#include <QSignalSpy>
#include <QWebSocket>
#include <QJsonDocument>
#include <QWebSocketServer>

RemoteProxyOnlineTests::RemoteProxyOnlineTests(QObject *parent) :
    BaseTest(parent)
{
    m_authenticator = qobject_cast<Authenticator *>(m_awsAuthenticator);

}

void RemoteProxyOnlineTests::awsStaticCredentials()
{
    startServer();

    // Connect to the server (insecure disabled)
    RemoteProxyConnection *connection = new RemoteProxyConnection(QUuid::createUuid(), "Test client one", this);
    connect(connection, &RemoteProxyConnection::sslErrors, this, &BaseTest::ignoreConnectionSslError);

    // Connect to server (insecue enabled for testing)
    QSignalSpy readySpy(connection, &RemoteProxyConnection::ready);
    QVERIFY(connection->connectServer(m_serverUrl));
    readySpy.wait();
    QVERIFY(readySpy.count() == 1);
    QVERIFY(connection->isConnected());
    QVERIFY(!connection->isRemoteConnected());
    QVERIFY(connection->state() == RemoteProxyConnection::StateReady);
    QVERIFY(connection->error() == RemoteProxyConnection::ErrorNoError);
    QVERIFY(connection->serverUrl() == m_serverUrl);
    QVERIFY(connection->connectionType() == RemoteProxyConnection::ConnectionTypeWebSocket);
    QVERIFY(connection->serverName() == SERVER_NAME_STRING);
    QVERIFY(connection->proxyServerName() == Engine::instance()->serverName());
    QVERIFY(connection->proxyServerVersion() == SERVER_VERSION_STRING);
    QVERIFY(connection->proxyServerApiVersion() == API_VERSION_STRING);

    QSignalSpy errorSpy(connection, &RemoteProxyConnection::errorOccured);
    QSignalSpy spyDisconnected(connection, &RemoteProxyConnection::disconnected);

    QVERIFY(connection->authenticate("foobar"));
    errorSpy.wait();
    QVERIFY(errorSpy.count() == 1);
    QVERIFY(connection->error() == RemoteProxyConnection::ErrorProxyAuthenticationFailed);

    // Disconnect and clean up
    connection->disconnectServer();
    // FIXME: check why it waits the full time here
    spyDisconnected.wait(100);
    QVERIFY(spyDisconnected.count() == 1);
    QVERIFY(!connection->isConnected());

    connection->deleteLater();

    stopServer();
}

void RemoteProxyOnlineTests::awsDynamicCredentials()
{


}

QTEST_MAIN(RemoteProxyOnlineTests)
