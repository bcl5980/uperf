#if defined(__aarch64__) || defined(_M_ARM64)
#include "arch.h"
#include "osutils.h"
#include "uperf.h"
#include <vector>

inline unsigned pair(unsigned char a0, unsigned char a1, unsigned char a2, unsigned char a3) {
    return ((unsigned)a3 << 24) | ((unsigned)a2 << 16) | ((unsigned)a1 << 8) | a0;
}

static void genDelayPattern(PatConfig &config, unsigned *inst, unsigned delayCnt, unsigned &i) {
    const std::vector<InstBytes> &insts = config.di.delayPat;
    if (insts.empty())
        return;

    for (unsigned j = 0; j < delayCnt; j++) {
        for (auto ii : insts) {
            inst[i++] = pair(ii[0], ii[1], ii[2], ii[3]);
        }
    }
}

static void genPrologue(PatConfig &config, unsigned *inst, unsigned instCnt, unsigned &i) {
    const std::vector<InstBytes> &insts = config.di.prologuePat;
    if (insts.empty())
        return;

    for (unsigned j = 0; j < instCnt; j++) {

        for (auto ii : insts) {
            inst[i++] = pair(ii[0], ii[1], ii[2], ii[3]);
        }
    }
}

static void genContent(PatConfig &config, unsigned *inst, unsigned testCnt, unsigned &i) {
    const std::vector<InstBytes> &insts = config.di.contentPat;
    if (insts.empty())
        return;

    for (unsigned j = 0; j < testCnt; j++) {
        for (auto ii : insts) {
            inst[i++] = pair(ii[0], ii[1], ii[2], ii[3]);
        }
    }
}

static void genEpilogue(PatConfig &config, unsigned *inst, unsigned gp, unsigned &i) {
    const std::vector<InstBytes> &insts = config.di.epiloguePat;
    if (insts.empty())
        return;

    for (unsigned j = 0; j < gp; j++) {
        for (auto ii : insts) {
            inst[i++] = pair(ii[0], ii[1], ii[2], ii[3]);
        }
    }
}

static void genPeriodPattern(PatConfig &config, unsigned *inst, unsigned period, unsigned instNum,
                             unsigned instThroughPut, unsigned nopThroughPut, unsigned &i) {
    unsigned InstCnt = instThroughPut * period;
    unsigned PeriodCnt = nopThroughPut * period;
    const std::vector<InstBytes> &periodPat = config.pi.periodPat;
    const std::vector<InstBytes> &fillPat = config.pi.fillPat;
    for (unsigned j = 0, k = 0; j < instNum; j++) {
        if (k < InstCnt) {
            for (auto ii : periodPat) {
                inst[i++] = pair(ii[0], ii[1], ii[2], ii[3]);
            }
        } else {
            for (auto ii : fillPat) {
                inst[i++] = pair(ii[0], ii[1], ii[2], ii[3]);
            }
        }
        k++;
        if (k >= PeriodCnt)
            k = 0;
    }
}

bool genPattern(PatConfig &config, unsigned char *instBuf, TestParam &param, unsigned testCnt) {
    unsigned i = 0;
    unsigned *inst = (unsigned *)instBuf;
    genCodeStart();

    // Microsft AARCH64 calling convention:
    // X0-X17, v0-v7, v16-v31 volatile, we can use them
    // X18-X30, v8-v15 nonvolatile, we can't use them
    // https://docs.microsoft.com/en-us/cpp/build/arm64-windows-abi-conventions?view=msvc-170

    // MACOSX ABI
    // AAPCS64 (Procedure Call Standard for the ARM 64-bit Architecture) says
    // registers x19 through x28 and sp are callee preserved. v8-v15 are non-
    // volatile (and specifically only the lower 8 bytes of these regs), the rest
    // of the fp/SIMD registers are volatile.
    //
    // v. https://github.com/ARM-software/abi-aa/blob/main/aapcs64/
    // Volatile registers: x0-x18, x30 (lr)
    if (config.mode == WorkMode::PeriodTest) {
        genPeriodPattern(config, inst, testCnt, param.instNum, param.testInstTP, param.fillInstTP,
                         i);
    } else {
        genDelayPattern(config, inst, param.delayCnt, i);
        genPrologue(config, inst, param.prologueCnt, i);
        genContent(config, inst, testCnt, i);
        genEpilogue(config, inst, param.epilogueCnt, i);
    }

    // ret 0xd65f03c0
    inst[i++] = 0xd65f03c0;

    genCodeEnd(inst, i * sizeof(unsigned));
    return true;
}

