#include "../ConfigManager.hpp"

void ConfigManager::inviteMember(std::vector<std::string> &commandAndParams, int clientFd)
{
    if (commandAndParams.size() < 3)
    {
        serverToClientMsg[clientFd] += ":irc.local 461 * INVITE :Not enough parameters.\r\n";
        setWriteEvent(clientFd);
        return;
    }

    if (fdNicknameMap.find(clientFd) == fdNicknameMap.end())
    {
        serverToClientMsg[clientFd] += ":irc.local 451 * INVITE :You have not registered.\r\n";
        setWriteEvent(clientFd);
        return;
    }

    std::string targetNick = commandAndParams[1];
    if (memberMap.find(targetNick) == memberMap.end())
    {
        serverToClientMsg[clientFd] += ":irc.local 401 " + fdNicknameMap[clientFd] + " " + targetNick + " :No such nick\r\n";
        setWriteEvent(clientFd);
        return;
    }

    std::string channelName = commandAndParams[2];
    if (channelName.size() == 0 || channelName[0] != '#' || channelMap.find(channelName.substr(1)) == channelMap.end())
    {
        serverToClientMsg[clientFd] += ":irc.local 403 * " + channelName + " :No such channel\r\n";
        setWriteEvent(clientFd);
        return;
    }

    channelName = channelName.substr(1);
    if (channelMap[channelName].operatorNickSet.find(fdNicknameMap[clientFd]) == channelMap[channelName].operatorNickSet.end())
    {
        serverToClientMsg[clientFd] += ":irc.local 482 * #" + channelName + " :You must be a channel op or higher to send an invite\r\n";
        setWriteEvent(clientFd);
        return;
    }

    if (channelMap[channelName].memberNickSet.find(targetNick) != channelMap[channelName].memberNickSet.end())
    {
        serverToClientMsg[clientFd] += ":irc.local 443 " + fdNicknameMap[clientFd] + " " + targetNick + " #" + channelName + " :is already on channel\r\n";
        setWriteEvent(clientFd);
        return;
    }

    channelMap[channelName].invitedMemberSet.insert(targetNick);
    memberMap[targetNick].invitedChannelSet.insert(channelName);

    IrcMember &sender = memberMap[fdNicknameMap[clientFd]];
    serverToClientMsg[memberMap[targetNick].fd] += ":" + sender.nickname + "!" + sender.username + "@" + sender.hostname + " INVITE " + targetNick + " :#" + channelName + "\r\n";
    setWriteEvent(memberMap[targetNick].fd);
}