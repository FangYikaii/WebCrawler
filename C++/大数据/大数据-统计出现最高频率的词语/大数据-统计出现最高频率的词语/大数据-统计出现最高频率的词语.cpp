// ������-ͳ�Ƴ������Ƶ�ʵĴ���.cpp : �������̨Ӧ�ó������ڵ㡣
// ��һ��1G��С��һ���ļ�������ÿһ����һ���ʣ��ʵĴ�С������16�ֽڣ��ڴ����ƴ�С��1M������Ƶ����ߵ�100����.
/*
1G=2^30B=2^26�����ʣ� 1M=2^20B=2^16������
���ԣ��ֳ�2^10���ļ���ȡ2000���ļ�

��Ϊ��Ҷ������֪��1G���ļ���������1M���ڴ�ռ䴦��,��������Ҫ����1M�����������㡣
����ÿ�����ʶ�Ϊ16���ֽڣ���ô1M���ڴ���Դ�����ٸ������أ� 1M = 1024 KB = 1024 * 1024 B ��Ȼ��1M / 16B = 2^16�����ʡ�
��ô1G����ж��ٸ������أ� ��2^26�����ʣ�����ʵ����ԶԶ��ֹ��Щ����Ϊ�����ǰ�����󵥴ʳ�����ġ�
������Ҫ����1G�ĵ��ʷ���������������ļ��㣬���Էֳɴ���2^10���ļ������Ծͷֳ�2000���ļ��ɣ�

��ô���أ��������֣����ܼ򵥵İ��յ��ʵ�˳��Ȼ��ģ2000���֣���Ϊ�����п�����ͬ�ĵ��ʱ����ֵ���ͬ���ļ���ȥ�ˡ�
������ͳ�Ƹ�����ʱ�򱻵��ɵĲ�ͬ�ĵ��ʣ���Ϊ����û���������ڲ�ͬ�ļ�����ͬ���ʳ��ֵĴ�����Խ�ļ�����ӣ������ʹ����Ҫ�Ѳ�ͬ��ŵ�ͬһ�����ʻ��ֵ�ͬһ���ļ��У�

Ӧ��hashͳ�ưɡ��Ժ��������������Ȼ���أ����Ƕ�ÿ���ļ����зֱ�������key-value�ķ�������ÿ�����ʣ����յó�ÿ���ļ��а���ÿ�����ʺ͵��ʳ��ֵĴ�����
Ȼ���ٽ�����СΪ100��С���ѡ�һ�α����ļ����д�����û��Ū1G���ļ���Ū1M�ģ��򵥵�ʵ����һ�£�����ԭ����������ġ�
*/
#include "stdafx.h"
#include<iostream>  
#include<string>  
using namespace std;

#define FILE_NUM 10  
#define WORDLEN 30  
#define HASHLEN 7303                //����洢�ṹ��Ҳ��HASH����ӳ�����ܴﵽ�����ֵ���൱��һ��59��'z'�ĳ���

typedef struct node_no_space
{
	char *word;
	int count;
	struct node_no_space *next;
}node_no_space, *p_node_no_space;

typedef struct node_has_space
{
	char word[WORDLEN];
	int count;
	struct node_has_space *next;
}node_has_space, *p_node_has_space;

//����洢�ṹ
p_node_no_space bin[HASHLEN] = { NULL };         //����Hash_Map����СΪHASHLEN

//����������
void swap(int *a, int *b)
{
	int temp;
	temp = *a;
	*a = *b;
	*b = temp;
}

//��ϣ����
unsigned int hashx(char *p_word)
{
	unsigned int index = 0;
	while (*p_word)
	{
		index += index * 31 + *p_word;
		p_word++;
	}
	return index % HASHLEN;
}

//���word�Ƿ�Ϊ����
int trim_word(char *word) 
{
	int n = strlen(word) - 1;
	int i = 0;
	if (n < 0)
		return 0;
	//48-57:'0'-'9'   65-90:'A'-'Z'  97-123:'a'-'z'
	//��0-9��a-z��A-Zd��ASCII��
	while (word[n] < '0' || (word[n] > '9' && word[n] < 'A') || (word[n] > 'Z' && word[n] < 'a') || word[n] > 'z')
	{
		word[n] = '\0';
		n--;
	}
	if (n < 0)
		return 0;
	while (word[i] < '0' || (word[i] > '9' && word[i] < 'A') || (word[i] > 'Z' && word[i] < 'a') || word[i] > 'z')
	{
		i++;
	}
	/*
	ԭ��������char *strcpy(char* dest, const char *src);
	ͷ�ļ���#include <string.h> �� #include <stdio.h>
	���ܣ��Ѵ�src��ַ��ʼ�Һ���NULL���������ַ������Ƶ���dest��ʼ�ĵ�ַ�ռ�
	*/
	strcpy(word, word + i);
	return 1;
}

