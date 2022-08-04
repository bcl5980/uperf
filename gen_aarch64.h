#ifndef __GEN_AARCH64_H__
#define __GEN_AARCH64_H__

static void genDelayPattern(TestCase caseId, unsigned int *inst, int delayCnt, unsigned &i) {
    if (caseId == UdivVFAdd || caseId == SqrtMovSelfFp) {
        for (int j = 0; j < delayCnt; j++) {
            inst[i++] = 0x9ac10821; // udiv x1, x1, x1
        }
    } else if (caseId >= SqrtNop && caseId < SchIAddNop) {
        for (int j = 0; j < delayCnt; j++) {
            inst[i++] = 0x1e61c000; // sqrt d0, d0
        }
    } else {
        for (int j = 0; j < delayCnt; j++) {
            switch (caseId) {
            case SchIAddNop:
                inst[i++] = 0x8b010020; // add x0, x1, x1
                break;
            case SchIAddChainNop:
                inst[i++] = 0x8b010000; // add x0, x0, x1
                break;
            case SchICmpNop:
                inst[i++] = 0xeb01001f; // cmp x0, x1
                break;
            case SchFAddNop:
                inst[i++] = 0x1e222841; // fadd s1, s2, s2
                break;
            case SchFAddChainNop:
                inst[i++] = 0x1e222821; // fadd s1, s1, s2
                break;
            default:
                break;
            }
        }
    }
}

static void genPrologue(TestCase caseId, unsigned int *inst, unsigned &i) {
    switch (caseId) {
    case SqrtCJmp:
    case SqrtMixJmp:
        // cmp x0, x0
        inst[i++] = 0xeb00001f;
        break;
    case SqrtLoadUnKnownAddr:
    case SqrtStoreUnknownAddr:
    case SqrtStoreUnknownVal:
        // fcvtzu x1, d0
        inst[i++] = 0x9e790001;
        break;
    default:
        break;
    }
}

static bool genContent(TestCase caseId, unsigned int *inst, int testCnt, int gp, unsigned &i) {
    for (int j = 0; j < testCnt; j++) {
        switch (caseId) {
        case InstIAddChain:
            inst[i++] = 0x8b010000; // add x0, x0, x1
            break;
        case InstFAddChain:
            inst[i++] = 0x1e222821; // fadd s1, s1, s2
            break;
        case InstNop:
        case SqrtNop:
        case SqrtNopIAdd:
        case SchIAddNop:
        case SchIAddChainNop:
        case SchICmpNop:
        case SchFAddNop:
        case SchFAddChainNop:
            inst[i++] = 0xd503201f; // nop
            break;
        case InstMov:
        case SqrtMov:
            inst[i++] = 0xaa0103e0; // mov x0, x1
            break;
        case SqrtMovSelf:
            inst[i++] = 0xaa0103e1; // mov x1, x1
            break;
        case SqrtMovSelfFp:
            inst[i++] = 0x1e604021; // fmov d1, d1
            break;
        case InstIAdd:
        case SqrtIAdd:
            inst[i++] = 0x8b010020; // add x0, x1, x1
            break;
        case InstFAdd:
        case UdivVFAdd:
        case SqrtVFAddIAdd:
            inst[i++] = 0x1e222841; // fadd s1, s2, s2
            break;
        case InstCmp:
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
            return false;
        }
    }
    return true;
}

static void genEpilogue(TestCase caseId, unsigned int *inst, int gp, unsigned &i) {
    if (caseId == SqrtNopIAdd || caseId == SqrtVFAddIAdd) {
        for (int j = 0; j < gp; j++)
            inst[i++] = 0x8b010020; // add x0, x1, x1
    }
}

static bool genPeriodPattern(TestCase caseId, unsigned int *inst, int period, int instNum, int instThroughPut,
                             int nopThroughPut, unsigned &i) {
    int InstCnt = instThroughPut * period;
    int PeriodCnt = nopThroughPut * period;
    for (int j = 0, k = 0; j < instNum; j++) {
        if (k < InstCnt) {
            switch (caseId) {
            case PeriodIAddNop:
                inst[i++] = 0x8b010020; // add x0, x1, x1
                break;
            case PeriodICmpNop:
                inst[i++] = 0xeb01001f; // cmp x0, x1
                break;
            case PeriodFAddNop:
                inst[i++] = 0x1e222841; // fadd s1, s2, s2
                break;
            default:
                return false;
            }
        } else {
            inst[i++] = 0xd503201f; // nop
        }
        k++;
        if (k >= PeriodCnt)
            k = 0;
    }
    return true;
}

bool genPattern(TestCase caseId, unsigned char *instBuf, int testCnt, int delayCnt, int codeDupCnt, int gp) {
    unsigned i = 0;
    unsigned int *inst = (unsigned int *)instBuf;
    genCodeStart();

    // Microsft AARCH64 calling convention:
    // X0-X17, v0-v7, v16-v31 volatile, we can use them
    // X18-X30, v8-v15 nonvolatile, we can't use them
    // https://docs.microsoft.com/en-us/cpp/build/arm64-windows-abi-conventions?view=msvc-170
    if (caseId >= PeriodIAddNop) {
        genPeriodPattern(caseId, inst, testCnt, delayCnt, codeDupCnt, gp, i);
    } else {
        for (int k = 0; k < codeDupCnt; k++) {
            genDelayPattern(caseId, inst, delayCnt, i);
            genPrologue(caseId, inst, i);
            if (!genContent(caseId, inst, testCnt, gp, i))
                return false;
            genEpilogue(caseId, inst, gp, i);
        }
    }

    // ret 0xd65f03c0
    inst[i++] = 0xd65f03c0;

    genCodeEnd(inst, i * sizeof(int));
    return true;
}

void fillnop(unsigned char *instBuf, unsigned sizeBytes) {
    genCodeStart();
    unsigned nopCnt = sizeBytes / 4;
    unsigned *inst = (unsigned *)instBuf;
    for (int i = 0; i < nopCnt; i++)
        inst[i] = 0xd503201f;
    genCodeEnd(instBuf, sizeBytes);
}

#endif // __GEN_AARCH64_H__