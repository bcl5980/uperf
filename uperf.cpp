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

enum DelayTestCase {
    SqrtNop,              // ROB Size
    SqrtMov,              // Check Zero Move feature
    SqrtMovSelf,          // Check Move Self Opt & Physical Reg Size
    SqrtMovSelfFp,        // Float version Check Move Self Opt & Physical Reg Size
    SqrtIAdd,             // Int Physical Reg Size
    UdivVFAdd,            // Float Physical Reg Size
    SqrtCmp,              // Status Physical Reg Size
    SqrtIAddICmp,         // Check if the Status and Int Physical Reg is shared or not
    SqrtIFAdd,            // Check if the Float and Int Physical Reg is shared or not
    SqrtLoad,             // Load Buffer Size Test1
    SqrtLoadSeq,          // Load Buffer Size Test2
    SqrtLoadUnKnownAddr,  // Load Buffer Size Test3
    SqrtStore,            // Store Buffer Size Test1
    SqrtStoreSeq,         // Store Buffer Size Test2
    SqrtStoreUnknownAddr, // Store Buffer Size Test3
    SqrtStoreUnknownVal,  // Store Buffer Size Test4
    SqrtCJmp,             // Condition Jump
    SqrtJmp,              // Jump
    SqrtMixJmp,           // Check if Jump and Condition Jump share or not
    SqrtNopIAdd,          // Test ROB NOP retire speed, rob clear nop speed
    SqrtVFAddIAdd,        // Test ROB register retire speed
    TestCaseEnd,
};

const char *TestCaseName[TestCaseEnd] = {"Sqrt Delay + Nop",
                                         "Sqrt Delay + Mov",
                                         "Sqrt Delay + Mov Self",
                                         "Sqrt Delay + Mov Self(FP)",
                                         "Sqrt Delay + IAdd",
                                         "Udiv Delay + V/FAdd",
                                         "Sqrt Delay + Cmp",
                                         "Sqrt Delay + Add&Cmp",
                                         "Sqrt Delay + IAdd&V/FAdd",
                                         "Sqrt Delay + Load Same Addr",
                                         "Sqrt Delay + Load Linear Addr",
                                         "Sqrt Delay + Load Unknown Addr",
                                         "Sqrt Delay + Store Same Addr",
                                         "Sqrt Delay + Store Linear Addr",
                                         "Sqrt Delay + Store Unknown Addr",
                                         "Sqrt Delay + Store Unknown Value",
                                         "Sqrt Delay + ConditionJump",
                                         "Sqrt Delay + Jump",
                                         "Sqrt Delay + Jump&CJump",
                                         "Sqrt Delay + Nop + IAdd",
                                         "Sqrt Delay + V/FAdd + IAdd"};

const char *TestCaseGP[TestCaseEnd] = {
    "None",
    "None",
    "None",
    "None",
    "None",
    "None",
    "None",
    "None",
    "None",
    "None",
    "None",
    "None",
    "None",
    "None",
    "None",
    "None",
    "None",
    "None",
    "None",
    "Int physical register size",
    "Int physical register size",
};

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

