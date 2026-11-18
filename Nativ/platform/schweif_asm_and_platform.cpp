/*
 * schweif_asm_and_platform.cpp - C-Portierung der 16 Assembler-Module (SEG009..OVL270),
 * XMS/EMS-Flat-RAM-Emulation, ISO9660-CD-Image-Reader (gamea.gog/gameb.gog)
 * sowie DOS/BIOS/Maus/Timer-Anbindung an dsa_platform_sdl2 fuer DSA 2: Sternenschweif.
 *
 * Provenienz-Hinweis:
 * - ppDecrunch (SEG033): adaptiert aus BrightEyes-SDL2 seg009.cpp (GPL v3, Kerygma83 / Hynek).
 * - SDL2-Plattform-Anbindung: gemeinsame Plattformschicht der Nordlandtrilogie-Ports.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <dos.h>
#include <dir.h>
#include <bios.h>
#include <alloc.h>

#include "dsa_platform.h"
#pragma pack(push, 1)
#include "schweif.h"
#pragma pack(pop)

extern "C" {

/* =========================================================================
 * 1. Globale Variablen der 7 ASM/DMO-Module + FARDATA + Plattform-Helfer
 * ========================================================================= */

unsigned short _psp = 0x1000;
unsigned char *g_port_shade_base = NULL;
static uint8_t g_ems_frame_buf[0x10000];
static uint8_t g_dummy_indos = 0;

/* SEG024 (ds:78da..85e4): Kantenpuffer & aktuelle Textur der 3D-Engine */
signed short   g_edge_lo[208];
signed short   g_edge_hi[208];
signed short   g_edge_lo_z[208];
signed short   g_edge_hi_z[208];
signed short   g_edge_lo_u[208];
signed short   g_edge_hi_u[208];
signed short   g_edge_lo_v[208];
signed short   g_edge_hi_v[208];
struct tex_ref g_cur_tex = { 0, 0 };
unsigned short g_cur_tex_w = 0;

/* SEG026 (ds:8664..8668) */
void *g_old_int3f = NULL;

/* SEG027 (ds:8668..869a): Reisekarten-Zeilentabellen */
void *g_tmap_src = NULL;
unsigned short g_tmap_row = 0;
unsigned short g_tmap_row_h[11] = { 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20 };
unsigned short g_tmap_row_ofs[12] = {
    0x0000, 0x1680, 0x2d00, 0x4380, 0x5a00, 0x7080,
    0x8700, 0x9d80, 0xb400, 0xca80, 0xe100, 0xf780
};
char g_str_emmxxxx0[10] = "EMMXXXX0";

/* SEG036 (ds:869a..869e) */
void *g_ems_frame_ptr = g_ems_frame_buf;

/* SEG039 (ds:86a0..86a6) */
signed short g_xms_present = 1;
void *g_xms_entry = NULL;

/* SEG046 (ds:86a6..86b6) */
void *g_scale_src = NULL;
void *g_scale_dst = NULL;
unsigned char g_scale_op_inc_si = 0x46;
unsigned char g_scale_op_lodsb  = 0xac;
unsigned char g_scale_op_stosb  = 0xaa;
unsigned char g_scale_op_retf   = 0xcb;
void *g_scale_code = NULL;

/* FARDATA (OVL024, OVL036) */
unsigned short g_finale_ticks = 0;
unsigned long  g_fli_ticks = 0;

/* OVL030 eigene Daten (ds:187a..1966, 232 Bytes aus SCHWEIF.EXE) */
signed long g_187a[20] = {
    2368548, 0, 9, 31, 42, 115, 143, 113, 0, 164,
    165, 167, 169, 170, 172, 173, 174, 175, 176, 177
};
char g_18ca[20] = { 0x27,0x20,(char)0x88,0x21,0x05,0x22,0x16,0x20,0x17,0x20,0x11,0x20,0x7e,0x20,0x7f,0x20,0x47,0x40,(char)0xff,(char)0xff };
char g_18de[4]  = { 0,0,0,0 };
char g_18e2[22] = { 0x2c,0x20,(char)0x8f,0x21,(char)0xf4,0x21,0x1e,0x20,(char)0xaf,0x40,(char)0xb0,0x40,(char)0xb1,0x40,(char)0xb2,0x40,(char)0xb3,0x40,(char)0xb4,0x40,(char)0xff,(char)0xff };
char g_18f8[4]  = { 0,0,0,0 };
char g_18fc[8]  = { 0x2a,0x20,(char)0x89,0x21,0x1a,0x20,(char)0xff,(char)0xff };
char g_1904[2]  = { 0,0 };
char g_1906[18] = { (char)0x82,0x20,(char)0x83,0x20,(char)0x84,0x20,(char)0x85,0x20,(char)0x86,0x20,(char)0x87,0x20,(char)0xf3,0x21,0x7d,0x20,(char)0xff,(char)0xff };
char g_1918[4]  = { 0,0,0,0 };
char g_191c[10] = { 0x2b,0x20,(char)0x8a,0x21,0x02,0x22,0x1d,0x20,(char)0xff,(char)0xff };
char g_1926[2]  = { 0,0 };
char g_1928[10] = { 0x29,0x20,0x28,0x20,(char)0xff,0x21,0x19,0x20,(char)0xff,(char)0xff };
char g_1932[2]  = { 0,0 };
char g_1934[10] = { 0x2d,0x20,0x2e,0x20,0x2f,0x20,0x22,0x20,(char)0xff,(char)0xff };
char g_193e[2]  = { 0,0 };
char g_1940[8]  = { (char)0x91,0x21,(char)0xfe,0x21,(char)0x95,0x21,(char)0xff,(char)0xff };
char g_1948[2]  = { 0,0 };
char g_194a[18] = { (char)0xae,0x20,(char)0xaf,0x20,(char)0xb0,0x20,(char)0xb1,0x20,(char)0xb2,0x20,(char)0xa1,0x20,(char)0xa2,0x20,0x6d,0x40,(char)0xff,(char)0xff };
char g_195c[2]  = { 0,0 };
char g_195e[6]  = { 0x20,0x22,0x21,0x22,(char)0xff,(char)0xff };
char g_1964[2]  = { 0,0 };

/* =========================================================================
 * 2. ISO9660 Mode2/2352 Virtuelles Dateisystem fuer gamea.gog / gameb.gog
 * ========================================================================= */

struct iso_file_entry {
    char     path[64];   /* z. B. "DATA/STAR.DAT" */
    int      disc;       /* 0 = gamea.gog, 1 = gameb.gog */
    uint32_t lba;
    uint32_t size;
};

#define MAX_ISO_FILES 128
static struct iso_file_entry g_iso_files[MAX_ISO_FILES];
static int g_num_iso_files = 0;
static FILE *g_gog_fp[2] = { NULL, NULL };

struct vfd_slot {
    int      in_use;
    int      disc;
    uint32_t start_lba;
    uint32_t size;
    uint32_t pos;
};

#define VFD_BASE 1000
#define MAX_VFDS 32
static struct vfd_slot g_vfds[MAX_VFDS];

static void iso_read_bytes(FILE *fp, uint32_t lba, uint32_t offset, void *buf, uint32_t len) {
    uint8_t *dst = (uint8_t *)buf;
    uint32_t cur_pos = offset;
    while (len > 0) {
        uint32_t sec = lba + (cur_pos >> 11);
        uint32_t sec_off = cur_pos & 0x7FFu;
        uint32_t chunk = 2048u - sec_off;
        if (chunk > len) chunk = len;
        fseek(fp, (long)(sec * 2352u + 24u + sec_off), SEEK_SET);
        size_t got = fread(dst, 1, chunk, fp);
        if (got == 0) break;
        dst += got;
        cur_pos += (uint32_t)got;
        len -= (uint32_t)got;
    }
}

static void iso_scan_dir(FILE *fp, int disc, uint32_t dir_lba, uint32_t dir_len, const char *prefix) {
    uint8_t *data = (uint8_t *)malloc(dir_len);
    if (!data) return;
    iso_read_bytes(fp, dir_lba, 0, data, dir_len);
    uint32_t pos = 0;
    while (pos < dir_len) {
        uint8_t reclen = data[pos];
        if (reclen == 0) {
            pos = ((pos >> 11) + 1) << 11;
            continue;
        }
        uint32_t flba = *(uint32_t *)(data + pos + 2);
        uint32_t flen = *(uint32_t *)(data + pos + 10);
        uint8_t flags = data[pos + 25];
        uint8_t nlen  = data[pos + 32];
        char name[64];
        if (nlen >= sizeof(name)) nlen = sizeof(name) - 1;
        memcpy(name, data + pos + 33, nlen);
        name[nlen] = '\0';
        char *semi = strchr(name, ';');
        if (semi) *semi = '\0';

        if (name[0] != '\0' && name[0] != '\x01') {
            char full[64];
            if (prefix[0])
                snprintf(full, sizeof(full), "%s/%s", prefix, name);
            else
                snprintf(full, sizeof(full), "%s", name);

            if (flags & 2) {
                iso_scan_dir(fp, disc, flba, flen, full);
            } else if (g_num_iso_files < MAX_ISO_FILES) {
                strncpy(g_iso_files[g_num_iso_files].path, full, sizeof(g_iso_files[0].path) - 1);
                g_iso_files[g_num_iso_files].disc = disc;
                g_iso_files[g_num_iso_files].lba  = flba;
                g_iso_files[g_num_iso_files].size = flen;
                g_num_iso_files++;
            }
        }
        pos += reclen;
    }
    free(data);
}

static void iso_init_images(const char *gamedir) {
    const char *names[2] = { "gamea.gog", "gameb.gog" };
    for (int d = 0; d < 2; ++d) {
        char path[1024];
        snprintf(path, sizeof(path), "%s\\%s", gamedir, names[d]);
        FILE *fp = fopen(path, "rb");
        if (!fp) {
            snprintf(path, sizeof(path), "%s/%s", gamedir, names[d]);
            fp = fopen(path, "rb");
        }
        if (!fp) continue;
        uint8_t pvd[2048];
        fseek(fp, 16L * 2352L + 24L, SEEK_SET);
        if (fread(pvd, 1, 2048, fp) == 2048 && memcmp(pvd + 1, "CD001", 5) == 0) {
            g_gog_fp[d] = fp;
            uint32_t root_lba = *(uint32_t *)(pvd + 156 + 2);
            uint32_t root_len = *(uint32_t *)(pvd + 156 + 10);
            iso_scan_dir(fp, d, root_lba, root_len, "");
        } else {
            fclose(fp);
        }
    }
}

static int iso_open_virtual(const char *dos_path) {
    if (!dos_path) return -1;
    const char *p = dos_path;
    if (isalpha((unsigned char)p[0]) && p[1] == ':') {
        char drv = (char)toupper((unsigned char)p[0]);
        if (drv == 'C') return -1;
        p += 2;
        while (*p == '\\' || *p == '/') p++;
    } else {
        return -1;
    }

    char norm[64];
    size_t j = 0;
    for (size_t i = 0; p[i] && j + 1 < sizeof(norm); ++i) {
        char c = (char)toupper((unsigned char)p[i]);
        if (c == '\\') c = '/';
        norm[j++] = c;
    }
    norm[j] = '\0';

    for (int i = 0; i < g_num_iso_files; ++i) {
        if (strcmp(g_iso_files[i].path, norm) == 0 && g_gog_fp[g_iso_files[i].disc]) {
            for (int slot = 0; slot < MAX_VFDS; ++slot) {
                if (!g_vfds[slot].in_use) {
                    g_vfds[slot].in_use    = 1;
                    g_vfds[slot].disc      = g_iso_files[i].disc;
                    g_vfds[slot].start_lba = g_iso_files[i].lba;
                    g_vfds[slot].size      = g_iso_files[i].size;
                    g_vfds[slot].pos       = 0;
                    return VFD_BASE + slot;
                }
            }
        }
    }
    return -1;
}

extern "C" void * __stdcall GetModuleHandleA(const char *lpModuleName);
extern "C" void * __stdcall GetProcAddress(void *hModule, const char *lpProcName);

typedef int  (*pfn_open_t)(const char *, int, ...);
typedef int  (*pfn_read_t)(int, void *, unsigned int);
typedef int  (*pfn_write_t)(int, const void *, unsigned int);
typedef long (*pfn_lseek_t)(int, long, int);
typedef int  (*pfn_close_t)(int);
typedef long (*pfn_filelength_t)(int);
typedef long (*pfn_tell_t)(int);
typedef int  (*pfn_unlink_t)(const char *);
typedef int  (*pfn_chdir_t)(const char *);
typedef int  (*pfn_mkdir_t)(const char *);

static pfn_open_t       crt_open_fn       = NULL;
static pfn_read_t       crt_read_fn       = NULL;
static pfn_write_t      crt_write_fn      = NULL;
static pfn_lseek_t      crt_lseek_fn      = NULL;
static pfn_close_t      crt_close_fn      = NULL;
static pfn_filelength_t crt_filelength_fn = NULL;
static pfn_tell_t       crt_tell_fn       = NULL;
static pfn_unlink_t     crt_unlink_fn     = NULL;
static pfn_chdir_t      crt_chdir_fn      = NULL;
static pfn_mkdir_t      crt_mkdir_fn      = NULL;

static void init_crt_io(void) {
    if (crt_open_fn) return;
    void *m = GetModuleHandleA("msvcrt.dll");
    crt_open_fn       = (pfn_open_t)GetProcAddress(m, "_open");
    crt_read_fn       = (pfn_read_t)GetProcAddress(m, "_read");
    crt_write_fn      = (pfn_write_t)GetProcAddress(m, "_write");
    crt_lseek_fn      = (pfn_lseek_t)GetProcAddress(m, "_lseek");
    crt_close_fn      = (pfn_close_t)GetProcAddress(m, "_close");
    crt_filelength_fn = (pfn_filelength_t)GetProcAddress(m, "_filelength");
    crt_tell_fn       = (pfn_tell_t)GetProcAddress(m, "_tell");
    crt_unlink_fn     = (pfn_unlink_t)GetProcAddress(m, "_unlink");
    crt_chdir_fn      = (pfn_chdir_t)GetProcAddress(m, "_chdir");
    crt_mkdir_fn      = (pfn_mkdir_t)GetProcAddress(m, "_mkdir");
}

static char g_save_dir[1024] = "";
static int  g_save_dir_redirected = 0;
static int  g_save_dir_notice_pending = 0;
static int  g_no_intro = 0;
static int  g_force_intro = 0;
static int  g_has_replay = 0;

static int is_redirected_save_rel(const char *rel) {
    if (!rel || !rel[0]) return 0;
    if ((_strnicmp(rel, "GAMES", 5) == 0) && (rel[5] == '\0' || rel[5] == '\\' || rel[5] == '/')) return 1;
    if ((_strnicmp(rel, "TEMP", 4) == 0)  && (rel[4] == '\0' || rel[4] == '\\' || rel[4] == '/')) return 1;
    if ((_strnicmp(rel, "CTEMP", 5) == 0) && (rel[5] == '\0' || rel[5] == '\\' || rel[5] == '/')) return 1;
    if (_stricmp(rel, "PROBEN.CSV") == 0) return 1;
    return 0;
}

