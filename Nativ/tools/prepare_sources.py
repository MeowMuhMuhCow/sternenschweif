#!/usr/bin/env python3
"""
prepare_sources.py - Kopiert Quelle/ nach Port/src/ und passt Borland-C++-3.1-Spezifika
fuer die 32-Bit-MinGW-GCC-Kompilierung (-m32 -fpack-struct=1 -mabi=sysv) an.

WICHTIG: Quelle/ wird niemals veraendert! Alle Schalter (FIX_BUGS, FIX_EXPLOITS,
FIX_nnnn, FIX_PROBENLOG, PORT_SKIP_COPYPROT) bleiben voll erhalten.
"""
import os
import re
import glob
import shutil

PORT_DIR = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
ROOT = os.path.dirname(PORT_DIR)
for _cand in [ROOT, os.path.dirname(ROOT)] + sorted(glob.glob(os.path.join(PORT_DIR, "..", "..", "..", "*"))):
    if os.path.exists(os.path.join(_cand, "Quelle", "inc", "schweif.h")):
        ROOT = os.path.abspath(_cand)
        break
QUELLE = os.path.join(ROOT, "Quelle")
PORT_SRC = os.path.join(PORT_DIR, "src")


def read_latin1(path):
    with open(path, "r", encoding="latin1", newline="") as f:
        return f.read()


def write_latin1(path, text):
    os.makedirs(os.path.dirname(path), exist_ok=True)
    new_text = text.replace("\r\n", "\n")
    if os.path.exists(path):
        with open(path, "r", encoding="latin1", newline="") as f:
            if f.read() == new_text:
                return
    with open(path, "w", encoding="latin1", newline="\n") as f:
        f.write(new_text)


def transform_schweif_h(txt):
    if "DS_STR(off)" not in txt:
        idx_end = txt.rfind("#endif")
        if idx_end != -1:
            txt = txt[:idx_end] + '#ifdef __cplusplus\nextern "C" {\n#endif\nextern unsigned char g_schweif_dgroup[0x8b06];\n#ifdef __cplusplus\n}\n#endif\n#ifndef DS_STR\n#define DS_STR(off) ((char far *)&g_schweif_dgroup[(off)])\n#endif\n\n' + txt[idx_end:]

    preamble = """#ifndef PORT_SKIP_COPYPROT
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
#ifndef _WIN32
#include <io.h>
#endif
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
void vga_set_color(void *rgb, short idx);
void vga_set_palette(void *pal, short first, unsigned short count);
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
"""
    return txt.replace('#define SCHWEIF_H\n', '#define SCHWEIF_H\n' + preamble, 1)


def transform_s_struct_h(txt):
    # Borland C++ 3.1 packt unsigned short b:1 in 1 Byte; GCC braucht unsigned char b:1
    txt = re.sub(r"unsigned short(\s+\w+\s*:\s*\d+\s*;)", r"unsigned char\1", txt)
    return txt


def transform_s_data_h(txt):
    txt = txt.replace("#define S_DATA_H\n", '#define S_DATA_H\n#ifdef __cplusplus\nextern "C" {\n#endif\n', 1)
    idx = txt.rfind("#endif")
    if idx != -1:
        txt = txt[:idx] + '#ifdef __cplusplus\n}\n#endif\n' + txt[idx:]
    return txt


def transform_s_proto_h(txt):
    # Doppelte Deklaration void interrupt timer_isr(...); entfernen
    txt = txt.replace("void interrupt timer_isr(...);\n", "")
    txt = txt.replace("#define S_PROTO_H\n", '#define S_PROTO_H\n#ifdef __cplusplus\nextern "C" {\n#endif\n', 1)
    idx = txt.rfind("#endif")
    if idx != -1:
        txt = txt[:idx] + '#ifdef __cplusplus\n}\n#endif\n' + txt[idx:]
    return txt


def replace_func_body(txt, func_header_regex, new_body):
    m = re.search(func_header_regex, txt, flags=re.M)
    if not m:
        raise RuntimeError(f"Function not found: {func_header_regex}")
    # find opening brace
    brace_start = txt.find("{", m.end() - 1)
    depth = 0
    i = brace_start
    while i < len(txt):
        if txt[i] == "{":
            depth += 1
        elif txt[i] == "}":
            depth -= 1
            if depth == 0:
                return txt[:brace_start] + "{\n" + new_body + "\n}" + txt[i + 1:]
        i += 1
    raise RuntimeError(f"Unclosed brace for {func_header_regex}")


