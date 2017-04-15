// 大数据-访问最多IP.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
//#include <atlstr.h>  
//#include <hash_map>  
//#include <string>  
//#include <iostream>  
//#include <time.h>  
//using namespace std;
//using namespace stdext;
//using namespace ATL;
//
//#define FileNum (1024)  
//
////typedef CString ClassA;  
//class MyString : public CString{
//public:
//	inline size_t hash_value(const MyString& str) const
//	{
//		size_t value = _HASH_SEED;
//		size_t size = str.GetLength();
//		if (size > 0) {
//			size_t temp = (size / 16) + 1;
//			size -= temp;
//			for (size_t idx = 0; idx <= size; idx += temp) {
//				value += (size_t)str[(int)idx];
//			}
//		}
//		return(value);
//	}
//};
//
//template<class _Tkey>
//class MyHashCompare : public hash_compare<_Tkey>
//{
//public:
//	size_t operator()(const _Tkey& _Key) const
//	{
//		return(_Key.hash_value(_Key));//此处需要注意更改   
//	}
//
//	bool operator()(const _Tkey& _Keyval1, const _Tkey& _Keyval2) const
//	{
//		return (comp(_Keyval1, _Keyval2));
//	}
//};
//
//typedef pair<MyString, int> MyPair;
//MyPair MaxValue(hash_map<MyString, int, MyHashCompare<MyString>> ipMap)//求每个hash_map最大值  
//{
//	hash_map<MyString, int>::iterator iter = ipMap.begin();
//	MyString maxStr = (*iter).first;
//	int maxNum = (*iter).second;
//	iter++;
//	int countNum = 0;
//	//int* intArray=new int[10*1024*1024];//10M  
//	while (iter != ipMap.end())
//	{
//		if (maxNum<(*iter).second)
//		{
//			maxStr = (*iter).first;
//			maxNum = (*iter).second;
//		}
//		iter++;
//	}
//	//int maxNum=MaxValue(intArray,countNum);  
//	//delete[] intArray;  
//	MyPair tempPair;
//	tempPair.first = maxStr;
//	tempPair.second = maxNum;
//	return tempPair;
//}
//
//MyPair MaxKey(MyPair* tempPairs, int num)
//{
//	MyPair tempPair;
//	MyString maxStr = tempPairs[0].first;
//	int maxNum = tempPairs[0].second;
//	for (int i = 1; i<num; i++)
//	{
//		if (maxNum<tempPairs[i].second)
//		{
//			maxStr = tempPairs[i].first;
//			maxNum = tempPairs[i].second;
//		}
//	}
//	tempPair.first = maxStr;
//	tempPair.second = maxNum;
//	return tempPair;
//}
//
//int main()
//{
//	hash_map<MyString, int, MyHashCompare<MyString>> ipMap[FileNum];
//	FILE* fpData[FileNum];
//	MyPair tempPairs[FileNum];
//	int iFileNum = 0;
//
//	//FILE* fp=fopen("data.txt","wb");  
//	//生成5个文件  
//	MyString strTemp1, strTemp2;
//	for (int i = 101; i <= 200; i++)
//	{
//		//strTemp.Format("192.168.0.%d-%d|",i,rand());  
//		if ((i - 1) % 20 == 0)
//		{
//			MyString strTemp;
//			strTemp.Format(_T("data%d.txt"), ++iFileNum);
//			fpData[iFileNum - 1] = fopen((LPSTR)(LPCTSTR)strTemp, "wb");//fpData[0]==data1.txt  
//		}
//		strTemp1.Format(_T("192.168.0.%d\r\n"), i);
//		fwrite(strTemp1, 1, strTemp1.GetLength(), fpData[iFileNum - 1]);
//		strTemp2.Format(_T("%d\r\n"), rand());
//		if (i % 20 == 0)
//		{
//			strTemp2.Format(_T("%d"), rand());//最后一行不换行  
//		}
//		fwrite(strTemp2, 1, strTemp2.GetLength(), fpData[iFileNum - 1]);
//		//hmap.insert(MyPair(strTemp,i+1));  
//	}
//	for (int j = 0; j<iFileNum; j++)
//	{
//		fclose(fpData[j]);
//	}
//
//	//5个文件存入hash_map  
//	for (int k = 0; k<iFileNum; k++)
//	{
//		MyString strTemp;
//		strTemp.Format(_T("data%d.txt"), k + 1);
//		fpData[k] = fopen((LPSTR)(LPCTSTR)strTemp, "rb");//fpData[0]==data1.txt  
//		while (!feof(fpData[k]))
//		{
//			char tempIP[25];
//			MyString strIP;
//			MyString strNum;
//			//fgets((LPSTR)(LPCTSTR)strIP,25,fpData[k]);//读取一行，第二个参数为最大长度  
//			fgets(tempIP, 25, fpData[k]);
//			strIP.Format(_T("%s"), tempIP);//此处strIP=tempIP会出错，MyString为CString的子类  
//			strIP.TrimRight();//去除右边处空格（不去除strIP包含空行字符，会被strNum覆盖）  
//			cout << "strIP==" << strIP << endl;
//			fgets((LPSTR)(LPCTSTR)strNum, 15, fpData[k]);
//			cout << "strNum==" << _ttoi(strNum) << endl;
//			ipMap[k].insert(MyPair(strIP, _ttoi(strNum)));
//			//cout<< "strIP==" << MyPair(strIP,_ttoi(strNum)).first<< endl;  
//			//cout<< "strNum==" << MyPair(strIP,_ttoi(strNum)).second<< endl;  
//		}
//	}
//
//	//int intArray[5]={8,10,6,5,3};  
//	//cout << "Max==" << MaxValue(intArray,5)<< endl;  
//
//	for (int k = 0; k<iFileNum; k++){
//		tempPairs[k].first = MaxValue(ipMap[k]).first;
//		tempPairs[k].second = MaxValue(ipMap[k]).second;
//		cout << "Max==" << tempPairs[k].first << "--" << tempPairs[k].second << endl;
//	}
//	MyPair m_maxPair = MaxKey(tempPairs, iFileNum);
//	cout << "Max==" << m_maxPair.first << "--" << m_maxPair.second << endl;//访问次数最多的IP  
//	for (int j = 0; j<iFileNum; j++)
//	{
//		fclose(fpData[j]);
//	}
//
//	system("pause");
//	return 0;
//}