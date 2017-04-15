/*
（1）利用散列函数，将大文件中的IP地址散列到若干个文件中。相同的IP地址肯定在同一个文件中。
（2）处理每个小文件，找到该文件中出现次数最多的那个IP，记录下IP地址和出现次数。可以用hash_map，IP地址为键值、出现次数为数值。
（3）将第（2）步中找到的IP地址及出现次数综合起来，找到这些IP地址中出现次数最多的那个IP。
hash_map 利用了C++的STL(标准模板库)的hash_map容器
*/

#include "stdafx.h"
#pragma warning(disable:4786) //VC6.0中 忽略警告  
#include <fstream>           //文件流
#include <iostream>  
#include <map>              //map关联容器
#include <string>  
#include <ctime>  
using namespace std;

const unsigned N = 4000000;      //随机产生的IP地址数  
const unsigned FILE_NUM = 16;      //产生的小文件个数  
const unsigned HASH_SHIFT = 28;    //散列值的位移量  

inline unsigned HashInt(unsigned value); //将整数散列到0到FILE_NUM之间  
bool ProduceIP(string fileName);                //随机产生IP地址，看成是32位无符号数  
bool DecomposeFile(string fileName);     //分而治之，将大文件分为若干个小文件  
bool FindTargetIP(unsigned result[2]);      //找到出现次数最多的IP  

int main()
{
	unsigned start, end;      //记录总的运行时间  
	unsigned start1, end1;  //产生大文件的时间  
	unsigned start2, end2;  //分解大文件的时间  
	unsigned start3, end3;  //找出现IP次数最多的时间  

	string name = "IP.bin";       //大文件  
	unsigned result[2] = { 0, 0 };   //保存结果  

	start = clock();           //total run time, start
	start1 = clock();          //produce IP run time, start
	//随机产生大量IP  
	if (ProduceIP(name) == false)
		return 1;
	end1 = clock();            //procuce IP run time, end

	start2 = clock();          //Decompose File run time, start
	//分而治之  
	if (DecomposeFile(name) == false)
		return 1;
	end2 = clock();           //Decompose File run time, end

	start3 = clock();         //Find Target IP run time, start
	//找到出现次数最多的IP  
	if (FindTargetIP(result) == false)
		return 1;
	end3 = clock();          //Find Target IP run time, end
	end = clock();            //total run time, end

	//打印结果  
	cout << "total run time : " << (end - start) / 1000.0 << endl;
	cout << "ProduceIP() run time : " << (end1 - start1) / 1000.0 << endl;
	cout << "DecomposeFile() run time : " << (end2 - start2) / 1000.0 << endl;
	cout << "FindTargetIP() run time : " << (end3 - start3) / 1000.0 << endl;
	cout << "IP : " << (result[0] >> 24) << '.' << ((result[0] & 0x00ff0000) >> 16) << '.';
	cout << ((result[0] & 0x0000ff00) >> 8) << '.' << ((result[0] & 0x000000ff)) << endl;
	cout << "appear time : " << result[1] << endl;
	return 0;
}

//将整数散列到0到FILE_NUM之间  
inline unsigned HashInt(unsigned value)
{
	//斐波那契(Fibonacci)散列法 hash_key=(value * M) >> S;  
	//value是16位整数，M = 40503   
	//value是32位整数，M = 2654435769   
	//value是64位整数，M = 11400714819323198485  
	//S与桶的个数有数，如果桶的个数为16，那么S为28  
	//对于32位整数，S=32-log2(桶的个数)  
	return (value * 2654435769) >> HASH_SHIFT;
}

//随机产生IP地址 看成是32位无符号数  
bool ProduceIP(string fileName)
{
	ofstream outFile(fileName.c_str(), ios::binary);
	if (!outFile)
	{
		cerr << "error: unable to open output file : " << fileName << endl;
		return false;
	}
	srand(time(0));
	for (unsigned i = 0; i<N; i++)
	{
		//产生一个大整数用来模拟IP地址,unsigned long取值范围：2^32-1
		//rand() % 256 最大值为255/二进制11111111
		unsigned x = ((rand() % 256) << 24) | ((rand() % 256) << 16) | ((rand() % 256) << 8) | (rand() % 256);
		//单个读写：outFile.write((char*)&x,sizeof(unsigned));
		outFile.write((char*)&x, sizeof(unsigned));
	}
	return true;
}

//分而治之 将大文件分为若干个小文件  
bool DecomposeFile(string fileName)
{
	ofstream outFiles[FILE_NUM];
	int i;
	for (i = 0; i<FILE_NUM; i++)
	{
		//小文件的名称  
		char buffer[10];
		string name = "tmp";
		//char*itoa(int value, char*string, int radix);
		//参数：int value 被转换的整数，char *string 转换后储存的字符数组，int radix 转换进制数
		//功能：将任意类型的数字转换为字符串。在<stdlib.h>中与之有相反功能的函数是atoi
		_itoa(i, buffer, 10);
		name = name + buffer + ".bin";

		//打开小文件 
		//const char *c_str();c_str()函数返回一个指向正规C字符串的指针常量
		outFiles[i].open(name.c_str(), ios::binary);
		if (!outFiles[i])
		{
			cerr << "error: unable to open output file :" << name << endl;
			return false;
		}
	}
	ifstream inFile(fileName.c_str(), ios::binary);
	while (inFile.good())
	{
		//散列到各个小文件中  
		unsigned int value = 0;
		if (inFile.read((char*)&value, sizeof(unsigned)))
		{
			int x = HashInt(value);
			outFiles[HashInt(value)].write((char*)&value, sizeof(unsigned));
		}
	}
	//关闭文件  
	inFile.close();
	for (i = 0; i<FILE_NUM; i++)
		outFiles[i].close();
	return true;
}

//找到出现次数最多的IP  
bool FindTargetIP(unsigned result[2])
{
	result[0] = 0;
	result[1] = 0;
	for (int i = 0; i<FILE_NUM; i++)
	{
		char buffer[10];
		string name = "tmp";
		_itoa(i, buffer, 10);
		name = name + buffer + ".bin";
		//处理每个小文件  
		ifstream inFile;
		inFile.open(name.c_str(), ios::binary);
		if (!inFile)
		{
			cerr << "error: unable to open input file :" << name << endl;
			return false;
		}
		//核心代码，由于STL中没有hash_map，用map来代替  
		map<unsigned, unsigned> ip_count;
		while (inFile.good())
		{
			unsigned key = 0;
			if (inFile.read((char*)&key, sizeof(unsigned)))
			{
				ip_count[key]++;
			}
		}
		map<unsigned, unsigned>::iterator it = ip_count.begin();
		for (; it != ip_count.end(); it++)
		{
			if (it->second>result[1])
			{
				result[0] = it->first;
				result[1] = it->second;
			}
		}
		inFile.close();
	}
	return true;
}