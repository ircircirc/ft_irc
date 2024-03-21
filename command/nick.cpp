#include "../ConfigManager.hpp"

void ConfigManager::registerNick(std::vector<std::string> &commandAndParams, int clientFd)
{
    if (commandAndParams.size() < 2)
    {
        serverToClientMsg[clientFd] += ":irc.local 461 * NICK :Not enough parameters.\r\n";
        EV_SET(&tempEvent, clientFd, EVFILT_WRITE, EV_ADD, 0, 0, NULL);
        change_list.push_back(tempEvent);
        return;
    }

    // 등록된 member가 로그인을 시도했을때 에러처리
    if (fdNicknameMap.find(clientFd) != fdNicknameMap.end())
    {
        serverToClientMsg[clientFd] += std::string(":irc.local 462" + fdNicknameMap[clientFd] + ":You may not reregister\r\n");
        EV_SET(&tempEvent, clientFd, EVFILT_WRITE, EV_ADD, 0, 0, NULL);
        change_list.push_back(tempEvent);
        return;
    }

    // 잘못된 닉네임 형식이 있다. 하지만 이거를 구현해야할까? -> 일단 패스
    std::string nickname = commandAndParams[1];
    if (memberMap.find(nickname) != memberMap.end())
    {
        serverToClientMsg[clientFd] += ":irc.local 433 * root :Nickname is already in use\r\n";
        EV_SET(&tempEvent, clientFd, EVFILT_WRITE, EV_ADD, 0, 0, NULL);
        change_list.push_back(tempEvent);
        return;
    }
    unregisterMemberMap[clientFd].nickname = nickname;
    checkRegister(clientFd);
}
