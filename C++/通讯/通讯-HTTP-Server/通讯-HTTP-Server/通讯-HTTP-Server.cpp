// 通讯-HTTP-Server.cpp : 定义控制台应用程序的入口点。
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


//链表：存储SOCKET信息
typedef struct _NODE_
{
	SOCKET s;
	sockaddr_in Addr;
	_NODE_* pNext;
}Node,*pNode;

//链表：多线程
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

bool InitSocket();                                                             //线程函数
DWORD WINAPI AcceptThread(LPVOID lpParam);                                     //
DWORD WINAPI ClientThread(LPVOID lpParam);                                     //
bool IoComplete(char* szRequest);                                              //数据包校验函数
bool AddClientList(SOCKET s, sockaddr_in addr);                                //
bool AddThreadList(HANDLE hThread, DWORD ThreadID);                            //
bool ParseRequest(char* szRequest, char* szResponse, BOOL &bKeepAlive);        //

char HtmlDir[512] = { 0 };

int _tmain(int argc, _TCHAR* argv[])
{
	//初始化线程
	if (!InitSocket())
	{
		printf("InitSocket Error\n");
		return 0;
	}

	//获取当前路径
	GetCurrentDirectory(512, HtmlDir);
	//当前入境+\\HTML
	strcat(HtmlDir, "\\HTML\\");
	
	//启动一个接收线程
	/*
	HANDLE CreateThread(
	  LPSECURITY_ATTRIBUTES lpsa, //安全属性大小
	  DWORD cbStack, //线程初始栈大小
	  LPTHREAD_START_ROUTINE lpStartAddr,  //线程开始的位置
	  LPVOID lpvThreadParam,  //接收线程过程函数的参数
	  DWORD fdwCreate,  //创建线程时的标志
	  LPDWORD lpIDThread, //保存线程的ID
	);
	*/
	HANDLE hAcceptThread = CreateThread(NULL, 0, AcceptThread, NULL, 0, NULL);

	//使用事件模型来实现Web服务器
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
	//创建一个监听套接字(使用事件重叠的套接字)
	SOCKET sListen = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (sListen == INVALID_SOCKET)
	{
		printf("Create Listen Error\n");
		return -1;
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
	//初始化本服务器地址
	sockaddr_in LocalAddr;
	LocalAddr.sin_addr.S_un.S_addr = INADDR_ANY;      //INADDR_ANY就是指定地址为0.0.0.0的地址, 表示不确定地址,或“任意地址”。”  
	LocalAddr.sin_family = AF_INET;                   //协议簇
	LocalAddr.sin_port = htons(uPort);                //端口

	//绑定：监听套接字+本服务器地址
	int Ret = bind(sListen, (sockaddr*)&LocalAddr, sizeof(LocalAddr));
	if (Ret == SOCKET_ERROR)
	{
		printf("Bind Error\n");
		return -1;
	}

	//监听
	listen(sListen, 5);
	//创建一个事件
	WSAEVENT Event = WSACreateEvent();
	if (Event == WSA_INVALID_EVENT)
	{
		printf("Create WSAEVENT Error\n");
		//创建事件失败
		closesocket(sListen);
		//关闭套接字
		CloseHandle(Event);
		return - 1;
	}
	
	//将我们监听的套接字与我们的事件进行关联属性为Accept
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
		//如果有真正的时间就进行判断
		WSAEnumNetworkEvents(sListen, Event, &NetWorkEvent);
		ResetEvent(&Event);
		if (NetWorkEvent.lNetworkEvents == FD_ACCEPT)
		{
			if (NetWorkEvent.iErrorCode[FD_ACCEPT_BIT] == 0)
			{
				//如果我们要为新的连接进行接受并申请内存存入链表中
				SOCKET sClient = WSAAccept(sListen, (sockaddr*)&ClientAddr, &nLen, NULL, NULL);
				if (sClient == INVALID_SOCKET)
				{
					continue;
				}
				//如果接受成功我们要把用户的所有信息存放到链表中
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
		
		//为用户开辟新的线程
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
	//将每个用户的信息以参数的形式传入该线程
	pNode pTemp = (pNode)lpParam;
	//客户端套接字
	SOCKET sClient = pTemp->s;
	WSAEVENT Event = WSACreateEvent();       //该事件是与通信套接字关联以判断事件的种类
	WSANETWORKEVENTS NetWorkEvent;
	char szRequest[1024] = { 0 };            //请求报文
	char szResponse[1024] = { 0 };           //响应报文
	BOOL bKeepAlive = FALSE;                 //是否持续连续
	if (Event == WSA_INVALID_EVENT)
	{
		return -1;
	}

	//关联事件和套接字
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

		//分析什么网络事件产生
		Ret = WSAEnumNetworkEvents(sClient, Event, &NetWorkEvent);
		//其他情况
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
			//校验数据包
			if (!IoComplete(szRequest))
			{
				continue;
			}
			//分析数据包
			if (!ParseRequest(szRequest, szResponse, bKeepAlive))
			{
				continue;
			}
			DWORD NumberOfBytesSent = 0;
			DWORD dwBytesSent = 0;
			//发送响应到客户端
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
			//将内存进行释放否则内存泄露
		}
	}
	return 0;
}

//校验数据包
bool IoComplete(char* szRequest)
{
	char* pTemp = NULL;                      //定义临时空指针
	int nLen = strlen(szRequest);            //请求数据包长度
	pTemp = szRequest + nLen - 4;            //定位指针
	if (strcmp(pTemp, "\r\n\r\n") == 0)      //校验请求头部行末尾的回车控制符和换行符以及空行
	{
		return true;
	}
	return false;
}

//分析数据包
bool ParseRequest(char* szRequest,char* szResponse,BOOL &bKeepAlive)
{
	char* p = NULL;
	p = szRequest;
	int n = 0;
	char* pTemp = strstr(p, " ");    //判断字符串str2是否是str1的子串。如果是，则该函数返回str2在str1中首次出现的地址；否则，返回NULL。
	n = pTemp - p;       //指针长度
	//定义一个临时的缓冲区
	char szMode[10] = { 0 };
	char szFileName[10] = { 0 };
	//将请求方法拷贝到szMode数组中
	memcpy(szMode, p, n); 
	//
	if (strcmp(szMode, "GET") == 0)
	{
		//获取文件名
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
	// 分析链接类型
	pTemp = strstr(szRequest, "\nConnection: Keep-Alive");  //协议版本
	n = pTemp - p;
	if (p>0)
	{
		bKeepAlive = TRUE;
	}
	else  //这里的设置是为了Proxy程序的运行
	{
		bKeepAlive = TRUE;
	}
	//定义一个回显头
	char pResponseHeader[512] = { 0 };
	char szStatusCode[20] = { 0 };
	char szContentType[20] = { 0 };
	strcpy(szStatusCode, "200 OK");
	strcpy(szContentType, "text/html");
	char szDT[128];
	time_t t = time(0);
	strftime(szDT, 128, "%a, %d %b %Y %H:%M:%S GMT", gmtime(&t));
	//读取文件
	//定义一个文件流指针
	FILE* fp = fopen(HtmlDir, "rb");
	fpos_t lengthActual = 0;
	int length = 0;
	char* BufferTemp = NULL;
	if (fp != NULL)
	{
		// 获得文件大小
		fseek(fp, 0, SEEK_END);
		fgetpos(fp, &lengthActual);
		fseek(fp, 0, SEEK_SET);
		//计算出文件的大小后我们进行分配内存
		BufferTemp = (char*)malloc(sizeof(char)*((int)lengthActual));
		length = fread(BufferTemp, 1, (int)lengthActual, fp);
		fclose(fp);
		// 返回响应
		sprintf(pResponseHeader, "HTTP/1.0 %s\r\nDate: %s\r\nServer: %s\r\nAccept-Ranges: bytes\r\nContent-Length: %d\r\nConnection: %s\r\nContent-Type: %s\r\n\r\n",
			szStatusCode, szDT, SERVERNAME, length, bKeepAlive ? "Keep-Alive" : "close", szContentType);   //响应报文
	}
	//如果我们的文件没有找到我们将引导用户到另外的错误页面
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