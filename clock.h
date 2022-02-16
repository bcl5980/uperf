#pragma once

inline unsigned long long getclock()
{
	unsigned long long clock;
#if defined(__aarch64__)
	asm volatile("isb" : : : "memory");
	asm volatile("mrs %0, PMCCNTR_EL0" : "+r"(clock));
#else
	unsigned int ui;
	clock = __rdtscp(&ui);
#endif
	return clock;
}