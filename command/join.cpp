#include "../ConfigManager.hpp"

static std::string getChannelMemberList(std::set<std::string> &operatorNickSet, std::set<std::string> &memberNickSet)
{
    std::string channelMemberList = "";
    std::set<std::string>::iterator it = memberNickSet.begin();
    for (; it != memberNickSet.end(); ++it)
    {
        if (operatorNickSet.find(*it) != operatorNickSet.end())
            channelMemberList += ("@" + *it + " ");
        else
            channelMemberList += (*it + " ");
    }
    return channelMemberList;
}

void ConfigManager::join(int clientFd, const std::string &channelName, std::vector<std::string> &commandAndParams)
{
    std::string clientNick = fdNicknameMap[clientFd];
    if (channelMap.find(channelName) == channelMap.end())
    {
        channelMap[channelName] = Channel(channelName);
        channelMap[channelName].operatorNickSet.insert(clientNick);
    }
    if (channelMap[channelName].memberNickSet.find(clientNick) != channelMap[channelName].memberNickSet.end())
        return;

    if (channelMap[channelName].invitedMemberSet.find(clientNick) != channelMap[channelName].invitedMemberSet.end())
    {
        channelMap[channelName].invitedMemberSet.erase(clientNick);
        memberMap[clientNick].invitedChannelSet.erase(channelName);
    }
    else
    {
        if (channelMap[channelName].inviteOnly)
        {
            serverToClientMsg[clientFd] += ":irc.local 473 " + clientNick + " #" + channelName + " :Cannot join channel (invite only)\r\n";
            setWriteEvent(clientFd);
            return;
        }
        if (channelMap[channelName].useKeyOnly)
        {
            if (commandAndParams.size() < 3 || commandAndParams[2].compare(channelMap[channelName].key) != 0)
            {
                serverToClientMsg[clientFd] += ":irc.local 475 " + clientNick + " #" + channelName + " :Cannot join channel (incorrect channel key)\r\n";
                setWriteEvent(clientFd);
                return;
            }
        }
        if (channelMap[channelName].isLimit)
        {
            if ((int)channelMap[channelName].memberNickSet.size() >= channelMap[channelName].limitCount)
            {
                serverToClientMsg[clientFd] += ":irc.local 471 " + clientNick + " #" + channelName + " :Cannot join channel (channel is full)\r\n";
                setWriteEvent(clientFd);
                return;
            }
        }
    }

    memberMap[clientNick].memberChannelSet.insert(channelName);
    channelMap[channelName].memberNickSet.insert(clientNick);
    std::string joinMsg = ":" + clientNick + "!" + memberMap[clientNick].username + "@" + memberMap[clientNick].hostname + " JOIN :#" + channelName + "\r\n";
    std::set<std::string>::iterator it = channelMap[channelName].memberNickSet.begin();
    for (; it != channelMap[channelName].memberNickSet.end(); ++it)
    {
        int channelMemberFd = memberMap[*it].fd;
        serverToClientMsg[channelMemberFd] += joinMsg;
        setWriteEvent(channelMemberFd);
    }

    std::string firstMsg = ":irc.local 353 " + clientNick + " = #" + channelName + " :" + getChannelMemberList(channelMap[channelName].operatorNickSet, channelMap[channelName].memberNickSet) + "\r\n";
    firstMsg += ":irc.local 366 " + clientNick + " #" + channelName + " :End of /NAMES list.\r\n";
    serverToClientMsg[clientFd] += firstMsg;
}

void ConfigManager::joinChannel(std::vector<std::string> &commandAndParams, int clientFd)
{
    if (commandAndParams.size() < 2)
    {
        serverToClientMsg[clientFd] += ":irc.local 461 * JOIN :Not enough parameters.\r\n";
        setWriteEvent(clientFd);
        return;
    }

    if (fdNicknameMap.find(clientFd) == fdNicknameMap.end())
    {
        serverToClientMsg[clientFd] += ":irc.local 451 * JOIN :You have not registered.\r\n";
        setWriteEvent(clientFd);
        return;
    }

    std::string &channels = commandAndParams[1];
    std::vector<std::string> splitChannels = split(channels, ",");
    for (int i = 0; i < (int)splitChannels.size(); i++)
    {
        if (splitChannels[i].size() == 0)
            continue;
        if (splitChannels[i][0] != '#')
        {
            serverToClientMsg[clientFd] += ":irc.local 476 " + memberMap[fdNicknameMap[clientFd]].nickname + " " + splitChannels[i] + ":Invalid channel name\r\n";
            setWriteEvent(clientFd);
            continue;
        }
        join(clientFd, splitChannels[i].substr(1), commandAndParams);
    }
}