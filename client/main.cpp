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

int main(int argc, char *argv[])
{

    QCoreApplication application(argc, argv);
    application.setApplicationName(SERVER_NAME_STRING);
    application.setOrganizationName("guh");
    application.setApplicationVersion(SERVER_VERSION_STRING);

    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addVersionOption();
    parser.setApplicationDescription(QString("\nThe nymea remote proxy server. This server allowes nymea-cloud users and "
                                             "registered nymea deamons to establish a tunnel connection.\n\n"
                                             "Server version: %1\n"
                                             "API version: %2\n\n"
                                             "Copyright %3 2018 Simon Stürz <simon.stuerz@guh.io>\n")
                                     .arg(SERVER_VERSION_STRING)
                                     .arg(API_VERSION_STRING)
                                     .arg(QChar(0xA9)));


    QCommandLineOption tokenOption(QStringList() << "t" << "token", "The AWS token for authentication.", "token");
    parser.addOption(tokenOption);


    parser.process(application);



    return application.exec();
}
