#include "message.h"
#include "character.h"
#include "widget.h"
#include "utils.h"
#include "serialize.h"

void sendMessage(Player* player,quint8 messageType, QByteArray data)
{
    QByteArray msg(3,0);
    // MessageType
    msg[0] = messageType;
    // Sequence
    msg[1] = 0;
    msg[2] = 0;
    if (messageType == MsgPing)
    {
        msg.resize(6);
        // Sequence
        msg[1] = 0;
        msg[2] = 0;
        // Payload size
        msg[3] = 8;
        msg[4] = 0;
        // Ping number
        player->lastPingNumber++;
        msg[5]=player->lastPingNumber;
    }
    else if (messageType == MsgPong)
    {
        msg.resize(6);
        // Payload size
        msg[3] = 8*5;
        msg[4] = 0;
        // Ping number
        msg[5]=player->lastPingNumber;
        // Timestamp
        msg += floatToData(timestampNow());
    }
    else if (messageType == MsgUserUnreliable)
    {
        msg.resize(5);
        // Sequence
        msg[1] = player->udpSequenceNumbers[32];
        msg[2] = player->udpSequenceNumbers[32]>>8;
        // Data size
        msg[3] = 8*(data.size());
        msg[4] = (8*(data.size())) >> 8;
        // Data
        msg += data;
        player->udpSequenceNumbers[32]++;

        //win.logMessage(QString("Sending sync data :")+msg.toHex());
    }
    else if (messageType >= MsgUserReliableOrdered1 && messageType <= MsgUserReliableOrdered32)
    {
        //win.logMessage("sendMessage locking");
        player->udpSendReliableMutex.lock();
        player->udpSendReliableGroupTimer->stop();
        msg.resize(5);
        // Sequence
        msg[1] = player->udpSequenceNumbers[messageType-MsgUserReliableOrdered1];
        msg[2] = player->udpSequenceNumbers[messageType-MsgUserReliableOrdered1] >> 8;
        // Payload size
        msg[3] = 8*(data.size());
        msg[4] = (8*(data.size())) >> 8;
        // strlen
        msg+=data;
        player->udpSequenceNumbers[messageType-MsgUserReliableOrdered1] += 2;

        if (player->udpSendReliableGroupBuffer.size() + msg.size() > 1024) // Flush the buffer before starting a new grouped msg
            player->udpDelayedSend();
        player->udpSendReliableGroupBuffer.append(msg);

        player->udpSendReliableGroupTimer->start(); // When this timeouts, the content of the buffer will be sent reliably

        //win.logMessage("sendMessage unlocking");
        player->udpSendReliableMutex.unlock();
        return; // This isn't a normal send, but a delayed one with the timer callbacks
    }
    else if (messageType == MsgAcknowledge)
    {
        msg.resize(5);
        // Payload size
        msg[3] = (quint8)(data.size()*8);
        msg[4] = (quint8)((data.size()*8)>>8);
        msg.append(data); // Format of packet data n*(Ack type, Ack seq, Ack seq)
    }
    else if (messageType == MsgConnectResponse)
    {
        msg.resize(5);
        // Payload size
        msg[3] = 0x88;
        msg[4] = 0x00;

        //win.logMessage("UDP: Header data : "+msg.toHex());

        // AppId + UniqueId
        msg += data;
        msg += floatToData(timestampNow());
    }
    else if (messageType == MsgDisconnect)
    {
        msg.resize(6);
        // Payload size
        msg[3] = ((data.size()+1)*8);
        msg[4] = ((data.size()+1)*8)>>8;
        // Message length
        msg[5] = data.size();
        // Disconnect message
        msg += data;
    }
    else
    {
        win.logStatusMessage("sendMessage : Unknown message type");
        return;
    }

    if (win.udpSocket->writeDatagram(msg,QHostAddress(player->IP),player->port) != msg.size())
    {
        win.logMessage("UDP: Error sending message to "+QString().setNum(player->pony.netviewId)
                       +" : "+win.udpSocket->errorString());
        win.logMessage("Restarting UDP server ...");
        win.udpSocket->close();
        if (!win.udpSocket->bind(win.gamePort, QUdpSocket::ReuseAddressHint|QUdpSocket::ShareAddress))
        {
            win.logStatusMessage("UDP: Unable to start server on port "+QString().setNum(win.gamePort));
            win.stopServer();
            return;
        }
    }
}
