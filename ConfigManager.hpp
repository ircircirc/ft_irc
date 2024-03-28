#ifndef CONFIG_MANAGER_HPP
#define CONFIG_MANAGER_HPP

#include "UnregisterMember.hpp"
#include "IrcMember.hpp"
#include "Channel.hpp"
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
#include <sstream>

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
    void sendDM(std::vector<std::string> &commandAndParams, int clientFd);
    void sendChannel(std::vector<std::string> &commandAndParams, int clientFd);
    void quitMember(int clientFd);
    void PingPongInteraction(const std::vector<std::string> &splitMessage, int clientFd);
    void joinChannel(std::vector<std::string> &commandAndParams, int clientFd);
    void join(int clientFd, const std::string &channelName, std::vector<std::string> &commandAndParams);
    void kickMember(std::vector<std::string> &commandAndParams, int clientFd);
    void processMode(std::vector<std::string> &commandAndParams, int clientFd);
    void processModeInvite(int clientFd, bool sign, std::string &channelName);
    void processModeKey(int clientFd, bool sign, std::string &channelName, std::vector<std::string> &commandAndParams);
    void processModeOperator(int clientFd, bool sign, std::string &channelName, std::vector<std::string> &commandAndParams);
    void processModeLimit(int clientFd, bool sign, std::string &channelName, std::vector<std::string> &commandAndParams);
    void processModeTopic(int clientFd, bool sign, std::string &channelName);
    void partChannel(std::vector<std::string> &commandAdnParams, int clientFd);
    void part(int clientFd, const std::string &channelName);
    void inviteMember(std::vector<std::string> &commandAndParams, int clientFd);
    void processTopic(std::vector<std::string> &commandAndParams, int clientFd);
    void topic(int clientFd, const std::vector<std::string> &commandAndParams, const std::string &channelName);


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
    std::map<std::string, Channel> channelMap;
};

#endif
