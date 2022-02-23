#include <intrin.h>
#include <math.h>
#include <stdio.h>
#include <windows.h>

#include "clock.h"

// Oline compiler
// https://godbolt.org/

// Online asm generate
// https://disasm.pro/
// https://armconverter.com/
// https://defuse.ca/online-x86-assembler.htm

enum DispatchCase {
    AddNop,    // Test Add Dispatch Buffer
    MulNop,    // Test Mul Dispatch Buffer
    AddMulNop, // Test Add/Mul share Dispatch Buffer or not
    TestCaseEnd,
};

const char *TestCaseName[TestCaseEnd] = {"Add + Nop", "Mul + Nop", "1Add&1Mul + Nop"};

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

double genCodeTest(DispatchCase caseId, unsigned char *instBuf, int testCnt, int changePoint, int codeLoopCnt,
                   int recycle) {
    int i = 0;
#ifdef __aarch64__
    // Microsft AARCH64 calling convention:
    // X0-X17, v0-v7, v16-v31 volatile, we can use them
    // X18-X30, v8-v15 nonvolatile, we can't use them
    // https://docs.microsoft.com/en-us/cpp/build/arm64-windows-abi-conventions?view=msvc-170
    unsigned int *inst = (unsigned int *)instBuf;
    for (int j = 0; j < testCnt; j++) {
        int cj = j;
        if (recycle != 0)
            cj %= recycle;
        switch (caseId) {
        case AddNop:
            if (cj < changePoint)
                inst[i++] = 0x8b010020; // add x0, x1, x1
            else
                inst[i++] = 0xd503201f; // nop
            break;
        case MulNop:
            if (cj < changePoint)
                inst[i++] = 0x9b017c20; // mul x0, x1, x1
            else
                inst[i++] = 0xd503201f; // nop
            break;
        case AddMulNop:
            if (cj < changePoint) {
                inst[i++] = 0x8b010020; // add x0, x1, x1
                inst[i++] = 0x9b017c22; // mul x2, x1, x1
            } else
                inst[i++] = 0xd503201f; // nop
            break;
        default:
            return;
        }
    }
    // ret 0xd65f03c0
    inst[i++] = 0xd65f03c0;

    __dmb(_ARM64_BARRIER_SY); // data memory barrier
    __isb(_ARM64_BARRIER_SY); // instruction barrier
#else
    // Microsft X64 calling convention:
    // RAX, RCX, RDX, R8, R9, R10, R11, and XMM0-XMM5 volatile, we can write them without saving
    // RBX, RBP, RDI, RSI, RSP, R12, R13, R14, R15, and XMM6-XMM15 nonvolatile, we can't write them
    // https://docs.microsoft.com/en-us/cpp/build/x64-calling-convention?view=msvc-170

    static unsigned char addByte0[] = {0x48, 0x48, 0x48, 0x49, 0x49, 0x49, 0x49};
    static unsigned char addByte2[] = {0xd8, 0xd9, 0xda, 0xd8, 0xd9, 0xda, 0xdb};
    static unsigned char mulByte0[] = {0x48, 0x48, 0x48, 0x4c, 0x4c, 0x4c, 0x4c};
    static unsigned char mulByte3[] = {0xc3, 0xcb, 0xd3, 0xc3, 0xcb, 0xd3, 0xdb};
    for (int j = 0; j < testCnt; j++) {
        int cj = j;
        if (recycle != 0)
            cj %= recycle;
        switch (caseId) {
        case AddNop:
            if (cj < changePoint) {
                instBuf[i++] = addByte0[j % 7]; // add [rax-r11], rbx
                instBuf[i++] = 0x01;
                instBuf[i++] = addByte2[j % 7];
            } else {
                instBuf[i++] = 0x48; // nop
                instBuf[i++] = 0x89;
                instBuf[i++] = 0xc8;
            }
            break;
        case MulNop:
            if (cj < changePoint) {
                instBuf[i++] = mulByte0[j % 7]; // imul [rax-r11], rbx
                instBuf[i++] = 0x0f;
                instBuf[i++] = 0xaf;
                instBuf[i++] = mulByte3[j % 7];
            } else {
                instBuf[i++] = 0x48; // nop
                instBuf[i++] = 0x89;
                instBuf[i++] = 0xc8;
            }
            break;
        case AddMulNop:
            if (cj < changePoint) {
                instBuf[i++] = addByte0[j % 7]; // add [rax-r11], rbx
                instBuf[i++] = 0x01;
                instBuf[i++] = addByte2[j % 7];
                instBuf[i++] = mulByte0[j % 7]; // imul [rax-r11], rbx
                instBuf[i++] = 0x0f;
                instBuf[i++] = 0xaf;
                instBuf[i++] = mulByte3[j % 7];
            } else {
                instBuf[i++] = 0x48; // nop
                instBuf[i++] = 0x89;
                instBuf[i++] = 0xc8;
            }
            break;
        default:
            return -1;
        }
    }
    // ret
    instBuf[i++] = 0xc3;
#endif

    size_t r0 = 1;
    size_t r1 = 1;
    // __debugbreak();
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

    double clockMin = (double)min / codeLoopCnt;
    return clockMin;
}

