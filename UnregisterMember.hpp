#ifndef UNREGISTER_MEMBER_HPP
# define UNREGISTER_MEMBER_HPP

#include <string>

class UnregisterMember
{
public:
    UnregisterMember();
    std::string password;
    std::string nickname;
    std::string username;
    std::string hostname;
    std::string servername;
    std::string realname;
    bool pendingCloseSocket;
};

#endif