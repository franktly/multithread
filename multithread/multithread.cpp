// multithread.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <stdio.h>
#include <process.h>
#include <windows.h>
#include <time.h>

/************************************************************************/
/* 
Windows multi-thread：

概念：
1. 同步和互斥：
当线程并发执行时，由于资源共享和线程协作，使用线程之间会存在以下两种制约关系。
（1）间接相互制约（互斥）：一个系统中的多个线程必然要共享某种系统资源，如共享CPU，共享I/O设备，线程A在使用共享资源时候，其它线程都要等待。
（2）直接相互制约（同步）： 这种制约主要是因为线程之间的合作，如有线程A将计算结果提供给线程B作进一步处理，那么线程B在线程A将数据送达之前都将处于阻塞状态。
对于互斥可以这样理解：线程A和线程B互斥访问某个资源则它们之间就会产个顺序问题——要么线程A等待线程B操作完毕，要么线程B等待线程操作完毕，这其实就是线程的同步了。
因此，同步包括互斥，互斥是一种特殊的同步

2. 临界资源和临界区
临界资源： 在一段时间内只允许一个线程访问的资源，计算机中大多数物理设备，它们要求被互斥的访问。
临界区： 每个进程中访问临界资源的代码

3. 实现同步和互斥方法：
1）关键段CriticalSection
2）互斥量Mutex
3）事件Event
4）信号量Semaphore
PS：
1）关键段与互斥量都有线程所有权概念（PV操作在同一个线程上进行）。互斥量能很好的处理遗弃情况；
2）互斥量，事件，信号量都是内核对象，可以跨进程使用（通过OpenMutex，OpenEvent，OpenSemaphore）

4. PV操作：
PV操作由P操作原语和V操作原语组成（原语也叫原子操作Atomic Operation，是不可中断的过程）
，对信号量进行操作，具体定义如下：
P(S)：
① 将信号量S的值减1，即S=S-1；
② 如果S>=0，则该进程继续执行；否则该进程置为等待状态。

V(S)：
① 将信号量S的值加1，即S=S+1；
② 该进程继续执行；如果该信号的等待队列中有等待进程就唤醒一等待进程。
*/
/************************************************************************/



volatile long g_nLogicCount;
const int THREAD_NUM = 10;  // 最多创建64个线程

/*
一般的共享全局变量资源（无互斥和同步）
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
	//重复20次以便观察多线程访问同一资源时导致的冲突 
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
		printf("有%d个用户登录后记录结果是%d[repeat = %d]\n", THREAD_NUM, g_nLogicCount, repeatCount);
	}
}

/*
增减和赋值操作互斥：

Windows系统提供了一些以Interlocked开头的函数保证对一个值的增减和赋值操作是原子操作——即不可打断性
1.增减操作:
LONG__cdeclInterlockedIncrement(LONG volatile* Addend);
LONG__cdeclInterlockedDecrement(LONG volatile* Addend);
返回变量执行增减操作之后的值:
LONG__cdec InterlockedExchangeAdd(LONG volatile* Addend, LONGValue);
返回运算后的值，加个负数就是减

2.赋值操作
LONG__cdeclInterlockedExchange(LONG volatile* Target, LONGValue);
Value就是新值，函数会返回原先的值
*/
unsigned int _stdcall threadFunEx(void *param)
{
	Sleep(100);
	// 改进(增加原子操作限制)：
	InterlockedIncrement((LPLONG)&g_nLogicCount);
	Sleep(50);
	return 0;
}

void testThreadEx(void)
{
	printf("test multi-thread with interlocked function\n");
	//重复20次以便观察多线程访问同一资源时导致的冲突 
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
		printf("有%d个用户登录后记录结果是%d[repeat = %d]\n", THREAD_NUM, g_nLogicCount, repeatCount);
	}
}

