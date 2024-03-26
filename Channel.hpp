#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <string>
#include <set>

class Channel
{
public:
    Channel(){};
    Channel(std::string name);
    std::string name;
    std::set<std::string> memberNickSet;
};

#endif