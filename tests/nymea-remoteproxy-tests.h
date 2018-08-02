#ifndef NYMEA_REMOTEPROXY_TESTS_H
#define NYMEA_REMOTEPROXY_TESTS_H

#include <QUrl>
#include <QtTest>
#include <QSslKey>
#include <QObject>
#include <QHostAddress>
#include <QSslCertificate>
#include <QSslConfiguration>

class RemoteProxyTests : public QObject
{
    Q_OBJECT
public:
    explicit RemoteProxyTests(QObject *parent = nullptr);

private:
    quint16 m_port = 1212;
    QHostAddress m_serverAddress = QHostAddress::LocalHost;
    QSslConfiguration m_sslConfiguration;

    void cleanUpEngine();
    void restartEngine();
    void startServer();

protected slots:
    void initTestCase();
    void cleanupTestCase();


private slots:
    void startStopServer();
    void authenticate();
    void sslConfigurations();

};

#endif // NYMEA_REMOTEPROXY_TESTS_H
