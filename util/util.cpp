#include "util.hpp"

std::vector<std::string> split(const std::string &str, const std::string &del)
{
    std::vector<std::string> tokens;
    size_t start = 0;
    size_t end;

    while ((end = str.find(del, start)) != std::string::npos)
    {
        std::string token = str.substr(start, end - start);
        if (!token.empty())
        {
            tokens.push_back(token);
        }
        start = end + del.length();
    }

    std::string lastToken = str.substr(start);
    if (!lastToken.empty())
    {
        tokens.push_back(lastToken);
    }

    return tokens;
}

void handleError(std::string errMsg)
{
    std::cout << "오류 발생: " << strerror(errno) << std::endl;
    std::cout << errMsg << std::endl;
    std::exit(0);
}

void makeNonBlock(int fd)
{
    if (fcntl(fd, F_SETFL, O_NONBLOCK) == -1)
        handleError("fcntl Error");
}