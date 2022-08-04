#ifndef __GEN_AMD64_H__
#define __GEN_AMD64_H__

bool genPattern(TestCase caseId, unsigned char *instBuf, int testCnt, int delayCnt, int codeDupCnt, int gp) {
    int i = 0;

    // Microsft X64 calling convention:
    // RAX, RCX, RDX, R8, R9, R10, R11, and XMM0-XMM5 volatile, we can write them without saving
    // RBX, RBP, RDI, RSI, RSP, R12, R13, R14, R15, and XMM6-XMM15 nonvolatile, we can't write them
    // https://docs.microsoft.com/en-us/cpp/build/x64-calling-convention?view=msvc-170

    for (int k = 0; k < codeDupCnt; k++) {
        if (caseId == UdivVFALU) {
            for (int j = 0; j < delayCnt; j++) {
                instBuf[i++] = 0x48; // xor    rdx,rdx
                instBuf[i++] = 0x31;
                instBuf[i++] = 0xd2;
                instBuf[i++] = 0x48; // div    rcx
                instBuf[i++] = 0xf7;
                instBuf[i++] = 0xf1;
            }
        } else if (caseId >= SqrtNop) {
            for (int j = 0; j < delayCnt; j++) {
                instBuf[i++] = 0xf2; // sqrtsd %xmm0, %xmm0
                instBuf[i++] = 0x0f;
                instBuf[i++] = 0x51;
                instBuf[i++] = 0xc0;
            }
        }

        switch (caseId) {
        case SqrtCJmp:
        case SqrtMixJmp:
            // test rcx, rcx
            instBuf[i++] = 0x48;
            instBuf[i++] = 0x85;
            instBuf[i++] = 0xC9;
            break;
        case SqrtLoadUnKnownAddr:
        case SqrtStoreUnknownAddr:
        case SqrtStoreUnknownVal:
            // cvttsd2si rcx,xmm0
            instBuf[i++] = 0xf2;
            instBuf[i++] = 0x48;
            instBuf[i++] = 0x0f;
            instBuf[i++] = 0x2c;
            instBuf[i++] = 0xc8;
            break;
        default:
            break;
        }

        for (int j = 0; j < testCnt; j++) {
            switch (caseId) {
            case InstIALUChain:
                instBuf[i++] = 0x48; // add rax, rcx
                instBuf[i++] = 0x01;
                instBuf[i++] = 0xc8;
                break;
            case InstFALUChain:
                instBuf[i++] = 0xc5; // VPADDD xmm0, xmm0, xmm1
                instBuf[i++] = 0xf9;
                instBuf[i++] = 0xfe;
                instBuf[i++] = 0xc1;
                break;
            case InstLea3:
                instBuf[i++] = 0x48; // lea rax, [rcx+8*rax+42]
                instBuf[i++] = 0x8d;
                instBuf[i++] = 0x44;
                instBuf[i++] = 0xd1;
                instBuf[i++] = 0x2a;
                break;
            case InstLea3Chain:
                instBuf[i++] = 0x48; // lea rax, [rcx+8*rax+42]
                instBuf[i++] = 0x8d;
                instBuf[i++] = 0x44;
                instBuf[i++] = 0xc1;
                instBuf[i++] = 0x2a;
                break;
            case InstNop:
            case SqrtNop:
            case SqrtNopIALU:
                instBuf[i++] = 0x48; // nop
                instBuf[i++] = 0x89;
                instBuf[i++] = 0xc8;
                break;
            case InstMov:
            case SqrtMov:
                instBuf[i++] = 0x48; // mov rax, rcx
                instBuf[i++] = 0x89;
                instBuf[i++] = 0xc8;
                break;
            case SqrtMovSelf:
                instBuf[i++] = 0x48; // mov rax, rax
                instBuf[i++] = 0x89;
                instBuf[i++] = 0xc0;
                break;
            case SqrtMovSelfFp:
                instBuf[i++] = 0xf3; // movq xmm1, xmm1
                instBuf[i++] = 0x0f;
                instBuf[i++] = 0x7e;
                instBuf[i++] = 0xc9;
                break;
            case InstIALU:
            case SqrtIALU:
                instBuf[i++] = 0x48; // lea rax,[rbx+rcx]
                instBuf[i++] = 0x8d;
                instBuf[i++] = 0x04;
                instBuf[i++] = 0x0b;
                break;
            case InstFALU:
            case UdivVFALU:
                instBuf[i++] = 0xc5; // VPADDD xmm0, xmm1, xmm1
                instBuf[i++] = 0xf1;
                instBuf[i++] = 0xfe;
                instBuf[i++] = 0xc1;
                break;
            case InstCmp:
            case SqrtCmp:
                instBuf[i++] = 0x48; // cmp rax, rcx
                instBuf[i++] = 0x39;
                instBuf[i++] = 0xC8;
                break;
            case SqrtIALUICmp:
                instBuf[i++] = 0x48; // lea rax,[rbx+rcx]
                instBuf[i++] = 0x8d;
                instBuf[i++] = 0x04;
                instBuf[i++] = 0x0b;
                instBuf[i++] = 0x4c; // cmp rdx, r8
                instBuf[i++] = 0x39;
                instBuf[i++] = 0xC2;
                break;
            case SqrtIFALU:
                instBuf[i++] = 0x48; // lea rax,[rbx+rcx]
                instBuf[i++] = 0x8d;
                instBuf[i++] = 0x04;
                instBuf[i++] = 0x0b;
                instBuf[i++] = 0xc5; // VPADDD xmm0, xmm1, xmm1
                instBuf[i++] = 0xf1;
                instBuf[i++] = 0xfe;
                instBuf[i++] = 0xc1;
                break;
            case SqrtLoad:
                instBuf[i++] = 0x49; // mov rax, QWORD PTR [r8]
                instBuf[i++] = 0x8b;
                instBuf[i++] = 0x00;
                break;
            case SqrtLoadSeq:
                instBuf[i++] = 0x49; // mov rax, QWORD PTR [r8]
                instBuf[i++] = 0x8b;
                instBuf[i++] = 0x00;
                instBuf[i++] = 0x49; // add r8, 8
                instBuf[i++] = 0x83;
                instBuf[i++] = 0xc0;
                instBuf[i++] = 0x08;
                break;
            case SqrtLoadUnKnownAddr:
                instBuf[i++] = 0x49; // mov rax, QWORD PTR [r8+rcx]
                instBuf[i++] = 0x8b;
                instBuf[i++] = 0x04;
                instBuf[i++] = 0x08;
                break;
            case SqrtStore:
                instBuf[i++] = 0x49; // mov QWORD PTR [r8], rax
                instBuf[i++] = 0x89;
                instBuf[i++] = 0x00;
                break;
            case SqrtStoreSeq:
                instBuf[i++] = 0x49; // mov QWORD PTR [r8], rax
                instBuf[i++] = 0x89;
                instBuf[i++] = 0x00;
                instBuf[i++] = 0x49; // add r8, 8
                instBuf[i++] = 0x83;
                instBuf[i++] = 0xc0;
                instBuf[i++] = 0x08;
                break;
            case SqrtStoreUnknownAddr:
                instBuf[i++] = 0x49; // mov QWORD PTR [r8+rcx], rax
                instBuf[i++] = 0x89;
                instBuf[i++] = 0x04;
                instBuf[i++] = 0x08;
                break;
            case SqrtStoreUnknownVal:
                instBuf[i++] = 0x49; // mov QWORD PTR [r8], rcx
                instBuf[i++] = 0x89;
                instBuf[i++] = 0x08;
                break;
            case SqrtCJmp:
                instBuf[i++] = 0x75; // jnz 0
                instBuf[i++] = 0x00;
                break;
            case SqrtJmp:
                instBuf[i++] = 0xeb; // jmp 0
                instBuf[i++] = 0x00;
                break;
            case SqrtMixJmp:
                instBuf[i++] = 0x75; // jnz 0
                instBuf[i++] = 0x00;
                instBuf[i++] = 0xeb; // jmp 0
                instBuf[i++] = 0x00;
                break;
            case SqrtVFALUIALU:
                instBuf[i++] = 0xc5; // VPADDD xmm2, xmm1, xmm1
                instBuf[i++] = 0xf1;
                instBuf[i++] = 0xfe;
                instBuf[i++] = 0xd1;
                break;
            default:
                return false;
            }
        }

        if (caseId == SqrtNopIALU || caseId == SqrtVFALUIALU) {
            for (int j = 0; j < gp; j++) {
                instBuf[i++] = 0x48; // lea rax,[rbx+rcx]
                instBuf[i++] = 0x8d;
                instBuf[i++] = 0x04;
                instBuf[i++] = 0x0b;
            }
        }
    }
    // ret
    instBuf[i++] = 0xc3;
    return true;
}

void fillnop(unsigned char *instBuf, unsigned sizeBytes) { memset(instBuf, 0x90, sizeBytes); }

#endif // __GEN_AMD64_H__