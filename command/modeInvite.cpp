#include "../ConfigManager.hpp"

void ConfigManager::processModeInvite(int clientFd, bool sign, std::string &channelName)
{
    if (channelMap[channelName].inviteOnly == sign)
        return;
    channelMap[channelName].inviteOnly = sign;
    char signChar = '+';
    if (!sign)
        signChar = '-';
    std::string msg = ":" + fdNicknameMap[clientFd] + "!" + memberMap[fdNicknameMap[clientFd]].username + "@" + memberMap[fdNicknameMap[clientFd]].hostname + " MODE #" + channelName + " :" + signChar + "i\r\n";
    std::set<std::string>::iterator it = channelMap[channelName].memberNickSet.begin();
    for (; it != channelMap[channelName].memberNickSet.end(); ++it)
    {
        int channelMemberFd = memberMap[*it].fd;
        serverToClientMsg[channelMemberFd] += msg;
        setWriteEvent(channelMemberFd);
    }
}