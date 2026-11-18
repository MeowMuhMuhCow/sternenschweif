#ifndef DSA_COMPAT_DIR_H
#define DSA_COMPAT_DIR_H

#include <io.h>
#include <direct.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef MAXPATH
#define MAXPATH 260
#endif

#ifndef FA_RDONLY
#define FA_NORMAL 0x00
#define FA_RDONLY 0x01
#define FA_HIDDEN 0x02
#define FA_SYSTEM 0x04
#define FA_LABEL  0x08
#define FA_DIREC  0x10
#define FA_ARCH   0x20
#endif

struct ffblk {
    char ff_reserved[21];
    char ff_attrib;
    unsigned short ff_ftime;
    unsigned short ff_fdate;
    long ff_fsize;
    char ff_name[13];
};

int dsa_findfirst(const char *pathname, struct ffblk *ffblk, int attrib);
int dsa_findnext(struct ffblk *ffblk);
int dsa_getdisk(void);
int dsa_setdisk(int drive);
int dsa_getcurdir(int drive, char *direc);

#define findfirst(p, f, a) dsa_findfirst(p, f, a)
#define findnext(f)        dsa_findnext(f)
#define getdisk()          dsa_getdisk()
#define setdisk(d)         dsa_setdisk(d)
#define getcurdir(d, p)    dsa_getcurdir(d, p)

#ifdef __cplusplus
}
#endif

#endif /* DSA_COMPAT_DIR_H */
