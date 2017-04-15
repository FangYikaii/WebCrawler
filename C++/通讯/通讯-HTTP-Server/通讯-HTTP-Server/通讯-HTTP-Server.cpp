// ͨѶ-HTTP-Server.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include <Winsock2.h>
#include <windows.h>
#include <malloc.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#pragma comment(lib,"ws2_32.lib")

#define uPort 80
#define MAX_BUFFER 100000
#define SENDBLOCK  200000
#define SERVERNAME "AcIDSoftWebServer/0.1b"
#define FileName   "HelloWorld.html"


//�����洢SOCKET��Ϣ
typedef struct _NODE_
{
	SOCKET s;
	sockaddr_in Addr;
	_NODE_* pNext;
}Node,*pNode;

//�������߳�
typedef struct _THREAD_
{
	DWORD ThreadID;
	HANDLE hThread;
	_THREAD_* pNext;
}Thread,*pThread;

pNode pHead = NULL;
pNode pTail = NULL;
pThread pHeadThread = NULL;
pThread pTailThread = NULL;

bool InitSocket();                                                             //�̺߳���
DWORD WINAPI AcceptThread(LPVOID lpParam);                                     //
DWORD WINAPI ClientThread(LPVOID lpParam);                                     //
bool IoComplete(char* szRequest);                                              //���ݰ�У�麯��
bool AddClientList(SOCKET s, sockaddr_in addr);                                //
bool AddThreadList(HANDLE hThread, DWORD ThreadID);                            //
bool ParseRequest(char* szRequest, char* szResponse, BOOL &bKeepAlive);        //

char HtmlDir[512] = { 0 };

int _tmain(int argc, _TCHAR* argv[])
{
	//��ʼ���߳�
	if (!InitSocket())
	{
		printf("InitSocket Error\n");
		return 0;
	}

	//��ȡ��ǰ·��
	GetCurrentDirectory(512, HtmlDir);
	//��ǰ�뾳+\\HTML
	strcat(HtmlDir, "\\HTML\\");
	
	//����һ�������߳�
	/*
	HANDLE CreateThread(
	  LPSECURITY_ATTRIBUTES lpsa, //��ȫ���Դ�С
	  DWORD cbStack, //�̳߳�ʼջ��С
	  LPTHREAD_START_ROUTINE lpStartAddr,  //�߳̿�ʼ��λ��
	  LPVOID lpvThreadParam,  //�����̹߳��̺����Ĳ���
	  DWORD fdwCreate,  //�����߳�ʱ�ı�־
	  LPDWORD lpIDThread, //�����̵߳�ID
	);
	*/
	HANDLE hAcceptThread = CreateThread(NULL, 0, AcceptThread, NULL, 0, NULL);

	//ʹ���¼�ģ����ʵ��Web������
	WaitForSingleObject(hAcceptThread, INFINITE);
	return 0;
}


bool InitSocket()
{
	WSADATA wsadata;
	if (WSAStartup(MAKEWORD(2, 2), &wsadata) == 0)  
	{
		return true;
	}
	else
	{
		return false;
	}
}

