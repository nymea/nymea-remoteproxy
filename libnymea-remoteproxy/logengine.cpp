// SPDX-License-Identifier: LGPL-3.0-or-later

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* nymea-remoteproxy
* Tunnel proxy server for the nymea remote access
*
* Copyright (C) 2013 - 2024, nymea GmbH
* Copyright (C) 2024 - 2025, chargebyte austria GmbH
*
* This file is part of nymea-remoteproxy.
*
* nymea-remoteproxy is free software: you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public License
* as published by the Free Software Foundation, either version 3
* of the License, or (at your option) any later version.
*
* nymea-remoteproxy is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with nymea-remoteproxy. If not, see <https://www.gnu.org/licenses/>.
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "logengine.h"
#include "loggingcategories.h"

#include <QDateTime>
#include <QtGlobal>

namespace remoteproxy {

LogEngine::LogEngine(QObject *parent) : QObject(parent)
{
    m_currentDay = QDateTime::currentDateTimeUtc().date().day();
    m_tunnelsFileName = "/var/log/nymea-remoteproxy-tunnels";
    m_statisticsFileName = "/var/log/nymea-remoteproxy-statistics";
}

LogEngine::~LogEngine()
{
    disable();
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

    QTextStream textStream(&m_statisticsFile);
    textStream << logString.join(" ") << "\n";

    // Check if we have to rotate the logfile
    if (m_currentDay != QDateTime::currentDateTimeUtc().date().day()) {
        // Day changed
        m_currentDay = QDateTime::currentDateTimeUtc().date().day();
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
    QString postfix =  "-" + QDateTime::currentDateTimeUtc().toString("yyyyMMddhhmmss") + ".log";

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
    return QString::number(QDateTime::currentDateTimeUtc().toSecsSinceEpoch());
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
