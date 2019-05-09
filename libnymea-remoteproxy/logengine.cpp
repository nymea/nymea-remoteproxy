/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                               *
 * Copyright (C) 2018 Simon Stürz <simon.stuerz@guh.io>                          *
 *                                                                               *
 * This file is part of nymea-remoteproxy.                                       *
 *                                                                               *
 * This program is free software: you can redistribute it and/or modify          *
 * it under the terms of the GNU General Public License as published by          *
 * the Free Software Foundation, either version 3 of the License, or             *
 * (at your option) any later version.                                           *
 *                                                                               *
 * This program is distributed in the hope that it will be useful,               *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of                *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                 *
 * GNU General Public License for more details.                                  *
 *                                                                               *
 * You should have received a copy of the GNU General Public License             *
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.         *
 *                                                                               *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "logengine.h"
#include "loggingcategories.h"

#include <QDateTime>
#include <QCryptographicHash>

namespace remoteproxy {

LogEngine::LogEngine(QObject *parent) : QObject(parent)
{
    m_currentDay = QDateTime::currentDateTime().date().day();
    m_tunnelsFileName = "/var/log/nymea-remoteproxy-tunnels";
    m_statisticsFileName = "/var/log/nymea-remoteproxy-statistics";
}

LogEngine::~LogEngine()
{
    disable();
}

void LogEngine::logTunnel(const TunnelConnection &tunnel)
{
    if (!m_tunnelsFile.isOpen())
        return;

    // <timestamp> <tunnel creation timestamp> <user name> <first client address> <second client addrees> <total tunnel traffic>

    QStringList logString;
    logString << createTimestamp();
    logString << QString::number(tunnel.creationTime());
    logString << QString::fromUtf8(QCryptographicHash::hash(tunnel.clientOne()->userName().toLatin1(), QCryptographicHash::Sha3_256).toHex());
    logString << tunnel.clientOne()->peerAddress().toString();
    logString << tunnel.clientTwo()->peerAddress().toString();
    logString << QString::number(tunnel.clientOne()->rxDataCount() + tunnel.clientOne()->txDataCount());

    qCDebug(dcLogEngine()) << "Logging tunnel" << logString;

    QTextStream textStream(&m_tunnelsFile);
    textStream << logString.join(" ") << endl;
}

void LogEngine::logStatistics(int tunnelCount, int connectionCount, int troughput)
{
    if (!m_statisticsFile.isOpen())
        return;

    // <timestamp> <current tunnel count> <current connection count> <current troughput B/s>
    QStringList logString;
    logString << createTimestamp();
    logString << QString::number(tunnelCount);
    logString << QString::number(connectionCount);
    logString << QString::number(troughput);

    qCDebug(dcLogEngine()) << "Logging statisitcs" << logString;

    QTextStream textStream(&m_statisticsFile);
    textStream << logString.join(" ") << endl;

    // Check if we have to rotate the logfile
    if (m_currentDay != QDateTime::currentDateTime().date().day()) {
        // Day changed
        m_currentDay = QDateTime::currentDateTime().date().day();
        rotateLogs();
    }
}

void LogEngine::rotateLogs()
{
    qCDebug(dcApplication()) << "Rotate log files.";

    // Close the log files
    if (m_tunnelsFile.isOpen())
        m_tunnelsFile.close();

    if (m_statisticsFile.isOpen())
        m_statisticsFile.close();

    // Rename the current files
    QString postfix =  "-" + QDateTime::currentDateTime().toString("yyyyMMddhhmmss") + ".log";

    m_tunnelsFile.rename(m_tunnelsFileName + postfix);
    qCDebug(dcApplication()) << "Rotate logfile" << m_tunnelsFile.fileName();

    m_statisticsFile.rename(m_statisticsFileName + postfix);
    qCDebug(dcApplication()) << "Rotate logfile" << m_statisticsFile.fileName();

    // Reopen the file with the default log file name
    m_statisticsFile.setFileName(m_statisticsFileName + ".log");
    if (!m_statisticsFile.open(QIODevice::WriteOnly | QIODevice::Append)) {
        qCDebug(dcApplication()) << "Could not open logfile" << m_statisticsFile.fileName();
    }

    m_tunnelsFile.setFileName(m_tunnelsFileName  + ".log");
    if (!m_tunnelsFile.open(QIODevice::WriteOnly | QIODevice::Append)) {
        qCDebug(dcApplication()) << "Could not open logfile" << m_tunnelsFile.fileName();
    }
}

QString LogEngine::createTimestamp()
{
    return QString::number(QDateTime::currentDateTime().toTime_t());
}

void LogEngine::enable()
{
    qCDebug(dcApplication()) << "Enable log engine";
    m_enabled = true;

    m_statisticsFile.setFileName(m_statisticsFileName + ".log");
    if (!m_statisticsFile.open(QIODevice::WriteOnly | QIODevice::Append)) {
        qCDebug(dcApplication()) << "Could not open logfile" << m_statisticsFile.fileName();
    }

    m_tunnelsFile.setFileName(m_tunnelsFileName + ".log");
    if (!m_tunnelsFile.open(QIODevice::WriteOnly | QIODevice::Append)) {
        qCDebug(dcApplication()) << "Could not open logfile" << m_tunnelsFile.fileName();
    }
}

void LogEngine::disable()
{
    qCDebug(dcApplication()) << "Disable log engine";
    m_enabled = false;

    if (m_tunnelsFile.isOpen())
        m_tunnelsFile.close();

    if (m_statisticsFile.isOpen())
        m_statisticsFile.close();
}

}
