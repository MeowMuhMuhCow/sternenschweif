#ifndef DSA_COMPAT_IO_H
#define DSA_COMPAT_IO_H

#ifdef _WIN32
#include_next <io.h>
#else
#include <unistd.h>
#include <fcntl.h>
#define mkdir __posix_mkdir_2arg
#include <sys/stat.h>
#undef mkdir

#ifndef S_IREAD
#define S_IREAD  0400
#define S_IWRITE 0200
#endif

#ifndef O_BINARY
#define O_BINARY 0
#endif
#ifndef _O_BINARY
#define _O_BINARY 0
#define _O_RDONLY O_RDONLY
#define _O_WRONLY O_WRONLY
#define _O_RDWR   O_RDWR
#define _O_CREAT  O_CREAT
#define _O_TRUNC  O_TRUNC
#define _O_APPEND O_APPEND
#endif

#ifdef __cplusplus
extern "C" {
#endif

int  _open(const char *path, int oflag, ...);
int  _creat(const char *path, int attr);
int  _read(int fd, void *buf, unsigned int cnt);
int  _write(int fd, const void *buf, unsigned int cnt);
long _lseek(int fd, long offset, int origin);
int  _close(int fd);
long _filelength(int fd);
long filelength(int fd);
long _tell(int fd);
long tell(int fd);
int  _unlink(const char *path);
int  _chdir(const char *path);
int  _mkdir(const char *path);
int  mkdir(const char *path);

#ifdef __cplusplus
}
#endif

#endif /* !_WIN32 */

#endif /* DSA_COMPAT_IO_H */
