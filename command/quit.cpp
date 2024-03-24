#include "../ConfigManager.hpp"

void ConfigManager::quitMember(int clientFd)
{
    serverToClientMsg[clientFd] += "ERROR :Closing link: [Client exited]\r\n";
    setWriteEvent(clientFd);
    if (unregisterMemberMap.find(clientFd) != unregisterMemberMap.end())
        unregisterMemberMap[clientFd].pendingCloseSocket = true;
    else
        memberMap[fdNicknameMap[clientFd]].pendingCloseSocket = true;
}