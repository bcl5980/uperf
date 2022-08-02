#include <math.h>
#include <stdio.h>
#include <string.h>

#include "osutils.h"

// Oline compiler
// https://godbolt.org/

// Online asm generate
// https://disasm.pro/
// https://armconverter.com/
// https://defuse.ca/online-x86-assembler.htm

enum InstTestCase {
    InstNop,
    InstMov,
    InstIAdd,
    InstIAddChain,
    InstFAdd,
    InstFAddChain,
    InstCmp,
    InstLea3,
    InstLea3Chain,
    TestCaseEnd,
};

const char *TestCaseName[TestCaseEnd] = {"Nop",       "Mov", "IAdd", "IAddChain", "FAdd",
                                         "FAddChain", "Cmp", "Lea3", "Lea3Chain"};

void fillnop(unsigned char *instBuf, unsigned sizeBytes) {
    genCodeStart();
#ifdef __aarch64__
    unsigned nopCnt = sizeBytes / 4;
    unsigned *inst = (unsigned *)instBuf;
    for (int i = 0; i < nopCnt; i++)
        inst[i] = 0xd503201f;
#else
    memset(instBuf, 0x90, sizeBytes);
#endif
    genCodeEnd(instBuf, sizeBytes);
}

void InstTest(InstTestCase caseId, unsigned char *instBuf, int testCnt, int codeDupCnt, int codeLoopCnt) {
    int i = 0;

#if defined(__aarch64__) || defined(_M_ARM64)
    // Microsft AARCH64 calling convention:
    // X0-X17, v0-v7, v16-v31 volatile, we can use them
    // X18-X30, v8-v15 nonvolatile, we can't use them
    // https://docs.microsoft.com/en-us/cpp/build/arm64-windows-abi-conventions?view=msvc-170
    unsigned int *inst = (unsigned int *)instBuf;
    for (int k = 0; k < codeDupCnt; k++) {
        for (int j = 0; j < testCnt; j++) {
            switch (caseId) {
            case InstNop:
                inst[i++] = 0xd503201f; // nop
                break;
            case InstMov:
                inst[i++] = 0xaa0103e0; // mov x0, x1
                break;
            case InstIAdd:
                inst[i++] = 0x8b010020; // add x0, x1, x1
                break;
            case InstIAddChain:
                inst[i++] = 0x8b010000; // add x0, x0, x1
                break;
            case InstFAdd:
                inst[i++] = 0x1e222841; // fadd s1, s2, s2
                break;
            case InstFAddChain:
                inst[i++] = 0x1e222821; // fadd s1, s1, s2
                break;
            case InstCmp:
                inst[i++] = 0xeb01001f; // cmp x0, x1
                break;
            case InstLea3:
            case InstLea3Chain:
            default:
                return;
            }
        }
    }

    // ret 0xd65f03c0
    inst[i++] = 0xd65f03c0;

    genCodeEnd(inst, i * sizeof(int));
#else
    // Microsft X64 calling convention:
    // RAX, RCX, RDX, R8, R9, R10, R11, and XMM0-XMM5 volatile, we can write them without saving
    // RBX, RBP, RDI, RSI, RSP, R12, R13, R14, R15, and XMM6-XMM15 nonvolatile, we can't write them
    // https://docs.microsoft.com/en-us/cpp/build/x64-calling-convention?view=msvc-170

    for (int k = 0; k < codeDupCnt; k++) {
        for (int j = 0; j < testCnt; j++) {
            switch (caseId) {
            case InstNop:
                instBuf[i++] = 0x48; // nop
                instBuf[i++] = 0x89;
                instBuf[i++] = 0xc8;
                break;
            case InstMov:
                instBuf[i++] = 0x48; // mov rax, rcx
                instBuf[i++] = 0x89;
                instBuf[i++] = 0xc8;
                break;
            case InstIAdd:
                instBuf[i++] = 0x48; // lea rax,[rbx+rcx]
                instBuf[i++] = 0x8d;
                instBuf[i++] = 0x04;
                instBuf[i++] = 0x0b;
                break;
            case InstIAddChain:
                instBuf[i++] = 0x48; // add rax, rcx
                instBuf[i++] = 0x01;
                instBuf[i++] = 0xc8;
                break;
            case InstFAdd:
                instBuf[i++] = 0xc5; // VPADDD xmm0, xmm1, xmm1
                instBuf[i++] = 0xf1;
                instBuf[i++] = 0xfe;
                instBuf[i++] = 0xc1;
                break;
            case InstFAddChain:
                instBuf[i++] = 0xc5; // VPADDD xmm0, xmm0, xmm1
                instBuf[i++] = 0xf9;
                instBuf[i++] = 0xfe;
                instBuf[i++] = 0xc1;
                break;
            case InstCmp:
                instBuf[i++] = 0x48; // cmp rax, rcx
                instBuf[i++] = 0x39;
                instBuf[i++] = 0xC8;
                break;
            case InstLea3:
                instBuf[i++] = 0x48; // lea rax, [rcx+8*rax+42]
                instBuf[i++] = 0x8d;
                instBuf[i++] = 0x44;
                instBuf[i++] = 0xd1;
                instBuf[i++] = 0x2a;
                break;
            case InstLea3Chain:
                instBuf[i++] = 0x48; // lea rax, [rcx+8*rax+42]
                instBuf[i++] = 0x8d;
                instBuf[i++] = 0x44;
                instBuf[i++] = 0xc1;
                instBuf[i++] = 0x2a;
                break;
            default:
                return;
            }
        }
    }
    // ret
    instBuf[i++] = 0xc3;
#endif

    size_t r0 = 1;
    size_t r1 = 1;
    size_t *r2 = nullptr;
    size_t *r3 = nullptr;
    // __debugbreak();
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

    printf("%.1f ", (double)min / (codeLoopCnt * codeDupCnt));
}

