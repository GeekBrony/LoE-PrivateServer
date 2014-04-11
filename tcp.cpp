#include "widget.h"
#include "ui_widget.h"
#include "message.h"
#include "utils.h"

void Widget::tcpConnectClient()
{
    //logMessage("TCP: New client connected");

    QTcpSocket *newClient = tcpServer->nextPendingConnection();
    tcpClientsList << newClient;

    connect(newClient, SIGNAL(readyRead()), this, SLOT(tcpProcessPendingDatagrams()));
    connect(newClient, SIGNAL(disconnected()), this, SLOT(tcpDisconnectClient()));
}

void Widget::tcpDisconnectClient()
{
    // Find who's disconnecting, if we can't, just give up
    QTcpSocket *socket = qobject_cast<QTcpSocket *>(sender());
    if (socket == 0)
    return;

    //logMessage("TCP: Client disconnected");
    disconnect(socket);

    tcpClientsList.removeOne(socket);

    socket->deleteLater();
    socket=0;
}



void Widget::tcpProcessPendingDatagrams()
{
    // Find who's sending
    QTcpSocket *socket = qobject_cast<QTcpSocket *>(sender());
    if (socket == 0)
        return;

    unsigned nTries = 0;

    // Acquire data
    while(socket->state()==QAbstractSocket::ConnectedState && nTries<3) // Exit if disconnected, too much retries, malformed HTTP request, or after all requests are processed
    {
        tcpReceivedDatas->append(socket->readAll());
        nTries++;

        if (!tcpReceivedDatas->startsWith("POST") && !tcpReceivedDatas->startsWith("GET")) // Not HTTP, clear the buffer
        {
            //logMessage("TCP: Received non-HTTP request");
            tcpReceivedDatas->clear();
            socket->close();
            return;
        }
        else if (tcpReceivedDatas->contains("Content-Length:")) // POST or GET request, wait for Content-Length header
        {
            QByteArray contentLength = *tcpReceivedDatas;
            contentLength = contentLength.right(contentLength.size() - contentLength.indexOf("Content-Length:") - 15);
            QList<QByteArray> lengthList = contentLength.trimmed().split('\n');
            if (lengthList.size()>1) // We want a number on this line and a next line to be sure we've got the full number
            {
                bool isNumeric;
                int length = lengthList[0].trimmed().toInt(&isNumeric);
                if (!isNumeric) // We've got something but it's not a number
                {
                    logMessage("TCP: Error: Content-Length must be a (decimal) number !");
                    tcpReceivedDatas->clear();
                    socket->close();
                    return;
                }

                // Detect and send data files if we need to
                QByteArray data = *tcpReceivedDatas;
                //logMessage("DataReceived:"+data);

                // Get the payload only (remove headers)
                data = removeHTTPHeader(data, "POST ");
                data = removeHTTPHeader(data, "GET ");
                data = removeHTTPHeader(data, "User-Agent:");
                data = removeHTTPHeader(data, "Host:");
                data = removeHTTPHeader(data, "host:");
                data = removeHTTPHeader(data, "Accept:");
                data = removeHTTPHeader(data, "Content-Length:");
                data = removeHTTPHeader(data, "Content-Type:");
                data = removeHTTPHeader(data, "Server:");
                data = removeHTTPHeader(data, "Date:");
                data = removeHTTPHeader(data, "Transfert-Encoding:");
                data = removeHTTPHeader(data, "Connection:");
                data = removeHTTPHeader(data, "Vary:");
                data = removeHTTPHeader(data, "X-Powered-By:");
                data = removeHTTPHeader(data, "accept-encoding:");
                data = removeHTTPHeader(data, "if-modified-since:");

                if (data.size() >= length) // Wait until we have all the data
                {
                    data.truncate(length);

                    // Process data, if the buffer is not empty, keep reading
                    tcpProcessData(data, socket);
                    // Delete the processed message from the buffer
                    *tcpReceivedDatas = tcpReceivedDatas->right(tcpReceivedDatas->size() - tcpReceivedDatas->indexOf(data) - data.size());
                    if (tcpReceivedDatas->isEmpty())
                        return;
                    nTries=0;
                }
            }
        }
        else if (tcpReceivedDatas->contains("\r\n\r\n")) // POST or GET request, without a Content-Length header
        {
            QByteArray data = *tcpReceivedDatas;
            data = data.left(data.indexOf("\r\n\r\n")+4);

            int i1=0;
            do
            {
                i1 = data.indexOf("GET");
                if (i1 != -1)
                {
                    int i2 = data.indexOf("HTTP")-1;
                    QString path = data.mid(i1 + 4, i2-i1-4);
                    if (path == "/log") // GET /log
                    {
                        data = removeHTTPHeader(data, "POST ");
                        data = removeHTTPHeader(data, "GET ");
                        if (!enableGetlog)
                            continue;
                        QFile head(QString(NETDATAPATH)+"/dataTextHeader.bin");
                        head.open(QIODevice::ReadOnly);
                        if (!head.isOpen())
                        {
                            logMessage("Can't open header : "+head.errorString());
                            continue;
                        }
                        QByteArray logData = ui->log->toPlainText().toLatin1();
                        socket->write(head.readAll());
                        socket->write(QString("Content-Length: "+QString().setNum(logData.size())+"\r\n\r\n").toLocal8Bit());
                        socket->write(logData);
                        head.close();
                        logMessage("Sent log to "+socket->peerAddress().toString());
                        continue;
                    }
                    // Other GETs (not getlog)
                    data = removeHTTPHeader(data, "POST ");
                    data = removeHTTPHeader(data, "GET ");
                    logMessage("TCP: Received GET:"+path);
                    QFile head(QString(NETDATAPATH)+"/dataHeader.bin");
                    QFile res("data/"+path);
                    head.open(QIODevice::ReadOnly);
                    if (!head.isOpen())
                    {
                        logMessage("Can't open header : "+head.errorString());
                        continue;
                    }
                    res.open(QIODevice::ReadOnly);
                    if (!res.isOpen())
                    {
                        logMessage("File not found");
                        head.close();
                        continue;
                    }
                    socket->write(head.readAll());
                    socket->write(QString("Content-Length: "+QString().setNum(res.size())+"\r\n\r\n").toLocal8Bit());
                    socket->write(res.readAll());
                    head.close();
                    res.close();
                    logMessage("TCP: Sent "+QString().setNum(res.size()+head.size())+" bytes");
                }
            } while (i1 != -1);

            *tcpReceivedDatas = tcpReceivedDatas->mid(data.size());
        }
    }
}

