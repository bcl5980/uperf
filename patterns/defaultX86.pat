Arch: X86  // must be the first one
WorkMode: Delay // Can be Delay/Period
Args: // Only Support 2 int, 2 ptr
    I0, 1
    I1, 1
    Ptr0, 0 // 0x
    Ptr1, 0 // 0x
Delay: bin // can be null/bin/asm
    0xF2, 0x0F, 0x51, 0xC0
Prologue: null
Content: bin // when use asm , llvm-mc should in the env path
    0x90 // nop
Epilogue: null
Period: null
PeriodFill: null