def transform_source(rel_path, txt):
    txt = txt.replace("\r\n", "\n")
    base = os.path.basename(rel_path)

    # 1. SEG001.CPP: COPYPROT_MARK + dsa_platform_poll_events in game_idle
    if base == "SEG001.CPP":
        txt = re.sub(r"#define\s+COPYPROT_MARK\(\)\s+asm\s+db[^\n]*", "#define COPYPROT_MARK() ((void)0)", txt)
        txt = txt.replace(
            "void game_idle(void)\n{",
            'extern "C" int dsa_platform_poll_events(void);\nvoid game_idle(void)\n{\n\tdsa_platform_poll_events();'
        )

    # 2. SEG002.C: EMS_map + set_palette, set_color
    elif base == "SEG002.C":
        txt = replace_func_body(txt, r"^signed short EMS_map\([^)]*\)\s*\{", "    (void)phys; (void)log; (void)handle;\n    return 0;")
        set_pal_c = """    void vga_set_palette(void *pal, short first, unsigned short count); /* Prototyp: ohne ihn fehlerhafte Signatur in WebAssembly (Absturz) */
    signed short i;
    for (i = 0; i < count; i++) {
        PAL[i * 3] &= 0x3f;
        (PAL + i * 3)[1] &= 0x3f;
        (PAL + i * 3)[2] &= 0x3f;
    }
    vga_set_palette(pal, first, count);
    if (g_palette && g_palette != pal) {
        memcpy(g_palette + first * 3, pal, count * 3);
    }"""
        set_col_c = """    void vga_set_color(void *rgb, short idx); /* Prototyp, siehe set_palette */
    RGB[0] &= 0x3f;
    RGB[1] &= 0x3f;
    RGB[2] &= 0x3f;
    vga_set_color(rgb, nr);
    if (g_palette) {
        memcpy(g_palette + nr * 3, rgb, 3);
    }"""
        txt = replace_func_body(txt, r"^void set_palette\([^)]*\)\s*\{", set_pal_c)
        txt = replace_func_body(txt, r"^void set_color\([^)]*\)\s*\{", set_col_c)

    # 2a. OVL075.CPP: drag_hero_portrait Polling in while (g_mouse_ldown) + Untergrenze dst >= 0 (Fix #62 auch fuer ORIG/WICHTIG)
    elif base == "OVL075.CPP":
        txt = txt.replace(
            """\t\twhile (g_mouse_ldown) {
\t\t\tif (g_mouse_posx != g_mouse_posx_bak) {
\t\t\t\twait_for_vsync();
\t\t\t\tunder->put(g_vga_buf, g_mouse_posx_bak, 0x9d);
\t\t\t\tg_mouse_busy = 1;
\t\t\t\tunder->get(g_vga_buf, g_mouse_posx, 0x9d);
\t\t\t\tportrait->put(g_vga_buf, g_mouse_posx, 0x9d);
\t\t\t\tg_mouse_busy = 0;
\t\t\t\tg_mouse_posx_bak = g_mouse_posx;
\t\t\t}
\t\t}""",
            """\t\twhile (g_mouse_ldown) {
\t\t\tif (g_mouse_posx != g_mouse_posx_bak) {
\t\t\t\twait_for_vsync();
\t\t\t\tunder->put(g_vga_buf, g_mouse_posx_bak, 0x9d);
\t\t\t\tg_mouse_busy = 1;
\t\t\t\tunder->get(g_vga_buf, g_mouse_posx, 0x9d);
\t\t\t\tportrait->put(g_vga_buf, g_mouse_posx, 0x9d);
\t\t\t\tg_mouse_busy = 0;
\t\t\t\tg_mouse_posx_bak = g_mouse_posx;
\t\t\t} else {
\t\t\t\twait_for_vsync();
\t\t\t}
\t\t}""",
        )
        txt = txt.replace(
            """#if FIX_0062
\t\t/* Fix #62: Untergrenze: nicht vor g_portrait_x[0] lesen */
\t\tfor (dst = 6; dst > 0 && g_portrait_x[--dst] > g_mouse_posx + 25; )
\t\t\t;
#else
\t\tfor (dst = 6; g_portrait_x[--dst] > g_mouse_posx + 25; )
\t\t\t;
#endif""",
            """#if FIX_0062 || !defined(__BORLANDC__)
\t\t/* Fix #62 / nativ: Untergrenze: nicht vor g_portrait_x[0] lesen */
\t\tfor (dst = 6; dst > 0 && g_portrait_x[--dst] > g_mouse_posx + 25; )
\t\t\t;
#else
\t\tfor (dst = 6; g_portrait_x[--dst] > g_mouse_posx + 25; )
\t\t\t;
#endif""",
        )

    # 2b. SEG045.C: wait_for_vsync + lvalue cast in do_h_line/do_v_line
    elif base == "SEG045.C":
        txt = replace_func_body(txt, r"^void wait_for_vsync\([^)]*\)\s*\{", "    extern void dsa_platform_wait_vsync(void);\n    dsa_platform_wait_vsync();")
        txt = txt.replace("(unsigned char far *)dst += y * SCREEN_W + x;", "dst = (unsigned char far *)dst + y * SCREEN_W + x;")

    # 3. SEG003.CPP: cpu_v86_active + CopyProt skip switch
    elif base == "SEG003.CPP":
        txt = replace_func_body(txt, r"^signed short cpu_v86_active\([^)]*\)\s*\{", "    return 0;")
        txt = txt.replace(
            "void copyprot_check(void)\n{",
            "void copyprot_check(void)\n{\n#if PORT_SKIP_COPYPROT\n\treturn;\n#endif"
        )
        txt = txt.replace(
            "void CopyProt::query(signed short chance)\n{",
            "void CopyProt::query(signed short chance)\n{\n#if PORT_SKIP_COPYPROT\n\t(void)chance;\n\tok = 1;\n\tok2 = 16;\n\tg_copyprot_passed = 1;\n\treturn;\n#endif"
        )
        txt = txt.replace(
            "void CopyProt::init()\n{",
            "void CopyProt::init()\n{\n#if PORT_SKIP_COPYPROT\n\tok = 1;\n\tok2 = 16;\n\tg_copyprot_passed = 1;\n\treturn;\n#endif"
        )

    # 4. SEG005.CPP: cd_check, _OvrInitExt
    elif base == "SEG005.CPP":
        txt = txt.replace("\t\tasm mov bx, _psp\n\t\tasm xor ax, ax\n\t\tasm push bx\n\t\tasm push ax\n\t\tasm retf", "\t\texit(1);")
        txt = txt.replace("_OvrInitExt(0, 0);", "((void)0);")
        txt = txt.replace(
            "signed short cd_check(void)\n{",
            "signed short cd_check(void)\n{\n#if PORT_SKIP_COPYPROT\n\treturn 1;\n#endif"
        )

    # 4b. SEG007.C: 16-Bit-Offset-Tricks ((unsigned short *)&p)[0] -= 4 und ((unsigned short *)&q)[0]++ / += 2
    elif base == "SEG007.C":
        txt = txt.replace(
            "\t\t\t((unsigned short *)&p)[0] -= 4;\t/* p -= 4 (nur Offset) */",
            "#if !defined(__BORLANDC__)\n\t\t\tp = (unsigned char far *)p - 4;\n#else\n\t\t\t((unsigned short *)&p)[0] -= 4;\t/* p -= 4 (nur Offset) */\n#endif"
        )
        txt = txt.replace(
            "\t\t\t\t\t\t((unsigned short*)&q)[0]++;",
            "#if !defined(__BORLANDC__)\n\t\t\t\t\t\tq++;\n#else\n\t\t\t\t\t\t((unsigned short*)&q)[0]++;\n#endif"
        )
        txt = txt.replace(
            "\t\t\t\t\t\t((unsigned short*)&q)[0] += 2;",
            "#if !defined(__BORLANDC__)\n\t\t\t\t\t\tq += 2;\n#else\n\t\t\t\t\t\t((unsigned short*)&q)[0] += 2;\n#endif"
        )

    # 4c. SEG012.CPP: draw_stars 16-Bit-Offset off = q.x + q.y * 320 (>= 32768 darf nicht negativ vorzeichenerweitert werden)
    elif base == "SEG012.CPP":
        txt = txt.replace(
            "\tregister signed short off, i;",
            "\tregister unsigned short off;\n\tregister signed short i;"
        )
        txt = txt.replace(
            "\tcopy_3d_to_screen();\n\tif (m)\n\t\tmouse_show(0);",
            "\tcopy_3d_to_screen();\n\tif (m)\n\t\tmouse_show(0);\n\twait_for_vsync();"
        )

    # 5. SEG014.C: guru_meditation asm + W1 Absturz-Logger
    elif base == "SEG014.C":
        txt = txt.replace("mouse_hide(0);", "dsa_log_guru_meditation(msg, ptr);\n\tmouse_hide(0);", 1)
        txt = re.sub(r"\s*asm mov ah, 4bh\s+asm int 67h\s+asm mov ems_handles, bx", "\n\t\tems_handles = 0;", txt)
        txt = re.sub(r"\s*asm nop\s+asm int 3", "\n\texit(1);", txt)

    # 6. SEG016.C: decomp_rle
    elif base == "SEG016.C":
        rle_c = """    (void)nvf;
    signed short y, x;
    const unsigned char *s = (const unsigned char *)src;
    for (y = 0; y < height; y++) {
        unsigned char *d = (unsigned char *)dst;
        x = width;
        while (x > 0) {
            unsigned char al = *s++;
            if (al == 0x7f) {
                unsigned char cnt = *s++;
                unsigned char val = *s++;
                for (unsigned char k = 0; k < cnt; k++)
                    *d++ = val;
                x -= cnt;
            } else {
                *d++ = al;
                x--;
            }
        }
        dst = (unsigned char *)dst + width;
    }"""
        txt = replace_func_body(txt, r"^void decomp_rle\([^)]*\)\s*\{", rle_c)

    # 7. SEG018.C: keyboard_isr + lvalue cast ((struct mouse_area far *)areas)++
    elif base == "SEG018.C":
        txt = replace_func_body(txt, r"^void interrupt keyboard_isr\([^)]*\)\s*\{", "    /* Im SDL2-Port ueber dsa_platform_poll_events behandelt */")
        txt = txt.replace("((struct mouse_area far *)areas)++;", "areas = (struct mouse_area far *)areas + 1;")

    # 8. SEG023.C: PUSH_E32, POP_E32, mouse_isr, fight_timer_cb copyprot, long_sqrt
    elif base == "SEG023.C":
        txt = re.sub(r"#define\s+PUSH_E32\(\)[^\n]*", "#define PUSH_E32() ((void)0)", txt)
        txt = re.sub(r"#define\s+POP_E32\(\)[^\n]*", "#define POP_E32() ((void)0)", txt)
        txt = replace_func_body(txt, r"^void interrupt mouse_isr\([^)]*\)\s*\{", "    /* Im SDL2-Port ueber dsa_platform_poll_events behandelt */")
        txt = txt.replace(
            "void far fight_timer_cb(void)\n{",
            "void far fight_timer_cb(void)\n{\n#if PORT_SKIP_COPYPROT\n\treturn;\n#endif"
        )
        sqrt_c = """    unsigned long res = 0;
    unsigned long odd = 1;
    long rem = n;
    while ((rem -= (long)odd) >= 0) {
        res++;
        odd += 2;
    }
    return (signed long)res;"""
        txt = replace_func_body(txt, r"^signed long long_sqrt\([^)]*\)\s*\{", sqrt_c)

    # 8b. SEG032.C: *WORDS++ / *BYTES++ lvalue casts
    elif base == "SEG032.C":
        txt = txt.replace("if (*WORDS++ == val)", "if (*WORDS == val) return i;\n\t\tlist = (signed short far *)list + 1;\n\t\tif (0)")
        txt = txt.replace("if (*BYTES++ == val)", "if (*BYTES == val) return i;\n\t\tlist = (signed char far *)list + 1;\n\t\tif (0)")

    # 8c. SEG042.C: gb_vline K&R parameter color (short in s_proto.h)
    elif base == "SEG042.C":
        txt = txt.replace("signed char color;\n{", "signed short color;\n{")

    # 9. SEG028.C: swap_u16
    elif base == "SEG028.C":
        txt = replace_func_body(
            txt,
            r"^unsigned short swap_u16\([^)]*\)\s*\{",
            "    return (unsigned short)(((value & 0xffu) << 8) | ((value >> 8) & 0xffu));"
        )

    # 10. OVL004.CPP: Virtuelles MSCDEX-CD-Laufwerk (kein physischer CD-Zwang, automatischer CD1/CD2-Wechsel)
    elif base == "OVL004.CPP":
        txt = replace_func_body(txt, r"^void cd_drive_reset\([^)]*\)\s*\{", "    (void)drv;")
        txt = replace_func_body(txt, r"^signed short cd_get_label\([^)]*\)\s*\{", '    (void)drv; if (label) strcpy(label, "DSA2_CD1   "); return 0;')
        txt = replace_func_body(txt, r"^signed short cd_check_drive\([^)]*\)\s*\{", "    (void)drv; return 1;")
        txt = replace_func_body(txt, r"^signed short CD::ndrives\([^)]*\)\s*\{", "    return 1;")
        txt = replace_func_body(txt, r"^unsigned short CD::version\([^)]*\)\s*\{", "    return 0x0210;")
        txt = replace_func_body(txt, r"^unsigned short CD::firstdrive\([^)]*\)\s*\{", "    return 3;")
        txt = replace_func_body(txt, r"^void CD::request\([^)]*\)\s*\{", "    if (rq) rq->status = 0;")
        txt = replace_func_body(txt, r"^void CD::status\([^)]*\)\s*\{", "    open = 0; busy = 0; paused = 0;")
        txt = replace_func_body(txt, r"^void CD::read_disc\([^)]*\)\s*\{", "    err = 0; if (disc == 0) disc = 1; if (di) { di->first = 1; di->last = 25; }")
        txt = txt.replace(
            "\tif (mu[n].track != track || mu[n].disc != 0 && mu[n].disc != disc) {",
            "\tif (mu[n].disc != 0) disc = mu[n].disc;\n\tif (mu[n].track != track || mu[n].disc != 0 && mu[n].disc != disc) {"
        )

    # 11. OVL010.CPP: g_shade_segs + ems_*
    elif base == "OVL010.CPP":
        txt = txt.replace(
            "\tp += 0x10 - FP_OFF(p);\n\tfor (i = 0; i < 32; i++)\n\t\tg_shade_segs[i] = FP_SEG(p) + (i << 4);",
            "\tp = (unsigned char *)g_shade_buf + 8;\n\tg_port_shade_base = (unsigned char *)p;\n\tfor (i = 0; i < 32; i++)\n\t\tg_shade_segs[i] = (unsigned short)(0x9000 + i);"
        )
        txt = replace_func_body(txt, r"^signed short ems_check\([^)]*\)\s*\{", "    return 1; /* Kein EMS -> XMS-Pfad nutzen */")
        txt = replace_func_body(txt, r"^signed short ems_frame_seg\([^)]*\)\s*\{", "    return (signed short)0x8000;")
        txt = replace_func_body(txt, r"^signed short ems_free_pages\([^)]*\)\s*\{", "    return 0;")
        txt = replace_func_body(txt, r"^signed short ems_alloc\([^)]*\)\s*\{", "    (void)n; if (h) *(signed short *)h = 0; return 1;")
        txt = replace_func_body(txt, r"^signed short ems_free\([^)]*\)\s*\{", "    (void)h; return 0;")

    # 12. OVL011.CPP: _AH / _BX / _SP / dos_open_files
    elif base == "OVL011.CPP":
        txt = txt.replace("\t\t\t_AH = 0x4b;\n\t\t\tgeninterrupt(0x67);\n\t\t\tv = _BX;", "\t\t\tv = 0;")
        txt = txt.replace("\t\tspv = _SP;", "\t\tspv = 0;")
        txt = replace_func_body(txt, r"^signed short dos_open_files\([^)]*\)\s*\{", "    return 5;")

    # 12b. OVL024.C: Vorspann-FLI auch per Leertaste und Linksklick abbrechbar (B1)
    elif base == "OVL024.C":
        txt = txt.replace(
            "\tif (g_fli_key == KEY_ESC || g_fli_key == KEY_CTRL_Q) {\n\t\treturn 1;\n\t}\n\tif (g_mouse_rclick) {\n\t\tg_mouse_rclick = 0;\n\t\tg_fli_key = KEY_ESC;\n\t\treturn 1;\n\t}",
            "\tif (g_fli_key == KEY_ESC || g_fli_key == KEY_CTRL_Q) {\n\t\treturn 1;\n\t}\n\tif ((g_fli_key & 0xff) == ' ') {\n\t\tg_fli_key = KEY_ESC;\n\t\treturn 1;\n\t}\n\tif (g_mouse_rclick || g_mouse_lclick) {\n\t\tg_mouse_rclick = 0;\n\t\tg_mouse_lclick = 0;\n\t\tg_fli_key = KEY_ESC;\n\t\treturn 1;\n\t}"
        )

    # 13. OVL027.CPP: Opening::copy_line + Opening::poll_key Mausklick-Abbruch (B1)
    elif base == "OVL027.CPP":
        txt = replace_func_body(txt, r"^void Opening::copy_line\([^)]*\)\s*\{", "    memcpy(dst, src, 320);")
        txt = txt.replace(
            "\tif (bioskey(1)) {",
            "\tif (g_mouse_lclick || g_mouse_rclick) {\n\t\tg_mouse_lclick = 0;\n\t\tg_mouse_rclick = 0;\n\t\tdone = 1;\n\t\tkey = KEY_ESC;\n\t} else if (bioskey(1)) {"
        )

    # 14. OVL032.CPP: main -> dsa_game_main, dos_files_total, W4 Replay-Seed, B1 Intro
    elif base == "OVL032.CPP":
        txt = txt.replace(
            "void main(register int argc, char **argv)",
            'extern "C" int dsa_platform_should_play_intro(int argc, int first_start);\nextern "C" int dsa_game_main(int argc, char **argv)'
        )
        txt = txt.replace("srand(time(0));", "srand(dsa_platform_init_seed((uint32_t)time(0)));")
        txt = txt.replace(
            "\t\tif ((argc > 1 && (argv[1][0] == '-' || argv[1][0] == '/')) || (g_first_start != 0 && argc <= 1)) {",
            "\t\tif ((argc > 1 && (argv[1][0] == '-' || argv[1][0] == '/')) || dsa_platform_should_play_intro(argc, g_first_start)) {"
        )
        txt = replace_func_body(txt, r"^signed short dos_files_total\([^)]*\)\s*\{", "    return 40;")

    # 14b. OVL041.CPP: Sprachzeilen in Outro1/Outro2 (Schiffsreise/Traum) per Taste/Mausklick ueberspringbar
    elif base == "OVL041.CPP":
        txt = txt.replace(
            "\t\t\t\twhile (snd_speech_busy(g_snd_speech)) {\n\t\t\t\t\tgame_idle();\n\t\t\t\t\twait_for_vsync();\n\t\t\t\t}",
            "\t\t\t\twhile (snd_speech_busy(g_snd_speech)) {\n\t\t\t\t\tpoll_input();\n\t\t\t\t\tif (g_action || g_mouse_lclick || g_mouse_rclick) {\n\t\t\t\t\t\tmouse_clear_left();\n\t\t\t\t\t\tmouse_clear_right();\n\t\t\t\t\t\tg_mouse_event = g_mouse_ldclick = 0;\n\t\t\t\t\t\tSpeech_stop(g_snd_speech);\n\t\t\t\t\t\tbreak;\n\t\t\t\t\t}\n\t\t\t\t\twait_for_vsync();\n\t\t\t\t}"
        )

    # 15. OVL096.C: view_alloc_lists + view_load_palette + pal_find_nearest
    elif base == "OVL096.C":
        alloc_lists_c = """    void *p;
    if ((p = mem_alloc(0x61c)) == 0) guru_meditation(DS_STR(0x5e78), 0);
    g_face_seg = 0;
    g_face_block = p;
    g_face_next = (unsigned char *)p + 0x10;
    g_face_depth = (unsigned char *)g_face_next + 0x102;
    g_vtx_buf = (struct vtx *)((unsigned char *)g_face_depth + 0x102);"""
        txt = replace_func_body(txt, r"^void view_alloc_lists\([^)]*\)\s*\{", alloc_lists_c)
        txt = txt.replace(
            "\tp += 0x10 - FP_OFF(p);\n\tfor (i = 0; i < 32; i++) g_shade_segs[i] = FP_SEG(p) + (i << 4);",
            "\tp = (unsigned char *)g_shade_buf + 8;\n\tg_port_shade_base = (unsigned char *)p;\n\tfor (i = 0; i < 32; i++) g_shade_segs[i] = (unsigned short)(0x9000 + i);"
        )
        pal_nearest_c = """    unsigned char r = c->r, g = c->g, b = c->b;
    unsigned short di = (unsigned short)start;
    const unsigned char *si = (const unsigned char *)g_view_pal;
#if FIX_0016
    si += di * 3;
#else
    si += di * 4;
#endif
    unsigned char best_dist = 0xff;
    unsigned short best = 0xffff;
    for (;;) {
        int dr = (signed char)(si[0] - r); if (dr < 0) dr = -dr;
        int dg = (signed char)(si[1] - g); if (dg < 0) dg = -dg;
        int db = (signed char)(si[2] - b); if (db < 0) db = -db;
        unsigned char dist = (unsigned char)(dr + dg + db);
        if (dist < best_dist) {
            best_dist = dist;
            best = (di & 0xff) | ((unsigned short)dist << 8);
            if (dist == 0)
                break;
        }
        di++;
        si += 3;
#if FIX_0016
        if (di >= (unsigned short)g_view_pal_n)
            break;
        if (di >= 0xa0)
            break;
#else
        if (di >= 0xa0)
            break;
#endif
    }
    return (signed short)best;"""
        txt = replace_func_body(txt, r"^signed short pal_find_nearest\([^)]*\)\s*\{", pal_nearest_c)

    # 16. OVL188.CPP, OVL189.CPP, OVL205.CPP & OVL198.C: va_arg-, Nullzeiger- und Struct-Return-Anpassungen fuer 32-Bit-GCC
    elif base == "OVL188.CPP":
        txt = txt.replace("va_arg(ap, signed short)", "(signed short)va_arg(ap, int)")
    elif base == "OVL189.CPP":
        txt = txt.replace("va_arg(ap, char far *)", "va_arg(*(va_list *)&ap, char far *)")
        txt = txt.replace(
            "\tif ((*s = toupper(*s)) == '<')\n\t\ts[1] = toupper(s[1]);\n\tstrcpy(text_buf, s);",
            "\tif (s != 0) {\n\t\tif ((*s = toupper(*s)) == '<')\n\t\t\ts[1] = toupper(s[1]);\n\t\tstrcpy(text_buf, s);\n\t} else {\n\t\ttext_buf[0] = 0;\n\t}"
        )
        txt = txt.replace("\tstrcpy(body, text_buf);", "\tif (body != 0) strcpy(body, text_buf);")
    elif base == "OVL205.CPP":
        txt = txt.replace("\tstrcpy(text, text_buf);", "\tif (text != 0) strcpy(text, text_buf);")
    elif base == "OVL198.C":
        txt = txt.replace(
            "#define GET_CELL_ROT(dst, x, y) ((void (*)(struct cell_rot far *, signed short, signed short))get_cell_c)(dst, x, y)",
            "#if !defined(__BORLANDC__)\n#define GET_CELL_ROT(dst, x, y) (*(dst) = get_cell_c(x, y))\n#else\n#define GET_CELL_ROT(dst, x, y) ((void (*)(struct cell_rot far *, signed short, signed short))get_cell_c)(dst, x, y)\n#endif"
        )

    # 16a. OVL042.CPP: Dummy-Treiberbeschreibung fuer Sound::mtype() bei stummgeschaltetem AIL in Etappe 1
    elif base == "OVL042.CPP":
        txt = txt.replace(
            "#define SND_FX\t\t((Sound far *)g_snd_fx)\n",
            "#define SND_FX\t\t((Sound far *)g_snd_fx)\nstatic struct ail_drv_desc s_dummy_desc = { 0, 0, \"\", (char *)\"\", 0, 0, 0, 0, 0, 0 };\n"
        )
        txt = txt.replace("Sound::Sound(File far *f, struct arc far *ar, File far *fs)\n{\n\tsigned short i;\n",
                          "Sound::Sound(File far *f, struct arc far *ar, File far *fs)\n{\n\tsigned short i;\n\tdesc = &s_dummy_desc;\n")
        txt = txt.replace("Sound::Sound(File far *f, File far *fs)\n{\n\tsigned short i;\n",
                          "Sound::Sound(File far *f, File far *fs)\n{\n\tsigned short i;\n\tdesc = &s_dummy_desc;\n")
        txt = txt.replace("Sound::Sound(struct arc far *ar)\n{\n\tsigned short i;\n",
                          "Sound::Sound(struct arc far *ar)\n{\n\tsigned short i;\n\tdesc = &s_dummy_desc;\n")
        txt = txt.replace("Sound::Sound(struct arc far *ar, File far *f)\n{\n\tsigned short i;\n",
                          "Sound::Sound(struct arc far *ar, File far *f)\n{\n\tsigned short i;\n\tdesc = &s_dummy_desc;\n")

    # 16b. OVL030.CPP: Rohadressnamen auf kanonische C-Symbole abbilden + eigene Daten (ds:187a..1966)
    elif base == "OVL030.CPP":
        ovl30_hdr = """#ifndef far
#define far
#define huge
#define near
#endif
#define g_256a g_town_cells_state
#define g_2808 g_loc_saved_flag
#define g_283f g_loc_id
#define g_2841 g_loc_name
#define g_285f g_exit_dir
#define g_35fe g_file_3
#define g_37ce g_cd_cache
#define g_382d g_snd_music
#define g_3892 g_music_id_req
#define g_395b g_loaded_town
#define g_395c g_loc_exit_mark
#define g_3996 g_leave_location
#define g_399a g_cd_music
#define g_8b94 g_view_recam
#define g_8baf g_mouse_ldclick
#define g_8bbb g_mouse_event
#define g_8bd5 g_key_flag
#define g_8c30 g_cam_angle
#define g_949e g_view_input_on
#define g_94a2 g_cam_speed
#define g_94a4 g_cam_roll
#define g_b11e g_step_active
#define g_b11f g_step_pending
#define g_b4a0 g_view_input_blocked
#define g_b59a g_dialog
#define g_b668 g_vga_buf
#define g_b67c g_ttx
#define g_b684 g_pal_dark
#define g_b77e g_screen
#define g_b77f g_screen_pic
#define g_b799 g_screen_pic_pal
#define g_c4c5 g_group
#define g_c4d0 g_refresh
#define g_c715 g_mem_saving
#define g_c71e g_palette

#define O003_0bca load_pic_list
#define O010_12ee swap_out_screen
#define O034_26a6 prepare_spell_icons
#define O042_086f Sound_stop_music
#define O042_0c9a snd_obj_stop
#define O042_0e53 Sound_volume3
#define O046_1a8e town_cache_files
#define O054_0ed5 anim_free_all
#define O055_032f ani_halt
#define O184_0479 ev_show_pic_197
#define O188_0cbf screen_reset_icons
#define O190_0082 dlg_yes_no
#define O190_0a4f dlg_set16
#define O201_0df8 group_set_on_route
#define R02_00f9 set_palette
#define R04_0760 snd_play
#define R06_0398 arc_open
#define R06_080f arc_read
#define R06_0c16 arc_close
#define R06_0d24 arc_seek
#define R18_0009 mouse_show
#define R18_001a mouse_hide
#define R18_0175 mouse_set_cursor
#define R18_0ae4 mouse_clear_left
#define R18_0afb mouse_clear_right
#define R18_0b12 clear_input
#define R31_0005 clear_kbd
#define R42_04df gb_blit_m
#define R44_0008 pal_fade
#define R45_0006 wait_for_vsync

#define f_0000 enter_location
#define f_0bda loc_return_redraw
#define f_0cbe loc_fade_out
#define f_0d47 loc_load_data
"""
        txt = txt.replace("extern struct party far *g_c4c5;\n", ovl30_hdr + 'extern "C" {\nextern struct party far *g_c4c5;\n', 1)
        txt = txt.replace("signed char f_1135(signed short n);\n", "signed char f_1135(signed short n);\n}\n", 1)
        txt = txt.replace("void O003_0bca(", "signed short O003_0bca(", 1)
        txt = txt.replace("void O010_12ee(void);", "signed short O010_12ee(void);", 1)
        txt = txt.replace("void O042_0c9a(void far *p);", "signed short O042_0c9a(void far *p);", 1)
        txt = txt.replace("void R06_0398(void far *f, signed short id);", "signed short R06_0398(void far *f, signed short id);", 1)
        txt = txt.replace("void R06_080f(void far *f, void far *buf, signed long n);", "signed long R06_080f(void far *f, void far *buf, signed long n);", 1)
        txt = txt.replace("void R06_0c16(void far *f);", "signed short R06_0c16(void far *f);", 1)
        txt = txt.replace("void R06_0d24(void far *f, signed long off, signed short whence);", "signed long R06_0d24(void far *f, signed long off, signed short whence);", 1)
        txt = txt.replace("\tcase 4:\n\t\tC166 o;", "\tcase 4:\n\t\t{\n\t\tC166 o;", 1)
        txt = txt.replace("\t\t\to.f1313();\n\t\t\tbreak;\n\t\t}\n\t\tbreak;", "\t\t\to.f1313();\n\t\t\tbreak;\n\t\t}\n\t\t}\n\t\tbreak;", 1)

    # 16c. OVL243.CPP: _AH = 0x0d; geninterrupt(0x21);
    elif base == "OVL243.CPP":
        txt = txt.replace("\t\t_AH = 0x0d;\t\t/* DOS: Puffer zurueckschreiben */\n\t\tgeninterrupt(0x21);", "\t\t((void)0);")

    # 16c2. OVL004.CPP: CD-Titel aus MUSIC/TrackNN.ogg statt MSCDEX-IOCTL
    if base == "OVL004.CPP":
        txt = txt.replace(
            "void CD::play(register signed short t)\n{\n\tunsigned long far *p;\n\tunsigned long len;\n\n\terr = 0;\n"
            "\tif (di->first > t || di->last < t) {\n\t\terr = CD_ERR_TRACK;\n\t\treturn;\n\t}\n"
            "\tvolume(g_vol_music);\n\trq->cmd = 0x84;\n"
            "\trq->ptr = (void far *)msf2hsg((unsigned char far *)&tr[t - 1].start);\n"
            "\tlen = msf2hsg((unsigned char far *)&tr[t].start) - (signed long)rq->ptr;\n"
            "\tp = (unsigned long far *)&rq->count;\n\t*p = len;\n\trequest();\n\tstatus();\n"
            "\tif (busy == 0)\n\t\terr = CD_ERR_PLAY;\n\telse\n\t\ttrack = t;\n"
            "\tlen *= 18;\n\tlen /= 75;\n\tendtime = g_random_seed2 + len - 10;\n}",
            "void CD::play(register signed short t)\n{\n\terr = 0;\n"
            "\tif (di->first > t || di->last < t) {\n\t\terr = CD_ERR_TRACK;\n\t\treturn;\n\t}\n"
            "\tvolume(g_vol_music);\n\tif (dsa_platform_play_cd_track(t)) {\n"
            "\t\ttrack = t;\n\t\tbusy = 1;\n\t\t/* Die Ogg-Spur laeuft in einer Schleife. update() soll sie nicht neu anwerfen. */\n\t\tendtime = g_random_seed2 + 6000;\n\t\treturn;\n\t}\n"
            "\terr = CD_ERR_PLAY;\n}"
        )
        txt = txt.replace(
            "void CD::stop()\n{\n\terr = 0;\n\ttrack = 0;\n\tstatus();",
            "void CD::stop()\n{\n\tdsa_platform_stop_cd_audio();\n\terr = 0;\n\ttrack = 0;\n\tstatus();"
        )
        txt = txt.replace(
            "\twhile (busy == 1 || paused == 1) {\n\t\trq->cmd = 0x85;\n\t\trequest();\n\t\tstatus();\n\t}",
            "\tfor (int guard = 0; (busy == 1 || paused == 1) && guard < 8; guard++) {\n"
            "\t\trq->cmd = 0x85;\n\t\trequest();\n\t\tstatus();\n\t}\n\tbusy = 0;\n\tpaused = 0;"
        )
        txt = txt.replace(
            "void CD::status()\n{\n    open = 0; busy = 0; paused = 0;\n}",
            "extern \"C\" int dsa_platform_play_cd_track(int track_no);\n"
            "extern \"C\" void dsa_platform_stop_cd_audio(void);\n"
            "extern \"C\" int dsa_platform_cd_is_playing(void);\n"
            "extern \"C\" void dsa_platform_set_music_volume(int vol_0_100);\n\n"
            "void CD::status()\n{\n    open = 0; paused = 0; busy = dsa_platform_cd_is_playing() ? 1 : 0;\n}"
        )
        txt = txt.replace(
            "\tif (v > 100)\n\t\tv = 100;\n\tv *= 255;",
            "\tif (v > 100)\n\t\tv = 100;\n\tdsa_platform_set_music_volume(v);\n\tv *= 255;"
        )

    # 16d. OVL049.CPP: Conv::ask 16-Bit Real-Mode Nullzeiger-Dereferenzierung vor s->list != 0 Pruefung
    if base == "OVL049.CPP":
        txt = txt.replace("\ts = script->topics + t;\n\te = s->list[k];", "\ts = script->topics + t;\n\te = s->list ? s->list[k] : 0;")

    # 16e. OVL072.CPP: camp_rest 16-Bit Real-Mode Nullzeiger-Dereferenzierung fuer leader->npc_id und hw->set_watches
    if base == "OVL072.CPP":
        txt = txt.replace("if (w == 1 && leader->npc_id == 3)", "if (w == 1 && leader && leader->npc_id == 3)")
        txt = txt.replace("hw->set_watches(hw->watches - 1);", "if (hw) hw->set_watches(hw->watches - 1);")

    # 16f. OVL144.CPP: fight_printer 16-Bit Real-Mode Nullzeiger-Dereferenzierung bei Lst_add0 (MSG.obj1 == 0 / ENEMY2 == 0)
    if base == "OVL144.CPP":
        txt = txt.replace(
            "\t\t\t\tMSG.type1 == 2 ? ((struct hero far *)MSG.obj1)->name :\n"
            "\t\t\t\t\tgrammar_article(0x8000, ((struct enemy far *)MSG.obj1)->mon_id, 1),\n"
            "\t\t\t\tMSG.type2 == 2 ? HERO2->name :\n"
            "\t\t\t\t\tgrammar_article(0x8002, ENEMY2->mon_id, 1));",
            "\t\t\t\t!MSG.obj1 ? \"\" : (MSG.type1 == 2 ? ((struct hero far *)MSG.obj1)->name :\n"
            "\t\t\t\t\tgrammar_article(0x8000, ((struct enemy far *)MSG.obj1)->mon_id, 1)),\n"
            "\t\t\t\t!ENEMY2 ? \"\" : (MSG.type2 == 2 ? HERO2->name :\n"
            "\t\t\t\t\tgrammar_article(0x8002, ENEMY2->mon_id, 1)));"
        )

    # 16g. OVL015.C: sheet_slot_changed leere g_anim_timer-Warteschleifen durch wait_for_vsync() ersetzen
    if base == "OVL015.C":
        txt = txt.replace("while (g_anim_timer == 0)\n\t\t\t\t\t;", "while (g_anim_timer == 0)\n\t\t\t\t\twait_for_vsync();")
        txt = txt.replace("while (g_anim_timer == 0)\n\t\t\t\t;", "while (g_anim_timer == 0)\n\t\t\t\twait_for_vsync();")
        txt = txt.replace("memcpy(g_vga_buf, buf, 64000U);", "memcpy(g_vga_buf, buf, 64000U); wait_for_vsync();")

    # 16h. OVL024.C: fli_play Warteschleife auf g_fli_ticks mit wait_for_vsync() fuer fli_cb_frames
    if base == "OVL024.C":
        txt = txt.replace(
            "\t\t\t\t\tnow = g_fli_ticks;\n\t\t\t\t\tif (now >= tnext) {\n\t\t\t\t\t\tt0 = now;\n\t\t\t\t\t\tbreak;\n\t\t\t\t\t}\n\t\t\t\t}",
            "\t\t\t\t\tnow = g_fli_ticks;\n\t\t\t\t\tif (now >= tnext) {\n\t\t\t\t\t\tt0 = now;\n\t\t\t\t\t\tbreak;\n\t\t\t\t\t}\n\t\t\t\t\twait_for_vsync();\n\t\t\t\t}"
        )

    # 16h2. OVL035.CPP: Abspann-Nachweis
    if base == "OVL035.CPP":
        txt = txt.replace(
            "void Finale::run()\n{\n\tSaveMenu far *sm;",
            "extern \"C\" void dsa_log_abspann(const char *);\n\n"
            "void Finale::run()\n{\n"
            "\tif (g_final_fight_won)\n\t\tdsa_log_abspann(\"Endkampf-Sieg\");\n"
            "\tdsa_log_abspann(\"Beginn Finale::run\");\n\tSaveMenu far *sm;"
        )
        txt = txt.replace(
            "\t\tdlg_message(g_dialog, txt[10]);\n\t\tg_win_centered = old;",
            "\t\tdlg_message(g_dialog, txt[10]);\n\t\tdsa_log_abspann(\"Ende Abspann\");\n\t\tg_win_centered = old;"
        )

    # 16h3. OVL131.CPP: Endkampf gesetzt
    if base == "OVL131.CPP":
        txt = txt.replace(
            "signed short dng03_fights(struct area_events far *ev, register signed short pos)",
            "extern \"C\" void dsa_log_abspann(const char *);\n\n"
            "signed short dng03_fights(struct area_events far *ev, register signed short pos)"
        )
        txt = txt.replace(
            "\t\tg_final_fight_won = 1;\n",
            "\t\tg_final_fight_won = 1;\n\t\tdsa_log_abspann(\"Endkampf-Sieg\");\n"
        )

    # 16i. OVL036.C: finale_play_script Warteschleife auf g_finale_ticks mit wait_for_vsync()
    if base == "OVL036.C":
        txt = txt.replace(
            "\t\t\tcase FINALE_OP_END:\n\t\t\t\tdone = 1;\n\t\t\t\tbreak;\n\t\t\t}\n\t\t}\n\t} while (done != 1);",
            "\t\t\tcase FINALE_OP_END:\n\t\t\t\tdone = 1;\n\t\t\t\tbreak;\n\t\t\t}\n\t\t} else {\n\t\t\twait_for_vsync();\n\t\t}\n\t} while (done != 1);"
        )

    # 16j. SEG019.C: Zellzugriff nur innerhalb der 64x64-Tabelle (freigegebene Karte, abgeschnittene Koordinate)
    if base == "SEG019.C":
        txt = txt.replace(
            "unsigned char get_cell_a(signed short y, signed short x)\n"
            "{\n"
            "\treturn g_cells[y][x].type;\n"
            "}\n"
            "\n"
            "signed short get_cell_b(signed short y, signed short x)\n"
            "{\n"
            "\treturn g_cells[y][x].flags;\n"
            "}\n"
            "\n"
            "struct cell_rot get_cell_c(signed short y, signed short x)\n"
            "{\n"
            "\tstruct cell_rot rot;\n"
            "\n"
            "\trot.ax = rot.ay = 0;\n"
            "\trot.az = g_cells[y][x].rot << 8;\n"
            "\treturn rot;\n"
            "}",
            "/* 32-Bit: freigegebene oder abgeschnittene 16-Bit-Koordinaten (unsigned char aus einem\n"
            "   negativen Kurzindex, z. B. 254) duerfen nicht in die 64x64-Tabelle zeigen. */\n"
            "static int cell_in_map(signed short y, signed short x)\n"
            "{\n"
            "\treturn g_cells && (unsigned short)y < 64 && (unsigned short)x < 64;\n"
            "}\n"
            "\n"
            "unsigned char get_cell_a(signed short y, signed short x)\n"
            "{\n"
            "\tif (!cell_in_map(y, x))\n"
            "\t\treturn 0;\n"
            "\treturn g_cells[y][x].type;\n"
            "}\n"
            "\n"
            "signed short get_cell_b(signed short y, signed short x)\n"
            "{\n"
            "\tif (!cell_in_map(y, x))\n"
            "\t\treturn 0;\n"
            "\treturn g_cells[y][x].flags;\n"
            "}\n"
            "\n"
            "struct cell_rot get_cell_c(signed short y, signed short x)\n"
            "{\n"
            "\tstruct cell_rot rot;\n"
            "\n"
            "\trot.ax = rot.ay = rot.az = rot.aw = 0;\n"
            "\tif (!cell_in_map(y, x))\n"
            "\t\treturn rot;\n"
            "\trot.az = g_cells[y][x].rot << 8;\n"
            "\treturn rot;\n"
            "}"
        )

    # 16k. SEG013.C: Profil einer fehlenden Zelle nicht anfassen
    if base == "SEG013.C":
        txt = txt.replace(
            "\tcell = get_cell_a(x, y);\n"
            "\tq = get_cell_c(x, y);\n"
            "\to = g_cell_objs[cell];\n"
            "\ti = o->nfaces;",
            "\tcell = get_cell_a(x, y);\n"
            "\tq = get_cell_c(x, y);\n"
            "\to = (g_cell_objs && (unsigned)cell < 32) ? g_cell_objs[cell] : 0;\n"
            "\tif (!o || !o->faces || !o->faces[0])\n"
            "\t\treturn slot;\n"
            "\ti = o->nfaces;"
        )

    # 16l. OVL010/OVL096: nach dem Freigeben keinen haengenden Kartenzeiger lassen
    if base == "OVL010.CPP" or base == "OVL096.C":
        txt = txt.replace(
            "mem_free(g_cells);",
            "g_cells = (struct map_cell (far *)[MAP_SIZE])mem_free(g_cells);"
        )
        txt = txt.replace(
            "\t\tmouse_hide(0);\n\t\tcopy_3d_to_screen();\n\t\tmouse_show(0);",
            "\t\tmouse_hide(0);\n\t\tcopy_3d_to_screen();\n\t\tmouse_show(0);\n\t\twait_for_vsync();"
        )

    # 16m. OVL230/OVL174: Ortsname nur innerhalb der geladenen Textliste
    if base == "OVL230.CPP":
        txt = txt.replace(
            "#define DATE\t\t((struct game_date far *)g_date)\n",
            "#define DATE\t\t((struct game_date far *)g_date)\n\n"
            "/* Ortsname gehoert zur geladenen Textliste. Ein Sprung ohne loc_load_data laesst\n"
            "   einen fremden Index stehen; der zeigt hinter die Liste auf den naechsten Blockkopf. */\n"
            "static void smith_clamp_name(void)\n{\n"
            "\tif ((unsigned short)g_loc_name >= SMITH_TXT_N)\n"
            "\t\tg_loc_name = 0;\n}\n\n"
            "extern \"C\" void dsa_open_place(int which)\n{\n"
            "\tif (g_loc_id < 0)\n\t\tg_loc_id = 1;\n"
            "\tif (which == 1) {\n\t\tSmith *s = new Smith;\n\t\ts->check_open();\n\t\ts->run();\n\t\tdelete s;\n"
            "\t} else if (which == 2) {\n\t\tHealer *h = new Healer;\n\t\th->is_open();\n\t\th->run();\n\t\tdelete h;\n"
            "\t} else if (which == 3) {\n\t\tStash *st = new Stash(0, 0);\n\t\tdelete st;\n\t}\n}\n"
        )
        txt = txt.replace(
            "signed short Smith::check_open()\n{\n\tstruct hero far *h;\n",
            "signed short Smith::check_open()\n{\n\tstruct hero far *h;\n\tsmith_clamp_name();\n"
        )
        txt = txt.replace(
            "void Smith::run()\n{\n\tstruct hero far *h;\n\tregister signed short ans;\n",
            "void Smith::run()\n{\n\tstruct hero far *h;\n\tregister signed short ans;\n\n\tsmith_clamp_name();\n"
        )
    if base == "OVL174.CPP":
        txt = txt.replace(
            "#define LOC_TEXTBAR(t) ((void (*)(void far *, char far *, signed short))screen_textbar)(g_screen, t, 0xff)\n",
            "#define LOC_TEXTBAR(t) ((void (*)(void far *, char far *, signed short))screen_textbar)(g_screen, t, 0xff)\n"
            "#define HEALER_TXT_N 0x23\n\n"
            "static void healer_clamp_name(void)\n{\n"
            "\tif ((unsigned short)g_loc_name >= HEALER_TXT_N)\n"
            "\t\tg_loc_name = 0;\n}\n"
        )
        txt = txt.replace(
            "\twhile (!done) {\n\t\tif (g_refresh) {\n",
            "\thealer_clamp_name();\n\twhile (!done) {\n\t\tif (g_refresh) {\n"
        )

    # 16n. OVL000: Bingen-Ereignis und Schloss einmal aufrufbar
    if base == "OVL000.CPP":
        txt = txt.replace(
            "\tif (g_dng10_chest_opened != 0) {\n"
            "\t\tStash far *s = new Stash(0x26, 0);\n\n"
            "\t\tdelete s;\n"
            "\t}\n"
            "\tdlg_set16(g_dialog, old);\n"
            "}",
            "\tif (g_dng10_chest_opened != 0) {\n"
            "\t\tStash far *s = new Stash(0x26, 0);\n\n"
            "\t\tdelete s;\n"
            "\t}\n"
            "\tdlg_set16(g_dialog, old);\n"
            "}\n\n"
            "/* Ein Aufruf der Bingen-Ereignisse und eines Schlosses, wenn der Dungeon schon steht.\n"
            "   Das Schlossobjekt bleibt liegen, damit der Destruktor die Datendatei nicht zurueckschreibt. */\n"
            "extern \"C\" void dsa_binge_probe(void)\n"
            "{\n"
            "\tif (g_area_events)\n"
            "\t\tdng10_countdown((struct area_events far *)g_area_events);\n"
            "\tnew Lock(0x6c);\n"
            "}"
        )

    # 16n2. OVL007.C, OVL081.CPP, OVL222.C: Testspruenge fuer Abdeckung
    if base == "OVL007.C":
        txt = txt.replace("#include <stdio.h>\n", "#include <stdio.h>\n#include <stdlib.h>\n")
        txt = txt.replace(
            "\tregister signed short c;\n\n\tanim_free_all(g_screen_pic);\n\tSound_stop_music(g_snd_music);\n\tdo {\n"
            "\t\ttxt_load_list(g_font, DEBUG_TXT, 0x1cc, &g_debug_txt_buf, &g_debug_txt);\n"
            "\t\tc = dlg_choose(g_dialog, g_debug_txt[53], 10, g_debug_txt[430], g_debug_txt[354], g_debug_txt[352], g_debug_txt[447],\n"
            "\t\t\tg_debug_txt[448], g_debug_txt[451], g_debug_txt[452], g_debug_txt[453], g_debug_txt[454], g_debug_txt[455]);\n"
            "\t\tg_debug_txt_buf = mem_free(g_debug_txt_buf);\n"
            "\t\tg_debug_txt = mem_free(g_debug_txt);",
            "\tregister signed short c;\n\n\tconst char *jump;\n\tsigned short once;\n\tsigned short forced;\n\n"
            "\tanim_free_all(g_screen_pic);\n\tSound_stop_music(g_snd_music);\n"
            "\tjump = getenv(\"DSA_TESTMENU\");\n\tonce = (jump && jump[0] && jump[0] != '0') ? (signed short)atoi(jump) : 0;\n\tforced = once;\n\tdo {\n"
            "\t\tif (once) {\n\t\t\tc = once;\n\t\t\tonce = 0;\n\t\t} else {\n"
            "\t\ttxt_load_list(g_font, DEBUG_TXT, 0x1cc, &g_debug_txt_buf, &g_debug_txt);\n"
            "\t\tc = dlg_choose(g_dialog, g_debug_txt[53], 10, g_debug_txt[430], g_debug_txt[354], g_debug_txt[352], g_debug_txt[447],\n"
            "\t\t\tg_debug_txt[448], g_debug_txt[451], g_debug_txt[452], g_debug_txt[453], g_debug_txt[454], g_debug_txt[455]);\n"
            "\t\tg_debug_txt_buf = mem_free(g_debug_txt_buf);\n"
            "\t\tg_debug_txt = mem_free(g_debug_txt);\n\t\t}"
        )
        txt = txt.replace(
            "\t\tcase 10:\n\t\t\tbreak;\n\t\t}\n\t} while (c != -1);",
            "\t\tcase 10:\n\t\t\tbreak;\n\t\t}\n\t\tif (forced)\n\t\t\tc = -1;\n\t} while (c != -1);"
        )

    if base == "OVL081.CPP":
        txt = txt.replace(
            "#include \"schweif.h\"\n\nvoid disease_effect(struct hero far *h)",
            "#include \"schweif.h\"\n\nextern \"C\" void dsa_effect_probe(void)\n{\n\tstruct hero far *h;\n\n"
            "\th = get_first_hero(g_group);\n\tif (!h)\n\t\treturn;\n\tdisease_effect(h);\n\tpoison_effect(h);\n}\n\n"
            "void disease_effect(struct hero far *h)"
        )

    if base == "OVL222.C":
        txt = txt.replace("#include \"schweif.h\"", "#include <stdlib.h>\n#include \"schweif.h\"", 1)
        txt = txt.replace(
            "\tsigned char c;\n\n\tc = 0;\n\twhile (c != 11) {\n\t\tc = dlg_choose(",
            "\tsigned char c;\n\n\tconst char *jump;\n\tsigned char once;\n\tsigned char forced;\n\n"
            "\tjump = getenv(\"DSA_TRAVEL\");\n\tonce = (jump && jump[0] && jump[0] != '0') ? (signed char)atoi(jump) : 0;\n\tforced = once;\n"
            "\tc = 0;\n\twhile (c != 11) {\n\t\tif (once) {\n\t\t\tc = once;\n\t\t\tonce = 0;\n\t\t} else\n\t\tc = dlg_choose("
        )
        txt = txt.replace(
            "\t\tcase 11:\n\t\t\tbreak;\n\t\t}\n\t}\n\tp->from = 1;",
            "\t\tcase 11:\n\t\t\tbreak;\n\t\t}\n\t\tif (forced)\n\t\t\tc = 11;\n\t}\n\tp->from = 1;"
        )

    # 16n3. OVL211.C, OVL210.CPP, OVL083.CPP, OVL085.CPP: A2e Absicherung fuer debug_travel_events / tevent_257 / drop_item_at
    if base == "OVL211.C":
        txt = txt.replace(
            "\told_hurry = p->hurry;\n",
            "\tif (!g_places) {\n\t\tg_places = (struct travel_place far *)mem_alloc(file_length(g_file_main, 0xa1));\n\t\tofile_load(g_ofile, 0xa1, g_places);\n\t}\n"
            "\tif (!g_routes) {\n\t\tg_routes = (struct route far *)mem_alloc(file_length(g_file_main, 0xa2));\n\t\tofile_load(g_ofile, 0xa2, g_routes);\n\t}\n"
            "\told_hurry = p->hurry;\n"
        )
    if base == "OVL210.CPP":
        txt = txt.replace(
            "\ttxt_load_list(g_font, TXT_TEV, TXT_TEV_N, &g_tev_buf, &g_tev_txt);",
            "\tif (!g_places) {\n\t\tg_places = (struct travel_place far *)mem_alloc(file_length(g_file_main, 0xa1));\n\t\tofile_load(g_ofile, 0xa1, g_places);\n\t}\n"
            "\tif (!g_routes) {\n\t\tg_routes = (struct route far *)mem_alloc(file_length(g_file_main, 0xa2));\n\t\tofile_load(g_ofile, 0xa2, g_routes);\n\t}\n"
            "\ttxt_load_list(g_font, TXT_TEV, TXT_TEV_N, &g_tev_buf, &g_tev_txt);"
        )
    if base == "OVL083.CPP":
        txt = txt.replace(
            "signed short drop_item_at(struct hero far *h, register signed short pos)\n{\n",
            "signed short drop_item_at(struct hero far *h, register signed short pos)\n{\n\tif (!h || pos < 0 || pos >= NR_INV_SLOTS)\n\t\treturn 0;\n"
        )
    if base == "OVL085.CPP":
        txt = txt.replace(
            "signed short drop_item(struct hero far *h, signed short pos, signed short n)\n{\n\tsigned short id;\n\tsigned short ret = 0;\n",
            "signed short drop_item(struct hero far *h, signed short pos, signed short n)\n{\n\tsigned short id;\n\tsigned short ret = 0;\n\tif (!h || pos < 0 || pos >= NR_INV_SLOTS)\n\t\treturn 0;\n"
        )

    # 16o. SEG018: Testspruenge fuer Abdeckung
    if base == "SEG018.C":
        txt = txt.replace(
            "\tif (g_key_shift == 3 && g_dev_screen_on == 0) {\n"
            "\t\tdev_info_screen();\n"
            "\t}\n\n"
            "\tif (g_mouse_event != 0) {",
            "\tif (g_key_shift == 3 && g_dev_screen_on == 0) {\n"
            "\t\tdev_info_screen();\n"
            "\t}\n\n"
            "\t/* DSA_TESTMENU=1..8 springt einen Punkt von debug_test_menu an.\n"
            "\t * DSA_TRAVEL=1..10 springt ein Reiseereignis von debug_travel_events an. */\n"
            "\t{\n"
            "\t\tstatic int jump_done;\n"
            "\t\tconst char *tm;\n"
            "\t\tconst char *tr;\n"
            "\t\textern uint32_t dsa_platform_get_frame_count(void);\n"
            "\t\tif (!jump_done && dsa_platform_get_frame_count() > 160) {\n"
            "\t\t\ttm = getenv(\"DSA_TESTMENU\");\n"
            "\t\t\ttr = getenv(\"DSA_TRAVEL\");\n"
            "\t\t\tif ((tm && tm[0] && tm[0] != '0') || (tr && tr[0] && tr[0] != '0')) {\n"
            "\t\t\t\tjump_done = 1;\n"
            "\t\t\t\tif (tm && tm[0] && tm[0] != '0')\n"
            "\t\t\t\t\tdebug_test_menu();\n"
            "\t\t\t\tif (tr && tr[0] && tr[0] != '0')\n"
            "\t\t\t\t\tdebug_travel_events((struct travel far *)g_travel);\n"
            "\t\t\t}\n"
            "\t\t\ttm = getenv(\"DSA_ORT\");\n"
            "\t\t\tif (tm && tm[0] && tm[0] != '0') {\n"
            "\t\t\t\textern void dsa_open_place(int);\n"
            "\t\t\t\tjump_done = 1;\n"
            "\t\t\t\tdsa_open_place(atoi(tm));\n"
            "\t\t\t}\n"
            "\t\t\ttm = getenv(\"DSA_BINGE\");\n"
            "\t\t\tif (tm && tm[0] == '1' && g_group && g_group->dungeon == 10) {\n"
            "\t\t\t\textern void dsa_binge_probe(void);\n"
            "\t\t\t\tjump_done = 1;\n"
            "\t\t\t\tdsa_binge_probe();\n"
            "\t\t\t}\n"
            "\t\t\ttm = getenv(\"DSA_EFFEKT\");\n"
            "\t\t\tif (tm && tm[0] == '1' && g_group) {\n"
            "\t\t\t\textern void dsa_effect_probe(void);\n"
            "\t\t\t\tjump_done = 1;\n"
            "\t\t\t\tdsa_effect_probe();\n"
            "\t\t\t}\n"
            "\t\t}\n"
            "\t}\n\n"
            "\tif (g_mouse_event != 0) {"
        )

    # 16p. OVL183: Schiffsankunft liest die Zelle, bevor die Karte ausgelagert wird
    if base == "OVL183.CPP":
        txt = txt.replace(
            "\tc = &g_cells[ev->x][ev->y];\t/* map_event: +2 = Zeile, +3 = Spalte (s_struct.h) */\n"
            "\treturn c->rot - 1;",
            "\tif (!g_cells || ev->x >= 64 || ev->y >= 64)\n"
            "\t\treturn 0;\n"
            "\tc = &g_cells[ev->x][ev->y];\t/* map_event: +2 = Zeile, +3 = Spalte (s_struct.h) */\n"
            "\treturn c->rot - 1;"
        )
        txt = txt.replace(
            "\tif (type == MAP_EVENT_ENTER) {\n"
            "\t\tif ((xy == 0xe07 || xy == 0xf07 || xy == 0xf08 || xy == 0x1007 || xy == 0xe08 ||\n",
            "\tif (type == MAP_EVENT_ENTER) {\n"
            "\t\t/* Vor ship_voyage lesen: die Auslagerung gibt g_cells frei, und der Handler\n"
            "\t\t   kann g_cell_event danach mit einer abgeschnittenen Kamerazelle ueberschreiben. */\n"
            "\t\tsigned short entered_cell = get_cell_a(ev->x, ev->y);\n"
            "\t\tif ((xy == 0xe07 || xy == 0xf07 || xy == 0xf08 || xy == 0x1007 || xy == 0xe08 ||\n"
        )
        txt = txt.replace(
            "\t\tif (get_cell_a(ev->x, ev->y) == 3)\n"
            "\t\t\tswim_check();",
            "\t\tif (entered_cell == 3)\n"
            "\t\t\tswim_check();"
        )

    # 17. OVL269.CPP: Pic::rle_fast
    if base == "OVL269.CPP":
        rle_fast_c = """    signed short y, x;
    const unsigned char *s = (const unsigned char *)src;
    for (y = 0; y < hh; y++) {
        unsigned char *d = (unsigned char *)dst;
        x = ww;
        while (x > 0) {
            unsigned char al = *s++;
            if (al == 0x7f) {
                unsigned char cnt = *s++;
                unsigned char val = *s++;
                for (unsigned char k = 0; k < cnt; k++)
                    *d++ = val;
                x -= cnt;
            } else {
                *d++ = al;
                x--;
            }
        }
        dst = (unsigned char *)dst + ww;
    }"""
        txt = replace_func_body(txt, r"^void Pic::rle_fast\([^)]*\)\s*\{", rle_fast_c)

    # 18. Clang / WebAssembly C++-Typkompatibilitaet
    if base == "OVL027.CPP":
        txt = txt.replace('load_pic_list(g_cd_cache, g_cl_opening, "\\xaa\\xaa\\xaa", 0, 1)', 'load_pic_list(g_cd_cache, g_cl_opening, (void *)"\\xaa\\xaa\\xaa", 0, 1)')
    elif base == "OVL076.CPP":
        txt = txt.replace("screen_mark_portrait(g_screen,", "screen_mark_portrait((signed char *)g_screen,")
    elif base == "OVL101.CPP":
        txt = txt.replace(": STR_EMPTY_TITLE1,", ": (char *)STR_EMPTY_TITLE1,")
        txt = txt.replace(": STR_EMPTY_TITLE2,", ": (char *)STR_EMPTY_TITLE2,")
    elif base == "OVL173.CPP":
        txt = txt.replace('screen_textbar(g_screen, "", 0xff);', 'screen_textbar(g_screen, (void *)"", 0xff);')
    elif base == "OVL199.CPP":
        for _v in ("s1", "s2", "s3", "s4"):
            txt = txt.replace(f", {_v});", f", (char *){_v});")
    elif base == "OVL202.CPP":
        txt = txt.replace("g_heroes + (signed short)h - 4", "g_heroes + (signed short)(uintptr_t)h - 4")
    elif base == "OVL267.CPP":
        txt = txt.replace('dlg_message(g_dialog, "Wow!");', 'dlg_message(g_dialog, (void *)"Wow!");')
    elif base == "SEG003.CPP":
        txt = txt.replace("set_f8((signed short)txt[0]);", "set_f8((signed short)(uintptr_t)txt[0]);")
    elif base == "OVL037.CPP":
        txt = txt.replace("\thave_arc = 0;\n}", "\thave_arc = 0;\n#ifdef __EMSCRIPTEN__\n\tname_list = 0;\n#endif\n}", 1)
        txt = txt.replace(
            "\tif (id == 0 || have_arc == 0)\n\t\treturn names[id];",
            "#ifdef __EMSCRIPTEN__\n\tif (id == 0 || (have_arc == 0 && name_list == 0))\n#else\n\tif (id == 0 || have_arc == 0)\n#endif\n\t\treturn names[id];",
            1,
        )
    elif base == "OVL220.CPP":
        txt = txt.replace(
            "\t\tif (rose_dir_allowed(p))\n\t\t\trose_draw(p);\n\t\treturn;",
            "\t\tif (rose_dir_allowed(p)) {\n\t\t\trose_draw(p);\n\t\t\treturn;\n\t\t}\n#ifndef __EMSCRIPTEN__\n\t\treturn;\n#endif",
            1,
        )

    txt = strip_dgroup_tables_for_port(base, txt)
    return txt

