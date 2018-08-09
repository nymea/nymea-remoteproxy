#ifndef NYMEA_REMOTEPROXY_TESTS_H
#define NYMEA_REMOTEPROXY_TESTS_H

#include <QUrl>
#include <QtTest>
#include <QSslKey>
#include <QObject>
#include <QWebSocket>
#include <QHostAddress>
#include <QSslCertificate>
#include <QSslConfiguration>

#include "jsonrpc/jsontypes.h"
#include "mockauthenticator.h"
#include "remoteproxyconnection.h"

using namespace remoteproxy;
using namespace remoteproxyclient;

class RemoteProxyTests : public QObject
{
    Q_OBJECT
public:
    explicit RemoteProxyTests(QObject *parent = nullptr);

private:
    quint16 m_port = 1212;
    QHostAddress m_serverAddress = QHostAddress::LocalHost;
    QSslConfiguration m_sslConfiguration;
    MockAuthenticator *m_authenticator = nullptr;

    int m_commandCounter = 0;

    void cleanUpEngine();
    void restartEngine();
    void startEngine();
    void startServer();
    void stopServer();

    QVariant invokeApiCall(const QString &method, const QVariantMap params = QVariantMap(), bool remainsConnected = true);
    QVariant injectSocketData(const QByteArray &data);

protected slots:
    void initTestCase();
    void cleanupTestCase();

private slots:
    // Basic stuff
    void startStopServer();

    // WebSocket connection
    void webserverConnectionBlocked();
    void webserverConnection();

    // Api
    void getIntrospect();
    void getHello();

    void apiBasicCalls_data();
    void apiBasicCalls();

    void authenticate_data();
    void authenticate();

    // Client lib
    void clientConnection();
    void sslConfigurations();

    void timeout();

public slots:
    void sslErrors(const QList<QSslError> &) {
        QWebSocket *socket = static_cast<QWebSocket*>(sender());
        socket->ignoreSslErrors();
    }

    inline void verifyError(const QVariant &response, const QString &fieldName, const QString &error)
    {
        QJsonDocument jsonDoc = QJsonDocument::fromVariant(response);
        QVERIFY2(response.toMap().value("status").toString() == QString("success"),
                 QString("\nExpected status: \"success\"\nGot: %2\nFull message: %3")
                 .arg(response.toMap().value("status").toString())
                 .arg(jsonDoc.toJson().data())
                 .toLatin1().data());
        QVERIFY2(response.toMap().value("params").toMap().value(fieldName).toString() == error,
                 QString("\nExpected: %1\nGot: %2\nFull message: %3\n")
                 .arg(error)
                 .arg(response.toMap().value("params").toMap().value(fieldName).toString())
                 .arg(jsonDoc.toJson().data())
                 .toLatin1().data());
    }


    inline void verifyAuthenticationError(const QVariant &response, Authenticator::AuthenticationError error = Authenticator::AuthenticationErrorNoError) {
        verifyError(response, "authenticationError", JsonTypes::authenticationErrorToString(error));
    }


};

#endif // NYMEA_REMOTEPROXY_TESTS_H
