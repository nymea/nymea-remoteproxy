#include "remoteproxyserverapplication.h"
#include "loggingcategories.h"
#include "engine.h"

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <cxxabi.h>

using namespace remoteproxy;

static void catchUnixSignals(const std::vector<int>& quitSignals, const std::vector<int>& ignoreSignals = std::vector<int>())
{
    auto handler = [](int sig) ->void {
        switch (sig) {
        case SIGQUIT:
            qCDebug(dcApplication()) << "Cought SIGQUIT quit signal...";
            break;
        case SIGINT:
            qCDebug(dcApplication()) << "Cought SIGINT quit signal...";
            break;
        case SIGTERM:
            qCDebug(dcApplication()) << "Cought SIGTERM quit signal...";
            break;
        case SIGHUP:
            qCDebug(dcApplication()) << "Cought SIGHUP quit signal...";
            break;
        case SIGSEGV: {
            qCDebug(dcApplication()) << "Cought SIGSEGV signal. Segmentation fault!";
            exit(1);
        }
        default:
            break;
        }

        qCDebug(dcApplication()) << "==========================================================";
        qCDebug(dcApplication) << "Shutting down nymea-remoteproxy";
        qCDebug(dcApplication()) << "==========================================================";

        Engine::instance()->destroy();
        RemoteProxyServerApplication::quit();
    };

    // all these signals will be ignored.
    for (int sig : ignoreSignals)
        signal(sig, SIG_IGN);

    for (int sig : quitSignals)
        signal(sig, handler);
}

RemoteProxyServerApplication::RemoteProxyServerApplication(int &argc, char **argv) :
    QCoreApplication(argc, argv)
{
    catchUnixSignals({SIGQUIT, SIGINT, SIGTERM, SIGHUP, SIGSEGV});
}
