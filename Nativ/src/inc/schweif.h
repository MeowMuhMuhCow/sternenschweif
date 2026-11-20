/* schweif.h - gemeinsamer Header fuer die C/C++-Rekonstruktion von DSA 2 Sternenschweif (Attic 1994)
 *
 * Einbinden mit  #include "schweif.h"  und  BCC ... -I..\names\inc  (BCC 3.1, -mlarge, C und C++).
 * Der Header erzeugt selbst keinen Code und keine Daten: nur Typen, Konstanten, extern-Deklarationen,
 * Prototypen und (C++) inline-Elementfunktionen, die erst bei Benutzung expandiert werden.
 * Standard-Header (stdio.h, string.h ...) bindet jedes Modul selbst ein - sie entscheiden ueber
 * Intrinsics (-Oi) und inline-Varianten (atoi, abs) und gehoeren daher zum Modul.
 *
 *   s_const.h   Konstanten und enums (Eigenschaften, Talente, Zauber, Krankheiten, Gifte ...)
 *   s_struct.h  Datenstrukturen (hero, group, item_desc, inv_slot, enemy ...) mit inline-Settern
 *   s_class.h   C++-Klassen (Bmp ...)
 *   s_data.h    globale Daten (erzeugt aus names/daten/manuell_data.tsv)
 *   s_proto.h   Prototypen (erzeugt aus names/namen.tsv + Deklarationen der Module)
 *   s_fix.h     Schalter fuer die Behebung der Originalfehler (FIX_BUGS, FIX_EXPLOITS, FIX_nnnn; nur Makros)
 *
 * Konventionen: names/KONVENTIONEN.md. Namen und Herkunft: names/namen.tsv. Abweichungen: konflikte.tsv.
 */
#ifndef SCHWEIF_H
#define SCHWEIF_H
#ifndef PORT_SKIP_COPYPROT
#define PORT_SKIP_COPYPROT 1
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
#ifndef cdecl
#define cdecl
#endif
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#ifndef __STRING_H
#define __STRING_H 1
#endif
#ifndef __abs__
#define __abs__(x) ((x) < 0 ? -(x) : (x))
#endif
#ifndef __rotl__
#define __rotl__(v, c) ((unsigned short)((((unsigned short)(v)) << (c)) | (((unsigned short)(v)) >> (16 - (c)))))
#endif
#ifndef _OvrInitExt
#define _OvrInitExt(a, b) 0
#endif
#ifndef O_BINARY
#define O_BINARY 0
#endif
#ifdef __cplusplus
extern "C" {
#endif
void *dsa_mk_fp(unsigned short seg, unsigned long off);
void dsa_log_stub(const char *func_name);
void dsa_log_guru_meditation(const char *msg, const void *ptr);
uint32_t dsa_platform_init_seed(uint32_t default_seed);
extern unsigned char *g_port_shade_base;
#ifndef _WIN32
char *itoa(int value, char *str, int base);
char *ltoa(long value, char *str, int base);
char *ultoa(unsigned long value, char *str, int base);
char *strupr(char *s);
char *strlwr(char *s);
int stricmp(const char *a, const char *b);
int strnicmp(const char *a, const char *b, size_t n);
#define _stricmp stricmp
#define _strnicmp strnicmp
unsigned int _rotl(unsigned int val, int shift);
unsigned int _rotr(unsigned int val, int shift);
unsigned long _lrotl(unsigned long val, int shift);
unsigned long _lrotr(unsigned long val, int shift);
#endif
#ifdef __cplusplus
}
#endif
#ifndef MK_FP
#define MK_FP(seg, off) dsa_mk_fp((unsigned short)(seg), (unsigned long)(off))
#endif
#ifndef FP_SEG
#define FP_SEG(p) ((unsigned short)(((uintptr_t)(p)) >> 16))
#endif
#ifndef FP_OFF
#define FP_OFF(p) ((unsigned short)(((uintptr_t)(p)) & 0xFFFFu))
#endif

#include "s_fix.h"
#include "s_const.h"
#include "s_struct.h"
#include "s_class.h"
#include "s_data.h"
#include "s_proto.h"

#ifdef __cplusplus
extern "C" {
#endif
extern unsigned char g_schweif_dgroup[0x8b06];
#ifdef __cplusplus
}
#endif
#ifndef DS_STR
#define DS_STR(off) ((char far *)&g_schweif_dgroup[(off)])
#endif

#endif
