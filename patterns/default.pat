Arch: AArch64 // must be the first one
WorkMode: Delay // Can be Delay/Period
Args: // Only Support 2 int, 2 ptr
    I0, 1    // x0
    I1, 1    // x1
    Ptr0, 0  // x2 0x
    Ptr1, 0  // x3 0x
Delay: asm // can be null/bin/asm
    fsqrt d0, d0
Prologue: null
Content: asm // when use asm , llvm-mc should in the env path
    nop
Epilogue: null
Period: null
PeriodFill: null