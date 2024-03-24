#include "../ConfigManager.hpp"

void ConfigManager::checkRegister(int clientFd)
{
    UnregisterMember &unregisterMember = unregisterMemberMap[clientFd];
    if (unregisterMember.nickname.size() != 0 && unregisterMember.username.size() != 0)
    {
        if (password.compare(unregisterMember.password) != 0)
        {
            serverToClientMsg[clientFd] += std::string("ERROR :Closing link: [Access denied by configuration]\r\n");
            unregisterMemberMap[clientFd].pendingCloseSocket = true;
            setWriteEvent(clientFd);
            return;
        }
        welcomeMember(clientFd);
    }
}

void ConfigManager::welcomeMember(int clientFd)
{
    UnregisterMember &unregisterMember = unregisterMemberMap[clientFd];
    memberMap[unregisterMember.nickname] = IrcMember(unregisterMember, clientFd);
    fdNicknameMap[clientFd] = unregisterMember.nickname;
    IrcMember &member = memberMap[unregisterMember.nickname];
    unregisterMemberMap.erase(clientFd);
    // hostname을 서버가 클라이언트에게 받았으나, 실제로 사용할때는 클라이언트의 IP주소를 조회해 사용한다고함.
    // 추후 수정 고려
    std::string ret1 = ":irc.local 001 " + member.nickname + " :Welcome to the Internet Relay Network" + member.nickname + "!" + member.username + "@" + member.hostname + "\r\n";
    // std::string ret2 = ":irc.local 002 root :Your host is ft_irc, running version 0.0.1\r\n";
    // std::string ret3 = ":irc.local 003 root :This server was created a long ago\r\n";
    // std::string ret4 = ":irc.local 004 root ft_irc\r\n";

    serverToClientMsg[clientFd] += ret1;
    setWriteEvent(clientFd);
}
