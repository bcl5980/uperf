#ifndef __ASSEMBLE_H__
#define __ASSEMBLE_H__
#include "uperf.h"

extern bool assemble(ArchType arch, const std::string& asmcode, InstBytes& bincode);

#endif // __ASSEMBLE_H__