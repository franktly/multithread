// multithread.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include <stdio.h>
#include <process.h>
#include <windows.h>
#include <time.h>

/************************************************************************/
/* 
Windows multi-thread��

���
1. ͬ���ͻ��⣺
���̲߳���ִ��ʱ��������Դ������߳�Э����ʹ���߳�֮����������������Լ��ϵ��
��1������໥��Լ�����⣩��һ��ϵͳ�еĶ���̱߳�ȻҪ����ĳ��ϵͳ��Դ���繲��CPU������I/O�豸���߳�A��ʹ�ù�����Դʱ�������̶߳�Ҫ�ȴ���
��2��ֱ���໥��Լ��ͬ������ ������Լ��Ҫ����Ϊ�߳�֮��ĺ����������߳�A���������ṩ���߳�B����һ��������ô�߳�B���߳�A�������ʹ�֮ǰ������������״̬��
���ڻ������������⣺�߳�A���߳�B�������ĳ����Դ������֮��ͻ����˳�����⡪��Ҫô�߳�A�ȴ��߳�B������ϣ�Ҫô�߳�B�ȴ��̲߳�����ϣ�����ʵ�����̵߳�ͬ���ˡ�
��ˣ�ͬ���������⣬������һ�������ͬ��

2. �ٽ���Դ���ٽ���
�ٽ���Դ�� ��һ��ʱ����ֻ����һ���̷߳��ʵ���Դ��������д���������豸������Ҫ�󱻻���ķ��ʡ�
�ٽ����� ÿ�������з����ٽ���Դ�Ĵ���

3. ʵ��ͬ���ͻ��ⷽ����
1���ؼ���CriticalSection
2��������Mutex
3���¼�Event
4���ź���Semaphore
PS��
1���ؼ����뻥���������߳�����Ȩ���PV������ͬһ���߳��Ͻ��У����������ܺܺõĴ������������
2�����������¼����ź��������ں˶��󣬿��Կ����ʹ�ã�ͨ��OpenMutex��OpenEvent��OpenSemaphore��

4. PV������
PV������P����ԭ���V����ԭ����ɣ�ԭ��Ҳ��ԭ�Ӳ���Atomic Operation���ǲ����жϵĹ��̣�
�����ź������в��������嶨�����£�
P(S)��
�� ���ź���S��ֵ��1����S=S-1��
�� ���S>=0����ý��̼���ִ�У�����ý�����Ϊ�ȴ�״̬��

V(S)��
�� ���ź���S��ֵ��1����S=S+1��
�� �ý��̼���ִ�У�������źŵĵȴ��������еȴ����̾ͻ���һ�ȴ����̡�
*/
/************************************************************************/



volatile long g_nLogicCount;
const int THREAD_NUM = 10;  // ��ഴ��64���߳�

/*
һ��Ĺ���ȫ�ֱ�����Դ���޻����ͬ����
*/
unsigned int _stdcall threadFun(void *param)
{
	Sleep(100);
	g_nLogicCount++;
	Sleep(50);
	return 0;
}

void testThread(void)
{
	printf("test multi-thread\n");
	//�ظ�20���Ա�۲���̷߳���ͬһ��Դʱ���µĳ�ͻ 
	int repeatCount = 20;

	while (repeatCount--)
	{
		g_nLogicCount = 0;
		HANDLE handle[THREAD_NUM];
		for (int i = 0; i < THREAD_NUM; i++)
		{
			handle[i] = (HANDLE)_beginthreadex(NULL, 0, threadFun, NULL, 0, NULL);
		}

		WaitForMultipleObjects(THREAD_NUM, handle, TRUE, INFINITE);
		printf("��%d���û���¼���¼�����%d[repeat = %d]\n", THREAD_NUM, g_nLogicCount, repeatCount);
	}
}

/*
�����͸�ֵ�������⣺

Windowsϵͳ�ṩ��һЩ��Interlocked��ͷ�ĺ�����֤��һ��ֵ�������͸�ֵ������ԭ�Ӳ������������ɴ����
1.��������:
LONG__cdeclInterlockedIncrement(LONG volatile* Addend);
LONG__cdeclInterlockedDecrement(LONG volatile* Addend);
���ر���ִ����������֮���ֵ:
LONG__cdec InterlockedExchangeAdd(LONG volatile* Addend, LONGValue);
����������ֵ���Ӹ��������Ǽ�

2.��ֵ����
LONG__cdeclInterlockedExchange(LONG volatile* Target, LONGValue);
Value������ֵ�������᷵��ԭ�ȵ�ֵ
*/
unsigned int _stdcall threadFunEx(void *param)
{
	Sleep(100);
	// �Ľ�(����ԭ�Ӳ�������)��
	InterlockedIncrement((LPLONG)&g_nLogicCount);
	Sleep(50);
	return 0;
}

void testThreadEx(void)
{
	printf("test multi-thread with interlocked function\n");
	//�ظ�20���Ա�۲���̷߳���ͬһ��Դʱ���µĳ�ͻ 
	int repeatCount = 20;

	while (repeatCount--)
	{
		g_nLogicCount = 0;
		HANDLE handle[THREAD_NUM];
		for (int i = 0; i < THREAD_NUM; i++)
		{
			handle[i] = (HANDLE)_beginthreadex(NULL, 0, threadFunEx, NULL, 0, NULL);
		}

		WaitForMultipleObjects(THREAD_NUM, handle, TRUE, INFINITE);
		printf("��%d���û���¼���¼�����%d[repeat = %d]\n", THREAD_NUM, g_nLogicCount, repeatCount);
	}
}

