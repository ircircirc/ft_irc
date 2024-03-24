#include "../ConfigManager.hpp"

void ConfigManager::authenticateUser(std::vector<std::string> &commandAndParams, int clientFd)
{
    if (commandAndParams.size() < 2)
    {
        serverToClientMsg[clientFd] += ":irc.local 461 * PASS :Not enough parameters.\r\n";
        setWriteEvent(clientFd);
        return;
    }

    // 등록된 member가 로그인을 시도했을떄 에러처리
    if (fdNicknameMap.find(clientFd) != fdNicknameMap.end())
    {
        serverToClientMsg[clientFd] += std::string(":irc.local 462" + fdNicknameMap[clientFd] + ":You may not reregister\r\n");
        setWriteEvent(clientFd);
        return;
    }
    unregisterMemberMap[clientFd].password = commandAndParams[1];
}
