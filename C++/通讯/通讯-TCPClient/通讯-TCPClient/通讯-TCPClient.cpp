// 通讯-TCPClient.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <iostream>
#include <cstdlib>
#include <WinSock2.h>
#include <WS2tcpip.h>
#pragma comment(lib,"ws2_32.lib")

using namespace std;

#define SERVERIP "192.168.5.108"
#define SERVERPORT 8951

int _tmain(int argc, _TCHAR* argv[])
{
	WORD sockVersion = MAKEWORD(2, 2);  //(1)MAKEWORD 宏，将两个8byte型合并成一个word型，一个在高8位，一个在低8位 (2)不是C++标准的类型，是微软SDK中的类型，WORD的意思为字，是2byte的无符号整数，表示范围0~65535.
	WSADATA initData;        //WSADATA，一种分子结构。这个结构被用来存储被WSAStartup函数调用后返回的Windows Sockets数据。它包含Winsock.dll执行的数据。

	//WSAStartup对winsock DLL（动态链接库文件）进行初始化，协商Winsock的版本支持，并分配必要的资源。  
	int err = WSAStartup(sockVersion, &initData);  //指定winsock并初始化
	if (err != 0)
	{
		cout << "init Failed!" << endl;
	}
	while (true)
	{
		/*
		struct sockaddr_in
		{
		short    sin_family;  //协议簇
		u_short  sin_port;    //端口号，使用前应用htons函数转换
		struct   in_addr  sin_addr;   //ip地址，使用前应该用htonl函数转换
		char sin_zero[8];
		}
		*/
		sockaddr_in hostAddr;
		hostAddr.sin_family = AF_INET;          //协议簇
		hostAddr.sin_port = htons(SERVERPORT);  //端口号，使用前应用htons函数转换

		in_addr addr;           //ip地址，使用前应该用htonl函数转换
		inet_pton(AF_INET, SERVERIP, (void*)&addr);
		hostAddr.sin_addr = addr;
		cout << "服务器ip地址:" << htonl(addr.S_un.S_addr) << endl;

		//SOCKET Socket（int af，int type,int protocol）;
		//调用成功则返回SOCKET类型的套接字句柄，否则返回INVALID_SOCKET错误
		//af：协议簇，在window socket中，只能为AF_INET(网际协议)
		SOCKET conSock = socket(AF_INET, SOCK_STREAM, 0);  //创建连接sock
		if (conSock == INVALID_SOCKET)
		{
			cout << "conSock failed" << endl;
			system("pause");
			return 0;
		}

		//建立套接字连接（客户端）
		err = connect(conSock, (sockaddr*)&hostAddr, sizeof(sockaddr));
		if (err == INVALID_SOCKET)
		{
			cout << "connect failed!" << endl;
			system("pause");
			return 0;
		}

		char buf[1024] = "\0";
		cout << "input data";
		cin >> buf;
		//发送数据
		err = send(conSock, buf, strlen(buf), 0);
		if (err == SOCKET_ERROR)
		{
			cout << "send failde!" << endl;
			system("pause");
			return 0;
		}
		//释放连接
		err = closesocket(conSock);
		if (err != 0)
		{
			cout << "closesocket failed!" << endl;
			system("pause");
			return 0;
		}
	}
	//功能是 终止Winsock 2 DLL (Ws2_32.dll) 的使用.
	WSACleanup();

	system("pause");
	return 0;
}