/*
�������Ķ��߳�(�޻����ͬ��)��
���߳�����10�����̲߳�����ʾ���߳���ŵı�����ַ��Ϊ�������ݸ����̡߳�
���߳̽��ղ��� -> sleep(50) -> ȫ�ֱ���++ -> sleep(0) -> ���������ȫ�ֱ���
*/
unsigned int _stdcall threadFun2(void *param)
{
	int threadNum = *((int*)param);
	Sleep(50);
	g_nLogicCount++;
	Sleep(0);
	printf("����������̱߳��Ϊ%d, ȫ�ֱ���Ϊ%d\n", threadNum, g_nLogicCount);
	
	return 0;
}

void testThread2(void)
{
	printf("test multi-thread with para\n");
	int i = 0;
	HANDLE handle[THREAD_NUM];
	g_nLogicCount = 0;
	while (i < THREAD_NUM)
	{
		handle[i] = (HANDLE)_beginthreadex(NULL, 0, threadFun2, &i, 0, NULL);
		i++; // �´��������̲߳�����������Ҫ�ģ����������߳��Ѿ��޸Ĺ���
	}
	WaitForMultipleObjects(THREAD_NUM, handle, TRUE, INFINITE);
}


/*
�ؼ��ξ��С��߳�����Ȩ������
1. �ؼ��ι���ʼ���������١�������뿪�ؼ������ĸ�������
2���ؼ��ο��Խ���̵߳Ļ������⣬����Ϊ���С��߳�����Ȩ���������޷����ͬ�����⡣
3���Ƽ��ؼ�������ת�����ʹ��
*/

CRITICAL_SECTION g_csThreadPara, g_csThreadCode;

unsigned int _stdcall threadFun2WithCriticalSection(void *param)
{
	int threadNum = *((int*)param);
	LeaveCriticalSection(&g_csThreadPara); 

	Sleep(50);

	EnterCriticalSection(&g_csThreadCode); // ȫ�ֱ������ٽ���(�����̼߳以��)�������𵽻���Ч��
	g_nLogicCount++;
	Sleep(0);
	printf("����������̱߳��Ϊ%d, ȫ�ֱ���Ϊ%d\n", threadNum, g_nLogicCount);
	LeaveCriticalSection(&g_csThreadCode);

	return 0;
}

// ���̺߳����߳�ͬ��ʧ��
void testThread2WithCriticalSection(void)
{
	printf("test multi-thread with critical section\n");

	InitializeCriticalSection(&g_csThreadPara);
	InitializeCriticalSection(&g_csThreadCode);

	int i = 0;
	HANDLE handle[THREAD_NUM];
	g_nLogicCount = 0;
	while (i < THREAD_NUM)
	{
		EnterCriticalSection(&g_csThreadPara); // ���̺߳����̵߳��ٽ���(�߳�ͬ��)�� ����ͬ����
		handle[i] = (HANDLE)_beginthreadex(NULL, 0, threadFun2WithCriticalSection, &i, 0, NULL);
		i++; // �´��������̲߳�����������Ҫ�ģ����������߳��Ѿ��޸Ĺ���
	}

	WaitForMultipleObjects(THREAD_NUM, handle, TRUE, INFINITE);
	
	DeleteCriticalSection(&g_csThreadPara);
	DeleteCriticalSection(&g_csThreadCode);
}


/*
1. �¼����ں˶����¼���Ϊ�ֶ���λ�¼����Զ���λ�¼���
   �¼�Event�ڲ�������һ��ʹ�ü����������ں˶����У���һ������ֵ��ʾ���ֶ���λ�¼������Զ���λ�¼�����һ������ֵ������ʾ�¼����޴�����
2���¼�������SetEvent()����������ResetEvent()�����δ��������������PulseEvent()������һ���¼����塣
3���¼����Խ���̼߳�ͬ�����⣬���Ҳ�ܽ���������⡣
*/
HANDLE g_hEvent;

unsigned int _stdcall threadFun2WithEvent(void *param)
{
	int threadNum = *((int*)param);
	SetEvent(g_hEvent); // V������ͬ���������߳�

	Sleep(50);

	EnterCriticalSection(&g_csThreadCode); // ȫ�ֱ������ٽ���(�����̼߳以��)�������𵽻���Ч��
	g_nLogicCount++;
	Sleep(0);
	printf("����������̱߳��Ϊ%d, ȫ�ֱ���Ϊ%d\n", threadNum, g_nLogicCount);
	LeaveCriticalSection(&g_csThreadCode);

	return 0;
}

// ���̺߳����߳�ͬ���ɹ�
void testThread2WithEvent(void)
{
	printf("test multi-thread with event\n");

	InitializeCriticalSection(&g_csThreadCode);
	// �Զ���Ĭ��Ϊδ����
	g_hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	int i = 0;
	HANDLE handle[THREAD_NUM];
	g_nLogicCount = 0;
	while (i < THREAD_NUM)
	{
		handle[i] = (HANDLE)_beginthreadex(NULL, 0, threadFun2WithEvent, &i, 0, NULL);
		WaitForSingleObject(g_hEvent, INFINITE); // P����
		i++; // �´��������̲߳�����������Ҫ�ģ����������߳��Ѿ��޸Ĺ���
	}

	WaitForMultipleObjects(THREAD_NUM, handle, TRUE, INFINITE);

	CloseHandle(g_hEvent);
	DeleteCriticalSection(&g_csThreadCode);
}

/*
1�����������ں˶�������ؼ��ζ��С��߳�����Ȩ�����Բ��������̵߳�ͬ����
2���������ܹ����ڶ������֮���̻߳������⣬�����������Ľ��ĳ����������ֹ����ɵġ�����������
*/
HANDLE g_hMutex;

