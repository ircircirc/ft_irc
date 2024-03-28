
// MODE #Spaceship -t
// :Alien!root@127.0.0.1 MODE #Spaceship :-t
// :Alien!root@127.0.0.1 MODE #Spaceship :-t
// TOPIC #Spaceship :love


// MODE #Spaceship +t
// :Alien!root@127.0.0.1 MODE #Spaceship :+t
// TOPIC #Spaceship :hppay
// :irc.local 482 Pony #Spaceship :You must be a channel op or higher to change the topic.

#include "../ConfigManager.hpp"

void ConfigManager::processModeTopic(int clientFd, bool sign, std::string &channelName)
{
    if (channelMap[channelName].topicOpOnly == sign)
        return;
    channelMap[channelName].topicOpOnly = sign;
    char signChar = '+';
    if (!sign)
        signChar = '-';
    std::string msg = ":" + fdNicknameMap[clientFd] + "!" + memberMap[fdNicknameMap[clientFd]].username + "@127.0.0.1 MODE #" + channelName + " :" + signChar + "t\r\n";
    std::set<std::string>::iterator it = channelMap[channelName].memberNickSet.begin();
    for (; it != channelMap[channelName].memberNickSet.end(); ++it)
    {
        int channelMemberFd = memberMap[*it].fd;
        serverToClientMsg[channelMemberFd] += msg;
        setWriteEvent(channelMemberFd);
    }
}