//��word���뵽Hash_map
void insert_word(char *p_word)
{
	unsigned int index = hashx(p_word);   //�õ�hash����ֵ,��ΪHash_map����
	node_no_space *p;                     //����ָ��
	//�����index����������ǿ�
	//��߾��õ��˽��Hash��ͻ��˼��
	for (p = bin[index]; p != NULL; p = p->next) 
	{
		//���keyvalue��ȣ���p->count��++
		if (strcmp(p_word, p->word) == 0) 
		{
			(p->count)++;
			return;
		}
	}
	//����õ���ӳ���hash����ֵ������������Hash_Map��û���ֹ�
	p = (node_no_space*)malloc(sizeof(node_no_space));
	p->count = 1;                                   //����Ϊ1
	p->word = (char*)malloc(strlen(p_word) + 1);    //����word�ڴ�
	strcpy(p->word, p_word);                        //����p_word��p_word��
	p->next = bin[index];                           //��ʼ����index������
	bin[index] = p;                        
}

//��С��
void min_heap(node_has_space *heap, int i, int len)      
{
	int left = 2 * i;
	int right = 2 * i + 1;
	int min_index = 0;

	if (left <= len && heap[left].count < heap[i].count) 
	{
		min_index = left;
	}
	else {
		min_index = i;
	}

	if (right <= len && heap[right].count < heap[min_index].count)
	{
		min_index = right;
	}
	if (min_index != i) 
	{
		swap(&heap[min_index].count, &heap[i].count);
		char buffer[WORDLEN];
		strcpy(buffer, heap[min_index].word);
		strcpy(heap[min_index].word, heap[i].word);
		strcpy(heap[i].word, buffer);
		min_heap(heap, min_index, len);
	}
}

void build_min_heap(node_has_space *heap, int len) 
{
	int index = len / 2;
	int i;
	for (i = index; i >= 1; i--)
	{
		min_heap(heap, i, len);
	}
}

//�������bin����ṹ����
void destroy_bin()
{
	node_no_space *p, *q;
	int i = 0;

	//����HASHLEN��
	while (i < HASHLEN) 
	{
		//��ֵ
		p = bin[i];
		while (p) 
		{
			//qΪ��ʱ����
			q = p->next;
			//���word
			if (p->word)
			{
				free(p->word);
				p->word = NULL;
			}
			//freeָ��
			free(p);
			p = NULL;
			p = q;
		}
		bin[i] = NULL;
		i++;
	}
}

//�ѵ���ͳ�ƽ��д���ı��ļ�
void write_to_file(char *path)
{
	FILE *out;
	if ((out = fopen(path, "w")) == NULL)   //��С�ı��ļ�
	{
		cout << "error, open " << path << " failed!" << endl;
		return;
	}
	int i;
	node_no_space *p;
	i = 0;
	while (i < HASHLEN)   //����HASHLEN�Σ�����bin����ṹ����
	{
		for (p = bin[i]; p != NULL; p = p->next)          //�����������word��count���ı��ļ����õ���hash��ͻ������һ��i��������ܶ�����
		{
			fprintf(out, "%s %d\n", p->word, p->count);
		}
		i++;                
	}
	fclose(out);         //�ر�С�ı��ļ�
	destroy_bin();       //�������bin����ṹ����
}

