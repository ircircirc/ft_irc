#include "../ConfigManager.hpp"

void ConfigManager::sendPrivateMsg(std::vector<std::string> &commandAndParams, int clientFd)
{
    if (commandAndParams.size() < 3)
    {
        serverToClientMsg[clientFd] += ":irc.local 461 * PASS :Not enough parameters.\r\n";
        setWriteEvent(clientFd);
        return;
    }

    // 등록된 멤버가 아닐때
    if (fdNicknameMap.find(clientFd) == fdNicknameMap.end())
    {
        serverToClientMsg[clientFd] += ":irc.local 451 * PRIVMSG :You have not registered.\r\n";
        setWriteEvent(clientFd);
        return;
    }

    // 메시지를 받을 사람의 닉네임을 찾지 못했을때
    std::string &targetNick = commandAndParams[1];
    if (memberMap.find(targetNick) == memberMap.end())
    {
        serverToClientMsg[clientFd] += ":irc.local 401 * " + targetNick + "  :No such nick\r\n";
        setWriteEvent(clientFd);
        return;
    }

    IrcMember &sourceMember = memberMap[fdNicknameMap[clientFd]];
    IrcMember &targetMember = memberMap[targetNick];

    std::string msg;
    msg += std::string(":" + sourceMember.nickname + "!" + sourceMember.username + "@" + "127.0.0.1" + " ");
    msg += std::string("PRIVMSG " + targetMember.nickname + " ");
    for (int i = 2; i < commandAndParams.size(); i++)
    {
        msg += commandAndParams[i];
        if (i != commandAndParams.size() - 1)
            msg += " ";
    }
    msg += "\r\n";
    serverToClientMsg[targetMember.fd] += msg;
    setWriteEvent(targetMember.fd);
}