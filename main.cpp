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
#include <sys/stat.h>

void handleError(std::string errMsg)
{
    fprintf(stderr, "오류 발생: %s\n", strerror(errno));
    std::cout << errMsg << std::endl;
    std::exit(1);
}

void makeNonBlock(int fd)
{
    if (fcntl(fd, F_SETFL, O_NONBLOCK) == -1)
        handleError("fcntl Error");
}

int main()
{
    // 소켓(엔드포인트) 할당 받음
    int socketFd = socket(PF_INET, SOCK_STREAM, 0); //IPV4 인터텟 프로토콜(프로토콜 패밀리), TCP 스트림
    if (socketFd == -1)
        handleError("socket Error");

    // reuseaddr 처리
    int reuse = 1;
    if (setsockopt(socketFd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(int)) < 0)
        handleError("setsockopt(SO_REUSEADDR) failed");

    // non-block io를 위해 non-blocking 처리
    makeNonBlock(socketFd);
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;                                        // IPV4 인터텟 프로토콜(주소 패밀리)
    addr.sin_port = htons(12345);                                     // listen할 포트 지정
    addr.sin_addr.s_addr = htonl(INADDR_ANY);                         // address에 관계없이 동작함
    if (bind(socketFd, (struct sockaddr *)&addr, sizeof(addr)) == -1) // 해당 주소와 포트에 바인드
        handleError("bind Error");
    if (listen(socketFd, 100) == -1) // listen함
        handleError("listen Error");

    // kqueue를 위한 FD할당받음
    int kqueueFd = kqueue();

    // 등록하고 싶은 FD를 change_list에 담으면 됨!
    std::vector<struct kevent> change_list;

    // listen하고있는 소켓 FD를 변경을 감지할 change_list에 등록
    struct kevent temp_event;
    EV_SET(&temp_event, socketFd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
    change_list.push_back(temp_event);

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
            if (curr_event->filter == EVFILT_READ) // 읽을 수 있는 데이터가 있을때 이벤트 발생
            {
                // listen하고 있는 소켓에 이벤트 발생
                if (curr_event->ident == socketFd)
                {
                    // 연결을 수락
                    int clientSocket = accept(socketFd, NULL, NULL);
                    makeNonBlock(clientSocket);
                    // 클라이언트 소켓을 change_list에 등록해 클라이언트의 읽기이벤트를 감지함
                    EV_SET(&temp_event, clientSocket, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
                    clients[clientSocket] = ""; //내부 자료구조에 클라이언트 소켓을 추가
                    change_list.push_back(temp_event);
                }
                else if (clients.find(curr_event->ident) != clients.end()) // 클라이언트 소켓이라면
                {
                    int clientSocket = curr_event->ident;
                    char buf[1000];
                    int n = read(clientSocket, buf, sizeof(buf));
                    if (n == 0) // EOF왔을때 인듯? 클라이언트 종료처리
                    {
                        close(clientSocket); //클라이언트 소켓 close
                        clients.erase(clientSocket); //클라이언트 소켓 
                        std::cout << "client disconnect " << clientSocket << "\n";
                        break;
                    }
                    clients[clientSocket] = std::string(buf, n);
                    std::cout << "client say : " << clients[clientSocket];
                    EV_SET(&temp_event, clientSocket, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, NULL);
                    change_list.push_back(temp_event);
                }
            }
            else if (curr_event->filter == EVFILT_WRITE) // 쓸 수 있는 상태일때 이벤트 발생
            {
                if (clients.find(curr_event->ident) != clients.end()) // 클라이언트 소켓이라면 -> 사실이거밖에없음
                {
                    int clientSocket = curr_event->ident;
                    write(clientSocket, clients[clientSocket].data(), clients[clientSocket].size()); //클라이언트 소켓에 write
                    clients[curr_event->ident].clear();
                }
            }
            else if (curr_event->flags == EV_ERROR) // 에러가 났을때 발생하는 것 같긴한데, 정확한거는 공식문서 다시 봐야함
            {
            }
        }
    }
    return 0;
}