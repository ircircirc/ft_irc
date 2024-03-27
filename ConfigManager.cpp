#include "ConfigManager.hpp"

ConfigManager::ConfigManager(int argc, char **argv)
{
    setConifg(argc, argv);
    listenSocket();
    startKqueue();
}

void ConfigManager::routine()
{
    // 현재 이벤트 출력 하는 함수
    printEventTypes(change_list);
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
            int error_code = curr_event->data;
            std::cout << strerror(error_code) << std::endl;
            handleError("EV_ERROR");
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
        // 참여하고 있는 채널에서 나가기
        std::set<std::string> &channeNameSet = memberMap[nickname].memberChannelSet;
        std::set<std::string>::iterator it;
        for (it = channeNameSet.begin(); it != channeNameSet.end(); it++)
            channelMap[*it].memberNickSet.erase(nickname);
        //멤버 삭제
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
    setReadEvent(listenFd);
}

void ConfigManager::handleReadEvent(struct kevent *curr_event)
{
    if (curr_event->ident == listenFd)
    {
        int clientSocket = accept(listenFd, NULL, NULL);
        std::cout << "client " << clientSocket << "is connected\n";
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
            std::cout << "client buf : " << clientsToServerMsg[clientSocket];
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

        // 쓸수있는 데이터가 몇 바이트인지알 수 있다. 추후 처리 고려(한번에 데이터 다 못보냈을때)
        write(clientSocket, serverToClientMsg[clientSocket].data(), serverToClientMsg[clientSocket].size());
        serverToClientMsg[curr_event->ident].clear();

        // 메시지 보내고 연결 끊어야 할 경우 체크
        if (unregisterMemberMap.find(clientSocket) != unregisterMemberMap.end())
        {
            if (unregisterMemberMap[clientSocket].pendingCloseSocket)
                clearMember(clientSocket);
            return;
        }
        if (fdNicknameMap.find(clientSocket) != fdNicknameMap.end())
        {
            if (memberMap[fdNicknameMap[clientSocket]].pendingCloseSocket)
                clearMember(clientSocket);
            return;
        }
        EV_SET(&tempEvent, clientSocket, EVFILT_WRITE, EV_DELETE, 0, 0, NULL);
        change_list.push_back(tempEvent);
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