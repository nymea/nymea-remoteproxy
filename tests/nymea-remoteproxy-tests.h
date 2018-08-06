#ifndef NYMEA_REMOTEPROXY_TESTS_H
#define NYMEA_REMOTEPROXY_TESTS_H

#include <QUrl>
#include <QtTest>
#include <QSslKey>
#include <QObject>
#include <QHostAddress>
#include <QSslCertificate>
#include <QSslConfiguration>

#include "mockauthenticator.h"

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

    void cleanUpEngine();
    void restartEngine();
    void startEngine();
    void startServer();
    void stopServer();

protected slots:
    void initTestCase();
    void cleanupTestCase();


private slots:
    void startStopServer();
    void webserverConnectionBlocked();
    void webserverSocketVersion();
    void webserverConnection();
    void sslConfigurations();
    void authenticate();

};

#endif // NYMEA_REMOTEPROXY_TESTS_H