void main() 
{
	char word[WORDLEN];
	char path[20];         //����һ��С�ļ�·�����ַ���
	int count;
	int n = 10;            //���Ƶ�ʵĴ��������10��
	unsigned int index = 0;
	int i;
	FILE *fin[10];         //10��С�ļ�
	FILE *fout;
	FILE *f_message;
	node_has_space *heap = (node_has_space*)malloc(sizeof(node_has_space) * (n + 1));   //���������ڴ�

	//��words
	/*
	����ԭ�ͣ�FILE * fopen(const char * path,const char * mode);
������غ�����open��fclose��fopen_s[1] ��_wfopen
��������⣺ <stdio.h>
��������ֵ�� �ļ�˳���򿪺�ָ��������ļ�ָ��ͻᱻ���ء�����ļ���ʧ���򷵻�NULL�����Ѵ���������ERROR�С�
	r ��ֻ����ʽ���ļ������ļ�������ڡ�
	r+ �Կɶ�д��ʽ���ļ������ļ�������ڡ�
	rb+ ��д��һ���������ļ���ֻ�����д���ݡ�
	rt+ ��д��һ���ı��ļ����������д��
	w ��ֻд�ļ������ļ��������ļ�������Ϊ0�������ļ����ݻ���ʧ�����ļ��������������ļ���
	w+ �򿪿ɶ�д�ļ������ļ��������ļ�������Ϊ�㣬�����ļ����ݻ���ʧ�����ļ��������������ļ���
	a �Ը��ӵķ�ʽ��ֻд�ļ������ļ������ڣ���Ὠ�����ļ�������ļ����ڣ�д������ݻᱻ�ӵ��ļ�β�����ļ�ԭ�ȵ����ݻᱻ��������EOF��������
	a+ �Ը��ӷ�ʽ�򿪿ɶ�д���ļ������ļ������ڣ���Ὠ�����ļ�������ļ����ڣ�д������ݻᱻ�ӵ��ļ�β�󣬼��ļ�ԭ�ȵ����ݻᱻ������ ��ԭ����EOF����������
	wb ֻд�򿪻��½�һ���������ļ���ֻ����д���ݡ�
	wb+ ��д�򿪻���һ���������ļ����������д��
	wt+ ��д�򿪻��Ž���һ���ı��ļ��������д��
	at+ ��д��һ���ı��ļ�������������ı�ĩ׷�����ݡ�
	ab+ ��д��һ���������ļ�������������ļ�ĩ׷�����ݡ�
	*/
	if ((f_message = fopen("words.txt", "r")) == NULL)  //"r"��ֻ����ʽ���ļ������ļ�������ڡ�
	{
		cout << "error, open source file failed!" << endl;
		return;
	}
	for (i = 0; i < n; i++)
	{
		sprintf(path, "tmp%d.txt", i);        //�Ѹ�ʽ��������д��ĳ���ַ���
		fin[i] = fopen(path, "w");            //"w"��ֻд�ļ������ļ��������ļ�������Ϊ0�������ļ����ݻ���ʧ�����ļ��������������ļ���
	}
	/*
	������:int fscanf(FILE*stream, constchar*format, [argument...]); 
	�� ��: ��һ������ִ�и�ʽ������,fscanf�����ո�ͻ���ʱ������ע��ո�ʱҲ����������fgets������fgets�����ո񲻽�����
	����ֵ�����ͣ��ɹ����ض���Ĳ����ĸ�����ʧ�ܷ���EOF(-1)��
	%d������һ��ʮ��������.
	%i ������ʮ���ƣ��˽��ƣ�ʮ��������������%d���ƣ������ڱ���ʱͨ������ǰ�û���������ֽ��ƣ�����롰0x������ʮ�����ƣ����롰0����Ϊ�˽��ơ����紮��031��ʹ��%dʱ�ᱻ����31������ʹ��%iʱ������25.
	%u������һ���޷���ʮ��������.
	%f %F %g %G : ��������ʵ����������С����ʽ��ָ����ʽ����.
	%x %X�� ����ʮ����������.
	%o': ����˽�������.
	%s : ����һ���ַ����������ַ���\0'������
	%c : ����һ���ַ����޷������ֵ���ո���Ա����롣
	*/
	while (fscanf(f_message, "%s", word) != EOF) 
	{
		if (trim_word(word))  //���word�ǵ���
		{
			index = hashx(word) % n;
			/*
			int fprintf (FILE* stream, const char*format, [argument])
			FILE*stream���ļ�ָ��
			const char* format�������ʽ
			[argument]�����Ӳ����б�
			*/
			fprintf(fin[index], "%s\n", word);
		}
	}
	for (i = 0; i < n; i++)
	{
		fclose(fin[i]);          //�ر����ļ�
	}
	// do hash count  
	for (i = 0; i < n; i++)
	{
		sprintf(path, "tmp%d.txt", i);
		fin[i] = fopen(path, "r");
		while (fscanf(fin[i], "%s", word) != EOF)       //��1-10���ļ��ڵĵ��ʣ�ֱ����Ϊֹ
		{
			insert_word(word);           //����word��Hash_map
		}
		fclose(fin[i]);                  //�ر�С�ı��ļ�
		write_to_file(path);            //д����ͳ����Ϣ��С�ı��ļ�
	}
	// heap find   
	for (i = 1; i <= n; i++) 
	{
		strcpy(heap[i].word, "");
		heap[i].count = 0;
	}
	build_min_heap(heap, n);      //������С��
	for (i = 0; i < n; i++) 
	{
		sprintf(path, "tmp%d.txt", i);
		fin[i] = fopen(path, "r");
		while (fscanf(fin[i], "%s %d", word, &count) != EOF) 
		{
			if (count > heap[1].count) 
			{
				heap[1].count = count;
				strcpy(heap[1].word, word);
				min_heap(heap, 1, n);      //��С������
			}
		}
		fclose(fin[i]);
	}
	for (i = 1; i <= n; i++)
		cout << heap[i].word << ":" << heap[i].count << endl;
}