void delay_test(DelayTestCase caseId, unsigned char *instBuf, int testCnt, int delayCnt, int codeDupCnt,
                int codeLoopCnt, size_t *data0, size_t *data1, int gp) {
    int i = 0;

#if defined(__aarch64__) || defined(_M_ARM64)
    genCodeStart();

    // Microsft AARCH64 calling convention:
    // X0-X17, v0-v7, v16-v31 volatile, we can use them
    // X18-X30, v8-v15 nonvolatile, we can't use them
    // https://docs.microsoft.com/en-us/cpp/build/arm64-windows-abi-conventions?view=msvc-170
    unsigned int *inst = (unsigned int *)instBuf;
    for (int k = 0; k < codeDupCnt; k++) {
        if (caseId == UdivVFAdd || caseId == SqrtMovSelfFp) {
            for (int j = 0; j < delayCnt; j++) {
                inst[i++] = 0x9ac10820; // udiv x0, x1, x1
            }
        } else {
            for (int j = 0; j < delayCnt; j++) {
                inst[i++] = 0x1e61c000; // sqrt d0, d0
            }
        }

        switch (caseId) {
        case SqrtCJmp:
        case SqrtMixJmp:
            // cmp x0, x0
            inst[i++] = 0xeb00001f;
            break;
        case SqrtLoadUnKnownAddr:
        case SqrtStoreUnknownAddr:
            // fcvtzu x1, d0
            inst[i++] = 0x9e790001;
            break;
        default:
            break;
        }

        for (int j = 0; j < testCnt; j++) {
            switch (caseId) {
            case SqrtNop:
            case SqrtNopIAdd:
                inst[i++] = 0xd503201f; // nop
                break;
            case SqrtMov:
                inst[i++] = 0xaa0103e0; // mov x0, x1
                break;
            case SqrtMovSelf:
                inst[i++] = 0xaa0103e1; // mov x1, x1
                break;
            case SqrtMovSelfFp:
                inst[i++] = 0x1e604021; // fmov d1, d1
                break;
            case SqrtIAdd:
                inst[i++] = 0x8b010020; // add x0, x1, x1
                break;
            case UdivVFAdd:
            case SqrtVFAddIAdd:
                inst[i++] = 0x1e222841; // fadd s1, s2, s2
                break;
            case SqrtCmp:
                inst[i++] = 0xeb01001f; // cmp x0, x1
                break;
            case SqrtIAddICmp:
                inst[i++] = 0x8b010020; // add x0, x1, x1
                inst[i++] = 0xeb03005f; // cmp x2, x3
                break;
            case SqrtIFAdd:
                inst[i++] = 0x8b010020; // add x0, x1, x1
                inst[i++] = 0x1e222841; // fadd s1, s2, s2
                break;
            case SqrtLoad:
                inst[i++] = 0xf9400041; // ldr x1, [x2]
                break;
            case SqrtLoadSeq:
                inst[i++] = 0xf8408441; // ldr x1, [x2], 8
                break;
            case SqrtLoadUnKnownAddr:
                inst[i++] = 0xf8616842; // ldr x2, [x2, x1]
                break;
            case SqrtStore:
                inst[i++] = 0xf9000040; // str x0, [x2]
                break;
            case SqrtStoreSeq:
                inst[i++] = 0xf8008440; // str x0, [x2], 8
                break;
            case SqrtStoreUnknownAddr:
                inst[i++] = 0xf8216840; // str x0, [x2, x1]
                break;
            case SqrtStoreUnknownVal:
                inst[i++] = 0xf9000041; // str x1, [x2]
                break;
            case SqrtCJmp:
                inst[i++] = 0x54000020; // b.eq .+4
                break;
            case SqrtJmp:
                inst[i++] = 0x14000001; // b .+4
                break;
            case SqrtMixJmp:
                inst[i++] = 0x54000020; // b.eq .+4
                inst[i++] = 0x14000001; // b .+4
                break;
            default:
                return;
            }
        }

        if (caseId == SqrtNopIAdd || caseId == SqrtVFAddIAdd) {
            for (int j = 0; j < gp; j++)
                inst[i++] = 0x8b010020; // add x0, x1, x1
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
        if (caseId == UdivVFAdd) {
            for (int j = 0; j < delayCnt; j++) {
                instBuf[i++] = 0x48; // xor    rdx,rdx
                instBuf[i++] = 0x31;
                instBuf[i++] = 0xd2;
                instBuf[i++] = 0x48; // div    rcx
                instBuf[i++] = 0xf7;
                instBuf[i++] = 0xf1;
            }
        } else {
            for (int j = 0; j < delayCnt; j++) {
                instBuf[i++] = 0xf2; // sqrtsd %xmm0, %xmm0
                instBuf[i++] = 0x0f;
                instBuf[i++] = 0x51;
                instBuf[i++] = 0xc0;
            }
        }

        switch (caseId) {
        case SqrtCJmp:
        case SqrtMixJmp:
            // test rcx, rcx
            instBuf[i++] = 0x48;
            instBuf[i++] = 0x85;
            instBuf[i++] = 0xC9;
            break;
        case SqrtLoadUnKnownAddr:
        case SqrtStoreUnknownAddr:
            // cvttsd2si rcx,xmm0
            instBuf[i++] = 0xf2;
            instBuf[i++] = 0x48;
            instBuf[i++] = 0x0f;
            instBuf[i++] = 0x2c;
            instBuf[i++] = 0xc8;
            break;
        }

        for (int j = 0; j < testCnt; j++) {
            switch (caseId) {
            case SqrtNop:
            case SqrtNopIAdd:
                instBuf[i++] = 0x48; // nop
                instBuf[i++] = 0x89;
                instBuf[i++] = 0xc8;
                break;
            case SqrtMov:
                instBuf[i++] = 0x48; // mov rax, rcx
                instBuf[i++] = 0x89;
                instBuf[i++] = 0xc8;
                break;
            case SqrtMovSelf:
                instBuf[i++] = 0x48; // mov rax, rax
                instBuf[i++] = 0x89;
                instBuf[i++] = 0xc0;
                break;
            case SqrtMovSelfFp:
                instBuf[i++] = 0xf3; // movq xmm1, xmm1
                instBuf[i++] = 0x0f;
                instBuf[i++] = 0x7e;
                instBuf[i++] = 0xc9;
                break;
            case SqrtIAdd:
                instBuf[i++] = 0x48; // lea rax,[rbx+rcx]
                instBuf[i++] = 0x8d;
                instBuf[i++] = 0x04;
                instBuf[i++] = 0x0b;
                break;
            case UdivVFAdd:
                instBuf[i++] = 0xc5; // VPADDD xmm0, xmm1, xmm1
                instBuf[i++] = 0xf1;
                instBuf[i++] = 0xfe;
                instBuf[i++] = 0xc1;
                break;
            case SqrtCmp:
                instBuf[i++] = 0x48; // cmp rax, rcx
                instBuf[i++] = 0x39;
                instBuf[i++] = 0xC8;
                break;
            case SqrtIAddICmp:
                instBuf[i++] = 0x48; // lea rax,[rbx+rcx]
                instBuf[i++] = 0x8d;
                instBuf[i++] = 0x04;
                instBuf[i++] = 0x0b;
                instBuf[i++] = 0x4c; // cmp rdx, r8
                instBuf[i++] = 0x39;
                instBuf[i++] = 0xC2;
                break;
            case SqrtIFAdd:
                instBuf[i++] = 0x48; // lea rax,[rbx+rcx]
                instBuf[i++] = 0x8d;
                instBuf[i++] = 0x04;
                instBuf[i++] = 0x0b;
                instBuf[i++] = 0xc5; // VPADDD xmm0, xmm1, xmm1
                instBuf[i++] = 0xf1;
                instBuf[i++] = 0xfe;
                instBuf[i++] = 0xc1;
                break;
            case SqrtLoad:
                instBuf[i++] = 0x49; // mov rax, QWORD PTR [r8]
                instBuf[i++] = 0x8b;
                instBuf[i++] = 0x00;
                break;
            case SqrtLoadSeq:
                instBuf[i++] = 0x49; // mov rax, QWORD PTR [r8]
                instBuf[i++] = 0x8b;
                instBuf[i++] = 0x00;
                instBuf[i++] = 0x49; // add r8, 8
                instBuf[i++] = 0x83;
                instBuf[i++] = 0xc0;
                instBuf[i++] = 0x08;
                break;
            case SqrtLoadUnKnownAddr:
                instBuf[i++] = 0x49; // mov rax, QWORD PTR [r8+rcx]
                instBuf[i++] = 0x8b;
                instBuf[i++] = 0x04;
                instBuf[i++] = 0x08;
                break;
            case SqrtStore:
                instBuf[i++] = 0x49; // mov QWORD PTR [r8], rax
                instBuf[i++] = 0x89;
                instBuf[i++] = 0x00;
                break;
            case SqrtStoreSeq:
                instBuf[i++] = 0x49; // mov QWORD PTR [r8], rax
                instBuf[i++] = 0x89;
                instBuf[i++] = 0x00;
                instBuf[i++] = 0x49; // add r8, 8
                instBuf[i++] = 0x83;
                instBuf[i++] = 0xc0;
                instBuf[i++] = 0x08;
                break;
            case SqrtStoreUnknownAddr:
                instBuf[i++] = 0x49; // mov QWORD PTR [r8+rcx], rax
                instBuf[i++] = 0x89;
                instBuf[i++] = 0x04;
                instBuf[i++] = 0x08;
                break;
            case SqrtStoreUnknownVal:
                instBuf[i++] = 0x49; // mov QWORD PTR [r8], rcx
                instBuf[i++] = 0x89;
                instBuf[i++] = 0x08;
                break;
            case SqrtCJmp:
                instBuf[i++] = 0x75; // jnz 0
                instBuf[i++] = 0x00;
                break;
            case SqrtJmp:
                instBuf[i++] = 0xeb; // jmp 0
                instBuf[i++] = 0x00;
                break;
            case SqrtMixJmp:
                instBuf[i++] = 0x75; // jnz 0
                instBuf[i++] = 0x00;
                instBuf[i++] = 0xeb; // jmp 0
                instBuf[i++] = 0x00;
                break;
            case SqrtVFAddIAdd:
                instBuf[i++] = 0xc5; // VPADDD xmm2, xmm1, xmm1
                instBuf[i++] = 0xf1;
                instBuf[i++] = 0xfe;
                instBuf[i++] = 0xd1;
                break;
            default:
                return;
            }
        }

        if (caseId == SqrtNopIAdd || caseId == SqrtVFAddIAdd) {
            for (int j = 0; j < gp; j++) {
                instBuf[i++] = 0x48; // lea rax,[rbx+rcx]
                instBuf[i++] = 0x8d;
                instBuf[i++] = 0x04;
                instBuf[i++] = 0x0b;
            }
        }
    }
    // ret
    instBuf[i++] = 0xc3;
#endif

    size_t r0 = 0;
    size_t r1 = 0;
    size_t *r2 = data0;
    size_t *r3 = data1;
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
    int testBase = 100;
    int testEnd = 1000;
    int testStep = 10;
    int delayCnt = 20;
    int codeDupCnt = 1;
    int codeLoopCnt = 1000;
    int gp = 160;
    DelayTestCase caseId = SqrtLoad;

    for (int i = 1; i < argc; i += 2) {
        if (strcmp(argv[i], "-case") == 0)
            caseId = static_cast<DelayTestCase>(atoi(argv[i + 1]));
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
        else if (strcmp(argv[i], "-gp") == 0)
            gp = atoi(argv[i + 1]);
        else {
            printf("caseId, case Name, case Parameter\n");
            for (int i = 0; i < TestCaseEnd; i++) {
                printf("%d, %s, %s\n", i, TestCaseName[i], TestCaseGP[i]);
            }

            printf("Example:\n"
                   "uperf -case  0\n"
                   "      -start 100\n"
                   "      -end   1000\n"
                   "      -step  10\n"
                   "      -delay 8\n"
                   "      -dup   1\n"
                   "      -loop  1000\n"
                   "      -gp    160");
            return 0;
        }
    }

    if (caseId < 0 || caseId >= TestCaseEnd) {
        printf("caseId, case Name, case Parameter\n");
        for (int i = 0; i < TestCaseEnd; i++) {
            printf("%d, %s, %s\n", i, TestCaseName[i], TestCaseGP[i]);
        }
        return 0;
    }

    if (!procInit(0x01))
        return 1;

    printf("case: %s\ndelayCnt:%d codeDupCnt:%d, codeLoopCnt:%d\n", TestCaseName[caseId], delayCnt, codeDupCnt,
           codeLoopCnt);
    unsigned char *code = allocVM(0x1001000);
    unsigned char *instBuf = (unsigned char *)((size_t)(code + 0xfff) & (~0xfff));
    fillnop(instBuf, 0x1000000);

    size_t *data0 = nullptr;
    size_t *data1 = nullptr;
    if (caseId == SqrtLoad || caseId == SqrtStore || caseId == SqrtStoreUnknownAddr) {
        data0 = new size_t[0x1000000];
        data1 = new size_t[0x1000000];
        data0[0] = (size_t)data0;
        data1[0] = (size_t)data1;
    }

    for (int testCnt = testBase; testCnt < testEnd; testCnt += testStep) {
        printf("%d ", testCnt);
        delay_test(caseId, instBuf, testCnt, delayCnt, codeDupCnt, codeLoopCnt, data0, data1, gp);
        printf("\n");
    }
    freeVM(code, 0x1001000);
    if (data0)
        delete[] data0;
    if (data1)
        delete[] data1;
    return 0;
}