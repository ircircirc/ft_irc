#include "../ConfigManager.hpp"

// 키 실패
// :irc.local 475 b #hi :Cannot join channel (incorrect channel key)

// JOIN #bb pass
// 성공

// 초대전용이면 키 모드와 관계없이 join할 수 있음

// 키 보내지 않을경우
// MODE #aa +k
// :irc.local 696 a #aa k * :You must specify a parameter for the key mode. Syntax: <key>.

// 키 변경하면 모두에게 메시지 보낸다.
// MODE #aa +k password
// :a!root@127.0.0.1 MODE #aa +k :password
// :a!root@127.0.0.1 MODE #aa +k :password

// 키 없앨때도 키 필요함
// MODE #aa -k passwor
// :irc.local 467 a #aa :Channel key already set

void ConfigManager::processModeKey(int clientFd, bool sign, std::string &channelName, std::vector<std::string> &commandAndParams)
{
    if (commandAndParams.size() < 4)
    {
        serverToClientMsg[clientFd] += ":irc.local 696 " + fdNicknameMap[clientFd] + " #" + channelName + " k * :You must specify a parameter for the key mode. Syntax: <key>\r\n";
        setWriteEvent(clientFd);
        return;
    }
    if (channelMap[channelName].useKeyOnly == sign)
        return;
    char signChar = '+';
    if (!sign)
        signChar = '-';
    std::string key = commandAndParams[3];
    if (sign == false && channelMap[channelName].key != key)
    {
        serverToClientMsg[clientFd] += ":irc.local 467 " + fdNicknameMap[clientFd] + " #" + channelName + " :Channel key already set\r\n";
        setWriteEvent(clientFd);
        return;
    }
    else if (sign == true)
        channelMap[channelName].key = key;
    channelMap[channelName].useKeyOnly = sign;

    std::string msg = ":" + fdNicknameMap[clientFd] + "!" + memberMap[fdNicknameMap[clientFd]].username + "@127.0.0.1 MODE #" + channelName + " :" + signChar + "k :" + key + "\r\n";
    std::set<std::string>::iterator it = channelMap[channelName].memberNickSet.begin();
    for (; it != channelMap[channelName].memberNickSet.end(); ++it)
    {
        int channelMemberFd = memberMap[*it].fd;
        serverToClientMsg[channelMemberFd] += msg;
        setWriteEvent(channelMemberFd);
    }
}