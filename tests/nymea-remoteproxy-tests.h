#ifndef NYMEA_REMOTEPROXY_TESTS_H
#define NYMEA_REMOTEPROXY_TESTS_H

#include <QtTest>
#include <QObject>

class RemoteProxyTests : public QObject
{
    Q_OBJECT
public:
    explicit RemoteProxyTests(QObject *parent = nullptr);

private:
    void cleanUpEngine();
    void restartEngine();

protected slots:
    void initTestCase();
    void cleanupTestCase();


private slots:
    void startStopServer();




};

#endif // NYMEA_REMOTEPROXY_TESTS_H
