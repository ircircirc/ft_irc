#include "../ConfigManager.hpp"

void ConfigManager::registerUser(std::vector<std::string> &commandAndParams, int clientFd)
{
    if (commandAndParams.size() < 4)
    {
        serverToClientMsg[clientFd] += ":irc.local 461 * USER :Not enough parameters.\r\n";
        EV_SET(&tempEvent, clientFd, EVFILT_WRITE, EV_ADD, 0, 0, NULL);
        change_list.push_back(tempEvent);
        return;
    }
    if (fdNicknameMap.find(clientFd) != fdNicknameMap.end())
    {
        serverToClientMsg[clientFd] += std::string(":irc.local 462" + fdNicknameMap[clientFd] + ":You may not reregister\r\n");
        EV_SET(&tempEvent, clientFd, EVFILT_WRITE, EV_ADD, 0, 0, NULL);
        change_list.push_back(tempEvent);
        return;
    }
    UnregisterMember &unregisterMember = unregisterMemberMap[clientFd];
    unregisterMember.username = commandAndParams[1];
    unregisterMember.hostname = commandAndParams[2];
    unregisterMember.servername = commandAndParams[3];
    unregisterMember.realname = commandAndParams[4];
    checkRegister(clientFd);
}