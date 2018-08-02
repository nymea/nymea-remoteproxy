#include "nymea-remoteproxy-tests.h"

#include "engine.h"
#include "loggingcategories.h"

RemoteProxyTests::RemoteProxyTests(QObject *parent) :
    QObject(parent)
{

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
