#include <stdio.h>
#include <winsock2.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <locale.h>
#endif

#pragma comment(lib, "ws2_32.lib")

// 根据实验手册定义服务器信息
#define SERV_TCP_PORT 6000           // 服务器端口号
#define SERV_HOST_ADDR "127.0.0.1" // 服务器 IP 地址

// 用于打印错误信息的宏
#define PRINTERROR(s) fprintf(stderr, "\n%s: %d\n", s, WSAGetLastError())

int main() {
    #ifdef _WIN32
        SetConsoleOutputCP(65001);
    #endif

    // 初始化 Winsock
    WSADATA wsaData;
    WORD wVersionRequested = MAKEWORD(2, 2);
    int nRet = WSAStartup(wVersionRequested, &wsaData);
    if (nRet != 0) {
        printf("WSAStartup 失败，错误代码: %d\n", nRet);
        return 1;
    }

    // 创建一个 TCP 套接字
    SOCKET theSocket;
    theSocket = socket(AF_INET,       // 地址族: Internet
                       SOCK_STREAM,   // 类型: 流式 (用于TCP)
                       0);            // 协议: 流式套接字的默认协议 (TCP)
    if (theSocket == INVALID_SOCKET) {
        PRINTERROR("socket()");
        WSACleanup();
        return 1;
    }

    // 设置服务器地址结构
    SOCKADDR_IN saServer;
    saServer.sin_family = AF_INET;
    saServer.sin_port = htons(SERV_TCP_PORT);
    saServer.sin_addr.s_addr = inet_addr(SERV_HOST_ADDR);

    // 连接到服务器
    printf("正在尝试连接到 %s:%d...\n", SERV_HOST_ADDR, SERV_TCP_PORT);
    nRet = connect(theSocket, (LPSOCKADDR)&saServer, sizeof(saServer));
    if (nRet == SOCKET_ERROR) {
        PRINTERROR("connect()");
        closesocket(theSocket);
        WSACleanup();
        return 1;
    }
    printf("连接已建立。\n");

    // 向服务器发送数据
    char txBuf[1024] = "Hello from TCP Client!";
    printf("正在向服务器发送数据: %s\n", txBuf);
    nRet = send(theSocket, txBuf, strlen(txBuf), 0);
    if (nRet == SOCKET_ERROR) {
        PRINTERROR("send()");
        closesocket(theSocket);
        WSACleanup();
        return 1;
    }

    // 从服务器接收数据
    char rxBuf[1024];
    memset(rxBuf, 0, sizeof(rxBuf));
    nRet = recv(theSocket, rxBuf, sizeof(rxBuf), 0);
    if (nRet > 0) {
        printf("从服务器接收到数据: %s\n", rxBuf);
    } else if (nRet == 0) {
        printf("服务器已关闭连接。\n");
    } else {
        PRINTERROR("recv()");
    }

    // 关闭套接字并清理 Winsock
    closesocket(theSocket); 
    WSACleanup();

    getchar();
    return 0;
}