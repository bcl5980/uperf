#include <stdio.h>
#include <stdlib.h>

#include "assemble.h"

using std::string;
using std::vector;

const static char AArch64LLVMMC[] = "llvm-mc -filetype=obj -triple=aarch64 -mattr=+sve %s -o %s";
const static char X86LLVMMC[] = "llvm-mc -filetype=obj -triple=x86 %s -o %s";

const static unsigned MaxInstNum = 0x100000;

bool assemble(ArchType arch, const string &asmcode, InstBytes &bincode) {
    char asmfilename[260];
    char objfilename[260];
    tmpnam(asmfilename);
    tmpnam(objfilename);
    FILE *asmfile = fopen(asmfilename, "w");
    if (asmfile == nullptr)
        return false;
    fputs(asmcode.c_str(), asmfile);
    fclose(asmfile);

    char cmd[256];
    sprintf(cmd, (arch == ArchType::X86_64) ? X86LLVMMC : AArch64LLVMMC, asmfilename, objfilename);
    system(cmd);
    remove(asmfilename);

    FILE *objfile = fopen(objfilename, "rb");
    if (objfile == nullptr)
        return false;

    const unsigned elf_header_szie = 64;
    char elf_header[elf_header_szie];
    size_t readsize;
    readsize = fread(elf_header, 1, elf_header_szie, objfile);
    if (readsize != elf_header_szie) {
        fclose(objfile);
        remove(objfilename);
        return false;
    }

    unsigned instsize = *(unsigned *)(elf_header + 40) - 0x94;
    if (instsize == 0 || instsize > MaxInstNum)
        return false;
    bincode.resize(instsize);
    readsize = fread(&bincode[0], 1, instsize, objfile);
    if (readsize != instsize) {
        fclose(objfile);
        remove(objfilename);
        return false;
    }

    fclose(objfile);
    remove(objfilename);
    return true;
}