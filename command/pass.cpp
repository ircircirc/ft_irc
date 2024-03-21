#include "../ConfigManager.hpp"

void ConfigManager::authenticateUser(std::vector<std::string> &commandAndParams, int clientFd)
{
    if (commandAndParams.size() < 2)
    {
        serverToClientMsg[clientFd] += ":irc.local 461 * PASS :Not enough parameters.\r\n";
        EV_SET(&tempEvent, clientFd, EVFILT_WRITE, EV_ADD, 0, 0, NULL);
        change_list.push_back(tempEvent);
    }

    // 등록된 member가 로그인을 시도했을떄 에러처리
    if (fdNicknameMap.find(clientFd) != fdNicknameMap.end())
    {
        serverToClientMsg[clientFd] += std::string(":irc.local 462" + fdNicknameMap[clientFd] + ":You may not reregister\r\n");
        EV_SET(&tempEvent, clientFd, EVFILT_WRITE, EV_ADD, 0, 0, NULL);
        change_list.push_back(tempEvent);
        return;
    }
    unregisterMemberMap[clientFd].password = commandAndParams[1];
}