/*
传参数的多线程(无互斥和同步)：
主线程启动10个子线程并将表示子线程序号的变量地址作为参数传递给子线程。
子线程接收参数 -> sleep(50) -> 全局变量++ -> sleep(0) -> 输出参数和全局变量
*/
unsigned int _stdcall threadFun2(void *param)
{
	int threadNum = *((int*)param);
	Sleep(50);
	g_nLogicCount++;
	Sleep(0);
	printf("传入参数的线程编号为%d, 全局变量为%d\n", threadNum, g_nLogicCount);
	
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
		i++; // 新创建的子线程参数不是所想要的，可能是主线程已经修改过的
	}
	WaitForMultipleObjects(THREAD_NUM, handle, TRUE, INFINITE);
}


/*
关键段具有“线程所有权”概念
1. 关键段共初始化化、销毁、进入和离开关键区域四个函数。
2．关键段可以解决线程的互斥问题，但因为具有“线程所有权”，所以无法解决同步问题。
3．推荐关键段与旋转锁配合使用
*/

CRITICAL_SECTION g_csThreadPara, g_csThreadCode;

unsigned int _stdcall threadFun2WithCriticalSection(void *param)
{
	int threadNum = *((int*)param);
	LeaveCriticalSection(&g_csThreadPara); 

	Sleep(50);

	EnterCriticalSection(&g_csThreadCode); // 全局变量的临界区(各子线程间互斥)，可以起到互斥效果
	g_nLogicCount++;
	Sleep(0);
	printf("传入参数的线程编号为%d, 全局变量为%d\n", threadNum, g_nLogicCount);
	LeaveCriticalSection(&g_csThreadCode);

	return 0;
}

// 主线程和子线程同步失败
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
		EnterCriticalSection(&g_csThreadPara); // 主线程和子线程的临界区(线程同步)， 不能同步，
		handle[i] = (HANDLE)_beginthreadex(NULL, 0, threadFun2WithCriticalSection, &i, 0, NULL);
		i++; // 新创建的子线程参数不是所想要的，可能是主线程已经修改过的
	}

	WaitForMultipleObjects(THREAD_NUM, handle, TRUE, INFINITE);
	
	DeleteCriticalSection(&g_csThreadPara);
	DeleteCriticalSection(&g_csThreadCode);
}


/*
1. 事件是内核对象，事件分为手动置位事件和自动置位事件。
   事件Event内部它包含一个使用计数（所有内核对象都有），一个布尔值表示是手动置位事件还是自动置位事件，另一个布尔值用来表示事件有无触发。
2．事件可以由SetEvent()来触发，由ResetEvent()来设成未触发。还可以由PulseEvent()来发出一个事件脉冲。
3．事件可以解决线程间同步问题，因此也能解决互斥问题。
*/
HANDLE g_hEvent;

unsigned int _stdcall threadFun2WithEvent(void *param)
{
	int threadNum = *((int*)param);
	SetEvent(g_hEvent); // V操作，同步主和子线程

	Sleep(50);

	EnterCriticalSection(&g_csThreadCode); // 全局变量的临界区(各子线程间互斥)，可以起到互斥效果
	g_nLogicCount++;
	Sleep(0);
	printf("传入参数的线程编号为%d, 全局变量为%d\n", threadNum, g_nLogicCount);
	LeaveCriticalSection(&g_csThreadCode);

	return 0;
}

// 主线程和子线程同步成功
void testThread2WithEvent(void)
{
	printf("test multi-thread with event\n");

	InitializeCriticalSection(&g_csThreadCode);
	// 自动，默认为未触发
	g_hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	int i = 0;
	HANDLE handle[THREAD_NUM];
	g_nLogicCount = 0;
	while (i < THREAD_NUM)
	{
		handle[i] = (HANDLE)_beginthreadex(NULL, 0, threadFun2WithEvent, &i, 0, NULL);
		WaitForSingleObject(g_hEvent, INFINITE); // P操作
		i++; // 新创建的子线程参数不是所想要的，可能是主线程已经修改过的
	}

	WaitForMultipleObjects(THREAD_NUM, handle, TRUE, INFINITE);

	CloseHandle(g_hEvent);
	DeleteCriticalSection(&g_csThreadCode);
}

/*
1．互斥量是内核对象，它与关键段都有“线程所有权”所以不能用于线程的同步。
2．互斥量能够用于多个进程之间线程互斥问题，并且能完美的解决某进程意外终止所造成的“遗弃”问题
*/
HANDLE g_hMutex;

