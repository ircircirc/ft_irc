#include "../ConfigManager.hpp"

void ConfigManager::partChannel(std::vector<std::string> &commandAndParams, int clientFd)
{
	if (commandAndParams.size() < 2)
	{
		serverToClientMsg[clientFd] += std::string("ERROR :No channel name provided\r\n");
		return;
	}

	// register 된 사람인지 확인
	if (fdNicknameMap.find(clientFd) == fdNicknameMap.end())
	{
		serverToClientMsg[clientFd] += std::string(":irc.local 451 * PING :You have not registered.\r\n");
		setWriteEvent(clientFd);
		return;
	}

	// part 문법 오류 확인
	std::string channels = commandAndParams[1];
	std::vector<std::string> splitChannels = split(channels, ",");	// ','을 구분자로 하여 part하고자 하는 channel을 추가할 수 있다. (복수의 channel에서 part)
	for (int i = 0; i < (int)splitChannels.size(); i++)
	{
		if (splitChannels[i].size() == 0)	// 인자가 빈문자열인 경우 continue. 
			continue;
		if (splitChannels[i][0] != '#')	// '#'이 있는지 확인 (상용서버의 동작) '#'이 없으면 에러. 
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
		// :irc.local 403 orange portion :No such channel
		serverToClientMsg[clientFd] += ":irc.local 403 " + memberMap[fdNicknameMap[clientFd]].nickname + " " + channelName + ":No such channel\r\n";
		setWriteEvent(clientFd);
        return;
	}

	if (channelMap[channelName].memberNickSet.find(clientNick) == channelMap[channelName].memberNickSet.end())
	{
		// irc.local 442 dragon #coffee :You're not on that channel
		serverToClientMsg[clientFd] += ":irc.local 442 " + memberMap[fdNicknameMap[clientFd]].nickname + " #" + channelName + " :You're not on that channel\r\n";
		setWriteEvent(clientFd);
        return;
	}

	// 채널에 참여한 인원에게 공통으로 알리는 메시지 (실제 메시지 내용 확인 대조)
	// 127.000.000.001.06667-127.000.000.001.63684: :dragon!root@127.0.0.1 PART :#coffee
	// 127.000.000.001.06667-127.000.000.001.63666: :dragon!root@127.0.0.1 PART :#coffee
	// 127.000.000.001.06667-127.000.000.001.63692: :dragon!root@127.0.0.1 PART :#coffee

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
	if (channelMap[channelName].memberNickSet.empty())	// 마지막 인원이 나갈 때
    {
        channelMap.erase(channelName);
	}
}
