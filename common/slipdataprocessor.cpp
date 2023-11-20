/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright 2013 - 2022, nymea GmbH
* Contact: contact@nymea.io
*
* This file is part of nymea.
* This project including source code and documentation is protected by copyright law, and
* remains the property of nymea GmbH. All rights, including reproduction, publication,
* editing and translation, are reserved. The use of this project is subject to the terms of a
* license agreement to be concluded with nymea GmbH in accordance with the terms
* of use of nymea GmbH, available under https://nymea.io/license
*
* GNU Lesser General Public License Usage
* Alternatively, this project may be redistributed and/or modified under the terms of the GNU
* Lesser General Public License as published by the Free Software Foundation; version 3.
* this project is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
* without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
* See the GNU Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License along with this project.
* If not, see <https://www.gnu.org/licenses/>.
*
* For any further details and any questions please contact us under contact@nymea.io
* or see our FAQ/Licensing Information on https://nymea.io/license/faq
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "slipdataprocessor.h"

#include <QDebug>

QByteArray SlipDataProcessor::deserializeData(const QByteArray &data)
{
    QByteArray deserializedData;
    // Parse serial data
    bool escaped = false;
    for (int i = 0; i < data.length(); i++) {
        quint8 byte = static_cast<quint8>(data.at(i));

        if (escaped) {
            if (byte == ProtocolByteTransposedEnd) {
                deserializedData.append(static_cast<char>(ProtocolByteEnd));
            } else if (byte == ProtocolByteTransposedEsc) {
                deserializedData.append(static_cast<char>(ProtocolByteEsc));
            } else {
                qWarning() << "Error while deserializing data. Escape character received but the escaped character was not recognized.";
                return QByteArray();
            }

            escaped = false;
            continue;
        }

        // If escape byte, the next byte has to be a modified byte
        if (byte == ProtocolByteEsc) {
            escaped = true;
        } else if (byte == ProtocolByteEnd) {
            // We are done...lets skip the rest of the data since we got the end byte
            break;
        } else {
            deserializedData.append(static_cast<char>(byte));
        }
    }

    return deserializedData;
}

QByteArray SlipDataProcessor::serializeData(const QByteArray &data)
{
    QByteArray serializedData;
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QDataStream stream(&serializedData, QDataStream::WriteOnly);
#else
    QDataStream stream(&serializedData, QIODevice::WriteOnly);
#endif
//    stream << static_cast<quint8>(ProtocolByteEnd);

    for (int i = 0; i < data.length(); i++) {
        quint8 byte = static_cast<quint8>(data.at(i));
        switch (byte) {
        case ProtocolByteEnd:
            stream << static_cast<quint8>(ProtocolByteEsc);
            stream << static_cast<quint8>(ProtocolByteTransposedEnd);
            break;
        case ProtocolByteEsc:
            stream << static_cast<quint8>(ProtocolByteEsc);
            stream << static_cast<quint8>(ProtocolByteTransposedEsc);
            break;
        default:
            stream << byte;
            break;
        }
    }

    // Add the protocol end byte
    stream << static_cast<quint8>(ProtocolByteEnd);

    return serializedData;
}

SlipDataProcessor::Frame SlipDataProcessor::parseFrame(const QByteArray &data)
{
    Frame frame;
    QDataStream stream(data);
    stream >> frame.socketAddress;
    while (!stream.atEnd()) {
        quint8 dataByte;
        stream >> dataByte;
        frame.data.append(dataByte);
    }
    return frame;
}

QByteArray SlipDataProcessor::buildFrame(const Frame &frame)
{
    QByteArray data;
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QDataStream stream(&data, QDataStream::WriteOnly);
#else
    QDataStream stream(&data, QIODevice::WriteOnly);
#endif
    stream << frame.socketAddress;
    for (int i = 0; i < frame.data.size(); i++) {
        stream << static_cast<quint8>(frame.data.at(i));
    }
    return data;
}
