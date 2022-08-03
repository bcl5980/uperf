#ifndef __GEN_AARCH64_H__
#define __GEN_AARCH64_H__

bool genPattern(TestCase caseId, unsigned char *instBuf, int testCnt, int delayCnt, int codeDupCnt, int codeLoopCnt,
                int gp) {
    int i = 0;

    genCodeStart();

    // Microsft AARCH64 calling convention:
    // X0-X17, v0-v7, v16-v31 volatile, we can use them
    // X18-X30, v8-v15 nonvolatile, we can't use them
    // https://docs.microsoft.com/en-us/cpp/build/arm64-windows-abi-conventions?view=msvc-170
    unsigned int *inst = (unsigned int *)instBuf;
    for (int k = 0; k < codeDupCnt; k++) {
        if (caseId == UdivVFAdd || caseId == SqrtMovSelfFp) {
            for (int j = 0; j < delayCnt; j++) {
                inst[i++] = 0x9ac10821; // udiv x1, x1, x1
            }
        } else if (caseId >= SqrtNop) {
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
        case SqrtStoreUnknownVal:
            // fcvtzu x1, d0
            inst[i++] = 0x9e790001;
            break;
        default:
            break;
        }

        for (int j = 0; j < testCnt; j++) {
            switch (caseId) {
            case InstNop:
                inst[i++] = 0xd503201f; // nop
                break;
            case InstMov:
                inst[i++] = 0xaa0103e0; // mov x0, x1
                break;
            case InstIAdd:
                inst[i++] = 0x8b010020; // add x0, x1, x1
                break;
            case InstIAddChain:
                inst[i++] = 0x8b010000; // add x0, x0, x1
                break;
            case InstFAdd:
                inst[i++] = 0x1e222841; // fadd s1, s2, s2
                break;
            case InstFAddChain:
                inst[i++] = 0x1e222821; // fadd s1, s1, s2
                break;
            case InstCmp:
                inst[i++] = 0xeb01001f; // cmp x0, x1
                break;
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
                return false;
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