unsigned int _stdcall threadFun2WithMutex(void *param)
{
	int threadNum = *((int*)param);
	ReleaseMutex(g_hMutex); // V操作，同步主和子线程

	Sleep(50);

	EnterCriticalSection(&g_csThreadCode); // 全局变量的临界区(各子线程间互斥)，可以起到互斥效果
	g_nLogicCount++;
	Sleep(0);
	printf("传入参数的线程编号为%d, 全局变量为%d\n", threadNum, g_nLogicCount);
	LeaveCriticalSection(&g_csThreadCode);

	return 0;
}

// 主线程和子线程同步失败
void testThread2WithMutex(void)
{
	printf("test multi-thread with mutex\n");

	InitializeCriticalSection(&g_csThreadCode);

	//初始化互斥量与关键段 第二个参数为TRUE表示互斥量为创建线程所有
	g_hMutex = CreateMutex(NULL, FALSE, NULL);
	int i = 0;
	HANDLE handle[THREAD_NUM];
	g_nLogicCount = 0;
	while (i < THREAD_NUM)
	{
		handle[i] = (HANDLE)_beginthreadex(NULL, 0, threadFun2WithEvent, &i, 0, NULL);
		WaitForSingleObject(g_hMutex, INFINITE); // P操作
		i++; // 新创建的子线程参数不是所想要的，可能是主线程已经修改过的
	}

	WaitForMultipleObjects(THREAD_NUM, handle, TRUE, INFINITE);

	CloseHandle(g_hMutex);
	DeleteCriticalSection(&g_csThreadCode);
}

/*
1. 当前资源数量大于0，表示信号量处于触发，等于0表示资源已经耗尽故信号量处于末触发。
  在对信号量调用等待函数时，等待函数会检查信号量的当前资源计数，如果大于0(即信号量处于触发状态)，减1后返回让调用线程继续执行。一个线程可以多次调用等待函数来减小信号量
2. 信号量可以解决线程之间的同步问题
3. 信号量是通过计算资源当前剩余量并根据当前剩余量与零比较来决定信号量是处于触发状态或是未触发状态
*/

HANDLE g_hSemaphore;
unsigned int _stdcall threadFun2WithSemaphore(void *param)
{
	int threadNum = *((int*)param);
	ReleaseSemaphore(g_hSemaphore, 1, NULL); // V操作，同步主和子线程

	Sleep(50);

	EnterCriticalSection(&g_csThreadCode); // 全局变量的临界区(各子线程间互斥)，可以起到互斥效果
	g_nLogicCount++;
	Sleep(0);
	printf("传入参数的线程编号为%d, 全局变量为%d\n", threadNum, g_nLogicCount);
	LeaveCriticalSection(&g_csThreadCode);

	return 0;
}

// 主线程和子线程同步成功
void testThread2WithSemaphore(void)
{
	printf("test multi-thread with semaphore\n");

	InitializeCriticalSection(&g_csThreadCode);

	//当前0个资源，最大允许1个同时访问
	g_hSemaphore = CreateSemaphore(NULL, 0, 1, NULL);

	int i = 0;
	HANDLE handle[THREAD_NUM];
	g_nLogicCount = 0;
	while (i < THREAD_NUM)
	{
		handle[i] = (HANDLE)_beginthreadex(NULL, 0, threadFun2WithSemaphore, &i, 0, NULL);
		WaitForSingleObject(g_hSemaphore, INFINITE); // P操作
		i++; // 新创建的子线程参数不是所想要的，可能是主线程已经修改过的
	}

	WaitForMultipleObjects(THREAD_NUM, handle, TRUE, INFINITE);

	CloseHandle(g_hSemaphore);
	DeleteCriticalSection(&g_csThreadCode);
}


