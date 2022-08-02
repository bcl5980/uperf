#ifndef __OSUTILS_H__
#define __OSUTILS_H__

extern bool procInit(unsigned AffinityMask);

extern void genCodeStart(void);

extern void genCodeEnd(void *Ptr, size_t Size);

extern unsigned char *allocVM(unsigned Size);

extern void freeVM(void *Ptr, unsigned Size);

extern unsigned long long getclock(void);

#endif // __OSUTILS_H__