#ifndef DSA_COMPAT_PROCESS_H
#define DSA_COMPAT_PROCESS_H

#ifdef _WIN32
#include_next <process.h>
#else
#include <stdlib.h>
#include <unistd.h>
#endif

#endif /* DSA_COMPAT_PROCESS_H */
