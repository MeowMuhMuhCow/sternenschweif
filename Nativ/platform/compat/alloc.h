#ifndef DSA_COMPAT_ALLOC_H
#define DSA_COMPAT_ALLOC_H

#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

#undef _HEAPEMPTY
#undef _HEAPOK
#undef _HEAPCORRUPT
#undef _HEAPEND
#define _HEAPEMPTY   1
#define _HEAPOK      2
#define _HEAPCORRUPT 3
#define _HEAPEND     4

struct farheapinfo {
    void *ptr;
    unsigned long size;
    int in_use;
};

#define DSA_NOINST __attribute__((no_instrument_function))

static inline DSA_NOINST void *farmalloc(unsigned long size) {
    return calloc(1, (size_t)(size ? size : 1) + 256);
}

static inline DSA_NOINST void farfree(void *block) {
    if (block) free(block);
}

static inline DSA_NOINST void *farcalloc(unsigned long nunits, unsigned long unitsz) {
    size_t total = (size_t)(nunits ? nunits : 1) * (size_t)(unitsz ? unitsz : 1);
    return calloc(1, total + 256);
}

static inline DSA_NOINST void *farrealloc(void *block, unsigned long newsize) {
    return realloc(block, (size_t)(newsize ? newsize : 1) + 256);
}

static inline DSA_NOINST unsigned long farcoreleft(void) {
    return 16UL * 1024UL * 1024UL;
}

static inline DSA_NOINST unsigned long coreleft(void) {
    return 16UL * 1024UL * 1024UL;
}

static inline DSA_NOINST int farheapcheck(void) {
    return _HEAPOK;
}

static inline DSA_NOINST int farheapwalk(struct farheapinfo *hi) {
    (void)hi;
    return _HEAPEND;
}

#ifdef __cplusplus
}
#endif

#endif /* DSA_COMPAT_ALLOC_H */
