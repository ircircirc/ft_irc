
#include "Channel.hpp"

Channel::Channel(std::string name)
{
    this->name = name;
    this->inviteOnly = false;
    this->useKeyOnly = false;
    this->isLimit = false;
    this->topicOpOnly = false;
}
