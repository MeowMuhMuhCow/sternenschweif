#ifndef DSA_COMPAT_CONIO_H
#define DSA_COMPAT_CONIO_H

#ifdef _WIN32
#include_next <conio.h>
#else
#include <stdio.h>
#include <stdarg.h>
#endif

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
#ifndef _WIN32
static inline int cputs(const char *s) { return s ? fputs(s, stdout) : 0; }
static inline int cprintf(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    int r = vprintf(fmt, ap);
    va_end(ap);
    return r;
}
static inline int getch(void) { return 0; }
static inline int getche(void) { return 0; }
static inline int kbhit(void) { return 0; }
static inline int putch(int c) { return putchar(c); }
#endif

#ifdef __cplusplus
}
#endif

#endif /* DSA_COMPAT_CONIO_H */
