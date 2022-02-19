#include <intrin.h>
#include <math.h>
#include <stdio.h>
#include <windows.h>

#include "clock.h"

void fillnop(unsigned char *instBuf, unsigned sizeBytes) {
#ifdef __aarch64__
    unsigned nopCnt = sizeBytes / 4;
    unsigned *inst = (unsigned *)instBuf;
    for (int i = 0; i < nopCnt; i++)
        inst[i] = 0xd503201f;
#else
    memset(instBuf, 0x90, sizeBytes);
#endif
}

void rob_test(unsigned char *instBuf, int testCnt, int delayCnt, int codeDupCnt, int codeLoopCnt) {
    int i = 0;
#ifdef __aarch64__
    // Microsft AARCH64 calling convention:
    // X0-X17, v0-v7, v16-v31 volatile, we can use them
    // X18-X30, v8-v15 nonvolatile, we can't use them
    // https://docs.microsoft.com/en-us/cpp/build/arm64-windows-abi-conventions?view=msvc-170
    unsigned int *inst = (unsigned int *)instBuf;
    for (int k = 0; k < codeDupCnt; k++) {
        // generate sqrt d0, d0
        for (int j = 0; j < delayCnt; j++) {
            inst[i++] = 0x1e61c000;
        }

        // generate nop
        for (int j = 0; j < testCnt; j++) {
            inst[i++] = 0xd503201f;
        }
    }

    // ret 0xd65f03c0
    inst[i++] = 0xd65f03c0;

    __dmb(_ARM64_BARRIER_SY); // data memory barrier
    __isb(_ARM64_BARRIER_SY); // instruction barrier
#else
    // Microsft X64 calling convention:
    // RAX, RCX, RDX, R8, R9, R10, R11, and XMM0-XMM5 volatile, we can use them without saving
    // RBX, RBP, RDI, RSI, RSP, R12, R13, R14, R15, and XMM6-XMM15 nonvolatile, we can't use them
    // https://docs.microsoft.com/en-us/cpp/build/x64-calling-convention?view=msvc-170
    for (int k = 0; k < codeDupCnt; k++) {
        // generate sqrtsd %xmm0, %xmm0
        for (int j = 0; j < delayCnt; j++) {
            instBuf[i++] = 0xf2;
            instBuf[i++] = 0x0f;
            instBuf[i++] = 0x51;
            instBuf[i++] = 0xc0;
        }

        // generate nop
        for (int j = 0; j < testCnt; j++) {
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
    for (int k = 0; k < 10; k++) {
        size_t start = getclock();
        for (int i = 0; i < codeLoopCnt; i++) {
            ((size_t(*)(size_t, size_t))instBuf)(r0, r1);
        }
        size_t end = getclock();
        size_t clock = end - start;
        if (min > clock)
            min = clock;
    }

    printf("%lld, ", min / (codeLoopCnt * codeDupCnt));
}

int main(int argc, char *argv[]) {
    int nopBase = 100;
    int nopEnd = 1000;
    int nopStep = 10;
    int delayCnt = 10;
    int codeDupCnt = 64;
    int codeLoopCnt = 1000;

    for (int i = 1; i < argc; i += 2) {
        if (strcmp(argv[i], "-start") == 0)
            nopBase = atoi(argv[i + 1]);
        else if (strcmp(argv[i], "-end") == 0)
            nopEnd = atoi(argv[i + 1]);
        else if (strcmp(argv[i], "-step") == 0)
            nopStep = atoi(argv[i + 1]);
        else if (strcmp(argv[i], "-delay") == 0)
            delayCnt = atoi(argv[i + 1]);
        else if (strcmp(argv[i], "-dup") == 0)
            codeDupCnt = atoi(argv[i + 1]);
        else if (strcmp(argv[i], "-loop") == 0)
            codeLoopCnt = atoi(argv[i + 1]);
        else {
            printf("rob -start 100\n"
                   "    -end 1000\n"
                   "    -step 10\n"
                   "    -delay 8\n"
                   "    -dup   64\n"
                   "    -loop  1000\n");
            return 0;
        }
    }

    printf("testStart:%d, testEnd:%d, testStep:%d, delayCnt:%d, codeDupCnt:%d, codeLoopCnt:%d\n", nopBase, nopEnd,
           nopStep, delayCnt, codeDupCnt, codeLoopCnt);
    SetProcessAffinityMask(GetCurrentProcess(), 0x10);
    SetProcessPriorityBoost(GetCurrentProcess(), true);
    SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS);
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);
    unsigned char *code = (unsigned char *)VirtualAlloc(0, 0x1001000, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
    unsigned char *instBuf = (unsigned char *)((size_t)(code + 0xfff) & (~0xfff));
    fillnop(instBuf, 0x1000000);

    for (int testCnt = nopBase; testCnt < nopEnd; testCnt += nopStep) {
        printf("%d, ", testCnt);
        rob_test(instBuf, testCnt, delayCnt, codeDupCnt, codeLoopCnt);
        printf("\n");
    }
    VirtualFree(code, 0x1001000, MEM_RELEASE);
    return 0;
}