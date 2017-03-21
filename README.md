LoE Private Server
===================================

**This project isn't maintained by the original author, but it was picked up by GeekBrony to see if he can help**.

**Please note that this will NOT work on new LoE versions unless we figure out how the new versions work**.

This is an open source Private Server, to play even when the official servers are closed.
The official release is for Windows (x86 and x64). The server should work on Linux and Mac too, but you'll need to compile it yourself.<br/>
<h5><b><a href="https://github.com/GeekBrony/LoE-PrivateServer/releases">Downloads</a></b></h5>

<h3>How to compile</h3>
- Download `Qt 5.2.0` or later
- Download the <a href="https://github.com/GeekBrony/LoE-PrivateServer/archive/master.zip">latest Private Server source snapshot</a>
- Extract zip archive with source snapshot into any desired directory
- Open `src\LoE_PrivateServer.pro` in Qt
- Press `Ctrl+B (or Build button)` to compile the project

<h4>How to compile a console version</h4>
- Open src\LoE_PrivateServer.pro in Qt
- Open <b>Projects</b> on the left sidebar
- Under <i>Edit Build Configuration</i> click <b>Add->Clone Selected</b>
- Name it something like <i>Console Release</i>
- Under <i>Build Steps</i>, expand <b>qmake</b>
- In <i>Additional Arguments</i>, paste the following: `CONFIG+=console_only`
- Press Ctrl+B (or Build button) to compile the project

<h3>How to use</h3>
Extract in the LoE game folder, start PrivateServer.exe and the game.
In the game pick a name/password (no need to register first)
Then use either :
- the Local server, for singleplayer or a multiplayer LAN in your house
- one of the Online servers, for multiplayer
- You can also add your own servers in data/serversList.cfg

<h3>Still in Beta</h3>
The private server is still in beta, expect bugs. Patches and pull requests are welcome.<br/>
Some important features are still lacking at the moment :
- Almost no monsters, and none of them can fight.
- Not as many quests as the official servers.
- No 'natural' items to collect (flowers, gems, ...)

<h3>Known issues:</h3>
- Positions of players will not sync properly as of the August 2014 release. actions/emotes work though... <a href="https://github.com/GeekBrony/LoE-PrivateServer/blob/master/src/sync.cpp">Help is appreciated</a>.
- No compatibility with the latest version of LoE - only supported up to August 2014 release.

<h3>Online private servers</h3>
The servers are run by independent volunteers. <br/>Anyone can start a private server, and they are not all running the same version or following the same rules. <br/>By default there is no chat filtering or moderation, the cutie marks are the ones of the official servers, and the quests/npcs are the ones on this repository.<br/>But server owners are free to change this as they wish, and their servers might not always be 'safe for work'.
You can find more informations on how to create/edit quests in the file QuestScript.txt.

<h3>Chat commands</h3>
- /stuck : Reloads and sends you to the spawn. Use it if you can't move.
- unstuck me : Same than above.
- :commands : Gives a list of chat commands
- :roll : Roll a dice, result between 0 and 99
- :msg player msg : Sends a private message to a player
- :names : Lists of the players on the server and where they are
- :me action : States your current action
- :tp location : Teleports your pony to this location (scene)

<h3>Server commands</h3>
You don't need any of those commands to play, but they might be useful.
setPeer is used to select a client. Most commands will only act on the selected client.
For example if you're stuck, do setPeer with your IP and port, then do for example "load Cottage".
- start/stop login : Starts and stops the login server
- start/stop game : Starts and stops the game server
- status : Prints status of the login and game servers
- exit : Stops the server and exit.
- clear : Clears the server's log.
- listPeers : Give the list of all the clients (= other players) connected
- setPeer : If there's only 1 client, select him for the other commands. If there's more than 1, use "setPeer IP port".
- setPeer \<IP\> \<port\> : Select the client at this IP and port. The other commands will act on the selected client.
- disconnect : Kick the player with the message "Connection closed by the server admin". Does not ban.
- load \<scene name\> : Loads a scene (PonyVille, Cloudsdale, ...) and teleport to its spawn. See the list of scenes below.
- getPos : Get the position (x, y, z) of the player. Often used with "move x y z".
- getRot : Get the rotation (x, y, z, w) of the player.
- move \<x\> \<y\> \<z\> : Instantly teleport the player to the given position. Not a spell. You just move.
- setStat \<statId\> \<value\> : Set the given stat (health, mana, ..) to the given value.
- setMaxStat \<statId\> \<value\> : Set the max of the given stat (health, mana, ..) to the given value.
- error \<message\> : Sends a message-box-scroll-thingy to the player with the title "Error" and the given message. Doesn't disconnect.

<br/>The following commands are for debugging only. You really don't need them, and most of the time you don't want to use them.
- sync : Syncs the position of all the clients now. Doesn't need setPeer to work.
- sendPonies : For debugging only.
- sendPonyData : For debugging only.
- sendUtils3 : For debugging only.
- setPlayerId \<id\> : For debugging only. Change the player's netview id.
- remove \<id\> : For debugging only. Remove the entity with this netview id from the selected player's point of view.
- instantiate : For debugging only. Spawns the player's body. Will clone the body if used repeatedly. May lag.
- instantiate \<key\> \<netview id\> \<view id\> : Will spawn key (PlayerBase for a pony). If the Ids are already taken, bad things will happen.
- instantiate \<key\> \<netview id\> \<view id\> \<x\> \<y\> \<z\> : Same than above but spawn at the given position.
- instantiate \<key\> \<netview id\> \<view id\> \<x\> \<y\> \<z\> \<rx\> \<ry\> \<rz\>: Same than above but spawn at the given position and rotation.
- beginDialog : For debugging only. Used when talking to NPCs.
- endDialog : For debugging only. Used when talking to NPCs.
- setDialogMsg \<message\> : For debugging only. Used when talking to NPCs.
- dbgStressLoad : For debugging only. Load GemMine on all clients now.
- listQuests : Lists the state of the player's quests
- listInventory : Lists the items in the player's inventory
- listWorn : Lists the items worn by the player's pony
