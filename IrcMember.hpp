#ifndef IRC_MEMBER_HPP
# define IRC_MEMBER_HPP


#include <string>
#include "UnregisterMember.hpp"

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
};

#endif