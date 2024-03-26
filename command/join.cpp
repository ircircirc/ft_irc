#include "../ConfigManager.hpp"

// 채널에 처믕 입장하는 사람에게 보내는 메시지임 -> @가 오퍼레이터 인듯 -> 추후 오퍼레이터 처리할것
// :irc.local 353 b = #hello :@a b
// :irc.local 366 b #hello :End of /NAMES list

// 실제 예시

// :yw!root@127.0.0.1 JOIN :#42seoul
// :irc.local 353 yw = #42seoul :@yw
// :irc.local 366 yw #42seoul :End of /NAMES list.

// :kk!root@127.0.0.1 JOIN :#42seoul
// :irc.local 353 kk = #42seoul :@yw kk
// :irc.local 366 kk #42seoul :End of /NAMES list.

// :yw! root@root JOIN :#42seoul
// :irc.local 353 yw = #42seoul :yw
// :irc.local 366 yw #42seoul :End of /NAMES list

// :kk! root@root JOIN :#42seoul

// :irc.local 353 kk = #42seoul :kk yw
// :irc.local 366 kk #42seoul :End of /NAMES list
// :kk! root@root JOIN :#42seoul

static std::string getChannelMemberList(std::set<std::string> &memberNickSet, std::map<std::string, IrcMember> &memberMap)
{
    std::string channelMemberList = "";
    std::set<std::string>::iterator it = memberNickSet.begin();
    for (; it != memberNickSet.end(); ++it)
    {
        channelMemberList += (memberMap[*it].nickname + " ");
    }
    return channelMemberList;
}

void ConfigManager::join(int clientFd, const std::string &channelName)
{
    std::string clientNick = fdNicknameMap[clientFd];
    //채널이 만들어 져야할 경우
    if (channelMap.find(channelName) == channelMap.end())
        channelMap[channelName] = Channel(channelName);
    //이미 가입한 채널에 join할 경우
    if (channelMap[channelName].memberNickSet.find(clientNick) != channelMap[channelName].memberNickSet.end())
        return;

    memberMap[clientNick].memberChannelSet.insert(channelName);
    channelMap[channelName].memberNickSet.insert(clientNick);

    //채널에 참여한 인원에게 공통으로 알리는 메시지
    std::string joinMsg = ":" + clientNick + "!" + memberMap[clientNick].username + "@" + memberMap[clientNick].hostname + " JOIN :#" + channelName + "\r\n";
    std::set<std::string>::iterator it = channelMap[channelName].memberNickSet.begin();
    for (; it != channelMap[channelName].memberNickSet.end(); ++it)
    {
        int channelMemberFd = memberMap[*it].fd;
        serverToClientMsg[channelMemberFd] += joinMsg;
        setWriteEvent(channelMemberFd);
    }

    std::string firstMsg = ":irc.local 353 " + clientNick + " = #" + channelName + " :" + getChannelMemberList(channelMap[channelName].memberNickSet, memberMap) + "\r\n";
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
    std::vector<std::string> spiltChannels = split(channels, ",");
    for (int i = 0; i < spiltChannels.size(); i++)
    {
        if (spiltChannels[i].size() == 0)
            continue;
        if (spiltChannels[i][0] != '#')
        {
            serverToClientMsg[clientFd] += ":irc.local 476 " + memberMap[fdNicknameMap[clientFd]].nickname + " " + spiltChannels[i] + ":Invalid channel name\r\n";
            setWriteEvent(clientFd);
            continue;
        }
        join(clientFd, spiltChannels[i].substr(1));
    }
}