static void resolve_dos_c_root(const char *path, char *out, size_t out_sz) {
    if (!path) { out[0] = '\0'; return; }
    const char *gdir = dsa_platform_get_gamedir();
    size_t glen = gdir ? strlen(gdir) : 0;
    if (glen > 0 && isalpha((unsigned char)path[0]) && path[1] == ':' &&
        (path[2] == '\\' || path[2] == '/') && _strnicmp(path, gdir, glen) != 0) {
        const char *rel = path + 3;
        while (*rel == '\\' || *rel == '/') rel++;
        if (*rel) snprintf(out, out_sz, "%s\\%s", gdir, rel);
        else      snprintf(out, out_sz, "%s", gdir);
    } else {
        snprintf(out, out_sz, "%s", path);
    }

    if (g_save_dir_redirected && g_save_dir[0]) {
        const char *rel = out;
        if (glen > 0 && _strnicmp(out, gdir, glen) == 0 && (out[glen] == '\\' || out[glen] == '/')) {
            rel = out + glen + 1;
        }
        while (*rel == '\\' || *rel == '/') rel++;
        if (_strnicmp(rel, "DATA\\..\\", 8) == 0 || _strnicmp(rel, "DATA/../", 8) == 0) {
            rel += 8;
        } else if (strncmp(rel, "..\\", 3) == 0 || strncmp(rel, "../", 3) == 0) {
            rel += 3;
        }
        while (*rel == '\\' || *rel == '/') rel++;
        if (is_redirected_save_rel(rel)) {
            char redir[1024];
            snprintf(redir, sizeof(redir), "%s\\%s", g_save_dir, rel);
            snprintf(out, out_sz, "%s", redir);
        }
    }

    /* MS-DOS 8.3-Dateinamen ignorieren Leerzeichen vor dem Punkt (z. B. "START   .GAM" -> "START.GAM") */
    char *slash = strrchr(out, '\\');
    char *fslash = strrchr(out, '/');
    char *base = slash ? (fslash && fslash > slash ? fslash + 1 : slash + 1) : (fslash ? fslash + 1 : out);
    char *dot = strrchr(base, '.');
    if (dot && dot > base && dot[-1] == ' ') {
        char *p = dot;
        while (p > base && p[-1] == ' ') p--;
        memmove(p, dot, strlen(dot) + 1);
    }
}

static int open_with_gog_dirs(const char *path, const char *resolved, int oflag) {
    int fd = crt_open_fn(resolved, oflag, 0666);
    if (fd >= 0 || !path) return fd;
    if (strchr(path, '\\') != NULL || strchr(path, '/') != NULL) return fd;
    const char *gdir = dsa_platform_get_gamedir();
    if (!gdir || !gdir[0]) return fd;
    char alt[1024];
    snprintf(alt, sizeof(alt), "%s\\DATA\\%s", gdir, path);
    fd = crt_open_fn(alt, oflag, 0666);
    if (fd < 0) {
        snprintf(alt, sizeof(alt), "%s\\DRIVERS\\%s", gdir, path);
        fd = crt_open_fn(alt, oflag, 0666);
    }
    return fd;
}

int __wrap__open(const char *path, int oflag, int pmode) {
    (void)pmode;
    init_crt_io();
    int vfd = iso_open_virtual(path);
    if (vfd >= 0) return vfd;
    char resolved[1024];
    resolve_dos_c_root(path, resolved, sizeof(resolved));
    oflag &= 0xFFFF;
    if (oflag == 0x8000 || oflag == 0) {
        return open_with_gog_dirs(path, resolved, _O_RDONLY | _O_BINARY);
    }
    if (oflag == 0x8001 || oflag == 1) {
        int fd = crt_open_fn(resolved, _O_RDWR | _O_BINARY, 0666);
        if (fd < 0) fd = crt_open_fn(resolved, _O_RDONLY | _O_BINARY, 0666);
        return fd;
    }
    if (oflag == 0x8002 || oflag == 0x8004 || oflag == 2 || oflag == 4) {
        return crt_open_fn(resolved, _O_RDWR | _O_BINARY, 0666);
    }
    if (oflag == 0x8101 || oflag == 0x8102 || oflag == 0x8104) {
        return crt_open_fn(resolved, _O_RDWR | _O_CREAT | _O_BINARY, 0666);
    }
    if (oflag == 0x8300 || oflag == 0x8301 || oflag == 0x8302 || oflag == 0x8304) {
        return crt_open_fn(resolved, _O_RDWR | _O_CREAT | _O_TRUNC | _O_BINARY, 0666);
    }
    if (oflag & 4) {
        oflag = (oflag & ~4) | _O_RDWR;
    }
    oflag |= _O_BINARY;
    return open_with_gog_dirs(path, resolved, oflag);
}

int __wrap_open(const char *path, int oflag, int pmode) {
    return __wrap__open(path, oflag, pmode);
}

int __wrap__creat(const char *path, int attr) {
    (void)attr;
    init_crt_io();
    char resolved[1024];
    resolve_dos_c_root(path, resolved, sizeof(resolved));
    return crt_open_fn(resolved, _O_BINARY | _O_CREAT | _O_TRUNC | _O_RDWR, 0666);
}

int __wrap_creat(const char *path, int attr) {
    return __wrap__creat(path, attr);
}

int __wrap__read(int fd, void *buf, unsigned int cnt) {
    init_crt_io();
    int idx = (fd & 0x3FFF) - VFD_BASE;
    if (idx >= 0 && idx < MAX_VFDS && g_vfds[idx].in_use) {
        struct vfd_slot *s = &g_vfds[idx];
        if (s->pos >= s->size) return 0;
        uint32_t rem = s->size - s->pos;
        uint32_t to_read = cnt < rem ? cnt : rem;
        iso_read_bytes(g_gog_fp[s->disc], s->start_lba, s->pos, buf, to_read);
        s->pos += to_read;
        return (int)to_read;
    }
    return crt_read_fn(fd, buf, cnt);
}

int __wrap_read(int fd, void *buf, unsigned int cnt) {
    return __wrap__read(fd, buf, cnt);
}

int __wrap__write(int fd, const void *buf, unsigned int cnt) {
    init_crt_io();
    return crt_write_fn(fd, buf, cnt);
}

int __wrap_write(int fd, const void *buf, unsigned int cnt) {
    return __wrap__write(fd, buf, cnt);
}

long __wrap__lseek(int fd, long offset, int origin) {
    init_crt_io();
    int idx = (fd & 0x3FFF) - VFD_BASE;
    if (idx >= 0 && idx < MAX_VFDS && g_vfds[idx].in_use) {
        struct vfd_slot *s = &g_vfds[idx];
        int64_t np = s->pos;
        if (origin == SEEK_SET) np = offset;
        else if (origin == SEEK_CUR) np += offset;
        else if (origin == SEEK_END) np = (int64_t)s->size + offset;
        if (np < 0) np = 0;
        if (np > (int64_t)s->size) np = s->size;
        s->pos = (uint32_t)np;
        return (long)s->pos;
    }
    return crt_lseek_fn(fd, offset, origin);
}

long __wrap_lseek(int fd, long offset, int origin) {
    return __wrap__lseek(fd, offset, origin);
}

int __wrap__close(int fd) {
    init_crt_io();
    int idx = (fd & 0x3FFF) - VFD_BASE;
    if (idx >= 0 && idx < MAX_VFDS && g_vfds[idx].in_use) {
        g_vfds[idx].in_use = 0;
        return 0;
    }
    return crt_close_fn(fd);
}

int __wrap_close(int fd) {
    return __wrap__close(fd);
}

long __wrap__filelength(int fd) {
    init_crt_io();
    int idx = (fd & 0x3FFF) - VFD_BASE;
    if (idx >= 0 && idx < MAX_VFDS && g_vfds[idx].in_use) {
        return (long)g_vfds[idx].size;
    }
    return crt_filelength_fn(fd);
}

long __wrap_filelength(int fd) {
    return __wrap__filelength(fd);
}

long __wrap__tell(int fd) {
    init_crt_io();
    int idx = (fd & 0x3FFF) - VFD_BASE;
    if (idx >= 0 && idx < MAX_VFDS && g_vfds[idx].in_use) {
        return (long)g_vfds[idx].pos;
    }
    return crt_tell_fn(fd);
}

long __wrap_tell(int fd) {
    return __wrap__tell(fd);
}

int __wrap__unlink(const char *path) {
    init_crt_io();
    char resolved[1024];
    resolve_dos_c_root(path, resolved, sizeof(resolved));
    return crt_unlink_fn(resolved);
}

int __wrap_unlink(const char *path) {
    return __wrap__unlink(path);
}

void *port_imp__open   __asm__("__imp___open")   = (void *)&__wrap__open;
void *port_imp_open    __asm__("__imp__open")    = (void *)&__wrap_open;
void *port_imp__creat  __asm__("__imp___creat")  = (void *)&__wrap__creat;
void *port_imp_creat   __asm__("__imp__creat")   = (void *)&__wrap_creat;
void *port_imp__read   __asm__("__imp___read")   = (void *)&__wrap__read;
void *port_imp_read    __asm__("__imp__read")    = (void *)&__wrap_read;
void *port_imp__write  __asm__("__imp___write")  = (void *)&__wrap__write;
void *port_imp_write   __asm__("__imp__write")   = (void *)&__wrap_write;
void *port_imp__lseek  __asm__("__imp___lseek")  = (void *)&__wrap__lseek;
void *port_imp_lseek   __asm__("__imp__lseek")   = (void *)&__wrap_lseek;
void *port_imp__close  __asm__("__imp___close")  = (void *)&__wrap__close;
void *port_imp_close   __asm__("__imp__close")   = (void *)&__wrap_close;
void *port_imp__unlink __asm__("__imp___unlink") = (void *)&__wrap__unlink;
void *port_imp_unlink  __asm__("__imp__unlink")  = (void *)&__wrap_unlink;

/* =========================================================================
 * 3. DOS/BIOS/Maus/Timer-Helfer
 * ========================================================================= */

void *dsa_mk_fp(unsigned short seg, unsigned long off) {
    if (seg == 0xa000) {
        return (void *)(dsa_platform_get_vram() + off);
    }
    if (seg == 0x8000) {
        return (void *)(g_ems_frame_buf + (off & 0xFFFFu));
    }
    if (seg >= 0x9000 && seg < 0x9020 && g_port_shade_base) {
        return (void *)(g_port_shade_base + ((uint32_t)(seg - 0x9000) << 8) + off);
    }
    return (void *)(((uintptr_t)seg << 16) + (uintptr_t)off);
}

int dsa_bioskey(int cmd) {
    if (cmd == 0) return dsa_platform_get_key();
    if (cmd == 1) return dsa_platform_check_key();
    if (cmd == 2) return dsa_platform_get_mods();
    return 0;
}

void dsa_delay(unsigned ms) {
    dsa_platform_delay_ms(ms);
}

int dsa_int86x(int intno, union REGS *inregs, union REGS *outregs, struct SREGS *segregs) {
    (void)segregs;
    if (!inregs || !outregs) return 0;
    *outregs = *inregs;
    if (intno == 0x33) {
        switch (inregs->x.ax) {
            case 0x0000:
                outregs->x.ax = 0xFFFF;
                outregs->x.bx = 2;
                break;
            case 0x0003: {
                int mx = 0, my = 0, mb = 0;
                dsa_platform_get_mouse(&mx, &my, &mb);
                outregs->x.bx = (unsigned short)mb;
                outregs->x.cx = (unsigned short)mx;
                outregs->x.dx = (unsigned short)my;
                break;
            }
            case 0x0004:
                dsa_platform_set_mouse_pos(inregs->x.cx, inregs->x.dx);
                break;
            default:
                break;
        }
    }
    return outregs->x.ax;
}

int dsa_int86(int intno, union REGS *inregs, union REGS *outregs) {
    struct SREGS sregs = { 0, 0, 0, 0 };
    return dsa_int86x(intno, inregs, outregs, &sregs);
}

int dsa_intdos(union REGS *inregs, union REGS *outregs) {
    if (!inregs || !outregs) return 0;
    *outregs = *inregs;
    if (inregs->h.ah == 0x19) {
        outregs->h.al = 2; /* C: */
    } else if (inregs->h.ah == 0x0E) {
        outregs->h.al = 26;
    }
    return outregs->x.ax;
}

void dsa_getdfree(unsigned char drive, struct dfree *dtable) {
    (void)drive;
    if (dtable) {
        dtable->df_avail = 32000;
        dtable->df_total = 64000;
        dtable->df_bsec  = 512;
        dtable->df_sclus = 16;
    }
}

static void fill_ffblk_from_finddata(struct ffblk *ff, const struct _finddata_t *fd) {
    ff->ff_attrib = (char)fd->attrib;
    ff->ff_fsize  = (long)fd->size;
    ff->ff_ftime  = (unsigned short)(fd->time_write & 0xFFFFu);
    ff->ff_fdate  = (unsigned short)((fd->time_write >> 16) & 0xFFFFu);
    strncpy(ff->ff_name, fd->name, sizeof(ff->ff_name) - 1);
    ff->ff_name[sizeof(ff->ff_name) - 1] = '\0';
}

static int ffblk_match_entry(const struct _finddata_t *fd, int attrib) {
    if (strcmp(fd->name, ".") == 0 || strcmp(fd->name, "..") == 0)
        return 0;
    if ((fd->attrib & 0x10) && !(attrib & FA_DIREC))
        return 0;
    return 1;
}

int dsa_findfirst(const char *pathname, struct ffblk *ff, int attrib) {
    if (!pathname || !ff) return -1;
    char resolved[1024];
    resolve_dos_c_root(pathname, resolved, sizeof(resolved));
    struct _finddata_t fd;
    intptr_t h = _findfirst(resolved, &fd);
    if (h == -1) {
        *(intptr_t *)ff->ff_reserved = -1;
        return -1;
    }
    *(intptr_t *)ff->ff_reserved = h;
    *(int *)(ff->ff_reserved + 8) = attrib;
    while (!ffblk_match_entry(&fd, attrib)) {
        if (_findnext(h, &fd) != 0) {
            _findclose(h);
            *(intptr_t *)ff->ff_reserved = -1;
            return -1;
        }
    }
    fill_ffblk_from_finddata(ff, &fd);
    return 0;
}

int dsa_findnext(struct ffblk *ff) {
    if (!ff) return -1;
    intptr_t h = *(intptr_t *)ff->ff_reserved;
    int attrib = *(int *)(ff->ff_reserved + 8);
    if (h == -1) return -1;
    struct _finddata_t fd;
    while (_findnext(h, &fd) == 0) {
        if (ffblk_match_entry(&fd, attrib)) {
            fill_ffblk_from_finddata(ff, &fd);
            return 0;
        }
    }
    _findclose(h);
    *(intptr_t *)ff->ff_reserved = -1;
    return -1;
}

static int g_cur_disk = 2; /* 2 = C: */

int dsa_getdisk(void) { return g_cur_disk; }
int dsa_setdisk(int drive) { g_cur_disk = drive; return 26; }

int dsa_getcurdir(int drive, char *direc) {
    (void)drive;
    if (!direc) return -1;
    char buf[512];
    if (_getcwd(buf, sizeof(buf))) {
        const char *gdir = dsa_platform_get_gamedir();
        size_t glen = gdir ? strlen(gdir) : 0;
        if (glen > 0 && _strnicmp(buf, gdir, glen) == 0) {
            const char *p = buf + glen;
            if (*p == '\\' || *p == '/') p++;
            strncpy(direc, p, 32);
            direc[32] = '\0';
            return 0;
        }
    }
    direc[0] = '\0';
    return 0;
}

int __wrap__chdir(const char *path) {
    init_crt_io();
    char resolved[1024];
    resolve_dos_c_root(path, resolved, sizeof(resolved));
    return crt_chdir_fn(resolved);
}

int __wrap_chdir(const char *path) {
    return __wrap__chdir(path);
}

int __wrap__mkdir(const char *path) {
    init_crt_io();
    char resolved[1024];
    resolve_dos_c_root(path, resolved, sizeof(resolved));
    return crt_mkdir_fn(resolved);
}

int __wrap_mkdir(const char *path) {
    return __wrap__mkdir(path);
}

static uint32_t g_last_tick_18hz = 0;
static uint32_t g_last_tick_100hz = 0;

extern "C" void timer_isr(void);
extern "C" signed short g_random_seed2;
static void dsa_cov_group_run(int grp);

