#ifndef __UPERF_H__
#define __UPERF_H__

#include <string>
#include <vector>

enum TestCase {
    InstNop,
    InstMov,
    InstIALU,
    InstIALUChain,
    InstFALU,
    InstFALUChain,
    InstICmp,
    InstLea3,      // x86 only
    InstLea3Chain, // x86 only
    InstLoad,
    InstStore,
    SqrtNop,              // ROB Size
    SqrtMov,              // Check Zero Move feature
    SqrtMovSelf,          // Check Move Self Opt & Physical Reg Size
    SqrtMovSelfFp,        // Float version Check Move Self Opt & Physical Reg Size
    SqrtIALU,             // Int Physical Reg Size
    UdivVFALU,            // Float Physical Reg Size
    SqrtICmp,             // Status Physical Reg Size
    SqrtIALUICmp,         // Check if the Status and Int Physical Reg is shared or not
    SqrtIFALU,            // Check if the Float and Int Physical Reg is shared or not
    SqrtLoad,             // Load Buffer Size Test1
    SqrtLoadSeq,          // Load Buffer Size Test2
    SqrtLoadUnKnownAddr,  // Load Buffer Size Test3
    SqrtLoadChain,        // Load Buffer Size Test4
    SqrtStore,            // Store Buffer Size Test1
    SqrtStoreSeq,         // Store Buffer Size Test2
    SqrtStoreUnknownAddr, // Store Buffer Size Test3
    SqrtStoreUnknownVal,  // Store Buffer Size Test4
    SqrtCJmp,             // Condition Jump
    SqrtJmp,              // Jump
    SqrtMixJmp,           // Check if Jump and Condition Jump share or not
    SqrtNopIALU,          // Test ROB NOP retire speed, rob clear nop speed
    SqrtVFALUIALU,        // Test ROB register retire speed
    SchSqrtIALUDep,       // Sqrt Result is IALU input
    SchSqrtIALUChainDep,  // Sqrt Result is IALU chain input
    SchSDivFALUDep,       // Sdiv result is FALU input
    SchSDivFALUChainDep,  // Sdiv result is FALU chain input
    SchIALUNop,           // Schedule Queue for Int Test1
    SchIALUChainNop,      // Schedule Queue for Int Test2
    SchICmpNop,           // Schedule Queue for Int Test3
    SchFALUNop,           // Schedule Queue for Float Test1
    SchFALUChainNop,      // Schedule Queue for Float Test2

    PeriodIALUNop, // Schedule Queue for Int Test4
    PeriodICmpNop, // Schedule Queue for Int Test5
    PeriodFALUNop, // Schedule Queue for Float Test3

    TestCaseEnd,
};

typedef std::vector<unsigned char> InstBytes;

enum class ArchType { X86_64, AArch64 };

enum class WorkMode { DelayTest, PeriodTest };

struct Arguments {
    unsigned long long iArg0;
    unsigned long long iArg1;
    void *ptrArg0;
    void *ptrArg1;
};

struct DelayInfos {
    std::vector<InstBytes> delayPat;
    std::vector<InstBytes> prologuePat;
    std::vector<InstBytes> contentPat;
    std::vector<InstBytes> epiloguePat;
};

struct PeriodInfos {
    std::vector<InstBytes> periodPat;
    std::vector<InstBytes> fillPat;
};

struct PatConfig {
    ArchType arch;
    WorkMode mode;
    Arguments args;
    DelayInfos di;
    PeriodInfos pi;
};

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
            unsigned instNum;
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

#endif // __UPERF_H__