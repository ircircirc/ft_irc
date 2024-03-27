#ifndef IRC_MEMBER_HPP
# define IRC_MEMBER_HPP


#include "UnregisterMember.hpp"
#include <string>
#include <set>

class IrcMember
{
public:
    IrcMember();
    IrcMember(UnregisterMember &unregisterMember, int fd);
    std::string nickname;
    std::string username;
    std::string hostname;
    std::string servername;
    std::string realname;
    int fd;
    bool pendingCloseSocket;
    std::set<std::string> memberChannelSet;
    std::set<std::string> invitedChannelSet;
};

#endif