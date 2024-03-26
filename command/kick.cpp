#include "../ConfigManager.hpp"

// KICK #aa kk__ :msg

// :kk!root@127.0.0.1 KICK #aa kk__ :msg
// :kk!root@127.0.0.1 KICK #aa kk__ :msg
// :kk!root@127.0.0.1 KICK #aa kk__ :msg

//: irc.local 403 kkk hello :No such channel
// 타겟 닉네임 못찾음
//: irc.local 401 kkk target_nick :No such nick
//: irc.local 482 kk__ #aa :You must be a channel op or higher to kick a more privileged user.

static std::string makeMsg(std::vector<std::string> &commandAndParams)
{
    std::string msg;
    for (int i = 3; i < commandAndParams.size(); i++)
    {
        if (i == 3 && commandAndParams[3].size() != 0 && commandAndParams[3][0] == ':')
            commandAndParams[3] = commandAndParams[3].substr(1);
        msg += commandAndParams[i];
        if (i != commandAndParams.size() - 1)
            msg += " ";
    }
    msg += "\r\n";
    return msg;
}

void ConfigManager::kickMember(std::vector<std::string> &commandAndParams, int clientFd)
{
    if (commandAndParams.size() < 3)
    {
        serverToClientMsg[clientFd] += ":irc.local 461 * KICK :Not enough parameters.\r\n";
        setWriteEvent(clientFd);
        return;
    }

    if (fdNicknameMap.find(clientFd) == fdNicknameMap.end())
    {
        serverToClientMsg[clientFd] += ":irc.local 451 * KICK :You have not registered.\r\n";
        setWriteEvent(clientFd);
        return;
    }

    std::string channelName = commandAndParams[1];
    if (channelName.size() == 0 || channelName[0] != '#' || channelMap.find(channelName.substr(1)) == channelMap.end())
    {
        serverToClientMsg[clientFd] += ":irc.local 403 * " + channelName + " :No such channel\r\n";
        setWriteEvent(clientFd);
        return;
    }

    channelName = channelName.substr(1);
    if (channelMap[channelName].operatorNickSet.find(fdNicknameMap[clientFd]) == channelMap[channelName].operatorNickSet.end())
    {
        serverToClientMsg[clientFd] += ":irc.local 482 * #" + channelName + " :You must be a channel op or higher to kick a more privileged user\r\n";
        setWriteEvent(clientFd);
        return;
    }

    std::string targetNick = commandAndParams[2];

    if (channelMap[channelName].memberNickSet.find(targetNick) == channelMap[channelName].memberNickSet.end())
    {
        serverToClientMsg[clientFd] += ":irc.local 401 #" + channelName + " " + targetNick + " :No such nick\r\n";
        setWriteEvent(clientFd);
        return;
    }

    std::string nick = fdNicknameMap[clientFd];
    std::string kickMsg = ":" + nick + "!" + memberMap[nick].username + "@127.0.0.1 KICK #" + channelName + " " + targetNick + " :" + makeMsg(commandAndParams) + "\r\n";

    std::set<std::string>::iterator it = channelMap[channelName].memberNickSet.begin();
    for (; it != channelMap[channelName].memberNickSet.end(); ++it)
    {
        int channelMemberFd = memberMap[*it].fd;
        serverToClientMsg[channelMemberFd] += kickMsg;
        setWriteEvent(channelMemberFd);
    }
    channelMap[channelName].operatorNickSet.erase(targetNick);
    channelMap[channelName].memberNickSet.erase(targetNick);
}