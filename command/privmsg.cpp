#include "../ConfigManager.hpp"

// 자신을 제외한 채널 멤버에게 전송
// PRIVMSG #42seoul :hello my name is yw3
// :yw3!root@127.0.0.1 PRIVMSG #42seoul :hello my name is yw3
// :yw3!root@127.0.0.1 PRIVMSG #42seoul :hello my name is yw3

// 등록되지 않은 채널에 전송
// :irc.local 404 kkk #42seoul :You cannot send external messages to this channel whilst the +n (noextmsg) mode is set.

// 채널 없음
// privmsg #hi : hello
// :irc.local 403 kkk #hi :No such channel

static std::string makeMsg(std::string &nickname, std::string &username, std::string &hostname, std::string targetName, std::vector<std::string> &commandAndParams)
{
    std::string msg;
    msg += std::string(":" + nickname + "!" + username + "@" + hostname + " ");
    msg += std::string("PRIVMSG " + targetName + " ");
    for (int i = 2; i < (int)commandAndParams.size(); i++)
    {
        msg += commandAndParams[i];
        if (i != (int)commandAndParams.size() - 1)
            msg += " ";
    }
    msg += "\r\n";
    return msg;
}

void ConfigManager::sendChannel(std::vector<std::string> &commandAndParams, int clientFd)
{
    std::string channelName = commandAndParams[1].substr(1);
    std::string &nickname = fdNicknameMap[clientFd];
    // 채널을 찾을 수 없으면
    if (channelMap.find(channelName) == channelMap.end())
    {
        serverToClientMsg[clientFd] += ":irc.local 403 " + nickname + " #" + channelName + "  :No such channel\r\n";
        setWriteEvent(clientFd);
        return;
    }
    // 채널에 참여하고 있지 않으면
    if (channelMap[channelName].memberNickSet.find(nickname) == channelMap[channelName].memberNickSet.end())
    {
        serverToClientMsg[clientFd] += ":irc.local 403 " + nickname + " #" + channelName + "  :Cannot send to channel\r\n";
        setWriteEvent(clientFd);
        return;
    }
    std::set<std::string>::iterator it = channelMap[channelName].memberNickSet.begin();
    std::string msg = makeMsg(nickname, memberMap[nickname].username, memberMap[nickname].hostname, ("#" + channelName), commandAndParams);
    for (; it != channelMap[channelName].memberNickSet.end(); ++it)
    {
        int channelMemberFd = memberMap[*it].fd;
        // 자기자신에게 메시지를 안보냄
        if ((*it).compare(nickname) == 0)
            continue;
        serverToClientMsg[channelMemberFd] += msg;
        setWriteEvent(channelMemberFd);
    }
}

void ConfigManager::sendDM(std::vector<std::string> &commandAndParams, int clientFd)
{
    // 메시지를 받을 사람의 닉네임을 찾지 못했을때
    std::string &targetNick = commandAndParams[1];
    if (memberMap.find(targetNick) == memberMap.end())
    {
        serverToClientMsg[clientFd] += ":irc.local 401 * " + targetNick + "  :No such nick\r\n";
        setWriteEvent(clientFd);
        return;
    }

    IrcMember &sourceMember = memberMap[fdNicknameMap[clientFd]];
    IrcMember &targetMember = memberMap[targetNick];

    serverToClientMsg[targetMember.fd] += makeMsg(sourceMember.nickname, sourceMember.username, sourceMember.hostname, targetMember.nickname, commandAndParams);
    setWriteEvent(targetMember.fd);
}

void ConfigManager::sendPrivateMsg(std::vector<std::string> &commandAndParams, int clientFd)
{
    if (commandAndParams.size() < 3)
    {
        serverToClientMsg[clientFd] += ":irc.local 461 * PRIVMSG :Not enough parameters.\r\n";
        setWriteEvent(clientFd);
        return;
    }

    // 등록된 멤버가 아닐때
    if (fdNicknameMap.find(clientFd) == fdNicknameMap.end())
    {
        serverToClientMsg[clientFd] += ":irc.local 451 * PRIVMSG :You have not registered.\r\n";
        setWriteEvent(clientFd);
        return;
    }
    if (commandAndParams[1].size() > 0 && commandAndParams[1][0] == '#')
        sendChannel(commandAndParams, clientFd);
    else
        sendDM(commandAndParams, clientFd);
}