/*
生产者消费者问题：
问题描述：有一个生产者在生产产品，这些产品将提供给若干个消费者去消费，
为了使生产者和消费者能并发执行，在两者之间设置一个具有多个缓冲区的缓冲池，
生产者将它生产的产品放入一个缓冲区中，消费者可以从缓冲区中取走产品进行消费，
显然生产者和消费者之间必须保持同步，即不允许消费者到一个空的缓冲区中取产品，
也不允许生产者向一个已经放入产品的缓冲区中再次投放产品

两方面：
1．首先要考虑生产者与消费者对缓冲区操作时的互斥。
2．不管生产者与消费者有多少个，缓冲池有多少个缓冲区。都只有二个同步过程:
分别是生产者要等待有空缓冲区才能投放产品，消费者要等待有非空缓冲区才能去取产品
*/

// 设置vs控制台颜色，方便显示区分生产者和消费者
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
one critical section ：  互斥访问g_nbuffer
two semaphore： 同步，(producer)等待缓存区为空信号和(consumer)等待缓存区不为空信号
or two event;
*/
const int PRODUCT_NUMBER = 50;
int g_nbuffer;
CRITICAL_SECTION g_cs;
HANDLE g_hEmptySemaphore, g_hNotEmptySemaphore;
HANDLE g_hEmptyEvent, g_hNotEmptyEvent;

unsigned int _stdcall producerThreadFun(void*)
{
	// 依次生产PRODUCT_NUMBER个产品放入g_nbuffer
	for (int i = 0 ; i < PRODUCT_NUMBER; i++) 
	{
		// 等到Empty信号,表明缓存为空才produce 
		//WaitForSingleObject(g_hEmptySemaphore, INFINITE);
		WaitForSingleObject(g_hEmptyEvent, INFINITE);

		EnterCriticalSection(&g_cs);
		g_nbuffer = i;
		printf("producer produce %d into buffer\n", g_nbuffer);
		LeaveCriticalSection(&g_cs);

		//通知缓存区有数据了
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
		// 等到Not Empty信号,表明有数据了才consumer 
		//WaitForSingleObject(g_hNotEmptySemaphore, INFINITE);
		WaitForSingleObject(g_hNotEmptyEvent, INFINITE);

		EnterCriticalSection(&g_cs);
		setConsoleColor(FOREGROUND_GREEN);  // 设置为颜色
		printf("consumer consume %d from buffer\n", g_nbuffer);
		setConsoleColor(FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_BLUE); // 还原颜色白色
		
		if (g_nbuffer == PRODUCT_NUMBER) // 消费到最后一个产品完成后就退出消费者线程
		{
			flag = false;
		}
		LeaveCriticalSection(&g_cs);

		//通知缓存区数据被消费了，无数据为空
		//ReleaseSemaphore(g_hEmptySemaphore, 1, NULL);
		SetEvent(g_hEmptyEvent);
	}

	return 0;
}

