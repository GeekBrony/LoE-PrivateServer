#ifndef MESSAGE_H
#define MESSAGE_H

#include <QPair>
#include <QMutex>
#include <QByteArray>
#include <QStringList>
#include "dataType.h"

// Resend the udp message if we didn't get an ACK before this timeouts (3000 is reasonable)
#define UDP_RESEND_TIMEOUT 3000
// If we send multiple reliable messages before this timeouts, group them before sending. (100 is reasonable)
#define UDP_GROUPING_TIMEOUT 100

enum MessageTypes {
    MsgUnconnected = 0,
    MsgUserUnreliable = 1,
    MsgPing = 0x81,
    MsgPong = 0x82,
    MsgConnect = 0x83,
    MsgConnectResponse = 0x84,
    MsgConnectionEstablished = 0x85,
    MsgAcknowledge = 0x86,
    MsgDisconnect = 0x87,
    MsgUserReliableOrdered1 = 0x43,
    MsgUserReliableOrdered2 = 0x44,
    MsgUserReliableOrdered3 = 0x45,
    MsgUserReliableOrdered4 = 0x46,
    MsgUserReliableOrdered5 = 0x47,
    MsgUserReliableOrdered6 = 0x48,
    MsgUserReliableOrdered11 = 0x4d,
    MsgUserReliableOrdered12 = 0x4e,
    MsgUserReliableOrdered18 = 0x54,
    MsgUserReliableOrdered20 = 0x56,
    MsgUserReliableOrdered32 = 0x62
};

enum ChatType
{
    ChatNone = 0,
    ChatUntyped = 1,
    ChatSystem = 2,
    ChatGeneral = 4,
    ChatLocal = 8,
    ChatParty = 16,
    ChatGuild = 32,
    ChatWhisper = 64
};

// Public functions
class Player;
class Pony;
void receiveMessage(Player* player);
void sendMessage(Player* player, quint8 messageType, QByteArray data=QByteArray());
void sendEntitiesList(Player* player);
void sendPonySave(Player* player, QByteArray msg);
void sendPonies(Player* player);
void sendPonyData(Player* player);
void sendPonyData(Pony *src, Player* dst);
void sendNetviewInstantiate(Player* player, QString key, quint16 ViewId, quint16 OwnerId, UVector pos, UQuaternion rot);
void sendNetviewInstantiate(Player* player);
void sendNetviewInstantiate(Pony *src, Player* dst);
void sendNetviewRemove(Player* player, quint16 netviewId);
void sendSetStatRPC(Player* player, quint8 statId, float value);
void sendSetMaxStatRPC(Player* player, quint8 statId, float value);
void sendSetStatRPC(Player* affected, Player* dest, quint8 statId, float value); // Set stat of affected on client dest
void sendSetMaxStatRPC(Player* affected, Player* dest, quint8 statId, float value); // Set stat of affected on client dest
void sendWornRPC(Player* player, QList<WearableItem>& worn); // Send items worn by player to himself
void sendWornRPC(Pony *wearing, Player* dest, QList<WearableItem>& worn); // Send items worn by wearing to dest
void sendInventoryRPC(Player* player, QList<InventoryItem>& inv, QList<WearableItem>& worn, quint32 nBits);
void sendSkillsRPC(Player* player, QList<QPair<quint32, quint32> >& skills);
void sendLoadSceneRPC(Player* player, QString sceneName);
void sendLoadSceneRPC(Player* player, QString sceneName, UVector pos);
void sendChatMessage(Player* player, QString message, QString author, quint8 chatType);
void sendMove(Player* player, float x, float y, float z);
void sendBeginDialog(Player* player);
void sendDialogMessage(Player* player, QString& message, QString NPCName, quint16 iconId=0);
void sendDialogOptions(Player* player, QList<QString> &answers);
void sendEndDialog(Player* player);

#endif // MESSAGE_H
