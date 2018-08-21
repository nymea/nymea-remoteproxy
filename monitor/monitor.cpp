#include "monitor.h"

Monitor::Monitor(const QString &serverName, QObject *parent) : QObject(parent)
{
    m_terminal = new TerminalWindow(this);
    m_monitorClient = new MonitorClient(serverName, this);

    connect(m_monitorClient, &MonitorClient::dataReady, m_terminal, &TerminalWindow::refreshWindow);

    m_monitorClient->connectMonitor();
}