void testThreadWithSingleProducerAndConsumer(void)
{
	printf("one producer - one consumer - one buffer:\n");

	InitializeCriticalSection(&g_cs);

	// 初始化空信号量为触发态，非空信号量为非触发态，最大个数均为1
	//g_hEmptySemaphore = CreateSemaphore(NULL, 1, 1, NULL);
	//g_hNotEmptySemaphore = CreateSemaphore(NULL, 0, 1, NULL);

	// 初始化空事件为触发态，非空事件为非触发态，均为自动类型调用WaitForSingleObject后自动调用ResetEvent
	g_hEmptyEvent = CreateEvent(NULL, FALSE, TRUE, NULL);
	g_hNotEmptyEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

	// 分别创建producer and consumer线程：
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
one critical section ：  互斥访问g_nbuffer
two semaphore： 同步，(producer)等待缓存区为空信号和(consumer)等待缓存区不为空信号
or two event;
*/
#define  BUFFER_SIZE 10
int g_nmultibuffer[BUFFER_SIZE];
int g_i, g_j;

unsigned int _stdcall producerThreadFun2(void*)
{
	// 依次生产PRODUCT_NUMBER个产品放入g_nmultibuffer
	for (int i = 0; i < PRODUCT_NUMBER; i++)
	{
		// 等到Empty信号,表明缓存为空才produce 
		WaitForSingleObject(g_hEmptySemaphore, INFINITE);

		EnterCriticalSection(&g_cs);
		g_nmultibuffer[g_i] = i;
		printf("producer produce %d into buffer %d \n", g_nmultibuffer[g_i], g_i);
		g_i = (g_i + 1) % BUFFER_SIZE;
		LeaveCriticalSection(&g_cs);

		//通知缓存区有数据了
		ReleaseSemaphore(g_hNotEmptySemaphore, 1, NULL); 
	}
	return 0;
}

unsigned int _stdcall consumerThreadFun2(void*)
{
	volatile bool flag = true;
	while (flag)
	{
		// 等到Not Empty信号,表明有数据了才consumer 
		WaitForSingleObject(g_hNotEmptySemaphore, INFINITE);

		EnterCriticalSection(&g_cs);
		setConsoleColor(FOREGROUND_GREEN);  // 设置为颜色
		printf("consumer consume %d from buffer %d \n", g_nmultibuffer[g_j], g_j);
		setConsoleColor(FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_BLUE); // 还原颜色白色
		if (g_nmultibuffer[g_j] == PRODUCT_NUMBER) // 消费到最后一个产品完成后就退出消费者线程
		{
			flag = false;
		}
		g_j = (g_j + 1) % BUFFER_SIZE;
		LeaveCriticalSection(&g_cs);

		//通知缓存区数据被消费了，无数据为空
		ReleaseSemaphore(g_hEmptySemaphore, 1, NULL);
	}

	return 0;
}

void testThreadWithMultiProducerAndConsumer(void)
{
	printf("one producer - multi consumer(2) - multi buffer(4):\n");

	InitializeCriticalSection(&g_cs);

	// 初始化空信号量为触发态，非空信号量为非触发态，最大个数均为1
	g_hEmptySemaphore = CreateSemaphore(NULL, BUFFER_SIZE, BUFFER_SIZE, NULL);
	g_hNotEmptySemaphore = CreateSemaphore(NULL, 0, BUFFER_SIZE, NULL);

	g_i = 0; // 生产者线程的缓存号
	g_j = 0; // 消费者线程的缓存号

	memset(g_nmultibuffer, 0, sizeof(g_nmultibuffer));

	// 分别创建producer and consumer线程：
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
读写者问题：
问题描述：
有一个写者很多读者，多个读者可以同时读文件，
但写者在写文件时不允许有读者在读文件，同样有读者在读文件时写者也不去能写文件
*/

#define MAX_READER_NUM 20
#define MAX_WRITER_NUM 20

HANDLE g_hEventNoWriter;  // 等待没有写者
HANDLE g_hEventNoReader;  // 等待没有读者

CRITICAL_SECTION g_csReaderandWriter; 

int  g_nReaderNumber;     // 读者数量
int  g_nWriterNumber;     // 写者数量

//  读线程打印,加互斥，防止被写线程意外中断，导致颜色输出不对
void readerPrint(char* pszFormat, ...)
{
	va_list argList;
	va_start(argList, pszFormat);

	EnterCriticalSection(&g_cs);
	vfprintf(stdout, pszFormat, argList);
	LeaveCriticalSection(&g_cs);

	va_end(argList);
}

// 写线程打印，互斥,防止被读线程在恢复颜色之前意外中断，导致颜色输出不正确
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

	// 等待无写者才会读
	WaitForSingleObject(g_hEventNoWriter, INFINITE);

	// 准备读文件，读者数++
	EnterCriticalSection(&g_csReaderandWriter);
	g_nReaderNumber++;
	if (1 == g_nReaderNumber) // 第一个读者，置无读者信号为未触发，通知写线程还不可以写
	{
		ResetEvent(g_hEventNoReader);
	}
	LeaveCriticalSection(&g_csReaderandWriter);

	//读文件：
	readerPrint("%d reader begin read file...\n", GetCurrentThreadId());
	Sleep(rand() % 100);
	readerPrint("%d reader end read file...\n", GetCurrentThreadId());
	
	//读完文件，离开读者数--
	EnterCriticalSection(&g_csReaderandWriter);
	g_nReaderNumber--;
	if (0 == g_nReaderNumber) // 最后一个读者，置无读者信号为触发，通知写线程可以写了
	{
		SetEvent(g_hEventNoReader);
	}
	LeaveCriticalSection(&g_csReaderandWriter);

	return 0;
}

unsigned int _stdcall writerThreadFun(void *)
{
	writerPrint("%d writer is waiting....\n", GetCurrentThreadId());

	// 等待无读者，才允许写
	WaitForSingleObject(g_hEventNoReader, INFINITE);

	// 准备写文件，写者数++
	EnterCriticalSection(&g_csReaderandWriter);
	g_nWriterNumber++;
	if (1 == g_nWriterNumber) // 第一个写者，置无写者信号为未触发，通知读线程还不可以读
	{
		ResetEvent(g_hEventNoWriter);
	}
	LeaveCriticalSection(&g_csReaderandWriter);

	//写文件：
	writerPrint("%d writer begin write file...\n", GetCurrentThreadId());
	Sleep(rand() % 100);
	writerPrint("%d writer end write file...\n", GetCurrentThreadId());

	//写完文件，离开写者数--
	EnterCriticalSection(&g_csReaderandWriter);
	g_nWriterNumber--;
	if (0 == g_nWriterNumber) // 最后一个写者，置无写者信号为触发，通知读线程可以写了
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
	// 均初始化为自动，且无读者事件是触发，无写者事件是未触发
	g_hEventNoReader = CreateEvent(NULL, FALSE, TRUE, NULL);
	g_hEventNoWriter = CreateEvent(NULL, FALSE, FALSE, NULL);

	g_nReaderNumber = 0;
	g_nWriterNumber = 0;

	HANDLE handle[MAX_READER_NUM+MAX_WRITER_NUM];
	// 分别创建读和写线程
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
读写锁：SRWLock（VS2008及其以上才支持）
读写锁在对资源进行保护的同时，还能区分想要读取资源值的线程（读取者线程）和想要更新资源的线程（写入者线程）。
对于读取者线程，读写锁会允许他们并发的执行。当有写入者线程在占有资源时，读写锁会让其它写入者线程和读取者线程等待
*/

SRWLOCK g_swlock;

unsigned int _stdcall readerThreadFun2(void *)
{
	readerPrint("%d reader is waiting....\n", GetCurrentThreadId());

	// 读者申请读锁
	AcquireSRWLockShared(&g_swlock);

	//读文件：
	readerPrint("%d reader begin read file...\n", GetCurrentThreadId());
	Sleep(rand() % 100);
	readerPrint("%d reader end read file...\n", GetCurrentThreadId());

	//读完文件
	ReleaseSRWLockShared(&g_swlock);

	return 0;
}

unsigned int _stdcall writerThreadFun2(void *)
{
	writerPrint("%d writer is waiting....\n", GetCurrentThreadId());

	// 写者申请写锁
	AcquireSRWLockExclusive(&g_swlock);

	//写文件：
	writerPrint("%d writer begin write file...\n", GetCurrentThreadId());
	Sleep(rand() % 100);
	writerPrint("%d writer end write file...\n", GetCurrentThreadId());

	//写完文件
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
	// 分别创建读和写线程
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
双线程读写队列问题
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
	// 初始化非空信号量为未触发，空信号量为触发
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
问题描述：
子线程循环 10 次，接着主线程循环 20 次，接着又回到子线程循环 10 次，
接着再回到主线程又循环 20 次，如此循环50次
*/

const int maxLoop = 10;
const int subThreadLoop = 10;
const int mainThreadLoop = 20;

// 彩色打印，互斥,防止被其他线程在恢复颜色之前意外中断，导致颜色输出不正确
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



// 一个信号量（主和子线程交替获取该信号量）和sleep控制
// 也可以用Event和Mutex

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
		Sleep(50); // 触发信号量，并让出调度权
	}
	
	return 0;
}

void testMainAndSubThreadLoop(void)
{
	printf("sub run 10 - main run 20 loop :\n");

	InitializeCriticalSection(&g_cs);
	//初始信号量为未触发，最大个数为1
	g_hSemaphore = CreateSemaphore(NULL, 0, 1, NULL);

	HANDLE handle = (HANDLE)_beginthreadex(NULL, 0, subThread, NULL, 0, NULL);
	ReleaseSemaphore(g_hSemaphore, 1, NULL); 
	Sleep(10); // 让子线程先获取初始触发的信号量

	for (int i = 0; i < maxLoop; i++)
	{
		WaitForSingleObject(g_hSemaphore, INFINITE);

		for (int j = 0; j < mainThreadLoop; j++)
		{
			colorPrint(FOREGROUND_INTENSITY, "%d main thread inner loop = %d [total loop = %d]\n", GetCurrentThreadId(), j, i);
		}
		ReleaseSemaphore(g_hSemaphore, 1, NULL);
		Sleep(50); // 触发信号量，并让出调度权
	}

	CloseHandle(g_hSemaphore);
	DeleteCriticalSection(&g_cs);

}

// 二个信号量（主和子线程分别获取该自己的信号量，自己信号量的触发则由对方来触发控制）控制
// 也可以用Event和Mutex

HANDLE g_hSubSemaphore;
HANDLE g_hMainSemaphore;

unsigned int _stdcall subThread2(void*)
{
	for (int i = 0; i < maxLoop; i++)
	{
		// 等待自己的信号量来
		WaitForSingleObject(g_hSubSemaphore, INFINITE);

		for (int j = 0; j < subThreadLoop; j++)
		{
			colorPrint(FOREGROUND_GREEN, "%d sub thread inner loop = %d [total loop = %d]\n", GetCurrentThreadId(), j, i);
		}
		//触发main线程的信号量
		ReleaseSemaphore(g_hMainSemaphore, 1, NULL);
		//Sleep(50);  // 无需睡眠让出控制权
	}

	return 0;
}

void testMainAndSubThreadLoop2(void)
{
	printf("sub run 10 - main run 20 loop ver 2:\n");

	InitializeCriticalSection(&g_cs);
	//初始两个信号量为未触发，最大个数为1
	g_hSubSemaphore = CreateSemaphore(NULL, 0, 1, NULL);
	g_hMainSemaphore = CreateSemaphore(NULL, 0, 1, NULL);

	HANDLE handle = (HANDLE)_beginthreadex(NULL, 0, subThread2, NULL, 0, NULL);
	ReleaseSemaphore(g_hSubSemaphore, 1, NULL); //先触发子线程的信号量

	for (int i = 0; i < maxLoop; i++)
	{
		// 等待自己的信号量来
		WaitForSingleObject(g_hMainSemaphore, INFINITE);

		for (int j = 0; j < mainThreadLoop; j++)
		{
			colorPrint(FOREGROUND_INTENSITY, "%d main thread inner loop = %d [total loop = %d]\n", GetCurrentThreadId(), j, i);
		}

		//触发子线程的信号量
		ReleaseSemaphore(g_hSubSemaphore, 1, NULL);
		//Sleep(50);  // 无需睡眠让出控制权
	}

	CloseHandle(g_hSubSemaphore);
	CloseHandle(g_hMainSemaphore);
	DeleteCriticalSection(&g_cs);
}


/*
问题描述：
编写一个程序，开启3个线程，这3个线程的ID分别为A、B、C，每个线程将自己的ID在屏幕上打印10遍，
要求输出结果必须按ABC的顺序显示；如：ABCABC….依次递推
*/

// 三个信号量（三个子线程分别获取该自己的信号量，自己信号量的触发则由上一个子线程来触发控制）控制
// 也可以用Event
// 该例使用semaphore实现，不能用Mutex和CriticalSection 两者具有线程所有权，一般用来用来互斥


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

	// 初始化g_hSubSemaphoreA信号量为触发态，其他为非触发态
	g_hSubSemaphoreA = CreateSemaphore(NULL, 1, 1, NULL);
	g_hSubSemaphoreB = CreateSemaphore(NULL, 0, 1, NULL);
	g_hSubSemaphoreC = CreateSemaphore(NULL, 0, 1, NULL);

	ReleaseMutex(g_hSubSemaphoreA); // 首先触发A开始运行

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

