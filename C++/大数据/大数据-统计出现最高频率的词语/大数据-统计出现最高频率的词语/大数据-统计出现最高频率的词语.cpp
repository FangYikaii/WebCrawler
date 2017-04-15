// 大数据-统计出现最高频率的词语.cpp : 定义控制台应用程序的入口点。
// 有一个1G大小的一个文件，里面每一行是一个词，词的大小不超过16字节，内存限制大小是1M。返回频数最高的100个词.
/*
1G=2^30B=2^26个单词， 1M=2^20B=2^16个单词
所以，分成2^10个文件，取2000个文件

因为大家都清楚的知道1G的文件不可能用1M的内存空间处理,所以我们要按照1M的上线来计算。
假设每个单词都为16个字节，那么1M的内存可以处理多少个单词呢？ 1M = 1024 KB = 1024 * 1024 B 。然后1M / 16B = 2^16个单词。
那么1G大概有多少个单词呢？ 有2^26个单词，但是实际中远远不止这些，因为我们是按照最大单词长度算的。
我们需要把这1G的单词分批处理，根据上面的计算，可以分成大于2^10个文件。索性就分成2000个文件吧，

怎么分呢，不能随便分，不能简单的按照单词的顺序然后模2000划分，因为这样有可能相同的单词被划分到不同的文件中去了。
这样在统计个数的时候被当成的不同的单词，因为我们没有能力把在不同文件中相同单词出现的次数跨越文件的相加，这就迫使我们要把不同序号的同一个单词划分到同一个文件中：

应用hash统计吧。稍后代码会给出方法。然后呢，我们对每个文件进行分别处理。按照key-value的方法处理每个单词，最终得出每个文件中包含每个单词和单词出现的次数。
然后再建立大小为100的小根堆。一次遍历文件进行处理。我没有弄1G的文件，弄1M的，简单的实现了一下，不过原理就是这样的。
*/
#include "stdafx.h"
#include<iostream>  
#include<string>  
using namespace std;

#define FILE_NUM 10  
#define WORDLEN 30  
#define HASHLEN 7303                //链表存储结构，也是HASH函数映射所能达到的最大值，相当于一个59个'z'的长度

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

//链表存储结构
p_node_no_space bin[HASHLEN] = { NULL };         //定义Hash_Map，大小为HASHLEN

//交换两个数
void swap(int *a, int *b)
{
	int temp;
	temp = *a;
	*a = *b;
	*b = temp;
}

//哈希函数
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

//检查word是否为单词
int trim_word(char *word) 
{
	int n = strlen(word) - 1;
	int i = 0;
	if (n < 0)
		return 0;
	//48-57:'0'-'9'   65-90:'A'-'Z'  97-123:'a'-'z'
	//非0-9，a-z，A-Zd的ASCII码
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
	原型声明：char *strcpy(char* dest, const char *src);
	头文件：#include <string.h> 和 #include <stdio.h>
	功能：把从src地址开始且含有NULL结束符的字符串复制到以dest开始的地址空间
	*/
	strcpy(word, word + i);
	return 1;
}

//把word插入到Hash_map
void insert_word(char *p_word)
{
	unsigned int index = hashx(p_word);   //得到hash函数值,作为Hash_map索引
	node_no_space *p;                     //链表指针
	//如果第index的链表数组非空
	//这边就用到了解决Hash冲突的思想
	for (p = bin[index]; p != NULL; p = p->next) 
	{
		//如果keyvalue相等，（p->count）++
		if (strcmp(p_word, p->word) == 0) 
		{
			(p->count)++;
			return;
		}
	}
	//如果该单词映射的hash函数值的索引在整个Hash_Map还没出现过
	p = (node_no_space*)malloc(sizeof(node_no_space));
	p->count = 1;                                   //计数为1
	p->word = (char*)malloc(strlen(p_word) + 1);    //申请word内存
	strcpy(p->word, p_word);                        //复制p_word到p_word中
	p->next = bin[index];                           //初始化第index个链表
	bin[index] = p;                        
}

//最小堆
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

//清空所有bin链表结构数组
void destroy_bin()
{
	node_no_space *p, *q;
	int i = 0;

	//遍历HASHLEN次
	while (i < HASHLEN) 
	{
		//赋值
		p = bin[i];
		while (p) 
		{
			//q为临时变量
			q = p->next;
			//清空word
			if (p->word)
			{
				free(p->word);
				p->word = NULL;
			}
			//free指针
			free(p);
			p = NULL;
			p = q;
		}
		bin[i] = NULL;
		i++;
	}
}

//把单词统计结果写入文本文件
void write_to_file(char *path)
{
	FILE *out;
	if ((out = fopen(path, "w")) == NULL)   //打开小文本文件
	{
		cout << "error, open " << path << " failed!" << endl;
		return;
	}
	int i;
	node_no_space *p;
	i = 0;
	while (i < HASHLEN)   //遍历HASHLEN次，所有bin链表结构数组
	{
		for (p = bin[i]; p != NULL; p = p->next)          //遍历链表，输出word和count到文本文件，用到了hash冲突，就是一个i可以延伸很多链表
		{
			fprintf(out, "%s %d\n", p->word, p->count);
		}
		i++;                
	}
	fclose(out);         //关闭小文本文件
	destroy_bin();       //清空所有bin链表结构数组
}