static void schweif_platform_tick(void) {
    {
        static int s_cov_grp_done = 0;
        if (!s_cov_grp_done && dsa_platform_get_frame_count() >= 160 && g_group && g_heroes) {
            const char *cg = getenv("DSA_COV_GROUP");
            if (cg && cg[0] >= '1' && cg[0] <= '9') {
                s_cov_grp_done = 1;
                dsa_cov_group_run(atoi(cg));
            }
        }
    }
    /* 1. Pfeiltasten fuer 3D-Bewegung */
    g_key_flag = (signed short)dsa_platform_get_arrow_mask();

    /* 2. Mausposition und Klicks (wie mouse_isr in SEG023.C) */
    int mx = g_mouse_posx, my = g_mouse_posy, mb = 0;
    dsa_platform_get_mouse(&mx, &my, &mb);
    int lc = 0, rc = 0;
    dsa_platform_consume_clicks(&lc, &rc);

    if (g_mouse_max_x > 0) {
        if (mx < g_mouse_min_x) mx = g_mouse_min_x;
        if (mx > g_mouse_max_x) mx = g_mouse_max_x;
    }
    if (g_mouse_max_y > 0) {
        if (my < g_mouse_min_y) my = g_mouse_min_y;
        if (my > g_mouse_max_y) my = g_mouse_max_y;
    }

    if (mx != g_mouse_posx || my != g_mouse_posy) {
        g_mouse_posx = (signed short)mx;
        g_mouse_posy = (signed short)my;
        g_mouse_moved = 1;
        g_idle_ticks = 0;
        g_saver_due = 0;
    }

    g_mouse_ldown = (mb & 1) ? 1 : 0;
    g_mouse_rdown = (mb & 2) ? 1 : 0;

    if (lc > 0) {
        g_mouse_ldown  = 1;
        g_mouse_clickx = (signed short)mx;
        g_mouse_clicky = (signed short)my;
        if (g_mouse_lpending == 0) {
            g_mouse_lpending = 1;
            g_mouse_lclick   = 1;
            g_mouse_event    = 1;
            g_mouse_lclick_time = g_random_seed2;
        } else {
            g_mouse_ldclick  = 1;
            g_mouse_lpending = 0;
            g_mouse_lclick   = 0;
            g_mouse_event    = 1;
        }
        g_idle_ticks = 0;
        g_saver_due = 0;
    }
    if (rc > 0) {
        g_mouse_rdown  = 1;
        g_mouse_rclick = 1;
        g_mouse_clickx = (signed short)mx;
        g_mouse_clicky = (signed short)my;
        g_idle_ticks = 0;
        g_saver_due = 0;
    }

    if (g_mouse_moved && !g_mouse_locked && g_vga_buf) {
        mouse_update();
    }

    /* 3. 18,2-Hz-Timer (timer_isr + FLI/Finale-Zaehler) */
    uint32_t now18 = dsa_platform_get_ticks_18hz();
    if (g_last_tick_18hz == 0) g_last_tick_18hz = now18;
    while (g_last_tick_18hz < now18) {
        g_last_tick_18hz++;
        g_finale_ticks++;
        g_fli_ticks += 4;
        if (g_mouse_lpending != 0 && g_random_seed2 - g_mouse_lclick_time >= 4) {
            g_mouse_lpending = 0;
        }
        timer_isr();
    }

    /* 4. 100-Hz-Timer fuer 3D-Ansicht (view_timer_cb) & 50-Hz-Timer fuer Kampfanimationen (g_anim_ticks) */
    uint32_t now100 = dsa_platform_get_ticks_ms() / 10u;
    if (g_last_tick_100hz == 0) g_last_tick_100hz = now100;
    while (g_last_tick_100hz < now100) {
        g_last_tick_100hz++;
        if ((g_last_tick_100hz & 1u) == 0) {
            g_anim_ticks++;
        }
        view_timer_cb();
    }
}

/* =========================================================================
 * 4. C-Ersatz der 16 Assembler-Module (exakte Prototypen laut s_proto.h)
 * ========================================================================= */

/* --- SEG009.ASM: huge_ptr_diff --- */
signed long huge_ptr_diff(void *p1, void *p2) {
    return (signed long)((uint8_t *)p1 - (uint8_t *)p2);
}

/* --- SEG022.ASM: Miles AIL 2.14 (in Etappe 1 stummgeschaltet) --- */
void AIL_startup(void) {}
void AIL_shutdown(short a, short b) { (void)a; (void)b; }
short AIL_register_timer(void *cb) { (void)cb; return 1; }
void AIL_set_timer_Period(short h, unsigned long p) { (void)h; (void)p; }
void AIL_set_timer_frequency(short h, signed long hz) { (void)h; (void)hz; }
void AIL_start_timer(short h) { (void)h; }
void AIL_stop_timer(short h) { (void)h; }
void AIL_release_timer_handle(short h) { (void)h; }
static struct ail_drv_desc g_ail_dsp_desc = {
    200, 2, "VOC", (char *)"Digital", 0x220, 7, 1, 1, 0, 0
};
static unsigned char *g_ail_last_voc = NULL;
static uint32_t g_ail_last_voc_len = 0;

short AIL_register_driver(void *drv) { (void)drv; dsa_platform_ton_log("ail_register"); return 1; }
void AIL_release_driver_handle(short h) { (void)h; }
void *AIL_describe_driver(short h) { (void)h; return &g_ail_dsp_desc; }
short AIL_detect_device(short h, short io, short irq, short dma, short drq) {
    (void)h; (void)io; (void)irq; (void)dma; (void)drq; return 1;
}
void AIL_init_driver(short h, short io, short irq, short dma, short drq) {
    (void)h; (void)io; (void)irq; (void)dma; (void)drq;
}
static uint32_t voc_declared_span(const unsigned char *v) {
    if (!v || memcmp(v, "Creative Voice File\x1a", 20) != 0) return 0x2800u;
    uint32_t hs = (uint32_t)v[20] | ((uint32_t)v[21] << 8);
    if (hs < 26 || hs + 4 > 0x2800u) return 0x2800u;
    uint32_t scan = hs;
    uint32_t end = hs;
    while (scan + 4 <= 0x2800u) {
        uint8_t bt = v[scan];
        if (bt == 0) { end = scan + 1; break; }
        uint32_t bl = (uint32_t)v[scan + 1] | ((uint32_t)v[scan + 2] << 8) | ((uint32_t)v[scan + 3] << 16);
        if (bl > 1024u * 1024u) break;
        end = scan + 4 + bl;
        scan += 4 + bl;
        if (scan > 0x2800u) break;
    }
    if (end < 0x200u) end = 0x200u;
    /* Eine Sprachzeile, nicht das ganze Archiv hinter dem Block. */
    if (end > 160u * 1024u) end = 160u * 1024u;
    return end;
}

void AIL_format_sound_buffer(short h, struct snd_buf *b) { (void)h; (void)b; }
void AIL_register_sound_buffer(short h, short buf_num, struct snd_buf *b) {
    (void)h; (void)buf_num;
    if (!b || !b->data || b->len == 0) return;
    uint32_t n = b->len;
    if (n > 0x2800u) n = 0x2800u;
    dsa_platform_submit_voc_chunk((const unsigned char *)b->data, n);
}
void AIL_format_VOC_file(short h, void *voc, short block) { (void)h; (void)voc; (void)block; }
void AIL_play_VOC_file(short h, void *voc, short block) {
    (void)h; (void)block;
    if (voc) dsa_platform_play_voc((const unsigned char *)voc, 0x2800u);
}
void AIL_index_VOC_block(short h, void *voc, short idx, struct snd_buf *b) {
    (void)h; (void)idx;
    if (!b) return;
    b->pack_type = 0;
    b->sample_rate = 11025;
    b->data = voc ? (unsigned char *)voc : NULL;
    b->len = voc ? voc_declared_span((const unsigned char *)voc) : 0;
    if (voc) {
        g_ail_last_voc = (unsigned char *)voc;
        g_ail_last_voc_len = b->len > 0x2800u ? 0x2800u : b->len;
    }
}
void AIL_start_digital_playback(short h) {
    (void)h;
    /* Die Puffer kommen ueber AIL_register_sound_buffer. Hier nur merken, dass gestartet wurde. */
    dsa_platform_ton_log(g_ail_last_voc ? "ail_start" : "ail_start_leer");
}
void AIL_stop_digital_playback(short h) { (void)h; g_ail_last_voc = NULL; }
void AIL_pause_digital_playback(short h) { (void)h; }
void AIL_resume_digital_playback(short h) { (void)h; AIL_start_digital_playback(h); }
void AIL_set_digital_playback_volume(short h, short vol) {
    (void)h;
    dsa_platform_set_sfx_volume(vol > 100 ? 100 : (vol < 0 ? 0 : vol));
}
short AIL_sound_buffer_status(short h, short b) {
    (void)h; (void)b;
    /* Der Mischer spielt fuer sich. Der Spielcode soll den Puffer als fertig sehen. */
    return 3;
}
unsigned short AIL_state_table_size(short h) { (void)h; return 64; }
short AIL_register_sequence(short h, void *xmi, short seq_num, void *state, void *ctrl) {
    (void)h; (void)xmi; (void)seq_num; (void)state; (void)ctrl; return -1;
}
void AIL_release_sequence_handle(short h, short seq) { (void)h; (void)seq; }
unsigned short AIL_default_timbre_cache_size(short h) { (void)h; return 0; }
void AIL_define_timbre_cache(short h, void *cache, unsigned short size) { (void)h; (void)cache; (void)size; }
unsigned short AIL_timbre_request(short h, short seq) { (void)h; (void)seq; return 0xffff; }
void AIL_install_timbre(short h, short bank, short patch, void *src) {
    (void)h; (void)bank; (void)patch; (void)src;
}
void AIL_start_sequence(short h, short seq) { (void)h; (void)seq; }
void AIL_stop_sequence(short h, short seq) { (void)h; (void)seq; }
void AIL_resume_sequence(short h, short seq) { (void)h; (void)seq; }
short AIL_sequence_status(short h, short seq) { (void)h; (void)seq; return 2; }
short AIL_relative_volume(short h, short seq) { (void)h; (void)seq; return 100; }
void AIL_set_relative_volume(short h, short seq, short pct, short ms) {
    (void)h; (void)seq; (void)pct; (void)ms;
}
void AIL_install_callback(short h, void (*fn)(void)) { (void)h; (void)fn; }
void AIL_cancel_callback(short h) { (void)h; }

/* --- SEG024.ASM: 3D-Software-Rasterizer (originalgetreue C-Portierung) --- */
#define VIEW_W_3D       208
#define VIEW_H_3D       134
#define VIEW_CX_3D      104
#define VIEW_CY_3D      67
#define VIEW_SCR_OFS_3D (2 * 320 + 15)
#define EDGE_EMPTY_LO   0x7fff
#define EDGE_EMPTY_HI   ((signed short)0x8001)

static const uint8_t *get_cur_tex_ptr(void) {
    if (g_cur_tex.seg != 0) {
        return (const uint8_t *)dsa_mk_fp(g_cur_tex.seg, g_cur_tex.off);
    }
    return (const uint8_t *)(uintptr_t)g_cur_tex.off;
}

static const uint8_t *get_shade_lut(int shade_idx) {
    if (shade_idx < 0) shade_idx = 0;
    if (shade_idx > 31) shade_idx = 31;
    if (g_port_shade_base) {
        return g_port_shade_base + (shade_idx << 8);
    }
    return NULL;
}

void edges_clear(void) {
    for (int i = 0; i < VIEW_W_3D; ++i) {
        g_edge_lo[i] = EDGE_EMPTY_LO;
        g_edge_hi[i] = EDGE_EMPTY_HI;
    }
}

void edge_trace(short x0, short y0, short z0,
                short x1, short y1, short z1,
                short u0, short v0, short u1, short v1,
                short horizontal) {
    if (horizontal != 0) {
        short t;
        t = x0; x0 = y0; y0 = t;
        t = x1; x1 = y1; y1 = t;
    }
    if (x0 < 0 && x1 < 0) return;
    if (x0 >= VIEW_W_3D && x1 >= VIEW_W_3D) return;
    if (x0 > x1) {
        short t;
        t = x0; x0 = x1; x1 = t;
        t = y0; y0 = y1; y1 = t;
        t = z0; z0 = z1; z1 = t;
        t = u0; u0 = u1; u1 = t;
        t = v0; v0 = v1; v1 = t;
    }
    short dx = x1 - x0;
    if (dx == 0) {
        if ((uint16_t)x0 < VIEW_W_3D) {
            if (y0 < g_edge_lo[x0]) {
                g_edge_lo[x0] = y0;
                g_edge_lo_z[x0] = z0;
                g_edge_lo_u[x0] = u0;
                g_edge_lo_v[x0] = v0;
            }
            if (y0 > g_edge_hi[x0]) {
                g_edge_hi[x0] = y0;
                g_edge_hi_z[x0] = z0;
                g_edge_hi_u[x0] = u0;
                g_edge_hi_v[x0] = v0;
            }
            if (y1 < g_edge_lo[x0]) {
                g_edge_lo[x0] = y1;
                g_edge_lo_z[x0] = z1;
                g_edge_lo_u[x0] = u1;
                g_edge_lo_v[x0] = v1;
            }
            if (y1 > g_edge_hi[x0]) {
                g_edge_hi[x0] = y1;
                g_edge_hi_z[x0] = z1;
                g_edge_hi_u[x0] = u1;
                g_edge_hi_v[x0] = v1;
            }
        }
        return;
    }

    int32_t count = (int32_t)dx + 1;
    int32_t sy = ((int32_t)(y1 - y0) << 16) / count;
    int32_t sz = ((int32_t)(z1 - z0) << 16) / count;
    int32_t su = ((int32_t)(u1 - u0) << 16) / count;
    int32_t sv = ((int32_t)(v1 - v0) << 16) / count;

    int16_t dpos = (int16_t)(sy >> 16); uint8_t dpos_f = (uint8_t)((sy >> 8) & 0xFF);
    int16_t dz   = (int16_t)(sz >> 16); uint8_t dz_f   = (uint8_t)((sz >> 8) & 0xFF);
    int16_t du   = (int16_t)(su >> 16); uint8_t du_f   = (uint8_t)((su >> 8) & 0xFF);
    int16_t dv   = (int16_t)(sv >> 16); uint8_t dv_f   = (uint8_t)((sv >> 8) & 0xFF);

    int16_t cur_y = y0, cur_z = z0, cur_u = u0, cur_v = v0;
    uint8_t pos_f = 0x80, z_f = 0x80, u_f = 0x80, v_f = 0x80;

    for (int32_t i = 0; i < count; ++i) {
        int16_t col = (int16_t)(x0 + i);
        if (col >= VIEW_W_3D) break;
        if (col >= 0) {
            if (cur_y < g_edge_lo[col]) {
                g_edge_lo[col] = cur_y;
                g_edge_lo_z[col] = cur_z;
                g_edge_lo_u[col] = cur_u;
                g_edge_lo_v[col] = cur_v;
            }
            if (cur_y > g_edge_hi[col]) {
                g_edge_hi[col] = cur_y;
                g_edge_hi_z[col] = cur_z;
                g_edge_hi_u[col] = cur_u;
                g_edge_hi_v[col] = cur_v;
            }
        }
        uint16_t s;
        s = (uint16_t)pos_f + dpos_f; pos_f = (uint8_t)s; cur_y = (int16_t)(cur_y + dpos + (s >> 8));
        s = (uint16_t)u_f   + du_f;   u_f   = (uint8_t)s; cur_u = (int16_t)(cur_u + du   + (s >> 8));
        s = (uint16_t)v_f   + dv_f;   v_f   = (uint8_t)s; cur_v = (int16_t)(cur_v + dv   + (s >> 8));
        s = (uint16_t)z_f   + dz_f;   z_f   = (uint8_t)s; cur_z = (int16_t)(cur_z + dz   + (s >> 8));
    }
}

