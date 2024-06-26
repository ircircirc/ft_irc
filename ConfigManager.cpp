#include "ConfigManager.hpp"

static void signalHandler(int signum)
{
    std::cout << "Interrupt by signal" << signum << "received.\n";
    std::exit(signum);
}

ConfigManager::ConfigManager(int argc, char **argv)
{
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
    signal(SIGQUIT, signalHandler);
    listenFd = -1;
    kqueueFd = -1;
    setConifg(argc, argv);
    listenSocket();
    startKqueue();
}

void ConfigManager::routine()
{
    struct kevent event_list[8];
    for (size_t i = 0; i < change_list.size();)
    {
        if (change_list[i].filter == EVFILT_WRITE && serverToClientMsg.find(change_list[i].ident) == serverToClientMsg.end())
            change_list.erase(change_list.begin() + i);
        else
            i++;
    }
    int new_events = kevent(kqueueFd, &change_list[0], change_list.size(), event_list, 8, NULL);

    change_list.clear();
    for (int i = 0; i < new_events; i++)
    {
        struct kevent *curr_event = &event_list[i];
        if (curr_event->flags & EV_ERROR)
        {
            int error_code = curr_event->data;
            std::cout << strerror(error_code) << std::endl;
            continue;
        }
        if (curr_event->filter == EVFILT_READ)
            handleReadEvent(curr_event);
        else if (curr_event->filter == EVFILT_WRITE)
            handleWriteEvent(curr_event);
    }
}

void ConfigManager::clearMember(int clientFd)
{
    close(clientFd);
    clientsToServerMsg.erase(clientFd);
    serverToClientMsg.erase(clientFd);
    if (fdNicknameMap.find(clientFd) != fdNicknameMap.end())
    {
        std::string nickname = fdNicknameMap[clientFd];
        std::set<std::string> &channeNameSet = memberMap[nickname].memberChannelSet;
        std::set<std::string>::iterator it;
        for (it = channeNameSet.begin(); it != channeNameSet.end(); it++)
        {
            channelMap[*it].memberNickSet.erase(nickname);
            channelMap[*it].operatorNickSet.erase(nickname);
        }
        std::set<std::string> &invitedChannelSet = memberMap[nickname].invitedChannelSet;
        for (it = invitedChannelSet.begin(); it != invitedChannelSet.end(); it++)
            channelMap[*it].invitedMemberSet.erase(nickname);
        memberMap.erase(nickname);
        fdNicknameMap.erase(clientFd);
    }
    else
    {
        unregisterMemberMap.erase(clientFd);
    }
    return;
}

void ConfigManager::processMessage(std::string &message, int clientFd)
{
    std::vector<std::string> spiltMessage = split(message, " ");
    if (spiltMessage.size() == 0)
        return;

    std::string command = spiltMessage[0];
    if (command.compare("PASS") == 0 || command.compare("pass") == 0)
        authenticateUser(spiltMessage, clientFd);
    else if (command.compare("NICK") == 0 || command.compare("nick") == 0)
        registerNick(spiltMessage, clientFd);
    else if (command.compare("USER") == 0 || command.compare("user") == 0)
        registerUser(spiltMessage, clientFd);
    else if (command.compare("PING") == 0)
        PingPongInteraction(spiltMessage, clientFd);
    else if (command.compare("PRIVMSG") == 0 || command.compare("privmsg") == 0)
        sendPrivateMsg(spiltMessage, clientFd);
    else if (command.compare("QUIT") == 0 || command.compare("quit") == 0)
        quitMember(clientFd);
    else if (command.compare("JOIN") == 0 || command.compare("join") == 0)
        joinChannel(spiltMessage, clientFd);
    else if (command.compare("PART") == 0 || command.compare("part") == 0)
        partChannel(spiltMessage, clientFd);
    else if (command.compare("KICK") == 0 || command.compare("kick") == 0)
        kickMember(spiltMessage, clientFd);
    else if (command.compare("MODE") == 0 || command.compare("mode") == 0)
        processMode(spiltMessage, clientFd);
    else if (command.compare("INVITE") == 0 || command.compare("invite") == 0)
        inviteMember(spiltMessage, clientFd);
    else if (command.compare("TOPIC") == 0 || command.compare("topic") == 0)
        processTopic(spiltMessage, clientFd);
}

