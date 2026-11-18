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

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
QUELLE = os.path.join(ROOT, "Quelle")
PORT_SRC = os.path.join(ROOT, "Port", "src")


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
#ifdef __cplusplus
extern "C" {
#endif
void *dsa_mk_fp(unsigned short seg, unsigned long off);
void dsa_log_stub(const char *func_name);
void dsa_log_guru_meditation(const char *msg, const void *ptr);
uint32_t dsa_platform_init_seed(uint32_t default_seed);
extern unsigned char *g_port_shade_base;
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
        set_pal_c = """    signed short i;
    for (i = 0; i < count; i++) {
        PAL[i * 3] &= 0x3f;
        (PAL + i * 3)[1] &= 0x3f;
        (PAL + i * 3)[2] &= 0x3f;
    }
    vga_set_palette(pal, first, count);
    if (g_palette && g_palette != pal) {
        memcpy(g_palette + first * 3, pal, count * 3);
    }"""
        set_col_c = """    RGB[0] &= 0x3f;
    RGB[1] &= 0x3f;
    RGB[2] &= 0x3f;
    vga_set_color(rgb, nr);
    if (g_palette) {
        memcpy(g_palette + nr * 3, rgb, 3);
    }"""
        txt = replace_func_body(txt, r"^void set_palette\([^)]*\)\s*\{", set_pal_c)
        txt = replace_func_body(txt, r"^void set_color\([^)]*\)\s*\{", set_col_c)

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

    # 14. OVL032.CPP: main -> dsa_game_main, dos_files_total, W4 Replay-Seed, B1 Intro, B4 Schreibschutz-Hinweis
    elif base == "OVL032.CPP":
        txt = txt.replace(
            "void main(register int argc, char **argv)",
            'extern "C" int dsa_platform_should_play_intro(int argc, int first_start);\nextern "C" int dsa_platform_consume_readonly_notice(void);\nextern "C" int dsa_game_main(int argc, char **argv)'
        )
        txt = txt.replace("srand(time(0));", "srand(dsa_platform_init_seed((uint32_t)time(0)));")
        txt = txt.replace(
            "\t\tif ((argc > 1 && (argv[1][0] == '-' || argv[1][0] == '/')) || (g_first_start != 0 && argc <= 1)) {",
            "\t\tif ((argc > 1 && (argv[1][0] == '-' || argv[1][0] == '/')) || dsa_platform_should_play_intro(argc, g_first_start)) {"
        )
        txt = txt.replace(
            "\t\t\tmain_loop();",
            '\t\t\tif (dsa_platform_consume_readonly_notice()) {\n\t\t\t\tdlg_message(g_dialog, (char *)"Hinweis: Der Spielordner ist schreibgesch\\x81tzt.@Spielst\\x84nde und PROBEN.CSV werden im Ordner@<Spielstaende> neben der Programmdatei gespeichert.");\n\t\t\t}\n\t\t\tmain_loop();'
        )
        txt = replace_func_body(txt, r"^signed short dos_files_total\([^)]*\)\s*\{", "    return 40;")

    # 15. OVL096.C: view_alloc_lists + view_load_palette + pal_find_nearest
    elif base == "OVL096.C":
        alloc_lists_c = """    void *p;
    if ((p = mem_alloc(0x61c)) == 0) guru_meditation("OUT OF MEMORY", 0);
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
