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

struct TestParam {
    WorkMode mode;
    unsigned begin;
    unsigned end;
    unsigned step;

    unsigned loopCnt;

    union {
        struct {
            unsigned delayCnt;
            unsigned prologueCnt;
            unsigned epilogueCnt;
        };
        struct {
            unsigned period;
            unsigned testInstTP;
            unsigned fillInstTP;
        };
    };

    TestParam() {
        mode = WorkMode::DelayTest;

        begin = 10;
        end = 150;
        step = 1;

        delayCnt = 20;
        prologueCnt = 1;
        epilogueCnt = 1;

        loopCnt = 1000;
    }
};

#include "gen.h"

const unsigned JitMemorySize = 0x1000000;

bool runPattern(PatConfig &config, unsigned char *instBuf, TestParam &param, unsigned testCnt) {
    genPattern(config, instBuf, param, testCnt);

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
        for (int i = 0; i < param.loopCnt; i++) {
            ((size_t(*)(size_t, size_t, void *, void *))instBuf)(r0, r1, r2, r3);
        }
        size_t end = getclock();
        size_t clock = end - start;
        if (min > clock)
            min = clock;
    }

    printf("%.1f ", (double)min / param.loopCnt);
    return true;
}

int main(int argc, char *argv[]) {
    TestParam param;
    TestCase caseId = SqrtNop;
    PatConfig config = {};
    int configFileIdx = -1;

    for (int i = 1; i < argc; i += 2) {
        if (strcmp(argv[i], "-case") == 0)
            caseId = static_cast<TestCase>(atoi(argv[i + 1]));
        else if (strcmp(argv[i], "-start") == 0)
            param.begin = atoi(argv[i + 1]);
        else if (strcmp(argv[i], "-end") == 0)
            param.end = atoi(argv[i + 1]);
        else if (strcmp(argv[i], "-step") == 0)
            param.step = atoi(argv[i + 1]);
        else if (strcmp(argv[i], "-delay") == 0)
            param.delayCnt = atoi(argv[i + 1]);
        else if (strcmp(argv[i], "-prologue") == 0)
            param.prologueCnt = atoi(argv[i + 1]);
        else if (strcmp(argv[i], "-epilogue") == 0)
            param.epilogueCnt = atoi(argv[i + 1]);
        else if (strcmp(argv[i], "-loop") == 0)
            param.loopCnt = atoi(argv[i + 1]);
        else if (strcmp(argv[i], "-period") == 0)
            param.period = atoi(argv[i + 1]);
        else if (strcmp(argv[i], "-thrput_inst") == 0)
            param.testInstTP = atoi(argv[i + 1]);
        else if (strcmp(argv[i], "-thrput_fill") == 0)
            param.fillInstTP = atoi(argv[i + 1]);
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
            param.delayCnt = 0;

        if (!genConfigForDefaultCases(caseId, config))
            return 1;

        printf("case: %s\ndelayCnt:%d, codeLoopCnt:%d\n", TestCaseName[caseId], param.delayCnt,
               param.loopCnt);
    } else {
        printf("configfile: %s\ndelayCnt:%d, codeLoopCnt:%d\n", argv[configFileIdx], param.delayCnt,
               param.loopCnt);
    }

    if (!procInit(0x01))
        return 1;

    unsigned char *instBuf = allocVM(JitMemorySize);
    fillnop(instBuf, JitMemorySize);

    for (unsigned testCnt = param.begin; testCnt < param.end; testCnt += param.step) {
        printf("%d ", testCnt);
        if (!runPattern(config, instBuf, param, testCnt)) {
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