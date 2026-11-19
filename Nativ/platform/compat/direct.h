#ifndef DSA_COMPAT_DIRECT_H
#define DSA_COMPAT_DIRECT_H

#ifdef _WIN32
#include_next <direct.h>
#else
#include <unistd.h>
#define mkdir __posix_mkdir_2arg
#include <sys/stat.h>
#undef mkdir

#ifdef __cplusplus
extern "C" {
#endif

int _chdir(const char *path);
int _mkdir(const char *path);
int mkdir(const char *path);
#define _getcwd getcwd

#ifdef __cplusplus
}
#endif

#endif

#endif /* DSA_COMPAT_DIRECT_H */
