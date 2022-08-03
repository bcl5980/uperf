#ifdef __APPLE__
#include <assert.h>
#include <dlfcn.h>
#include <pthread.h>
#include <stdio.h>

#include <sys/mman.h>
#include <sys/types.h>

#include <libkern/OSCacheControl.h>

typedef int kpc_force_all_ctrs_setproc(int);
static kpc_force_all_ctrs_setproc *kpc_force_all_ctrs_set;
typedef int kpc_set_countingproc(uint32_t);
static kpc_set_countingproc *kpc_set_counting;
typedef int kpc_set_thread_countingproc(uint32_t);
static kpc_set_thread_countingproc *kpc_set_thread_counting;
typedef int kpc_set_configproc(uint32_t, void *);
static kpc_set_configproc *kpc_set_config;
typedef int kpc_get_configproc(uint32_t, void *);
static kpc_get_configproc *kpc_get_config;
typedef uint32_t kpc_get_counter_countproc(uint32_t);
static kpc_get_counter_countproc *kpc_get_counter_count;
typedef uint32_t kpc_get_config_countproc(uint32_t);
static kpc_get_config_countproc *kpc_get_config_count;
typedef int kpc_get_thread_countersproc(int, unsigned int, void *);
static kpc_get_thread_countersproc *kpc_get_thread_counters;

#define CFGWORD_EL0A32EN_MASK (0x10000)
#define CFGWORD_EL0A64EN_MASK (0x20000)
#define CFGWORD_EL1EN_MASK (0x40000)
#define CFGWORD_EL3EN_MASK (0x80000)
#define CFGWORD_ALLMODES_MASK (0xf0000)

#define CPMU_NONE 0
#define CPMU_CORE_CYCLE 0x02
#define CPMU_INST_A64 0x8c
#define CPMU_INST_BRANCH 0x8d
#define CPMU_SYNC_DC_LOAD_MISS 0xbf
#define CPMU_SYNC_DC_STORE_MISS 0xc0
#define CPMU_SYNC_DTLB_MISS 0xc1
#define CPMU_SYNC_ST_HIT_YNGR_LD 0xc4
#define CPMU_SYNC_BR_ANY_MISP 0xcb
#define CPMU_FED_IC_MISS_DEM 0xd3
#define CPMU_FED_ITLB_MISS 0xd4

#define KPC_CLASS_FIXED (0)
#define KPC_CLASS_CONFIGURABLE (1)
#define KPC_CLASS_POWER (2)
#define KPC_CLASS_RAWPMU (3)
#define KPC_CLASS_FIXED_MASK (1u << KPC_CLASS_FIXED)
#define KPC_CLASS_CONFIGURABLE_MASK (1u << KPC_CLASS_CONFIGURABLE)
#define KPC_CLASS_POWER_MASK (1u << KPC_CLASS_POWER)
#define KPC_CLASS_RAWPMU_MASK (1u << KPC_CLASS_RAWPMU)

#define COUNTERS_COUNT 10
#define CONFIG_COUNT 8
#define KPC_MASK (KPC_CLASS_CONFIGURABLE_MASK | KPC_CLASS_FIXED_MASK)
static uint64_t g_counters[COUNTERS_COUNT];

unsigned long long getclock() {
    if (kpc_get_thread_counters(0, COUNTERS_COUNT, g_counters)) {
        printf("kpc_get_thread_counters failed\n");
        return 1;
    }
    return g_counters[2];
}