unsigned int _stdcall threadFun2WithMutex(void *param)
{
	int threadNum = *((int*)param);
	ReleaseMutex(g_hMutex); // V������ͬ���������߳�

	Sleep(50);

	EnterCriticalSection(&g_csThreadCode); // ȫ�ֱ������ٽ���(�����̼߳以��)�������𵽻���Ч��
	g_nLogicCount++;
	Sleep(0);
	printf("����������̱߳��Ϊ%d, ȫ�ֱ���Ϊ%d\n", threadNum, g_nLogicCount);
	LeaveCriticalSection(&g_csThreadCode);

	return 0;
}

// ���̺߳����߳�ͬ��ʧ��
void testThread2WithMutex(void)
{
	printf("test multi-thread with mutex\n");

	InitializeCriticalSection(&g_csThreadCode);

	//��ʼ����������ؼ��� �ڶ�������ΪTRUE��ʾ������Ϊ�����߳�����
	g_hMutex = CreateMutex(NULL, FALSE, NULL);
	int i = 0;
	HANDLE handle[THREAD_NUM];
	g_nLogicCount = 0;
	while (i < THREAD_NUM)
	{
		handle[i] = (HANDLE)_beginthreadex(NULL, 0, threadFun2WithEvent, &i, 0, NULL);
		WaitForSingleObject(g_hMutex, INFINITE); // P����
		i++; // �´��������̲߳�����������Ҫ�ģ����������߳��Ѿ��޸Ĺ���
	}

	WaitForMultipleObjects(THREAD_NUM, handle, TRUE, INFINITE);

	CloseHandle(g_hMutex);
	DeleteCriticalSection(&g_csThreadCode);
}

/*
1. ��ǰ��Դ��������0����ʾ�ź������ڴ���������0��ʾ��Դ�Ѿ��ľ����ź�������ĩ������
  �ڶ��ź������õȴ�����ʱ���ȴ����������ź����ĵ�ǰ��Դ�������������0(���ź������ڴ���״̬)����1�󷵻��õ����̼߳���ִ�С�һ���߳̿��Զ�ε��õȴ���������С�ź���
2. �ź������Խ���߳�֮���ͬ������
3. �ź�����ͨ��������Դ��ǰʣ���������ݵ�ǰʣ��������Ƚ��������ź����Ǵ��ڴ���״̬����δ����״̬
*/

HANDLE g_hSemaphore;
unsigned int _stdcall threadFun2WithSemaphore(void *param)
{
	int threadNum = *((int*)param);
	ReleaseSemaphore(g_hSemaphore, 1, NULL); // V������ͬ���������߳�

	Sleep(50);

	EnterCriticalSection(&g_csThreadCode); // ȫ�ֱ������ٽ���(�����̼߳以��)�������𵽻���Ч��
	g_nLogicCount++;
	Sleep(0);
	printf("����������̱߳��Ϊ%d, ȫ�ֱ���Ϊ%d\n", threadNum, g_nLogicCount);
	LeaveCriticalSection(&g_csThreadCode);

	return 0;
}

// ���̺߳����߳�ͬ���ɹ�
void testThread2WithSemaphore(void)
{
	printf("test multi-thread with semaphore\n");

	InitializeCriticalSection(&g_csThreadCode);

	//��ǰ0����Դ���������1��ͬʱ����
	g_hSemaphore = CreateSemaphore(NULL, 0, 1, NULL);

	int i = 0;
	HANDLE handle[THREAD_NUM];
	g_nLogicCount = 0;
	while (i < THREAD_NUM)
	{
		handle[i] = (HANDLE)_beginthreadex(NULL, 0, threadFun2WithSemaphore, &i, 0, NULL);
		WaitForSingleObject(g_hSemaphore, INFINITE); // P����
		i++; // �´��������̲߳�����������Ҫ�ģ����������߳��Ѿ��޸Ĺ���
	}

	WaitForMultipleObjects(THREAD_NUM, handle, TRUE, INFINITE);

	CloseHandle(g_hSemaphore);
	DeleteCriticalSection(&g_csThreadCode);
}


/*
���������������⣺
������������һ����������������Ʒ����Щ��Ʒ���ṩ�����ɸ�������ȥ���ѣ�
Ϊ��ʹ�����ߺ��������ܲ���ִ�У�������֮������һ�����ж���������Ļ���أ�
�����߽��������Ĳ�Ʒ����һ���������У������߿��Դӻ�������ȡ�߲�Ʒ�������ѣ�
��Ȼ�����ߺ�������֮����뱣��ͬ�����������������ߵ�һ���յĻ�������ȡ��Ʒ��
Ҳ��������������һ���Ѿ������Ʒ�Ļ��������ٴ�Ͷ�Ų�Ʒ

�����棺
1������Ҫ�����������������߶Ի���������ʱ�Ļ��⡣
2���������������������ж��ٸ���������ж��ٸ�����������ֻ�ж���ͬ������:
�ֱ���������Ҫ�ȴ��пջ���������Ͷ�Ų�Ʒ��������Ҫ�ȴ��зǿջ���������ȥȡ��Ʒ
*/

// ����vs����̨��ɫ��������ʾ���������ߺ�������
BOOL setConsoleColor(WORD attributes)
{
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	if (INVALID_HANDLE_VALUE == hConsole)
	{
		printf("invalid handle!\n");
		return FALSE;
	}

	return SetConsoleTextAttribute(hConsole, attributes);
}

