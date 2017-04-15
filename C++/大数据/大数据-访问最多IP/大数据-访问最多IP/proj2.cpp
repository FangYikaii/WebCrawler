/*
假设一天之内某个IP访问百度的次数不超过40亿次,则访问次数可以用unsigned表示.用数组统计出每个IP地址出现的次数,  即可得到访问次数最大的IP地址.
IP地址是32位的二进制数,所以共有N=2^32=4G个不同的IP地址, 创建一个unsigned count[N];的数组,即可统计出每个IP的访问次数,
而sizeof(count) == 4G*4=16G, 远远超过了32位计算机所支持的内存大小,因此不能直接创建这个数组.下面采用划分法解决这个问题.
假设允许使用的内存是512M,  512M/4=128M 即512M内存可以统计128M个不同的IP地址的访问次数.而N/128M =4G/128M = 32 ,
所以只要把IP地址划分成32个不同的区间,分别统计出每个区间中访问次数最大的IP, 然后就可以计算出所有IP地址中访问次数最大的IP了.
因为2^5=32, 所以可以把IP地址的最高5位作为区间编号, 剩下的27为作为区间内的值,建立32个临时文件,代表32个区间,把相同区间的IP地址保存到同一的临时文件中.
*/
/*
ip1=0x1f4e2342
ip1的高5位是id1  =  ip1 >>27 = 0x11 = 3
ip1的其余27位是value1  =  ip1 &0x07ffffff = 0x074e2342
所以把 value1 保存在tmp3文件中.
由id1和value1可以还原成ip1, 即 ip1 =(id1<<27)|value1
按照上面的方法可以得到32个临时文件,每个临时文件中的IP地址的取值范围属于[0-128M),
因此可以统计出每个IP地址的访问次数.从而找到访问次数最大的IP地址
*/
/*
算法思想：分而治之+Hash
1、IP地址最多有2^32=4G种取值情况，所以不能完全加载到内存中处理；
2、可以考虑采用分而治之的思想，按照IP地址的Hash(IP) % 1024值，把海量IP日志分别存储到1024个小文件中，这样，每个小文件最多包含4MB个IP地址；
这里解释一下为什么用Hash(IP) % 1024值，如果不用，而直接分类的话，可能会出现这样一种情况，就是有个IP在每个小文件中都存在，而且这个IP并不一定在那个小文件中是数量最多的，那么最终可能选择的结果会有问题，所以这里用了Hash(IP)%1024值，这样的话，通过计算IP的Hash值，相同IP肯定会放到一个文件中，当然了不同的IP的Hash值也可能相同，就存在一个小文件中。
3、对于每一个小文件，可以构建一个IP为key，出现的次数为value的Hash Map，同时记录当前出现次数最多的那个IP地址；
4、可以得到1024个小文件中的出现次数最多的那个IP，再依据常规的排序算法得出总体上出现次数最多的IP。
*/
#include "stdafx.h"
//#include <fstream>   
//#include <iostream>   
//#include <ctime>   
//
//using namespace std;
//#define N 32           //临时文件数   
//
//#define ID(x)  (x>>27)                 //x对应的文件编号   
//#define VALUE(x) (x&0x07ffffff)        //x在文件中保存的值   
//#define MAKE_IP(x,y)  ((x<<27)|y)      //由文件编号和值得到IP地址.   
//
//#define MEM_SIZE  128*1024*1024       //需分配内存的大小为 MEM_SIZE*sizeof(unsigned)      
//
//char* data_path = "D:/test/ip.dat";        //ip数据   
//
////产生n个随机IP地址   
//void make_data(const int& n)
//{
//	ofstream out(data_path, ios::out | ios::binary);
//	srand((unsigned)(time(NULL)));
//	if (out)
//	{
//		//产生n次循环
//		for (int i = 0; i<n; ++i)
//		{
//			unsigned val = unsigned(rand());
//			val = (val << 24) | val;              //产生unsigned类型的随机数   
//
//			out.write((char *)&val, sizeof(unsigned));
//		}
//	}
//}
//
////找到访问次数最大的ip地址   
//int main()
//{
//	//make_data(100);     //    
//	make_data(100000000);       //产生测试用的IP数据   
//	fstream arr[N];
//
//	for (int i = 0; i<N; ++i)                 //创建N个临时文件   
//	{
//		char tmp_path[128];     //临时文件路径   
//		/*
//		函数功能：把格式化的数据写入某个字符串
//		char* who = "I";
//		char* whom = "CSDN";
//		sprintf(s, "%s love %s.", who, whom); //产生："I love CSDN. "  这字符串写到s中
//		sprintf(s, "%10.3f", 3.1415626); //产生：" 3.142"
//		*/
//		sprintf(tmp_path, "D:/test/tmp%d.dat", i);
//		arr[i].open(tmp_path, ios::trunc | ios::in | ios::out | ios::binary);  //打开第i个文件   
//
//		if (!arr[i])
//		{
//			cout << "open file" << i << "error" << endl;
//		}
//	}
//
//	ifstream infile(data_path, ios::in | ios::binary);   //读入测试用的IP数据   
//	unsigned data;
//
//	while (infile.read((char*)(&data), sizeof(data)))
//	{
//		unsigned val = VALUE(data);
//		int key = ID(data);
//		arr[ID(data)].write((char*)(&val), sizeof(val));           //保存到临时文件件中   
//	}
//
//	for (unsigned i = 0; i<N; ++i)
//	{
//		//C++中seekg函数的功能是：设置输入文件流的文件流指针位置。
//		arr[i].seekg(0);
//	}
//	unsigned max_ip = 0;    //出现次数最多的ip地址   
//	unsigned max_times = 0;     //最大只出现的次数   
//
//	//分配512M内存,用于统计每个数出现的次数   
//	unsigned *count = new unsigned[MEM_SIZE];
//
//	for (unsigned i = 0; i<N; ++i)
//	{
//		memset(count, 0, sizeof(unsigned)*MEM_SIZE);
//
//		//统计每个临时文件件中不同数字出现的次数   
//		unsigned data;
//		while (arr[i].read((char*)(&data), sizeof(unsigned)))
//		{
//			++count[data];
//		}
//
//		//找出出现次数最多的IP地址   
//		for (unsigned j = 0; j<MEM_SIZE; ++j)
//		{
//			if (max_times<count[j])
//			{
//				max_times = count[j];
//				max_ip = MAKE_IP(i, j);        // 恢复成原ip地址.   
//			}
//		}
//	}
//	delete[] count;
//	unsigned char *result = (unsigned char *)(&max_ip);
//	printf("出现次数最多的IP为:%d.%d.%d.%d,共出现%d次",
//		result[0], result[1], result[2], result[3], max_times);
//}