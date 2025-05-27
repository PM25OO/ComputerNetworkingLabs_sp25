#include <stdio.h>
#include <winsock2.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <locale.h>
#endif

#pragma comment(lib, "ws2_32.lib")

#define SERVER_PORT 6000 // 服务器监听的端口
#define BUFFER_SIZE 1024 // 缓冲区大小

// 打印错误信息的宏
#define PRINTERROR(s) fprintf(stderr, "\n%s: %d\n", s, WSAGetLastError())

int main() {
    #ifdef _WIN32
        SetConsoleOutputCP(65001);         // 设置控制台输出编码为UTF-8
    #endif

    // 1. 初始化 Winsock
    WSADATA wsaData;
    int nRet = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (nRet != 0) {
        printf("WSAStartup 失败，错误代码: %d\n", nRet);
        return 1;
    }

    // 2. 创建 UDP 套接字
    SOCKET serverSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (serverSocket == INVALID_SOCKET) {
        PRINTERROR("socket() 创建失败");
        WSACleanup();
        return 1;
    }

    // 3. 设置服务器地址并绑定
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(SERVER_PORT);
    serverAddr.sin_addr.s_addr = INADDR_ANY; // 监听任意网络接口上的连接

    nRet = bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
    if (nRet == SOCKET_ERROR) {
        PRINTERROR("bind() 失败");
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    printf("UDP 服务器已启动，正在端口 %d 上等待客户端消息...\n", SERVER_PORT);

    // 4. 循环接收和发送数据
    char recvBuf[BUFFER_SIZE];
    struct sockaddr_in clientAddr;
    int clientAddrLen = sizeof(clientAddr);

    while (1) {
        // 清空接收缓冲区
        memset(recvBuf, 0, BUFFER_SIZE);

        // 接收来自客户端的数据 (阻塞)
        int recvLen = recvfrom(serverSocket, recvBuf, BUFFER_SIZE, 0, (struct sockaddr*)&clientAddr, &clientAddrLen);
        if (recvLen > 0) {
            // 打印收到的消息和客户端地址
            printf("收到来自 %s:%d 的消息: %s\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port), recvBuf);

            // 准备并发送回复消息
            const char* sendBuf = "服务器已收到您的UDP消息!";
            sendto(serverSocket, sendBuf, strlen(sendBuf), 0, (struct sockaddr*)&clientAddr, clientAddrLen);
        } else {
            PRINTERROR("recvfrom() 失败");
        }
    }

    // 5. 清理 (在无限循环中，此部分代码不会执行)
    closesocket(serverSocket);
    WSACleanup();

    return 0;
}