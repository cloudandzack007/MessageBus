#pragma once
#include <multidocument/DocumentContext.h>
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

    template <typename Receiver>
    void subscribe(Receiver* receiver, void (Receiver::*slot)(MessageType, const QVariant&, DocumentContext*))
    {
        static_assert(std::is_base_of<QObject, Receiver>::value, "Receiver must be a QObject or its derivative.");

        if (!connections.contains(receiver))
        {
            auto connection = connect(this, &MessageBus::messageSent, receiver, slot);
            connections[receiver] = connection;
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

    void publish(MessageType type, const QVariant& message , DocumentContext* context = nullptr)
    {
        emit messageSent(type, message , context);
    }

signals:
    void messageSent(MessageType, const QVariant& , DocumentContext* context /*= nullptr*/);

private:
    MessageBus(QObject* parent = nullptr) : QObject(parent) {}
    Q_DISABLE_COPY(MessageBus)
    QMap<QObject*, QMetaObject::Connection> connections;
};
