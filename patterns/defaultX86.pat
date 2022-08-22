Arch: X86  // must be the first one
WorkMode: Delay // Can be Delay/Period
Args: // Only Support 2 int, 2 ptr
    I0, 1
    I1, 1
    Ptr0, 0x100 // 0x
    Ptr1, 0x100 // 0x
Delay: asm // can be null/bin/asm
    sqrtsd xmm0, xmm0
Prologue: asm
    cvttsd2si rcx, xmm0
Content: asm // when use asm , llvm-mc should in the env path
    mov rax, QWORD PTR [r8+rcx]
Epilogue: null
Period: null
PeriodFill: null 