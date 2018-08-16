#include <QUrl>
#include <QCoreApplication>
#include <QLoggingCategory>
#include <QCommandLineParser>
#include <QCommandLineOption>

int main(int argc, char *argv[])
{

    QCoreApplication application(argc, argv);
    application.setApplicationName(SERVER_NAME_STRING);
    application.setOrganizationName("nymea");
    application.setApplicationVersion(SERVER_VERSION_STRING);

    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addVersionOption();
    parser.setApplicationDescription(QString("\nThe nymea remote proxy server client application. This client allowes to test "
                                             "a server application as client perspective.\n\n"
                                             "Server version: %1\n"
                                             "API version: %2\n\n"
                                             "Copyright %3 2018 Simon Stürz <simon.stuerz@guh.io>\n")
                                     .arg(SERVER_VERSION_STRING)
                                     .arg(API_VERSION_STRING)
                                     .arg(QChar(0xA9)));




    QCommandLineOption tokenOption(QStringList() << "s" << "socket", "The AWS token for authentication. Default /tmp/", "socket");
    parser.addOption(tokenOption);

    parser.process(application);

    return application.exec();
}