DWORD WINAPI AcceptThread(LPVOID lpParam)
{
	//����һ�������׽���(ʹ���¼��ص����׽���)
	SOCKET sListen = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (sListen == INVALID_SOCKET)
	{
		printf("Create Listen Error\n");
		return -1;
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
	//��ʼ������������ַ
	sockaddr_in LocalAddr;
	LocalAddr.sin_addr.S_un.S_addr = INADDR_ANY;      //INADDR_ANY����ָ����ַΪ0.0.0.0�ĵ�ַ, ��ʾ��ȷ����ַ,�������ַ������  
	LocalAddr.sin_family = AF_INET;                   //Э���
	LocalAddr.sin_port = htons(uPort);                //�˿�

	//�󶨣������׽���+����������ַ
	int Ret = bind(sListen, (sockaddr*)&LocalAddr, sizeof(LocalAddr));
	if (Ret == SOCKET_ERROR)
	{
		printf("Bind Error\n");
		return -1;
	}

	//����
	listen(sListen, 5);
	//����һ���¼�
	WSAEVENT Event = WSACreateEvent();
	if (Event == WSA_INVALID_EVENT)
	{
		printf("Create WSAEVENT Error\n");
		//�����¼�ʧ��
		closesocket(sListen);
		//�ر��׽���
		CloseHandle(Event);
		return - 1;
	}
	
	//�����Ǽ������׽��������ǵ��¼����й�������ΪAccept
	WSAEventSelect(sListen, Event, FD_ACCEPT);
	WSANETWORKEVENTS NetWorkEvent;
	sockaddr_in ClientAddr;
	int nLen = sizeof(ClientAddr);
	DWORD dwIndex = 0;
	while (true)
	{
		dwIndex = WSAWaitForMultipleEvents(1, &Event, FALSE, WSA_INFINITE, FALSE);
		dwIndex = dwIndex - WAIT_OBJECT_0;
		if (dwIndex == WSA_WAIT_TIMEOUT || dwIndex == WSA_WAIT_FAILED)
		{
			continue;
		}
		//�����������ʱ��ͽ����ж�
		WSAEnumNetworkEvents(sListen, Event, &NetWorkEvent);
		ResetEvent(&Event);
		if (NetWorkEvent.lNetworkEvents == FD_ACCEPT)
		{
			if (NetWorkEvent.iErrorCode[FD_ACCEPT_BIT] == 0)
			{
				//�������ҪΪ�µ����ӽ��н��ܲ������ڴ����������
				SOCKET sClient = WSAAccept(sListen, (sockaddr*)&ClientAddr, &nLen, NULL, NULL);
				if (sClient == INVALID_SOCKET)
				{
					continue;
				}
				//������ܳɹ�����Ҫ���û���������Ϣ��ŵ�������
				else
				{
					if (!AddClientList(sClient, ClientAddr))
					{
						continue;
					}
				}
			}
		}
	}
	return 0;
}

bool AddClientList(SOCKET s, sockaddr_in addr)
{
	pNode pTemp = (pNode)malloc(sizeof(Node));
	HANDLE hThread = NULL;
	DWORD ThreadID = 0;
	if (pTemp == NULL)
	{
		printf("No Memory\n");
		return false;
	}
	else
	{
		pTemp->s = s;
		pTemp->Addr = addr;
		pTemp->pNext = NULL;
		
		if (pHead == NULL)
		{
			pHead = pTail = pTemp;
		}
		else
		{
			pTail->pNext = pTemp;
			pTail = pTail->pNext;
		}
		
		//Ϊ�û������µ��߳�
		hThread = CreateThread(NULL, 0, ClientThread, (LPVOID)pTemp, 0, &ThreadID);
		if (hThread == NULL)
		{
			free(pTemp);
			return false;
		}
		if (!AddThreadList(hThread, ThreadID))
		{
			free(pTemp);
			return false;
		}
	}
	return true;
}

DWORD WINAPI ClientThread(LPVOID lpParam)
{
	//��ÿ���û�����Ϣ�Բ�������ʽ������߳�
	pNode pTemp = (pNode)lpParam;
	//�ͻ����׽���
	SOCKET sClient = pTemp->s;
	WSAEVENT Event = WSACreateEvent();       //���¼�����ͨ���׽��ֹ������ж��¼�������
	WSANETWORKEVENTS NetWorkEvent;
	char szRequest[1024] = { 0 };            //������
	char szResponse[1024] = { 0 };           //��Ӧ����
	BOOL bKeepAlive = FALSE;                 //�Ƿ��������
	if (Event == WSA_INVALID_EVENT)
	{
		return -1;
	}

	//�����¼����׽���
	int Ret = WSAEventSelect(sClient, Event, FD_READ | FD_WRITE | FD_CLOSE);
	DWORD dwIndex = 0;
	while (true)
	{
		dwIndex = WSAWaitForMultipleEvents(1, &Event, FALSE, WSA_INFINITE, FALSE);
		dwIndex = dwIndex - WAIT_OBJECT_0;
		if (dwIndex == WSA_WAIT_TIMEOUT || dwIndex == WSA_WAIT_FAILED)
		{
			continue;
		}

		//����ʲô�����¼�����
		Ret = WSAEnumNetworkEvents(sClient, Event, &NetWorkEvent);
		//�������
		if (!NetWorkEvent.lNetworkEvents)
		{
			continue;
		}
		if (NetWorkEvent.lNetworkEvents&FD_READ)
		{
			DWORD NumberOfBytesRecvd;
			WSABUF Buffers;
			DWORD dwBufferCount = 1;
			char szBuffer[MAX_BUFFER];
			DWORD Flags = 0;
			Buffers.buf = szBuffer;
			Buffers.len = MAX_BUFFER;
			Ret = WSARecv(sClient, &Buffers, dwBufferCount, &NumberOfBytesRecvd, &Flags, NULL, NULL);
			
			memcpy(szRequest, szBuffer, NumberOfBytesRecvd);
			//У�����ݰ�
			if (!IoComplete(szRequest))
			{
				continue;
			}
			//�������ݰ�
			if (!ParseRequest(szRequest, szResponse, bKeepAlive))
			{
				continue;
			}
			DWORD NumberOfBytesSent = 0;
			DWORD dwBytesSent = 0;
			//������Ӧ���ͻ���
			do
			{
				Buffers.len = (strlen(szResponse) - dwBytesSent) >= SENDBLOCK ? SENDBLOCK : strlen(szResponse) - dwBytesSent;
				Buffers.buf = (char*)((DWORD)szResponse + dwBytesSent);
				Ret = WSASend(sClient, &Buffers, 1, &NumberOfBytesSent, 0, 0, NULL);
				if (SOCKET_ERROR != Ret)
				{
					dwBytesSent += NumberOfBytesSent;
				}
			} while ((dwBytesSent < strlen(szResponse)) && SOCKET_ERROR != Ret);
		}

		if (NetWorkEvent.lNetworkEvents&FD_CLOSE)
		{
			//���ڴ�����ͷŷ����ڴ�й¶
		}
	}
	return 0;
}

//У�����ݰ�
bool IoComplete(char* szRequest)
{
	char* pTemp = NULL;                      //������ʱ��ָ��
	int nLen = strlen(szRequest);            //�������ݰ�����
	pTemp = szRequest + nLen - 4;            //��λָ��
	if (strcmp(pTemp, "\r\n\r\n") == 0)      //У������ͷ����ĩβ�Ļس����Ʒ��ͻ��з��Լ�����
	{
		return true;
	}
	return false;
}

//�������ݰ�
bool ParseRequest(char* szRequest,char* szResponse,BOOL &bKeepAlive)
{
	char* p = NULL;
	p = szRequest;
	int n = 0;
	char* pTemp = strstr(p, " ");    //�ж��ַ���str2�Ƿ���str1���Ӵ�������ǣ���ú�������str2��str1���״γ��ֵĵ�ַ�����򣬷���NULL��
	n = pTemp - p;       //ָ�볤��
	//����һ����ʱ�Ļ�����
	char szMode[10] = { 0 };
	char szFileName[10] = { 0 };
	//�����󷽷�������szMode������
	memcpy(szMode, p, n); 
	//
	if (strcmp(szMode, "GET") == 0)
	{
		//��ȡ�ļ���
		pTemp = strstr(pTemp, " ");
		pTemp = pTemp + 1;
		memcpy(szFileName, pTemp, 1);
		if (strcmp(szFileName, FileName))
		{
			strcpy(szFileName, FileName);
		}
		else
		{
			return false;
		}
	}
	else
	{
		return false;
	}
	// ������������
	pTemp = strstr(szRequest, "\nConnection: Keep-Alive");  //Э��汾
	n = pTemp - p;
	if (p>0)
	{
		bKeepAlive = TRUE;
	}
	else  //�����������Ϊ��Proxy���������
	{
		bKeepAlive = TRUE;
	}
	//����һ������ͷ
	char pResponseHeader[512] = { 0 };
	char szStatusCode[20] = { 0 };
	char szContentType[20] = { 0 };
	strcpy(szStatusCode, "200 OK");
	strcpy(szContentType, "text/html");
	char szDT[128];
	time_t t = time(0);
	strftime(szDT, 128, "%a, %d %b %Y %H:%M:%S GMT", gmtime(&t));
	//��ȡ�ļ�
	//����һ���ļ���ָ��
	FILE* fp = fopen(HtmlDir, "rb");
	fpos_t lengthActual = 0;
	int length = 0;
	char* BufferTemp = NULL;
	if (fp != NULL)
	{
		// ����ļ���С
		fseek(fp, 0, SEEK_END);
		fgetpos(fp, &lengthActual);
		fseek(fp, 0, SEEK_SET);
		//������ļ��Ĵ�С�����ǽ��з����ڴ�
		BufferTemp = (char*)malloc(sizeof(char)*((int)lengthActual));
		length = fread(BufferTemp, 1, (int)lengthActual, fp);
		fclose(fp);
		// ������Ӧ
		sprintf(pResponseHeader, "HTTP/1.0 %s\r\nDate: %s\r\nServer: %s\r\nAccept-Ranges: bytes\r\nContent-Length: %d\r\nConnection: %s\r\nContent-Type: %s\r\n\r\n",
			szStatusCode, szDT, SERVERNAME, length, bKeepAlive ? "Keep-Alive" : "close", szContentType);   //��Ӧ����
	}
	//������ǵ��ļ�û���ҵ����ǽ������û�������Ĵ���ҳ��
	else
	{
	}
	strcpy(szResponse, pResponseHeader);
	strcat(szResponse, BufferTemp);
	free(BufferTemp);
	BufferTemp = NULL;
	return true;

}

bool AddThreadList(HANDLE hThread, DWORD ThreadID)
{
	pThread pTemp = (pThread)malloc(sizeof(Thread));
	if (pTemp == NULL)
	{
		printf("No Memory\n");
		return false;
	}
	else
	{
		pTemp->hThread = hThread;
		pTemp->ThreadID = ThreadID;
		pTemp->pNext = NULL;
		if (pHeadThread == NULL)
		{
			pHeadThread = pTailThread = pTemp;
		}
		else
		{
			pTailThread->pNext = pTemp;
			pTailThread = pTailThread->pNext;
		}
	}
	return true;
}