void ConfigManager::processMessageBuffer(std::string &clientMsg, int clientFd)
{

    while (1)
    {
        size_t idx = clientMsg.find("\r\n");
        size_t secondIdx = clientMsg.find("\n");
        if (idx != std::string::npos)
        {
            std::string message = clientMsg.substr(0, idx);
            clientMsg = clientMsg.substr(idx + 2);
            processMessage(message, clientFd);
        }
        else if (secondIdx != std::string::npos)
        {
            std::string message = clientMsg.substr(0, secondIdx);
            clientMsg = clientMsg.substr(secondIdx + 1);
            processMessage(message, clientFd);
        }
        else
            break;
    }
}

void ConfigManager::startKqueue()
{
    kqueueFd = kqueue();
    if (kqueueFd == -1)
        handleError("kqueue");
    setReadEvent(listenFd);
}

void ConfigManager::handleReadEvent(struct kevent *curr_event)
{
    if ((int)curr_event->ident == listenFd)
    {
        int clientSocket = accept(listenFd, NULL, NULL);
        makeNonBlock(clientSocket);
        setReadEvent(clientSocket);
        clientsToServerMsg[clientSocket] = "";
        serverToClientMsg[clientSocket] = "";
        unregisterMemberMap[clientSocket] = UnregisterMember();
    }
    else if (clientsToServerMsg.find(curr_event->ident) != clientsToServerMsg.end())
    {
        int clientSocket = curr_event->ident;
        char buf[1000];
        int n = read(clientSocket, buf, sizeof(buf));
        if (n != 0)
        {
            clientsToServerMsg[clientSocket] += std::string(buf, n);
            processMessageBuffer(clientsToServerMsg[clientSocket], clientSocket);
        }
        if (curr_event->flags & EV_EOF)
        {
            clearMember(clientSocket);
        }
    }
}

void ConfigManager::handleWriteEvent(struct kevent *curr_event)
{
    if (serverToClientMsg.find(curr_event->ident) != serverToClientMsg.end())
    {
        int clientSocket = curr_event->ident;
        int n = write(clientSocket, serverToClientMsg[clientSocket].data(), serverToClientMsg[clientSocket].size());
        if (n < (int)serverToClientMsg[clientSocket].size())
            serverToClientMsg[clientSocket] = serverToClientMsg[clientSocket].substr(n);
        else
            serverToClientMsg[clientSocket].clear();

        if (serverToClientMsg[curr_event->ident].size() == 0)
        {
            if (unregisterMemberMap.find(clientSocket) != unregisterMemberMap.end())
            {
                if (unregisterMemberMap[clientSocket].pendingCloseSocket)
                {
                    clearMember(clientSocket);
                    return;
                }
            }
            if (fdNicknameMap.find(clientSocket) != fdNicknameMap.end())
            {
                if (memberMap[fdNicknameMap[clientSocket]].pendingCloseSocket)
                {
                    clearMember(clientSocket);
                    return;
                }
            }
            EV_SET(&tempEvent, clientSocket, EVFILT_WRITE, EV_DELETE, 0, 0, NULL);
            change_list.push_back(tempEvent);
        }
    }
}

void ConfigManager::setReadEvent(int fd)
{
    EV_SET(&tempEvent, fd, EVFILT_READ, EV_ADD, 0, 0, NULL);
    change_list.push_back(tempEvent);
}

void ConfigManager::setWriteEvent(int fd)
{
    EV_SET(&tempEvent, fd, EVFILT_WRITE, EV_ADD, 0, 0, NULL);
    change_list.push_back(tempEvent);
}

void ConfigManager::cleanup()
{
    if (listenFd != -1)
        close(listenFd);
    if (kqueueFd != -1)
        close(listenFd);
    for (std::map<int, std::string>::iterator it = clientsToServerMsg.begin(); it != clientsToServerMsg.end(); ++it)
    {
        close(it->first);
    }
}