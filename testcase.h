#ifndef __TESTCASE_H__
#define __TESTCASE_H__

enum TestCase {
    InstNop,
    InstMov,
    InstIALU,
    InstIALUChain,
    InstFALU,
    InstFALUChain,
    InstICmp,
    InstLea3,             // x86 only
    InstLea3Chain,        // x86 only
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

    // @todo: add x86 support
    SchSqrtIALUDep,       // Sqrt Result is IALU input
    SchSqrtIALUChainDep,  // Sqrt Result is IALU chain input
    SchSDivFALUDep,       // Sdiv result is FALU input
    SchSDivFALUChainDep,  // Sdiv result is FALU chain input

    SchIALUNop,         // Schedule Queue for Int Test1
    SchIALUChainNop,    // Schedule Queue for Int Test2
    SchICmpNop,         // Schedule Queue for Int Test3
    SchFALUNop,         // Schedule Queue for Float Test1
    SchFALUChainNop,    // Schedule Queue for Float Test2

    PeriodIALUNop,      // Schedule Queue for Int Test4
    PeriodICmpNop,      // Schedule Queue for Int Test5
    PeriodFALUNop,      // Schedule Queue for Float Test3

    TestCaseEnd,
};

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
    "Sqrt Dealy + Load Chain Addr",
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
    "None",
    "None",
    "None",
    "None",
    "Renaming Throughput",
    "Renaming Throughput",
    "Renaming Throughput",
};

#endif // __TESTCASE_H__