// one producer - one consumer - one buffer:
/*
one critical section ��  �������g_nbuffer
two semaphore�� ͬ����(producer)�ȴ�������Ϊ���źź�(consumer)�ȴ���������Ϊ���ź�
or two event;
*/
const int PRODUCT_NUMBER = 50;
int g_nbuffer;
CRITICAL_SECTION g_cs;
HANDLE g_hEmptySemaphore, g_hNotEmptySemaphore;
HANDLE g_hEmptyEvent, g_hNotEmptyEvent;

unsigned int _stdcall producerThreadFun(void*)
{
	// ��������PRODUCT_NUMBER����Ʒ����g_nbuffer
	for (int i = 0 ; i < PRODUCT_NUMBER; i++) 
	{
		// �ȵ�Empty�ź�,��������Ϊ�ղ�produce 
		//WaitForSingleObject(g_hEmptySemaphore, INFINITE);
		WaitForSingleObject(g_hEmptyEvent, INFINITE);

		EnterCriticalSection(&g_cs);
		g_nbuffer = i;
		printf("producer produce %d into buffer\n", g_nbuffer);
		LeaveCriticalSection(&g_cs);

		//֪ͨ��������������
		//ReleaseSemaphore(g_hNotEmptySemaphore, 1, NULL); 
		SetEvent(g_hNotEmptyEvent);
	}
	return 0;
}

unsigned int _stdcall consumerThreadFun(void*)
{
	volatile bool flag = true;
	while (flag)
	{
		// �ȵ�Not Empty�ź�,�����������˲�consumer 
		//WaitForSingleObject(g_hNotEmptySemaphore, INFINITE);
		WaitForSingleObject(g_hNotEmptyEvent, INFINITE);

		EnterCriticalSection(&g_cs);
		setConsoleColor(FOREGROUND_GREEN);  // ����Ϊ��ɫ
		printf("consumer consume %d from buffer\n", g_nbuffer);
		setConsoleColor(FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_BLUE); // ��ԭ��ɫ��ɫ
		
		if (g_nbuffer == PRODUCT_NUMBER) // ���ѵ����һ����Ʒ��ɺ���˳��������߳�
		{
			flag = false;
		}
		LeaveCriticalSection(&g_cs);

		//֪ͨ���������ݱ������ˣ�������Ϊ��
		//ReleaseSemaphore(g_hEmptySemaphore, 1, NULL);
		SetEvent(g_hEmptyEvent);
	}

	return 0;
}

void testThreadWithSingleProducerAndConsumer(void)
{
	printf("one producer - one consumer - one buffer:\n");

	InitializeCriticalSection(&g_cs);

	// ��ʼ�����ź���Ϊ����̬���ǿ��ź���Ϊ�Ǵ���̬����������Ϊ1
	//g_hEmptySemaphore = CreateSemaphore(NULL, 1, 1, NULL);
	//g_hNotEmptySemaphore = CreateSemaphore(NULL, 0, 1, NULL);

	// ��ʼ�����¼�Ϊ����̬���ǿ��¼�Ϊ�Ǵ���̬����Ϊ�Զ����͵���WaitForSingleObject���Զ�����ResetEvent
	g_hEmptyEvent = CreateEvent(NULL, FALSE, TRUE, NULL);
	g_hNotEmptyEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

	// �ֱ𴴽�producer and consumer�̣߳�
	HANDLE handle[2]; 
	handle[0] = (HANDLE)_beginthreadex(NULL, 0, producerThreadFun, NULL, 0, NULL);
	handle[1] = (HANDLE)_beginthreadex(NULL, 0, consumerThreadFun, NULL, 0, NULL);

	WaitForMultipleObjects(2, handle, TRUE, INFINITE);

	//CloseHandle(g_hEmptySemaphore);
	//CloseHandle(g_hNotEmptySemaphore);
	CloseHandle(g_hEmptyEvent);
	CloseHandle(g_hNotEmptyEvent);

	DeleteCriticalSection(&g_cs);
}


// one- producer - multi- consumer - multi- buffer:
/*
one critical section ��  �������g_nbuffer
two semaphore�� ͬ����(producer)�ȴ�������Ϊ���źź�(consumer)�ȴ���������Ϊ���ź�
or two event;
*/
#define  BUFFER_SIZE 10
int g_nmultibuffer[BUFFER_SIZE];
int g_i, g_j;

unsigned int _stdcall producerThreadFun2(void*)
{
	// ��������PRODUCT_NUMBER����Ʒ����g_nmultibuffer
	for (int i = 0; i < PRODUCT_NUMBER; i++)
	{
		// �ȵ�Empty�ź�,��������Ϊ�ղ�produce 
		WaitForSingleObject(g_hEmptySemaphore, INFINITE);

		EnterCriticalSection(&g_cs);
		g_nmultibuffer[g_i] = i;
		printf("producer produce %d into buffer %d \n", g_nmultibuffer[g_i], g_i);
		g_i = (g_i + 1) % BUFFER_SIZE;
		LeaveCriticalSection(&g_cs);

		//֪ͨ��������������
		ReleaseSemaphore(g_hNotEmptySemaphore, 1, NULL); 
	}
	return 0;
}

unsigned int _stdcall consumerThreadFun2(void*)
{
	volatile bool flag = true;
	while (flag)
	{
		// �ȵ�Not Empty�ź�,�����������˲�consumer 
		WaitForSingleObject(g_hNotEmptySemaphore, INFINITE);

		EnterCriticalSection(&g_cs);
		setConsoleColor(FOREGROUND_GREEN);  // ����Ϊ��ɫ
		printf("consumer consume %d from buffer %d \n", g_nmultibuffer[g_j], g_j);
		setConsoleColor(FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_BLUE); // ��ԭ��ɫ��ɫ
		if (g_nmultibuffer[g_j] == PRODUCT_NUMBER) // ���ѵ����һ����Ʒ��ɺ���˳��������߳�
		{
			flag = false;
		}
		g_j = (g_j + 1) % BUFFER_SIZE;
		LeaveCriticalSection(&g_cs);

		//֪ͨ���������ݱ������ˣ�������Ϊ��
		ReleaseSemaphore(g_hEmptySemaphore, 1, NULL);
	}

	return 0;
}

