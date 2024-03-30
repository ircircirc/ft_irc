#include "../ConfigManager.hpp"

// 파라미터가 적을때
//  MODE #hi -o
//  :irc.local 696 a #hi o * :You must specify a parameter for the op mode. Syntax: <nick>.

// 없는 닉네임
// MODE #hi -o noNick
// :irc.local 401 a noNick :No such nick

// operator 아닌 사용자 -o시 무반응
// 채널에 없는 사용자 초대시 무반응

// 최초 operator도 다른 operator가 -o 가능
// 자기자신 -o 가능

// 성공시
// MODE #hi +o b
// :a!root@127.0.0.1 MODE #hi +o :b
// :a!root@127.0.0.1 MODE #hi +o :b
// :a!root@127.0.0.1 MODE #hi +o :b

void ConfigManager::processModeOperator(int clientFd, bool sign, std::string &channelName, std::vector<std::string> &commandAndParams)
{
    if (commandAndParams.size() < 4)
    {
        serverToClientMsg[clientFd] += ":irc.local 696 " + fdNicknameMap[clientFd] + " #" + channelName + " k * :You must specify a parameter for the key mode. Syntax: <nick>\r\n";
        setWriteEvent(clientFd);
        return;
    }
    std::string targetNick = commandAndParams[3];
    if (memberMap.find(targetNick) == memberMap.end())
    {
        serverToClientMsg[clientFd] += ":irc.local 401 " + fdNicknameMap[clientFd] + " " + targetNick + " :No such nick\r\n";
        setWriteEvent(clientFd);
        return;
    }
    if (channelMap[channelName].memberNickSet.find(targetNick) == channelMap[channelName].memberNickSet.end())
        return;

    if (sign == true)
    {
        if (channelMap[channelName].operatorNickSet.find(targetNick) != channelMap[channelName].operatorNickSet.end())
            return;
        channelMap[channelName].operatorNickSet.insert(targetNick);
    }
    else if (sign == false)
    {
        if (channelMap[channelName].operatorNickSet.find(targetNick) == channelMap[channelName].operatorNickSet.end())
            return;
        channelMap[channelName].operatorNickSet.erase(targetNick);
    }
    char signChar = '+';
    if (!sign)
        signChar = '-';
    std::string msg = ":" + fdNicknameMap[clientFd] + "!" + memberMap[fdNicknameMap[clientFd]].username + "@" + memberMap[fdNicknameMap[clientFd]].hostname + " MODE #" + channelName + " " + signChar + "o :" + targetNick + "\r\n";
    std::set<std::string>::iterator it = channelMap[channelName].memberNickSet.begin();
    for (; it != channelMap[channelName].memberNickSet.end(); ++it)
    {
        int channelMemberFd = memberMap[*it].fd;
        serverToClientMsg[channelMemberFd] += msg;
        setWriteEvent(channelMemberFd);
    }
}