bool procInit(unsigned AffinityMask) {
    if (AffinityMask) {
        pthread_set_qos_class_self_np(QOS_CLASS_USER_INTERACTIVE, 0);
    } else {
        pthread_set_qos_class_self_np(QOS_CLASS_BACKGROUND, 0);
    }
    void *kperf = dlopen("/System/Library/PrivateFrameworks/kperf.framework/Versions/A/kperf", RTLD_LAZY);
    if (!kperf) {
        printf("kperf = %p\n", kperf);
        return false;
    }
    kpc_force_all_ctrs_set = (kpc_force_all_ctrs_setproc *)(dlsym(kperf, "kpc_force_all_ctrs_set"));
    if (!kpc_force_all_ctrs_set) {
        printf("%s = %p\n", "kpc_force_all_ctrs_set", (void *)kpc_force_all_ctrs_set);
        return false;
    }
    kpc_set_counting = (kpc_set_countingproc *)(dlsym(kperf, "kpc_set_counting"));
    if (!kpc_set_counting) {
        printf("%s = %p\n", "kpc_set_counting", (void *)kpc_set_counting);
        return false;
    }
    kpc_set_thread_counting = (kpc_set_thread_countingproc *)(dlsym(kperf, "kpc_set_thread_counting"));
    if (!kpc_set_thread_counting) {
        printf("%s = %p\n", "kpc_set_thread_counting", (void *)kpc_set_thread_counting);
        return false;
    }
    kpc_set_config = (kpc_set_configproc *)(dlsym(kperf, "kpc_set_config"));
    if (!kpc_set_config) {
        printf("%s = %p\n", "kpc_set_config", (void *)kpc_set_config);
        return false;
    }
    kpc_get_config = (kpc_get_configproc *)(dlsym(kperf, "kpc_get_config"));
    if (!kpc_get_config) {
        printf("%s = %p\n", "kpc_get_config", (void *)kpc_get_config);
        return false;
    }
    kpc_get_counter_count = (kpc_get_counter_countproc *)(dlsym(kperf, "kpc_get_counter_count"));
    if (!kpc_get_counter_count) {
        printf("%s = %p\n", "kpc_get_counter_count", (void *)kpc_get_counter_count);
        return false;
    }
    kpc_get_config_count = (kpc_get_config_countproc *)(dlsym(kperf, "kpc_get_config_count"));
    if (!kpc_get_config_count) {
        printf("%s = %p\n", "kpc_get_config_count", (void *)kpc_get_config_count);
        return false;
    }
    kpc_get_thread_counters = (kpc_get_thread_countersproc *)(dlsym(kperf, "kpc_get_thread_counters"));
    if (!kpc_get_thread_counters) {
        printf("%s = %p\n", "kpc_get_thread_counters", (void *)kpc_get_thread_counters);
        return false;
    }

    if (kpc_get_counter_count(KPC_MASK) != COUNTERS_COUNT) {
        printf("wrong fixed counters count\n");
        return false;
    }

    if (kpc_get_config_count(KPC_MASK) != CONFIG_COUNT) {
        printf("wrong fixed config count\n");
        return false;
    }

    uint64_t config[COUNTERS_COUNT];
    config[0] = CPMU_CORE_CYCLE | CFGWORD_EL0A64EN_MASK;

    if (kpc_set_config(KPC_MASK, config)) {
        printf("kpc_set_config failed\n");
        return false;
    }

    if (kpc_force_all_ctrs_set(1)) {
        printf("kpc_force_all_ctrs_set failed\n");
        return false;
    }

    if (kpc_set_counting(KPC_MASK)) {
        printf("kpc_set_counting failed\n");
        return false;
    }

    if (kpc_set_thread_counting(KPC_MASK)) {
        printf("kpc_set_thread_counting failed\n");
        return false;
    }
    return true;
}

void genCodeStart() { pthread_jit_write_protect_np(0); }

void genCodeEnd(void *Ptr, size_t Size) {
    pthread_jit_write_protect_np(1);
    sys_icache_invalidate(Ptr, Size);
}

unsigned char *allocVM(unsigned Size) {
    unsigned char *Ptr =
        (unsigned char *)mmap(0, Size, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_ANON | MAP_PRIVATE | MAP_JIT, -1, 0);
    assert(Ptr != MAP_FAILED);
    return Ptr;
}

void freeVM(void *Ptr, unsigned Size) { munmap(Ptr, Size); }

#endif