void testThreadWithMultiProducerAndConsumer(void)
{
	printf("one producer - multi consumer(2) - multi buffer(4):\n");

	InitializeCriticalSection(&g_cs);

	// ��ʼ�����ź���Ϊ����̬���ǿ��ź���Ϊ�Ǵ���̬����������Ϊ1
	g_hEmptySemaphore = CreateSemaphore(NULL, BUFFER_SIZE, BUFFER_SIZE, NULL);
	g_hNotEmptySemaphore = CreateSemaphore(NULL, 0, BUFFER_SIZE, NULL);

	g_i = 0; // �������̵߳Ļ����
	g_j = 0; // �������̵߳Ļ����

	memset(g_nmultibuffer, 0, sizeof(g_nmultibuffer));

	// �ֱ𴴽�producer and consumer�̣߳�
	HANDLE handle[3];
	handle[0] = (HANDLE)_beginthreadex(NULL, 0, producerThreadFun2, NULL, 0, NULL);
	handle[1] = (HANDLE)_beginthreadex(NULL, 0, consumerThreadFun2, NULL, 0, NULL);
	handle[2] = (HANDLE)_beginthreadex(NULL, 0, consumerThreadFun2, NULL, 0, NULL);

	WaitForMultipleObjects(3, handle, TRUE, INFINITE);

	CloseHandle(g_hEmptySemaphore);
	CloseHandle(g_hNotEmptySemaphore);

	DeleteCriticalSection(&g_cs);
}

/*
��д�����⣺
����������
��һ��д�ߺܶ���ߣ�������߿���ͬʱ���ļ���
��д����д�ļ�ʱ�������ж����ڶ��ļ���ͬ���ж����ڶ��ļ�ʱд��Ҳ��ȥ��д�ļ�
*/

#define MAX_READER_NUM 20
#define MAX_WRITER_NUM 20

HANDLE g_hEventNoWriter;  // �ȴ�û��д��
HANDLE g_hEventNoReader;  // �ȴ�û�ж���

CRITICAL_SECTION g_csReaderandWriter; 

int  g_nReaderNumber;     // ��������
int  g_nWriterNumber;     // д������

//  ���̴߳�ӡ,�ӻ��⣬��ֹ��д�߳������жϣ�������ɫ�������
void readerPrint(char* pszFormat, ...)
{
	va_list argList;
	va_start(argList, pszFormat);

	EnterCriticalSection(&g_cs);
	vfprintf(stdout, pszFormat, argList);
	LeaveCriticalSection(&g_cs);

	va_end(argList);
}

// д�̴߳�ӡ������,��ֹ�����߳��ڻָ���ɫ֮ǰ�����жϣ�������ɫ�������ȷ
void writerPrint(char* pszFormat, ...)
{
	va_list argList;
	va_start(argList, pszFormat);

	EnterCriticalSection(&g_cs);
	setConsoleColor(FOREGROUND_GREEN);
	vfprintf(stdout, pszFormat, argList);
	setConsoleColor(FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_BLUE);
	LeaveCriticalSection(&g_cs);

	va_end(argList);
}

unsigned int _stdcall readerThreadFun(void *)
{
	readerPrint("%d reader is waiting....\n", GetCurrentThreadId());

	// �ȴ���д�߲Ż��
	WaitForSingleObject(g_hEventNoWriter, INFINITE);

	// ׼�����ļ���������++
	EnterCriticalSection(&g_csReaderandWriter);
	g_nReaderNumber++;
	if (1 == g_nReaderNumber) // ��һ�����ߣ����޶����ź�Ϊδ������֪ͨд�̻߳�������д
	{
		ResetEvent(g_hEventNoReader);
	}
	LeaveCriticalSection(&g_csReaderandWriter);

	//���ļ���
	readerPrint("%d reader begin read file...\n", GetCurrentThreadId());
	Sleep(rand() % 100);
	readerPrint("%d reader end read file...\n", GetCurrentThreadId());
	
	//�����ļ����뿪������--
	EnterCriticalSection(&g_csReaderandWriter);
	g_nReaderNumber--;
	if (0 == g_nReaderNumber) // ���һ�����ߣ����޶����ź�Ϊ������֪ͨд�߳̿���д��
	{
		SetEvent(g_hEventNoReader);
	}
	LeaveCriticalSection(&g_csReaderandWriter);

	return 0;
}

unsigned int _stdcall writerThreadFun(void *)
{
	writerPrint("%d writer is waiting....\n", GetCurrentThreadId());

	// �ȴ��޶��ߣ�������д
	WaitForSingleObject(g_hEventNoReader, INFINITE);

	// ׼��д�ļ���д����++
	EnterCriticalSection(&g_csReaderandWriter);
	g_nWriterNumber++;
	if (1 == g_nWriterNumber) // ��һ��д�ߣ�����д���ź�Ϊδ������֪ͨ���̻߳������Զ�
	{
		ResetEvent(g_hEventNoWriter);
	}
	LeaveCriticalSection(&g_csReaderandWriter);

	//д�ļ���
	writerPrint("%d writer begin write file...\n", GetCurrentThreadId());
	Sleep(rand() % 100);
	writerPrint("%d writer end write file...\n", GetCurrentThreadId());

	//д���ļ����뿪д����--
	EnterCriticalSection(&g_csReaderandWriter);
	g_nWriterNumber--;
	if (0 == g_nWriterNumber) // ���һ��д�ߣ�����д���ź�Ϊ������֪ͨ���߳̿���д��
	{
		SetEvent(g_hEventNoWriter);
	}
	LeaveCriticalSection(&g_csReaderandWriter);

	return 0;
}