bool genConfigForDefaultCases(TestCase caseId, PatConfig &config) {
    config.arch = ArchType::AArch64;
    config.mode = caseId >= PeriodIALUNop ? WorkMode::PeriodTest : WorkMode::DelayTest;
    config.args.iArg0 = 0;
    config.args.iArg1 = 8;
    if ((caseId >= SqrtLoad && caseId <= SqrtStoreUnknownVal) ||
        (caseId >= InstLoad && caseId <= InstStore)) {
        config.args.ptrArg0 = new size_t[0x1000000];
        config.args.ptrArg0 = new size_t[0x1000000];
    }
    switch (caseId) {
    case InstNop:
        config.di.delayPat.clear();
        config.di.prologuePat.clear();
        config.di.contentPat.push_back({0x1f, 0x20, 0x03, 0xd5}); // nop
        config.di.epiloguePat.clear();
        break;
    case InstMov:
        config.di.delayPat.clear();
        config.di.prologuePat.clear();
        config.di.contentPat.push_back({0xe0, 0x03, 0x01, 0xaa}); // mov x0, x1
        config.di.epiloguePat.clear();
        break;
    case InstIALU:
        config.di.delayPat.clear();
        config.di.prologuePat.clear();
        config.di.contentPat.push_back({0x20, 0x00, 0x01, 0x8b}); // add x0, x1, x1
        config.di.epiloguePat.clear();
        break;
    case InstIALUChain:
        config.di.delayPat.clear();
        config.di.prologuePat.clear();
        config.di.contentPat.push_back({0x00, 0x00, 0x01, 0x8b}); // add x0, x0, x1
        config.di.epiloguePat.clear();
        break;
    case InstFALU:
        config.di.delayPat.clear();
        config.di.prologuePat.clear();
        config.di.contentPat.push_back({0x01, 0xc0, 0x60, 0x1e}); // fabs d1, d0
        config.di.epiloguePat.clear();
        break;
    case InstFALUChain:
        config.di.delayPat.clear();
        config.di.prologuePat.clear();
        config.di.contentPat.push_back({0x00, 0xc0, 0x60, 0x1e}); // fabs d0, d0
        config.di.epiloguePat.clear();
        break;
    case InstICmp:
        config.di.delayPat.clear();
        config.di.prologuePat.clear();
        config.di.contentPat.push_back({0x1f, 0x00, 0x01, 0xeb}); // cmp x0, x1
        config.di.epiloguePat.clear();
        break;
    case InstLea3:
        return false;
    case InstLea3Chain:
        return false;
    case InstLoad:
        config.di.delayPat.clear();
        config.di.prologuePat.clear();
        config.di.contentPat.push_back({0x41, 0x00, 0x40, 0xf9}); // ldr x1, [x2]
        config.di.epiloguePat.clear();
        break;
    case InstStore:
        config.di.delayPat.clear();
        config.di.prologuePat.clear();
        config.di.contentPat.push_back({0x40, 0x00, 0x00, 0xf9}); // str x0, [x2]
        config.di.epiloguePat.clear();
        break;
    case SqrtNop:
        config.di.delayPat.push_back({0x00, 0xc0, 0x61, 0x1e}); // sqrt d0, d0
        config.di.prologuePat.clear();
        config.di.contentPat.push_back({0x1f, 0x20, 0x03, 0xd5}); // nop
        config.di.epiloguePat.clear();
        break;
    case SqrtMov:
        config.di.delayPat.push_back({0x00, 0xc0, 0x61, 0x1e}); // sqrt d0, d0
        config.di.prologuePat.clear();
        config.di.contentPat.push_back({0xe0, 0x03, 0x01, 0xaa}); // mov x0, x1
        config.di.epiloguePat.clear();
        break;
    case SqrtMovSelf:
        config.di.delayPat.push_back({0x00, 0xc0, 0x61, 0x1e}); // sqrt d0, d0
        config.di.prologuePat.clear();
        config.di.contentPat.push_back({0xe1, 0x03, 0x01, 0xaa}); // mov x1, x1
        config.di.epiloguePat.clear();
        break;
    case SqrtMovSelfFp:
        config.di.delayPat.push_back({0x41, 0x08, 0xc1, 0x9a}); // udiv x1, x2, x1
        config.di.prologuePat.clear();
        config.di.contentPat.push_back({0x21, 0x40, 0x60, 0x1e}); // fmov d1, d1
        config.di.epiloguePat.clear();
        break;
    case SqrtIALU:
        config.di.delayPat.push_back({0x00, 0xc0, 0x61, 0x1e}); // sqrt d0, d0
        config.di.prologuePat.clear();
        config.di.contentPat.push_back({0x20, 0x00, 0x01, 0x8b}); // add x0, x1, x1
        config.di.epiloguePat.clear();
        break;
    case UdivVFALU:
        config.di.delayPat.push_back({0x41, 0x08, 0xc1, 0x9a}); // udiv x1, x2, x1
        config.di.prologuePat.clear();
        config.di.contentPat.push_back({0x01, 0xc0, 0x60, 0x1e}); // fabs d1, d0
        config.di.epiloguePat.clear();
        break;
    case SqrtICmp:
        config.di.delayPat.push_back({0x00, 0xc0, 0x61, 0x1e}); // sqrt d0, d0
        config.di.prologuePat.clear();
        config.di.contentPat.push_back({0x1f, 0x00, 0x01, 0xeb}); // cmp x0, x1
        config.di.epiloguePat.clear();
        break;
    case SqrtIALUICmp:
        config.di.delayPat.push_back({0x00, 0xc0, 0x61, 0x1e}); // sqrt d0, d0
        config.di.prologuePat.clear();
        config.di.contentPat.push_back({0x20, 0x00, 0x01, 0x8b}); // add x0, x1, x1
        config.di.contentPat.push_back({0x5f, 0x00, 0x03, 0xeb}); // cmp x2, x3
        config.di.epiloguePat.clear();
        break;
    case SqrtIFALU:
        config.di.delayPat.push_back({0x00, 0xc0, 0x61, 0x1e}); // sqrt d0, d0
        config.di.prologuePat.clear();
        config.di.contentPat.push_back({0x20, 0x00, 0x01, 0x8b}); // add x0, x1, x1
        config.di.contentPat.push_back({0x01, 0xc0, 0x60, 0x1e}); // fabs d1, d0
        config.di.epiloguePat.clear();
        break;
    case SqrtLoad:
        config.di.delayPat.push_back({0x00, 0xc0, 0x61, 0x1e}); // sqrt d0, d0
        config.di.prologuePat.clear();
        config.di.contentPat.push_back({0x41, 0x00, 0x40, 0xf9}); // ldr x1, [x2]
        config.di.epiloguePat.clear();
        break;
    case SqrtLoadSeq:
        config.di.delayPat.push_back({0x00, 0xc0, 0x61, 0x1e}); // sqrt d0, d0
        config.di.prologuePat.clear();
        config.di.contentPat.push_back({0x41, 0x84, 0x40, 0xf8}); // ldr x1, [x2], 8
        config.di.epiloguePat.clear();
        break;
    case SqrtLoadUnKnownAddr:
        config.di.delayPat.push_back({0x00, 0xc0, 0x61, 0x1e});    // sqrt d0, d0
        config.di.prologuePat.push_back({0x01, 0x00, 0x79, 0x9e}); // fcvtzu x1, d0
        config.di.contentPat.push_back({0x40, 0x68, 0x61, 0xf8});  // ldr x0, [x2, x1]
        config.di.epiloguePat.clear();
        break;
    case SqrtLoadChain:
        config.di.delayPat.push_back({0x00, 0xc0, 0x61, 0x1e});    // sqrt d0, d0
        config.di.prologuePat.push_back({0x42, 0x00, 0x00, 0xf9}); // str x2, [x2]
        config.di.contentPat.push_back({0x42, 0x00, 0x40, 0xf9});  // ldr x2, [x2]
        config.di.epiloguePat.clear();
        break;
    case SqrtStore:
        config.di.delayPat.push_back({0x00, 0xc0, 0x61, 0x1e}); // sqrt d0, d0
        config.di.prologuePat.clear();
        config.di.contentPat.push_back({0x40, 0x00, 0x00, 0xf9}); // str x0, [x2]
        config.di.epiloguePat.clear();
        break;
    case SqrtStoreSeq:
        config.di.delayPat.push_back({0x00, 0xc0, 0x61, 0x1e}); // sqrt d0, d0
        config.di.prologuePat.clear();
        config.di.contentPat.push_back({0x40, 0x84, 0x00, 0xf8}); // str x0, [x2], 8
        config.di.epiloguePat.clear();
        break;
    case SqrtStoreUnknownAddr:
        config.di.delayPat.push_back({0x00, 0xc0, 0x61, 0x1e});    // sqrt d0, d0
        config.di.prologuePat.push_back({0x01, 0x00, 0x79, 0x9e}); // fcvtzu x1, d0
        config.di.contentPat.push_back({0x40, 0x68, 0x21, 0xf8});  // str x0, [x2, x1]
        config.di.epiloguePat.clear();
        break;
    case SqrtStoreUnknownVal:
        config.di.delayPat.push_back({0x00, 0xc0, 0x61, 0x1e});    // sqrt d0, d0
        config.di.prologuePat.push_back({0x01, 0x00, 0x79, 0x9e}); // fcvtzu x1, d0
        config.di.contentPat.push_back({0x41, 0x00, 0x00, 0xf9});  // str x1, [x2]
        config.di.epiloguePat.clear();
        break;
    case SqrtCJmp:
        config.di.delayPat.push_back({0x00, 0xc0, 0x61, 0x1e});    // sqrt d0, d0
        config.di.prologuePat.push_back({0x1f, 0x00, 0x00, 0xeb}); // cmp x0, x0
        config.di.contentPat.push_back({0x20, 0x00, 0x00, 0x54});  // b.eq .+4
        config.di.epiloguePat.clear();
        break;
    case SqrtJmp:
        config.di.delayPat.push_back({0x00, 0xc0, 0x61, 0x1e}); // sqrt d0, d0
        config.di.prologuePat.clear();
        config.di.contentPat.push_back({0x01, 0x00, 0x00, 0x14}); // b .+4
        config.di.epiloguePat.clear();
        break;
    case SqrtMixJmp:
        config.di.delayPat.push_back({0x00, 0xc0, 0x61, 0x1e});    // sqrt d0, d0
        config.di.prologuePat.push_back({0x1f, 0x00, 0x00, 0xeb}); // cmp x0, x0
        config.di.contentPat.push_back({0x20, 0x00, 0x00, 0x54});  // b.eq .+4
        config.di.contentPat.push_back({0x01, 0x00, 0x00, 0x14});  // b .+4
        config.di.epiloguePat.clear();
        break;
    case SqrtNopIALU:
        config.di.delayPat.push_back({0x00, 0xc0, 0x61, 0x1e}); // sqrt d0, d0
        config.di.prologuePat.clear();
        config.di.contentPat.push_back({0x1f, 0x20, 0x03, 0xd5});    // nop
        config.di.epiloguePat.push_back({{0x20, 0x00, 0x01, 0x8b}}); // add x0, x1, x1
        break;
    case SqrtVFALUIALU:
        config.di.delayPat.push_back({0x00, 0xc0, 0x61, 0x1e}); // sqrt d0, d0
        config.di.prologuePat.clear();
        config.di.contentPat.push_back({0x01, 0xc0, 0x60, 0x1e});    // fabs d1, d0
        config.di.epiloguePat.push_back({{0x20, 0x00, 0x01, 0x8b}}); // add x0, x1, x1
        break;
    case SchSqrtIALUDep:
        config.di.delayPat.push_back({0x00, 0xc0, 0x61, 0x1e});    // sqrt d0, d0
        config.di.prologuePat.push_back({0x01, 0x00, 0x79, 0x9e}); // fcvtzu x1, d0
        config.di.contentPat.push_back({0x20, 0x00, 0x01, 0x8b});  // add x0, x1, x1
        config.di.epiloguePat.clear();
        break;
    case SchSqrtIALUChainDep:
        config.di.delayPat.push_back({0x00, 0xc0, 0x61, 0x1e});    // sqrt d0, d0
        config.di.prologuePat.push_back({0x01, 0x00, 0x79, 0x9e}); // fcvtzu x1, d0
        config.di.contentPat.push_back({0x00, 0x00, 0x01, 0x8b});  // add x0, x0, x1
        config.di.epiloguePat.clear();
        break;
    case SchSDivFALUDep:
        config.di.delayPat.push_back({0x41, 0x0c, 0xc1, 0x9a});    // sdiv x1, x2, x1
        config.di.prologuePat.push_back({0x20, 0x00, 0x62, 0x9e}); // scvtf d0, x1
        config.di.contentPat.push_back({0x01, 0xc0, 0x60, 0x1e});  // fabs d1, d0
        config.di.epiloguePat.clear();
        break;
    case SchSDivFALUChainDep:
        config.di.delayPat.push_back({0x41, 0x0c, 0xc1, 0x9a});    // sdiv x1, x2, x1
        config.di.prologuePat.push_back({0x20, 0x00, 0x62, 0x9e}); // scvtf d0, x1
        config.di.contentPat.push_back({0x00, 0xc0, 0x60, 0x1e});  // fabs d0, d0
        config.di.epiloguePat.clear();
        break;
    case SchIALUNop:
        config.di.delayPat.push_back({0x20, 0x00, 0x01, 0x8b}); // add x0, x1, x1
        config.di.prologuePat.clear();
        config.di.contentPat.push_back({0x1f, 0x20, 0x03, 0xd5}); // nop
        config.di.epiloguePat.clear();
        break;
    case SchIALUChainNop:
        config.di.delayPat.push_back({0x00, 0x00, 0x01, 0x8b}); // add x0, x0, x1
        config.di.prologuePat.clear();
        config.di.contentPat.push_back({0x1f, 0x20, 0x03, 0xd5}); // nop
        config.di.epiloguePat.clear();
        break;
    case SchICmpNop:
        config.di.delayPat.push_back({0x1f, 0x00, 0x01, 0xeb}); // cmp x0, x1
        config.di.prologuePat.clear();
        config.di.contentPat.push_back({0x1f, 0x20, 0x03, 0xd5}); // nop
        config.di.epiloguePat.clear();
        break;
    case SchFALUNop:
        config.di.delayPat.push_back({0x01, 0xc0, 0x60, 0x1e}); // fabs d1, d0
        config.di.prologuePat.clear();
        config.di.contentPat.push_back({0x1f, 0x20, 0x03, 0xd5}); // nop
        config.di.epiloguePat.clear();
        break;
    case SchFALUChainNop:
        config.di.delayPat.push_back({0x00, 0xc0, 0x60, 0x1e}); // fabs d0, d0
        config.di.prologuePat.clear();
        config.di.contentPat.push_back({0x1f, 0x20, 0x03, 0xd5}); // nop
        config.di.epiloguePat.clear();
        break;
    case PeriodIALUNop:
        config.pi.periodPat.push_back({0x20, 0x00, 0x01, 0x8b}); // add x0, x1, x1
        config.pi.fillPat.push_back({0x1f, 0x20, 0x03, 0xd5});   // nop
        break;
    case PeriodICmpNop:
        config.pi.periodPat.push_back({0x1f, 0x00, 0x01, 0xeb}); // cmp x0, x1
        config.pi.fillPat.push_back({0x1f, 0x20, 0x03, 0xd5});   // nop
        break;
    case PeriodFALUNop:
        config.pi.periodPat.push_back({0x41, 0xc0, 0x20, 0x1e}); // fabs s1, s2
        config.pi.fillPat.push_back({0x1f, 0x20, 0x03, 0xd5});   // nop
        break;
    default:
        return false;
    }
    return true;
}

void fillnop(unsigned char *instBuf, unsigned sizeBytes) {
    genCodeStart();
    unsigned nopCnt = sizeBytes / 4;
    unsigned *inst = (unsigned *)instBuf;
    for (unsigned i = 0; i < nopCnt; i++)
        inst[i] = 0xd503201f;
    genCodeEnd(instBuf, sizeBytes);
}

#endif // __arch64__ || _M_ARM64