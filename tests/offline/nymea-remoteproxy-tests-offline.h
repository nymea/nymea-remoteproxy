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

#include "basetest.h"

using namespace remoteproxy;
using namespace remoteproxyclient;

class RemoteProxyTests : public BaseTest
{
    Q_OBJECT
public:
    explicit RemoteProxyTests(QObject *parent = nullptr);

private slots:
    // Basic stuff
    void startStopServer();
    void dummyAuthenticator();

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
    void remoteConnection();
    void sslConfigurations();
    void timeout();

};

#endif // NYMEA_REMOTEPROXY_TESTS_H