int main(int argc, char *argv[]) {
    int testBase = 900;
    int testEnd = 1001;
    int testStep = 10;
    int codeDupCnt = 1;
    int codeLoopCnt = 50000;
    InstTestCase caseId = InstNop;

    for (int i = 1; i < argc; i += 2) {
        if (strcmp(argv[i], "-case") == 0)
            caseId = static_cast<InstTestCase>(atoi(argv[i + 1]));
        else if (strcmp(argv[i], "-start") == 0)
            testBase = atoi(argv[i + 1]);
        else if (strcmp(argv[i], "-end") == 0)
            testEnd = atoi(argv[i + 1]);
        else if (strcmp(argv[i], "-step") == 0)
            testStep = atoi(argv[i + 1]);
        else if (strcmp(argv[i], "-dup") == 0)
            codeDupCnt = atoi(argv[i + 1]);
        else if (strcmp(argv[i], "-loop") == 0)
            codeLoopCnt = atoi(argv[i + 1]);
        else {
            printf("caseId, case Name, case Parameter\n");
            for (int i = 0; i < TestCaseEnd; i++) {
                printf("%d, %s\n", i, TestCaseName[i]);
            }

            printf("Example:\n"
                   "uperf_inst -case  0\n"
                   "           -start 100\n"
                   "           -end   1000\n"
                   "           -step  10\n"
                   "           -dup   1\n"
                   "           -loop  1000\n");
            return 0;
        }
    }

    if (caseId < 0 || caseId >= TestCaseEnd) {
        printf("caseId, case Name, case Parameter\n");
        for (int i = 0; i < TestCaseEnd; i++) {
            printf("%d, %s\n", i, TestCaseName[i]);
        }
        return 0;
    }

    if (!procInit(0x01))
        return 1;
    unsigned char *code = allocVM(0x1001000);
    unsigned char *instBuf = (unsigned char *)((size_t)(code + 0xfff) & (~0xfff));
    fillnop(instBuf, 0x1000000);

    for (int testCnt = testBase; testCnt < testEnd; testCnt += testStep) {
        printf("%d ", testCnt);
        InstTest(caseId, instBuf, testCnt, codeDupCnt, codeLoopCnt);
        printf("\n");
    }
    freeVM(code, 0x1001000);
    return 0;
}