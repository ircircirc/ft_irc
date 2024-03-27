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
    std::string channelName = commandAndParams[1];

    // 처음에 연결되면 mode로 물어봄 -> 닉네임에 대한 모드? 인듯?? -> 일단 미처리
    if (channelName.size() == 0 || channelName[0] != '#')
    {
        return;
    }

    channelName = channelName.substr(1);
    if (channelName.size() != 0 && channelMap.find(channelName) == channelMap.end())
    {
        serverToClientMsg[clientFd] += ":irc.local 403 " + fdNicknameMap[clientFd] + " " + channelName + " :No such channel\r\n";
        setWriteEvent(clientFd);
        return;
    }

    // mode #channel -> 이렇게 온 경우 채널정보 반환하는듯? -> 나중에 처리할 생각
    if (commandAndParams.size() == 2)
        return;

    std::string nick = fdNicknameMap[clientFd];
    Channel &channel = channelMap[channelName];

    // operator가 아니라면 if문에 걸림
    if (channel.memberNickSet.find(nick) == channel.memberNickSet.end() || channel.operatorNickSet.find(nick) == channel.operatorNickSet.end())
    {
        serverToClientMsg[clientFd] += ": irc.local 482 " + fdNicknameMap[clientFd] + " " + channelName + " :You must be a channel op or higher to set channel mode";
        setWriteEvent(clientFd);
        return;
    }

    std::string modes = commandAndParams[2];
    if (modes.size() == 0)
        return;
    bool sign;
    char signChar;
    if (modes[0] == '-')
    {
        sign = false;
        signChar = '-';
    }
    else
    {
        sign = true;
        signChar = '+';
    }
    if (modes[0] == '+' || modes[0] == '-')
        modes = modes.substr(1);
    for (int i = 0; i < modes.size(); i++)
    {
        if (modes[i] == 'i')
        {
            if (channelMap[channelName].inviteOnly == sign)
                continue;
            channelMap[channelName].inviteOnly = sign;
            std::string msg = ":" + fdNicknameMap[clientFd] + "!" + memberMap[fdNicknameMap[clientFd]].username + "@127.0.0.1 MODE #" + channelName + " :" + signChar + "i\r\n";
            std::set<std::string>::iterator it = channelMap[channelName].memberNickSet.begin();
            for (; it != channelMap[channelName].memberNickSet.end(); ++it)
            {
                int channelMemberFd = memberMap[*it].fd;
                serverToClientMsg[channelMemberFd] += msg;
                setWriteEvent(channelMemberFd);
            }
        }
        else
        {
            serverToClientMsg[clientFd] += ":irc.local 472 " + fdNicknameMap[clientFd] + " " + modes[i] + " :is not a recognised channel mode.\r\n";
            setWriteEvent(clientFd);
        }
    }
}

// 모드가 변경되지 않으면 메시지가 전달되지 않음
// MODE #hi +i

// operator가 아닌 사람이 채널모드 변경 불가능, 채널에 참가하지 않은경우도 같은 메시지
// MODE #hi +i
//: irc.local 482 b #hi :You must be a channel op or higher to set channel mode i (inviteonly).

// mode #hehe 하면 모드정보 오는듯?
// mode #hehe
// :irc.local 324 kkk #hehe :+nt
// :irc.local 329 kkk #hehe :1711437811

// +생략해 사용가능하다.
// mode #hehe i
// :kkk!a@127.0.0.1 MODE #hehe :+i

// 잘못된 모드
// mode #hehe asdfsd
//  :irc.local 472 kkk a :is not a recognised channel mode.
//  :irc.local 472 kkk d :is not a recognised channel mode.
//  :irc.local 472 kkk f :is not a recognised channel mode.
//  :irc.local 472 kkk d :is not a recognised channel mode.

// 모드 변경한 메시지가 채널의 모두에게 알려짐
// MODE #hi +i
// :c!root@127.0.0.1 MODE #hi :+i
// :c!root@127.0.0.1 MODE #hi :+i