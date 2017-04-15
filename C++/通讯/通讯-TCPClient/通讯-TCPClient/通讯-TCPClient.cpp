// ͨѶ-TCPClient.cpp : �������̨Ӧ�ó������ڵ㡣
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
	WORD sockVersion = MAKEWORD(2, 2);  //(1)MAKEWORD �꣬������8byte�ͺϲ���һ��word�ͣ�һ���ڸ�8λ��һ���ڵ�8λ (2)����C++��׼�����ͣ���΢��SDK�е����ͣ�WORD����˼Ϊ�֣���2byte���޷�����������ʾ��Χ0~65535.
	WSADATA initData;        //WSADATA��һ�ַ��ӽṹ������ṹ�������洢��WSAStartup�������ú󷵻ص�Windows Sockets���ݡ�������Winsock.dllִ�е����ݡ�

	//WSAStartup��winsock DLL����̬���ӿ��ļ������г�ʼ����Э��Winsock�İ汾֧�֣��������Ҫ����Դ��  
	int err = WSAStartup(sockVersion, &initData);  //ָ��winsock����ʼ��
	if (err != 0)
	{
		cout << "init Failed!" << endl;
	}
	while (true)
	{
		/*
		struct sockaddr_in
		{
		short    sin_family;  //Э���
		u_short  sin_port;    //�˿ںţ�ʹ��ǰӦ��htons����ת��
		struct   in_addr  sin_addr;   //ip��ַ��ʹ��ǰӦ����htonl����ת��
		char sin_zero[8];
		}
		*/
		sockaddr_in hostAddr;
		hostAddr.sin_family = AF_INET;          //Э���
		hostAddr.sin_port = htons(SERVERPORT);  //�˿ںţ�ʹ��ǰӦ��htons����ת��

		in_addr addr;           //ip��ַ��ʹ��ǰӦ����htonl����ת��
		inet_pton(AF_INET, SERVERIP, (void*)&addr);
		hostAddr.sin_addr = addr;
		cout << "������ip��ַ:" << htonl(addr.S_un.S_addr) << endl;

		//SOCKET Socket��int af��int type,int protocol��;
		//���óɹ��򷵻�SOCKET���͵��׽��־�������򷵻�INVALID_SOCKET����
		//af��Э��أ���window socket�У�ֻ��ΪAF_INET(����Э��)
		SOCKET conSock = socket(AF_INET, SOCK_STREAM, 0);  //��������sock
		if (conSock == INVALID_SOCKET)
		{
			cout << "conSock failed" << endl;
			system("pause");
			return 0;
		}

		//�����׽������ӣ��ͻ��ˣ�
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
		//��������
		err = send(conSock, buf, strlen(buf), 0);
		if (err == SOCKET_ERROR)
		{
			cout << "send failde!" << endl;
			system("pause");
			return 0;
		}
		//�ͷ�����
		err = closesocket(conSock);
		if (err != 0)
		{
			cout << "closesocket failed!" << endl;
			system("pause");
			return 0;
		}
	}
	//������ ��ֹWinsock 2 DLL (Ws2_32.dll) ��ʹ��.
	WSACleanup();

	system("pause");
	return 0;
}

