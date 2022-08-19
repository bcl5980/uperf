#if defined(__amd64__) || defined(_M_AMD64)
#include "archbase.h"
#include "osutils.h"
#include <vector>

bool genPattern(PatConfig &config, unsigned char *instBuf, TestParam &param, unsigned testCnt) {
    size_t i = 0;
    genCodeStart();

    // Microsft X64 calling convention:
    // RAX, RCX, RDX, R8, R9, R10, R11, and XMM0-XMM5 volatile, we can write them without saving
    // RBX, RBP, RDI, RSI, RSP, R12, R13, R14, R15, and XMM6-XMM15 nonvolatile, we can't write them
    // https://docs.microsoft.com/en-us/cpp/build/x64-calling-convention?view=msvc-170
    if (config.mode == WorkMode::PeriodTest) {
        genPeriodPattern(config, instBuf, testCnt, param.instNum, param.testInstTP,
                         param.fillInstTP, i);
    } else {
        genDelayPattern(config.di.delayPat, instBuf, param.delayCnt, i);
        genDelayPattern(config.di.prologuePat, instBuf, param.prologueCnt, i);
        genDelayPattern(config.di.contentPat, instBuf, testCnt, i);
        genDelayPattern(config.di.epiloguePat, instBuf, param.epilogueCnt, i);
    }

    instBuf[i++] = 0xc3;

    genCodeEnd(instBuf, i);
    return true;
}

