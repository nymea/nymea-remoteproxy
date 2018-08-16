#include <QDir>
#include <QUrl>
#include <QtDebug>
#include <QSslKey>
#include <QDateTime>
#include <QFileInfo>
#include <QTextStream>
#include <QMessageLogger>
#include <QSslCertificate>
#include <QCoreApplication>
#include <QCoreApplication>
#include <QLoggingCategory>
#include <QSslConfiguration>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QStandardPaths>

#include <stdio.h>
#include <unistd.h>

#include "engine.h"
#include "loggingcategories.h"
#include "proxyconfiguration.h"
#include "authentication/awsauthenticator.h"
#include "authentication/dummyauthenticator.h"

using namespace remoteproxy;

static QHash<QString, bool> s_loggingFilters;

static QFile s_logFile;
static bool s_loggingEnabled = false;

static const char *const normal = "\033[0m";
static const char *const warning = "\e[33m";
static const char *const error = "\e[31m";

static void loggingCategoryFilter(QLoggingCategory *category)
{
    if (s_loggingFilters.contains(category->categoryName())) {
        bool debugEnabled = s_loggingFilters.value(category->categoryName());
        category->setEnabled(QtDebugMsg, debugEnabled);
        category->setEnabled(QtWarningMsg, debugEnabled || s_loggingFilters.value("Warnings"));
    } else {
        // Enable default debug output
        category->setEnabled(QtDebugMsg, true);
        category->setEnabled(QtWarningMsg, s_loggingFilters.value("Warnings"));
    }
}

static void consoleLogHandler(QtMsgType type, const QMessageLogContext& context, const QString& message)
{
    QString messageString;
    QString timeString = QDateTime::currentDateTime().toString("yyyy.MM.dd hh:mm:ss.zzz");
    switch (type) {
    case QtInfoMsg:
        messageString = QString(" I %1 | %2: %3").arg(timeString).arg(context.category).arg(message);
        fprintf(stdout, " I | %s: %s\n", context.category, message.toUtf8().data());
        break;
    case QtDebugMsg:
        messageString = QString(" I %1 | %2: %3").arg(timeString).arg(context.category).arg(message);
        fprintf(stdout, " I | %s: %s\n", context.category, message.toUtf8().data());
        break;
    case QtWarningMsg:
        messageString = QString(" W %1 | %2: %3").arg(timeString).arg(context.category).arg(message);
        fprintf(stdout, "%s W | %s: %s%s\n", warning, context.category, message.toUtf8().data(), normal);
        break;
    case QtCriticalMsg:
        messageString = QString(" C %1 | %2: %3").arg(timeString).arg(context.category).arg(message);
        fprintf(stdout, "%s C | %s: %s%s\n", error, context.category, message.toUtf8().data(), normal);
        break;
    case QtFatalMsg:
        messageString = QString(" F %1 | %2: %3").arg(timeString).arg(context.category).arg(message);
        fprintf(stdout, "%s F | %s: %s%s\n", error, context.category, message.toUtf8().data(), normal);
        break;
    }
    fflush(stdout);

    if (s_logFile.isOpen()) {
        QTextStream textStream(&s_logFile);
        textStream << messageString << endl;
    }
}


