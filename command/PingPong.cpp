#include "../ConfigManager.hpp"

void ConfigManager::PingPongInteraction(const std::vector<std::string>& splitMessage, int clientFd) 
{
	if (fdNicknameMap.find(clientFd) == fdNicknameMap.end())
	{
		serverToClientMsg[clientFd] += std::string(":irc.local 451 * PING :You have not registered.\r\n");
		setWriteEvent(clientFd);
		return;
	}

    if (splitMessage.size() < 2)
    {
        serverToClientMsg[clientFd] += ":irc.local 461 * PING :Not enough parameters.\r\n";
		setWriteEvent(clientFd);
        return;
    }

	std::string response;
	if (splitMessage.size() == 2) {
    	response = "PONG " + splitMessage[1] + "\r\n";
	}
	else {
		response = "PONG " + splitMessage[1] + " " + splitMessage[2] + "\r\n";
	}
	serverToClientMsg[clientFd] += response;
	setWriteEvent(clientFd);
}