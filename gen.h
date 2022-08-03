#ifndef __GEN_H__
#define __GEN_H__

#include "testcase.h"

#if defined(__aarch64__) || defined(_M_ARM64)
#include "gen_aarch64.h"
#else
#include "gen_amd64.h"
#endif // defined(__aarch64__) || defined(_M_ARM64)

#endif // __GEN_AMD64_H__