#include <windows.h>
#include <intrin.h>
#include <stdio.h>
#include <math.h>

#include "clock.h"

void fillnop(unsigned char *instBuf, unsigned sizeBytes)
{
#ifdef __aarch64__
    unsigned nopCnt = sizeBytes / 4;
    unsigned *inst = (unsigned *)instBuf;
    for (int i = 0; i < nopCnt; i++)
        inst[i] = 0xd503201f;
#else
    memset(instBuf, 0x90, sizeBytes);
#endif
}

void rob_test(unsigned char *instBuf, int nopCnt, int sqrtCnt)
{
    int i = 0;
    const static int GenCodeCnt = 500;
#ifdef __aarch64__
    // double d = (double)rand();
    // d = sqrt(d);

    // generate sqrt d0, d0
    unsigned int *inst = (unsigned int *)instBuf;
    for (int k = 0; k < GenCodeCnt; k++)
    {
        for (int j = 0; j < sqrtCnt; j++)
        {
            inst[i++] = 0x1e61c000;
        }

        // generate nop
        for (int j = 0; j < nopCnt; j++)
        {
            inst[i++] = 0xd503201f;
        }
    }

    // ret 0xd65f03c0
    inst[i++] = 0xd65f03c0;
#else
    for (int k = 0; k < GenCodeCnt; k++)
    {
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
    }
    // generate ret
    instBuf[i++] = 0xc3;
#endif

    // warm icache
    ((void (*)())instBuf)();

    unsigned long long min = -1ull;
    const static int LoopCnt = 500;
    for (int k = 0; k < 10; k++)
    {
        unsigned long long start = getclock();
        for (int i = 0; i < LoopCnt; i++)
        {
            ((void (*)())instBuf)();
        }
        unsigned long long end = getclock();
        unsigned long long clock = end - start;
        if (min > clock)
            min = clock;
    }

    printf("%lld, ", min / (LoopCnt * GenCodeCnt));
}

int main(int argc, char **argv)
{
    SetProcessAffinityMask(GetCurrentProcess(), 0x1);
    SetProcessPriorityBoost(GetCurrentProcess(), true);
    SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS);
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);
    unsigned char *code = (unsigned char *)VirtualAlloc(0, 0x1001000, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
    unsigned char *instBuf = (unsigned char *)((size_t)(code + 0xfff) & (~0xfff));
    fillnop(instBuf, 0x100000);

    for (int nopCnt = 490; nopCnt < 520; nopCnt++)
    {
        int sqrtCnt = 3;
        printf("%d, ", nopCnt + sqrtCnt);
        for (; sqrtCnt < 15; sqrtCnt += 2)
        {
            rob_test(instBuf, nopCnt, sqrtCnt);
        }
        printf("\n");
    }
    VirtualFree(code, 0x1001000, MEM_RELEASE);
    return 0;
}