#include "../ConfigManager.hpp"

void ConfigManager::processTopic(std::vector<std::string> &commandAndParams, int clientFd)
{
	if (fdNicknameMap.find(clientFd) == fdNicknameMap.end())
	{
		serverToClientMsg[clientFd] += std::string(":irc.local 451 * TOPIC :You have not registered.\r\n");
		setWriteEvent(clientFd);
		return;
	}

	if (commandAndParams.size() < 2)
	{
		serverToClientMsg[clientFd] += ":irc.local 461 " + fdNicknameMap[clientFd] + " TOPIC :Not enough parameters.\r\n";
        setWriteEvent(clientFd);
    	return;
	}

	std::string channel = commandAndParams[1];

	if (channel.size() == 0 || channel[0] != '#' || channelMap.find(channel.substr(1)) == channelMap.end())
	{
		serverToClientMsg[clientFd] += ":irc.local 403 " + fdNicknameMap[clientFd] + " " + channel + " :No such channel\r\n";
		setWriteEvent(clientFd);
		return;
	}

	std::string channelName = channel.substr(1);
	std::string clientNick = fdNicknameMap[clientFd];

	if (channelMap[channelName].memberNickSet.find(clientNick) == channelMap[channelName].memberNickSet.end())
	{
		serverToClientMsg[clientFd] += ":irc.local 442 " + fdNicknameMap[clientFd] + " #" + channelName + " :You're not on that channel!\r\n";
		setWriteEvent(clientFd);
        return;
	}

	if (channelMap[channelName].topicOpOnly && channelMap[channelName].operatorNickSet.find(clientNick) == channelMap[channelName].operatorNickSet.end())
	{
		serverToClientMsg[clientFd] += ":irc.local 482 " + fdNicknameMap[clientFd] + " #" + channelName + " :You must be a channel op or higher to change the topic.\r\n";
		setWriteEvent(clientFd);
        return;
	}

	if (commandAndParams.size() == 2)
	{
		serverToClientMsg[clientFd] += ":irc.local 332 " + fdNicknameMap[clientFd] + " " + channel + "\r\n";
		setWriteEvent(clientFd);
		return;
	}

	topic(clientFd, commandAndParams, channelName);
}

void ConfigManager::topic(int clientFd, const std::vector<std::string> &commandAndParams, const std::string &channelName)
{
	std::string clientNick = fdNicknameMap[clientFd];
	std::string newTopicMsg;
	for (int i = 2; i < (int)commandAndParams.size(); i++)
	{
		newTopicMsg += commandAndParams[i];
		if (i < (int)commandAndParams.size() - 1)
			newTopicMsg += " ";
	}

	std::string displayMsg = ":" + clientNick + "!" + memberMap[clientNick].username + "@" + memberMap[clientNick].hostname + " TOPIC #" + channelName + " " + newTopicMsg + "\r\n";
	std::set<std::string>::iterator it = channelMap[channelName].memberNickSet.begin();
	for (; it != channelMap[channelName].memberNickSet.end(); ++it)
	{
		int channelMemberFd = memberMap[*it].fd;
		serverToClientMsg[channelMemberFd] += displayMsg;
		setWriteEvent(channelMemberFd);
	}
}
