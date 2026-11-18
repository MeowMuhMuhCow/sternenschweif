#ifndef DSA_COMPAT_DOS_H
#define DSA_COMPAT_DOS_H

#include_next <dos.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef far
#define far
#endif
#ifndef huge
#define huge
#endif
#ifndef near
#define near
#endif
#ifndef interrupt
#define interrupt
#endif

void *dsa_mk_fp(unsigned short seg, unsigned long off);

#undef MK_FP
#undef FP_SEG
#undef FP_OFF
#define MK_FP(seg, off) dsa_mk_fp((unsigned short)(seg), (unsigned long)(off))
#define FP_SEG(p)       ((unsigned short)(((uintptr_t)(p)) >> 16))
#define FP_OFF(p)       ((unsigned short)(((uintptr_t)(p)) & 0xFFFFu))

struct WORDREGS {
    unsigned short ax, bx, cx, dx, si, di, cflag, flags;
};

struct BYTEREGS {
    unsigned char al, ah, bl, bh, cl, ch, dl, dh;
};

union REGS {
    struct WORDREGS x;
    struct BYTEREGS h;
};

struct SREGS {
    unsigned short es, cs, ss, ds;
};

struct dfree {
    unsigned df_avail;
    unsigned df_total;
    unsigned df_bsec;
    unsigned df_sclus;
};

extern unsigned short _psp;

int  dsa_int86(int intno, union REGS *inregs, union REGS *outregs);
int  dsa_int86x(int intno, union REGS *inregs, union REGS *outregs, struct SREGS *segregs);
int  dsa_intdos(union REGS *inregs, union REGS *outregs);
void dsa_getdfree(unsigned char drive, struct dfree *dtable);
void dsa_delay(unsigned ms);

#define int86(n, i, o)     dsa_int86(n, i, o)
#define int86x(n, i, o, s) dsa_int86x(n, i, o, s)
#define intdos(i, o)       dsa_intdos(i, o)
#define getdfree(d, t)     dsa_getdfree(d, t)
#define delay(ms)          dsa_delay(ms)

static inline void sound(unsigned freq) { (void)freq; }
static inline void nosound(void) {}
static inline void geninterrupt(int intr_num) { (void)intr_num; }
static inline unsigned char inportb(unsigned short port) { (void)port; return 0; }
static inline void outportb(unsigned short port, unsigned char val) { (void)port; (void)val; }

typedef void (*dsa_isr_t)(void);
static inline void dsa_noop_isr(void) {}
static inline dsa_isr_t getvect(int intr_num) { (void)intr_num; return dsa_noop_isr; }
static inline void setvect(int intr_num, dsa_isr_t isr) { (void)intr_num; (void)isr; }
static inline void ctrlbrk(int (*handler)(void)) { (void)handler; }

#ifdef __cplusplus
}
#endif

#endif /* DSA_COMPAT_DOS_H */
