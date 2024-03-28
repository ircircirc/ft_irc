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

static std::string getChannelMemberList(std::set<std::string> &operatorNickSet, std::set<std::string> &memberNickSet, std::map<std::string, IrcMember> &memberMap)
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
    // 채널이 만들어 져야할 경우
    if (channelMap.find(channelName) == channelMap.end())
    {
        channelMap[channelName] = Channel(channelName);
        channelMap[channelName].operatorNickSet.insert(clientNick);
    }
    // 이미 가입한 채널에 join할 경우
    if (channelMap[channelName].memberNickSet.find(clientNick) != channelMap[channelName].memberNickSet.end())
        return;

    // 초대 받았으면 바로 입장
    if (channelMap[channelName].invitedMemberSet.find(clientNick) != channelMap[channelName].invitedMemberSet.end())
    {
        channelMap[channelName].invitedMemberSet.erase(clientNick);
        memberMap[clientNick].invitedChannelSet.erase(channelName);
    }
    else
    {
        // 초대전용인 경우 퇴장
        if (channelMap[channelName].inviteOnly)
        {
            serverToClientMsg[clientFd] += ":irc.local 473 " + clientNick + " #" + channelName + " :Cannot join channel (invite only)\r\n";
            setWriteEvent(clientFd);
            return;
        }
        // 키 모드면 키 확인
        if (channelMap[channelName].useKeyOnly)
        {
            if (commandAndParams.size() < 3 || commandAndParams[2].compare(channelMap[channelName].key) != 0)
            {
                serverToClientMsg[clientFd] += ":irc.local 475 " + clientNick + " #" + channelName + " :Cannot join channel (incorrect channel key)\r\n";
                setWriteEvent(clientFd);
                return;
            }
        }
        // 초대전용 아니면서 제한 모드면 제한인원 확인
        if (channelMap[channelName].isLimit)
        {
            if (channelMap[channelName].memberNickSet.size() >= channelMap[channelName].limitCount)
            {
                serverToClientMsg[clientFd] += ":irc.local 471 " + clientNick + " #" + channelName + " :Cannot join channel (channel is full)\r\n";
                setWriteEvent(clientFd);
                return;
            }
        }
    }

    memberMap[clientNick].memberChannelSet.insert(channelName);
    channelMap[channelName].memberNickSet.insert(clientNick);

    // 채널에 참여한 인원에게 공통으로 알리는 메시지
    //  std::string joinMsg = ":" + clientNick + "!" + memberMap[clientNick].username + "@" + memberMap[clientNick].hostname + " JOIN :#" + channelName + "\r\n";
    std::string joinMsg = ":" + clientNick + "!" + memberMap[clientNick].username + "@127.0.0.1 JOIN :#" + channelName + "\r\n";
    std::set<std::string>::iterator it = channelMap[channelName].memberNickSet.begin();
    for (; it != channelMap[channelName].memberNickSet.end(); ++it)
    {
        int channelMemberFd = memberMap[*it].fd;
        serverToClientMsg[channelMemberFd] += joinMsg;
        setWriteEvent(channelMemberFd);
    }

    std::string firstMsg = ":irc.local 353 " + clientNick + " = #" + channelName + " :" + getChannelMemberList(channelMap[channelName].operatorNickSet, channelMap[channelName].memberNickSet, memberMap) + "\r\n";
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
    for (int i = 0; i < splitChannels.size(); i++)
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