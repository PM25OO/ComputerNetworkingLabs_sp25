#include <stdio.h>
#include <winsock2.h> // 建议使用 winsock2.h，功能更全

#pragma comment(lib, "ws2_32.lib") // 链接 Winsock 库

// 根据实验手册定义服务器信息
#define SERV_UDP_PORT 6000           // 服务器端口号
#define SERV_HOST_ADDR "127.0.0.1" // 服务器 IP 地址

// 用于打印错误信息的宏
#define PRINTERROR(s) fprintf(stderr, "\n%s: %d\n", s, WSAGetLastError())

int main() {
    // 初始化 Winsock
    WSADATA wsaData;
    WORD wVersionRequested = MAKEWORD(2, 2); // 请求 Winsock 2.2 版本
    int nRet = WSAStartup(wVersionRequested, &wsaData);
    if (nRet != 0) {
        printf("WSAStartup 失败，错误代码: %d\n", nRet);
        return 1;
    }

    // 创建一个 UDP 套接字
    SOCKET theSocket;
    theSocket = socket(AF_INET,       // 地址族: Internet [cite: 1]
                       SOCK_DGRAM,    // 类型: 数据报 (用于UDP) [cite: 3]
                       IPPROTO_UDP);  // 协议: UDP [cite: 16]
    if (theSocket == INVALID_SOCKET) {
        PRINTERROR("socket()");
        WSACleanup();
        return 1;
    }

    // 设置服务器地址结构
    SOCKADDR_IN saServer;
    saServer.sin_family = AF_INET;
    saServer.sin_port = htons(SERV_UDP_PORT); // 设置端口号，并转换成网络字节顺序 [cite: 1]
    saServer.sin_addr.s_addr = inet_addr(SERV_HOST_ADDR); // 设置 IP 地址

    // 向服务器发送数据
    char txBuf[1024] = "Hello from UDP Client!";
    printf("正在向服务器发送数据: %s\n", txBuf);
    nRet = sendto(theSocket,                   // 要发送的套接字
                  txBuf,                       // 存放待发送数据的缓冲区
                  strlen(txBuf),               // 数据长度
                  0,                           // 标志位
                  (LPSOCKADDR)&saServer,       // 目的地址
                  sizeof(struct sockaddr));    // 地址结构的大小

    if (nRet == SOCKET_ERROR) {
        PRINTERROR("sendto()");
        closesocket(theSocket);
        WSACleanup();
        return 1;
    }

    // 从服务器接收数据
    char rxBuf[1024];
    memset(rxBuf, 0, sizeof(rxBuf));
    SOCKADDR_IN saClient;
    int nLen = sizeof(saClient);

    nRet = recvfrom(theSocket,            // 要接收的套接字
                    rxBuf,                // 存放接收数据的缓冲区 [cite: 18]
                    sizeof(rxBuf),        // 缓冲区大小 [cite: 18]
                    0,                    // 标志位 [cite: 18]
                    (LPSOCKADDR)&saClient,// 发送方地址
                    &nLen);               // 地址结构的大小 [cite: 18]

    if (nRet > 0) {
        printf("从服务器接收到数据: %s\n", rxBuf);
    } else if (nRet == SOCKET_ERROR) {
        PRINTERROR("recvfrom()");
    }

    // 关闭套接字并清理 Winsock
    closesocket(theSocket); // [cite: 13]
    WSACleanup();           // [cite: 22]

    getchar();
    return 0;
}