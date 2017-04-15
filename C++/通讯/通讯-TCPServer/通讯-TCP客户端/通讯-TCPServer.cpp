// ͨѶ-TCPServer.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include <iostream>
#include <cstdlib>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <Windows.h>
#pragma comment(lib,"ws2_32.lib")  //��̬���ӿ�

using namespace std;

#define SERVERIP "192.168.5.108"            //IP��ַ
#define SERVERPORT 8951                    //�˿ں�

int _tmain(int argc, _TCHAR* argv[])
{
	WORD version = MAKEWORD(2, 2);                    //(1)MAKEWORD �꣬������8babyte�ͺϲ���һ��word�ͣ�һ���ڸ�8λ��һ���ڵ�8λ (2)����C++��׼�����ͣ���΢��SDK�е����ͣ�WORD����˼Ϊ�֣���2byte���޷�����������ʾ��Χ0~65535.
	WSADATA lpData;                                    //WSADATA��һ�ַ��ӽṹ������ṹ�������洢��WSAStartup�������ú󷵻ص�Windows Sockets���ݡ�������Winsock.dllִ�е����ݡ�
	
	//WSAStartup��winsock DLL����̬���ӿ��ļ������г�ʼ����Э��Winsock�İ汾֧�֣��������Ҫ����Դ��  
	int intEr = WSAStartup(version, &lpData);          //ָ��winsock����ʼ��
	if (intEr != 0)                                    //�����ʼ��ʧ�ܣ�����ֵΪ��0��
	{
		cout << "winsock init failed!" << endl;
		return 0;
	}
	else
	{
		cout << "winsock init success!" << endl;
	}

	//LOBYTE()ȡ��16���������λ��HIBYTE()ȡ��16��������ߣ�����ߣ��Ǹ��ֽڵ�����        
	if (LOBYTE(lpData.wVersion) != 2 || HIBYTE(lpData.wVersion) != 2)
	{
		WSACleanup();
	}


	//SOCKET Socket��int af��int type,int protocol��;
	//���óɹ��򷵻�SOCKET���͵��׽��־�������򷵻�INVALID_SOCKET����
	//af��Э��أ���window socket�У�ֻ��ΪAF_INET(����Э��)
	SOCKET listenSock = socket(AF_INET, SOCK_STREAM, 0);                      	//��������socket
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
		short    sin_family;  //Э���
		u_short  sin_port;    //�˿ںţ�ʹ��ǰӦ��htons����ת��
		struct   in_addr  sin_addr;   //ip��ַ��ʹ��ǰӦ����htonl����ת��
		char sin_zero[8];
	}
	*/
	sockaddr_in hostAddr;
	
	//Э���
	hostAddr.sin_family = AF_INET;           

	//�˿ں�
	hostAddr.sin_port = htons(SERVERPORT);    //htons�����������ֽ�˳��ת��Ϊ�����ֽ�˳��(to network short)
	
	//IP
	//INADDR_ANY����ָ����ַΪ0.0.0.0�ĵ�ַ, ��ʾ��ȷ����ַ,�������ַ������  
	hostAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);      	//htonl�����������ֽ�˳��ת��Ϊ�����ֽ�˳��(to network long)  

	/*
	Int bind(Socket s,const struct sockaddr Far* name,int namelen)
	�����ص�ַ�󶨵���������socket�ϣ���ʹ�������ϱ�ʶ��socket�������׽���listenSock�󶨱�����ַ����Ϣ
	�ɹ�����0��ʧ�ܷ��ط�0
	����˵����
	s���ȴ��ͻ��������ӵ��׽���
	name:ָ��struct sockaddr���͵Ļ�������ָ�룬�ڵ���bindǰ�������ȶ��������䣬����Ϊ���ص�ַ��Ϣ
	struct sockaddr_in
	{
		short    sin_family;  //Э���
		u_short  sin_port;    //�˿ںţ�ʹ��ǰӦ��htons����ת��
		struct   in_addr  sin_addr;   //ip��ַ��ʹ��ǰӦ����htonl����ת��
		char sin_zero[8];
	}
	*/
	int err;
	err = bind(listenSock, (struct sockaddr*)&hostAddr, sizeof(sockaddr));          //ָ��������ַ
	if (err != 0)
	{
		cout << "hostAddr bind failed!" << endl;
		return 0;
	}

	/*
	int listen(SOCKET s,int backlog);
	s��Ҫ�������Ѱ󶨵�ȴû�����ӵ��׽���
	backlog���ں�Ϊ���׽����Ŷӵ�������Ӹ���������backlog�Ķ���
	*/
	err = listen(listenSock, 3);          //��������
	if (err != 0)
	{
		cout << "listen socket listen failed!" << endl;
	}
	cout << "listening..." << endl;
	
	int no = 1;
	while (true)
	{
		sockaddr_in clientAddr;
		int len = sizeof(struct sockaddr);  //����ָ�����ȣ����߻ᵼ��accept����10014����

		//accept��ѭ���ȴ��ͻ��˵�����
		SOCKET clientSock = accept(listenSock, (struct sockaddr*)&clientAddr, &len);
		if (clientSock == INVALID_SOCKET)
		{
			cout << "accept failed!" << endl;
			cout << WSAGetLastError() << endl;
			system("pause");
			return 0;
		}

		cout << "���ӳɹ����ͻ���IP��ַΪ��" << endl;
		cout << inet_ntoa(clientAddr.sin_addr) << endl;  //����ip��ַ

		unsigned long node = clientAddr.sin_addr.S_un.S_addr;
		//���ض�Ӧ������ַ�İ����������ֺ���Ϣ��hostent�ṹָ��
		struct hostent * phost = gethostbyaddr((char *)&node, 4, AF_INET);      

		cout << "�ͻ��˵������ǣ�" << endl;
		cout << phost->h_name << endl;


		//������������
		char buf[1024] = "\0"; 

		//recv����
		int buflen = recv(clientSock, buf, 1024, 0);
		if (buflen == SOCKET_ERROR)
		{
			cout << "recv failed!" << endl;
			return 0;
		}

		cout << "receive data" << no++ << ": " << buf << endl;

		//�ͷ����ӣ������ӵĿͻ��ˣ�
		err = shutdown(clientSock, 2);        
		if (err == SOCKET_ERROR)
		{
			cout << "shutdown failed!" << endl;
			return 0;
		}
	}
	
	//�ͷ����ӣ��������ˣ�
	err = shutdown(listenSock, 2);          //����ѡ��shutdown
	if (err == SOCKET_ERROR)
	{
		cout << "shutdown failed!" << endl;
		return 0;
	}

	//�ͷ����ӣ��������ˣ�
	err = closesocket(listenSock);     //����������ݶ�ʧ
	if (err == SOCKET_ERROR)
	{
		cout << "closesocket failed!" << endl;
		return 0;
	}

	if (WSACleanup() != 0)         //������ ��ֹWinsock 2 DLL (Ws2_32.dll) ��ʹ��.
	{
		cout << "WSACleanup failup!" << endl;
		return 0;
	}

	system("pause");
	return 0;
}

