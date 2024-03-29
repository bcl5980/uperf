#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "arch.h"
#include "osutils.h"
#include "pat_config.h"

// Oline compiler
// https://godbolt.org/

// Online asm generate
// https://disasm.pro/
// https://armconverter.com/
// https://defuse.ca/online-x86-assembler.htm

const unsigned JitMemorySize = 0x1000000;

const char *TestCaseName[TestCaseEnd] = {
    "Inst Nop",
    "Inst Mov",
    "Inst IALU",
    "Inst IALUChain",
    "Inst FALU",
    "Inst FALUChain",
    "Inst Cmp",
    "Inst Lea3",
    "Inst Lea3Chain",
    "Inst Load",
    "Inst Store",
    "Sqrt Delay + Nop",
    "Sqrt Delay + Mov",
    "Sqrt Delay + Mov Self",
    "Sqrt Delay + Mov Self(FP)",
    "Sqrt Delay + IALU",
    "Udiv Delay + V/FALU",
    "Sqrt Delay + Cmp",
    "Sqrt Delay + Add&Cmp",
    "Sqrt Delay + IALU&V/FALU",
    "Sqrt Delay + Load Same Addr",
    "Sqrt Delay + Load Linear Addr",
    "Sqrt Delay + Load Unknown Addr",
    "Sqrt Delay + Load Chain Addr",
    "Sqrt Delay + Store Same Addr",
    "Sqrt Delay + Store Linear Addr",
    "Sqrt Delay + Store Unknown Addr",
    "Sqrt Delay + Store Unknown Value",
    "Sqrt Delay + ConditionJump",
    "Sqrt Delay + Jump",
    "Sqrt Delay + Jump&CJump",
    "Sqrt Delay + Nop + IALU",
    "Sqrt Delay + V/FALU + IALU",
    "Sqrt Delay + IALU depency on Delay",
    "Sqrt Delay + IALU chain depency on Delay",
    "SDiv Delay + FALU depency on Delay",
    "SDiv Delay + FALU chain depency on Delay",
    "IALU + Nop",
    "IALUChain + Nop",
    "ICmp + Nop",
    "FALU + Nop",
    "FALUChain + Nop",
    "Period IALU + Nop",
    "Period ICmp + Nop",
    "Period FALU + Nop",
};

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
    for (unsigned k = 0; k < 10; k++) {
        size_t start = getclock();
        for (unsigned i = 0; i < param.loopCnt; i++) {
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

bool parseArgs(int argc, char *argv[], TestParam &param, TestCase &caseId, PatConfig &config,
               unsigned &affinity) {
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
        else if (strcmp(argv[i], "-inst_num") == 0)
            param.instNum = atoi(argv[i + 1]);
        else if (strcmp(argv[i], "-thrput_inst") == 0)
            param.testInstTP = atoi(argv[i + 1]);
        else if (strcmp(argv[i], "-thrput_fill") == 0)
            param.fillInstTP = atoi(argv[i + 1]);
        else if (strcmp(argv[i], "-affinity") == 0)
            affinity = atoi(argv[i + 1]);
        else if (strcmp(argv[i], "-f") == 0) {
            if (!parseConfig(config, argv[i + 1])) {
                printf("Config file parse failed\n");
                return false;
            }
            configFileIdx = i + 1;
        } else {
            printf("caseId                          case Name\n");
            for (int i = 0; i < TestCaseEnd; i++) {
                printf("%2d, %36s\n", i, TestCaseName[i]);
            }

            printf("DelayMode:\n"
                   "uperf -case       14   \n"
                   "      -start      100  \n"
                   "      -end        1000 \n"
                   "      -step       10   \n"
                   "      -delay      8    \n"
                   "      -prologue   1    \n"
                   "      -epilogue   160  \n"
                   "PeriodMode:\n"
                   "uperf -case        42    \n"
                   "      -start       1     \n"
                   "      -end         16    \n"
                   "      -step        1     \n"
                   "      -inst_num    10000 \n"
                   "      -thrput_inst 6     \n"
                   "      -thrput_fill 8     \n\n"
                   "      -loop       1000   \n"
                   "      -affinity   1      \n");
            return false;
        }
    }

    if (configFileIdx < 0) {
        if ((caseId < 0 || caseId >= TestCaseEnd)) {
            printf("caseId                          case Name\n");
            for (int i = 0; i < TestCaseEnd; i++) {
                printf("%2d, %36s\n", i, TestCaseName[i]);
            }
            return false;
        }

        if (caseId < SqrtNop)
            param.delayCnt = 0;

        if (!genConfigForDefaultCases(caseId, config))
            return false;

        printf("affinity:0x%x, loop: %d, case: %s\n", affinity, param.loopCnt,
               TestCaseName[caseId]);
    } else {
        printf("affinity:0x%x, loop: %d, configfile: %s\n", affinity, param.loopCnt,
               argv[configFileIdx]);
    }

    if (config.mode == WorkMode::DelayTest) {
        if (config.di.delayPat.size())
            printf("delayCnt:%d ", param.delayCnt);
        if (config.di.prologuePat.size())
            printf("prologueCnt:%d ", param.prologueCnt);
        if (config.di.epiloguePat.size())
            printf("epilogueCnt:%d ", param.epilogueCnt);
    } else if (config.mode == WorkMode::PeriodTest) {
        printf("instNum:%d test inst throughput:%d, fill inst throughput:%d", param.instNum,
               param.testInstTP, param.fillInstTP);
    }
    printf("\n");
    return true;
}

int main(int argc, char *argv[]) {
    TestParam param;
    TestCase caseId = SqrtNop;
    PatConfig config = {};
    unsigned affinity = 1;

    if (!parseArgs(argc, argv, param, caseId, config, affinity))
        return 0;

    if (!procInit(affinity))
        return 1;

    unsigned char *instBuf = allocVM(JitMemorySize);
    fillNop(instBuf, JitMemorySize);

    printf("arg0:%llu, arg1:%llu, ptr0:%p, ptr1:%p\n", config.args.iArg0, config.args.iArg1,
           config.args.ptrArg0, config.args.ptrArg1);
    printf("test start:\n");
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