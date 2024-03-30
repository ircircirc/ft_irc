#include "UnregisterMember.hpp"
#include <iostream>

UnregisterMember::UnregisterMember()
{
    this->pendingCloseSocket = false;
    std::cout << this->pendingCloseSocket << std::endl;
}