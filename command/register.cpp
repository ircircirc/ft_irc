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
    std::string ret1 = ":irc.local 001 " + member.nickname + " :Welcome to the Internet Relay Network " + member.nickname + "!" + member.username + "@" + memberMap[fdNicknameMap[clientFd]].hostname + "\r\n";

    serverToClientMsg[clientFd] += ret1;
    setWriteEvent(clientFd);
}
