#pragma once
#include <multidocument/DocumentContext.h>
#include "MessageTypes.h"
#include <QThread>
#include <QPointer>
#include <QCoreApplication>
#include <QMutex>
#include <QMutexLocker>
#define MESSAGE_BUS_INSTANCE (MessageBus::instance())

class MessageBus : public QObject
{
    Q_OBJECT

public:
    static MessageBus* instance()
    {
static QPointer<MessageBus> s_instance;

        if (s_instance.isNull())
        {
            if (QThread::currentThread() != QCoreApplication::instance()->thread())
            {
                QMetaObject::invokeMethod(
                    QCoreApplication::instance(),
                    [&]()
                    {
                        if (s_instance.isNull())
                        {
                            s_instance = new MessageBus();
                        }
                    },
                    Qt::BlockingQueuedConnection);
            }
            else
            {
                s_instance = new MessageBus();
            }
        }
        return s_instance;
    }

    template <typename Receiver>
    void subscribe(Receiver* receiver, void (Receiver::*slot)(MessageType, const QVariant&, DocumentContext*))
    {
        static_assert(std::is_base_of<QObject, Receiver>::value, "Receiver must be a QObject or its derivative.");
        QMutexLocker locker(&m_mutex);
        if (!connections.contains(receiver))
        {
            auto connection = connect(this, &MessageBus::messageSent, receiver, slot);
            connections[receiver] = connection;
        }
    }

    void unsubscribe(QObject* receiver)
    {
        QMutexLocker locker(&m_mutex);
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
    QMutex m_mutex;
};
