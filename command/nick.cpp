#include "../ConfigManager.hpp"

void ConfigManager::registerNick(std::vector<std::string> &commandAndParams, int clientFd)
{
    if (commandAndParams.size() < 2)
    {
        serverToClientMsg[clientFd] += ":irc.local 461 * NICK :Not enough parameters.\r\n";
        setWriteEvent(clientFd);
        return;
    }

    if (fdNicknameMap.find(clientFd) != fdNicknameMap.end())
    {
        serverToClientMsg[clientFd] += std::string(":irc.local 462" + fdNicknameMap[clientFd] + ":You may not reregister\r\n");
        setWriteEvent(clientFd);
        return;
    }

    std::string nickname = commandAndParams[1];
    if (memberMap.find(nickname) != memberMap.end())
    {
        serverToClientMsg[clientFd] += ":irc.local 433 * " + nickname + " :Nickname is already in use\r\n";
        setWriteEvent(clientFd);
        return;
    }

    for (std::map<int, UnregisterMember>::iterator it = unregisterMemberMap.begin(); it != unregisterMemberMap.end(); ++it)
    {
        if (it->second.nickname.compare(nickname) == 0)
        {
            serverToClientMsg[it->first] += ":irc.local 433 " + nickname + " " + nickname + " :Nickname overruled.\r\n";
            setWriteEvent(it->first);
            it->second.nickname.clear();
        }
    }

    unregisterMemberMap[clientFd].nickname = nickname;
    checkRegister(clientFd);
}