void edge_trace_persp(short x0, short y0, short z0,
                      short x1, short y1, short z1,
                      short u0, short v0, short u1, short v1,
                      short horizontal) {
    if (horizontal != 0) {
        short t;
        t = x0; x0 = y0; y0 = t;
        t = x1; x1 = y1; y1 = t;
    }
    if (x0 < 0 && x1 < 0) return;
    if (x0 >= VIEW_W_3D && x1 >= VIEW_W_3D) return;
    if (x0 > x1) {
        short t;
        t = x0; x0 = x1; x1 = t;
        t = y0; y0 = y1; y1 = t;
        t = z0; z0 = z1; z1 = t;
        t = u0; u0 = u1; u1 = t;
        t = v0; v0 = v1; v1 = t;
    }
    short dx = x1 - x0;
    if (dx == 0) {
        edge_trace(x0, y0, z0, x1, y1, z1, u0, v0, u1, v1, 0);
        return;
    }

    int32_t n = (int32_t)dx + 1;
    int32_t ep_dpos = ((int32_t)(y1 - y0) << 16) / n;
    int32_t ep_dw   = ((int32_t)(int16_t)(z0 - z1) << 14) / n;
    int32_t w0      = (int32_t)(int16_t)(z0 + g_proj_dist);
    int32_t ep_du_w = (((int32_t)(int16_t)(u1 - u0) << 14) / n) * w0;
    int32_t ep_dv_w = (((int32_t)(int16_t)(v1 - v0) << 14) / n) * w0;
    int32_t ep_dz_w = (((int32_t)(int16_t)(z1 - z0) << 14) / n) * w0;

    int32_t ep_pos   = (int32_t)y0 << 16;
    int32_t ep_den   = (int32_t)(int16_t)(z1 + g_proj_dist) << 14;
    int32_t ep_num_u = 0;
    int32_t ep_num_v = 0;
    int32_t ep_num_z = 0;

    for (int32_t i = 0; i < n; ++i) {
        int16_t col = (int16_t)(x0 + i);
        if (col >= VIEW_W_3D) break;
        if (col >= 0 && ep_den != 0) {
            int16_t cur_u = (int16_t)(ep_num_u / ep_den) + u0;
            int16_t cur_v = (int16_t)(ep_num_v / ep_den) + v0;
            int16_t cur_z = (int16_t)(ep_num_z / ep_den) + z0;
            int16_t cur_y = (int16_t)(ep_pos >> 16);
            if (cur_y < g_edge_lo[col]) {
                g_edge_lo[col] = cur_y;
                g_edge_lo_z[col] = cur_z;
                g_edge_lo_u[col] = cur_u;
                g_edge_lo_v[col] = cur_v;
            }
            if (cur_y > g_edge_hi[col]) {
                g_edge_hi[col] = cur_y;
                g_edge_hi_z[col] = cur_z;
                g_edge_hi_u[col] = cur_u;
                g_edge_hi_v[col] = cur_v;
            }
        }
        ep_num_u += ep_du_w;
        ep_num_v += ep_dv_w;
        ep_num_z += ep_dz_w;
        ep_pos   += ep_dpos;
        ep_den   += ep_dw;
    }
}

void poly_edges(short n, short *idx, struct vtx *vtx_arr, short *uv, short horizontal) {
    if (!idx || !vtx_arr || !uv || n <= 0) return;
    for (short i = 0; i < n; ++i) {
        short j = (i + 1 < n) ? (i + 1) : 0;
        short i0 = idx[i], i1 = idx[j];
        edge_trace(
            vtx_arr[i0].x, vtx_arr[i0].y, vtx_arr[i0].z,
            vtx_arr[i1].x, vtx_arr[i1].y, vtx_arr[i1].z,
            uv[i * 2], uv[i * 2 + 1],
            uv[j * 2], uv[j * 2 + 1],
            horizontal
        );
    }
}

void poly_edges_persp(short n, short *idx, struct vtx *vtx_arr, short *uv, short horizontal) {
    if (!idx || !vtx_arr || !uv || n <= 0) return;
    for (short i = 0; i < n; ++i) {
        short j = (i + 1 < n) ? (i + 1) : 0;
        short i0 = idx[i], i1 = idx[j];
        edge_trace_persp(
            vtx_arr[i0].x, vtx_arr[i0].y, vtx_arr[i0].z,
            vtx_arr[i1].x, vtx_arr[i1].y, vtx_arr[i1].z,
            uv[i * 2], uv[i * 2 + 1],
            uv[j * 2], uv[j * 2 + 1],
            horizontal
        );
    }
}

static void poly_tex_cols_impl(short shade, int masked) {
    if (!g_renderbuf_ptr) return;
    const uint8_t *tex = get_cur_tex_ptr();
    if (!tex || g_cur_tex_w == 0) return;
    uint8_t *dst_base = (uint8_t *)g_renderbuf_ptr;

    int start_col = -1;
    for (int c = 0; c < VIEW_W_3D; ++c) {
        if (g_edge_hi[c] != EDGE_EMPTY_HI && g_edge_hi[c] >= 0) {
            start_col = c;
            break;
        }
    }
    if (start_col < 0) return;

    for (int col = start_col; col < VIEW_W_3D; ++col) {
        int16_t hi = g_edge_hi[col];
        if (hi < 0) return;
        int16_t lo = g_edge_lo[col];
        if (lo > VIEW_H_3D) continue;
        int16_t diff = hi - lo;
        if (diff <= 0) continue;
        int16_t n = diff + 1;

        uint16_t fog_idx = (uint16_t)(g_edge_lo_z[col] >> 4);
        if (fog_idx > 31) fog_idx = 31;
        uint16_t s_idx = (uint16_t)((int8_t)g_fog_table[fog_idx] + shade);
        if (s_idx > 31) s_idx = 31;
        const uint8_t *lut = get_shade_lut((int)s_idx);

        int32_t tex_w = (int32_t)g_cur_tex_w;
        int32_t tx_vwrap = tex_w;
        int16_t dv = g_edge_hi_v[col] - g_edge_lo_v[col];
        int v_neg = 0;
        if (dv < 0) { dv = -dv; v_neg = 1; }
        int16_t v_quot = dv / n;
        int16_t v_rem  = dv % n;
        int32_t tx_vstep = (int32_t)(int16_t)(v_quot * (int16_t)tex_w);
        if (v_neg) {
            tx_vstep = -tx_vstep;
            tx_vwrap = -tx_vwrap;
        }
        uint8_t ch = (uint8_t)((((int32_t)v_rem << 16) / (int32_t)n) >> 8);

        int32_t u_div = ((int32_t)(int16_t)(g_edge_hi_u[col] - g_edge_lo_u[col]) << 16) / (int32_t)n;
        uint8_t cl = (uint8_t)((u_div >> 8) & 0xFF);
        int32_t tx_ustep = (int32_t)(int16_t)((uint32_t)u_div >> 16);

        int32_t esi = (uint16_t)((uint16_t)g_edge_lo_v[col] * (uint16_t)g_cur_tex_w + (uint16_t)g_edge_lo_u[col]);
        uint8_t al = 0x80, ah = 0x80;

        int16_t y_cur = lo;
        if (y_cur < 0) {
            while (y_cur < 0) {
                uint16_t su = (uint16_t)al + cl; al = (uint8_t)su;
                esi = (esi & ~0xFFFF) | ((esi + tx_ustep + (su >> 8)) & 0xFFFF);
                uint16_t sv = (uint16_t)ah + ch; ah = (uint8_t)sv;
                if (sv >> 8) esi = (esi & ~0xFFFF) | ((esi + tx_vwrap) & 0xFFFF);
                esi = (esi & ~0xFFFF) | ((esi + tx_vstep) & 0xFFFF);
                n--;
                y_cur++;
            }
        }
        if (hi > VIEW_H_3D - 1) {
            n -= (hi - (VIEW_H_3D - 1));
        }
        if (n <= 0 || n > VIEW_H_3D) continue;

        uint8_t *dst = dst_base + (int32_t)y_cur * 320 + col;
        for (int16_t k = 0; k < n; ++k) {
            uint8_t bl = tex[esi];
            uint16_t su = (uint16_t)al + cl; al = (uint8_t)su;
            esi += tx_ustep + (int32_t)(su >> 8);
            if (!masked || bl != 0) {
                *dst = lut ? lut[bl] : bl;
            }
            dst += 320;
            uint16_t sv = (uint16_t)ah + ch; ah = (uint8_t)sv;
            if (sv >> 8) esi += tx_vwrap;
            esi += tx_vstep;
        }
    }
}

static void poly_tex_rows_impl(short shade, int masked) {
    if (!g_renderbuf_ptr) return;
    const uint8_t *tex = get_cur_tex_ptr();
    if (!tex || g_cur_tex_w == 0) return;
    uint8_t *dst_base = (uint8_t *)g_renderbuf_ptr;

    int start_row = -1;
    for (int r = 0; r < VIEW_H_3D; ++r) {
        if (g_edge_hi[r] != EDGE_EMPTY_HI && g_edge_hi[r] >= 0) {
            start_row = r;
            break;
        }
    }
    if (start_row < 0) return;

    for (int row = start_row; row < VIEW_H_3D; ++row) {
        int16_t hi = g_edge_hi[row];
        if (hi < 0) return;
        int16_t lo = g_edge_lo[row];
        if (lo > VIEW_W_3D) continue;
        int16_t n = hi - lo + 2;
        if (n <= 0) continue;

        uint16_t fog_idx = (uint16_t)(g_edge_lo_z[row] >> 4);
        if (fog_idx > 31) fog_idx = 31;
        uint16_t s_idx = (uint16_t)((int8_t)g_fog_table[fog_idx] + shade);
        if (s_idx > 31) s_idx = 31;
        s_idx &= 31;
        const uint8_t *lut = get_shade_lut((int)s_idx);

        int32_t tex_w = (int32_t)g_cur_tex_w;
        int32_t tx_vwrap = tex_w;
        int16_t dv = g_edge_hi_v[row] - g_edge_lo_v[row];
        int v_neg = 0;
        if (dv < 0) { dv = -dv; v_neg = 1; }
        int16_t v_quot = dv / n;
        int16_t v_rem  = dv % n;
        int32_t tx_vstep = (int32_t)(int16_t)(v_quot * (int16_t)tex_w);
        if (v_neg) {
            tx_vstep = -tx_vstep;
            tx_vwrap = -tx_vwrap;
        }
        uint8_t ch = (uint8_t)((((int32_t)v_rem << 16) / (int32_t)n) >> 8);

        int32_t u_div = ((int32_t)(int16_t)(g_edge_hi_u[row] - g_edge_lo_u[row]) << 16) / (int32_t)n;
        uint8_t cl = (uint8_t)((u_div >> 8) & 0xFF);
        int32_t tx_ustep = (int32_t)(int16_t)((uint32_t)u_div >> 16);

        int32_t esi = (uint16_t)((uint16_t)g_edge_lo_v[row] * (uint16_t)g_cur_tex_w + (uint16_t)g_edge_lo_u[row]);
        uint8_t al = 0x80, ah = 0x80;

        int16_t x_cur = lo - 1;
        if (x_cur < 0) {
            while (x_cur < 0) {
                uint16_t su = (uint16_t)al + cl; al = (uint8_t)su;
                esi += tx_ustep + (int32_t)(su >> 8);
                uint16_t sv = (uint16_t)ah + ch; ah = (uint8_t)sv;
                if (sv >> 8) esi = (esi & ~0xFFFF) | ((esi + tx_vwrap) & 0xFFFF);
                esi = (esi & ~0xFFFF) | ((esi + tx_vstep) & 0xFFFF);
                n--;
                x_cur++;
            }
        }
        if (hi > VIEW_W_3D - 1) {
            n -= (hi - (VIEW_W_3D - 1));
        }
        if (n <= 0 || n > VIEW_W_3D) continue;

        uint8_t *dst = dst_base + (int32_t)row * 320 + x_cur;
        for (int16_t k = 0; k < n; ++k) {
            uint8_t bl = tex[esi];
            uint16_t su = (uint16_t)al + cl; al = (uint8_t)su;
            esi += tx_ustep + (int32_t)(su >> 8);
            if (!masked || bl != 0) {
                dst[k] = lut ? lut[bl] : bl;
            }
            uint16_t sv = (uint16_t)ah + ch; ah = (uint8_t)sv;
            if (sv >> 8) esi += tx_vwrap;
            esi += tx_vstep;
        }
    }
}

void poly_tex_cols(short shade)   { poly_tex_cols_impl(shade, 0); }
void poly_tex_cols_m(short shade) { poly_tex_cols_impl(shade, 1); }
void poly_tex_rows(short shade)   { poly_tex_rows_impl(shade, 0); }
void poly_tex_rows_m(short shade) { poly_tex_rows_impl(shade, 1); }

void poly_flat(short shade, short color) {
    if (!g_renderbuf_ptr) return;
    uint8_t *dst_base = (uint8_t *)g_renderbuf_ptr;
    const uint8_t *lut = get_shade_lut(shade);
    uint8_t c = (uint8_t)color;
    if (lut) c = lut[c];

    int start_col = -1;
    for (int col = 0; col < VIEW_W_3D; ++col) {
        if (g_edge_hi[col] >= 0) {
            start_col = col;
            break;
        }
    }
    if (start_col < 0) return;

    for (int col = start_col; col < VIEW_W_3D; ++col) {
        int16_t hi = g_edge_hi[col];
        if (hi < 0) continue;
        int16_t lo = g_edge_lo[col];
        if (lo >= VIEW_H_3D) continue;
        int16_t n = hi - lo + 1;
        if (n <= 0) continue;
        if (lo < 0) {
            n += lo;
            if (n < 0) continue;
            lo = 0;
        }
        if (hi > VIEW_H_3D - 1) {
            n -= (hi - (VIEW_H_3D - 1));
        }
        if (n <= 0 || n > VIEW_H_3D) continue;
        uint8_t *dst = dst_base + (int32_t)lo * 320 + col;
        for (int16_t k = 0; k < n; ++k) {
            *dst = c;
            dst += 320;
        }
    }
}

void fill_scene(short color) {
    if (!g_renderbuf_ptr) return;
    uint8_t *dst = (uint8_t *)g_renderbuf_ptr;
    for (int y = 0; y < VIEW_H_3D; ++y) {
        memset(dst + y * 320, (uint8_t)color, VIEW_W_3D);
    }
}

void copy_3d_to_screen(void) {
    if (!g_renderbuf_ptr || !g_vga_buf) return;
    const uint8_t *src = (const uint8_t *)g_renderbuf_ptr;
    uint8_t *dst = (uint8_t *)g_vga_buf + VIEW_SCR_OFS_3D;
    for (int y = 0; y < VIEW_H_3D; ++y) {
        memcpy(dst + y * 320, src + y * 320, VIEW_W_3D);
    }
    dsa_platform_wait_vsync();
}

/* --- SEG025.ASM: 3D-Clipping, Projektion & Flat-Copy --- */
#define CLIP_MAX_3D 16
#define FACE_MAX_3D 129
#define VTX_MAX_3D  129
#define VTX_PATH_3D 0x7fff

static signed short s_pj_scale = 90;
static signed short s_pj_dist  = 32;
static struct face3d  s_clip_buf[CLIP_MAX_3D];
static struct face3d *s_face_ptrs[FACE_MAX_3D];

static inline int16_t mul_shr15(int16_t a, int16_t b) {
    return (int16_t)(((int32_t)a * (int32_t)b) >> 15);
}

