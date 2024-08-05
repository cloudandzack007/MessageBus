#ifndef MESSAGEBUS_H
#define MESSAGEBUS_H

#include <QObject>
#include <QMap>
#include <QSet>
#include <QVariant>
#include <QMetaEnum>
#include "MessageTypes.h"

#define MESSAGE_BUS_INSTANCE (MessageBus::instance())

class MessageBus : public QObject
{
    Q_OBJECT

public:
    static MessageBus* instance()
    {
        static MessageBus instance;
        return &instance;
    }

    template<typename Func>
    void subscribe(MessageType messageType, QObject* receiver, Func slot)
    {
        if (!receiver || !slot) {
            qWarning() << "Error: Invalid receiver or slot.";
            return;
        }

        // Check if the receiver is already subscribed to this message type
        if (connections[messageType].contains(receiver)) {
            qWarning() << "Warning: Receiver is already subscribed to message type" << QMetaEnum::fromType<MessageType>().valueToKeys(static_cast<int>(messageType));
            return;
        }

        auto connection = connect(this, &MessageBus::messageReceived, receiver, [=](MessageType type, const QVariant& data) {
            if (type == messageType) {
                QMetaObject::invokeMethod(receiver, slot, Qt::QueuedConnection, Q_ARG(QVariant, data));
            }
        });
        connections[messageType][receiver].insert(connection);
    }

    void unsubscribe(MessageType messageType, QObject* receiver)
    {
        if (!receiver) {
            qWarning() << "Error: Invalid receiver.";
            return;
        }

        if (connections.contains(messageType) && connections[messageType].contains(receiver)) {
            for (const auto& connection : connections[messageType][receiver]) {
                disconnect(connection);
            }
            connections[messageType].remove(receiver);
        } else {
            qWarning() << "Warning: Receiver is not subscribed to message type" << QMetaEnum::fromType<MessageType>().valueToKeys(static_cast<int>(messageType));
        }
    }

    template<typename Func>
    void subscribe(const QString& messageTypeStr, QObject* receiver, Func slot)
    {
        MessageType messageType = stringToEnum(messageTypeStr);
        if (messageType == MessageType()) {
            qWarning() << "Error: Invalid message type string" << messageTypeStr;
            return;
        }
        subscribe(messageType, receiver, slot);
    }

    void unsubscribe(const QString& messageTypeStr, QObject* receiver)
    {
        MessageType messageType = stringToEnum(messageTypeStr);
        if (messageType == MessageType()) {
            qWarning() << "Error: Invalid message type string" << messageTypeStr;
            return;
        }
        unsubscribe(messageType, receiver);
    }

    void publish(MessageType messageType, const QVariant& messageData)
    {
        emit messageReceived(messageType, messageData);
    }

signals:
    void messageReceived(MessageType messageType, const QVariant& messageData);

private:
    MessageBus(QObject* parent = nullptr) : QObject(parent) {}
    Q_DISABLE_COPY(MessageBus)

    MessageType stringToEnum(const QString& typeStr)
    {
        QMetaEnum metaEnum = QMetaEnum::fromType<MessageType>();
        return static_cast<MessageType>(metaEnum.keyToValue(typeStr.toUtf8().constData()));
    }

    QString enumToString(MessageType type)
    {
        QMetaEnum metaEnum = QMetaEnum::fromType<MessageType>();
        return QString(metaEnum.valueToKey(static_cast<int>(type)));
    }

private:
    QMap<MessageType, QMap<QObject*, QSet<QMetaObject::Connection>>> connections;
};

#endif // MESSAGEBUS_H
