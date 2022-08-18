#ifndef __PAT_CONFIG_H__
#define __PAT_CONFIG_H__
#include <vector>
#include <string>

typedef std::vector<unsigned char> InstBytes;

enum class ArchType { X86_64, AArch64 };

enum class WorkMode { DelayTest, PeriodTest };

struct Arguments {
    unsigned long long iArg0;
    unsigned long long iArg1;
    void *ptrArg0;
    void *ptrArg1;
};

struct DelayInfos {
    std::vector<InstBytes> delayPat;
    std::vector<InstBytes> prologuePat;
    std::vector<InstBytes> contentPat;
    std::vector<InstBytes> epiloguePat;
};

struct PeriodInfos {
    std::vector<InstBytes> periodPat;
    std::vector<InstBytes> fillPat;
};

struct PatConfig {
    ArchType arch;
    WorkMode mode;
    Arguments args;
    DelayInfos di;
    PeriodInfos pi;
};

extern bool parseConfig(PatConfig& config, const std::string& path);

#endif // __PAT_CONFIG_H__