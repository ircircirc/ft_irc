#include "../ConfigManager.hpp"

static std::string makeMsg(std::string &nickname, std::string &username, std::string &hostname, std::string targetName, std::vector<std::string> &commandAndParams)
{
    std::string msg;
    msg += std::string(":" + nickname + "!" + username + "@" + hostname + " ");
    msg += std::string("PRIVMSG " + targetName + " ");
    for (int i = 2; i < (int)commandAndParams.size(); i++)
    {
        msg += commandAndParams[i];
        if (i != (int)commandAndParams.size() - 1)
            msg += " ";
    }
    msg += "\r\n";
    return msg;
}

void ConfigManager::sendChannel(std::vector<std::string> &commandAndParams, int clientFd)
{
    std::string channelName = commandAndParams[1].substr(1);
    std::string &nickname = fdNicknameMap[clientFd];
    if (channelMap.find(channelName) == channelMap.end())
    {
        serverToClientMsg[clientFd] += ":irc.local 403 " + nickname + " #" + channelName + "  :No such channel\r\n";
        setWriteEvent(clientFd);
        return;
    }
    if (channelMap[channelName].memberNickSet.find(nickname) == channelMap[channelName].memberNickSet.end())
    {
        serverToClientMsg[clientFd] += ":irc.local 403 " + nickname + " #" + channelName + "  :Cannot send to channel\r\n";
        setWriteEvent(clientFd);
        return;
    }
    std::set<std::string>::iterator it = channelMap[channelName].memberNickSet.begin();
    std::string msg = makeMsg(nickname, memberMap[nickname].username, memberMap[nickname].hostname, ("#" + channelName), commandAndParams);
    for (; it != channelMap[channelName].memberNickSet.end(); ++it)
    {
        int channelMemberFd = memberMap[*it].fd;
        if ((*it).compare(nickname) == 0)
            continue;
        serverToClientMsg[channelMemberFd] += msg;
        setWriteEvent(channelMemberFd);
    }
}

void ConfigManager::sendDM(std::vector<std::string> &commandAndParams, int clientFd)
{
    std::string &targetNick = commandAndParams[1];
    if (memberMap.find(targetNick) == memberMap.end())
    {
        serverToClientMsg[clientFd] += ":irc.local 401 * " + targetNick + "  :No such nick\r\n";
        setWriteEvent(clientFd);
        return;
    }

    IrcMember &sourceMember = memberMap[fdNicknameMap[clientFd]];
    IrcMember &targetMember = memberMap[targetNick];

    serverToClientMsg[targetMember.fd] += makeMsg(sourceMember.nickname, sourceMember.username, sourceMember.hostname, targetMember.nickname, commandAndParams);
    setWriteEvent(targetMember.fd);
}

void ConfigManager::sendPrivateMsg(std::vector<std::string> &commandAndParams, int clientFd)
{
    if (commandAndParams.size() < 3)
    {
        serverToClientMsg[clientFd] += ":irc.local 461 * PRIVMSG :Not enough parameters.\r\n";
        setWriteEvent(clientFd);
        return;
    }

    if (fdNicknameMap.find(clientFd) == fdNicknameMap.end())
    {
        serverToClientMsg[clientFd] += ":irc.local 451 * PRIVMSG :You have not registered.\r\n";
        setWriteEvent(clientFd);
        return;
    }
    if (commandAndParams[1].size() > 0 && commandAndParams[1][0] == '#')
        sendChannel(commandAndParams, clientFd);
    else
        sendDM(commandAndParams, clientFd);
}