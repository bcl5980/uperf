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
    const static int GenCodeCnt = 64;
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

    __dmb(_ARM64_BARRIER_SY); // data memory barrier
    __isb(_ARM64_BARRIER_SY); // instruction barrier
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

    size_t r0 = 0;
    size_t r1 = 0;
    // warm icache
    ((size_t(*)(size_t, size_t))instBuf)(r0, r1);

    size_t min = -1ull;
    const static int LoopCnt = 500;
    for (int k = 0; k < 10; k++)
    {
        size_t start = getclock();
        for (int i = 0; i < LoopCnt; i++)
        {
            ((size_t(*)(size_t, size_t))instBuf)(r0, r1);
        }
        size_t end = getclock();
        size_t clock = end - start;
        if (min > clock)
            min = clock;
    }

    printf("%lld, ", min / (LoopCnt * GenCodeCnt));
}

int main(int argc, char *argv[])
{
    int nopBase = 100;
    int nopEnd = 1000;
    int nopStep = 10;
    int sqrtCnt = 10;

    for (int i = 1; i < argc; i += 2)
    {
        if (strcmp(argv[i], "-start") == 0)
            nopBase = atoi(argv[i + 1]);
        else if (strcmp(argv[i], "-end") == 0)
            nopEnd = atoi(argv[i + 1]);
        else if (strcmp(argv[i], "-step") == 0)
            nopStep = atoi(argv[i + 1]);
        else if (strcmp(argv[i], "-sqrt") == 0)
            sqrtCnt = atoi(argv[i + 1]);
        else
        {
            printf("rob -start 100 -end 1000 -step 10 -sqrt 8\n");
            return 0;
        }
    }

    printf("nopStart:%d, nopEnd:%d, nopStep:%d, sqrtCnt:%d\n", nopBase, nopEnd, nopStep, sqrtCnt);
    SetProcessAffinityMask(GetCurrentProcess(), 0x10);
    SetProcessPriorityBoost(GetCurrentProcess(), true);
    SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS);
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);
    unsigned char *code = (unsigned char *)VirtualAlloc(0, 0x1001000, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
    unsigned char *instBuf = (unsigned char *)((size_t)(code + 0xfff) & (~0xfff));
    fillnop(instBuf, 0x1000000);

    for (int nopCnt = nopBase; nopCnt < nopEnd; nopCnt += nopStep)
    {
        printf("%d, ", nopCnt);
        rob_test(instBuf, nopCnt, sqrtCnt);
        printf("\n");
    }
    VirtualFree(code, 0x1001000, MEM_RELEASE);
    return 0;
}