// 通讯-网络爬虫.cpp : 定义控制台应用程序的入口点。
//
/*
http协议, socket, 正则表达式
所有的 URL 全都放在 urls 这个队列中.
首先要 push 一个根 URL.
之后爬虫就行动了.
过程大概是这样:
从urls取出一个URL => 读出URL网页全部内容 => 分析所有URL => 把URL放进 urls => 从 urls 弹出一个 URL.
URL 是 host + get.因此需要一个 binaryString 把它切开.
效率也不是很快, 1分钟大概4W条URL, 去掉重复至少也有好几千吧.
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

void GoURL(const string &, int);                                              //分割URL:HOST+GET
inline pair<string, string> binaryString(const string&, const string&);       //连接socket
inline SOCKET connect(const string &);                                        //
inline string getIpByHostName(const string &);                                //用域名或主机名获取IP地址
inline bool sendRequest(SOCKET, const string &, const string &);
inline string recvRequest(SOCKET);
inline void extUrl(const string &, queue<string>&);

int _tmain(int argc, _TCHAR* argv[])
{ 
	//(1)MAKEWORD 宏，将两个8babyte型合并成一个word型，一个在高8位，一个在低8位 (2)不是C++标准的类型，是微软SDK中的类型，WORD的意思为字，是2byte的无符号整数，表示范围0~65535.
	//WSADATA，一种分子结构。这个结构被用来存储被WSAStartup函数调用后返回的Windows Sockets数据。它包含Winsock.dll执行的数据。
	WSADATA wsadata;   

	//WSAStartup对winsock DLL（动态链接库文件）进行初始化，协商Winsock的版本支持，并分配必要的资源。 
	WSAStartup(MAKEWORD(2, 0), &wsadata);
	
	//网络爬虫处理 
	GoURL("www.pcauto.com.cn", 200);

	//清空winsock
	WSACleanup();

	return 0;
}

void GoURL(const string &url, int count)
{
	queue<string> urls;   //定义queue对象urls，sring的队列
	urls.push(url);		  //入队
	//auto：用来声明自动变量。它是存储类型标识符，表明变量（自动）具有本地范围，块范围的变量声明（如for循环体内的变量声明）默认为auto存储类型。
	//其实大多普通声明方式声明的变量都是auto变量, 他们不需要明确指定auto关键字，默认就是auto的了。
	//auto变量在离开作用域是会变程序自动释放，不会发生内存溢出情况（除了包含指针的类）。使用auto变量的优势是不需要考虑去变量是否被释放，比较安全吧。
	for (auto i = 0; i != count; i++)
	{
		if (!urls.empty())
		{
			auto &url = urls.front();                //提取url
			auto pair = binaryString(url, "/");
			auto sock = connect(pair.first);         //建立套接字,连接host
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

//分割URL:HOST+GET
inline pair<string, string> binaryString(const string&str, const string&dilme)
{
	//pair<T1,T2> P1; 创建空的pair对象P1，两个元素（first成员和second成员）分别为T1和T2类型
	pair<string, string> result(str, "");
	auto pos = str.find(dilme);
	//查找字符串a是否包含子串b,
	//不是用strA.find(strB) > 0而是strA.find(strB) != string:npos(不包含，则返回一个很大的值，等于string::npos；若包含，则返回子字符串所在的index，不等于string::npos)
	//如果包含"/"
	if (pos != string::npos)
	{
		//result的first成员
		result.first = str.substr(0, pos);
		//result的first成员
		result.second = str.substr(pos + dilme.size());
	}
	return result;
}

//连接Socket
inline SOCKET connect(const string &hostName)
{
	//字符串ip地址
	auto ip = getIpByHostName(hostName);
	if (ip.empty())
	{
		return 0;
	}
	//SOCKET Socket（int af，int type,int protocol）;
	//调用成功则返回SOCKET类型的套接字句柄，否则返回INVALID_SOCKET错误
	//af：协议簇，在window socket中，只能为AF_INET(网际协议)
	auto sock = socket(AF_INET, SOCK_STREAM, 0);     
	if (sock == INVALID_SOCKET)
	{
		return 0;
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
	SOCKADDR_IN addr;
	addr.sin_family = AF_INET;          	//协议簇
	addr.sin_port = htons(PORT);            //htons用来将主机字节顺序转换为网络字节顺序(to network short),http默认80
	//string类对象的成员函数c_str()把string 对象转换成c中的字符串样式。
	addr.sin_addr.s_addr = inet_addr(ip.c_str());       //inet_addr()的功能是将一个点分十进制的IP转换成一个长整数型数（u_long类型）
	//建立套接字连接
	if (connect(sock, (const sockaddr*)&addr, sizeof(SOCKADDR_IN)) == SOCKET_ERROR)
	{
		return 0;
	}
	return sock;
}

//用域名或主机名获取IP地址
inline string getIpByHostName(const string &hostName)
{
	//gethostbyname()返回对应于给定主机名的包含主机名字和地址信息的hostent结构指针,用域名或主机名获取IP地址
	hostent* phost = gethostbyname(hostName.c_str());
	//inet_ntoa():将一个十进制网络字节序转换为点分十进制IP格式的字符串。
	//如果得到了一个ip包，想要打印出它的ip地址（用点分十进制） ，则要用到inet_ntoa，（即network to ascii）
	return phost ? inet_ntoa(*(in_addr *)phost->h_addr_list[0]) : "";
}

//发送HTTP请求（GET）
inline bool sendRequest(SOCKET sock, const string &host, const string &get)
{
	string http = "GET " + get + " HTTP/1.1\r\n" + "HOST: " + host + "\r\n" + "Connection: close\r\n\r\n";
	return http.size() == send(sock, &http[0], http.size(), 0);
}

//HTTP接收响应
inline string recvRequest(SOCKET sock)
{
	static timeval wait = { 2, 0 };
	static auto buffer = string(2048 * 100, '\0');
	auto len = 0, reclen = 0;
	//读取所有的HTTP协议响应，并把它存入buffer
	do
	{
		fd_set fd = { 0 };
		FD_SET(sock, &fd);
		reclen = 0;
		/*
		 select函数用于决定一个或者多个套接字的状态。对于每一个套接字，调用者可以请求读、写或者错误状态信息。
		 一个请求给定状态的套接字集由fd_set结构体指定。 在fd_set结构体中的套接字必须和单个服务提供者联系在一起。
		 基于此，如果WSAPROTOCOL_INFO结构体中有相同的providerId值，套接字被认为来自同一个服务提供者。直到返回，结构体更新去反映满足指定条件套接字子集。
		 select函数返回满足条件的套接字个数。fd_set集合可以通过一些宏手动操作。这些宏也适合伯克利套接字，但是它们的机理是根本不同的
		 int select(int nfds,fd_set FAR* readfds,fd_set FAR* writefds,fd_set FAR* exceptfds,const struct timeval FAR* timeout);
		 nfds：可忽略
		 readfds：指向套接字集合，是否可读
		 writefds：指向套接字集合，是否可写
		 exceptfds：指向套接字集合，是否出错检测
		 timeout：timeval结构类型结构指针
		*/
		if (select(0, &fd, nullptr, nullptr, &wait) > 0)
		{
			/*
			recv(SOCKET s,const char FAR* buf,int len，int flag);
			s：接收端的套接字描述符
			buf：接收数据的缓冲区地址
			len：接收数据缓冲区长度
			flags：处理数据方式
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

//URL入栈
inline void extUrl(const string &buffer, queue<string>&urlQueue)
{
	if (buffer.empty())
	{
		return;
	}
	smatch result;    
	auto curIter = buffer.begin();   
	auto endIter = buffer.end();
	//在regex_search函数中，会将找到匹配结果保存到一个smatch类中
	while (regex_search(curIter, endIter, result, regex("href=\"(https?:)?//\\S+\"")))
	{
		//把匹配结果压入栈中
		urlQueue.push(regex_replace(result[0].str(), regex("href=\"(https?:)?//(\\S+)\""), "$2"));
		curIter = result[0].second;
	}
}