void Widget::tcpProcessData(QByteArray data, QTcpSocket* socket)
{
    // Login request (forwarded)
    if (useRemoteLogin && tcpReceivedDatas->contains("commfunction=login&") && tcpReceivedDatas->contains("&version="))
    {
        logMessage("TCP: Remote login not implemented yet.");
        // We need to add the client with his IP/port/passhash to tcpPlayers if he isn't already there
        Player newPlayer;
        newPlayer.IP = socket->peerAddress().toIPv4Address();
        newPlayer.port = socket->peerPort();
        QString passhash = QString(*tcpReceivedDatas);
        passhash = passhash.mid(passhash.indexOf("passhash=")+9);
        passhash.truncate(passhash.indexOf('&'));
        newPlayer.passhash = passhash;
        logMessage("IP:"+newPlayer.IP+", passhash:"+newPlayer.passhash);

        // Then connect to the remote and forward the client's requests
        if (!remoteLoginSock.isOpen())
        {
            remoteLoginSock.connectToHost(remoteLoginIP, remoteLoginPort);
            remoteLoginSock.waitForConnected(remoteLoginTimeout);
            if (!remoteLoginSock.isOpen())
            {
                win.logMessage("TCP: Can't connect to remote login server : timed out.");
                return;
            }
        }
        // We just blindly send everything that we're going to remove from tcpReceivedDatas at the end of tcpProcessData
        QByteArray toSend = *tcpReceivedDatas;
        toSend.left(toSend.indexOf(data) + data.size());
        remoteLoginSock.write(toSend);
    }
    else if (useRemoteLogin && tcpReceivedDatas->contains("Server:")) // Login reply (forwarded)
    {
        logMessage("TCP: Remote login not implemented yet.");
        // First we need to find a player matching the received passhash in tcpPlayers
        // Use the player's IP/port to find a matching socket in tcpClientsList
        // The login headers are all the same, so we can just use loginHeader.bin and send back data
    }
    else if (tcpReceivedDatas->contains("commfunction=login&") && tcpReceivedDatas->contains("&version=")) // Login request
    {
        QString postData = QString(*tcpReceivedDatas);
        *tcpReceivedDatas = tcpReceivedDatas->right(postData.size()-postData.indexOf("version=")-8-4); // 4 : size of version number (ie:version=1344)
        logMessage("TCP: Login request received :");
        QFile file(QString(NETDATAPATH)+"/loginHeader.bin");
        QFile fileServersList(SERVERSLISTFILEPATH);
        QFile fileBadPassword(QString(NETDATAPATH)+"/loginWrongPassword.bin");
        QFile fileAlready(QString(NETDATAPATH)+"/loginAlreadyConnected.bin");
        QFile fileMaxRegistration(QString(NETDATAPATH)+"/loginMaxRegistered.bin");
        QFile fileMaxConnected(QString(NETDATAPATH)+"/loginMaxConnected.bin");
        if (!file.open(QIODevice::ReadOnly) || !fileBadPassword.open(QIODevice::ReadOnly)
        || !fileAlready.open(QIODevice::ReadOnly) || !fileMaxRegistration.open(QIODevice::ReadOnly)
        || !fileMaxConnected.open(QIODevice::ReadOnly) || !fileServersList.open(QIODevice::ReadOnly))
        {
            logStatusMessage("Error reading login data files");
            stopServer();
        }
        else
        {
            bool ok=true;
            postData = postData.right(postData.size()-postData.indexOf("username=")-9);
            QString username = postData;
            username.truncate(postData.indexOf('&'));
            postData = postData.right(postData.size()-postData.indexOf("passhash=")-9);
            QString passhash = postData;
            passhash.truncate(postData.indexOf('&'));
            logMessage(QString("IP : ")+socket->peerAddress().toString());
            logMessage(QString("Username : ")+username);
            logMessage(QString("Passhash : ")+passhash);

            // Add player to the players list
            Player* player = Player::findPlayer(tcpPlayers, username);
            if (player->name != username) // Not found, create a new player
            {
                // Check max registered number
                if (tcpPlayers.size() >= maxRegistered)
                {
                    logMessage("TCP: Registration failed, too many players registered");
                    socket->write(fileMaxRegistration.readAll());
                    ok = false;
                }
                else
                {
                    logMessage("TCP: Creating user "+username+" in database");
                    Player* newPlayer = new Player;
                    newPlayer->name = username;
                    newPlayer->passhash = passhash;
                    newPlayer->IP = socket->peerAddress().toString();
                    newPlayer->connected = false; // The connection checks are done by the game servers

                    tcpPlayers << newPlayer;
                    if (!Player::savePlayers(tcpPlayers))
                        ok = false;
                }
            }
            else // Found, compare passhashes, check if already connected
            {
                if (player->passhash != passhash) // Bad password
                {
                    logMessage("TCP: Login failed, wrong password");
                    socket->write(fileBadPassword.readAll());
                    ok=false;
                }
                /*
                else if (newPlayer.connected) // Already connected
                {
                    logMessage("TCP: Login failed, player already connected");
                    socket->write(fileAlready.readAll());
                    ok=false;
                }
                */
                else // Good password
                {
                    /*
                    // Check too many connected
                    int n=0;
                    for (int i=0;i<tcpPlayers.size();i++)
                        if (tcpPlayers[i].connected)
                            n++;
                    if (n>=maxConnected)
                    {
                        logMessage("TCP: Login failed, too much players connected");
                        socket->write(fileMaxConnected.readAll());
                        ok=false;
                    }
                    else
                    */
                    {
                        //player->reset();
                        player->IP = socket->peerAddress().toString();
                        player->lastPingTime = timestampNow();
                        player->connected = true;
                    }
                }
            }

            if (ok) // Send servers list
            {
                QByteArray customData = file.readAll();

                QByteArray data1 = QByteArray::fromHex("0D0A61757468726573706F6E73653A0A747275650A");
                QByteArray sesskey = QCryptographicHash::hash(QString(passhash + saltPassword).toLatin1(), QCryptographicHash::Md5).toHex();
                sesskey += passhash;
                QByteArray data2 = QByteArray::fromHex("0A310A");
                QByteArray serversList;
                QByteArray line;
                do {
                    line = fileServersList.readLine().trimmed();
                    serversList+=line;
                    serversList+=0x0A;
                } while (!line.isEmpty());
                serversList = serversList.trimmed();
                QByteArray data3 = QByteArray::fromHex("0D0A300D0A0D0A");
                int dataSize = data1.size() + sesskey.size() + data2.size() + serversList.size() - 2;
                QString dataSizeStr = QString().setNum(dataSize, 16);

                customData += dataSizeStr;
                customData += data1;
                customData += sesskey;
                customData += data2;
                customData += serversList;
                customData += data3;

                logMessage("TCP: Login successful, sending servers list");
                socket->write(customData);
                socket->close();
            }
        }
    }
    else if (data.contains("commfunction=removesession"))
    {
        //logMessage("TCP: Session closed by client");
    }
    else // Unknown request, erase tcp buffer
    {
        // Display data
        logMessage("TCP: Unknown request received : ");
        logMessage(QString(data.data()));
    }
}
