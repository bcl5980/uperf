#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "osutils.h"

// Oline compiler
// https://godbolt.org/

// Online asm generate
// https://disasm.pro/
// https://armconverter.com/
// https://defuse.ca/online-x86-assembler.htm

#include "gen.h"

bool runPattern(TestCase caseId, unsigned char *instBuf, int testCnt, int delayCnt, int codeDupCnt, int codeLoopCnt,
                size_t *data0, size_t *data1, int gp) {
    if (!genPattern(caseId, instBuf, testCnt, delayCnt, codeDupCnt, gp))
        return false;

    size_t r0 = 0;
    size_t r1 = 8;
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

    int codeExecCnt;
    if (caseId >= PeriodIALUNop)
        codeExecCnt = codeLoopCnt;
    else
        codeExecCnt = codeLoopCnt * codeDupCnt;
    printf("%.1f ", (double)min / codeExecCnt);
    return true;
}

int main(int argc, char *argv[]) {
    int testBase = 100;
    int testEnd = 1000;
    int testStep = 10;
    int delayCnt = 20;
    int codeDupCnt = 1;
    int codeLoopCnt = 1000;
    int gp = 160;
    TestCase caseId = InstNop;

    for (int i = 1; i < argc; i += 2) {
        if (strcmp(argv[i], "-case") == 0)
            caseId = static_cast<TestCase>(atoi(argv[i + 1]));
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
            printf("caseId                          case Name    case Parameter\n");
            for (int i = 0; i < TestCaseEnd; i++) {
                printf("%2d, %36s,    %s\n", i, TestCaseName[i], TestCaseGP[i]);
            }

            printf("Example:\n"
                   "uperf -case  0    \n"
                   "      -start 100  \n"
                   "      -end   1000 \n"
                   "      -step  10   \n"
                   "      -delay 8    :ignore when only measure inst throughput\n"
                   "      -dup   1    :unroll counter for the measure pattern\n"
                   "      -loop  1000 :loop counter for the measure pattern\n"
                   "      -gp    160  :general paramater for some cases\n");
            return 0;
        }
    }

    if (caseId < 0 || caseId >= TestCaseEnd) {
        printf("caseId                          case Name    case Parameter\n");
        for (int i = 0; i < TestCaseEnd; i++) {
            printf("%2d, %36s,    %s\n", i, TestCaseName[i], TestCaseGP[i]);
        }
        return 0;
    }

    if (!procInit(0x01))
        return 1;

    if (caseId < SqrtNop)
        delayCnt = 0;
    printf("case: %s\ndelayCnt:%d codeDupCnt:%d, codeLoopCnt:%d\n", TestCaseName[caseId], delayCnt, codeDupCnt,
           codeLoopCnt);
    unsigned char *code = allocVM(0x1001000);
    unsigned char *instBuf = (unsigned char *)((size_t)(code + 0xfff) & (~0xfff));
    fillnop(instBuf, 0x1000000);

    size_t *data0 = nullptr;
    size_t *data1 = nullptr;
    if (caseId >= SqrtLoad && caseId <= SqrtStoreUnknownVal) {
        data0 = new size_t[0x1000000];
        data1 = new size_t[0x1000000];
        data0[0] = (size_t)data0;
        data1[0] = (size_t)data1;
    }

    for (int testCnt = testBase; testCnt < testEnd; testCnt += testStep) {
        printf("%d ", testCnt);
        if (!runPattern(caseId, instBuf, testCnt, delayCnt, codeDupCnt, codeLoopCnt, data0, data1, gp)) {
            printf("current test case is not support on the platform\n");
            return 1;
        }
        printf("\n");
    }
    freeVM(code, 0x1001000);
    if (data0)
        delete[] data0;
    if (data1)
        delete[] data1;
    return 0;
}