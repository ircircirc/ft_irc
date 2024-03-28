#include "../ConfigManager.hpp"

void ConfigManager::processTopic(std::vector<std::string> &commandAndParams, int clientFd)
{
	// 1) You have not registered (미등록 사용자 에러 처리)
	if (fdNicknameMap.find(clientFd) == fdNicknameMap.end())
	{
		serverToClientMsg[clientFd] += std::string(":irc.local 451 * TOPIC :You have not registered.\r\n");
		setWriteEvent(clientFd);
		return;
	}

	// 2) Not enought parameters (parameter 부족 에러 처리)
	if (commandAndParams.size() < 2)
	{
		serverToClientMsg[clientFd] += ":irc.local 461 " + fdNicknameMap[clientFd] + " TOPIC :Not enough parameters.\r\n";
        setWriteEvent(clientFd);
    	return;
	}

	// 3) No such channel (#이 없거나, 없는 channel이면 error)
	std::string channel = commandAndParams[1];

	if (channel.size() == 0 || channel[0] != '#' || channelMap.find(channel.substr(1)) == channelMap.end())
	{
		serverToClientMsg[clientFd] += ":irc.local 403 " + fdNicknameMap[clientFd] + " " + channel + " :No such channel\r\n";
		setWriteEvent(clientFd);
		return;
	}

	std::string channelName = channel.substr(1);
	std::string clientNick = fdNicknameMap[clientFd];

	// 4) You're not on that channel (채널은 존재하지만 채널에 join않은 경우)
	if (channelMap[channelName].memberNickSet.find(clientNick) == channelMap[channelName].memberNickSet.end())
	{
		serverToClientMsg[clientFd] += ":irc.local 442 " + fdNicknameMap[clientFd] + " #" + channelName + " :You're not on that channel!\r\n";
		setWriteEvent(clientFd);
        return;
	}

	// 5) 사용자가 operator 여부 및 MODE +-t에 따른 처리 (채널도 있고, 사용자가 채널에 join한 상태인데 operator)
	if (channelMap[channelName].topicOpOnly && channelMap[channelName].operatorNickSet.find(clientNick) == channelMap[channelName].operatorNickSet.end())
	{
		serverToClientMsg[clientFd] += ":irc.local 482 " + fdNicknameMap[clientFd] + " #" + channelName + " :You must be a channel op or higher to change the topic.\r\n";
		setWriteEvent(clientFd);
        return;
	}


	// 6) 332 (존재하는 채널이고, 사용자가 채널에 join한 상태인데, 변경할 topic 매개변수를 입력하지 않은 경우)
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
	for (int i = 2; i < commandAndParams.size(); i++)
	{
		newTopicMsg += commandAndParams[i];
		if (i < commandAndParams.size() - 1)
			newTopicMsg += " ";
	}
	std::cout << "확인!! :" << newTopicMsg << std::endl;

	std::string displayMsg = ":" + clientNick + "!" + memberMap[clientNick].username + "@" + memberMap[clientNick].hostname + " TOPIC #" + channelName + " " + newTopicMsg + "\r\n";
	std::set<std::string>::iterator it = channelMap[channelName].memberNickSet.begin();
	for (; it != channelMap[channelName].memberNickSet.end(); ++it)
	{
		int channelMemberFd = memberMap[*it].fd;
		serverToClientMsg[channelMemberFd] += displayMsg;
		setWriteEvent(channelMemberFd);
	}
}