static inline void rot_vec3(int16_t x, int16_t y, int16_t z,
                            int16_t s0, int16_t c0,
                            int16_t s1, int16_t c1,
                            int16_t s2, int16_t c2,
                            int16_t *ox, int16_t *oy, int16_t *oz) {
    int16_t z1 = (int16_t)(mul_shr15(z, c2) - mul_shr15(x, s2));
    int16_t x1 = (int16_t)(mul_shr15(z, s2) + mul_shr15(x, c2));
    int16_t y2 = (int16_t)(mul_shr15(y, c1) - mul_shr15(z1, s1));
    int16_t z2 = (int16_t)(mul_shr15(y, s1) + mul_shr15(z1, c1));
    int16_t x3 = (int16_t)(mul_shr15(x1, c0) - mul_shr15(y2, s0));
    int16_t y3 = (int16_t)(mul_shr15(x1, s0) + mul_shr15(y2, c0));
    *ox = x3;
    *oy = y3;
    *oz = z2;
}

static struct face3d *clip_face_z0(struct face3d *f, int16_t first_vtx) {
    if (g_nr_clip_faces >= CLIP_MAX_3D) {
        return f;
    }
    struct face3d *cf = &s_clip_buf[g_nr_clip_faces++];
    memcpy(cf, f, 0x18);
    cf->nverts = 0;

    int16_t n = f->nverts;
    for (int16_t i = 0; i < n; ++i) {
        int16_t j = (i + 1 < n) ? (i + 1) : 0;
        int16_t a_idx = (int16_t)(first_vtx + f->idx[i]);
        int16_t b_idx = (int16_t)(first_vtx + f->idx[j]);
        struct vtx a = g_vtx_buf[a_idx];
        struct vtx b = g_vtx_buf[b_idx];
        int16_t ua = f->uv[i * 2], va = f->uv[i * 2 + 1];
        int16_t ub = f->uv[j * 2], vb = f->uv[j * 2 + 1];

        if (a.z >= 0 && b.z >= 0) {
            int16_t k = cf->nverts++;
            cf->idx[k] = (int16_t)(a_idx - first_vtx);
            cf->uv[k * 2]     = ua;
            cf->uv[k * 2 + 1] = va;
        } else if (a.z < 0 && b.z < 0) {
            /* beide hinter dem Auge */
        } else {
            int16_t dz_abs = (int16_t)(a.z - b.z);
            if (dz_abs < 0) dz_abs = -dz_abs;
            int16_t za_abs = a.z;
            if (za_abs < 0) za_abs = -za_abs;
            int16_t nx_v = a.x, ny_v = a.y, nu_v = ua, nv_v = va;
            if (dz_abs != 0) {
                nx_v = (int16_t)(a.x + (int16_t)(((int32_t)(b.x - a.x) * za_abs) / dz_abs));
                ny_v = (int16_t)(a.y + (int16_t)(((int32_t)(b.y - a.y) * za_abs) / dz_abs));
                nu_v = (int16_t)(ua  + (int16_t)(((int32_t)(ub - ua)   * za_abs) / dz_abs));
                nv_v = (int16_t)(va  + (int16_t)(((int32_t)(vb - va)   * za_abs) / dz_abs));
            }
            if (a.z >= 0) {
                int16_t k = cf->nverts++;
                cf->idx[k] = (int16_t)(a_idx - first_vtx);
                cf->uv[k * 2]     = ua;
                cf->uv[k * 2 + 1] = va;
            }
            int16_t k = cf->nverts++;
            cf->idx[k] = (int16_t)(g_nr_vtx - first_vtx);
            cf->uv[k * 2]     = nu_v;
            cf->uv[k * 2 + 1] = nv_v;
            g_vtx_buf[g_nr_vtx].x = nx_v;
            g_vtx_buf[g_nr_vtx].y = ny_v;
            g_vtx_buf[g_nr_vtx].z = 0;
            g_vtx_buf[g_nr_vtx].w = 0;
            g_nr_vtx++;
        }
    }
    return cf;
}

void view_set_projection(short scale, short eye_dist) {
    s_pj_scale   = scale;
    g_proj_scale = scale;
    s_pj_dist    = eye_dist;
    g_proj_dist  = eye_dist;
}

short obj_project(void *obj_ptr) {
    struct obj3d *obj = (struct obj3d *)obj_ptr;
    if (!obj || !obj->vl || !g_vtx_buf || !g_face_list || !g_face_next || !g_face_depth) {
        return 1;
    }

    /* 1. Lage des Objekts relativ zum Auge */
    int16_t dx = (int16_t)(obj->x - g_cam_eye_x);
    int16_t dy = (int16_t)(obj->y - g_cam_eye_y);
    int16_t dz = (int16_t)(obj->z - g_cam_eye_z);

    int16_t s0 = g_sin_table[g_cam_neg[0] & 0x3ff];
    int16_t c0 = g_sin_table[(g_cam_neg[0] + 0x100) & 0x3ff];
    int16_t s1 = g_sin_table[g_cam_neg[1] & 0x3ff];
    int16_t c1 = g_sin_table[(g_cam_neg[1] + 0x100) & 0x3ff];
    int16_t s2 = g_sin_table[g_cam_neg[2] & 0x3ff];
    int16_t c2 = g_sin_table[(g_cam_neg[2] + 0x100) & 0x3ff];

    int16_t ox, oy, oz;
    rot_vec3(dx, dy, dz, s0, c0, s1, c1, s2, c2, &ox, &oy, &oz);

    /* 2. Winkel fuer die Punkte: Kamera + Objekt */
    int16_t a0 = (int16_t)(g_cam_neg[0] + obj->rot.ax);
    int16_t a1 = (int16_t)(g_cam_neg[1] + obj->rot.ay);
    int16_t a2 = (int16_t)(g_cam_neg[2] + obj->rot.az);

    s0 = g_sin_table[a0 & 0x3ff];
    c0 = g_sin_table[(a0 + 0x100) & 0x3ff];
    s1 = g_sin_table[a1 & 0x3ff];
    c1 = g_sin_table[(a1 + 0x100) & 0x3ff];
    s2 = g_sin_table[a2 & 0x3ff];
    c2 = g_sin_table[(a2 + 0x100) & 0x3ff];

    int16_t first_vtx = g_nr_vtx;
    struct vtx_list *vl = obj->vl;
    int16_t n_pts = vl->n;
    g_nr_vtx = (int16_t)(g_nr_vtx + n_pts);
    if ((uint16_t)g_nr_vtx >= VTX_MAX_3D) {
        return 0;
    }

    for (int16_t i = 0; i < n_pts; ++i) {
        struct vtx src = vl->v[i];
        if (src.x == VTX_PATH_3D) {
            struct vtx_path *vp = g_vtx_paths[src.y];
            if (vp && vp->n > 0) {
                int16_t ph_val = (&obj->phase)[vp->m];
                int16_t idx = (int16_t)((uint16_t)ph_val % (uint16_t)vp->n);
                src = vp->e[idx];
            }
        }
        int16_t vx, vy, vz;
        rot_vec3(src.x, src.y, src.z, s0, c0, s1, c1, s2, c2, &vx, &vy, &vz);
        g_vtx_buf[first_vtx + i].x = (int16_t)(vx + ox);
        g_vtx_buf[first_vtx + i].y = (int16_t)(vy + oy);
        g_vtx_buf[first_vtx + i].z = (int16_t)(vz + oz);
        g_vtx_buf[first_vtx + i].w = 0;
    }

    /* 3. Flaechen mit einer Ecke hinter dem Auge abschneiden */
    int16_t nfaces = obj->nfaces;
    if (nfaces > FACE_MAX_3D) {
        return 0;
    }
    for (int16_t f_i = 0; f_i < nfaces; ++f_i) {
        struct face3d *f = obj->faces[f_i];
        int16_t nv = f->nverts;
        for (int16_t c = 0; c < nv; ++c) {
            if (g_vtx_buf[first_vtx + f->idx[c]].z < 0) {
                f = clip_face_z0(f, first_vtx);
                break;
            }
        }
        s_face_ptrs[f_i] = f;
    }

    /* 4. Punkte projizieren */
    for (int16_t v_i = first_vtx; v_i < g_nr_vtx; ++v_i) {
        int16_t vz = g_vtx_buf[v_i].z;
        if (vz >= 0) {
            int16_t denom = (int16_t)(vz + s_pj_dist);
            if (denom != 0) {
                int16_t sx = (int16_t)(((int32_t)g_vtx_buf[v_i].x * s_pj_scale) >> 4);
                int16_t sy = (int16_t)(((int32_t)g_vtx_buf[v_i].y * s_pj_scale) >> 4);
                g_vtx_buf[v_i].x = (int16_t)(VIEW_CX_3D + ((int32_t)sx * s_pj_dist) / denom);
                g_vtx_buf[v_i].y = (int16_t)(VIEW_CY_3D + ((int32_t)sy * s_pj_dist) / denom);
            }
        }
    }

    /* 5. Normale drehen, Listeneintrag, Sichtbarkeit, einsortieren */
    int16_t *next_arr  = (int16_t *)g_face_next;
    int16_t *depth_arr = (int16_t *)g_face_depth;

    for (int16_t f_i = 0; f_i < nfaces; ++f_i) {
        struct face3d *f = s_face_ptrs[f_i];
        uint16_t flags = f->flags;

        int16_t cnx, cny, cnz;
        rot_vec3(f->nx, f->ny, f->nz, s0, c0, s1, c1, s2, c2, &cnx, &cny, &cnz);
        *(int16_t *)((uint8_t *)f + 0x08) = cnx;
        *(int16_t *)((uint8_t *)f + 0x0a) = cny;
        *(int16_t *)((uint8_t *)f + 0x0c) = cnz;

        if ((uint16_t)g_nr_vis_faces >= FACE_MAX_3D - 1) {
            return 0;
        }
        struct face_ent *fe = &g_face_list[g_nr_vis_faces];
        fe->face = f;
        *(int16_t *)(&fe->f04[0]) = first_vtx;
        *(int16_t *)(&fe->f04[2]) = cnx;
        *(int16_t *)(&fe->f04[4]) = cny;
        fe->angle = cnz;
        *(int16_t *)(&fe->f0c[0]) = *(int16_t *)((uint8_t *)f + 0x0e);
        fe->event = *(int32_t *)(&f->tex);

        int16_t nv = f->nverts;
        if (nv < 3) continue;

        if (flags & 0x0001) {
            const struct vtx *v0 = &g_vtx_buf[first_vtx + f->idx[0]];
            const struct vtx *v1 = &g_vtx_buf[first_vtx + f->idx[1]];
            const struct vtx *v2 = &g_vtx_buf[first_vtx + f->idx[2]];
            int32_t term1 = (int32_t)(int16_t)(v1->y - v0->y) * (int32_t)(int16_t)(v2->x - v0->x);
            int32_t term2 = (int32_t)(int16_t)(v1->x - v0->x) * (int32_t)(int16_t)(v2->y - v0->y);
            if (term2 - term1 < 0) continue;
        }

        int16_t left = 0, right = 0, above = 0, below = 0;
        int16_t zmax = 0, zmin = 0x7fff;
        int behind = 0;
        for (int16_t c = 0; c < nv; ++c) {
            const struct vtx *v = &g_vtx_buf[first_vtx + f->idx[c]];
            if (v->z < 0) { behind = 1; break; }
            if (v->z >= zmax) zmax = v->z;
            if (v->z <= zmin) zmin = v->z;
            if (v->x < 0) left++;
            if (v->x > VIEW_W_3D) right++;
            if (v->y < 0) above++;
            if (v->y > VIEW_H_3D) below++;
        }
        if (behind) continue;
        if (left == nv || right == nv || above == nv || below == nv) continue;

        if (g_nr_vis_faces == 0) {
            next_arr[0]  = 1;
            depth_arr[1] = zmax;
        } else {
            int16_t cur  = next_arr[0];
            int16_t prev = 0;
            for (int16_t cnt = 0; cnt < g_nr_vis_faces; ++cnt) {
                if (depth_arr[cur] < zmax) break;
                prev = cur;
                cur  = next_arr[cur];
            }
            int16_t k_new = (int16_t)(g_nr_vis_faces + 1);
            next_arr[prev]   = k_new;
            next_arr[k_new]  = cur;
            depth_arr[k_new] = zmax;
        }
        g_nr_vis_faces++;
    }
    return 1;
}

void clip_face(struct face3d *f) { (void)f; }

void tex_copy_flat(short src_seg, signed long src_off,
                   short dst_seg, signed long dst_off,
                   signed long len) {
    const void *src = (src_seg == 0) ? (const void *)(uintptr_t)src_off
                                     : dsa_mk_fp((unsigned short)src_seg, (unsigned long)src_off);
    void *dst = ((unsigned short)dst_seg == 0xa000)
                ? dsa_mk_fp(0xa000, (unsigned long)dst_off)
                : (void *)(((uintptr_t)(unsigned short)dst_seg << 16) + (uintptr_t)(unsigned long)dst_off);
    if (src && dst && len > 0) {
        memcpy(dst, src, (size_t)len);
    }
}

/* --- SEG026.ASM: Glyphen-Blitter --- */
void ovl_trap_isr(void) {}

void glyph_blit(unsigned char *dst, unsigned char *src, unsigned char *pal,
                unsigned char w, unsigned char h) {
    if (!dst || !src || !pal || w == 0 || h == 0) return;
    for (unsigned char row = 0; row < h; ++row) {
        for (unsigned char col = 0; col < w; ++col) {
            uint8_t idx = *src++;
            if (idx != 0) {
                dst[col] = pal[idx];
            }
        }
        dst += 320;
    }
}

void glyph_blit_bg(unsigned char *dst, unsigned char *src, unsigned char *pal,
                   unsigned char w, unsigned char h, unsigned char bg) {
    if (!dst || !src || !pal || w == 0 || h == 0) return;
    for (unsigned char row = 0; row < h; ++row) {
        for (unsigned char col = 0; col < w; ++col) {
            uint8_t idx = *src++;
            dst[col] = (idx != 0) ? pal[idx] : bg;
        }
        dst += 320;
    }
}

/* --- SEG027.ASM: Reisekarten-Blitter --- */
void tmap_blit_rows(char *src, unsigned char *rows_tab, short fine_y) {
    if (!src || !rows_tab || !g_vga_buf) return;
    const uint8_t *base = (const uint8_t *)src;
    uint8_t *dst = (uint8_t *)g_vga_buf;

    g_tmap_row_h[0]  = (unsigned short)(20 - fine_y);
    g_tmap_row_h[10] = (unsigned short)fine_y;

    int screen_y = 0;
    for (int r = 0; r < 11; ++r) {
        int h = g_tmap_row_h[r];
        if (h <= 0) continue;
        uint8_t row_idx = rows_tab[r + 1];
        if (row_idx >= 12) continue;
        const uint8_t *row_src = base + g_tmap_row_ofs[row_idx];
        if (r == 0) {
            row_src += fine_y * 0x120;
        }
        for (int y = 0; y < h && screen_y < 200; ++y, ++screen_y) {
            memcpy(dst + screen_y * 320, row_src + y * 0x120, 240);
        }
    }
}

/* --- SEG031.ASM: BIOS-Tastaturpuffer leeren --- */
void clear_kbd(void) {
    while (dsa_platform_check_key()) {
        dsa_platform_get_key();
    }
}

void wait_key_clear(void) {
    dsa_platform_get_key();
    clear_kbd();
}

/* --- SEG033.ASM: PowerPacker PP20 Entpacker (adaptiert aus BrightEyes seg009.cpp, GPL v3) --- */
static uint32_t pp_val;
static int      pp_bits_left;
static const uint8_t *pp_src;

static uint32_t pp_get_bits(int count) {
    uint32_t res = 0;
    while (count > 0) {
        if (pp_bits_left == 0) {
            pp_src -= 4;
            pp_val = ((uint32_t)pp_src[0] << 24) |
                     ((uint32_t)pp_src[1] << 16) |
                     ((uint32_t)pp_src[2] << 8)  |
                     ((uint32_t)pp_src[3]);
            pp_bits_left = 32;
        }
        res = (res << 1) | (pp_val & 1u);
        pp_val >>= 1;
        pp_bits_left--;
        count--;
    }
    return res;
}

