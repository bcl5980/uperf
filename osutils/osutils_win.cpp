#ifdef WIN32
#include <intrin.h>
#include <windows.h>

void genCodeStart() { pthread_jit_write_protect_np(0); }

void genCodeEnd(void *Ptr, size_t Size) {
    __dmb(_ARM64_BARRIER_SY); // data memory barrier
    __isb(_ARM64_BARRIER_SY); // instruction barrier
}

bool procInit(unsigned AffinityMask) {
    SetProcessAffinityMask(GetCurrentProcess(), AffinityMask);
    SetProcessPriorityBoost(GetCurrentProcess(), true);
    SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS);
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);
    return true;
}

unsigned char *allocVM(unsigned Size) { return VirtualAlloc(0, Size, MEM_COMMIT, PAGE_EXECUTE_READWRITE); }

void freeVM(void *Ptr, unsigned Size) { VirtualFree(Ptr, Size, MEM_RELEASE); }

unsigned long long getclock() {
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
#endif