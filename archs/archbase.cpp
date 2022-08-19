#include "archbase.h"
#include "osutils.h"

void genDelayPattern(const std::vector<InstBytes> &insts, unsigned char *inst, unsigned instCnt,
                     size_t &i) {
    if (insts.empty())
        return;

    for (unsigned j = 0; j < instCnt; j++) {
        for (auto ii : insts) {
            memcpy(inst + i, &ii[0], ii.size());
            i += ii.size();
        }
    }
}

void genPeriodPattern(PatConfig &config, unsigned char *inst, unsigned period, unsigned instNum,
                      unsigned instThroughPut, unsigned nopThroughPut, size_t &i) {
    unsigned InstCnt = instThroughPut * period;
    unsigned PeriodCnt = nopThroughPut * period;
    const std::vector<InstBytes> &periodPat = config.pi.periodPat;
    const std::vector<InstBytes> &fillPat = config.pi.fillPat;
    for (unsigned j = 0, k = 0; j < instNum; j++) {
        if (k < InstCnt) {
            for (auto ii : periodPat) {
                memcpy(inst + i, &ii[0], ii.size());
                i += ii.size();
            }
        } else {
            for (auto ii : fillPat) {
                memcpy(inst + i, &ii[0], ii.size());
                i += ii.size();
            }
        }
        k++;
        if (k >= PeriodCnt)
            k = 0;
    }
}
