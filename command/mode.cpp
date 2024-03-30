#include "../ConfigManager.hpp"

void ConfigManager::processMode(std::vector<std::string> &commandAndParams, int clientFd)
{
    if (commandAndParams.size() < 2)
    {
        serverToClientMsg[clientFd] += ":irc.local 461 * MODE :Not enough parameters.\r\n";
        setWriteEvent(clientFd);
        return;
    }

    if (fdNicknameMap.find(clientFd) == fdNicknameMap.end())
    {
        serverToClientMsg[clientFd] += ":irc.local 451 * MODE :You have not registered.\r\n";
        setWriteEvent(clientFd);
        return;
    }

    if (commandAndParams.size() == 2)
        return;

    if (commandAndParams[1].size() == 0 || commandAndParams[1][0] != '#')
        return;

    std::string channelName = commandAndParams[1].substr(1);

    if (channelName.size() != 0 && channelMap.find(channelName) == channelMap.end())
    {
        serverToClientMsg[clientFd] += ":irc.local 403 " + fdNicknameMap[clientFd] + " " + channelName + " :No such channel\r\n";
        setWriteEvent(clientFd);
        return;
    }

    std::string nick = fdNicknameMap[clientFd];
    Channel &channel = channelMap[channelName];

    if (channel.memberNickSet.find(nick) == channel.memberNickSet.end() || channel.operatorNickSet.find(nick) == channel.operatorNickSet.end())
    {
        serverToClientMsg[clientFd] += ": irc.local 482 " + fdNicknameMap[clientFd] + " " + channelName + " :You must be a channel op or higher to set channel mode";
        setWriteEvent(clientFd);
        return;
    }

    std::string modes = commandAndParams[2];
    if (modes.size() == 0)
        return;
    bool sign = true;
    if (modes[0] == '-')
        sign = false;
    if (modes[0] == '+' || modes[0] == '-')
        modes = modes.substr(1);
    for (int i = 0; i < (int)modes.size(); i++)
    {
        if (modes[i] == 'i')
            processModeInvite(clientFd, sign, channelName);
        else if(modes[i] == 'k')
            processModeKey(clientFd, sign, channelName, commandAndParams);
        else if(modes[i] == 'o')
            processModeOperator(clientFd, sign, channelName, commandAndParams);
        else if(modes[i] == 'l')
            processModeLimit(clientFd, sign, channelName, commandAndParams);
        else if(modes[i] == 't')
            processModeTopic(clientFd, sign, channelName);
        else
        {
            serverToClientMsg[clientFd] += ":irc.local 472 " + fdNicknameMap[clientFd] + " " + modes[i] + " :is not a recognised channel mode.\r\n";
            setWriteEvent(clientFd);
        }
    }
}
