#ifndef __TESTCASE_H__
#define __TESTCASE_H__

enum TestCase {
    InstNop,
    InstMov,
    InstIAdd,
    InstIAddChain,
    InstFAdd,
    InstFAddChain,
    InstCmp,
    InstLea3,
    InstLea3Chain,

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

    // @todo: add x86 support
    SchIAddNop,         // Schedule Queue for Int Test1
    SchIAddChainNop,    // Schedule Queue for Int Test2
    SchICmpNop,         // Schedule Queue for Int Test3
    SchFAddNop,         // Schedule Queue for Float Test1
    SchFAddChainNop,    // Schedule Queue for Float Test2

    PeriodIAddNop,      // Schedule Queue for Int Test4
    PeriodICmpNop,      // Schedule Queue for Int Test5
    PeriodFAddNop,      // Schedule Queue for Float Test3
    TestCaseEnd,
};

const char *TestCaseName[TestCaseEnd] = {
    "Inst Nop",
    "Inst Mov",
    "Inst IAdd",
    "Inst IAddChain",
    "Inst FAdd",
    "Inst FAddChain",
    "Inst Cmp",
    "Inst Lea3",
    "Inst Lea3Chain",
    "Sqrt Delay + Nop",
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
    "Sqrt Delay + V/FAdd + IAdd",
    "IAdd + Nop",
    "IAddChain + Nop",
    "ICmp + Nop",
    "FAdd + Nop",
    "FAddChain + Nop",
    "Period IAdd + Nop",
    "Period ICmp + Nop",
    "Period FAdd + Nop",
};

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
    "None",
    "None",
    "None",
    "None",
    "None",
    "Renaming Throughput",
    "Renaming Throughput",
    "Renaming Throughput",
};

#endif // __TESTCASE_H__