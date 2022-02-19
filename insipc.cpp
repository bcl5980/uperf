#include <intrin.h>
#include <math.h>
#include <stdio.h>
#include <windows.h>

#include "clock.h"

void cpuwarm() {
#pragma unroll
    for (int j = 0; j < 10000; j++)
        asm volatile("nop");
}

template <int Cnt> inline void add_test() {
    cpuwarm();
    unsigned long long start = getclock();
    const static int LoopCnt = 10000;
    unsigned ui;
    for (int i = 0; i < LoopCnt; i++) {
#pragma unroll
        for (int j = 0; j < Cnt; j++)
            asm volatile("add %0, %1" : "=r"(ui) : "r"(ui));
    }
    unsigned long long end = getclock();
    unsigned long long clock = end - start;
    printf("%f, %lld\n", (double)clock / (LoopCnt * Cnt), clock / LoopCnt);
}

template <int Cnt> inline void fadd_test() {
    float a = (rand() % 100) / 100.f;
    float c = (rand() % 100) / 100.f;
    cpuwarm();
    unsigned int ui;
    unsigned long long start = getclock();
    const static int LoopCnt = 10000;
    for (int i = 0; i < LoopCnt; i++) {
#pragma unroll
        for (int j = 0; j < Cnt; j++)
            c += a;
    }
    unsigned long long end = getclock();
    unsigned long long clock = end - start;
    printf("%f, %lld, %f\n", (double)clock / (LoopCnt * Cnt), clock / LoopCnt, c);
}

template <int Cnt> inline void fma_test() {
    float a = (rand() % 100) / 100.f;
    float b = (rand() % 100) / 100.f;
    float c = (rand() % 100) / 100.f;
    cpuwarm();
    unsigned int ui;
    unsigned long long start = getclock();
    const static int LoopCnt = 10000;
    for (int i = 0; i < LoopCnt; i++) {
#pragma unroll
        for (int j = 0; j < Cnt; j++)
            c += a * b;
    }
    unsigned long long end = getclock();
    unsigned long long clock = end - start;
    printf("%f, %lld, %f\n", (double)clock / (LoopCnt * Cnt), clock / LoopCnt, c);
}

template <int Cnt> inline void sqrtsd_test() {
    cpuwarm();
    unsigned int ui;
    unsigned long long start = getclock();
    const static int LoopCnt = 10000;
    for (int i = 0; i < LoopCnt; i++) {
#pragma unroll
        for (int j = 0; j < Cnt; j++)
            asm volatile("sqrtsd %xmm0, %xmm0");
    }
    unsigned long long end = getclock();
    unsigned long long clock = end - start;
    printf("%f, %lld\n", (double)clock / (LoopCnt * Cnt), clock / LoopCnt);
}

int main(int argc, char **argv) {
    SetProcessAffinityMask(GetCurrentProcess(), 0x1);
    SetProcessPriorityBoost(GetCurrentProcess(), true);
    SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS);
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);
    printf("add: ");
    add_test<5000>();
    printf("fadd: ");
    fadd_test<5000>();
    printf("fma: ");
    fma_test<5000>();
    printf("sqrtsd: ");
    sqrtsd_test<5000>();

    return 0;
}