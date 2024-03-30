#include "../ConfigManager.hpp"

void ConfigManager::partChannel(std::vector<std::string> &commandAndParams, int clientFd)
{
	if (commandAndParams.size() < 2)
	{
		serverToClientMsg[clientFd] += std::string("ERROR :No channel name provided\r\n");
		return;
	}

	if (fdNicknameMap.find(clientFd) == fdNicknameMap.end())
	{
		serverToClientMsg[clientFd] += std::string(":irc.local 451 * PART :You have not registered.\r\n");
		setWriteEvent(clientFd);
		return;
	}

	std::string channels = commandAndParams[1];
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
		part(clientFd, splitChannels[i].substr(1));
	}
}

void ConfigManager::part(int clientFd, const std::string &channelName)
{
	std::string clientNick = fdNicknameMap[clientFd];

	if (channelMap.find(channelName) == channelMap.end())
	{
		serverToClientMsg[clientFd] += ":irc.local 403 " + memberMap[fdNicknameMap[clientFd]].nickname + " " + channelName + ":No such channel\r\n";
		setWriteEvent(clientFd);
        return;
	}

	if (channelMap[channelName].memberNickSet.find(clientNick) == channelMap[channelName].memberNickSet.end())
	{
		serverToClientMsg[clientFd] += ":irc.local 442 " + memberMap[fdNicknameMap[clientFd]].nickname + " #" + channelName + " :You're not on that channel\r\n";
		setWriteEvent(clientFd);
        return;
	}

	std::string partMsg = ":" + clientNick + "!" + memberMap[clientNick].username + "@" + memberMap[clientNick].hostname + " PART :#" + channelName + "\r\n";
	std::set<std::string>::iterator it = channelMap[channelName].memberNickSet.begin();
	for (; it != channelMap[channelName].memberNickSet.end(); ++it)
	{
		int channelMemberFd = memberMap[*it].fd;
		serverToClientMsg[channelMemberFd] += partMsg;
		setWriteEvent(channelMemberFd);
	}
	
	channelMap[channelName].operatorNickSet.erase(clientNick);
	channelMap[channelName].memberNickSet.erase(clientNick);
	memberMap[clientNick].memberChannelSet.erase(channelName);
	if (channelMap[channelName].memberNickSet.empty())
    {
        channelMap.erase(channelName);
	}
}
