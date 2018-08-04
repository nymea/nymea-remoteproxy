#include "nymea-remoteproxy-tests.h"

#include "engine.h"
#include "loggingcategories.h"
#include "remoteproxyconnector.h"

#include <QMetaType>
#include <QSignalSpy>

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
        Engine::instance()->destroy();
        QVERIFY(!Engine::exists());
    }
}

void RemoteProxyTests::restartEngine()
{
    cleanUpEngine();

    QVERIFY(Engine::instance() != nullptr);
    QVERIFY(Engine::exists());
}

void RemoteProxyTests::startServer()
{
    restartEngine();

    Engine::instance()->setAuthenticator(m_authenticator);
    Engine::instance()->setWebSocketServerPort(m_port);
    Engine::instance()->setWebSocketServerHostAddress(QHostAddress::LocalHost);
    Engine::instance()->setSslConfiguration(m_sslConfiguration);
    Engine::instance()->start();

    QVERIFY(Engine::instance()->running());

}

void RemoteProxyTests::stopServer()
{
    if (!Engine::instance()->running())
        return;

    Engine::instance()->stop();
    QVERIFY(!Engine::instance()->running());
}

void RemoteProxyTests::initTestCase()
{
    qRegisterMetaType<RemoteProxyConnector::Error>();

    qCDebug(dcApplication()) << "Init test case.";
    restartEngine();
}

void RemoteProxyTests::cleanupTestCase()
{
    qCDebug(dcApplication()) << "Clean up test case.";
    cleanUpEngine();
}

void RemoteProxyTests::authenticate()
{
//    // Start the server
//    startServer();

//    // Connect to the server
//    RemoteProxyConnector *connector = new RemoteProxyConnector(this);
//    connector->setInsecureConnection(true);

//    QSignalSpy spy(connector, &RemoteProxyConnector::connected);
//    connector->connectServer(QHostAddress::LocalHost, m_port);
//    spy.wait();

//    connector->disconnectServer();
//    connector->deleteLater();
//    Engine::instance()->stop();
}

void RemoteProxyTests::startStopServer()
{
    restartEngine();
    startServer();
    stopServer();
    cleanUpEngine();
}

void RemoteProxyTests::sslConfigurations()
{
    // Start the server
    startServer();

    // Connect to the server (insecure disabled)
    RemoteProxyConnector *connector = new RemoteProxyConnector(this);
    connector->setInsecureConnection(false);

    QSignalSpy spyError(connector, &RemoteProxyConnector::errorOccured);
    connector->connectServer(QHostAddress::LocalHost, m_port);
    spyError.wait();

    QCOMPARE(connector->error(), RemoteProxyConnector::ErrorSocketError);
    QCOMPARE(connector->socketError(), QAbstractSocket::SslHandshakeFailedError);
    QCOMPARE(connector->state(), RemoteProxyConnector::StateDisconnected);

    // Connect to server (insecue enabled)
    QSignalSpy spyConnected(connector, &RemoteProxyConnector::connected);
    connector->setInsecureConnection(true);
    connector->connectServer(QHostAddress::LocalHost, m_port);
    spyConnected.wait();

    QVERIFY(connector->isConnected());

    // Disconnect and clean up
    connector->disconnectServer();
    QVERIFY(!connector->isConnected());

    connector->deleteLater();
    Engine::instance()->stop();
}

QTEST_MAIN(RemoteProxyTests)
