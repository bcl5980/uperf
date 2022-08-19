#ifndef __ARCHBASE_H__
#define __ARCHBASE_H__
#include "arch.h"
#include <vector>

extern void genDelayPattern(const std::vector<InstBytes> &insts, unsigned char *inst,
                            unsigned instCnt, size_t &i);

extern void genPeriodPattern(PatConfig &config, unsigned char *inst, unsigned period,
                             unsigned instNum, unsigned instThroughPut, unsigned nopThroughPut,
                             size_t &i);

#endif __ARCHBASE_H__