void testThreadWithReadAndWrite(void)
{
	printf("multi-thread read and write with Event:\n");

	InitializeCriticalSection(&g_cs);
	InitializeCriticalSection(&g_csReaderandWriter);

	srand(time(NULL));
	// ����ʼ��Ϊ�Զ������޶����¼��Ǵ�������д���¼���δ����
	g_hEventNoReader = CreateEvent(NULL, FALSE, TRUE, NULL);
	g_hEventNoWriter = CreateEvent(NULL, FALSE, FALSE, NULL);

	g_nReaderNumber = 0;
	g_nWriterNumber = 0;

	HANDLE handle[MAX_READER_NUM+MAX_WRITER_NUM];
	// �ֱ𴴽�����д�߳�
	for (int i = 0; i < MAX_READER_NUM; i++)
	{
		handle[i] = (HANDLE)_beginthreadex(NULL, 0, readerThreadFun, NULL, 0, NULL);
	}
	for (int i = MAX_READER_NUM; i < MAX_READER_NUM + MAX_WRITER_NUM; i++)
	{
		handle[i] = (HANDLE)_beginthreadex(NULL, 0, writerThreadFun, NULL, 0, NULL);
	}

	WaitForMultipleObjects(MAX_READER_NUM + MAX_WRITER_NUM, handle, TRUE, INFINITE);

	CloseHandle(g_hEventNoReader);
	CloseHandle(g_hEventNoWriter);

	DeleteCriticalSection(&g_cs);
	DeleteCriticalSection(&g_csReaderandWriter);
}

/*
��д����SRWLock��VS2008�������ϲ�֧�֣�
��д���ڶ���Դ���б�����ͬʱ������������Ҫ��ȡ��Դֵ���̣߳���ȡ���̣߳�����Ҫ������Դ���̣߳�д�����̣߳���
���ڶ�ȡ���̣߳���д�����������ǲ�����ִ�С�����д�����߳���ռ����Դʱ����д����������д�����̺߳Ͷ�ȡ���̵߳ȴ�
*/

SRWLOCK g_swlock;

unsigned int _stdcall readerThreadFun2(void *)
{
	readerPrint("%d reader is waiting....\n", GetCurrentThreadId());

	// �����������
	AcquireSRWLockShared(&g_swlock);

	//���ļ���
	readerPrint("%d reader begin read file...\n", GetCurrentThreadId());
	Sleep(rand() % 100);
	readerPrint("%d reader end read file...\n", GetCurrentThreadId());

	//�����ļ�
	ReleaseSRWLockShared(&g_swlock);

	return 0;
}

unsigned int _stdcall writerThreadFun2(void *)
{
	writerPrint("%d writer is waiting....\n", GetCurrentThreadId());

	// д������д��
	AcquireSRWLockExclusive(&g_swlock);

	//д�ļ���
	writerPrint("%d writer begin write file...\n", GetCurrentThreadId());
	Sleep(rand() % 100);
	writerPrint("%d writer end write file...\n", GetCurrentThreadId());

	//д���ļ�
	ReleaseSRWLockExclusive(&g_swlock);

	return 0;
}


void testThreadWithReadAndWrite2(void)
{
	printf("multi-thread read and write with SWRLock:\n");

	InitializeCriticalSection(&g_cs);
	InitializeSRWLock(&g_swlock);

	srand(time(NULL));
	HANDLE handle[MAX_READER_NUM + MAX_WRITER_NUM];
	// �ֱ𴴽�����д�߳�
	for (int i = 0; i < MAX_READER_NUM; i++)
	{
		handle[i] = (HANDLE)_beginthreadex(NULL, 0, readerThreadFun2, NULL, 0, NULL);
	}
	for (int i = MAX_READER_NUM; i < MAX_READER_NUM + MAX_WRITER_NUM; i++)
	{
		handle[i] = (HANDLE)_beginthreadex(NULL, 0, writerThreadFun2, NULL, 0, NULL);
	}

	WaitForMultipleObjects(MAX_READER_NUM + MAX_WRITER_NUM, handle, TRUE, INFINITE);

	DeleteCriticalSection(&g_cs);
}


/*
˫�̶߳�д��������
*/
#define MAX_READ_QUEUE_THREAD_NUM 5
#define MAX_WRITE_QUEUE_THREAD_NUM 5

const int QUEUE_LEN = 50;
int g_queue[QUEUE_LEN];
int g_readdata;
int g_writedata;

unsigned int _stdcall readQueueThread(void*)
{
	while (g_readdata < 2* QUEUE_LEN)
	{
		WaitForSingleObject(g_hNotEmptySemaphore, INFINITE);

		EnterCriticalSection(&g_cs);
		g_readdata = g_queue[g_j];
		g_j = (g_j + 1) % QUEUE_LEN;
		readerPrint("%d thread read %d from queue\n", GetCurrentThreadId(), g_readdata);
		LeaveCriticalSection(&g_cs);

		Sleep(rand() % 100);
		ReleaseSemaphore(g_hEmptySemaphore, 1, NULL);
	}

	return 0;
}

