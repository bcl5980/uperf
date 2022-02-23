#pragma once

inline unsigned long long getclock() {
    unsigned long long clock;
#ifdef _M_ARM64
    __isb(_ARM64_BARRIER_SY);
    clock = _ReadStatusReg(ARM64_PMCCNTR_EL0);
#elif defined(__aarch64__)
    asm volatile("isb" : : : "memory");
    asm volatile("mrs %0, PMCCNTR_EL0" : "+r"(clock));
#else
    unsigned int ui;
    clock = __rdtscp(&ui);
#endif
    return clock;
}