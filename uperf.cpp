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

// @todo: FAdd should replace the sqrt delay to idiv/udiv
enum DelayCase { DelayNop, DelayIAdd, DelayFAdd, DelayCmp, DelayIAddICmp, DelayIFAdd, DelayLoad, DelayStore, DelayMax };

const char *DelayCaseName[DelayMax] = {"Sqrt Delay + Nop",  "Sqrt Delay + IAdd",    "Sqrt Delay + FAdd",
                                       "Sqrt Delay + Cmp",  "Sqrt Delay + Add&Cmp", "Sqrt Delay + IAdd&FAdd",
                                       "Sqrt Delay + Load", "Sqrt Delay + Store"};

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

void delay_test(DelayCase caseId, unsigned char *instBuf, int testCnt, int delayCnt, int codeDupCnt, int codeLoopCnt,
                size_t *data0, size_t *data1) {
    int i = 0;

#ifdef __aarch64__
    // Microsft AARCH64 calling convention:
    // X0-X17, v0-v7, v16-v31 volatile, we can use them
    // X18-X30, v8-v15 nonvolatile, we can't use them
    // https://docs.microsoft.com/en-us/cpp/build/arm64-windows-abi-conventions?view=msvc-170
    unsigned int *inst = (unsigned int *)instBuf;
    for (int k = 0; k < codeDupCnt; k++) {
        // sqrt d0, d0
        for (int j = 0; j < delayCnt; j++) {
            inst[i++] = 0x1e61c000;
        }

        for (int j = 0; j < testCnt; j++) {
            switch (caseId) {
            // nop
            case DelayNop:
                inst[i++] = 0xd503201f;
                break;
            // add x0, x1, x1
            case DelayIAdd:
                inst[i++] = 0x8b010020;
                break;
            // fadd s1, s2, s2
            case DelayFAdd:
                inst[i++] = 0x1e222841;
                break;
            // cmp x0, x1
            case DelayCmp:
                inst[i++] = 0xeb01001f;
                break;
            // add x0, x1, x1
            // cmp x2, x3
            case DelayIAddICmp:
                inst[i++] = 0x8b010020;
                inst[i++] = 0xeb03005f;
                break;
            // add x0, x1, x1
            // fadd s1, s2, s2
            case DelayIFAdd:
                inst[i++] = 0x8b010020;
                inst[i++] = 0x1e222841;
                break;
            // ldr x0, [x2]
            case DelayLoad:
                inst[i++] = 0xf9400040;
                break;
            // str x0, [x2, #8]
            case DelayStore:
                inst[i++] = 0xf9000440;
                break;
            }
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
        // sqrtsd %xmm0, %xmm0
        for (int j = 0; j < delayCnt; j++) {
            instBuf[i++] = 0xf2;
            instBuf[i++] = 0x0f;
            instBuf[i++] = 0x51;
            instBuf[i++] = 0xc0;
        }

        for (int j = 0; j < testCnt; j++) {
            switch (caseId) {
            // nop
            case DelayNop:
                instBuf[i++] = 0x90;
                break;
            // add rax, rcx
            case DelayIAdd:
                instBuf[i++] = 0x48;
                instBuf[i++] = 0x01;
                instBuf[i++] = 0xc8;
                break;
            // addss xmm2, xmm3
            case DelayFAdd:
                instBuf[i++] = 0xf3;
                instBuf[i++] = 0x0f;
                instBuf[i++] = 0x58;
                instBuf[i++] = 0xd3;
                break;
            // cmp rax, rcx
            case DelayCmp:
                instBuf[i++] = 0x48;
                instBuf[i++] = 0x39;
                instBuf[i++] = 0xC8;
                break;
            // add rax, rcx
            // cmp rdx, r8
            case DelayIAddICmp:
                instBuf[i++] = 0x48;
                instBuf[i++] = 0x01;
                instBuf[i++] = 0xc8;
                instBuf[i++] = 0x4c;
                instBuf[i++] = 0x39;
                instBuf[i++] = 0xC2;
                break;
            // add rax, rcx
            // addss xmm2, xmm3
            case DelayIFAdd:
                instBuf[i++] = 0x48;
                instBuf[i++] = 0x01;
                instBuf[i++] = 0xc8;
                instBuf[i++] = 0xf3;
                instBuf[i++] = 0x0f;
                instBuf[i++] = 0x58;
                instBuf[i++] = 0xd3;
                break;
            // mov rax, QWORD PTR [r8]
            case DelayLoad:
                instBuf[i++] = 0x49;
                instBuf[i++] = 0x8b;
                instBuf[i++] = 0x00;
                break;
            // mov QWORD PTR [r8], rax
            case DelayStore:
                instBuf[i++] = 0x49;
                instBuf[i++] = 0x89;
                instBuf[i++] = 0x00;
                break;
            }
        }
    }
    // ret
    instBuf[i++] = 0xc3;
#endif

    size_t r0 = 1;
    size_t r1 = 1;
    size_t *r2 = data0;
    size_t *r3 = data1;
    // warm icache
    ((size_t(*)(size_t, size_t, size_t *, size_t *))instBuf)(r0, r1, r2, r3);

    size_t min = -1ull;
    for (int k = 0; k < 10; k++) {
        size_t start = getclock();
        for (int i = 0; i < codeLoopCnt; i++) {
            ((size_t(*)(size_t, size_t, size_t *, size_t *))instBuf)(r0, r1, r2, r3);
        }
        size_t end = getclock();
        size_t clock = end - start;
        if (min > clock)
            min = clock;
    }

    printf("%lld, ", min / (codeLoopCnt * codeDupCnt));
}

int main(int argc, char *argv[]) {
    int testBase = 100;
    int testEnd = 1000;
    int testStep = 10;
    int delayCnt = 10;
    int codeDupCnt = 64;
    int codeLoopCnt = 1000;
    DelayCase caseId = DelayStore;

    for (int i = 1; i < argc; i += 2) {
        if (strcmp(argv[i], "-case") == 0)
            caseId = static_cast<DelayCase>(atoi(argv[i + 1]));
        else if (strcmp(argv[i], "-start") == 0)
            testBase = atoi(argv[i + 1]);
        else if (strcmp(argv[i], "-end") == 0)
            testEnd = atoi(argv[i + 1]);
        else if (strcmp(argv[i], "-step") == 0)
            testStep = atoi(argv[i + 1]);
        else if (strcmp(argv[i], "-delay") == 0)
            delayCnt = atoi(argv[i + 1]);
        else if (strcmp(argv[i], "-dup") == 0)
            codeDupCnt = atoi(argv[i + 1]);
        else if (strcmp(argv[i], "-loop") == 0)
            codeLoopCnt = atoi(argv[i + 1]);
        else {
            printf("delay_test -case 0\n"
                   "    -start 100\n"
                   "    -end 1000\n"
                   "    -step 10\n"
                   "    -delay 8\n"
                   "    -dup   64\n"
                   "    -loop  1000\n");
            return 0;
        }
    }

    if (caseId < 0 || caseId >= DelayMax) {
        printf("0 nop\n1 iadd\n2 fadd\n3 cmp\n4 add+cmp\n5 iadd+fadd\n6 load\n7 store\n");
    }

    printf("case: %s\ndelayCnt:%d codeDupCnt:%d, codeLoopCnt:%d\n", DelayCaseName[caseId], delayCnt, codeDupCnt,
           codeLoopCnt);
    SetProcessAffinityMask(GetCurrentProcess(), 0x10);
    SetProcessPriorityBoost(GetCurrentProcess(), true);
    SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS);
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);
    unsigned char *code = (unsigned char *)VirtualAlloc(0, 0x1001000, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
    unsigned char *instBuf = (unsigned char *)((size_t)(code + 0xfff) & (~0xfff));
    fillnop(instBuf, 0x1000000);

    size_t *data0 = nullptr;
    size_t *data1 = nullptr;
    if (caseId == DelayLoad || caseId == DelayStore) {
        data0 = new size_t[0x1000000];
        data1 = new size_t[0x1000000];
    }

    for (int testCnt = testBase; testCnt < testEnd; testCnt += testStep) {
        printf("%d, ", testCnt);
        delay_test(caseId, instBuf, testCnt, delayCnt, codeDupCnt, codeLoopCnt, data0, data1);
        printf("\n");
    }
    VirtualFree(code, 0x1001000, MEM_RELEASE);
    if (data0)
        delete[] data0;
    if (data1)
        delete[] data1;
    return 0;
}