int main(int argc, char *argv[])
{
    qInstallMessageHandler(consoleLogHandler);

    QCoreApplication application(argc, argv);
    application.setApplicationName(SERVER_NAME_STRING);
    application.setOrganizationName("nymea");
    application.setApplicationVersion(SERVER_VERSION_STRING);

    s_loggingFilters.insert("Application", true);
    s_loggingFilters.insert("Engine", true);
    s_loggingFilters.insert("JsonRpc", true);
    s_loggingFilters.insert("WebSocketServer", true);
    s_loggingFilters.insert("Authentication", true);
    s_loggingFilters.insert("ProxyServer", true);
    s_loggingFilters.insert("MonitorServer", true);

    // Only with verbose enabled
    s_loggingFilters.insert("JsonRpcTraffic", false);
    s_loggingFilters.insert("ProxyServerTraffic", false);
    s_loggingFilters.insert("AuthenticationProcess", false);
    s_loggingFilters.insert("WebSocketServerTraffic", false);

    QString configFile = "/etc/nymea/nymea-remoteproxy.conf";

    // command line parser
    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addVersionOption();
    parser.setApplicationDescription(QString("\nThe nymea remote proxy server. This server allowes nymea-cloud users and "
                                             "registered nymea deamons to establish a tunnel connection.\n\n"
                                             "Version: %1\n"
                                             "API version: %2\n\n"
                                             "Copyright %3 2018 Simon Stürz <simon.stuerz@guh.io>\n")
                                     .arg(SERVER_VERSION_STRING)
                                     .arg(API_VERSION_STRING)
                                     .arg(QChar(0xA9)));

    QCommandLineOption logfileOption(QStringList() << "l" << "logging", "Write log file to the given logfile.",
                                     "logfile", "/var/log/nymea-remoteproxy.log");
    parser.addOption(logfileOption);

    QCommandLineOption developmentOption(QStringList() << "d" << "development", "Enable the development mode. This enabled the server "
                                                                                "assumes there are static AWS credentials provided to aws-cli.");
    parser.addOption(developmentOption);

    QCommandLineOption mockAuthenticatorOption(QStringList() << "m" << "mock-authenticator", "Start the server using a mock authenticator which returns always true.");
    parser.addOption(mockAuthenticatorOption);

    QCommandLineOption configOption(QStringList() << "c" <<"configuration", "The path to the proxy server configuration file. The default is " + configFile, "configuration");
    configOption.setDefaultValue(configFile);
    parser.addOption(configOption);

    QCommandLineOption verboseOption(QStringList() << "verbose", "Print more verbose.");
    parser.addOption(verboseOption);

    parser.process(application);

    // Create a default configuration
    ProxyConfiguration *configuration = new ProxyConfiguration(nullptr);
    if (parser.isSet(configOption))
        configFile = parser.value(configOption);

    qCDebug(dcApplication()) << "Loading configuration file from" << configFile;
    if (!configuration->loadConfiguration(parser.value(configOption))) {
        qCCritical(dcApplication()) << "Invalid configuration file passed" << parser.value(configOption);
        exit(-1);
    }

    if (parser.isSet(logfileOption)) {
        configuration->setWriteLogFile(true);
        configuration->setLogFileName(parser.value(logfileOption));
    }

    if (parser.isSet(verboseOption)) {
        s_loggingFilters["JsonRpcTraffic"] = true;
        s_loggingFilters["ProxyServerTraffic"] = true;
        s_loggingFilters["AuthenticationProcess"] = true;
        s_loggingFilters["WebSocketServerTraffic"] = true;
    }
    QLoggingCategory::installFilter(loggingCategoryFilter);

    // Open logfile if configured
    if (configuration->writeLogFile()) {
        s_loggingEnabled = true;
        QFileInfo fi(configuration->logFileName());
        QDir dir(fi.absolutePath());
        if (!dir.exists() && !dir.mkpath(dir.absolutePath())) {
            qCWarning(dcApplication()) << "Error opening log file" << configuration->logFileName();
            exit(-1);
        }
        s_logFile.setFileName(configuration->logFileName());
        if (!s_logFile.open(QFile::WriteOnly | QFile::Append)) {
            qWarning() << "Error opening log file" << configuration->logFileName();
            exit(-1);
        }
    }

    // Verify webserver configuration
    if (configuration->webSocketServerHost().isNull()) {
        qCCritical(dcApplication()) << "Invalid web socket host address passed.";
        exit(-1);
    }

    // Verify tcp server configuration
    if (configuration->tcpServerHost().isNull()) {
        qCCritical(dcApplication()) << "Invalid TCP server host address passed.";
        exit(-1);
    }

    // Verify SSL configuration
    if (configuration->sslConfiguration().isNull()) {
        qCCritical(dcApplication()) << "No SSL configuration specified. The server does not suppoert insecure connections.";
        exit(-1);
    }

    qCDebug(dcApplication()) << "==========================================================";
    qCDebug(dcApplication()) << "Starting" << application.applicationName() << application.applicationVersion();
    qCDebug(dcApplication()) << "==========================================================";
    if (parser.isSet(developmentOption)) {
        qCWarning(dcApplication()) << "##########################################################";
        qCWarning(dcApplication()) << "#                   DEVELOPMENT MODE                     #";
        qCWarning(dcApplication()) << "##########################################################";
    }

    if (s_loggingEnabled)
        qCDebug(dcApplication()) << "Logging enabled. Writing logs to" << s_logFile.fileName();

    qCDebug(dcApplication()) << "Using ssl version:" << QSslSocket::sslLibraryVersionString();

    Authenticator *authenticator = nullptr;
    if (parser.isSet(mockAuthenticatorOption)) {
        authenticator = qobject_cast<Authenticator *>(new DummyAuthenticator(nullptr));
    } else {
        // Create default authenticator
        authenticator = qobject_cast<Authenticator *>(new AwsAuthenticator(nullptr));
    }

    // Configure and start the engines
    Engine::instance()->setAuthenticator(authenticator);
    Engine::instance()->setDeveloperModeEnabled(parser.isSet(developmentOption));
    Engine::instance()->start(configuration);

    return application.exec();
}
