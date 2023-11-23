/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
*  Copyright 2013 - 2023, nymea GmbH
*  Contact: contact@nymea.io
*
*  This file is part of nymea.
*  This project including source code and documentation is protected by copyright law, and
*  remains the property of nymea GmbH. All rights, including reproduction, publication,
*  editing and translation, are reserved. The use of this project is subject to the terms of a
*  license agreement to be concluded with nymea GmbH in accordance with the terms
*  of use of nymea GmbH, available under https://nymea.io/license
*
*  GNU General Public License Usage
*  Alternatively, this project may be redistributed and/or modified under
*  the terms of the GNU General Public License as published by the Free Software Foundation,
*  GNU version 3. this project is distributed in the hope that it will be useful, but WITHOUT ANY
*  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
*  PURPOSE. See the GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License along with this project.
*  If not, see <https://www.gnu.org/licenses/>.
*
*  For any further details and any questions please contact us under contact@nymea.io
*  or see our FAQ/Licensing Information on https://nymea.io/license/faq
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "logengine.h"
#include "loggingcategories.h"

#include <QDateTime>

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
    return QString::number(QDateTime::currentDateTimeUtc().toTime_t());
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
