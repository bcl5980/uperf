Arch: AArch64 // must be the first one
WorkMode: Delay // Can be Delay/Period
Args: // Only Support 2 int, 2 ptr
    I0, 1
    I1, 1
    Ptr0, 0 // 0x
    Ptr1, 0 // 0x
Delay: asm // can be null/bin/asm
    fsqrt d0, d0
    // 0x00, 0xc0, 0x61, 0x1e // fsqrt d0, d0
Prologue: null
Content: asm // when use asm , llvm-mc should in the env path
    nop
Epilogue: null
Period: null
PeriodFill: null