unsigned int _stdcall writeQueueThread(void*)
{

	while (g_writedata < 2* QUEUE_LEN)
	{
		WaitForSingleObject(g_hEmptySemaphore, INFINITE); 

		EnterCriticalSection(&g_cs);
		g_queue[g_i] = ++g_writedata;
		g_i = (g_i + 1) % QUEUE_LEN;
		writerPrint("%d thread write %d into queue\n", GetCurrentThreadId(), g_writedata);
		LeaveCriticalSection(&g_cs);

		Sleep(rand() % 100);
		ReleaseSemaphore(g_hNotEmptySemaphore, 1, NULL);
	}

	return 0;
}

void testThreadWithReadAndWriteQueue(void)
{
	printf("multi-thread read and write queue:\n");
	
	InitializeCriticalSection(&g_cs);
	// ��ʼ���ǿ��ź���Ϊδ���������ź���Ϊ����
	g_hNotEmptySemaphore = CreateSemaphore(NULL, 0, QUEUE_LEN, NULL);
	g_hEmptySemaphore = CreateSemaphore(NULL, QUEUE_LEN, QUEUE_LEN, NULL);

	srand(time(NULL));
	g_i = 0;
	g_j = 0;
	g_readdata = 0;
	g_writedata = 0;

	HANDLE handle[MAX_READ_QUEUE_THREAD_NUM + MAX_WRITE_QUEUE_THREAD_NUM];
	for (int i = 0 ; i < MAX_READ_QUEUE_THREAD_NUM; i++)
	{
		handle[i] = (HANDLE)_beginthreadex(NULL, 0, readQueueThread, NULL, 0, NULL);
	}
	for (int i = MAX_READ_QUEUE_THREAD_NUM; i < MAX_READ_QUEUE_THREAD_NUM + MAX_WRITE_QUEUE_THREAD_NUM; i++)
	{
		handle[i] = (HANDLE)_beginthreadex(NULL, 0, writeQueueThread, NULL, 0, NULL);
	}

	WaitForMultipleObjects(MAX_READ_QUEUE_THREAD_NUM + MAX_WRITE_QUEUE_THREAD_NUM, handle, TRUE, INFINITE);

	CloseHandle(g_hNotEmptySemaphore);
	CloseHandle(g_hEmptySemaphore);
	DeleteCriticalSection(&g_cs);
}



/*
����������
���߳�ѭ�� 10 �Σ��������߳�ѭ�� 20 �Σ������ֻص����߳�ѭ�� 10 �Σ�
�����ٻص����߳���ѭ�� 20 �Σ����ѭ��50��
*/

const int maxLoop = 10;
const int subThreadLoop = 10;
const int mainThreadLoop = 20;

// ��ɫ��ӡ������,��ֹ�������߳��ڻָ���ɫ֮ǰ�����жϣ�������ɫ�������ȷ
void colorPrint(WORD color, char* pszFormat, ...)
{
	va_list argList;
	va_start(argList, pszFormat);

	EnterCriticalSection(&g_cs);
	setConsoleColor(color);
	vfprintf(stdout, pszFormat, argList);
	setConsoleColor(FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_BLUE);
	LeaveCriticalSection(&g_cs);

	va_end(argList);
}



// һ���ź������������߳̽����ȡ���ź�������sleep����
// Ҳ������Event��Mutex

unsigned int _stdcall subThread(void*)
{
	for(int i = 0; i < maxLoop; i++)
	{
		WaitForSingleObject(g_hSemaphore, INFINITE);

		for (int j = 0; j < subThreadLoop; j++)
		{
			colorPrint(FOREGROUND_GREEN ,"%d sub thread inner loop = %d [total loop = %d]\n", GetCurrentThreadId(), j, i);
		}

		ReleaseSemaphore(g_hSemaphore, 1, NULL);
		Sleep(50); // �����ź��������ó�����Ȩ
	}
	
	return 0;
}

void testMainAndSubThreadLoop(void)
{
	printf("sub run 10 - main run 20 loop :\n");

	InitializeCriticalSection(&g_cs);
	//��ʼ�ź���Ϊδ������������Ϊ1
	g_hSemaphore = CreateSemaphore(NULL, 0, 1, NULL);

	HANDLE handle = (HANDLE)_beginthreadex(NULL, 0, subThread, NULL, 0, NULL);
	ReleaseSemaphore(g_hSemaphore, 1, NULL); 
	Sleep(10); // �����߳��Ȼ�ȡ��ʼ�������ź���

	for (int i = 0; i < maxLoop; i++)
	{
		WaitForSingleObject(g_hSemaphore, INFINITE);

		for (int j = 0; j < mainThreadLoop; j++)
		{
			colorPrint(FOREGROUND_INTENSITY, "%d main thread inner loop = %d [total loop = %d]\n", GetCurrentThreadId(), j, i);
		}
		ReleaseSemaphore(g_hSemaphore, 1, NULL);
		Sleep(50); // �����ź��������ó�����Ȩ
	}

	CloseHandle(g_hSemaphore);
	DeleteCriticalSection(&g_cs);

}

// �����ź������������̷ֱ߳��ȡ���Լ����ź������Լ��ź����Ĵ������ɶԷ����������ƣ�����
// Ҳ������Event��Mutex

HANDLE g_hSubSemaphore;
HANDLE g_hMainSemaphore;

unsigned int _stdcall subThread2(void*)
{
	for (int i = 0; i < maxLoop; i++)
	{
		// �ȴ��Լ����ź�����
		WaitForSingleObject(g_hSubSemaphore, INFINITE);

		for (int j = 0; j < subThreadLoop; j++)
		{
			colorPrint(FOREGROUND_GREEN, "%d sub thread inner loop = %d [total loop = %d]\n", GetCurrentThreadId(), j, i);
		}
		//����main�̵߳��ź���
		ReleaseSemaphore(g_hMainSemaphore, 1, NULL);
		//Sleep(50);  // ����˯���ó�����Ȩ
	}

	return 0;
}

