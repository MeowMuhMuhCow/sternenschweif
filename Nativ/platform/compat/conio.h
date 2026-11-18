#ifndef DSA_COMPAT_CONIO_H
#define DSA_COMPAT_CONIO_H

#include_next <conio.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef BLINK
#define BLINK     128
#define LIGHTRED  12
#define LIGHTGRAY 7
#endif

static inline void clrscr(void) {}
static inline void gotoxy(int x, int y) { (void)x; (void)y; }
static inline void textcolor(int c) { (void)c; }
static inline void textbackground(int c) { (void)c; }

#ifdef __cplusplus
}
#endif

#endif /* DSA_COMPAT_CONIO_H */