UNSIZED_DGROUP_ARRAYS = {'g_dng_objs_01': 87, 'g_dng_objs_02': 63, 'g_dng_objs_03': 69, 'g_dng_objs_04': 81, 'g_dng_objs_09': 66, 'g_dng_objs_05': 69, 'g_dng_objs_06': 90, 'g_dng_objs_07': 72, 'g_dng_objs_08': 75, 'g_dng_objs_10': 57, 'g_dng_objs_11': 60, 'g_dng_objs_12': 78, 'g_dng_objs_13': 66, 'g_dng_objs_14': 33, 'g_dng_objs_16': 75, 'g_type_money_01': 18, 'g_type_money_02': 12, 'g_type_money_03': 36, 'g_type_money_04': 24, 'g_type_money_05': 24, 'g_type_money_06': 24, 'g_type_money_07': 6, 'g_type_money_08': 36, 'g_type_money_09': 30, 'g_type_money_10': 30, 'g_type_money_11': 12, 'g_type_money_12': 18, 'g_forbidden_items_01': 60, 'g_forbidden_items_02': 64, 'g_forbidden_items_03': 34, 'g_forbidden_items_04': 62, 'g_forbidden_items_05': 46, 'g_forbidden_items_06': 42, 'g_forbidden_items_07': 164, 'g_forbidden_items_08': 176, 'g_forbidden_items_09': 184, 'g_forbidden_items_10': 60, 'g_forbidden_items_11': 62, 'g_forbidden_items_12': 62, 'g_town_objs_1': 75, 'g_town_objs_2': 81, 'g_town_objs_6': 72, 'g_town_objs_3': 42, 'g_town_objs_4': 87, 'g_town_objs_5': 90, 'g_walk_default': 27, 'g_walk_town5': 13, 'g_walk_town4': 14, 'g_walk_dng_01': 49, 'g_walk_dng_02': 11, 'g_walk_dng_03': 16, 'g_walk_dng_04': 27, 'g_walk_dng_05': 22, 'g_walk_dng_06': 36, 'g_walk_dng_07': 36, 'g_walk_dng_08': 36, 'g_walk_town3': 48, 'g_walk_dng_10': 17, 'g_walk_dng_11': 16, 'g_walk_dng_12': 22, 'g_walk_dng_13': 18, 'g_walk_dng_14': 20, 'g_walk_dng_16': 17, 'g_lvl_spells_witch': 6, 'g_lvl_spells_druid': 8, 'g_lvl_spells_school1': 22, 'g_lvl_spells_school2': 24, 'g_lvl_spells_school3': 24, 'g_lvl_spells_school4': 24, 'g_lvl_spells_school5': 22, 'g_lvl_spells_school6': 20, 'g_lvl_spells_school7': 30, 'g_lvl_spells_school8': 24, 'g_lvl_spells_school9': 26, 'g_lvl_spells_green_elf': 14, 'g_lvl_spells_sylvan_elf': 14, 'g_lvl_spells_ice_elf': 24}
KEEP_PTR_TABLES = ['g_area_descs', 'g_blaster_dest', 'g_chargen_mouse_areas', 'g_chest_fail', 'g_chest_special', 'g_conv_callbacks', 'g_file_names', 'g_fmt_s_s', 'g_fmt_von_s_s', 'g_forbidden_item_lists', 'g_grammar_articles', 'g_grammar_pronouns', 'g_item_use_funcs', 'g_lock_fail', 'g_lvl_spells_school', 'g_mon_spell_handlers', 'g_path_fmts', 'g_spell_handlers', 'g_temple_god_locs', 'g_town_area_descs', 'g_type_money_ranges', 'g_walk_dng']
ELEM_SIZES = {'char': 1, 'signed char': 1, 'unsigned char': 1, 'signed short': 2, 'unsigned short': 2, 'struct view_obj': 3, 'struct money_range': 6, 'struct dual_ammo': 4}
D2_LINE_PATCHES = {'OVL011.CPP': [(90, 91, '\t\t\tprintf(DS_STR(0x05a2), ((signed long)g_ems_free_pages << 4) << 10);\n'), (101, 102, '\t\tprintf(DS_STR(0x05e2), dos_open_files());\n')], 'OVL043.CPP': [(148, 149, '\t\t\t\t\t\tdlg_choose(g_dialog, DS_STR(0x42ce), 1, "Weiter");\n'), (156, 157, '\t\t\t\t\tdlg_choose(g_dialog, DS_STR(0x4309), 1, "Weiter");\n'), (159, 160, '\t\t\t\t\tdlg_choose(g_dialog, DS_STR(0x432b), 1, "Weiter");\n'), (162, 163, '\t\t\t\t\tdlg_choose(g_dialog, DS_STR(0x4355), 1, "Weiter");\n'), (172, 173, '\t\t\tSND_MUSIC->init(DS_STR(0x4387), AIL_XMIDI_DRVR, music_port, music_irq);\n'), (176, 177, '\t\tSND_FX->init(DS_STR(0x4391), AIL_DSP_DRVR, digi_port, digi_irq);\n')], 'OVL084.CPP': [(320, 321, '\tsigned short gear[5];\t/* Grundausstattung jedes Helden (ds:5a16) */\n'), (322, 336, '\tsigned short by_type[12][4];\t/* je Heldentyp 1..12 (Index type - 1), -1 = Ende (ds:5a20) */\n\n\tmemcpy(gear, &g_schweif_dgroup[0x5a16], sizeof(gear));\n\tmemcpy(by_type, &g_schweif_dgroup[0x5a20], sizeof(by_type));\n')], 'OVL096.C': [(249, 250, '\tif ((p = g_shade_buf = mem_alloc(0x2010)) == 0) guru_meditation(DS_STR(0x5e86), 0);\n')], 'OVL097.CPP': [(274, 275, '\t\tguru_meditation(DS_STR(0x5eb2), 0);\n'), (277, 278, '\t\tguru_meditation(DS_STR(0x5ec3), 0);\n'), (298, 299, '\t\tguru_meditation(DS_STR(0x5ed5), 0);\n')], 'OVL106.CPP': [(113, 114, '\t\tif (_write(g_save_fd, DS_STR(0x6151), 4) != 4) good = 0;\n'), (117, 118, '\t\tif (_write(g_save_fd, DS_STR(0x6156), 4) != 4) good = 0;\n'), (121, 122, '\t\tif (_write(g_save_fd, DS_STR(0x615b), 4) != 4) good = 0;\n'), (125, 126, '\t\tif (_write(g_save_fd, DS_STR(0x6160), 4) != 4) good = 0;\n'), (129, 130, '\t\tif (_write(g_save_fd, DS_STR(0x6165), 4) != 4) good = 0;\n'), (136, 137, '\t\t_write(g_save_fd, DS_STR(0x6151), 4);\n'), (140, 141, '\t\t_write(g_save_fd, DS_STR(0x6156), 4);\n'), (144, 145, '\t\t_write(g_save_fd, DS_STR(0x615b), 4);\n'), (148, 149, '\t\t_write(g_save_fd, DS_STR(0x6160), 4);\n'), (152, 153, '\t\t_write(g_save_fd, DS_STR(0x6165), 4);\n')], 'OVL232.C': [(95, 96, '\tenv = getenv(DS_STR(0x7461));\n'), (116, 117, '\t\t\t\t\t\tif (toupper(d) == DS_STR(0x7469)[m])\n')], 'SEG005.CPP': [(77, 79, '\tchar far *msg_insert = DS_STR(0x445b);\n\tchar far *msg_esc = DS_STR(0x449f);\n'), (81, 82, '\tfd = _open(DS_STR(0x44a6), O_RDONLY | O_BINARY);\n'), (102, 104, '\t\t\t\t\t\tputs(DS_STR(0x44af));\n\t\t\t\t} else if (!strcmp(&cfg[1], DS_STR(0x44b0)) || !strcmp(&cfg[1], DS_STR(0x44bc))) {\n'), (112, 113, '\t\t\t\t\t\tputs(DS_STR(0x44c8));\n'), (116, 117, '\t\t\tputs(DS_STR(0x44c9));\n'), (120, 121, '\t\tputs(DS_STR(0x4508));\n'), (211, 213, '\t\tputs(DS_STR(0x4523));\n\t\tputs(DS_STR(0x4561));\n'), (214, 217, '\t\t\tputs(DS_STR(0x4597));\n\t\t\tputs(DS_STR(0x45ca));\n\t\t\tsprintf(buf, DS_STR(0x4612), ((signed long)g_ems_free_pages << 4) << 10);\n'), (218, 219, '\t\t\tsprintf(buf, DS_STR(0x4648), (signed long)xms_kb << 10);\n'), (220, 226, '\t\t\tputs(DS_STR(0x467a));\n\t\t\tputs(DS_STR(0x46c3));\n\t\t\tputs(DS_STR(0x46f6));\n\t\t\tputs(DS_STR(0x4739));\n\t\t\tputs(DS_STR(0x4780));\n\t\t\tputs(DS_STR(0x47c7));\n'), (227, 228, '\t\t\tsprintf(buf, DS_STR(0x47f3), (signed long)g_xms_free_kb << 10);\n'), (234, 235, '\t\t\tsprintf(buf, DS_STR(0x482a), ovr_kb);\n'), (238, 239, '\t\tputs(DS_STR(0x485e));\n'), (248, 249, '\tstrcpy(g_start_dir, DS_STR(0x488b));\n'), (264, 265, '\tsprintf(g_text_output_buf, g_path_fmts[0], DS_STR(0x488f));\n'), (288, 292, '\tfd = open(DS_STR(0x4893), O_RDONLY | O_BINARY);\n\tif (fd != -1) {\n\t\tclose(fd);\n\t\tputs(DS_STR(0x489c));\n'), (297, 298, '\t\tfd = open(DS_STR(0x48ad), O_RDONLY | O_BINARY);\n'), (303, 306, '\t\tload_ltx(DS_STR(0x48b8), &g_names_star, &g_names_star_idx);\n\t}\n\tfd = open(DS_STR(0x48c5), O_RDONLY | O_BINARY);\n'), (313, 314, '\t\tfd = open(DS_STR(0x48cc), O_RDONLY | O_BINARY);\n'), (319, 322, '\t\tload_ltx(DS_STR(0x48d7), &g_names_fx, &g_names_fx_idx);\n\t}\n\tfd = open(DS_STR(0x48e2), O_RDONLY | O_BINARY);\n'), (329, 330, '\t\tfd = open(DS_STR(0x48ed), O_RDONLY | O_BINARY);\n'), (335, 342, '\t\tfd = open(DS_STR(0x48f8), O_RDONLY | O_BINARY);\n\t\tif (fd != -1) {\n\t\t\tclose(fd);\n\t\t\tload_ltx(DS_STR(0x4904), &g_names_speech, &g_names_speech_idx);\n\t\t}\n\t}\n\tfd = open(DS_STR(0x4910), O_RDONLY | O_BINARY);\n'), (349, 350, '\t\tfd = open(DS_STR(0x4918), O_RDONLY | O_BINARY);\n'), (355, 356, '\t\tload_ltx(DS_STR(0x4923), &g_names_raw, &g_names_raw_idx);\n'), (366, 367, '\t\tfd = open(DS_STR(0x492f), O_RDONLY | O_BINARY);\n'), (386, 387, '\t\t\t\t\tsprintf(path.s, g_file_names[4], DS_STR(0x493b));\n'), (392, 393, '\t\t\t\t\t\tload_ltx(DS_STR(0x4945), &g_names_starcd, &g_names_starcd_idx);\n'), (397, 398, '\t\t\tfd = open(DS_STR(0x4951), O_RDONLY | O_BINARY);\n'), (400, 401, '\t\t\t\tload_ltx(DS_STR(0x495d), &g_names_starcd, &g_names_starcd_idx);\n'), (405, 406, '\t\t\tfd = open(DS_STR(0x4969), O_RDONLY | O_BINARY);\n'), (412, 413, '\tfd = open(DS_STR(0x4973), O_RDONLY | O_BINARY);\n'), (414, 415, '\t\tsprintf(path.s, g_file_names[4], DS_STR(0x4980));\n'), (431, 435, '\t\tfd = open(DS_STR(0x498d), O_RDONLY | O_BINARY);\n\t\tif (fd != -1) {\n\t\t\tclose(fd);\n\t\t\tload_ltx(DS_STR(0x4999), &g_names_speechcd, &g_names_speechcd_idx);\n'), (443, 444, '\tfd = open(DS_STR(0x49a5), O_RDONLY | O_BINARY);\n'), (451, 452, '\tfd = open(DS_STR(0x49b2), O_RDONLY | O_BINARY);\n')], 'SEG007.C': [(53, 54, '\t\t\tsprintf(g_text_output_buf, DS_STR(0x5158), size, farcoreleft());\n'), (92, 93, '\t\t\t\t\tguru_meditation(DS_STR(0x518c), p);\n'), (125, 126, '\t\t\t\tguru_meditation(DS_STR(0x51e3), p);\n'), (163, 164, '\tguru_meditation(DS_STR(0x5208), 0);\n'), (182, 183, '\t\t\t\t\t\tguru_meditation(DS_STR(0x522d), start);\n')], 'SEG014.C': [(61, 62, '\tcputs(DS_STR(0x5f24));\n'), (71, 72, '\tsprintf(g_portrait_buf, DS_STR(0x5f5d), g_func_trace[0], g_func_trace[1], g_func_trace[2],\n'), (75, 76, '\tsprintf(g_portrait_buf, DS_STR(0x5f95), msg, g_func_trace[0], g_func_trace[1], g_func_trace[2],\n'), (81, 82, '\t\tcprintf(DS_STR(0x5fcf), g_dbg_dest);\n'), (83, 84, '\t\tcprintf(DS_STR(0x5fdf), g_dbg_src);\n'), (89, 90, '\t\tfd = _creat(DS_STR(0x5ff1), 0);\n'), (91, 92, '\t\t\tsprintf(buf, DS_STR(0x5ffb), i, g_mem_blocks[i].start, g_mem_blocks[i].end,\n'), (102, 104, '\t\tcprintf(DS_STR(0x6028), ptr);\n\t\tsprintf(g_portrait_buf, DS_STR(0x603b), ptr);\n'), (105, 106, '\t\tdump_fd = _creat(DS_STR(0x604e), 0);\n')], 'SEG020.CPP': [(53, 54, '\t\tcode = ((unsigned char far *)DS_STR(0x75de))[i];\n')]}

