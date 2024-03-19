#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/event.h>
#include <sys/time.h>
#include <vector>
#include <map>
#include <string>
#include <cstdlib>

void handleError(std::string errMsg)
{
    fprintf(stderr, "오류 발생: %s\n", strerror(errno));
    std::cout << errMsg << std::endl;
    std::exit(0);
}

void makeNonBlock(int fd)
{
    if (fcntl(fd, F_SETFL, O_NONBLOCK) == -1)
        handleError("fcntl Error");
}

bool isValidNum(std::string arg)
{
    for (int i = 0; i < arg.size(); i++)
    {
        if (!std::isdigit(arg[i]))
            return false;
    }
    // 포트에 0을 넣으면 서버가 작동하나 클라이언트 연결이 안된다.
    // 서치해보니 포트가 0이면 임의의 가용한 포트를 사용하는것으로 보인다.
    if (arg.compare("0") == 0)
        return false;
    return true;
}

bool isValidInput(int argc, char **argv)
{
    return (argc != 3 || !isValidNum(argv[1]));
}

int main(int argc, char **argv)
{
    if (isValidInput(argc, argv))
        handleError("invalid port input");
    int port = std::atoi(argv[1]);
    std::string password = std::string(argv[2]);
    std::cout << "포트 : " << port << " 비밀번호 : " << password << std::endl;

    // 소켓(엔드포인트) 할당 받음
    int socketFd = socket(PF_INET, SOCK_STREAM, 0); // IPV4 인터텟 프로토콜(프로토콜 패밀리), TCP 스트림
    if (socketFd == -1)
        handleError("socket Error");

    // reuseaddr 처리 -> 나중에는 지우는게 나을듯?
    int reuse = 1;
    if (setsockopt(socketFd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(int)) < 0)
        handleError("setsockopt(SO_REUSEADDR) failed");

    // non-block io를 위해 non-blocking 처리
    makeNonBlock(socketFd);
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;                                        // IPV4 인터텟 프로토콜(주소 패밀리)
    addr.sin_port = htons(port);                                      // listen할 포트 지정
    addr.sin_addr.s_addr = htonl(INADDR_ANY);                         // address에 관계없이 동작함
    if (bind(socketFd, (struct sockaddr *)&addr, sizeof(addr)) == -1) // 해당 주소와 포트에 바인드
        handleError("bind Error");
    if (listen(socketFd, 100) == -1) // listen함
        handleError("listen Error");

    // kqueue를 위한 FD할당받음
    int kqueueFd = kqueue();
    if (kqueueFd == -1)
        handleError("kqueue");

    // 등록하고 싶은 FD를 change_list에 담으면 됨!
    std::vector<struct kevent> change_list;

    // listen하고있는 소켓 FD를 변경을 감지할 change_list에 등록
    struct kevent tempEvent;
    EV_SET(&tempEvent, socketFd, EVFILT_READ, EV_ADD, 0, 0, NULL);
    change_list.push_back(tempEvent);

    std::map<int, std::string> clients;
    while (1)
    {
        struct kevent event_list[8];
        // change_list의 이벤트를 등록했고, event_list에 현재 일어난 이벤트를 받음
        int new_events = kevent(kqueueFd, &change_list[0], change_list.size(), event_list, 8, NULL);
        if (new_events == -1)
            handleError("kevent");
        // 이벤트를 등록했으므로 change_list를 초기화한다.
        change_list.clear();
        for (int i = 0; i < new_events; i++) // 이벤트가 일어난 횟수만큼 순회한다.
        {
            struct kevent *curr_event = &event_list[i];
            // 리소스 부족같은 경우 체크
            if (curr_event->flags & EV_ERROR)
                handleError("EV_ERROR");

            if (curr_event->filter == EVFILT_READ) // 읽을 수 있는 데이터가 있을때 이벤트 발생
            {
                // listen하고 있는 소켓에 이벤트 발생
                if (curr_event->ident == socketFd)
                {
                    // 연결을 수락
                    int clientSocket = accept(socketFd, NULL, NULL);
                    makeNonBlock(clientSocket);
                    // 클라이언트 소켓을 change_list에 등록해 클라이언트의 읽기이벤트를 감지함
                    EV_SET(&tempEvent, clientSocket, EVFILT_READ, EV_ADD, 0, 0, NULL);
                    clients[clientSocket] = ""; // 내부 자료구조에 클라이언트 소켓을 추가
                    change_list.push_back(tempEvent);
                }
                else if (clients.find(curr_event->ident) != clients.end()) // 클라이언트 소켓이라면
                {
                    // 읽을수 있는 데이터가 몇바이트인지 알 수 있다.
                    // 이 정보는 안써도 될것같긴함
                    // std::cout << "data byte : " << curr_event->data << std::endl;
                    int clientSocket = curr_event->ident;
                    char buf[1000];
                    int n = read(clientSocket, buf, sizeof(buf));
                    if (n != 0)
                    {
                        clients[clientSocket] = std::string(buf, n);
                        std::cout << "client say : " << clients[clientSocket];
                    }
                    if (curr_event->flags & EV_EOF) // 클라이언트가 소켓 닫은 경우
                    {
                        close(clientSocket);         // 클라이언트 소켓 close
                        clients.erase(clientSocket); // 클라이언트 자료구조 삭제
                        std::cout << "client disconnect " << clientSocket << "\n";
                    }
                    else
                    {
                        EV_SET(&tempEvent, clientSocket, EVFILT_WRITE, EV_ADD, 0, 0, NULL);
                        change_list.push_back(tempEvent);
                    }
                }
            }
            else if (curr_event->filter == EVFILT_WRITE) // 쓸 수 있는 상태일때 이벤트 발생
            {
                if (clients.find(curr_event->ident) != clients.end()) // 클라이언트 소켓이라면 -> 사실이거밖에없음
                {
                    int clientSocket = curr_event->ident;
                    // 쓸수있는 데이터가 몇 바이트인지알 수 있다.
                    // 만약 내가 쓰고싶은 공간의 크기가 쓸 수있는 크기보다 작다면?
                    // 여러번 써야할듯
                    // std::cout << "쓸수있는 공간의 크기 : " << curr_event->data << std::endl;
                    write(clientSocket, clients[clientSocket].data(), clients[clientSocket].size()); // 클라이언트 소켓에 write
                    clients[curr_event->ident].clear();

                    EV_SET(&tempEvent, clientSocket, EVFILT_WRITE, EV_DELETE, 0, 0, NULL);
                    change_list.push_back(tempEvent);
                }
            }
        }
    }

    // clean
    // close(kqueueFd);
    // close(socketFd);
    // close client socket
    //~~ close client vector~~
    return 0;
}