#ifndef MESSAGEBUS_H
#define MESSAGEBUS_H

#include <QObject>
#include <QMap>
#include <QSet>
#include <QVariant>
#include <QMetaObject>

#define MESSAGE_TYPE_EXAMPLE 1
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

    // Template method for connecting signals and slots
    template<typename Func>
    void subscribe(int messageType, QObject* receiver, Func slot)
    {
        if (!receiver || !slot) {
            qWarning() << "Error: Invalid receiver or slot.";
            return;
        }

        // Check if the receiver is already subscribed to this message type
        if (connections[messageType].contains(receiver)) {
            qWarning() << "Warning: Receiver is already subscribed to message type" << messageType;
            return;
        }

        auto connection = connect(this, &MessageBus::messageReceived, receiver, [=](int type, const QVariant& data) {
            if (type == messageType) {
                (receiver->*slot)(data);
            }
        });
        // Store the connection
        connections[messageType][receiver].insert(connection);
    }

    void unsubscribe(int messageType, QObject* receiver)
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
            qWarning() << "Warning: Receiver is not subscribed to message type" << messageType;
        }
    }

    void publish(int messageType, const QVariant& messageData)
    {
        emit messageReceived(messageType, messageData);
    }

signals:
    void messageReceived(int messageType, const QVariant& messageData);

private:
    MessageBus(QObject* parent = nullptr) : QObject(parent) {}
    Q_DISABLE_COPY(MessageBus)

    // Map message type to a map of receivers and their connections
    QMap<int, QMap<QObject*, QSet<QMetaObject::Connection>>> connections;
};

#endif // MESSAGEBUS_H
