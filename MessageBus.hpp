#pragma once
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

    void subscribe(QObject* obj, const char* slot)
    {
        if (!connections.contains(obj))
        {
            auto connection = connect(this, SIGNAL(messageSent(MessageType, const QVariant&)), obj, slot);
            connections[obj] = connection;
        }
    }

    void unsubscribe(QObject* receiver)
    {
        if (connections.contains(receiver))
        {
            disconnect(connections[receiver]);
            connections.remove(receiver);
        }
    }

    void publish(MessageType type, const QVariant& message)
    {
        emit messageSent(type, message);
    }

signals:
    void messageSent(MessageType, const QVariant&);

private:
    MessageBus(QObject* parent = nullptr) : QObject(parent) {}
    Q_DISABLE_COPY(MessageBus)
    QMap<QObject*, QMetaObject::Connection> connections;
};

