#ifndef NYMEA_REMOTEPROXY_TESTS_ONLINE_H
#define NYMEA_REMOTEPROXY_TESTS_ONLINE_H

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

class RemoteProxyOnlineTests : public BaseTest
{
    Q_OBJECT
public:
    explicit RemoteProxyOnlineTests(QObject *parent = nullptr);

private slots:
    void awsStaticCredentials();
    void awsDynamicCredentials();


};

#endif // NYMEA_REMOTEPROXY_TESTS_ONLINE_H
