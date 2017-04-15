// ͨѶ-��������.cpp : �������̨Ӧ�ó������ڵ㡣
//
/*
httpЭ��, socket, ������ʽ
���е� URL ȫ������ urls ���������.
����Ҫ push һ���� URL.
֮��������ж���.
���̴��������:
��urlsȡ��һ��URL => ����URL��ҳȫ������ => ��������URL => ��URL�Ž� urls => �� urls ����һ�� URL.
URL �� host + get.�����Ҫһ�� binaryString �����п�.
Ч��Ҳ���Ǻܿ�, 1���Ӵ��4W��URL, ȥ���ظ�����Ҳ�кü�ǧ��.
*/

#include "stdafx.h"
#include <WinSock2.h>
#include <string>
#include <queue>
#include <iostream>
#include <regex>
#include <Windows.h>

#pragma comment(lib,"ws2_32.lib")
using namespace std;

#define PORT 80

void GoURL(const string &, int);                                              //�ָ�URL:HOST+GET
inline pair<string, string> binaryString(const string&, const string&);       //����socket
inline SOCKET connect(const string &);                                        //
inline string getIpByHostName(const string &);                                //����������������ȡIP��ַ
inline bool sendRequest(SOCKET, const string &, const string &);
inline string recvRequest(SOCKET);
inline void extUrl(const string &, queue<string>&);

int _tmain(int argc, _TCHAR* argv[])
{ 
	//(1)MAKEWORD �꣬������8babyte�ͺϲ���һ��word�ͣ�һ���ڸ�8λ��һ���ڵ�8λ (2)����C++��׼�����ͣ���΢��SDK�е����ͣ�WORD����˼Ϊ�֣���2byte���޷�����������ʾ��Χ0~65535.
	//WSADATA��һ�ַ��ӽṹ������ṹ�������洢��WSAStartup�������ú󷵻ص�Windows Sockets���ݡ�������Winsock.dllִ�е����ݡ�
	WSADATA wsadata;   

	//WSAStartup��winsock DLL����̬���ӿ��ļ������г�ʼ����Э��Winsock�İ汾֧�֣��������Ҫ����Դ�� 
	WSAStartup(MAKEWORD(2, 0), &wsadata);
	
	//�������洦�� 
	GoURL("www.pcauto.com.cn", 200);

	//���winsock
	WSACleanup();

	return 0;
}

void GoURL(const string &url, int count)
{
	queue<string> urls;   //����queue����urls��sring�Ķ���
	urls.push(url);		  //���
	//auto�����������Զ����������Ǵ洢���ͱ�ʶ���������������Զ������б��ط�Χ���鷶Χ�ı�����������forѭ�����ڵı���������Ĭ��Ϊauto�洢���͡�
	//��ʵ�����ͨ������ʽ�����ı�������auto����, ���ǲ���Ҫ��ȷָ��auto�ؼ��֣�Ĭ�Ͼ���auto���ˡ�
	//auto�������뿪�������ǻ������Զ��ͷţ����ᷢ���ڴ������������˰���ָ����ࣩ��ʹ��auto�����������ǲ���Ҫ����ȥ�����Ƿ��ͷţ��Ƚϰ�ȫ�ɡ�
	for (auto i = 0; i != count; i++)
	{
		if (!urls.empty())
		{
			auto &url = urls.front();                //��ȡurl
			auto pair = binaryString(url, "/");
			auto sock = connect(pair.first);         //�����׽���,����host
			if (sock&&sendRequest(sock, pair.first, "/" + pair.second))
			{
				string x;
				x = recvRequest(sock);
				auto buffer = move(x);
				extUrl(buffer, urls);
			}
			closesocket(sock);
			cout << url << ":count=> " << urls.size() << endl;
			urls.pop();
		}
	}

}

//�ָ�URL:HOST+GET
inline pair<string, string> binaryString(const string&str, const string&dilme)
{
	//pair<T1,T2> P1; �����յ�pair����P1������Ԫ�أ�first��Ա��second��Ա���ֱ�ΪT1��T2����
	pair<string, string> result(str, "");
	auto pos = str.find(dilme);
	//�����ַ���a�Ƿ�����Ӵ�b,
	//������strA.find(strB) > 0����strA.find(strB) != string:npos(���������򷵻�һ���ܴ��ֵ������string::npos�����������򷵻����ַ������ڵ�index��������string::npos)
	//�������"/"
	if (pos != string::npos)
	{
		//result��first��Ա
		result.first = str.substr(0, pos);
		//result��first��Ա
		result.second = str.substr(pos + dilme.size());
	}
	return result;
}

