#ifndef DSA_COMPAT_FCNTL_H
#define DSA_COMPAT_FCNTL_H

#include_next <fcntl.h>

#ifndef O_BINARY
#define O_BINARY 0
#endif
#ifndef _O_BINARY
#define _O_BINARY 0
#endif
#ifndef O_TEXT
#define O_TEXT 0
#endif
#ifndef _O_TEXT
#define _O_TEXT 0
#endif
#ifndef _O_RDONLY
#define _O_RDONLY O_RDONLY
#define _O_WRONLY O_WRONLY
#define _O_RDWR   O_RDWR
#define _O_CREAT  O_CREAT
#define _O_TRUNC  O_TRUNC
#define _O_APPEND O_APPEND
#endif

#endif /* DSA_COMPAT_FCNTL_H */
