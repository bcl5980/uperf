#include <windows.h>
#include <intrin.h>
#include <stdio.h>
#include <math.h>

void rob_test(unsigned char *instBuf, int nopCnt, int sqrtCnt)
{
	int i = 0;
	// generate sqrtsd %xmm0, %xmm0
	for (int j = 0; j < sqrtCnt; j++)
	{
		instBuf[i++] = 0xf2;
		instBuf[i++] = 0x0f;
		instBuf[i++] = 0x51;
		instBuf[i++] = 0xc0;
	}

	// generate nop
	for (int j = 0; j < nopCnt; j++)
	{
		instBuf[i++] = 0x90;
	}
	// generate ret
	instBuf[i++] = 0xc3;

	// warm icache
	((void (*)())instBuf)();

	unsigned int ui;
	unsigned long long start = __rdtscp(&ui);
	const static int LoopCnt = 10000;
	for (int i = 0; i < LoopCnt; i++)
	{
		((void (*)())instBuf)();
	}
	unsigned long long end = __rdtscp(&ui);
	unsigned long long clock = end - start;
	printf("%lld, ", clock / LoopCnt);
}

int main(int argc, char **argv)
{
	SetProcessAffinityMask(GetCurrentProcess(), 0x1);
	SetProcessPriorityBoost(GetCurrentProcess(), true);
	SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS);
	SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);
	unsigned char *code = (unsigned char *)VirtualAlloc(0, 0x100000, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	unsigned char *instBuf = (unsigned char*)((size_t)(code + 0xfff) & (~0xfff));
	for (int nopCnt = 100; nopCnt < 1000; nopCnt += 10)
	{
		printf("%d, ", nopCnt);
		for (int sqrtCnt = 3; sqrtCnt < 15; sqrtCnt++)
		{
			rob_test(instBuf, nopCnt, sqrtCnt);
		}
		printf("\n");
	}
	VirtualFree(code, 0x100000, MEM_RELEASE);
	return 0;
}