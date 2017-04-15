/*
��1������ɢ�к����������ļ��е�IP��ַɢ�е����ɸ��ļ��С���ͬ��IP��ַ�϶���ͬһ���ļ��С�
��2������ÿ��С�ļ����ҵ����ļ��г��ִ��������Ǹ�IP����¼��IP��ַ�ͳ��ִ�����������hash_map��IP��ַΪ��ֵ�����ִ���Ϊ��ֵ��
��3�����ڣ�2�������ҵ���IP��ַ�����ִ����ۺ��������ҵ���ЩIP��ַ�г��ִ��������Ǹ�IP��
hash_map ������C++��STL(��׼ģ���)��hash_map����
*/

#include "stdafx.h"
#pragma warning(disable:4786) //VC6.0�� ���Ծ���  
#include <fstream>           //�ļ���
#include <iostream>  
#include <map>              //map��������
#include <string>  
#include <ctime>  
using namespace std;

const unsigned N = 4000000;      //���������IP��ַ��  
const unsigned FILE_NUM = 16;      //������С�ļ�����  
const unsigned HASH_SHIFT = 28;    //ɢ��ֵ��λ����  

inline unsigned HashInt(unsigned value); //������ɢ�е�0��FILE_NUM֮��  
bool ProduceIP(string fileName);                //�������IP��ַ��������32λ�޷�����  
bool DecomposeFile(string fileName);     //�ֶ���֮�������ļ���Ϊ���ɸ�С�ļ�  
bool FindTargetIP(unsigned result[2]);      //�ҵ����ִ�������IP  

int main()
{
	unsigned start, end;      //��¼�ܵ�����ʱ��  
	unsigned start1, end1;  //�������ļ���ʱ��  
	unsigned start2, end2;  //�ֽ���ļ���ʱ��  
	unsigned start3, end3;  //�ҳ���IP��������ʱ��  

	string name = "IP.bin";       //���ļ�  
	unsigned result[2] = { 0, 0 };   //������  

	start = clock();           //total run time, start
	start1 = clock();          //produce IP run time, start
	//�����������IP  
	if (ProduceIP(name) == false)
		return 1;
	end1 = clock();            //procuce IP run time, end

	start2 = clock();          //Decompose File run time, start
	//�ֶ���֮  
	if (DecomposeFile(name) == false)
		return 1;
	end2 = clock();           //Decompose File run time, end

	start3 = clock();         //Find Target IP run time, start
	//�ҵ����ִ�������IP  
	if (FindTargetIP(result) == false)
		return 1;
	end3 = clock();          //Find Target IP run time, end
	end = clock();            //total run time, end

	//��ӡ���  
	cout << "total run time : " << (end - start) / 1000.0 << endl;
	cout << "ProduceIP() run time : " << (end1 - start1) / 1000.0 << endl;
	cout << "DecomposeFile() run time : " << (end2 - start2) / 1000.0 << endl;
	cout << "FindTargetIP() run time : " << (end3 - start3) / 1000.0 << endl;
	cout << "IP : " << (result[0] >> 24) << '.' << ((result[0] & 0x00ff0000) >> 16) << '.';
	cout << ((result[0] & 0x0000ff00) >> 8) << '.' << ((result[0] & 0x000000ff)) << endl;
	cout << "appear time : " << result[1] << endl;
	return 0;
}

//������ɢ�е�0��FILE_NUM֮��  
inline unsigned HashInt(unsigned value)
{
	//쳲�����(Fibonacci)ɢ�з� hash_key=(value * M) >> S;  
	//value��16λ������M = 40503   
	//value��32λ������M = 2654435769   
	//value��64λ������M = 11400714819323198485  
	//S��Ͱ�ĸ������������Ͱ�ĸ���Ϊ16����ôSΪ28  
	//����32λ������S=32-log2(Ͱ�ĸ���)  
	return (value * 2654435769) >> HASH_SHIFT;
}

//�������IP��ַ ������32λ�޷�����  
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
		//����һ������������ģ��IP��ַ,unsigned longȡֵ��Χ��2^32-1
		//rand() % 256 ���ֵΪ255/������11111111
		unsigned x = ((rand() % 256) << 24) | ((rand() % 256) << 16) | ((rand() % 256) << 8) | (rand() % 256);
		//������д��outFile.write((char*)&x,sizeof(unsigned));
		outFile.write((char*)&x, sizeof(unsigned));
	}
	return true;
}

//�ֶ���֮ �����ļ���Ϊ���ɸ�С�ļ�  
bool DecomposeFile(string fileName)
{
	ofstream outFiles[FILE_NUM];
	int i;
	for (i = 0; i<FILE_NUM; i++)
	{
		//С�ļ�������  
		char buffer[10];
		string name = "tmp";
		//char*itoa(int value, char*string, int radix);
		//������int value ��ת����������char *string ת���󴢴���ַ����飬int radix ת��������
		//���ܣ����������͵�����ת��Ϊ�ַ�������<stdlib.h>����֮���෴���ܵĺ�����atoi
		_itoa(i, buffer, 10);
		name = name + buffer + ".bin";

		//��С�ļ� 
		//const char *c_str();c_str()��������һ��ָ������C�ַ�����ָ�볣��
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
		//ɢ�е�����С�ļ���  
		unsigned int value = 0;
		if (inFile.read((char*)&value, sizeof(unsigned)))
		{
			int x = HashInt(value);
			outFiles[HashInt(value)].write((char*)&value, sizeof(unsigned));
		}
	}
	//�ر��ļ�  
	inFile.close();
	for (i = 0; i<FILE_NUM; i++)
		outFiles[i].close();
	return true;
}

//�ҵ����ִ�������IP  
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
		//����ÿ��С�ļ�  
		ifstream inFile;
		inFile.open(name.c_str(), ios::binary);
		if (!inFile)
		{
			cerr << "error: unable to open input file :" << name << endl;
			return false;
		}
		//���Ĵ��룬����STL��û��hash_map����map������  
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