int main(int argc, char *argv[]) {
    int testBase = 1;
    int testEnd = 200;
    int testStep = 1;
    int codeLoopCnt = 100000;
    int lines = 10;
    DispatchCase caseId = AddNop;

    for (int i = 1; i < argc; i += 2) {
        if (strcmp(argv[i], "-case") == 0)
            caseId = static_cast<DispatchCase>(atoi(argv[i + 1]));
        else if (strcmp(argv[i], "-start") == 0)
            testBase = atoi(argv[i + 1]);
        else if (strcmp(argv[i], "-end") == 0)
            testEnd = atoi(argv[i + 1]);
        else if (strcmp(argv[i], "-step") == 0)
            testStep = atoi(argv[i + 1]);
        else if (strcmp(argv[i], "-lines") == 0)
            lines = atoi(argv[i + 1]);
        else if (strcmp(argv[i], "-loop") == 0)
            codeLoopCnt = atoi(argv[i + 1]);
        else {
            printf("uperf_dispatch -case 0\n"
                   "    -start 1\n"
                   "    -end 400\n"
                   "    -step 1\n"
                   "    -lines 10\n"
                   "    -loop  100000\n");
            return 0;
        }
    }

    if (caseId < 0 || caseId >= TestCaseEnd) {
        for (int i = 0; i < TestCaseEnd; i++) {
            printf("%d %s\n", i, TestCaseName[i]);
        }
        return 0;
    }

    unsigned char *code = (unsigned char *)VirtualAlloc(0, 0x1001000, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
    unsigned char *instBuf = (unsigned char *)((size_t)(code + 0xfff) & (~0xfff));
    fillnop(instBuf, 0x1000000);

    printf("case: %s\nstart:%d, end:%d, step:%d, lines:%d, loop:%d\n", TestCaseName[caseId], testBase, testEnd,
           testStep, lines, codeLoopCnt);
    SetProcessAffinityMask(GetCurrentProcess(), 0x10);
    SetProcessPriorityBoost(GetCurrentProcess(), true);
    SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS);
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);

    double testClock = genCodeTest(caseId, instBuf, 100, 100, 100000, 0);
    unsigned schQueueIPC = (unsigned)(100 / testClock + 0.5);
    testClock = genCodeTest(caseId, instBuf, 100, 0, 100000, 0);
    unsigned mapIPC = (unsigned)(100 / testClock + 0.5);
    printf("test Inst IPC:%d/%d\n", schQueueIPC, mapIPC);

    for (int testCnt = testBase; testCnt < testEnd; testCnt += testStep) {
        printf("%d, ", testCnt);
        for (int N = 1; N <= lines; N++) {
            double clockMin = genCodeTest(caseId, instBuf, testCnt, schQueueIPC * N, codeLoopCnt, mapIPC * N);
            printf("%.1f, ", clockMin);
        }
        printf("\n");
    }
    VirtualFree(code, 0x1001000, MEM_RELEASE);

    return 0;
}