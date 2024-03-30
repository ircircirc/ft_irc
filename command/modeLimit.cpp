#include "../ConfigManager.hpp"

// MODE #hi +l
// :irc.local 696 a #hi l * :You must specify a parameter for the limit mode. Syntax: <limit>.

// 성공
// MODE #hi +l 2
// :a!root@127.0.0.1 MODE #hi +l :2
// :a!root@127.0.0.1 MODE #hi +l :2

// MODE #hi :-l
// :a!root@127.0.0.1 MODE #hi :-l

// + 명령은 기존의 값과 같아도 모두에게 메시지 보냄
// - 명령은 기존에 +일때만 메시지 보냄

// 문자포함될때는 atoi처럼 처리함
// MODE #hi +l 3a
// :a!root@127.0.0.1 MODE #hi +l :3

// 꽉 찼을때
// JOIN #hi
// :irc.local 471 b #hi :Cannot join channel (channel is full)

// operator가 초대하면 인원에 관계없이 들어올 수 있다.

void ConfigManager::processModeLimit(int clientFd, bool sign, std::string &channelName, std::vector<std::string> &commandAndParams)
{
    char signChar = '+';
    if (!sign)
        signChar = '-';
    if (sign == true)
    {
        if (commandAndParams.size() < 4)
        {
            serverToClientMsg[clientFd] += ":irc.local 696 " + fdNicknameMap[clientFd] + " #" + channelName + " k * :You must specify a parameter for the limit mode. Syntax: <limit>\r\n";
            setWriteEvent(clientFd);
            return;
        }
        channelMap[channelName].isLimit = true;
        channelMap[channelName].limitCount = std::atoi(commandAndParams[3].c_str());

        std::ostringstream oss;
        oss << channelMap[channelName].limitCount;
        std::string limitStr = oss.str();

        std::string msg = ":" + fdNicknameMap[clientFd] + "!" + memberMap[fdNicknameMap[clientFd]].username + "@" + memberMap[fdNicknameMap[clientFd]].hostname + " MODE #" + channelName + " " + "+l :" + limitStr + "\r\n";
        std::set<std::string>::iterator it = channelMap[channelName].memberNickSet.begin();
        for (; it != channelMap[channelName].memberNickSet.end(); ++it)
        {
            int channelMemberFd = memberMap[*it].fd;
            serverToClientMsg[channelMemberFd] += msg;
            setWriteEvent(channelMemberFd);
        }
    }
    else if (sign == false)
    {
        if (channelMap[channelName].isLimit == false)
            return;
        channelMap[channelName].isLimit = false;
        std::string msg = ":" + fdNicknameMap[clientFd] + "!" + memberMap[fdNicknameMap[clientFd]].username + "@127.0.0.1 MODE #" + channelName + " " + " :-l\r\n";
        std::set<std::string>::iterator it = channelMap[channelName].memberNickSet.begin();
        for (; it != channelMap[channelName].memberNickSet.end(); ++it)
        {
            int channelMemberFd = memberMap[*it].fd;
            serverToClientMsg[channelMemberFd] += msg;
            setWriteEvent(channelMemberFd);
        }
    }
}