//����Socket
inline SOCKET connect(const string &hostName)
{
	//�ַ���ip��ַ
	auto ip = getIpByHostName(hostName);
	if (ip.empty())
	{
		return 0;
	}
	//SOCKET Socket��int af��int type,int protocol��;
	//���óɹ��򷵻�SOCKET���͵��׽��־�������򷵻�INVALID_SOCKET����
	//af��Э��أ���window socket�У�ֻ��ΪAF_INET(����Э��)
	auto sock = socket(AF_INET, SOCK_STREAM, 0);     
	if (sock == INVALID_SOCKET)
	{
		return 0;
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
	SOCKADDR_IN addr;
	addr.sin_family = AF_INET;          	//Э���
	addr.sin_port = htons(PORT);            //htons�����������ֽ�˳��ת��Ϊ�����ֽ�˳��(to network short),httpĬ��80
	//string�����ĳ�Ա����c_str()��string ����ת����c�е��ַ�����ʽ��
	addr.sin_addr.s_addr = inet_addr(ip.c_str());       //inet_addr()�Ĺ����ǽ�һ�����ʮ���Ƶ�IPת����һ��������������u_long���ͣ�
	//�����׽�������
	if (connect(sock, (const sockaddr*)&addr, sizeof(SOCKADDR_IN)) == SOCKET_ERROR)
	{
		return 0;
	}
	return sock;
}

//����������������ȡIP��ַ
inline string getIpByHostName(const string &hostName)
{
	//gethostbyname()���ض�Ӧ�ڸ����������İ����������ֺ͵�ַ��Ϣ��hostent�ṹָ��,����������������ȡIP��ַ
	hostent* phost = gethostbyname(hostName.c_str());
	//inet_ntoa():��һ��ʮ���������ֽ���ת��Ϊ���ʮ����IP��ʽ���ַ�����
	//����õ���һ��ip������Ҫ��ӡ������ip��ַ���õ��ʮ���ƣ� ����Ҫ�õ�inet_ntoa������network to ascii��
	return phost ? inet_ntoa(*(in_addr *)phost->h_addr_list[0]) : "";
}

//����HTTP����GET��
inline bool sendRequest(SOCKET sock, const string &host, const string &get)
{
	string http = "GET " + get + " HTTP/1.1\r\n" + "HOST: " + host + "\r\n" + "Connection: close\r\n\r\n";
	return http.size() == send(sock, &http[0], http.size(), 0);
}

//HTTP������Ӧ
inline string recvRequest(SOCKET sock)
{
	static timeval wait = { 2, 0 };
	static auto buffer = string(2048 * 100, '\0');
	auto len = 0, reclen = 0;
	//��ȡ���е�HTTPЭ����Ӧ������������buffer
	do
	{
		fd_set fd = { 0 };
		FD_SET(sock, &fd);
		reclen = 0;
		/*
		 select�������ھ���һ�����߶���׽��ֵ�״̬������ÿһ���׽��֣������߿����������д���ߴ���״̬��Ϣ��
		 һ���������״̬���׽��ּ���fd_set�ṹ��ָ���� ��fd_set�ṹ���е��׽��ֱ���͵��������ṩ����ϵ��һ��
		 ���ڴˣ����WSAPROTOCOL_INFO�ṹ��������ͬ��providerIdֵ���׽��ֱ���Ϊ����ͬһ�������ṩ�ߡ�ֱ�����أ��ṹ�����ȥ��ӳ����ָ�������׽����Ӽ���
		 select�������������������׽��ָ�����fd_set���Ͽ���ͨ��һЩ���ֶ���������Щ��Ҳ�ʺϲ������׽��֣��������ǵĻ����Ǹ�����ͬ��
		 int select(int nfds,fd_set FAR* readfds,fd_set FAR* writefds,fd_set FAR* exceptfds,const struct timeval FAR* timeout);
		 nfds���ɺ���
		 readfds��ָ���׽��ּ��ϣ��Ƿ�ɶ�
		 writefds��ָ���׽��ּ��ϣ��Ƿ��д
		 exceptfds��ָ���׽��ּ��ϣ��Ƿ������
		 timeout��timeval�ṹ���ͽṹָ��
		*/
		if (select(0, &fd, nullptr, nullptr, &wait) > 0)
		{
			/*
			recv(SOCKET s,const char FAR* buf,int len��int flag);
			s�����ն˵��׽���������
			buf���������ݵĻ�������ַ
			len���������ݻ���������
			flags���������ݷ�ʽ
			*/
			reclen = recv(sock, &buffer[0] + len, 2048 * 100 - len, 0);
			if (reclen > 0)
			{
				len += reclen;
			}
		}
		FD_ZERO(&fd);
	} while (reclen > 0);
	string x = len > 11 ? buffer[9] == '2'&&buffer[10] == '0'&&buffer[11] == '0' ? buffer.substr(0, len) : "" : "";
	return x;
}

//URL��ջ
inline void extUrl(const string &buffer, queue<string>&urlQueue)
{
	if (buffer.empty())
	{
		return;
	}
	smatch result;    
	auto curIter = buffer.begin();   
	auto endIter = buffer.end();
	//��regex_search�����У��Ὣ�ҵ�ƥ�������浽һ��smatch����
	while (regex_search(curIter, endIter, result, regex("href=\"(https?:)?//\\S+\"")))
	{
		//��ƥ����ѹ��ջ��
		urlQueue.push(regex_replace(result[0].str(), regex("href=\"(https?:)?//(\\S+)\""), "$2"));
		curIter = result[0].second;
	}
}