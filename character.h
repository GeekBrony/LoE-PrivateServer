#ifndef CHARACTER_H
#define CHARACTER_H

#include <QObject>
#include <QString>
#include <QVector>
#include <QTimer>
#include <QMutex>
#include <QByteArray>
#include "dataType.h"
#include "quest.h"

#define MAX_INVENTORY_SIZE 12
#define MAX_WORN_ITEMS 32

struct SceneEntity
{
public:
    SceneEntity();

public:
    // Infos
    QString modelName;
    quint16 id;
    quint16 netviewId;

    // Pos
    QString sceneName;
    UVector pos;
    UQuaternion rot;
};

struct Pony : public SceneEntity
{
public:
    enum type
    {
        None,
        EarthPony,
        Unicorn,
        Pegasus,
        Moose
    };

public:
    Pony();
    type getType();
    void saveQuests(Player* owner); ///< Saves the state of all the quests (NOT thread safe)
    void loadQuests(Player* owner); ///< Loads the state of all the quests
    void saveInventory(Player *owner); ///< Saves the worn/possesed items and the money (NOT thread safe)
    bool loadInventory(Player *owner); ///< Loads the worn/possesed items and the money. False on error.
    void addInventoryItem(quint8 id, quint32 qty); ///< Adds qty items with the given id to the inventory
    void removeInventoryItem(quint8 id, quint32 qty); ///< Removes qty of the item from the inventory
    bool hasInventoryItem(quint8 id, quint32 qty=1); ///< Whether of not there are qty of this item in inventory

public:
    QByteArray ponyData;
    QString name;
    QList<InventoryItem> inv; // Inventory
    QList<WearableItem> worn; // Worn items
    quint32 nBits; // Number of bits (money)
    QList<Quest> quests; // State of the player's quests
    quint32 lastQuest; // Last quest script the player ran
};

class Player : QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(Player)

public:
    Player();
    ~Player();
    static void savePonies(Player* player,QList<Pony> ponies);
    static QList<Pony> loadPonies(Player *player);
    static bool savePlayers(QList<Player*>& playersData);
    static QList<Player*> loadPlayers();
    static Player* findPlayer(QList<Player*>& players, QString uname);
    static Player* findPlayer(QList<Player*>& players, QString uIP, quint16 uport);
    static Player* findPlayer(QList<Player*>& players, quint16 netviewId);
    static void removePlayer(QList<Player*>& players, QString uIP, quint16 uport);
    static void disconnectPlayerCleanup(Player* player);

public slots:
    void udpResendLast(); // If a reliable msg wasn't ACK'd yet, resend it now.
    void udpDelayedSend(); // Enqueue and send the content of the player's grouped message buffer

public:
    void reset(); // Reconstructs an empty Player
    void resetNetwork(); // Resets all the network-related members

public:
    QString IP;
    quint16 port;
    QString name;
    QString passhash;
    float lastPingTime;
    int lastPingNumber;
    bool connected;
    quint16 udpSequenceNumbers[33]; // Next seq number to use when sending a message
    quint16 udpRecvSequenceNumbers[33]; // Last seq number received
    QByteArray *receivedDatas;
    QVector<MessageHead> udpRecvMissing; // When a message is skipped, mark it as missing and wait for a retransmission
    QVector<QByteArray> udpSendReliableQueue; // Messages that we're sending and that aren't ACKd yet.
    QByteArray udpSendReliableGroupBuffer; // Groups the udp message in this buffer before sending them
    QTimer* udpSendReliableGroupTimer; // Delays the sending until we finished grouping the messages
    QTimer* udpSendReliableTimer; // If we didn't get an ACK before this timeouts, resend the last message
    QMutex udpSendReliableMutex; // Protects the buffer/queue/timers from concurrency hell
    Pony pony;
    QByteArray lastValidReceivedAnimation;
    quint8 inGame; // 0:Not in game, 1:Loading, 2:Instantiated & waiting savegame, 3:In game and loaded
    quint16 nReceivedDups; // Number of duplicate packets that we didn't miss and had to discard.
};

#endif // CHARACTER_H
