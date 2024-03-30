#include "../ConfigManager.hpp"

void ConfigManager::registerNick(std::vector<std::string> &commandAndParams, int clientFd)
{
    if (commandAndParams.size() < 2)
    {
        serverToClientMsg[clientFd] += ":irc.local 461 * NICK :Not enough parameters.\r\n";
        setWriteEvent(clientFd);
        return;
    }

    // 등록된 member가 로그인을 시도했을때 에러처리
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

    // 등록하지 않은 회원의 닉네임이 같을때 해당 회원의 닉네임을 초기화함
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
