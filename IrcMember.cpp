#include "IrcMember.hpp"

IrcMember::IrcMember() {
    this->pendingCloseSocket = false;
}

IrcMember::IrcMember(UnregisterMember &unregisterMember, int fd)
{
    nickname = unregisterMember.nickname;
    username = unregisterMember.username;
    hostname = unregisterMember.hostname;
    servername = unregisterMember.servername;
    realname = unregisterMember.realname;
    this->fd = fd;
    this->pendingCloseSocket = false;
}