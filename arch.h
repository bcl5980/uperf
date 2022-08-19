#ifndef __GEN_H__
#define __GEN_H__

#include "uperf.h"

extern void fillNop(unsigned char *instBuf, unsigned sizeBytes);
extern bool genConfigForDefaultCases(TestCase caseId, PatConfig &config);
extern bool genPattern(PatConfig &config, unsigned char *instBuf, TestParam &param, unsigned testCnt);

#endif // __GEN_AMD64_H__