void testMainAndSubThreadLoop2(void)
{
	printf("sub run 10 - main run 20 loop ver 2:\n");

	InitializeCriticalSection(&g_cs);
	//��ʼ�����ź���Ϊδ������������Ϊ1
	g_hSubSemaphore = CreateSemaphore(NULL, 0, 1, NULL);
	g_hMainSemaphore = CreateSemaphore(NULL, 0, 1, NULL);

	HANDLE handle = (HANDLE)_beginthreadex(NULL, 0, subThread2, NULL, 0, NULL);
	ReleaseSemaphore(g_hSubSemaphore, 1, NULL); //�ȴ������̵߳��ź���

	for (int i = 0; i < maxLoop; i++)
	{
		// �ȴ��Լ����ź�����
		WaitForSingleObject(g_hMainSemaphore, INFINITE);

		for (int j = 0; j < mainThreadLoop; j++)
		{
			colorPrint(FOREGROUND_INTENSITY, "%d main thread inner loop = %d [total loop = %d]\n", GetCurrentThreadId(), j, i);
		}

		//�������̵߳��ź���
		ReleaseSemaphore(g_hSubSemaphore, 1, NULL);
		//Sleep(50);  // ����˯���ó�����Ȩ
	}

	CloseHandle(g_hSubSemaphore);
	CloseHandle(g_hMainSemaphore);
	DeleteCriticalSection(&g_cs);
}


/*
����������
��дһ�����򣬿���3���̣߳���3���̵߳�ID�ֱ�ΪA��B��C��ÿ���߳̽��Լ���ID����Ļ�ϴ�ӡ10�飬
Ҫ�����������밴ABC��˳����ʾ���磺ABCABC��.���ε���
*/

// �����ź������������̷ֱ߳��ȡ���Լ����ź������Լ��ź����Ĵ���������һ�����߳����������ƣ�����
// Ҳ������Event
// ����ʹ��semaphoreʵ�֣�������Mutex��CriticalSection ���߾����߳�����Ȩ��һ��������������


HANDLE  g_hSubSemaphoreA, g_hSubSemaphoreB, g_hSubSemaphoreC;
#define MAX_LOOP_NUM 30

unsigned int _stdcall subThreadA(void*)
{
	for (int  i = 0; i < MAX_LOOP_NUM; i++)
	{
		WaitForSingleObject(g_hSubSemaphoreA, INFINITE);
		colorPrint(FOREGROUND_INTENSITY, "%d sub thread A\n", GetCurrentThreadId());
		ReleaseSemaphore(g_hSubSemaphoreB, 1, NULL);
	}

	return 0;
}

unsigned int _stdcall subThreadB(void*)
{
	for (int i = 0; i < MAX_LOOP_NUM; i++)
	{
		WaitForSingleObject(g_hSubSemaphoreB, INFINITE);
		colorPrint(FOREGROUND_GREEN, "%d sub thread B\n", GetCurrentThreadId());
		ReleaseSemaphore(g_hSubSemaphoreC, 1, NULL);
	}

	return 0;
}

unsigned int _stdcall subThreadC(void*)
{
	for (int i = 0; i < MAX_LOOP_NUM; i++)
	{
		WaitForSingleObject(g_hSubSemaphoreC, INFINITE);
		colorPrint(FOREGROUND_RED, "%d sub thread C\n", GetCurrentThreadId());
		ReleaseSemaphore(g_hSubSemaphoreA, 1, NULL);
	}
	return 0;
}

void testThreeSequenceSubThreadLoop(void)
{
	printf("A - B - C loop %d times:\n", MAX_LOOP_NUM);

	InitializeCriticalSection(&g_cs);

	// ��ʼ��g_hSubSemaphoreA�ź���Ϊ����̬������Ϊ�Ǵ���̬
	g_hSubSemaphoreA = CreateSemaphore(NULL, 1, 1, NULL);
	g_hSubSemaphoreB = CreateSemaphore(NULL, 0, 1, NULL);
	g_hSubSemaphoreC = CreateSemaphore(NULL, 0, 1, NULL);

	ReleaseMutex(g_hSubSemaphoreA); // ���ȴ���A��ʼ����

	HANDLE handle[3];
	handle[0] = (HANDLE)_beginthreadex(NULL, 0, subThreadA, NULL, 0, NULL);
	handle[1] = (HANDLE)_beginthreadex(NULL, 0, subThreadB, NULL, 0, NULL);
	handle[2] = (HANDLE)_beginthreadex(NULL, 0, subThreadC, NULL, 0, NULL);

	WaitForMultipleObjects(3, handle, TRUE, INFINITE);

	CloseHandle(g_hSubSemaphoreA);
	CloseHandle(g_hSubSemaphoreB);
	CloseHandle(g_hSubSemaphoreC);

	DeleteCriticalSection(&g_cs);
}

int main()
{
	//testThread();
	//testThreadEx();
	//testThread2();
	//testThread2WithCriticalSection();
	//testThread2WithEvent();
	//testThread2WithMutex();
	//testThread2WithSemaphore();
	//testThreadWithSingleProducerAndConsumer();
	//testThreadWithMultiProducerAndConsumer();
	//testThreadWithReadAndWrite();
	//testThreadWithReadAndWrite2();
	//testThreadWithReadAndWriteQueue();
	//testMainAndSubThreadLoop();
	//testMainAndSubThreadLoop2();
	testThreeSequenceSubThreadLoop();
}