def strip_dgroup_tables_for_port(base, txt):
    m = re.search(r"/\* Phase B: eigene Daten des Moduls \(ds:([0-9a-fA-F]+)\.\.([0-9a-fA-F]+).*?\*/", txt, flags=re.S)
    if m:
        pos = m.end()
        rest = txt[pos:]
        out_parts = [txt[:pos]]
        i = 0
        n = len(rest)
        changed = False
        while i < n:
            ws_start = i
            while i < n and rest[i] in " \t\r\n":
                i += 1
            out_parts.append(rest[ws_start:i])
            if i >= n:
                break
            if rest.startswith("/*", i):
                end_c = rest.find("*/", i + 2)
                end_c = end_c + 2 if end_c != -1 else n
                out_parts.append(rest[i:end_c]); i = end_c; continue
            if rest.startswith("//", i):
                end_l = rest.find("\n", i + 2)
                end_l = end_l + 1 if end_l != -1 else n
                out_parts.append(rest[i:end_l]); i = end_l; continue
            if rest[i] == "#":
                p_start = i
                while i < n:
                    end_l = rest.find("\n", i)
                    if end_l == -1: i = n; break
                    if rest[end_l - 1] == "\\": i = end_l + 1
                    else: i = end_l + 1; break
                out_parts.append(rest[p_start:i]); continue
            stmt_start = i
            paren_d = 0; brace_d = 0; in_str = False; str_ch = ""; eq_pos = -1
            while i < n:
                ch = rest[i]
                if in_str:
                    if ch == "\\": i += 2; continue
                    if ch == str_ch: in_str = False
                else:
                    if ch in ("\"", "\x27"): in_str = True; str_ch = ch
                    elif rest.startswith("/*", i):
                        end_c = rest.find("*/", i + 2); i = end_c + 2 if end_c != -1 else n; continue
                    elif rest.startswith("//", i):
                        end_l = rest.find("\n", i + 2); i = end_l + 1 if end_l != -1 else n; continue
                    elif ch == "(": paren_d += 1
                    elif ch == ")": paren_d -= 1
                    elif ch == "{":
                        if paren_d == 0 and eq_pos == -1: break
                        brace_d += 1
                    elif ch == "}": brace_d -= 1
                    elif ch == "=" and paren_d == 0 and brace_d == 0 and eq_pos == -1: eq_pos = i
                    elif ch == ";" and paren_d == 0 and brace_d == 0: i += 1; break
                i += 1
            stmt = rest[stmt_start:i]
            if not stmt.rstrip().endswith(";"):
                if stmt.lstrip().startswith(("struct ", "class ", "union ", "enum ", "typedef ")):
                    while i < n:
                        if rest[i] == "{": brace_d += 1
                        elif rest[i] == "}":
                            brace_d -= 1
                            if brace_d == 0:
                                semi = rest.find(";", i); i = semi + 1 if semi != -1 else n; break
                        i += 1
                    out_parts.append(rest[stmt_start:i]); continue
                else:
                    out_parts.append(rest[stmt_start:]); break
            if eq_pos == -1:
                out_parts.append(stmt); continue
            lhs = rest[stmt_start:eq_pos].strip()
            rhs = rest[eq_pos + 1 : i - 1].strip()
            m_var = re.search(r"\b(g_[A-Za-z0-9_]+)\s*(?:\[[^\]]*\])*$", lhs)
            if not m_var:
                out_parts.append(stmt); continue
            var_name = m_var.group(1)
            if var_name in KEEP_PTR_TABLES:
                out_parts.append(stmt); continue
            changed = True
            if var_name == "g_dng_view_areas":
                new_rhs = "{\n" + ",\n".join(
                    f"\t{{ 0, 0, 0, 0, 0, 0, 0, g_dng_objs_{k} }}"
                    for k in ["01","02","03","04","05","06","07","08","09","10","11","12","13","14","14","16"]
                ) + "\n}"
                out_parts.append(f"{lhs} = {new_rhs};")
            elif var_name == "g_town_view_areas":
                new_rhs = "{\n" + ",\n".join(
                    f"\t{{ 0, 0, 0, 0, 0, 0, 0, g_town_objs_{k} }}" for k in range(1, 7)
                ) + "\n}"
                out_parts.append(f"{lhs} = {new_rhs};")
            elif var_name == "g_fli_screen":
                out_parts.append(f"{lhs} = {{ 0, 0, 0, 0, 0, g_fli_palette }};")
            elif var_name == "g_opt_entries":
                ptrs = re.findall(r"\(signed char far \*\)&g_[A-Za-z0-9_]+", rhs)
                rows = [f"\t{{ 0, 0, 0, {p}, {{ 0, 0, 0, 0 }} }}" for p in ptrs]
                out_parts.append(f"{lhs} = {{\n" + ",\n".join(rows) + "\n};")
            else:
                if "[]" in lhs:
                    m_u = re.match(r"^(.*?)\s+(g_[A-Za-z0-9_]+)\[\]$", lhs)
                    tp, nm = m_u.group(1).strip(), m_u.group(2)
                    cnt = UNSIZED_DGROUP_ARRAYS[nm] // ELEM_SIZES[tp]
                    lhs = f"{tp} {nm}[{cnt}]"
                is_agg = ("[" in lhs) or lhs.startswith(("struct ", "union "))
                z = "{ 0 }" if is_agg else "0"
                out_parts.append(f"{lhs} = {z};")
        if changed:
            txt = "".join(out_parts)
    if base in D2_LINE_PATCHES:
        lines = txt.splitlines(True)
        for i1, i2, rep_s in reversed(D2_LINE_PATCHES[base]):
            lines[i1:i2] = [rep_s]
        txt = "".join(lines)
    return txt


