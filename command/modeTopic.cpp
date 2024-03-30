#include "../ConfigManager.hpp"

void ConfigManager::processModeTopic(int clientFd, bool sign, std::string &channelName)
{
    if (channelMap[channelName].topicOpOnly == sign)
        return;
    channelMap[channelName].topicOpOnly = sign;
    char signChar = '+';
    if (!sign)
        signChar = '-';
    std::string msg = ":" + fdNicknameMap[clientFd] + "!" + memberMap[fdNicknameMap[clientFd]].username + "@" + memberMap[fdNicknameMap[clientFd]].hostname + " MODE #" + channelName + " :" + signChar + "t\r\n";
    std::set<std::string>::iterator it = channelMap[channelName].memberNickSet.begin();
    for (; it != channelMap[channelName].memberNickSet.end(); ++it)
    {
        int channelMemberFd = memberMap[*it].fd;
        serverToClientMsg[channelMemberFd] += msg;
        setWriteEvent(channelMemberFd);
    }
}