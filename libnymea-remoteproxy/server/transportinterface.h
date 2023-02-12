/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
*  Copyright 2013 - 2020, nymea GmbH
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

#ifndef TRANSPORTINTERFACE_H
#define TRANSPORTINTERFACE_H

#include <QUrl>
#include <QObject>
#include <QHostAddress>

namespace remoteproxy {

class TransportInterface : public QObject
{
    Q_OBJECT
public:
    explicit TransportInterface(QObject *parent = nullptr);
    virtual ~TransportInterface() = 0;

    QString serverName() const;

    virtual void sendData(const QUuid &clientId, const QByteArray &data) = 0;
    virtual void killClientConnection(const QUuid &clientId, const QString &killReason) = 0;

    virtual QString name() const = 0;
    virtual uint connectionsCount() const = 0;

    QUrl serverUrl() const;
    void setServerUrl(const QUrl &serverUrl);

    virtual bool running() const = 0;

signals:
    void clientConnected(const QUuid &clientId, const QHostAddress &address);
    void clientDisconnected(const QUuid &clientId);
    void dataAvailable(const QUuid &clientId, const QByteArray &data);

protected:
    QUrl m_serverUrl;
    QString m_serverName;

public slots:
    virtual bool startServer() = 0;
    virtual bool stopServer() = 0;

};

}

#endif // TRANSPORTINTERFACE_H
