// 通讯-TCPServer.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <iostream>
#include <cstdlib>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <Windows.h>
#pragma comment(lib,"ws2_32.lib")  //静态连接库

using namespace std;

#define SERVERIP "192.168.5.108"            //IP地址
#define SERVERPORT 8951                    //端口号

int _tmain(int argc, _TCHAR* argv[])
{
	WORD version = MAKEWORD(2, 2);                    //(1)MAKEWORD 宏，将两个8babyte型合并成一个word型，一个在高8位，一个在低8位 (2)不是C++标准的类型，是微软SDK中的类型，WORD的意思为字，是2byte的无符号整数，表示范围0~65535.
	WSADATA lpData;                                    //WSADATA，一种分子结构。这个结构被用来存储被WSAStartup函数调用后返回的Windows Sockets数据。它包含Winsock.dll执行的数据。
	
	//WSAStartup对winsock DLL（动态链接库文件）进行初始化，协商Winsock的版本支持，并分配必要的资源。  
	int intEr = WSAStartup(version, &lpData);          //指定winsock并初始化
	if (intEr != 0)                                    //如果初始化失败（返回值为非0）
	{
		cout << "winsock init failed!" << endl;
		return 0;
	}
	else
	{
		cout << "winsock init success!" << endl;
	}

	//LOBYTE()取得16进制数最低位；HIBYTE()取得16进制数最高（最左边）那个字节的内容        
	if (LOBYTE(lpData.wVersion) != 2 || HIBYTE(lpData.wVersion) != 2)
	{
		WSACleanup();
	}


	//SOCKET Socket（int af，int type,int protocol）;
	//调用成功则返回SOCKET类型的套接字句柄，否则返回INVALID_SOCKET错误
	//af：协议簇，在window socket中，只能为AF_INET(网际协议)
	SOCKET listenSock = socket(AF_INET, SOCK_STREAM, 0);                      	//创建侦听socket
	if (listenSock == INVALID_SOCKET)
	{
		cout << "socket error" << endl;
		return 0;
	}
	else
	{
		cout << "create socket success" << endl;
	}

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
	
	//协议簇
	hostAddr.sin_family = AF_INET;           

	//端口号
	hostAddr.sin_port = htons(SERVERPORT);    //htons用来将主机字节顺序转换为网络字节顺序(to network short)
	
	//IP
	//INADDR_ANY就是指定地址为0.0.0.0的地址, 表示不确定地址,或“任意地址”。”  
	hostAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);      	//htonl用来将主机字节顺序转换为网络字节顺序(to network long)  

	/*
	Int bind(Socket s,const struct sockaddr Far* name,int namelen)
	将本地地址绑定到所创建的socket上，以使在网络上标识该socket，侦听套接字listenSock绑定本机地址的信息
	成功返回0，失败返回非0
	参数说明：
	s：等待客户进行连接的套接字
	name:指向struct sockaddr类型的缓冲区的指针，在调用bind前，必须先对其进行填充，内容为本地地址信息
	struct sockaddr_in
	{
		short    sin_family;  //协议簇
		u_short  sin_port;    //端口号，使用前应用htons函数转换
		struct   in_addr  sin_addr;   //ip地址，使用前应该用htonl函数转换
		char sin_zero[8];
	}
	*/
	int err;
	err = bind(listenSock, (struct sockaddr*)&hostAddr, sizeof(sockaddr));          //指定本机地址
	if (err != 0)
	{
		cout << "hostAddr bind failed!" << endl;
		return 0;
	}

	/*
	int listen(SOCKET s,int backlog);
	s：要监听的已绑定但却没有连接的套接字
	backlog：内核为此套接字排队的最大连接个数，多于backlog的丢弃
	*/
	err = listen(listenSock, 3);          //监听连接
	if (err != 0)
	{
		cout << "listen socket listen failed!" << endl;
	}
	cout << "listening..." << endl;
	
	int no = 1;
	while (true)
	{
		sockaddr_in clientAddr;
		int len = sizeof(struct sockaddr);  //必须指定长度，否者会导致accept返回10014错误

		//accept会循环等待客户端的连接
		SOCKET clientSock = accept(listenSock, (struct sockaddr*)&clientAddr, &len);
		if (clientSock == INVALID_SOCKET)
		{
			cout << "accept failed!" << endl;
			cout << WSAGetLastError() << endl;
			system("pause");
			return 0;
		}

		cout << "连接成功，客户端IP地址为：" << endl;
		cout << inet_ntoa(clientAddr.sin_addr) << endl;  //返回ip地址

		unsigned long node = clientAddr.sin_addr.S_un.S_addr;
		//返回对应给定地址的包含主机名字和信息的hostent结构指针
		struct hostent * phost = gethostbyaddr((char *)&node, 4, AF_INET);      

		cout << "客户端的名称是：" << endl;
		cout << phost->h_name << endl;


		//接收数据数组
		char buf[1024] = "\0"; 

		//recv接收
		int buflen = recv(clientSock, buf, 1024, 0);
		if (buflen == SOCKET_ERROR)
		{
			cout << "recv failed!" << endl;
			return 0;
		}

		cout << "receive data" << no++ << ": " << buf << endl;

		//释放连接（所连接的客户端）
		err = shutdown(clientSock, 2);        
		if (err == SOCKET_ERROR)
		{
			cout << "shutdown failed!" << endl;
			return 0;
		}
	}
	
	//释放连接（服务器端）
	err = shutdown(listenSock, 2);          //尽量选用shutdown
	if (err == SOCKET_ERROR)
	{
		cout << "shutdown failed!" << endl;
		return 0;
	}

	//释放连接（服务器端）
	err = closesocket(listenSock);     //可能造成数据丢失
	if (err == SOCKET_ERROR)
	{
		cout << "closesocket failed!" << endl;
		return 0;
	}

	if (WSACleanup() != 0)         //功能是 终止Winsock 2 DLL (Ws2_32.dll) 的使用.
	{
		cout << "WSACleanup failup!" << endl;
		return 0;
	}

	system("pause");
	return 0;
}