void main() 
{
	char word[WORDLEN];
	char path[20];         //定义一个小文件路径的字符串
	int count;
	int n = 10;            //最高频率的词语次数：10个
	unsigned int index = 0;
	int i;
	FILE *fin[10];         //10个小文件
	FILE *fout;
	FILE *f_message;
	node_has_space *heap = (node_has_space*)malloc(sizeof(node_has_space) * (n + 1));   //分配链表内存

	//打开words
	/*
	函数原型：FILE * fopen(const char * path,const char * mode);
　　相关函数：open，fclose，fopen_s[1] ，_wfopen
　　所需库： <stdio.h>
　　返回值： 文件顺利打开后，指向该流的文件指针就会被返回。如果文件打开失败则返回NULL，并把错误代码存在ERROR中。
	r 以只读方式打开文件，该文件必须存在。
	r+ 以可读写方式打开文件，该文件必须存在。
	rb+ 读写打开一个二进制文件，只允许读写数据。
	rt+ 读写打开一个文本文件，允许读和写。
	w 打开只写文件，若文件存在则文件长度清为0，即该文件内容会消失。若文件不存在则建立该文件。
	w+ 打开可读写文件，若文件存在则文件长度清为零，即该文件内容会消失。若文件不存在则建立该文件。
	a 以附加的方式打开只写文件。若文件不存在，则会建立该文件，如果文件存在，写入的数据会被加到文件尾，即文件原先的内容会被保留。（EOF符保留）
	a+ 以附加方式打开可读写的文件。若文件不存在，则会建立该文件，如果文件存在，写入的数据会被加到文件尾后，即文件原先的内容会被保留。 （原来的EOF符不保留）
	wb 只写打开或新建一个二进制文件；只允许写数据。
	wb+ 读写打开或建立一个二进制文件，允许读和写。
	wt+ 读写打开或着建立一个文本文件；允许读写。
	at+ 读写打开一个文本文件，允许读或在文本末追加数据。
	ab+ 读写打开一个二进制文件，允许读或在文件末追加数据。
	*/
	if ((f_message = fopen("words.txt", "r")) == NULL)  //"r"以只读方式打开文件，该文件必须存在。
	{
		cout << "error, open source file failed!" << endl;
		return;
	}
	for (i = 0; i < n; i++)
	{
		sprintf(path, "tmp%d.txt", i);        //把格式化的数据写入某个字符串
		fin[i] = fopen(path, "w");            //"w"打开只写文件，若文件存在则文件长度清为0，即该文件内容会消失。若文件不存在则建立该文件。
	}
	/*
	函数名:int fscanf(FILE*stream, constchar*format, [argument...]); 
	功 能: 从一个流中执行格式化输入,fscanf遇到空格和换行时结束，注意空格时也结束。这与fgets有区别，fgets遇到空格不结束。
	返回值：整型，成功返回读入的参数的个数，失败返回EOF(-1)。
	%d：读入一个十进制整数.
	%i ：读入十进制，八进制，十六进制整数，与%d类似，但是在编译时通过数据前置或后置来区分进制，如加入“0x”则是十六进制，加入“0”则为八进制。例如串“031”使用%d时会被算作31，但是使用%i时会算作25.
	%u：读入一个无符号十进制整数.
	%f %F %g %G : 用来输入实数，可以用小数形式或指数形式输入.
	%x %X： 读入十六进制整数.
	%o': 读入八进制整数.
	%s : 读入一个字符串，遇空字符‘\0'结束。
	%c : 读入一个字符。无法读入空值。空格可以被读入。
	*/
	while (fscanf(f_message, "%s", word) != EOF) 
	{
		if (trim_word(word))  //如果word是单词
		{
			index = hashx(word) % n;
			/*
			int fprintf (FILE* stream, const char*format, [argument])
			FILE*stream：文件指针
			const char* format：输出格式
			[argument]：附加参数列表
			*/
			fprintf(fin[index], "%s\n", word);
		}
	}
	for (i = 0; i < n; i++)
	{
		fclose(fin[i]);          //关闭流文件
	}
	// do hash count  
	for (i = 0; i < n; i++)
	{
		sprintf(path, "tmp%d.txt", i);
		fin[i] = fopen(path, "r");
		while (fscanf(fin[i], "%s", word) != EOF)       //读1-10个文件内的单词，直到空为止
		{
			insert_word(word);           //插入word到Hash_map
		}
		fclose(fin[i]);                  //关闭小文本文件
		write_to_file(path);            //写单词统计信息到小文本文件
	}
	// heap find   
	for (i = 1; i <= n; i++) 
	{
		strcpy(heap[i].word, "");
		heap[i].count = 0;
	}
	build_min_heap(heap, n);      //构建最小堆
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
				min_heap(heap, 1, n);      //最小堆排序
			}
		}
		fclose(fin[i]);
	}
	for (i = 1; i <= n; i++)
		cout << heap[i].word << ":" << heap[i].count << endl;
}