bool genConfigForDefaultCases(TestCase caseId, PatConfig &config) {
    config.arch = ArchType::X86_64;
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
        config.di.contentPat.push_back({0x90}); // nop
        config.di.epiloguePat.clear();
        break;
    case InstMov:
        config.di.delayPat.clear();
        config.di.prologuePat.clear();
        config.di.contentPat.push_back({0x48, 0x89, 0xC8}); // mov rax, rcx
        config.di.epiloguePat.clear();
        break;
    case InstIALU:
        config.di.delayPat.clear();
        config.di.prologuePat.clear();
        config.di.contentPat.push_back({0x48, 0x8D, 0x41, 0x04}); // lea rax, [rcx+4]
        config.di.epiloguePat.clear();
        break;
    case InstIALUChain:
        config.di.delayPat.clear();
        config.di.prologuePat.clear();
        config.di.contentPat.push_back({0x48, 0x83, 0xC0, 0x04}); // add rax, 4
        config.di.epiloguePat.clear();
        break;
    case InstFALU:
        config.di.delayPat.clear();
        config.di.prologuePat.clear();
        config.di.contentPat.push_back({0xC5, 0xF1, 0xFE, 0xC1}); // VPADDD xmm0, xmm1, xmm1
        config.di.epiloguePat.clear();
        break;
    case InstFALUChain:
        config.di.delayPat.clear();
        config.di.prologuePat.clear();
        config.di.contentPat.push_back({0xC5, 0xF9, 0xFE, 0xC1}); // VPADDD xmm0, xmm0, xmm1
        config.di.epiloguePat.clear();
        break;
    case InstICmp:
        config.di.delayPat.clear();
        config.di.prologuePat.clear();
        config.di.contentPat.push_back({0x48, 0x39, 0xC8}); // cmp rax, rcx
        config.di.epiloguePat.clear();
        break;
    case InstLea3:
        config.di.delayPat.clear();
        config.di.prologuePat.clear();
        config.di.contentPat.push_back({0x48, 0x8D, 0x44, 0xD1, 0x2A}); // lea rax, [rcx+8*rdx+42]
        config.di.epiloguePat.clear();
        break;
    case InstLea3Chain:
        config.di.delayPat.clear();
        config.di.prologuePat.clear();
        config.di.contentPat.push_back({0x48, 0x8D, 0x44, 0xC1, 0x2A}); // lea rax, [rcx+8*rax+42]
        config.di.epiloguePat.clear();
        break;
    case InstLoad:
        config.di.delayPat.clear();
        config.di.prologuePat.clear();
        config.di.contentPat.push_back({0x49, 0x8B, 0x00}); // mov rax, QWORD PTR [r8]
        config.di.epiloguePat.clear();
        break;
    case InstStore:
        config.di.delayPat.clear();
        config.di.prologuePat.clear();
        config.di.contentPat.push_back({0x49, 0x89, 0x00}); // mov QWORD PTR [r8], rax
        config.di.epiloguePat.clear();
        break;
    case SqrtNop:
        config.di.delayPat.push_back({0xF2, 0x0F, 0x51, 0xC0}); // sqrtsd xmm0,xmm0
        config.di.prologuePat.clear();
        config.di.contentPat.push_back({0x90}); // nop
        config.di.epiloguePat.clear();
        break;
    case SqrtMov:
        config.di.delayPat.push_back({0xF2, 0x0F, 0x51, 0xC0}); // sqrtsd xmm0,xmm0
        config.di.prologuePat.clear();
        config.di.contentPat.push_back({0x48, 0x89, 0xC8}); // mov rax, rcx
        config.di.epiloguePat.clear();
        break;
    case SqrtMovSelf:
        config.di.delayPat.push_back({0xF2, 0x0F, 0x51, 0xC0}); // sqrtsd xmm0,xmm0
        config.di.prologuePat.clear();
        config.di.contentPat.push_back({0x48, 0x89, 0xC0}); // mov rax, rax
        config.di.epiloguePat.clear();
        break;
    case SqrtMovSelfFp:
        config.args.iArg0 = 7;                            // rcx = 7;
        config.di.delayPat.push_back({0x48, 0x31, 0xD2}); // xor rdx,rdx;
        config.di.delayPat.push_back({0x48, 0xF7, 0xF1}); // div rcx
        config.di.prologuePat.clear();
        config.di.contentPat.push_back({0xF3, 0x0F, 0x7E, 0xC9}); // movq xmm1, xmm1
        config.di.epiloguePat.clear();
        break;
    case SqrtIALU:
        config.di.delayPat.push_back({0xF2, 0x0F, 0x51, 0xC0}); // sqrtsd xmm0,xmm0
        config.di.prologuePat.clear();
        config.di.contentPat.push_back({0x48, 0x8D, 0x41, 0x04}); // lea rax, [rcx+4]
        config.di.epiloguePat.clear();
        break;
    case UdivVFALU:
        config.args.iArg0 = 7;                            // rcx = 7;
        config.di.delayPat.push_back({0x48, 0x31, 0xD2}); // xor rdx,rdx;
        config.di.delayPat.push_back({0x48, 0xF7, 0xF1}); // div rcx
        config.di.prologuePat.clear();
        config.di.contentPat.push_back({0xC5, 0xF1, 0xFE, 0xC1}); // VPADDD xmm0, xmm1, xmm1
        config.di.epiloguePat.clear();
        break;
    case SqrtICmp:
        config.di.delayPat.push_back({0xF2, 0x0F, 0x51, 0xC0}); // sqrtsd xmm0,xmm0
        config.di.prologuePat.clear();
        config.di.contentPat.push_back({0x48, 0x39, 0xC8}); // cmp rax, rcx
        config.di.epiloguePat.clear();
        break;
    case SqrtIALUICmp:
        config.di.delayPat.push_back({0xF2, 0x0F, 0x51, 0xC0}); // sqrtsd xmm0,xmm0
        config.di.prologuePat.clear();
        config.di.contentPat.push_back({0x48, 0x8D, 0x41, 0x04}); // lea rax, [rcx+4]
        config.di.contentPat.push_back({0x48, 0x39, 0xC8});       // cmp rax, rcx
        config.di.epiloguePat.clear();
        break;
    case SqrtIFALU:
        config.di.delayPat.push_back({0xF2, 0x0F, 0x51, 0xC0}); // sqrtsd xmm0,xmm0
        config.di.prologuePat.clear();
        config.di.contentPat.push_back({0x48, 0x8D, 0x41, 0x04}); // lea rax, [rcx+4]
        config.di.contentPat.push_back({0xC5, 0xF1, 0xFE, 0xC1}); // VPADDD xmm0, xmm1, xmm1
        config.di.epiloguePat.clear();
        break;
    case SqrtLoad:
        config.di.delayPat.push_back({0xF2, 0x0F, 0x51, 0xC0}); // sqrtsd xmm0,xmm0
        config.di.prologuePat.clear();
        config.di.contentPat.push_back({0x49, 0x8B, 0x00}); // mov rax, QWORD PTR [r8]
        config.di.epiloguePat.clear();
        break;
    case SqrtLoadSeq:
        config.di.delayPat.push_back({0xF2, 0x0F, 0x51, 0xC0}); // sqrtsd xmm0,xmm0
        config.di.prologuePat.clear();
        config.di.contentPat.push_back({0x49, 0x8B, 0x00});       // mov rax, QWORD PTR [r8]
        config.di.contentPat.push_back({0x49, 0x83, 0xC0, 0x08}); // add r8, 8
        config.di.epiloguePat.clear();
        break;
    case SqrtLoadUnKnownAddr:
        config.di.delayPat.push_back({0xF2, 0x0F, 0x51, 0xC0});          // sqrtsd xmm0,xmm0
        config.di.prologuePat.push_back({0xF2, 0x48, 0x0F, 0x2C, 0xC8}); // cvttsd2si rcx,xmm0
        config.di.contentPat.push_back({0x49, 0x8B, 0x04, 0x08}); // mov rax, QWORD PTR [r8+rcx]
        config.di.epiloguePat.clear();
        break;
    case SqrtLoadChain:
        config.di.delayPat.push_back({0xF2, 0x0F, 0x51, 0xC0}); // sqrtsd xmm0,xmm0
        config.di.prologuePat.push_back({0x4D, 0x89, 0x00});    // mov QWORD PTR [r8], r8
        config.di.contentPat.push_back({0x4D, 0x8B, 0x00});     // mov r8, QWORD PTR [r8]
        config.di.epiloguePat.clear();
        break;
    case SqrtStore:
        config.di.delayPat.push_back({0xF2, 0x0F, 0x51, 0xC0}); // sqrtsd xmm0,xmm0
        config.di.prologuePat.clear();
        config.di.contentPat.push_back({0x49, 0x89, 0x00}); // mov QWORD PTR [r8], rax
        config.di.epiloguePat.clear();
        break;
    case SqrtStoreSeq:
        config.di.delayPat.push_back({0xF2, 0x0F, 0x51, 0xC0}); // sqrtsd xmm0,xmm0
        config.di.prologuePat.clear();
        config.di.contentPat.push_back({0x49, 0x89, 0x00});       // mov QWORD PTR [r8], rax
        config.di.contentPat.push_back({0x49, 0x83, 0xC0, 0x08}); // add r8, 8
        config.di.epiloguePat.clear();
        break;
    case SqrtStoreUnknownAddr:
        config.di.delayPat.push_back({0xF2, 0x0F, 0x51, 0xC0});          // sqrtsd xmm0,xmm0
        config.di.prologuePat.push_back({0xF2, 0x48, 0x0F, 0x2C, 0xC8}); // cvttsd2si rcx,xmm0
        config.di.contentPat.push_back({0x49, 0x89, 0x04, 0x08}); // mov QWORD PTR [r8+rcx], rax
        config.di.epiloguePat.clear();
        break;
    case SqrtStoreUnknownVal:
        config.di.delayPat.push_back({0xF2, 0x0F, 0x51, 0xC0});          // sqrtsd xmm0,xmm0
        config.di.prologuePat.push_back({0xF2, 0x48, 0x0F, 0x2C, 0xC8}); // cvttsd2si rcx,xmm0
        config.di.contentPat.push_back({0x49, 0x89, 0x08});              // mov QWORD PTR [r8], rcx
        config.di.epiloguePat.clear();
        break;
    case SqrtCJmp:
        config.di.delayPat.push_back({0xF2, 0x0F, 0x51, 0xC0}); // sqrtsd xmm0,xmm0
        config.di.prologuePat.push_back({0x48, 0x85, 0xC9});    // test rcx, rcx
        config.di.contentPat.push_back({0x75, 0x00});           // jne 2
        config.di.epiloguePat.clear();
        break;
    case SqrtJmp:
        config.di.delayPat.push_back({0xF2, 0x0F, 0x51, 0xC0}); // sqrtsd xmm0,xmm0
        config.di.prologuePat.clear();
        config.di.contentPat.push_back({0xeb, 0x00}); // jmp 2
        config.di.epiloguePat.clear();
        break;
    case SqrtMixJmp:
        config.di.delayPat.push_back({0xF2, 0x0F, 0x51, 0xC0}); // sqrtsd xmm0,xmm0
        config.di.prologuePat.push_back({0x48, 0x85, 0xC9});    // test rcx, rcx
        config.di.contentPat.push_back({0x75, 0x00});           // jne 2
        config.di.contentPat.push_back({0xeb, 0x00});           // jmp 2
        config.di.epiloguePat.clear();
        break;
    case SqrtNopIALU:
        config.di.delayPat.push_back({0xF2, 0x0F, 0x51, 0xC0}); // sqrtsd xmm0,xmm0
        config.di.prologuePat.clear();
        config.di.contentPat.push_back({0x90});                    // nop
        config.di.epiloguePat.push_back({0x48, 0x8D, 0x41, 0x04}); // lea rax, [rcx+4]
        break;
    case SqrtVFALUIALU:
        config.di.delayPat.push_back({0xF2, 0x0F, 0x51, 0xC0}); // sqrtsd xmm0,xmm0
        config.di.prologuePat.clear();
        config.di.contentPat.push_back({0xC5, 0xF1, 0xFE, 0xC1});  // VPADDD xmm0, xmm1, xmm1
        config.di.epiloguePat.push_back({0x48, 0x8D, 0x41, 0x04}); // lea rax, [rcx+4]
        break;
    case SchSqrtIALUDep:
        config.di.delayPat.push_back({0xF2, 0x0F, 0x51, 0xC0});          // sqrtsd xmm0,xmm0
        config.di.prologuePat.push_back({0xF2, 0x48, 0x0F, 0x2C, 0xC8}); // cvttsd2si rcx,xmm0
        config.di.contentPat.push_back({0x48, 0x8D, 0x41, 0x04});        // lea rax, [rcx+4]
        config.di.epiloguePat.clear();
        break;
    case SchSqrtIALUChainDep:
        config.di.delayPat.push_back({0xF2, 0x0F, 0x51, 0xC0});          // sqrtsd xmm0,xmm0
        config.di.prologuePat.push_back({0xF2, 0x48, 0x0F, 0x2C, 0xC8}); // cvttsd2si rcx,xmm0
        config.di.contentPat.push_back({0x48, 0x8D, 0x41, 0x04});        // lea rax, [rcx+4]
        config.di.epiloguePat.clear();
        break;
    case SchSDivFALUDep:
        config.args.iArg0 = 7;                                           // rcx = 7;
        config.di.delayPat.push_back({0x48, 0x31, 0xD2});                // xor rdx,rdx
        config.di.delayPat.push_back({0x48, 0xF7, 0xF9});                // idiv rcx
        config.di.prologuePat.push_back({0xF2, 0x48, 0x0F, 0x2A, 0xC8}); // cvtsi2sd xmm1,rax
        config.di.contentPat.push_back({0xC5, 0xF1, 0xFE, 0xC1});        // VPADDD xmm0, xmm1, xmm1
        config.di.epiloguePat.clear();
        break;
    case SchSDivFALUChainDep:
        config.args.iArg0 = 7;                                           // rcx = 7;
        config.di.delayPat.push_back({0x48, 0x31, 0xD2});                // xor rdx,rdx
        config.di.delayPat.push_back({0x48, 0xF7, 0xF9});                // idiv rcx
        config.di.prologuePat.push_back({0xF2, 0x48, 0x0F, 0x2A, 0xC8}); // cvtsi2sd xmm1,rax
        config.di.contentPat.push_back({0xC5, 0xF9, 0xFE, 0xC1});        // VPADDD xmm0, xmm0, xmm1
        config.di.epiloguePat.clear();
        break;
    case SchIALUNop:
        config.di.delayPat.push_back({0x48, 0x8D, 0x41, 0x04}); // lea rax, [rcx+4]
        config.di.prologuePat.clear();
        config.di.contentPat.push_back({0x90}); // nop
        config.di.epiloguePat.clear();
        break;
    case SchIALUChainNop:
        config.di.delayPat.push_back({0x48, 0x83, 0xC0, 0x04}); // add rax, 4
        config.di.prologuePat.clear();
        config.di.contentPat.push_back({0x90}); // nop
        config.di.epiloguePat.clear();
        break;
    case SchICmpNop:
        config.di.delayPat.push_back({0x48, 0x39, 0xC8}); // cmp rax, rcx
        config.di.prologuePat.clear();
        config.di.contentPat.push_back({0x90}); // nop
        config.di.epiloguePat.clear();
        break;
    case SchFALUNop:
        config.di.delayPat.push_back({0xC5, 0xF1, 0xFE, 0xC1}); // VPADDD xmm0, xmm1, xmm1
        config.di.prologuePat.clear();
        config.di.contentPat.push_back({0x90}); // nop
        config.di.epiloguePat.clear();
        break;
    case SchFALUChainNop:
        config.di.contentPat.push_back({0xC5, 0xF9, 0xFE, 0xC1}); // VPADDD xmm0, xmm0, xmm1
        config.di.prologuePat.clear();
        config.di.contentPat.push_back({0x90}); // nop
        config.di.epiloguePat.clear();
        break;
    case PeriodIALUNop:
        config.pi.periodPat.push_back({0x48, 0x83, 0xC0, 0x04}); // add rax, 4
        config.pi.fillPat.push_back({0x90});                     // nop
        break;
    case PeriodICmpNop:
        config.pi.periodPat.push_back({0x48, 0x39, 0xC8}); // cmp rax, rcx
        config.pi.fillPat.push_back({0x90});               // nop
        break;
    case PeriodFALUNop:
        config.pi.periodPat.push_back({0xC5, 0xF1, 0xFE, 0xC1}); // VPADDD xmm0, xmm1, xmm1
        config.pi.fillPat.push_back({0x90});                     // nop
        break;
    default:
        return false;
    }
    return true;
}

void fillnop(unsigned char *instBuf, unsigned sizeBytes) {
    genCodeStart();
    memset(instBuf, 0x90, sizeBytes);
    genCodeEnd(instBuf, sizeBytes);
}

#endif // defined(__amd64__) || defined(_M_AMD64)