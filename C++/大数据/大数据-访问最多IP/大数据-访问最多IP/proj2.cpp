/*
����һ��֮��ĳ��IP���ʰٶȵĴ���������40�ڴ�,����ʴ���������unsigned��ʾ.������ͳ�Ƴ�ÿ��IP��ַ���ֵĴ���,  ���ɵõ����ʴ�������IP��ַ.
IP��ַ��32λ�Ķ�������,���Թ���N=2^32=4G����ͬ��IP��ַ, ����һ��unsigned count[N];������,����ͳ�Ƴ�ÿ��IP�ķ��ʴ���,
��sizeof(count) == 4G*4=16G, ԶԶ������32λ�������֧�ֵ��ڴ��С,��˲���ֱ�Ӵ����������.������û��ַ�����������.
��������ʹ�õ��ڴ���512M,  512M/4=128M ��512M�ڴ����ͳ��128M����ͬ��IP��ַ�ķ��ʴ���.��N/128M =4G/128M = 32 ,
����ֻҪ��IP��ַ���ֳ�32����ͬ������,�ֱ�ͳ�Ƴ�ÿ�������з��ʴ�������IP, Ȼ��Ϳ��Լ��������IP��ַ�з��ʴ�������IP��.
��Ϊ2^5=32, ���Կ��԰�IP��ַ�����5λ��Ϊ������, ʣ�µ�27Ϊ��Ϊ�����ڵ�ֵ,����32����ʱ�ļ�,����32������,����ͬ�����IP��ַ���浽ͬһ����ʱ�ļ���.
*/
/*
ip1=0x1f4e2342
ip1�ĸ�5λ��id1  =  ip1 >>27 = 0x11 = 3
ip1������27λ��value1  =  ip1 &0x07ffffff = 0x074e2342
���԰� value1 ������tmp3�ļ���.
��id1��value1���Ի�ԭ��ip1, �� ip1 =(id1<<27)|value1
��������ķ������Եõ�32����ʱ�ļ�,ÿ����ʱ�ļ��е�IP��ַ��ȡֵ��Χ����[0-128M),
��˿���ͳ�Ƴ�ÿ��IP��ַ�ķ��ʴ���.�Ӷ��ҵ����ʴ�������IP��ַ
*/
/*
�㷨˼�룺�ֶ���֮+Hash
1��IP��ַ�����2^32=4G��ȡֵ��������Բ�����ȫ���ص��ڴ��д���
2�����Կ��ǲ��÷ֶ���֮��˼�룬����IP��ַ��Hash(IP) % 1024ֵ���Ѻ���IP��־�ֱ�洢��1024��С�ļ��У�������ÿ��С�ļ�������4MB��IP��ַ��
�������һ��Ϊʲô��Hash(IP) % 1024ֵ��������ã���ֱ�ӷ���Ļ������ܻ��������һ������������и�IP��ÿ��С�ļ��ж����ڣ��������IP����һ�����Ǹ�С�ļ������������ģ���ô���տ���ѡ��Ľ���������⣬������������Hash(IP)%1024ֵ�������Ļ���ͨ������IP��Hashֵ����ͬIP�϶���ŵ�һ���ļ��У���Ȼ�˲�ͬ��IP��HashֵҲ������ͬ���ʹ���һ��С�ļ��С�
3������ÿһ��С�ļ������Թ���һ��IPΪkey�����ֵĴ���Ϊvalue��Hash Map��ͬʱ��¼��ǰ���ִ��������Ǹ�IP��ַ��
4�����Եõ�1024��С�ļ��еĳ��ִ��������Ǹ�IP�������ݳ���������㷨�ó������ϳ��ִ�������IP��
*/
#include "stdafx.h"
//#include <fstream>   
//#include <iostream>   
//#include <ctime>   
//
//using namespace std;
//#define N 32           //��ʱ�ļ���   
//
//#define ID(x)  (x>>27)                 //x��Ӧ���ļ����   
//#define VALUE(x) (x&0x07ffffff)        //x���ļ��б����ֵ   
//#define MAKE_IP(x,y)  ((x<<27)|y)      //���ļ���ź�ֵ�õ�IP��ַ.   
//
//#define MEM_SIZE  128*1024*1024       //������ڴ�Ĵ�СΪ MEM_SIZE*sizeof(unsigned)      
//
//char* data_path = "D:/test/ip.dat";        //ip����   
//
////����n�����IP��ַ   
//void make_data(const int& n)
//{
//	ofstream out(data_path, ios::out | ios::binary);
//	srand((unsigned)(time(NULL)));
//	if (out)
//	{
//		//����n��ѭ��
//		for (int i = 0; i<n; ++i)
//		{
//			unsigned val = unsigned(rand());
//			val = (val << 24) | val;              //����unsigned���͵������   
//
//			out.write((char *)&val, sizeof(unsigned));
//		}
//	}
//}
//
////�ҵ����ʴ�������ip��ַ   
//int main()
//{
//	//make_data(100);     //    
//	make_data(100000000);       //���������õ�IP����   
//	fstream arr[N];
//
//	for (int i = 0; i<N; ++i)                 //����N����ʱ�ļ�   
//	{
//		char tmp_path[128];     //��ʱ�ļ�·��   
//		/*
//		�������ܣ��Ѹ�ʽ��������д��ĳ���ַ���
//		char* who = "I";
//		char* whom = "CSDN";
//		sprintf(s, "%s love %s.", who, whom); //������"I love CSDN. "  ���ַ���д��s��
//		sprintf(s, "%10.3f", 3.1415626); //������" 3.142"
//		*/
//		sprintf(tmp_path, "D:/test/tmp%d.dat", i);
//		arr[i].open(tmp_path, ios::trunc | ios::in | ios::out | ios::binary);  //�򿪵�i���ļ�   
//
//		if (!arr[i])
//		{
//			cout << "open file" << i << "error" << endl;
//		}
//	}
//
//	ifstream infile(data_path, ios::in | ios::binary);   //��������õ�IP����   
//	unsigned data;
//
//	while (infile.read((char*)(&data), sizeof(data)))
//	{
//		unsigned val = VALUE(data);
//		int key = ID(data);
//		arr[ID(data)].write((char*)(&val), sizeof(val));           //���浽��ʱ�ļ�����   
//	}
//
//	for (unsigned i = 0; i<N; ++i)
//	{
//		//C++��seekg�����Ĺ����ǣ����������ļ������ļ���ָ��λ�á�
//		arr[i].seekg(0);
//	}
//	unsigned max_ip = 0;    //���ִ�������ip��ַ   
//	unsigned max_times = 0;     //���ֻ���ֵĴ���   
//
//	//����512M�ڴ�,����ͳ��ÿ�������ֵĴ���   
//	unsigned *count = new unsigned[MEM_SIZE];
//
//	for (unsigned i = 0; i<N; ++i)
//	{
//		memset(count, 0, sizeof(unsigned)*MEM_SIZE);
//
//		//ͳ��ÿ����ʱ�ļ����в�ͬ���ֳ��ֵĴ���   
//		unsigned data;
//		while (arr[i].read((char*)(&data), sizeof(unsigned)))
//		{
//			++count[data];
//		}
//
//		//�ҳ����ִ�������IP��ַ   
//		for (unsigned j = 0; j<MEM_SIZE; ++j)
//		{
//			if (max_times<count[j])
//			{
//				max_times = count[j];
//				max_ip = MAKE_IP(i, j);        // �ָ���ԭip��ַ.   
//			}
//		}
//	}
//	delete[] count;
//	unsigned char *result = (unsigned char *)(&max_ip);
//	printf("���ִ�������IPΪ:%d.%d.%d.%d,������%d��",
//		result[0], result[1], result[2], result[3], max_times);
//}