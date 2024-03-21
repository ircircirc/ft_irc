#include "ConfigManager.hpp"

ConfigManager::ConfigManager(int argc, char **argv)
{
    setConifg(argc, argv);
    listenSocket();
    startKqueue();
}

void ConfigManager::routine()
{
    struct kevent event_list[8];
    int new_events = kevent(kqueueFd, &change_list[0], change_list.size(), event_list, 8, NULL);
    if (new_events == -1)
        handleError("kevent");
    change_list.clear();
    for (int i = 0; i < new_events; i++)
    {
        struct kevent *curr_event = &event_list[i];
        if (curr_event->flags & EV_ERROR)
        {
            std::cout << curr_event->ident << std::endl;
            handleError("EV_ERROR");
        }
        if (curr_event->filter == EVFILT_READ)
            handleReadEvent(curr_event);
        else if (curr_event->filter == EVFILT_WRITE)
            handleWriteEvent(curr_event);
    }
}

void ConfigManager::checkRegister(int clientFd)
{
    UnregisterMember &unregisterMember = unregisterMemberMap[clientFd];
    if (unregisterMember.nickname.size() != 0 && unregisterMember.username.size() != 0)
    {
        if (password.compare(unregisterMember.password) != 0)
        {
            clearMember(clientFd);
            return;
        }
        welcomeMember(clientFd);
    }
}

void ConfigManager::welcomeMember(int clientFd)
{
    UnregisterMember &unregisterMember = unregisterMemberMap[clientFd];
    memberMap[unregisterMember.nickname] = IrcMember(unregisterMember, clientFd);
    fdNicknameMap[clientFd] = unregisterMember.nickname;
    IrcMember &member = memberMap[unregisterMember.nickname];
    unregisterMemberMap.erase(clientFd);
    // hostname을 서버가 클라이언트에게 받았으나, 실제로 사용할때는 클라이언트의 IP주소를 조회해 사용한다고함.
    // 추후 수정 고려
    std::string ret1 = ":irc.local 001 root :Welcome to the Internet Relay Network" + member.nickname + "!" + member.username + "@" + member.hostname + "\r\n";
    // std::string ret2 = ":irc.local 002 root :Your host is ft_irc, running version 0.0.1\r\n";
    // std::string ret3 = ":irc.local 003 root :This server was created a long ago\r\n";
    // std::string ret4 = ":irc.local 004 root ft_irc\r\n";

    serverToClientMsg[clientFd] += ret1;
    EV_SET(&tempEvent, clientFd, EVFILT_WRITE, EV_ADD, 0, 0, NULL);
    change_list.push_back(tempEvent);
}

void ConfigManager::clearMember(int clientFd)
{
    close(clientFd);
    clientsToServerMsg.erase(clientFd);
    serverToClientMsg.erase(clientFd);
    if (fdNicknameMap.find(clientFd) != fdNicknameMap.end())
    {
        std::string nickname = fdNicknameMap[clientFd];
        memberMap.erase(nickname);
        fdNicknameMap.erase(clientFd);
    }
    else
    {
        unregisterMemberMap.erase(clientFd);
    }
    std::cout << "client disconnect " << clientFd << "\n";
    return;
}

void ConfigManager::processMessage(std::string &message, int clientFd)
{
    std::vector<std::string> spiltMessage = split(message, " ");
    if (spiltMessage.size() == 0)
        return;

    for (int i = 0; i < spiltMessage.size(); i++)
        std::cout << i << " : ||" << spiltMessage[i] << "||" << std::endl;

    // case Insensitive하게 변경해야함 -> 지금은 대문자 or 소문자임
    std::string command = spiltMessage[0];
    if (command.compare("PASS") == 0 || command.compare("pass") == 0)
        authenticateUser(spiltMessage, clientFd);
    else if (command.compare("NICK") == 0 || command.compare("nick") == 0)
        registerNick(spiltMessage, clientFd);
    else if (command.compare("USER") == 0 || command.compare("user") == 0)
        registerUser(spiltMessage, clientFd);
}

void ConfigManager::processMessageBuffer(std::string &clientMsg, int clientFd)
{
    while (1)
    {
        size_t idx = clientMsg.find("\r\n");
        size_t secondIdx = clientMsg.find("\n");
        std::cout << clientMsg;
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
    EV_SET(&tempEvent, listenFd, EVFILT_READ, EV_ADD, 0, 0, NULL);
    change_list.push_back(tempEvent);
}

void ConfigManager::handleReadEvent(struct kevent *curr_event)
{
    if (curr_event->ident == listenFd)
    {
        int clientSocket = accept(listenFd, NULL, NULL);
        std::cout << "client " << clientSocket << "is connected\n";
        makeNonBlock(clientSocket);
        EV_SET(&tempEvent, clientSocket, EVFILT_READ, EV_ADD, 0, 0, NULL);
        change_list.push_back(tempEvent);
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
            std::cout << "client buf : " << clientsToServerMsg[clientSocket];
            processMessageBuffer(clientsToServerMsg[clientSocket], clientSocket);
        }
        if (curr_event->flags & EV_EOF)
            clearMember(clientSocket);
        else
        {
            EV_SET(&tempEvent, clientSocket, EVFILT_WRITE, EV_ADD, 0, 0, NULL);
            change_list.push_back(tempEvent);
        }
    }
}

void ConfigManager::handleWriteEvent(struct kevent *curr_event)
{
    if (serverToClientMsg.find(curr_event->ident) != serverToClientMsg.end())
    {
        int clientSocket = curr_event->ident;
        // 쓸수있는 데이터가 몇 바이트인지알 수 있다. 추후 처리 고려(한번에 데이터 다 못보냈을때)
        write(clientSocket, serverToClientMsg[clientSocket].data(), serverToClientMsg[clientSocket].size());
        serverToClientMsg[curr_event->ident].clear();
        EV_SET(&tempEvent, clientSocket, EVFILT_WRITE, EV_DELETE, 0, 0, NULL);
        change_list.push_back(tempEvent);
    }
}