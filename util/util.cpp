#include "util.hpp"

std::vector<std::string> split(const std::string &str, const std::string &del)
{
    std::vector<std::string> tokens;
    size_t start = 0;
    size_t end;

    while ((end = str.find(del, start)) != std::string::npos)
    {
        std::string token = str.substr(start, end - start);
        tokens.push_back(token);
        start = end + del.length();
    }
    std::string lastToken = str.substr(start);
    tokens.push_back(lastToken);
    return tokens;
}

void handleError(std::string errMsg)
{
    std::cerr << "오류 발생: " << strerror(errno) << std::endl;
    if (!errMsg.empty()) // 빈문자열 "" 처리
        std::cerr << errMsg << std::endl;
    else
        std::cerr << "An unspecified error occurred." << std::endl;
    std::exit(EXIT_FAILURE);
}

void makeNonBlock(int fd)
{
    if (fcntl(fd, F_SETFL, O_NONBLOCK) == -1)
        handleError("fcntl Error");
}

void printEventTypes(const std::vector<struct kevent> &change_list)
{
    for (int i = 0; i < change_list.size(); i++)
    {
        struct kevent kev = change_list[i];
        std::cout << "Event for fd " << kev.ident << ": ";
        switch (kev.filter)
        {
        case EVFILT_READ:
            std::cout << "READ";
            break;
        case EVFILT_WRITE:
            std::cout << "WRITE";
            break;
        default:
            std::cout << "UNKNOWN";
            break;
        }
        std::cout << std::endl;
    }
}