#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "osutils.h"
#include "pat_config.h"

// Oline compiler
// https://godbolt.org/

// Online asm generate
// https://disasm.pro/
// https://armconverter.com/
// https://defuse.ca/online-x86-assembler.htm

#include "gen.h"

const unsigned JitMemorySize = 0x1000000;

bool runPattern(PatConfig &config, unsigned char *instBuf, int testCnt, int delayCnt, int codeDupCnt, int codeLoopCnt,
                int gp) {
    genPattern(config, instBuf, testCnt, delayCnt, codeDupCnt, gp);

    size_t r0 = config.args.iArg0;
    size_t r1 = config.args.iArg1;
    void *r2 = config.args.ptrArg0;
    void *r3 = config.args.ptrArg1;
    // __debugbreak();
    // warm icache
    ((size_t(*)(size_t, size_t, void *, void *))instBuf)(r0, r1, r2, r3);

    size_t min = -1;
    for (int k = 0; k < 10; k++) {
        size_t start = getclock();
        for (int i = 0; i < codeLoopCnt; i++) {
            ((size_t(*)(size_t, size_t, void *, void *))instBuf)(r0, r1, r2, r3);
        }
        size_t end = getclock();
        size_t clock = end - start;
        if (min > clock)
            min = clock;
    }

    int codeExecCnt;
    if (config.mode == WorkMode::PeriodTest)
        codeExecCnt = codeLoopCnt;
    else
        codeExecCnt = codeLoopCnt * codeDupCnt;
    printf("%.1f ", (double)min / codeExecCnt);
    return true;
}

int main(int argc, char *argv[]) {
    int testBase = 10;
    int testEnd = 150;
    int testStep = 1;
    int delayCnt = 20;
    int codeDupCnt = 1;
    int codeLoopCnt = 1000;
    int gp = 160;
    TestCase caseId = SqrtNop;
    PatConfig config = {};
    int configFileIdx = -1;

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
        else if (strcmp(argv[i], "-f") == 0) {
            if (!parseConfig(config, argv[i + 1])) {
                printf("Config file parse failed\n");
                return 1;
            }
            configFileIdx = i + 1;
        } else {
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

    if (configFileIdx < 0) {
        if ((caseId < 0 || caseId >= TestCaseEnd)) {
            printf("caseId                          case Name    case Parameter\n");
            for (int i = 0; i < TestCaseEnd; i++) {
                printf("%2d, %36s,    %s\n", i, TestCaseName[i], TestCaseGP[i]);
            }
            return 0;
        }

        if (caseId < SqrtNop)
            delayCnt = 0;

        if (!genConfigForDefaultCases(caseId, config))
            return 1;

        printf("case: %s\ndelayCnt:%d codeDupCnt:%d, codeLoopCnt:%d\n", TestCaseName[caseId], delayCnt, codeDupCnt,
            codeLoopCnt);
    } else {
        printf("configfile: %s\ndelayCnt:%d codeDupCnt:%d, codeLoopCnt:%d\n", argv[configFileIdx], delayCnt, codeDupCnt,
            codeLoopCnt);
    }

    if (!procInit(0x01))
        return 1;

    unsigned char *instBuf = allocVM(JitMemorySize);
    fillnop(instBuf, JitMemorySize);

    for (int testCnt = testBase; testCnt < testEnd; testCnt += testStep) {
        printf("%d ", testCnt);
        if (!runPattern(config, instBuf, testCnt, delayCnt, codeDupCnt, codeLoopCnt, gp)) {
            printf("current test case is not support on the platform\n");
            return 1;
        }
        printf("\n");
    }
    freeVM(instBuf, JitMemorySize);
    if (config.args.ptrArg0)
        free(config.args.ptrArg0);
    if (config.args.ptrArg1)
        free(config.args.ptrArg1);

    return 0;
}