void ppDecrunch(unsigned char *src, unsigned char *dst,
                unsigned short eff_off, unsigned short eff_seg, unsigned long size) {
    (void)eff_off; (void)eff_seg;
    if (!src || !dst || size < 8) return;
    const uint8_t *eff   = src + 4;
    const uint8_t *end_p = src + size - 4;

    uint32_t dec_len = ((uint32_t)end_p[0] << 16) | ((uint32_t)end_p[1] << 8) | (uint32_t)end_p[2];
    uint8_t  skip_bits = end_p[3];
    uint8_t *out_p = dst + dec_len;

    pp_src = end_p;
    pp_bits_left = 0;
    pp_val = 0;
    if (skip_bits > 0) {
        pp_get_bits(skip_bits);
    }

    while (out_p > dst) {
        if (pp_get_bits(1) == 0) {
            uint32_t cnt = 1;
            uint32_t add;
            do {
                add = pp_get_bits(2);
                cnt += add;
            } while (add == 3);
            while (cnt-- > 0 && out_p > dst) {
                *--out_p = (uint8_t)pp_get_bits(8);
            }
            if (out_p <= dst) break;
        }
        uint32_t idx = pp_get_bits(2);
        uint32_t nbits = eff[idx];
        uint32_t len = idx + 2;
        uint32_t off;
        if (idx == 3) {
            if (pp_get_bits(1) == 0) {
                off = pp_get_bits(7);
            } else {
                off = pp_get_bits((int)nbits);
            }
            uint32_t add;
            do {
                add = pp_get_bits(3);
                len += add;
            } while (add == 7);
        } else {
            off = pp_get_bits((int)nbits);
        }
        while (len-- > 0 && out_p > dst) {
            uint8_t b = out_p[off];
            *--out_p = b;
        }
    }
}

/* --- SEG034.ASM: Linien & Fadenkreuz-/Rahmen-Sicherung --- */
void fill_line(unsigned char *dst, short len, signed char color) {
    if (!dst || len <= 0) return;
    memset(dst, (uint8_t)color, (size_t)len);
}

void fill_v_line(unsigned char *dst, short len, signed char color, short stride) {
    if (!dst || len <= 0) return;
    for (short i = 0; i < len; ++i, dst += stride) {
        *dst = (uint8_t)color;
    }
}

void vga_save_row(short off, char *buf) {
    if (!buf) return;
    uint16_t uoff = (uint16_t)off;
    if ((uint32_t)uoff + 320 <= DSA_VRAM_SIZE) {
        memcpy(buf, dsa_platform_get_vram() + uoff, 320);
    }
}

void vga_restore_row(short off, char *buf) {
    if (!buf) return;
    uint16_t uoff = (uint16_t)off;
    if ((uint32_t)uoff + 320 <= DSA_VRAM_SIZE) {
        memcpy(dsa_platform_get_vram() + uoff, buf, 320);
    }
}

void vga_restore_row_part(short off, char *buf, short n) {
    if (!buf || n <= 0) return;
    uint16_t uoff = (uint16_t)off;
    if ((uint32_t)uoff + (uint32_t)n <= DSA_VRAM_SIZE) {
        memcpy(dsa_platform_get_vram() + uoff, buf, (size_t)n);
    }
}

void vga_save_col(short off, char *buf) {
    if (!buf) return;
    uint16_t uoff = (uint16_t)off;
    const uint8_t *vram = dsa_platform_get_vram();
    for (int y = 0; y < 200; ++y) {
        uint32_t pos = (uint32_t)uoff + (uint32_t)y * 320u;
        if (pos < DSA_VRAM_SIZE) buf[y] = (char)vram[pos];
    }
}

void vga_restore_col(short off, char *buf) {
    if (!buf) return;
    uint16_t uoff = (uint16_t)off;
    uint8_t *vram = dsa_platform_get_vram();
    for (int y = 0; y < 200; ++y) {
        uint32_t pos = (uint32_t)uoff + (uint32_t)y * 320u;
        if (pos < DSA_VRAM_SIZE) vram[pos] = (uint8_t)buf[y];
    }
}

void vga_restore_col_part(short off, char *buf, short n) {
    if (!buf || n <= 0) return;
    uint16_t uoff = (uint16_t)off;
    uint8_t *vram = dsa_platform_get_vram();
    for (short y = 0; y < n && y < 200; ++y) {
        uint32_t pos = (uint32_t)uoff + (uint32_t)y * 320u;
        if (pos < DSA_VRAM_SIZE) vram[pos] = (uint8_t)buf[y];
    }
}

/* --- SEG035.ASM: Flat Mode (Unreal Mode) --- */
short flat_enter(void) { return 1; }
void  flat_leave(void) {}
void  flat_copy(unsigned short src_seg, unsigned long src_off,
                unsigned short dst_seg, unsigned long dst_off,
                unsigned long len) {
    const void *src = (src_seg == 0) ? (const void *)(uintptr_t)src_off : dsa_mk_fp(src_seg, src_off);
    void *dst = (dst_seg == 0) ? (void *)(uintptr_t)dst_off : dsa_mk_fp(dst_seg, dst_off);
    if (src && dst && len > 0) {
        memcpy(dst, src, (size_t)len);
    }
}

/* --- SEG036.ASM: LIM-EMS --- */
void *EMS_norm_ptr(void *p) { return p; }
short EMS_init(void) { return 0; }
void *EMS_get_frame_ptr(void) { return g_ems_frame_buf; }
int   EMS_get_num_pages_unalloced(void) { return 0; }
short EMS_alloc_pages(short pages) { (void)pages; return 0; }
short EMS_map_memory(short handle, short log_page, char phys_page) {
    (void)handle; (void)log_page; (void)phys_page; return 0;
}
short EMS_free_pages(short handle) { (void)handle; return 0; }

/* --- SEG038.C + SEG039.ASM: XMS-Emulation im flachen 32-Bit-Heap --- */
struct xms_block {
    uint8_t *ptr;
    uint32_t size;
};

#define MAX_XMS_HANDLES 128
static struct xms_block g_xms_blocks[MAX_XMS_HANDLES];

short xms_check(void) {
    return 1;
}

short xms_present(void) {
    return 1;
}

short xms_free_kb(void) {
    return 8192;
}

signed long xms_alloc_kb(short kb) {
    if ((unsigned short)kb >= 0x4000) return 0;
    if (kb <= 0) kb = 1;
    for (int h = 1; h < MAX_XMS_HANDLES; ++h) {
        if (!g_xms_blocks[h].ptr) {
            g_xms_blocks[h].size = (uint32_t)(unsigned short)kb * 1024u;
            g_xms_blocks[h].ptr  = (uint8_t *)calloc(1, g_xms_blocks[h].size + 64);
            return h;
        }
    }
    return 0;
}

void xms_free_block(short h) {
    if (h > 0 && h < MAX_XMS_HANDLES && g_xms_blocks[h].ptr) {
        free(g_xms_blocks[h].ptr);
        g_xms_blocks[h].ptr = NULL;
        g_xms_blocks[h].size = 0;
    }
}

void *xms_lock(short h) {
    if (h > 0 && h < MAX_XMS_HANDLES && g_xms_blocks[h].ptr) {
        return g_xms_blocks[h].ptr;
    }
    return NULL;
}

signed long xms_unlock(short h) {
    (void)h;
    return 1;
}

int cputs(const char *s) {
    return s ? fputs(s, stdout) : 0;
}

int cprintf(const char *fmt, ...) {
    (void)fmt;
    return 0;
}

void xms_local_a20_on(void) {}
void xms_local_a20_off(void) {}

short xms_call(short fn, short dx, short *out_ax, short *out_dx) {
    int ah = (fn >> 8) & 0xff;
    if (ah == 0x08) {
        if (out_ax) *out_ax = 8192;
        if (out_dx) *out_dx = 8192;
        return 8192;
    } else if (ah == 0x09) {
        int kb = (unsigned short)dx;
        if (kb <= 0) kb = 1;
        for (int h = 1; h < MAX_XMS_HANDLES; ++h) {
            if (!g_xms_blocks[h].ptr) {
                g_xms_blocks[h].size = (uint32_t)kb * 1024u;
                g_xms_blocks[h].ptr  = (uint8_t *)calloc(1, g_xms_blocks[h].size + 64);
                if (out_ax) *out_ax = 1;
                if (out_dx) *out_dx = (short)h;
                return 1;
            }
        }
        if (out_ax) *out_ax = 0;
        if (out_dx) *out_dx = 0;
        return 0;
    } else if (ah == 0x0A) {
        int h = (unsigned short)dx;
        if (h > 0 && h < MAX_XMS_HANDLES && g_xms_blocks[h].ptr) {
            free(g_xms_blocks[h].ptr);
            g_xms_blocks[h].ptr = NULL;
            g_xms_blocks[h].size = 0;
        }
        if (out_ax) *out_ax = 1;
        if (out_dx) *out_dx = 0;
        return 1;
    } else if (ah == 0x0C) {
        int h = (unsigned short)dx;
        uintptr_t addr = 0;
        if (h > 0 && h < MAX_XMS_HANDLES && g_xms_blocks[h].ptr) {
            addr = (uintptr_t)g_xms_blocks[h].ptr;
        }
        if (out_ax) *out_ax = (short)(addr & 0xFFFFu);
        if (out_dx) *out_dx = (short)((addr >> 16) & 0xFFFFu);
        return 1;
    }
    if (out_ax) *out_ax = 1;
    if (out_dx) *out_dx = 0;
    return 1;
}

signed long xms_move(unsigned long move_ptr) {
    struct mem_move *m = (struct mem_move *)(uintptr_t)move_ptr;
    if (!m || m->len <= 0) return 1;
    const uint8_t *src = NULL;
    uint8_t *dst = NULL;

    if (m->src_handle == 0) {
        src = (const uint8_t *)(uintptr_t)(uint32_t)m->src;
    } else if (m->src_handle > 0 && m->src_handle < MAX_XMS_HANDLES && g_xms_blocks[m->src_handle].ptr) {
        src = g_xms_blocks[m->src_handle].ptr + (uint32_t)m->src;
    }

    if (m->dst_handle == 0) {
        dst = (uint8_t *)(uintptr_t)(uint32_t)m->dst;
    } else if (m->dst_handle > 0 && m->dst_handle < MAX_XMS_HANDLES && g_xms_blocks[m->dst_handle].ptr) {
        dst = g_xms_blocks[m->dst_handle].ptr + (uint32_t)m->dst;
    }

    if (src && dst) {
        memmove(dst, src, (size_t)m->len);
        return 1;
    }
    return 0;
}

/* --- SEG043.ASM: VGA-Modus & Palette --- */
void set_video_mode(short mode) {
    (void)mode;
    dsa_platform_present();
}

void set_display_page(short page) { (void)page; }

void save_display_stat(void *p) {
    if (p) memset(p, 0, sizeof(struct video_state));
}

void restore_video_mode(void *p) { (void)p; }

void vga_set_color(void *rgb, short idx) {
    if (!rgb) return;
    dsa_platform_set_palette(idx & 0xff, 1, (const uint8_t *)rgb);
    dsa_platform_present();
}

void vga_set_palette(void *pal, short first, unsigned short count) {
    if (!pal || count == 0) return;
    dsa_platform_set_palette(first & 0xff, count, (const uint8_t *)pal);
    dsa_platform_present();
}

void get_color(unsigned char *rgb, short idx) {
    if (!rgb) return;
    dsa_platform_get_palette(idx & 0xff, 1, rgb);
}

void get_palette(void *pal, short first, short count) {
    if (!pal || count <= 0) return;
    dsa_platform_get_palette(first & 0xff, count, (uint8_t *)pal);
}

void *dos_get_indos_ptr(void) {
    return &g_dummy_indos;
}

/* --- SEG046.ASM: 2D-Sprite-Blitter (7 Modi + Clipping) & Skalierer --- */
void blit_lines(short x, short y, short w, short h, short unused,
                unsigned char **lines, signed char mode, void *dst_ptr, short back) {
    (void)unused;
    if (!dst_ptr || !lines || w <= 0 || h <= 0) return;
    uint8_t *dst_base = (uint8_t *)dst_ptr;

    int cx0 = g_clip_rect.x0;
    int cy0 = g_clip_rect.y0;
    int cx1 = g_clip_rect.x1;
    int cy1 = g_clip_rect.y1;
    if (cx1 <= 0) cx1 = 319;
    if (cy1 <= 0) cy1 = 199;

    int skip_top  = (y < cy0) ? (cy0 - y) : 0;
    int skip_left = (x < cx0) ? (cx0 - x) : 0;
    int end_x     = (x + w - 1 > cx1) ? (cx1 + 1) : (x + w);
    int end_y     = (y + h - 1 > cy1) ? (cy1 + 1) : (y + h);
    int draw_x    = x + skip_left;
    int draw_y    = y + skip_top;
    int draw_w    = end_x - draw_x;
    int draw_h    = end_y - draw_y;
    if (draw_w <= 0 || draw_h <= 0) return;

    for (int row = 0; row < draw_h; ++row) {
        int src_row = back ? (h - 1 - (skip_top + row)) : (skip_top + row);
        const uint8_t *src = lines[src_row];
        if (!src) continue;
        src += skip_left;
        uint8_t *dst = dst_base + (draw_y + row) * 320 + draw_x;

        switch (mode) {
            case 1: /* BLIT_COPY */
                memcpy(dst, src, (size_t)draw_w);
                break;
            case 2: /* BLIT_DARK */
                for (int i = 0; i < draw_w; ++i) {
                    uint8_t c = src[i];
                    dst[i] = (c < 0xc8) ? (uint8_t)(c + 0x20) : c;
                }
                break;
            case 3: /* BLIT_FLIP_X */
                for (int i = 0; i < draw_w; ++i) {
                    uint8_t c = lines[src_row][w - 1 - (skip_left + i)];
                    if (c != 0) dst[i] = c;
                }
                break;
            case 4: /* BLIT_MASKED */
                for (int i = 0; i < draw_w; ++i) {
                    uint8_t c = src[i];
                    if (c != 0) dst[i] = c;
                }
                break;
            case 5: /* BLIT_BEHIND */
                for (int i = 0; i < draw_w; ++i) {
                    if (dst[i] == 0) dst[i] = src[i];
                }
                break;
            case 6: /* BLIT_GRAY */
                for (int i = 0; i < draw_w; ++i) {
                    uint8_t c = src[i];
                    if (c != 0) dst[i] = (uint8_t)((c >> 3) + 0xc0);
                }
                break;
            case 7: /* BLIT_MASKED_FF */
                for (int i = 0; i < draw_w; ++i) {
                    uint8_t c = src[i];
                    if (c != 0xff) dst[i] = c;
                }
                break;
            default:
                memcpy(dst, src, (size_t)draw_w);
                break;
        }
    }
    if (dst_ptr == g_vga_buf) {
        dsa_platform_present();
    }
}

void grab_lines(short x, short y, short w, short h, unsigned char **lines, void *src_ptr) {
    if (!src_ptr || !lines || w <= 0 || h <= 0) return;
    const uint8_t *src_base = (const uint8_t *)src_ptr;
    for (int row = 0; row < h; ++row) {
        int sy = y + row;
        if (sy < 0 || sy >= 200 || !lines[row]) continue;
        int sx = x;
        int cw = w;
        int dx = 0;
        if (sx < 0) { dx = -sx; cw += sx; sx = 0; }
        if (sx + cw > 320) cw = 320 - sx;
        if (cw > 0) {
            memcpy(lines[row] + dx, src_base + sy * 320 + sx, (size_t)cw);
        }
    }
}

