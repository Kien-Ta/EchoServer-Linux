#include<iostream>
#include<netdb.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<arpa/inet.h>
#include<cstdlib>
#include<signal.h>
#include<string.h>
#include<string>

int sockfd;

void sig_handler(int signo)
{
    if(signo == SIGINT)
    {
        // stop da server
        std::cout << "Disconnecting from server...\n";
        close(sockfd);
        exit(1);
    }
}

int main (int argc, char** argv)
{
    if (argc != 3)
    {
        std::cerr << "Usage: ./EchoServer <IP> <Port>\n";
        return -1;
    }

    signal(SIGINT, sig_handler);
    signal(SIGTSTP, sig_handler);

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
    {
        std::cerr << "Socket cannot be created\n";
        return -1;
    }
    std::cout << "Socket has been created\n";

    sockaddr_in server_addr;
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

    int connectRes = connect(sockfd, (sockaddr*)&server_addr, sizeof(server_addr));
    if (connectRes == -1)
    {
        std::cerr << "Cannot connect to server\n";
        close(sockfd);
        return -1;
    }

    char msg[4096];
    std::string userInput;
    do {
        std::cout << "> ";
        getline(std::cin, userInput);
        if(userInput.length() > 4095)
        {
            userInput.erase(userInput.begin() + 4095, userInput.end());
        }

        int sendRes = send(sockfd, userInput.c_str(), userInput.size() + 1, 0);
        if (sendRes == -1)
        {
            std::cerr << "Could not send to server\n";
            continue;
        }
        if(userInput.length() == 0)
        {
            std::cout << "Disconnecting from server...\n";
            close(sockfd);
            break;
        }

        memset(msg, 0, 4096);
        int bytesReceived = recv(sockfd, msg, 4096, 0);
        if (bytesReceived == -1)
        {
            std::cerr << "Error getting response from server\n";
            break;
        }
        else if (bytesReceived == 0)
        {
            std::cout << "Server is down\n";
            break;
        }
        else
        {
            std::cout << "SERVER> " << std::string(msg, bytesReceived) << "\n";
        }
    }while(true);

    close(sockfd);
    return 0;
}