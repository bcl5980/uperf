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

void phyreg_test(unsigned char *instBuf, int addCnt, int nopCnt)
{
    int i = 0;
    const static int GenCodeCnt = 64;
#ifdef __aarch64__
    // generate add x0, x5, x5
    unsigned int *inst = (unsigned int *)instBuf;
    for (int k = 0; k < GenCodeCnt; k++)
    {
        for (int j = 0; j < addCnt; j++)
        {
            inst[i++] = 0x8b0500a0;
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
        // generate: add rax, rbx            --> 0x48, 0x01, 0xd8
        // generate: addss xmm0, xmm1        --> 0xf3, 0x0f, 0x58, 0xc1
        // generate: vaddps ymm0, ymm1, ymm1 --> 0xc5, 0xf4, 0x58, 0xc1
        for (int j = 0; j < addCnt; j++)
        {
            instBuf[i++] = 0x48;
            instBuf[i++] = 0x01;
            instBuf[i++] = 0xd8;

            // instBuf[i++] = 0xf3;
            // instBuf[i++] = 0x0f;
            // instBuf[i++] = 0x58;
            // instBuf[i++] = 0xc1;

            // instBuf[i++] = 0xc5;
            // instBuf[i++] = 0xf4;
            // instBuf[i++] = 0x58;
            // instBuf[i++] = 0xc1;
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

int main(int argc, char *argv[])
{
    int addBase = 100;
    int addEnd = 200;
    int addStep = 4;
    int nopCnt = 800;

    for (int i = 1; i < argc; i += 2)
    {
        if (strcmp(argv[i], "-start") == 0)
            addBase = atoi(argv[i + 1]);
        else if (strcmp(argv[i], "-end") == 0)
            addEnd = atoi(argv[i + 1]);
        else if (strcmp(argv[i], "-step") == 0)
            addStep = atoi(argv[i + 1]);
        else if (strcmp(argv[i], "-nop") == 0)
            nopCnt = atoi(argv[i + 1]);
        else
        {
            printf("phyreg -start 100 -end 200 -step 4 -nop 800\n");
            return 0;
        }
    }

    printf("nopStart:%d, addEnd:%d, addStep:%d, nopCnt:%d\n", addBase, addEnd, addStep, nopCnt);
    SetProcessAffinityMask(GetCurrentProcess(), 0x10);
    SetProcessPriorityBoost(GetCurrentProcess(), true);
    SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS);
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);
    unsigned char *code = (unsigned char *)VirtualAlloc(0, 0x1001000, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
    unsigned char *instBuf = (unsigned char *)((size_t)(code + 0xfff) & (~0xfff));
    fillnop(instBuf, 0x1000000);

    for (int addCnt = addBase; addCnt < addEnd; addCnt += addStep)
    {
        printf("%d, ", addCnt);
        phyreg_test(instBuf, addCnt, nopCnt);
        printf("\n");
    }
    VirtualFree(code, 0x1001000, MEM_RELEASE);
    return 0;
}