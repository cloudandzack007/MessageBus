#ifndef MESSAGETYPES_H
#define MESSAGETYPES_H

#include <QObject>
#include <QString>

// Define an enum for message types and register it with Q_ENUMS
enum class MessageType {
    MessageTypeExample = 0,
    // Add other message types here
};

Q_ENUMS(MessageType)

#endif // MESSAGETYPES_H