void scale_lines(unsigned char **src_lines, short src_w, short src_h,
                 unsigned char **dst_lines, short dst_w, short dst_h) {
    if (!src_lines || !dst_lines || src_w <= 0 || src_h <= 0 || dst_w <= 0 || dst_h <= 0)
        return;
    for (short dy = 0; dy < dst_h; ++dy) {
        short sy = (short)(((int32_t)dy * src_h) / dst_h);
        const uint8_t *srow = src_lines[sy];
        uint8_t *drow = dst_lines[dy];
        if (!srow || !drow) continue;
        for (short dx = 0; dx < dst_w; ++dx) {
            short sx = (short)(((int32_t)dx * src_w) / dst_w);
            drow[dx] = srow[sx];
        }
    }
}

/* --- OVL191.ASM: Maskierter Vollbild-Blit --- */
void blit_masked_screen(void *src_ptr) {
    if (!src_ptr || !g_vga_buf) return;
    const uint8_t *src = (const uint8_t *)src_ptr;
    uint8_t *dst = (uint8_t *)g_vga_buf;
    for (int i = 0; i < 64000; ++i) {
        if (src[i] != 0) dst[i] = src[i];
    }
    dsa_platform_present();
}

/* --- OVL270.ASM: FLI-Chunk-Entpacker --- */
void fli_wait_retrace(void) {
    dsa_platform_wait_vsync();
}

unsigned char *fli_normalize(unsigned char *p) {
    return p;
}

void fli_black(unsigned char *dst, unsigned short count) {
    if (!dst) return;
    if ((uintptr_t)dst == 0xa0000000u) dst = dsa_platform_get_vram();
    memset(dst, 0, (size_t)count * 2u);
}

void fli_copy(unsigned char *src, unsigned char *dst, unsigned short count) {
    if (!src || !dst) return;
    if ((uintptr_t)dst == 0xa0000000u) dst = dsa_platform_get_vram();
    memcpy(dst, src, (size_t)count * 2u);
}

void fli_color_pal(unsigned char *chunk_data, unsigned char *pal) {
    if (!chunk_data || !pal) return;
    const uint8_t *s = (const uint8_t *)chunk_data;
    uint16_t packets = *(const uint16_t *)s;
    s += 2;
    int idx = 0;
    for (uint16_t p = 0; p < packets; ++p) {
        idx += *s++;
        int cnt = *s++;
        if (cnt == 0) cnt = 256;
        for (int i = 0; i < cnt; ++i) {
            int c = (idx + i) & 0xff;
            pal[c * 3 + 0] = s[0];
            pal[c * 3 + 1] = s[1];
            pal[c * 3 + 2] = s[2];
            s += 3;
        }
        idx += cnt;
    }
}

void fli_color_dac(unsigned char *chunk_data) {
    if (!chunk_data) return;
    const uint8_t *s = (const uint8_t *)chunk_data;
    uint16_t packets = *(const uint16_t *)s;
    s += 2;
    int idx = 0;
    for (uint16_t p = 0; p < packets; ++p) {
        idx += *s++;
        int cnt = *s++;
        if (cnt == 0) cnt = 256;
        dsa_platform_set_palette(idx & 0xff, cnt, s);
        s += cnt * 3;
        idx += cnt;
    }
    dsa_platform_present();
}

void fli_lc(unsigned char *chunk_data, unsigned char *dst_ptr) {
    if (!chunk_data || !dst_ptr) return;
    if ((uintptr_t)dst_ptr == 0xa0000000u) dst_ptr = dsa_platform_get_vram();
    const uint8_t *s = (const uint8_t *)chunk_data;
    uint16_t start_line = *(const uint16_t *)s; s += 2;
    uint16_t num_lines  = *(const uint16_t *)s; s += 2;
    for (uint16_t l = 0; l < num_lines; ++l) {
        uint8_t *line_dst = (uint8_t *)dst_ptr + (start_line + l) * 320;
        uint8_t packets = *s++;
        int x = 0;
        for (uint8_t p = 0; p < packets; ++p) {
            x += *s++;
            int8_t cnt = (int8_t)*s++;
            if (cnt >= 0) {
                memcpy(line_dst + x, s, (size_t)cnt);
                s += cnt;
                x += cnt;
            } else {
                int rep = -cnt;
                uint8_t val = *s++;
                memset(line_dst + x, val, (size_t)rep);
                x += rep;
            }
        }
    }
}

void fli_brun(unsigned char *chunk_data, unsigned char *dst_ptr, short height) {
    if (!chunk_data || !dst_ptr) return;
    if ((uintptr_t)dst_ptr == 0xa0000000u) dst_ptr = dsa_platform_get_vram();
    const uint8_t *s = (const uint8_t *)chunk_data;
    for (short l = 0; l < height; ++l) {
        uint8_t *line_dst = (uint8_t *)dst_ptr + l * 320;
        uint8_t packets = *s++;
        int x = 0;
        for (uint8_t p = 0; p < packets; ++p) {
            int8_t cnt = (int8_t)*s++;
            if (cnt >= 0) {
                uint8_t val = *s++;
                if (x + cnt > 320) cnt = (int8_t)(320 - x);
                if (cnt > 0) memset(line_dst + x, val, (size_t)cnt);
                x += cnt;
            } else {
                int lit = -cnt;
                if (x + lit > 320) lit = 320 - x;
                if (lit > 0) memcpy(line_dst + x, s, (size_t)lit);
                s += -cnt;
                x += lit;
            }
        }
    }
}

/* =========================================================================
 * 5. Haupteinstiegspunkt (main) fuer den Windows-Port
 * ========================================================================= */

int dsa_game_main(int argc, char **argv);

#pragma pack(push, 4)
struct MinExceptionRecord {
    uint32_t ExceptionCode;
    uint32_t ExceptionFlags;
    void    *ExceptionRecord;
    void    *ExceptionAddress;
    uint32_t NumberParameters;
    uint32_t ExceptionInformation[15];
};
struct MinContext {
    uint32_t ContextFlags;
    uint32_t Dr0, Dr1, Dr2, Dr3, Dr6, Dr7;
    uint8_t  FloatSave[112];
    uint32_t SegGs, SegFs, SegEs, SegDs;
    uint32_t Edi, Esi, Ebx, Edx, Ecx, Eax;
    uint32_t Ebp, Eip, SegCs, EFlags, Esp, SegSs;
};
struct MinExceptionPointers {
    struct MinExceptionRecord *ExceptionRecord;
    struct MinContext         *ContextRecord;
};
#pragma pack(pop)

extern "C" void * __stdcall AddVectoredExceptionHandler(uint32_t first, long (__stdcall *handler)(struct MinExceptionPointers *));
extern "C" uint32_t __stdcall GetModuleFileNameA(void *hModule, char *lpFilename, uint32_t nSize);
extern "C" int __stdcall FreeConsole(void);
extern "C" void * __stdcall GetCurrentProcess(void);
extern "C" int __stdcall TerminateProcess(void *hProcess, uint32_t uExitCode);
extern "C" void * __stdcall CreateThread(void *lpThreadAttributes, size_t dwStackSize, uint32_t (__stdcall *lpStartAddress)(void *), void *lpParameter, uint32_t dwCreationFlags, uint32_t *lpThreadId);
extern "C" int __stdcall CloseHandle(void *hObject);
extern "C" void __stdcall Sleep(uint32_t dwMilliseconds);
extern "C" void __cdecl _exit(int status);

/* =========================================================================
 * W1 (Absturz-Logger), W2 (Stub-Waechter), W3 (Abdeckung)
 * ========================================================================= */
static char g_build_dir[1024]      = "Port\\build";
static char g_absturz_path[1024]   = "Port\\build\\absturz.log";
static char g_abspann_path[1024]   = "Port\\build\\abspann.log";

extern "C" void dsa_log_abspann(const char *msg) {
    if (!msg || !msg[0] || !crt_open_fn || !crt_write_fn || !crt_close_fn) return;
    int fd = crt_open_fn(g_abspann_path, _O_BINARY | _O_CREAT | _O_APPEND | _O_WRONLY, 0666);
    if (fd < 0) return;
    crt_write_fn(fd, msg, (unsigned int)strlen(msg));
    crt_write_fn(fd, "\n", 1);
    crt_close_fn(fd);
}
static char g_stubs_path[1024]     = "Port\\build\\stubs.log";
static char g_abdeckung_path[1024] = "Port\\build\\abdeckung.tsv";

static void *g_last_funcs[16];
static volatile uint32_t g_last_func_idx = 0;

/* W2: Stub-Waechter (schreibt jede Stub-Funktion beim ersten Aufruf nach Port/build/stubs.log) */
static const char *g_logged_stubs[256];
static int g_num_logged_stubs = 0;

void dsa_log_stub(const char *func_name) {
    if (!func_name) return;
    for (int i = 0; i < g_num_logged_stubs; ++i) {
        if (strcmp(g_logged_stubs[i], func_name) == 0) return;
    }
    if (g_num_logged_stubs < 256) {
        g_logged_stubs[g_num_logged_stubs++] = func_name;
    }
    init_crt_io();
    if (crt_open_fn && crt_write_fn && crt_close_fn) {
        int fd = crt_open_fn(g_stubs_path, _O_BINARY | _O_CREAT | _O_APPEND | _O_WRONLY, 0666);
        if (fd >= 0) {
            char line[256];
            int n = snprintf(line, sizeof(line), "STUB %s\n", func_name);
            crt_write_fn(fd, line, (unsigned int)n);
            crt_close_fn(fd);
        }
    }
}

/* W3: Funktions-Abdeckung (DSA_ABDECKUNG=1 -> schreibt beim ersten Aufruf nach Port/build/abdeckung.tsv) */
static int      g_cov_enabled = 0;
static int      g_cov_fd      = -1;
static uint8_t  g_cov_bitmap[0x300000 >> 3]; /* deckt 0x00400000..0x00700000 ab */
static uint32_t g_cov_order   = 0;

extern "C" __attribute__((no_instrument_function))
void __cyg_profile_func_enter(void *this_fn, void *call_site) {
    (void)call_site;
    uint32_t idx = g_last_func_idx++;
    g_last_funcs[idx & 15u] = this_fn;

    if (!g_cov_enabled) return;
    uint32_t addr = (uint32_t)(uintptr_t)this_fn;
    if (addr < 0x00400000u || addr >= 0x00700000u) return;
    uint32_t bit = addr - 0x00400000u;
    uint32_t byte_idx = bit >> 3;
    uint8_t  mask     = (uint8_t)(1u << (bit & 7u));
    if (!(g_cov_bitmap[byte_idx] & mask)) {
        g_cov_bitmap[byte_idx] |= mask;
        g_cov_order++;
        if (g_cov_fd >= 0 && crt_write_fn) {
            char buf[64];
            int n = snprintf(buf, sizeof(buf), "0x%08X\t%u\t%u\n",
                             addr, g_cov_order, dsa_platform_get_frame_count());
            crt_write_fn(g_cov_fd, buf, (unsigned int)n);
        }
    }
}

extern "C" __attribute__((no_instrument_function))
void __cyg_profile_func_exit(void *this_fn, void *call_site) {
    (void)this_fn;
    (void)call_site;
}

#include "dsa_cov_groups.inc"

extern "C" void f_0b88(void);
extern "C" void talk_nop(void);
extern "C" void dsa_effect_probe(void);
extern "C" void unused_scene_stub(void);
extern "C" void fli_timer_tick(void);
extern "C" void finale_timer_tick(void);
extern "C" void o045_stub_1(void);
signed short can_use_spellclass(struct hero *, signed short) __asm__("__Z18can_use_spellclassP4heros");
signed short has_herb_for_disease(struct hero *, signed short) __asm__("__Z20has_herb_for_diseaseP4heros");
void cd_drive_reset(signed short) __asm__("__Z14cd_drive_resets");

static void dsa_cov_group_run(int grp) {
    char buf[4096];
    memset(buf, 0, sizeof(buf));
    struct map_event mev0;
    memset(&mev0, 0, sizeof(mev0));
    signed short w = 0;
    signed long l = 0;
    signed char c0 = 0;
    signed char path_minus1[4] = { -1, -2, 0, 0 };

    if (grp == 1 || grp == 7) {
        if (!g_places) {
            g_places = (struct travel_place *)mem_alloc(file_length(g_file_main, 0xa1));
            ofile_load(g_ofile, 0xa1, g_places);
        }
        if (!g_routes) {
            g_routes = (struct route *)mem_alloc(file_length(g_file_main, 0xa2));
            ofile_load(g_ofile, 0xa2, g_routes);
        }
        Travel *tr = (Travel *)g_travel;
        tr->from = 0; tr->to = 0;
        place_event_039(tr);
        tevent_008(tr);
        tevent_024(tr);
        g_routes[37].event = 3; tevent_037(tr);
        tevent_051(tr);
        tevent_073(tr);
        unused_tevent_o118();
        tevent_081(tr);
        tevent_108(tr);
        tevent_134(tr);
        g_routes[141].event = 2; tevent_141(tr);
        unused_tevent_o122(tr);
        g_routes[157].event = 2; tevent_157(tr);
        g_routes[192].event = 2; tevent_192(tr);
        g_routes[238].event = 2; tevent_238(tr);
        tevent_246(tr);
        place_event_104(tr);
        place_event_nop(tr);
        flag_not(tr, 0);
        group_has_special_item();
        group_damage(tr, 1, 0, 120, 0, 0);
        place_has_event(tr, 1);
        fight_result(tr, 0);
        encounter_level(tr, 0);
        travel_nop_o213(tr);
        travel_resume(tr, 0, 0);
        travel_slow5_off(tr, 0);
        route_dest_by_dir(tr, 1, 0);
        route_is_known(tr, 1);
        debug_nop1();
        travel_reset_pos(tr);
        town_rnd_open(0, 0x40);
        town_rnd_close();
        dsa_cov_touch_group_1();
    }
    if (grp == 2 || grp == 7) {
        can_use_spellclass(g_heroes, 0);
        mon_spell_cost(1, 0);
        spell_class_first(0);
        hero_spell_cost(1, 0);
        dsa_cov_touch_group_2();
    }
    if (grp == 3 || grp == 7) {
        talk_nop();
        town03_handler(0, &mev0);
        f_0b88();
        miracle_group_effect(0, 1, 10, 0, 0);
        temple_is_open((Temple *)buf);
        groups_check_npc();
        shop_compar(buf, buf);
        tavern_leave((struct tavern *)buf);
        tavern_night_event((struct tavern *)buf);
        town_tjolmar_event(0, &mev0);
        conv_cb_low07(0, 0);
        dsa_cov_touch_group_3();
    }
    if (grp == 4 || grp == 7) {
        struct area_events *aev = (struct area_events *)buf;
        dng16_handler(aev, &mev0);
        dng16_seq_golem_cb(100, 10);
        dng10_handler(aev, &mev0);
        dng11_handler(aev, &mev0);
        dng13_handler(aev, &mev0);
        dng14_handler(aev, &mev0);
        dng02_fights(aev, &mev0);
        dng03_fights(aev, &mev0);
        dng05_fights(aev, &mev0);
        dng06_fights(aev, &mev0);
        blut_nop();
        dng07_handler(aev, &mev0);
        dng07_fights(aev, &mev0);
        dng08_handler(aev, &mev0);
        dng08_fights(aev, &mev0);
        cursor_for_cell(0);
        dsa_cov_touch_group_4();
    }
    if (grp == 5 || grp == 7) {
        get_skill_value(g_heroes, 0);
        sheet_equip_anim_id(0, 0, 0);
        preselect_hero(g_group, g_heroes);
        fill_saved_values(g_heroes, 0);
        calc_bp(g_heroes);
        dsa_effect_probe();
        can_hero_use_item(g_heroes, 1);
        hero_add_inv_f10(g_heroes, 0, 0);
        get_first_active_hero(g_group);
        has_herb_for_disease(g_heroes, 1);
        box_has(g_heroes, 0, 0);
        group_free_capacity(g_group);
        item_poison_type(0);
        dsa_cov_touch_group_5();
    }
    if (grp == 6 || grp == 7) {
        cd_drive_reset(0);
        unused_scene_stub();
        fight_nop();
        fli_timer_tick();
        dos_files_total();
        finale_timer_tick();
        o045_stub_1();
        grammar_pronoun_of(0, 1, 0);
        hero_is_inactive(g_heroes);
        set_asleep(g_heroes, 0);
        can_inc_skills(g_heroes);
        can_inc_spells(g_heroes);
        unused_view_stub();
        savemenu_cmp(buf, buf);
        o109_init_stub();
        fig_anim_reset((FigAnim *)buf);
        fig_count_dir_changes(0, path_minus1);
        fig_is_cell_89_8c(0);
        fig_obj_transparent(0, 0);
        g_fig_path[0] = -1;
        fig_move_pathlen(0);
        get_foe_attack_mode(0, 0, 0);
        manhattan_dist(0, 0, 1, 1);
        ki_best_target((unsigned char *)buf, 0);
        range_attack_check_ammo(0, g_heroes, 0);
        ((CharGen *)buf)->hero = g_heroes;
        chargen_calc_atpa((CharGen *)buf);
        swap16(&w);
        swap32(&l);
        bitrev16((unsigned short *)&w);
        sel_compar("a", "b");
        Compass_dtor(0, 0);
        cell_forget(0, 0);
        dsa_cov_touch_group_6();
    }
}

