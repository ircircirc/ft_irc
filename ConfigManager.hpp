#ifndef CONFIG_MANAGER_HPP
#define CONFIG_MANAGER_HPP

#include "UnregisterMember.hpp"
#include "IrcMember.hpp"
#include "util/util.hpp"
#include <vector>
#include <map>
#include <string>
#include <sys/event.h>
#include <iostream>
#include <fstream>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstring>
#include <cstdlib>
#include <cctype>

class ConfigManager
{
public:
    ConfigManager(int argc, char **argv);
    void routine();

private:
    void authenticateUser(std::vector<std::string> &commandAndParams, int clientFd);
    void registerNick(std::vector<std::string> &commandAndParams, int clientFd);
    void registerUser(std::vector<std::string> &commandAndParams, int clientFd);
    void sendPrivateMsg(std::vector<std::string> &commandAndParams, int clientFd);
    void quitMember(int clientFd);
    void checkRegister(int clientFd);
    void welcomeMember(int clientFd);
    void clearMember(int clientFd);
    void processMessage(std::string &message, int clientFd);
    void processMessageBuffer(std::string &clientMsg, int clientFd);
    void setConifg(int argc, char **argv);
    void listenSocket();
    void startKqueue();
    void handleReadEvent(struct kevent *curr_event);
    void handleWriteEvent(struct kevent *curr_event);
    void setReadEvent(int fd);
    void setWriteEvent(int fd);
    void PingPongInteraction(const std::vector<std::string>& splitMessage, int clientFd);

    int port;
    int listenFd;
    int kqueueFd;
    std::string password;
    struct kevent tempEvent;
    std::vector<struct kevent> change_list;
    std::map<int, std::string> clientsToServerMsg;
    std::map<int, std::string> serverToClientMsg;
    std::map<int, UnregisterMember> unregisterMemberMap;
    std::map<std::string, IrcMember> memberMap;
    std::map<int, std::string> fdNicknameMap;
};

#endif