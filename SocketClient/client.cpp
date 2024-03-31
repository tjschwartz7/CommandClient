#include <iostream>
#include <string>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

int main() {
    // Initialize Winsock
    WSADATA wsData;
    WORD ver = MAKEWORD(2, 2);
    int wsResult = WSAStartup(ver, &wsData);
    if (wsResult != 0) 
    {
        std::cerr << "Can't start Winsock, Error: " << wsResult << std::endl;
        return 1;
    }

    // Create a socket
    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) 
    {
        std::cerr << "Can't create socket, Error: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return 1;
    }

    // Connect to the server
    sockaddr_in hint;
    hint.sin_family = AF_INET;
    hint.sin_port = htons(54000); // Port server is listening on

    // Convert IP address from string to binary form
    //We are connecting to... localhost!
    if (InetPton(AF_INET, TEXT("127.0.0.1"), &hint.sin_addr) != 1) 
    {
        std::cerr << "Invalid address. Error: " << WSAGetLastError() << std::endl;
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    //Attempt to connect 
    int connResult = connect(sock, (sockaddr*)&hint, sizeof(hint));
    if (connResult == SOCKET_ERROR) 
    {
        std::cerr << "Can't connect to server, Error: " << WSAGetLastError() << std::endl;
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    // Do-while loop to send and receive data
    char buf[4096];
    std::string userInput;
    bool connectionTerminated = false;

    do 
    {
        userInput.clear();
        // Prompt the user for some text
        std::cout << "> ";
        std::cin.getline(buf, 4096);
        userInput = buf;

        fd_set fdread;
        struct timeval socket_timeout;
        if (userInput.size() > 0) {
            // Send the text
            std::cout << "Attempting to send a message: " << userInput.c_str() << std::endl;

            int sendResult = send(sock, userInput.c_str(), userInput.size() + 1, 0);
            std::cout << sendResult << std::endl;
            if (sendResult != SOCKET_ERROR) {
                // Wait for response
                ZeroMemory(buf, 4096);

                socket_timeout.tv_sec = 1;
                socket_timeout.tv_usec = 0;
                FD_ZERO(&fdread);
                FD_SET(sock, &fdread);
                
                //Select returns the number of FDs on success
                int selectStatus = select(sock+1, &fdread, NULL, NULL, &socket_timeout);
                switch (selectStatus)
                {
                    //Any nonpositive integer is a failure case
                case -1:
                case 0:
                    //Timed out
                    connectionTerminated = true;
                    break;
                default:
                    int bytesReceived = recv(sock, buf, 4096, 0);
                    if (bytesReceived > 0) {
                        // Echo response to console
                        std::cout << "Server> " << std::string(buf, 0, bytesReceived) << std::endl;
                    }
                }
                
            }
            else {
                std::cerr << "Send failed, Error: " << WSAGetLastError() << std::endl;
                break;
            }
        }

        ZeroMemory(buf, 4096);
    } while (userInput.size() > 0 && !connectionTerminated);

    // Gracefully close everything
    closesocket(sock);
    WSACleanup();

    return 0;
}
