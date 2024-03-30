#include "../ConfigManager.hpp"


void ConfigManager::processModeKey(int clientFd, bool sign, std::string &channelName, std::vector<std::string> &commandAndParams)
{
    if (commandAndParams.size() < 4)
    {
        serverToClientMsg[clientFd] += ":irc.local 696 " + fdNicknameMap[clientFd] + " #" + channelName + " k * :You must specify a parameter for the key mode. Syntax: <key>\r\n";
        setWriteEvent(clientFd);
        return;
    }
    if (channelMap[channelName].useKeyOnly == sign)
        return;
    char signChar = '+';
    if (!sign)
        signChar = '-';
    std::string key = commandAndParams[3];
    if (sign == false && channelMap[channelName].key != key)
    {
        serverToClientMsg[clientFd] += ":irc.local 467 " + fdNicknameMap[clientFd] + " #" + channelName + " :Channel key already set\r\n";
        setWriteEvent(clientFd);
        return;
    }
    else if (sign == true)
        channelMap[channelName].key = key;
    channelMap[channelName].useKeyOnly = sign;

    std::string msg = ":" + fdNicknameMap[clientFd] + "!" + memberMap[fdNicknameMap[clientFd]].username + "@" + memberMap[fdNicknameMap[clientFd]].hostname + " MODE #" + channelName + " :" + signChar + "k :" + key + "\r\n";
    std::set<std::string>::iterator it = channelMap[channelName].memberNickSet.begin();
    for (; it != channelMap[channelName].memberNickSet.end(); ++it)
    {
        int channelMemberFd = memberMap[*it].fd;
        serverToClientMsg[channelMemberFd] += msg;
        setWriteEvent(channelMemberFd);
    }
}