def main():
    os.makedirs(os.path.join(PORT_SRC, "inc"), exist_ok=True)
    os.makedirs(os.path.join(PORT_SRC, "Overlays"), exist_ok=True)

    # 1. Header kopieren und anpassen
    for h_path in sorted(glob.glob(os.path.join(QUELLE, "inc", "*.h"))):
        name = os.path.basename(h_path)
        txt = read_latin1(h_path)
        if name == "schweif.h":
            txt = transform_schweif_h(txt)
        elif name == "s_struct.h":
            txt = transform_s_struct_h(txt)
        elif name == "s_data.h":
            txt = transform_s_data_h(txt)
        elif name == "s_proto.h":
            txt = transform_s_proto_h(txt)
        write_latin1(os.path.join(PORT_SRC, "inc", name), txt)

    # 2. C/C++-Module kopieren und anpassen
    src_files = sorted(
        glob.glob(os.path.join(QUELLE, "*.C")) +
        glob.glob(os.path.join(QUELLE, "*.CPP")) +
        glob.glob(os.path.join(QUELLE, "Overlays", "*.C")) +
        glob.glob(os.path.join(QUELLE, "Overlays", "*.CPP"))
    )
    for p in src_files:
        rel = os.path.relpath(p, QUELLE)
        txt = read_latin1(p)
        txt = transform_source(rel, txt)
        write_latin1(os.path.join(PORT_SRC, rel), txt)

    print(f"[prepare_sources] {len(src_files)} C/C++-Module und Header nach Port/src/ vorbereitet.")


if __name__ == "__main__":
    main()
