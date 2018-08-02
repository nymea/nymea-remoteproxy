#include "nymea-remoteproxy-tests.h"

#include "engine.h"
#include "loggingcategories.h"

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

    Engine::instance()->setWebSocketServerPort(m_port);
    Engine::instance()->setWebSocketServerHostAddress(QHostAddress::LocalHost);
    Engine::instance()->setSslConfiguration(m_sslConfiguration);
    Engine::instance()->start();

}

void RemoteProxyTests::initTestCase()
{
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
    startServer();


    Engine::instance()->stop();
}

void RemoteProxyTests::startStopServer()
{
    cleanUpEngine();

    QVERIFY(Engine::instance() != nullptr);
    QVERIFY(Engine::exists());

    Engine::instance()->start();
    QVERIFY(Engine::instance()->running());

    Engine::instance()->stop();
    QVERIFY(!Engine::instance()->running());

    Engine::instance()->destroy();
    QVERIFY(!Engine::exists());
}

QTEST_MAIN(RemoteProxyTests)
