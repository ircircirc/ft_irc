#include "../ConfigManager.hpp"

void ConfigManager::processModeLimit(int clientFd, bool sign, std::string &channelName, std::vector<std::string> &commandAndParams)
{
    char signChar = '+';
    if (!sign)
        signChar = '-';
    if (sign == true)
    {
        if (commandAndParams.size() < 4)
        {
            serverToClientMsg[clientFd] += ":irc.local 696 " + fdNicknameMap[clientFd] + " #" + channelName + " k * :You must specify a parameter for the limit mode. Syntax: <limit>\r\n";
            setWriteEvent(clientFd);
            return;
        }
        channelMap[channelName].isLimit = true;
        channelMap[channelName].limitCount = std::atoi(commandAndParams[3].c_str());

        std::ostringstream oss;
        oss << channelMap[channelName].limitCount;
        std::string limitStr = oss.str();

        std::string msg = ":" + fdNicknameMap[clientFd] + "!" + memberMap[fdNicknameMap[clientFd]].username + "@" + memberMap[fdNicknameMap[clientFd]].hostname + " MODE #" + channelName + " " + "+l :" + limitStr + "\r\n";
        std::set<std::string>::iterator it = channelMap[channelName].memberNickSet.begin();
        for (; it != channelMap[channelName].memberNickSet.end(); ++it)
        {
            int channelMemberFd = memberMap[*it].fd;
            serverToClientMsg[channelMemberFd] += msg;
            setWriteEvent(channelMemberFd);
        }
    }
    else if (sign == false)
    {
        if (channelMap[channelName].isLimit == false)
            return;
        channelMap[channelName].isLimit = false;
        std::string msg = ":" + fdNicknameMap[clientFd] + "!" + memberMap[fdNicknameMap[clientFd]].username + "@127.0.0.1 MODE #" + channelName + " " + " :-l\r\n";
        std::set<std::string>::iterator it = channelMap[channelName].memberNickSet.begin();
        for (; it != channelMap[channelName].memberNickSet.end(); ++it)
        {
            int channelMemberFd = memberMap[*it].fd;
            serverToClientMsg[channelMemberFd] += msg;
            setWriteEvent(channelMemberFd);
        }
    }
}