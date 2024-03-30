#include "../ConfigManager.hpp"

bool isValidNum(std::string arg)
{
    for (int i = 0; i < (int)arg.size(); i++)
    {
        if (!std::isdigit(arg[i]))
            return false;
    }
    if (arg.size() > 1 && arg[0] == '0')
        return false;
    return true;
}

bool isValidInput(int argc, char **argv)
{
    return (argc == 3 && isValidNum(argv[1]));
}

void ConfigManager::setConifg(int argc, char **argv)
{
    if (!isValidInput(argc, argv))
        handleError("invalid input");
    port = std::atoi(argv[1]);
    password = std::string(argv[2]);
    std::cout << "포트 : " << port << " 비밀번호 : " << password << std::endl;
}
void ConfigManager::listenSocket()
{
    listenFd = socket(PF_INET, SOCK_STREAM, 0);
    if (listenFd == -1)
        handleError("socket Error");

    //////////// reuseaddr 처리 -> 나중에는 지우는게 나을듯?///////////////
    int reuse = 1;
    if (setsockopt(listenFd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(int)) < 0)
        handleError("setsockopt(SO_REUSEADDR) failed");
    ////////////////////////////////////////////////////////////
    makeNonBlock(listenFd);
    struct sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(listenFd, (struct sockaddr *)&addr, sizeof(addr)) == -1)
        handleError("bind Error");
    if (listen(listenFd, 100) == -1)
        handleError("listen Error");
}