static void write_game_state_summary(int fd) {
    char buf[1024];
    int n = snprintf(buf, sizeof(buf),
        "Frame=%u AreaType=%d LocID=%d LoadedTown=%d LoadedDng=%d Daytime=%ld Date=%u.%u.%u\n",
        dsa_platform_get_frame_count(),
        (int)g_area_type, (int)g_loc_id, (int)g_loaded_town, (int)g_loaded_dungeon,
        (long)g_daytime, (unsigned)g_date_day, (unsigned)g_date_month, (unsigned)g_date_year);
    crt_write_fn(fd, buf, (unsigned int)n);

    if (g_group) {
        n = snprintf(buf, sizeof(buf),
            "Group: town=%d dungeon=%d loctype=0x%02X x=%d y=%d dir=%d nr_heroes=%d money=%ld\n",
            (int)g_group->town, (int)g_group->dungeon, (unsigned)(uint8_t)g_group->loctype,
            (int)g_group->x, (int)g_group->y, (int)g_group->dir,
            (int)g_group->nr_heroes, (long)g_group->money);
        crt_write_fn(fd, buf, (unsigned int)n);
    }
    if (g_heroes) {
        for (int i = 0; i < 7; ++i) {
            if (g_heroes[i].type > 0) {
                n = snprintf(buf, sizeof(buf),
                    "  Hero[%d]: name='%s' typ=%d lvl=%d LE=%d/%d AE=%d/%d money=%ld\n",
                    i, g_heroes[i].name, (int)g_heroes[i].type, (int)g_heroes[i].level,
                    (int)g_heroes[i].le, (int)g_heroes[i].le_max,
                    (int)g_heroes[i].ae, (int)g_heroes[i].ae_max,
                    (long)g_heroes[i].money);
                crt_write_fn(fd, buf, (unsigned int)n);
            }
        }
    }
    n = snprintf(buf, sizeof(buf),
        "FCTrace: %d %d %d %d %d %d %d %d %d %d\nLastFuncs:",
        g_func_trace[0], g_func_trace[1], g_func_trace[2], g_func_trace[3], g_func_trace[4],
        g_func_trace[5], g_func_trace[6], g_func_trace[7], g_func_trace[8], g_func_trace[9]);
    crt_write_fn(fd, buf, (unsigned int)n);
    uint32_t cur = g_last_func_idx;
    for (int k = 1; k <= 16; ++k) {
        void *fn = g_last_funcs[(cur - k) & 15u];
        if (fn) {
            n = snprintf(buf, sizeof(buf), " 0x%08X", (uint32_t)(uintptr_t)fn);
            crt_write_fn(fd, buf, (unsigned int)n);
        }
    }
    crt_write_fn(fd, "\n", 1);
}

void dsa_log_guru_meditation(const char *msg, const void *ptr) {
    init_crt_io();
    if (!crt_open_fn || !crt_write_fn || !crt_close_fn) return;
    int fd = crt_open_fn(g_absturz_path, _O_BINARY | _O_CREAT | _O_TRUNC | _O_WRONLY, 0666);
    if (fd >= 0) {
        char buf[512];
        int n = snprintf(buf, sizeof(buf), "[GURU MEDITATION] msg='%s' ptr=%p\n", msg ? msg : "", ptr);
        crt_write_fn(fd, buf, (unsigned int)n);
        write_game_state_summary(fd);
        crt_close_fn(fd);
    }
}

static long __stdcall schweif_exception_filter(struct MinExceptionPointers *ep) {
    if (!ep || !ep->ExceptionRecord || (ep->ExceptionRecord->ExceptionCode & 0xF0000000u) != 0xC0000000u) {
        return 0; /* EXCEPTION_CONTINUE_SEARCH */
    }
    init_crt_io();
    if (ep->ContextRecord && crt_open_fn && crt_write_fn && crt_close_fn) {
        static char cbuf[2048];
        int n = snprintf(cbuf, sizeof(cbuf),
            "[CRASH] Code=0x%08X EIP=0x%08X EAX=0x%08X EBX=0x%08X ECX=0x%08X EDX=0x%08X ESI=0x%08X EDI=0x%08X EBP=0x%08X ESP=0x%08X Info1=0x%08X\n",
            ep->ExceptionRecord->ExceptionCode,
            ep->ContextRecord->Eip,
            ep->ContextRecord->Eax, ep->ContextRecord->Ebx, ep->ContextRecord->Ecx, ep->ContextRecord->Edx,
            ep->ContextRecord->Esi, ep->ContextRecord->Edi, ep->ContextRecord->Ebp, ep->ContextRecord->Esp,
            ep->ExceptionRecord->ExceptionInformation[1]);

        const char *paths[2] = { g_absturz_path, "..\\CTEMP\\crash.log" };
        for (int p_i = 0; p_i < 2; ++p_i) {
            int fd = crt_open_fn(paths[p_i], _O_BINARY | _O_CREAT | _O_TRUNC | _O_WRONLY, 0666);
            if (fd >= 0) {
                crt_write_fn(fd, cbuf, (unsigned int)n);
                write_game_state_summary(fd);
                if (ep->ExceptionRecord->ExceptionCode != 0xC00000FDu) {
                    crt_write_fn(fd, "Stack:", 6);
                    uint32_t *sp = (uint32_t *)(uintptr_t)ep->ContextRecord->Esp;
                    for (int i = 0; i < 200; ++i) {
                        uint32_t v = sp[i];
                        if (v >= 0x00401000 && v <= 0x00650000) {
                            int m = snprintf(cbuf, sizeof(cbuf), " [+%d]=0x%08X", i * 4, v);
                            crt_write_fn(fd, cbuf, (unsigned int)m);
                        }
                    }
                    crt_write_fn(fd, "\n", 1);
                }
                crt_close_fn(fd);
            }
        }
    }
    _exit(77);
    return 0;
}

static void close_cov_file(void) {
    if (g_cov_fd >= 0 && crt_close_fn) {
        crt_close_fn(g_cov_fd);
        g_cov_fd = -1;
    }
}

static uint32_t __stdcall dsa_watchdog_thread_proc(void *param) {
    uint32_t timeout_ms = (uint32_t)(uintptr_t)param;
    Sleep(timeout_ms);
    close_cov_file();
    TerminateProcess(GetCurrentProcess(), 124);
    return 0;
}

static void copy_file_if_missing(const char *src_path, const char *dst_path) {
    if (!crt_open_fn || !crt_read_fn || !crt_write_fn || !crt_close_fn) return;
    int check_fd = crt_open_fn(dst_path, _O_RDONLY | _O_BINARY, 0666);
    if (check_fd >= 0) {
        crt_close_fn(check_fd);
        return;
    }
    int sfd = crt_open_fn(src_path, _O_RDONLY | _O_BINARY, 0666);
    if (sfd < 0) return;
    int dfd = crt_open_fn(dst_path, _O_WRONLY | _O_CREAT | _O_TRUNC | _O_BINARY, 0666);
    if (dfd >= 0) {
        char buf[8192];
        int n;
        while ((n = crt_read_fn(sfd, buf, sizeof(buf))) > 0) {
            crt_write_fn(dfd, buf, (unsigned int)n);
        }
        crt_close_fn(dfd);
    }
    crt_close_fn(sfd);
}

static void setup_save_dir(const char *gamedir) {
    init_crt_io();
    const char *env_save = getenv("DSA_SAVE_DIR");
    const char *env_ro   = getenv("DSA_FORCE_READONLY_GAMEDIR");
    int readonly = (env_ro && env_ro[0] == '1') ? 1 : 0;

    if (!readonly && (!env_save || !env_save[0]) && gamedir && gamedir[0]) {
        char test_path[1024];
        snprintf(test_path, sizeof(test_path), "%s\\.schweif_wtest.tmp", gamedir);
        int tfd = crt_open_fn(test_path, _O_WRONLY | _O_CREAT | _O_TRUNC | _O_BINARY, 0666);
        if (tfd >= 0) {
            crt_close_fn(tfd);
            crt_unlink_fn(test_path);
        } else {
            readonly = 1;
        }
    }

    if ((env_save && env_save[0]) || readonly) {
        if (env_save && env_save[0]) {
            snprintf(g_save_dir, sizeof(g_save_dir), "%s", env_save);
        } else {
            snprintf(g_save_dir, sizeof(g_save_dir), "%s\\Spielstaende", g_build_dir);
        }
        g_save_dir_redirected = 1;
        if (readonly) g_save_dir_notice_pending = 1;

        if (crt_mkdir_fn) {
            char sub[1024];
            crt_mkdir_fn(g_save_dir);
            snprintf(sub, sizeof(sub), "%s\\GAMES", g_save_dir);
            crt_mkdir_fn(sub);
            snprintf(sub, sizeof(sub), "%s\\TEMP", g_save_dir);
            crt_mkdir_fn(sub);
            snprintf(sub, sizeof(sub), "%s\\CTEMP", g_save_dir);
            crt_mkdir_fn(sub);
        }

        if (gamedir && gamedir[0]) {
            char pattern[1024];
            snprintf(pattern, sizeof(pattern), "%s\\GAMES\\*.*", gamedir);
            struct _finddata_t fdata;
            intptr_t h = _findfirst(pattern, &fdata);
            if (h != -1) {
                do {
                    if (!(fdata.attrib & 0x10) && strcmp(fdata.name, ".") != 0 && strcmp(fdata.name, "..") != 0) {
                        char sfile[1024], dfile[1024];
                        snprintf(sfile, sizeof(sfile), "%s\\GAMES\\%s", gamedir, fdata.name);
                        snprintf(dfile, sizeof(dfile), "%s\\GAMES\\%s", g_save_dir, fdata.name);
                        copy_file_if_missing(sfile, dfile);
                    }
                } while (_findnext(h, &fdata) == 0);
                _findclose(h);
            }
        }
    }
}

extern "C" int dsa_platform_should_play_intro(int argc, int first_start) {
    if (g_no_intro) return 0;
    const char *env_intro = getenv("DSA_INTRO");
    if (env_intro && env_intro[0] == '0') return 0;
    if (g_force_intro || (env_intro && env_intro[0] == '1')) return 1;
    if (g_has_replay) {
        return (first_start != 0 && argc <= 1) ? 1 : 0;
    }
    return (argc <= 1) ? 1 : 0;
}

extern "C" int dsa_platform_consume_readonly_notice(void) {
    if (!g_save_dir_notice_pending) return 0;
    g_save_dir_notice_pending = 0;
    const char *env_show = getenv("DSA_SHOW_READONLY_NOTICE");
    if (g_has_replay && !(env_show && env_show[0] == '1')) return 0;
    return 1;
}

int main(int argc, char **argv) {
    FreeConsole();
    init_crt_io();
    char exe_path[1024];
    uint32_t len = GetModuleFileNameA(NULL, exe_path, sizeof(exe_path));
    if (len > 0 && len < sizeof(exe_path)) {
        char *slash = strrchr(exe_path, '\\');
        if (!slash) slash = strrchr(exe_path, '/');
        if (slash) {
            *slash = '\0';
            snprintf(g_build_dir, sizeof(g_build_dir), "%s", exe_path);
            snprintf(g_absturz_path, sizeof(g_absturz_path), "%s\\absturz.log", g_build_dir);
            snprintf(g_abspann_path, sizeof(g_abspann_path), "%s\\abspann.log", g_build_dir);
            snprintf(g_stubs_path, sizeof(g_stubs_path), "%s\\stubs.log", g_build_dir);
            snprintf(g_abdeckung_path, sizeof(g_abdeckung_path), "%s\\abdeckung.tsv", g_build_dir);
        }
    }

    AddVectoredExceptionHandler(1, schweif_exception_filter);

    const char *cov_env = getenv("DSA_ABDECKUNG");
    if (cov_env && cov_env[0] == '1') {
        const char *trunc_env = getenv("DSA_ABDECKUNG_TRUNC");
        int cov_flags = _O_BINARY | _O_CREAT | _O_WRONLY | ((trunc_env && trunc_env[0] == '1') ? _O_TRUNC : _O_APPEND);
        g_cov_fd = crt_open_fn(g_abdeckung_path, cov_flags, 0666);
        if (g_cov_fd >= 0) {
            if (!crt_filelength_fn || crt_filelength_fn(g_cov_fd) <= 0) {
                const char *hdr = "Adresse\tReihenfolge\tFrame\n";
                crt_write_fn(g_cov_fd, hdr, (unsigned int)strlen(hdr));
            }
            g_cov_enabled = 1;
        }
    }

    static char *game_argv[32];
    int game_argc = 0;
    int has_replay = 0;
    game_argv[game_argc++] = argv[0];

    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--gamedir") == 0 && i + 1 < argc) {
            dsa_platform_set_gamedir(argv[++i]);
        } else if (strcmp(argv[i], "--record") == 0 && i + 1 < argc) {
            dsa_platform_set_record_file(argv[++i]);
        } else if (strcmp(argv[i], "--replay") == 0 && i + 1 < argc) {
            dsa_platform_set_replay_file(argv[++i]);
            has_replay = 1;
            g_has_replay = 1;
        } else if (strcmp(argv[i], "--kein-intro") == 0 || strcmp(argv[i], "--no-intro") == 0) {
            g_no_intro = 1;
        } else if (strcmp(argv[i], "--intro") == 0) {
            g_force_intro = 1;
        } else if (game_argc < 31) {
            game_argv[game_argc++] = argv[i];
        }
    }
    game_argv[game_argc] = NULL;

    {
        const char *wd_env = getenv("DSA_WATCHDOG_MS");
        uint32_t wd_ms = (wd_env && wd_env[0]) ? (uint32_t)atoi(wd_env) : (has_replay ? 25000u : 0u);
        if (wd_ms > 0) {
            void *hThread = CreateThread(NULL, 0, dsa_watchdog_thread_proc, (void *)(uintptr_t)wd_ms, 0, NULL);
            if (hThread) CloseHandle(hThread);
        }
    }

    if (dsa_platform_init("DSA - Sternenschweif", 3) != 0) {
        return 1;
    }

    const char *gamedir = dsa_platform_get_gamedir();
    iso_init_images(gamedir);
    setup_save_dir(gamedir);
    _chdir(gamedir);
    if (!g_save_dir_redirected) {
        _mkdir("CTEMP");
        _mkdir("TEMP");
        _mkdir("GAMES");
    }

    g_vga_buf = dsa_platform_get_vram();
    g_fli_screen.scr = dsa_platform_get_vram();
    g_group = g_groups;
    dsa_platform_set_tick_hook(schweif_platform_tick);

    globals_ctor();
    atexit(globals_dtor);
    atexit(close_cov_file);
    atexit(dsa_platform_shutdown);

    return dsa_game_main(game_argc, game_argv);
}

} /* extern "C" */
