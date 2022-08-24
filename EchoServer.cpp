#include<iostream>
#include<netdb.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<arpa/inet.h>
#include<cstdlib>
#include<signal.h>
#include<string.h>
#include<vector>
#include<thread>


//terminate moi thread? dong moi socket
int sockfd;

void sig_handler(int signo)
{
    if(signo == SIGINT)
    {
        // stop da server
        std::cout << "Server stopping...\n";
        close(sockfd);
        exit(1);
    }
}

void echo(sockaddr_in client_addr, int connfd)
{
    char buffer[1024] = { 0 };
    char host[NI_MAXHOST];
    char service[NI_MAXSERV];
    memset(host, 0, NI_MAXHOST);
    memset(service, 0, NI_MAXSERV);
    if (getnameinfo((sockaddr*)&client_addr, sizeof(client_addr), host, NI_MAXHOST, service, NI_MAXSERV, 0) == 0)
    {
        std::cout << host << " connected on port " << service << std::endl;
    }
    else
    {
        inet_ntop(AF_INET, &client_addr.sin_addr, host, NI_MAXHOST);
        std::cout << host << " connected on port " << ntohs(client_addr.sin_port) << std::endl;
    }

    char msg[4096] = { 0 };
    while (1)
    {
        memset(msg, 0, 4096);
        int bytesReceived = recv(connfd, msg, 4096, 0);
        if (bytesReceived == -1)
        {
            std::cerr << host << " :Error in recv()\n";
            break;
        }
        if (bytesReceived == 0)
        {
            std::cout << host << " disconnected\n";
            break;
        }

        send(connfd, msg, bytesReceived + 1, 0);
    }
    
    close(connfd);
}

int main(int argc, char **argv)
{
    if(argc != 3)
    {
        std::cerr << "Usage: ./EchoServer <IP> <Port>\n";
        return -1;
    }

    signal(SIGINT, sig_handler);
    signal(SIGTSTP, sig_handler);

    sockaddr_in server_addr;
    sockaddr_in client_addr;
    //std::vector<std::thread> thread_pool;
    socklen_t clientSize = sizeof(client_addr);

    std::cout << "Server starting...\n";

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
    {
        std::cerr << "Socket cannot be created\n";
        return -1;
    }
    std::cout << "Socket has been created\n";

    memset((sockaddr*)&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;

    int port = std::atoi(argv[2]);
    if (port < 0 || port > 65535) 
    {
        std::cerr << "Port number out of range (0-65535)\n";
        close(sockfd);
        return -1;
    }

    server_addr.sin_port = htons(port);
    int ip_addr = inet_pton(AF_INET, argv[1], &server_addr.sin_addr);
    if (ip_addr < 1)
    {
        std::cerr << "IP not valid\n";
        close(sockfd);
        return -1;
    }

    if (bind (sockfd, (sockaddr*)&server_addr, sizeof(server_addr)) != 0)
    {
        std::cerr << "Binding failed\n";
        close(sockfd);
        return -1;
    }
    std::cout << "Binding successful\n";

    if ((listen(sockfd, 5)) != 0)
    {
        std::cerr << "Server not listening\n";
        close(sockfd);
        return -1;
    }

    std::cout << "Server listening...\n";

    while(1)
    {
        int connfd;
        connfd = accept(sockfd, (sockaddr*)&client_addr, &clientSize);

        std::thread t(echo, client_addr, connfd);
        t.detach();
    }
}