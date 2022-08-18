Arch: AArch64
WorkMode: Delay // Can be Delay/Period
Args: // Only Support 2 int, 2 ptr
    I0, 1
    I1, 1
    Ptr0, 0 // 0x
    Ptr1, 0 // 0x
Delay:
    0x00, 0xc0, 0x61, 0x1e // sqrt d0, d0
Prologue: null
Content:
    0x1f, 0x20, 0x03, 0xd5 // nop
Epilogue: null
Period: null
PeriodFill: null