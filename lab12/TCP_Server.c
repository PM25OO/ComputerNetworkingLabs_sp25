#include <stdio.h>
#include <winsock2.h>
#include <process.h> // 用于多线程 _beginthreadex

#pragma comment(lib, "ws2_32.lib")

#define SERVER_PORT 6000 // 服务器监听的端口
#define BUFFER_SIZE 1024 // 缓冲区大小
#define MAX_CONN 5       // 最大等待连接数

// 打印错误信息的宏
#define PRINTERROR(s) fprintf(stderr, "\n%s: %d\n", s, WSAGetLastError())

// 线程函数，用于处理单个客户端的请求
unsigned int __stdcall ClientThread(void* pParam) {
    SOCKET clientSocket = (SOCKET)pParam;
    char buffer[BUFFER_SIZE];

    // 1. 接收客户端数据
    int recvLen = recv(clientSocket, buffer, BUFFER_SIZE, 0);
    if (recvLen > 0) {
        buffer[recvLen] = '\0'; // 添加字符串结束符
        printf("收到客户端消息: %s\n", buffer);

        // 2. 发送回复给客户端
        const char* response = "服务器已收到您的TCP消息!";
        send(clientSocket, response, strlen(response), 0);
    } else if (recvLen == 0) {
        printf("客户端已断开连接。\n");
    } else {
        PRINTERROR("recv() 失败");
    }

    // 3. 关闭与此客户端的连接
    closesocket(clientSocket);
    return 0;
}


int main() {
    // 1. 初始化 Winsock
    WSADATA wsaData;
    int nRet = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (nRet != 0) {
        printf("WSAStartup 失败，错误代码: %d\n", nRet);
        return 1;
    }

    // 2. 创建 TCP 监听套接字
    SOCKET listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listenSocket == INVALID_SOCKET) {
        PRINTERROR("socket() 创建失败");
        WSACleanup();
        return 1;
    }

    // 3. 绑定地址和端口
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(SERVER_PORT);
    serverAddr.sin_addr.s_addr = INADDR_ANY; // 监听任意地址

    if (bind(listenSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        PRINTERROR("bind() 失败");
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }

    // 4. 开始监听
    if (listen(listenSocket, MAX_CONN) == SOCKET_ERROR) {
        PRINTERROR("listen() 失败");
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }

    printf("TCP 服务器已启动，正在端口 %d 上等待客户端连接...\n", SERVER_PORT);

    // 5. 循环接受客户端连接
    SOCKET clientSocket;
    struct sockaddr_in clientAddr;
    int clientAddrLen = sizeof(clientAddr);

    while (1) {
        // 接受一个客户端连接 (阻塞)
        clientSocket = accept(listenSocket, (struct sockaddr*)&clientAddr, &clientAddrLen);
        if (clientSocket == INVALID_SOCKET) {
            PRINTERROR("accept() 失败");
            continue; // 继续等待下一个连接
        }

        printf("接受到来自 %s:%d 的新连接。\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));

        // 创建一个新线程来处理此客户端
        HANDLE hThread = (HANDLE)_beginthreadex(NULL, 0, &ClientThread, (void*)clientSocket, 0, NULL);
        if (hThread) {
            // 关闭线程句柄，因为主线程不再需要它
            CloseHandle(hThread);
        }
    }

    // 6. 清理 (在无限循环中，此部分代码不会执行)
    closesocket(listenSocket);
    WSACleanup();

    return 0;
}