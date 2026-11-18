/*
 * dsa_platform_sdl2.cpp - SDL2-Implementierung der gemeinsamen Plattformschicht
 * der Nordlandtrilogie-Ports fuer DSA 2: Sternenschweif.
 */
#define SDL_MAIN_HANDLED
#include <SDL.h>

#include "dsa_platform.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <direct.h>
#include <io.h>
#define STRICMP  _stricmp
#define STRNICMP _strnicmp
#else
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <strings.h>
#define STRICMP  strcasecmp
#define STRNICMP strncasecmp
#endif

static SDL_Window   *g_window      = NULL;
static SDL_Renderer *g_renderer    = NULL;
static SDL_Texture  *g_texture     = NULL;
static SDL_Texture  *g_display_tex = NULL;
static uint32_t     *g_display_buf = NULL;
static int           g_display_w   = 0;
static int           g_display_h   = 0;

#define DSA_SKAL_GANZZAHLIG 0
#define DSA_SKAL_FUELLEND   1

static int g_display_inited   = 0;
static int g_is_fullscreen    = 1;
static int g_fenster_faktor   = 4;
static int g_fenster_explicit = 0;
static int g_skalierung_mode  = DSA_SKAL_FUELLEND;
static int g_vp_x = 0, g_vp_y = 0, g_vp_w = 1280, g_vp_h = 960;

static uint8_t  g_vram[DSA_VRAM_SIZE];
static uint8_t  g_pal6[256 * 3];
static uint32_t g_argb_pal[256];
static uint32_t g_argb_buf[DSA_VRAM_SIZE];

static int g_mouse_x = 160;
static int g_mouse_y = 100;
static int g_mouse_buttons = 0;
static int g_left_clicks = 0;
static int g_right_clicks = 0;
static int g_arrow_mask = 0;

/* Ringpuffer fuer BIOS-Tastencodes (High-Byte = Scancode, Low-Byte = ASCII) */
#define KEYBUF_SIZE 64
static uint16_t g_keybuf[KEYBUF_SIZE];
static int g_key_head = 0;
static int g_key_tail = 0;
static int g_key_mods = 0;

static uint32_t g_start_ms = 0;
static uint32_t g_last_present_ms = 0;
static uint32_t g_present_count = 0;
static uint32_t g_virtual_ms = 0;
static uint32_t g_idle_polls = 0;
static void (*g_tick_hook)(void) = NULL;
static int g_in_tick_hook = 0;

static char g_gamedir[1024] = "C:\\Program Files (x86)\\GOG Galaxy\\Games\\Realms of Arkania 2";
static char g_launch_cwd[1024] = "";

/* W4: Aufzeichnung (--record) & Wiedergabe (--replay) */
struct replay_event {
    uint32_t frame;
    char     type; /* 'M' = Maus, 'K' = Taste, 'B' = BMP-Schnappschuss, 'E' = Ende */
    int      mx, my, mb, lclicks, rclicks, arrows, mods;
    uint16_t key;
    char     bmp_path[256];
};

static FILE *g_record_fp = NULL;
static int   g_prev_rec_mx = -1, g_prev_rec_my = -1, g_prev_rec_mb = -1, g_prev_rec_arrows = 0, g_prev_rec_mods = 0;
static struct replay_event *g_replay_events = NULL;
static size_t g_replay_count = 0;
static size_t g_replay_cap   = 0;
static size_t g_replay_pos   = 0;
static int    g_replay_active = 0;
static int    g_replay_has_seed = 0;
static uint32_t g_replay_seed = 0;

static void push_key(uint16_t code) {
    int next = (g_key_head + 1) % KEYBUF_SIZE;
    if (next != g_key_tail) {
        g_keybuf[g_key_head] = code;
        g_key_head = next;
    }
    if (g_record_fp) {
        fprintf(g_record_fp, "F %u K 0x%04X\n", g_present_count, (unsigned)code);
        fflush(g_record_fp);
    }
}

#ifndef SDL_DISABLE
#define SDL_DISABLE 0
#define SDL_ENABLE  1
#endif

#ifndef SDL_SCANCODE_A
#define SDL_SCANCODE_A          4
#define SDL_SCANCODE_Z          29
#define SDL_SCANCODE_1          30
#define SDL_SCANCODE_9          38
#define SDL_SCANCODE_0          39
#define SDL_SCANCODE_RETURN     40
#define SDL_SCANCODE_ESCAPE     41
#define SDL_SCANCODE_BACKSPACE  42
#define SDL_SCANCODE_TAB        43
#define SDL_SCANCODE_SPACE      44
#define SDL_SCANCODE_F1         58
#define SDL_SCANCODE_F2         59
#define SDL_SCANCODE_F3         60
#define SDL_SCANCODE_F4         61
#define SDL_SCANCODE_F5         62
#define SDL_SCANCODE_F6         63
#define SDL_SCANCODE_F7         64
#define SDL_SCANCODE_F8         65
#define SDL_SCANCODE_F9         66
#define SDL_SCANCODE_F10        67
#define SDL_SCANCODE_INSERT     73
#define SDL_SCANCODE_HOME       74
#define SDL_SCANCODE_PAGEUP     75
#define SDL_SCANCODE_DELETE     76
#define SDL_SCANCODE_END        77
#define SDL_SCANCODE_PAGEDOWN   78
#define SDL_SCANCODE_RIGHT      79
#define SDL_SCANCODE_LEFT       80
#define SDL_SCANCODE_DOWN       81
#define SDL_SCANCODE_UP         82
#define SDL_SCANCODE_KP_ENTER   88
#endif

static void write_bmp_320x200(const char *path, const uint32_t *argb) {
    char resolved[1024];
    if (g_launch_cwd[0] && path && !(isalpha((unsigned char)path[0]) && path[1] == ':') && path[0] != '\\' && path[0] != '/') {
        snprintf(resolved, sizeof(resolved), "%s\\%s", g_launch_cwd, path);
        path = resolved;
    }
    FILE *fp = fopen(path, "wb");
    if (!fp) return;
    const int w = DSA_SCREEN_W, h = DSA_SCREEN_H;
    const int row_bytes = w * 3;
    const int data_size = row_bytes * h;
    const int file_size = 54 + data_size;
    uint8_t hdr[54];
    memset(hdr, 0, sizeof(hdr));
    hdr[0] = 'B'; hdr[1] = 'M';
    hdr[2] = (uint8_t)(file_size); hdr[3] = (uint8_t)(file_size >> 8);
    hdr[4] = (uint8_t)(file_size >> 16); hdr[5] = (uint8_t)(file_size >> 24);
    hdr[10] = 54;
    hdr[14] = 40;
    hdr[18] = (uint8_t)(w); hdr[19] = (uint8_t)(w >> 8);
    hdr[22] = (uint8_t)(h); hdr[23] = (uint8_t)(h >> 8);
    hdr[26] = 1;
    hdr[28] = 24;
    fwrite(hdr, 1, 54, fp);
    uint8_t row[DSA_SCREEN_W * 3];
    for (int y = h - 1; y >= 0; --y) {
        for (int x = 0; x < w; ++x) {
            uint32_t c = argb[y * w + x];
            row[x * 3 + 0] = (uint8_t)(c & 0xFF);
            row[x * 3 + 1] = (uint8_t)((c >> 8) & 0xFF);
            row[x * 3 + 2] = (uint8_t)((c >> 16) & 0xFF);
        }
        fwrite(row, 1, row_bytes, fp);
    }
    fclose(fp);
}

static uint16_t sdl_to_bios_key(SDL_Keysym sym) {
    if (sym.scancode < 0) {
        return (uint16_t)(sym.unused & 0xFFFF);
    }
    uint8_t scan = 0;
    uint8_t ascii = 0;
    int shift = (sym.mod & KMOD_SHIFT) != 0;

    switch (sym.scancode) {
        case SDL_SCANCODE_ESCAPE:    scan = 0x01; ascii = 27; break;
        case SDL_SCANCODE_RETURN:
        case SDL_SCANCODE_KP_ENTER:  scan = 0x1C; ascii = 13; break;
        case SDL_SCANCODE_BACKSPACE: scan = 0x0E; ascii = 8;  break;
        case SDL_SCANCODE_TAB:       scan = 0x0F; ascii = 9;  break;
        case SDL_SCANCODE_SPACE:     scan = 0x39; ascii = ' '; break;

        case SDL_SCANCODE_UP:        scan = 0x48; ascii = 0; break;
        case SDL_SCANCODE_DOWN:      scan = 0x50; ascii = 0; break;
        case SDL_SCANCODE_LEFT:      scan = 0x4B; ascii = 0; break;
        case SDL_SCANCODE_RIGHT:     scan = 0x4D; ascii = 0; break;
        case SDL_SCANCODE_HOME:      scan = 0x47; ascii = 0; break;
        case SDL_SCANCODE_END:       scan = 0x4F; ascii = 0; break;
        case SDL_SCANCODE_PAGEUP:    scan = 0x49; ascii = 0; break;
        case SDL_SCANCODE_PAGEDOWN:  scan = 0x51; ascii = 0; break;
        case SDL_SCANCODE_INSERT:    scan = 0x52; ascii = 0; break;
        case SDL_SCANCODE_DELETE:    scan = 0x53; ascii = 0; break;

        case SDL_SCANCODE_F1:        scan = 0x3B; ascii = 0; break;
        case SDL_SCANCODE_F2:        scan = 0x3C; ascii = 0; break;
        case SDL_SCANCODE_F3:        scan = 0x3D; ascii = 0; break;
        case SDL_SCANCODE_F4:        scan = 0x3E; ascii = 0; break;
        case SDL_SCANCODE_F5:        scan = 0x3F; ascii = 0; break;
        case SDL_SCANCODE_F6:        scan = 0x40; ascii = 0; break;
        case SDL_SCANCODE_F7:        scan = 0x41; ascii = 0; break;
        case SDL_SCANCODE_F8:        scan = 0x42; ascii = 0; break;
        case SDL_SCANCODE_F9:        scan = 0x43; ascii = 0; break;
        case SDL_SCANCODE_F10:       scan = 0x44; ascii = 0; break;

        default: {
            static const uint8_t letter_scans[26] = {
                0x1E, 0x30, 0x2E, 0x20, 0x12, 0x21, 0x22, 0x23, 0x17, 0x24,
                0x25, 0x26, 0x32, 0x31, 0x18, 0x19, 0x10, 0x13, 0x1F, 0x14,
                0x16, 0x2F, 0x11, 0x2D, 0x15, 0x2C
            };
            if (sym.scancode >= SDL_SCANCODE_A && sym.scancode <= SDL_SCANCODE_Z) {
                int idx = sym.scancode - SDL_SCANCODE_A;
                scan = letter_scans[idx];
                ascii = (uint8_t)(shift ? ('A' + idx) : ('a' + idx));
            } else if (sym.scancode >= SDL_SCANCODE_1 && sym.scancode <= SDL_SCANCODE_9) {
                int idx = sym.scancode - SDL_SCANCODE_1;
                scan = (uint8_t)(0x02 + idx);
                ascii = (uint8_t)('1' + idx);
            } else if (sym.scancode == SDL_SCANCODE_0) {
                scan = 0x0B;
                ascii = '0';
            } else if (sym.sym == 0xE4 || sym.sym == 0xC4) {
                scan = 0x28; ascii = shift ? 0x8E : 0x84; /* Ä / ä (CP437) */
            } else if (sym.sym == 0xF6 || sym.sym == 0xD6) {
                scan = 0x27; ascii = shift ? 0x99 : 0x94; /* Ö / ö (CP437) */
            } else if (sym.sym == 0xFC || sym.sym == 0xDC) {
                scan = 0x1A; ascii = shift ? 0x9A : 0x81; /* Ü / ü (CP437) */
            } else if (sym.sym == 0xDF) {
                scan = 0x0C; ascii = 0xE1;                /* ß (CP437) */
            } else if (sym.sym >= 32 && sym.sym < 127) {
                scan = 0x35; ascii = (uint8_t)sym.sym;
            }
            break;
        }
    }

    if (scan == 0 && ascii == 0)
        return 0;
    return (uint16_t)((scan << 8) | ascii);
}

static void try_load_spielpfad(void) {
    const char *env_dir = getenv("DSA_GAMEDIR");
    if (env_dir && env_dir[0]) {
        dsa_platform_set_gamedir(env_dir);
        return;
    }
    const char *candidates[] = {
        "SPIELPFAD.txt",
        "..\\SPIELPFAD.txt",
        "..\\..\\SPIELPFAD.txt",
        "../SPIELPFAD.txt",
        "../../SPIELPFAD.txt",
        NULL
    };
    for (int i = 0; candidates[i]; ++i) {
        FILE *f = fopen(candidates[i], "rb");
        if (f) {
            char line[1024];
            while (fgets(line, sizeof(line), f)) {
                char *p = line;
                while (*p == ' ' || *p == '\t') ++p;
                if (*p == '#' || *p == '\r' || *p == '\n' || *p == '\0')
                    continue;
                size_t len = strlen(p);
                while (len > 0 && (p[len - 1] == '\r' || p[len - 1] == '\n' || p[len - 1] == ' '))
                    p[--len] = '\0';
                if (len > 0) {
                    dsa_platform_set_gamedir(p);
                    break;
                }
            }
            fclose(f);
            return;
        }
    }
}

static void dsa_parse_fenster_token(const char *val) {
    if (!val || !val[0]) return;
    if (STRICMP(val, "vollbild") == 0 || STRICMP(val, "fullscreen") == 0) {
        g_is_fullscreen    = 1;
        g_fenster_explicit = 1;
    } else {
        int f = atoi(val);
        if (f >= 2 && f <= 5) {
            g_fenster_faktor   = f;
            g_is_fullscreen    = 0;
            g_fenster_explicit = 1;
        }
    }
}

static void dsa_parse_skalierung_token(const char *val) {
    if (!val || !val[0]) return;
    if (STRICMP(val, "ganzzahlig") == 0 || STRICMP(val, "integer") == 0 || strcmp(val, "0") == 0) {
        g_skalierung_mode = DSA_SKAL_GANZZAHLIG;
    } else if (STRICMP(val, "fuellend") == 0 || STRICMP(val, "füllend") == 0 || STRICMP(val, "sharp") == 0 || strcmp(val, "1") == 0) {
        g_skalierung_mode = DSA_SKAL_FUELLEND;
    }
}

static void dsa_display_init_from_env(void) {
    if (g_display_inited) return;
    g_display_inited = 1;

    g_is_fullscreen   = 1;
    g_fenster_faktor  = 4;
    g_skalierung_mode = DSA_SKAL_FUELLEND;

    const char *env_fs = getenv("DSA_FULLSCREEN");
    if (env_fs && env_fs[0]) {
        if (env_fs[0] == '0') { g_is_fullscreen = 0; g_fenster_explicit = 1; }
        else if (env_fs[0] == '1') { g_is_fullscreen = 1; g_fenster_explicit = 1; }
    }
    const char *env_fen = getenv("DSA_FENSTER");
    if (env_fen && env_fen[0]) {
        dsa_parse_fenster_token(env_fen);
    }
    const char *env_skal = getenv("DSA_SKALIERUNG");
    if (env_skal && env_skal[0]) {
        dsa_parse_skalierung_token(env_skal);
    }
    const char *env_fres = getenv("DSA_VOLLBILD_AUFLOESUNG");
    if (env_fres && env_fres[0]) {
        int fw = 0, fh = 0;
        if (sscanf(env_fres, "%dx%d", &fw, &fh) == 2 && fw >= 320 && fh >= 240) {
            SDL_SetFullscreenOverrideRes(fw, fh);
        }
    }

#ifdef _WIN32
    const char *cmdline = GetCommandLineA();
    if (cmdline) {
        const char *p = cmdline;
        while (*p) {
            while (*p == ' ' || *p == '\t') ++p;
            if (!*p) break;
            char tok[512];
            int ti = 0, in_q = 0;
            while (*p && (in_q || (*p != ' ' && *p != '\t'))) {
                if (*p == '"') { in_q = !in_q; ++p; continue; }
                if (ti < (int)sizeof(tok) - 1) tok[ti++] = *p;
                ++p;
            }
            tok[ti] = '\0';

            if (STRNICMP(tok, "--fenster=", 10) == 0) {
                dsa_parse_fenster_token(tok + 10);
            } else if (STRICMP(tok, "--vollbild") == 0 || STRICMP(tok, "--fullscreen") == 0) {
                g_is_fullscreen = 1;
                g_fenster_explicit = 1;
            } else if (STRICMP(tok, "--fenstermodus") == 0 || STRICMP(tok, "--windowed") == 0) {
                g_is_fullscreen = 0;
                g_fenster_explicit = 1;
            } else if (STRNICMP(tok, "--skalierung=", 13) == 0) {
                dsa_parse_skalierung_token(tok + 13);
            } else if (STRNICMP(tok, "--vollbild-aufloesung=", 22) == 0) {
                int fw = 0, fh = 0;
                if (sscanf(tok + 22, "%dx%d", &fw, &fh) == 2 && fw >= 320 && fh >= 240) {
                    SDL_SetFullscreenOverrideRes(fw, fh);
                }
            }
        }
    }
#endif

    int is_at = (getenv("DSA_PORT_AUTOTEST") && getenv("DSA_PORT_AUTOTEST")[0] == '1') ||
                (getenv("DSA_HIDDEN") && getenv("DSA_HIDDEN")[0] == '1');
    if ((g_replay_active || is_at) && !g_fenster_explicit) {
        g_is_fullscreen = 0;
    }
}

static void dsa_compute_43_viewport(int cw, int ch, int *out_vx, int *out_vy, int *out_vw, int *out_vh) {
    if (cw < 320) cw = 320;
    if (ch < 240) ch = 240;
    int vw = cw, vh = ch;

    if (g_skalierung_mode == DSA_SKAL_GANZZAHLIG) {
        int sx = cw / 320;
        int sy43 = ch / 240;
        if (sy43 < sx) sx = sy43;
        if (sx < 1) sx = 1;
        vw = 320 * sx;
        vh = 240 * sx;
    } else {
        if (cw * 3 <= ch * 4) {
            vh = (cw * 3) / 4;
            vw = (vh * 4) / 3;
        } else {
            vw = (ch * 4) / 3;
            vh = (vw * 3) / 4;
        }
        if (vw < 320) { vw = 320; vh = 240; }
    }

    *out_vx = (cw - vw) / 2;
    *out_vy = (ch - vh) / 2;
    *out_vw = vw;
    *out_vh = vh;
}

static void dsa_render_scaled_display_buffer(int cw, int ch) {
    if (cw < 320) cw = 320;
    if (ch < 240) ch = 240;

    static int s_last_vx = -1, s_last_vy = -1, s_last_vw = -1, s_last_vh = -1;
    bool buf_reallocated = false;

    if (!g_display_buf || g_display_w != cw || g_display_h != ch) {
        if (g_display_buf) free(g_display_buf);
        g_display_buf = (uint32_t *)calloc((size_t)cw * (size_t)ch, sizeof(uint32_t));
        g_display_w = cw;
        g_display_h = ch;
        buf_reallocated = true;
        if (g_display_tex) {
            SDL_DestroyTexture(g_display_tex);
            g_display_tex = NULL;
        }
    }
    if (!g_display_tex && g_renderer) {
        if (SDL_PortUsingExternalDLL()) {
            SDL_RenderSetLogicalSize(g_renderer, cw, ch);
        } else {
            SDL_RenderSetLogicalSize(g_renderer, DSA_SCREEN_W, DSA_SCREEN_H);
        }
        g_display_tex = SDL_CreateTexture(
            g_renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, cw, ch
        );
    }

    int vx = 0, vy = 0, vw = cw, vh = ch;
    dsa_compute_43_viewport(cw, ch, &vx, &vy, &vw, &vh);
    g_vp_x = vx; g_vp_y = vy; g_vp_w = vw; g_vp_h = vh;
    SDL_SetWindowViewport(g_window, vx, vy, vw, vh);

    if (buf_reallocated || vx != s_last_vx || vy != s_last_vy || vw != s_last_vw || vh != s_last_vh) {
        s_last_vx = vx; s_last_vy = vy; s_last_vw = vw; s_last_vh = vh;
        if (vx > 0 || vy > 0 || vw < cw || vh < ch) {
            for (int i = 0; i < cw * ch; ++i) {
                g_display_buf[i] = 0xFF000000u;
            }
        }
    }

    int sw = DSA_SCREEN_W;
    int sh = DSA_SCREEN_H;

    if (g_skalierung_mode == DSA_SKAL_GANZZAHLIG) {
        int prev_sy = -1;
        uint32_t *prev_drow = NULL;
        for (int dy = 0; dy < vh; ++dy) {
            int sy = (dy * sh) / vh;
            uint32_t *drow = g_display_buf + (vy + dy) * cw + vx;
            if (sy == prev_sy && prev_drow) {
                memcpy(drow, prev_drow, (size_t)vw * sizeof(uint32_t));
            } else {
                const uint32_t *srow = g_argb_buf + sy * sw;
                for (int dx = 0; dx < vw; ++dx) {
                    int sx = (dx * sw) / vw;
                    drow[dx] = srow[sx];
                }
                prev_sy = sy;
                prev_drow = drow;
            }
        }
    } else {
        static int x0_tab[4096], x1_tab[4096], wx_tab[4096];
        static int cached_vw = -1, cached_sw = -1;
        int max_vw = (vw < 4096) ? vw : 4096;
        if (vw != cached_vw || sw != cached_sw) {
            cached_vw = vw;
            cached_sw = sw;
            for (int dx = 0; dx < max_vw; ++dx) {
                int s0 = (dx * sw) / vw;
                int s1 = ((dx + 1) * sw - 1) / vw;
                if (s0 >= sw) s0 = sw - 1;
                if (s1 >= sw) s1 = sw - 1;
                x0_tab[dx] = s0;
                x1_tab[dx] = s1;
                wx_tab[dx] = (s0 == s1) ? 0 : ((((dx + 1) * sw - s1 * vw) * 256) / sw);
            }
        }

        int prev_pure_y0 = -1;
        uint32_t *prev_pure_drow = NULL;

        for (int dy = 0; dy < vh; ++dy) {
            int y0 = (dy * sh) / vh;
            int y1 = ((dy + 1) * sh - 1) / vh;
            if (y0 >= sh) y0 = sh - 1;
            if (y1 >= sh) y1 = sh - 1;
            int wy = (y0 == y1) ? 0 : ((((dy + 1) * sh - y1 * vh) * 256) / sh);

            uint32_t *drow = g_display_buf + (vy + dy) * cw + vx;

            if (wy == 0) {
                if (y0 == prev_pure_y0 && prev_pure_drow) {
                    memcpy(drow, prev_pure_drow, (size_t)max_vw * sizeof(uint32_t));
                } else {
                    const uint32_t *row0 = g_argb_buf + y0 * sw;
                    for (int dx = 0; dx < max_vw; ++dx) {
                        int wx = wx_tab[dx];
                        if (wx == 0) {
                            drow[dx] = row0[x0_tab[dx]];
                        } else {
                            uint32_t c0 = row0[x0_tab[dx]];
                            uint32_t c1 = row0[x1_tab[dx]];
                            uint32_t r = (((c0 >> 16) & 0xFF) * (256 - wx) + ((c1 >> 16) & 0xFF) * wx) >> 8;
                            uint32_t g = (((c0 >> 8)  & 0xFF) * (256 - wx) + ((c1 >> 8)  & 0xFF) * wx) >> 8;
                            uint32_t b = ((c0         & 0xFF) * (256 - wx) + (c1         & 0xFF) * wx) >> 8;
                            drow[dx] = 0xFF000000u | (r << 16) | (g << 8) | b;
                        }
                    }
                    prev_pure_y0 = y0;
                    prev_pure_drow = drow;
                }
            } else {
                const uint32_t *row0 = g_argb_buf + y0 * sw;
                const uint32_t *row1 = g_argb_buf + y1 * sw;
                int iwy = 256 - wy;
                for (int dx = 0; dx < max_vw; ++dx) {
                    int wx = wx_tab[dx];
                    int x0 = x0_tab[dx];
                    int x1 = x1_tab[dx];
                    uint32_t c00 = row0[x0];
                    uint32_t c01 = row1[x0];
                    if (wx == 0) {
                        uint32_t r = (((c00 >> 16) & 0xFF) * iwy + ((c01 >> 16) & 0xFF) * wy) >> 8;
                        uint32_t g = (((c00 >> 8)  & 0xFF) * iwy + ((c01 >> 8)  & 0xFF) * wy) >> 8;
                        uint32_t b = ((c00         & 0xFF) * iwy + (c01         & 0xFF) * wy) >> 8;
                        drow[dx] = 0xFF000000u | (r << 16) | (g << 8) | b;
                    } else {
                        int iwx = 256 - wx;
                        uint32_t c10 = row0[x1];
                        uint32_t c11 = row1[x1];
                        uint32_t w00 = (uint32_t)(iwx * iwy);
                        uint32_t w10 = (uint32_t)(wx  * iwy);
                        uint32_t w01 = (uint32_t)(iwx * wy);
                        uint32_t w11 = (uint32_t)(wx  * wy);
                        uint32_t r = (((c00 >> 16) & 0xFF) * w00 + ((c10 >> 16) & 0xFF) * w10 + ((c01 >> 16) & 0xFF) * w01 + ((c11 >> 16) & 0xFF) * w11) >> 16;
                        uint32_t g = (((c00 >> 8)  & 0xFF) * w00 + ((c10 >> 8)  & 0xFF) * w10 + ((c01 >> 8)  & 0xFF) * w01 + ((c11 >> 8)  & 0xFF) * w11) >> 16;
                        uint32_t b = ((c00         & 0xFF) * w00 + (c10         & 0xFF) * w10 + (c01         & 0xFF) * w01 + (c11         & 0xFF) * w11) >> 16;
                        drow[dx] = 0xFF000000u | (r << 16) | (g << 8) | b;
                    }
                }
            }
        }
    }
}

static void dsa_platform_toggle_fullscreen(void) {
    g_is_fullscreen = !g_is_fullscreen;
    if (g_window) {
        SDL_SetWindowFullscreen(g_window, g_is_fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
    }
}

static void dsa_platform_cycle_scale(void) {
    if (g_is_fullscreen) {
        g_is_fullscreen = 0;
        SDL_SetWindowFullscreen(g_window, 0);
    }
    g_fenster_faktor = (g_fenster_faktor < 5) ? (g_fenster_faktor + 1) : 2;
    if (g_window) {
        SDL_SetWindowSize(g_window, 320 * g_fenster_faktor, 240 * g_fenster_faktor);
    }
}

int dsa_platform_init(const char *title, int scale) {
    (void)scale;
    if (g_window != NULL)
        return 0;

    if (!g_launch_cwd[0]) {
        _getcwd(g_launch_cwd, sizeof(g_launch_cwd));
    }

    try_load_spielpfad();

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_EVENTS) != 0) {
        fprintf(stderr, "[DSA-Platform] SDL_Init fehlgeschlagen: %s\n", SDL_GetError());
        return -1;
    }

    dsa_display_init_from_env();

    int win_w = 320 * g_fenster_faktor;
    int win_h = 240 * g_fenster_faktor;
    const char *hidden_env = getenv("DSA_HIDDEN");
    uint32_t win_flags = SDL_WINDOW_RESIZABLE | ((hidden_env && hidden_env[0] == '1') ? 0u : (uint32_t)SDL_WINDOW_SHOWN);
    if (g_is_fullscreen) {
        win_flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
    }
    g_window = SDL_CreateWindow(
        title ? title : "DSA 2: Sternenschweif (Windows Port)",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        win_w, win_h,
        win_flags
    );
    if (!g_window) {
        fprintf(stderr, "[DSA-Platform] SDL_CreateWindow fehlgeschlagen: %s\n", SDL_GetError());
        return -1;
    }

    g_renderer = SDL_CreateRenderer(g_window, -1, SDL_RENDERER_ACCELERATED);
    if (!g_renderer) {
        g_renderer = SDL_CreateRenderer(g_window, -1, SDL_RENDERER_SOFTWARE);
    }
    SDL_RenderSetLogicalSize(g_renderer, DSA_SCREEN_W, DSA_SCREEN_H);
    SDL_ShowCursor(SDL_DISABLE);

    g_texture = SDL_CreateTexture(
        g_renderer,
        SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING,
        DSA_SCREEN_W, DSA_SCREEN_H
    );

    memset(g_vram, 0, sizeof(g_vram));
    memset(g_pal6, 0, sizeof(g_pal6));
    for (int i = 0; i < 256; ++i) {
        g_argb_pal[i] = 0xFF000000u;
    }

    g_start_ms = SDL_GetTicks();
    return 0;
}

void dsa_platform_shutdown(void) {
    if (g_record_fp) {
        fprintf(g_record_fp, "F %u E\n", g_present_count);
        fclose(g_record_fp);
        g_record_fp = NULL;
    }
    if (g_display_tex) { SDL_DestroyTexture(g_display_tex); g_display_tex = NULL; }
    if (g_display_buf) { free(g_display_buf); g_display_buf = NULL; }
    if (g_texture)  { SDL_DestroyTexture(g_texture);   g_texture = NULL; }
    if (g_renderer) { SDL_DestroyRenderer(g_renderer); g_renderer = NULL; }
    if (g_window)   { SDL_DestroyWindow(g_window);     g_window = NULL; }
    SDL_Quit();
}

uint8_t *dsa_platform_get_vram(void) {
    return g_vram;
}

void dsa_platform_set_palette(int start, int count, const uint8_t *rgb6) {
    if (!rgb6 || count <= 0) return;
    for (int i = 0; i < count; ++i) {
        int idx = (start + i) & 0xFF;
        uint8_t r6 = rgb6[i * 3 + 0] & 0x3F;
        uint8_t g6 = rgb6[i * 3 + 1] & 0x3F;
        uint8_t b6 = rgb6[i * 3 + 2] & 0x3F;
        g_pal6[idx * 3 + 0] = r6;
        g_pal6[idx * 3 + 1] = g6;
        g_pal6[idx * 3 + 2] = b6;
        uint32_t r8 = (r6 << 2) | (r6 >> 4);
        uint32_t g8 = (g6 << 2) | (g6 >> 4);
        uint32_t b8 = (b6 << 2) | (b6 >> 4);
        g_argb_pal[idx] = 0xFF000000u | (r8 << 16) | (g8 << 8) | b8;
    }
}

void dsa_platform_get_palette(int start, int count, uint8_t *rgb6) {
    if (!rgb6 || count <= 0) return;
    for (int i = 0; i < count; ++i) {
        int idx = (start + i) & 0xFF;
        rgb6[i * 3 + 0] = g_pal6[idx * 3 + 0];
        rgb6[i * 3 + 1] = g_pal6[idx * 3 + 1];
        rgb6[i * 3 + 2] = g_pal6[idx * 3 + 2];
    }
}

void dsa_platform_present(void) {
    if (!g_renderer || !g_texture) return;

    for (int i = 0; i < DSA_VRAM_SIZE; ++i) {
        g_argb_buf[i] = g_argb_pal[g_vram[i]];
    }

    g_present_count++;

    const char *script = getenv("DSA_TEST_SCRIPT");
    if (script && script[0]) {
        char sbuf[2048];
        strncpy(sbuf, script, sizeof(sbuf) - 1);
        sbuf[sizeof(sbuf) - 1] = '\0';
        for (char *tok = strtok(sbuf, ";"); tok != NULL; tok = strtok(NULL, ";")) {
            uint32_t fnum = (uint32_t)strtoul(tok, NULL, 10);
            if (fnum != g_present_count) continue;
            const char *c1 = strchr(tok, ':');
            if (!c1) continue;
            c1++;
            if (strncmp(c1, "bmp:", 4) == 0) {
                write_bmp_320x200(c1 + 4, g_argb_buf);
                if (g_record_fp) {
                    fprintf(g_record_fp, "F %u B %s\n", g_present_count, c1 + 4);
                    fflush(g_record_fp);
                }
            } else if (strncmp(c1, "key:", 4) == 0) {
                uint16_t k = (uint16_t)strtoul(c1 + 4, NULL, 0);
                push_key(k);
            } else if (strncmp(c1, "move:", 5) == 0) {
                int mx = 0, my = 0;
                if (sscanf(c1 + 5, "%d,%d", &mx, &my) == 2) {
                    g_mouse_x = mx;
                    g_mouse_y = my;
                }
            } else if (strncmp(c1, "click:", 6) == 0) {
                int mx = 0, my = 0;
                if (sscanf(c1 + 6, "%d,%d", &mx, &my) == 2) {
                    g_mouse_x = mx;
                    g_mouse_y = my;
                    g_mouse_buttons |= 1;
                    g_left_clicks++;
                }
            } else if (strncmp(c1, "rclick:", 7) == 0) {
                int mx = 0, my = 0;
                if (sscanf(c1 + 7, "%d,%d", &mx, &my) == 2) {
                    g_mouse_x = mx;
                    g_mouse_y = my;
                    g_mouse_buttons |= 2;
                    g_right_clicks++;
                }
            } else if (strncmp(c1, "release", 7) == 0) {
                g_mouse_buttons = 0;
            } else if (strncmp(c1, "arrows:", 7) == 0) {
                g_arrow_mask = atoi(c1 + 7);
            }
        }
    }

    /* W4: Aufzeichnung (--record) des aktuellen Frame-Zustands */
    if (g_record_fp) {
        if (g_mouse_x != g_prev_rec_mx || g_mouse_y != g_prev_rec_my ||
            g_mouse_buttons != g_prev_rec_mb || g_left_clicks > 0 || g_right_clicks > 0 ||
            g_arrow_mask != g_prev_rec_arrows || g_key_mods != g_prev_rec_mods) {
            fprintf(g_record_fp, "F %u M %d %d %d %d %d %d %d\n",
                    g_present_count, g_mouse_x, g_mouse_y, g_mouse_buttons,
                    g_left_clicks, g_right_clicks, g_arrow_mask, g_key_mods);
            fflush(g_record_fp);
            g_prev_rec_mx = g_mouse_x;
            g_prev_rec_my = g_mouse_y;
            g_prev_rec_mb = g_mouse_buttons;
            g_prev_rec_arrows = g_arrow_mask;
            g_prev_rec_mods   = g_key_mods;
        }
    }

    /* W4: Wiedergabe (--replay) fuer diesen Frame */
    if (g_replay_active) {
        while (g_replay_pos < g_replay_count && g_replay_events[g_replay_pos].frame <= g_present_count) {
            const struct replay_event *re = &g_replay_events[g_replay_pos++];
            if (re->type == 'M') {
                g_mouse_x = re->mx;
                g_mouse_y = re->my;
                g_mouse_buttons = re->mb;
                g_left_clicks += re->lclicks;
                g_right_clicks += re->rclicks;
                g_arrow_mask = re->arrows;
                g_key_mods   = re->mods;
            } else if (re->type == 'K') {
                push_key(re->key);
            } else if (re->type == 'B') {
                write_bmp_320x200(re->bmp_path, g_argb_buf);
            } else if (re->type == 'E') {
                dsa_platform_shutdown();
                _exit(0);
            }
        }
    }

    const char *dump_bmp = getenv("DSA_DUMP_BMP");
    if (dump_bmp && dump_bmp[0]) {
        const char *dump_after = getenv("DSA_DUMP_AFTER");
        uint32_t target_frame = dump_after ? (uint32_t)atoi(dump_after) : 30;
        if (g_present_count == target_frame) {
            write_bmp_320x200(dump_bmp, g_argb_buf);
        }
    }

    const char *max_frames = getenv("DSA_MAX_FRAMES");
    if (max_frames && max_frames[0]) {
        uint32_t limit = (uint32_t)atoi(max_frames);
        if (limit > 0 && g_present_count >= limit) {
            dsa_platform_shutdown();
            _exit(0);
        }
    }

    int cw = 320 * g_fenster_faktor;
    int ch = 240 * g_fenster_faktor;
    if (g_window) {
        SDL_GetWindowSize(g_window, &cw, &ch);
    }
    dsa_render_scaled_display_buffer(cw, ch);
    if (g_display_tex && g_display_buf) {
        SDL_UpdateTexture(g_display_tex, NULL, g_display_buf, g_display_w * (int)sizeof(uint32_t));
        SDL_RenderClear(g_renderer);
        SDL_RenderCopy(g_renderer, g_display_tex, NULL, NULL);
        SDL_RenderPresent(g_renderer);
    } else {
        SDL_UpdateTexture(g_texture, NULL, g_argb_buf, DSA_SCREEN_W * (int)sizeof(uint32_t));
        SDL_RenderClear(g_renderer);
        SDL_RenderCopy(g_renderer, g_texture, NULL, NULL);
        SDL_RenderPresent(g_renderer);
    }
    g_last_present_ms = SDL_GetTicks();
    g_idle_polls = 0;
}

void dsa_platform_wait_vsync(void) {
    const char *fast = getenv("DSA_FAST_VSYNC");
    g_virtual_ms += (fast && fast[0] == '1') ? 2u : 14u;
    dsa_platform_poll_events();
    dsa_platform_present();
    if (!g_replay_active) {
        SDL_Delay((fast && fast[0] == '1') ? 2 : 14);
    }
}

void dsa_platform_set_tick_hook(void (*hook)(void)) {
    g_tick_hook = hook;
}

int dsa_platform_poll_events(void) {
    if (g_in_tick_hook) {
        return 1;
    }
    SDL_Event ev;
    const char *script_env = getenv("DSA_TEST_SCRIPT");
    int ignore_real_input = g_replay_active || (script_env && script_env[0] != '\0');
    while (SDL_PollEvent(&ev)) {
        if (ev.type == SDL_QUIT) {
            dsa_platform_shutdown();
            exit(0);
            return 0;
        }
        if (ignore_real_input) {
            continue;
        }
        switch (ev.type) {

            case SDL_MOUSEMOTION: {
                int x = ev.motion.x;
                int y = ev.motion.y;
                if (SDL_PortUsingExternalDLL() && g_vp_w > 0 && g_vp_h > 0) {
                    x = ((x - g_vp_x) * DSA_SCREEN_W) / g_vp_w;
                    y = ((y - g_vp_y) * DSA_SCREEN_H) / g_vp_h;
                }
                if (x < 0) x = 0;
                if (x >= DSA_SCREEN_W) x = DSA_SCREEN_W - 1;
                if (y < 0) y = 0;
                if (y >= DSA_SCREEN_H) y = DSA_SCREEN_H - 1;
                g_mouse_x = x;
                g_mouse_y = y;
                break;
            }

            case SDL_MOUSEBUTTONDOWN:
            case SDL_MOUSEBUTTONUP: {
                int x = ev.button.x;
                int y = ev.button.y;
                if (SDL_PortUsingExternalDLL() && g_vp_w > 0 && g_vp_h > 0) {
                    x = ((x - g_vp_x) * DSA_SCREEN_W) / g_vp_w;
                    y = ((y - g_vp_y) * DSA_SCREEN_H) / g_vp_h;
                }
                if (x < 0) x = 0;
                if (x >= DSA_SCREEN_W) x = DSA_SCREEN_W - 1;
                if (y < 0) y = 0;
                if (y >= DSA_SCREEN_H) y = DSA_SCREEN_H - 1;
                g_mouse_x = x;
                g_mouse_y = y;
                if (ev.type == SDL_MOUSEBUTTONDOWN) {
                    if (ev.button.button == SDL_BUTTON_LEFT) {
                        g_mouse_buttons |= 1;
                        g_left_clicks++;
                    } else if (ev.button.button == SDL_BUTTON_RIGHT) {
                        g_mouse_buttons |= 2;
                        g_right_clicks++;
                    }
                } else {
                    if (ev.button.button == SDL_BUTTON_LEFT)
                        g_mouse_buttons &= ~1;
                    else if (ev.button.button == SDL_BUTTON_RIGHT)
                        g_mouse_buttons &= ~2;
                }
                break;
            }

            case SDL_KEYDOWN: {
                SDL_Scancode sc = ev.key.keysym.scancode;
                int dos_sc = (sc < 0) ? ((-sc) & 0x7F) : 0;
                Uint16 m = ev.key.keysym.mod;
                int mods = 0;
                if (m & KMOD_RSHIFT) mods |= 0x01;
                if (m & KMOD_LSHIFT) mods |= 0x02;
                if (m & KMOD_CTRL)   mods |= 0x04;
                if (m & KMOD_ALT)    mods |= 0x08;
                g_key_mods = mods;

                if ((m & KMOD_ALT) && (dos_sc == 0x1C || sc == SDL_SCANCODE_RETURN || sc == SDL_SCANCODE_KP_ENTER || ev.key.keysym.sym == 13)) {
                    /* Alt+Enter: Fenster / Vollbild umschalten */
                    dsa_platform_toggle_fullscreen();
                    break;
                }
                if (!(m & (KMOD_CTRL | KMOD_ALT)) && (dos_sc == 0x0F || sc == SDL_SCANCODE_TAB)) {
                    /* Tab: Fenstergroesse durchschalten */
                    dsa_platform_cycle_scale();
                    break;
                }

                if (sc == SDL_SCANCODE_UP || sc == -0x48)    g_arrow_mask |= 0x01;
                if (sc == SDL_SCANCODE_DOWN || sc == -0x50)  g_arrow_mask |= 0x02;
                if (sc == SDL_SCANCODE_LEFT || sc == -0x4B)  g_arrow_mask |= 0x04;
                if (sc == SDL_SCANCODE_RIGHT || sc == -0x4D) g_arrow_mask |= 0x08;
                uint16_t code = sdl_to_bios_key(ev.key.keysym);
                /* F11/F12: Lautstaerke. F10 bleibt die Original-Pause. */
                if (code == 0x5700 || code == 0x5800) {
                    int step = (code == 0x5800) ? 10 : -10;
                    if (mods & 0x04)
                        dsa_platform_set_sfx_volume(dsa_platform_get_sfx_volume() + step);
                    else
                        dsa_platform_set_music_volume(dsa_platform_get_music_volume() + step);
                    char vmsg[80];
                    snprintf(vmsg, sizeof(vmsg), "laut musik=%d effekt=%d",
                             dsa_platform_get_music_volume(), dsa_platform_get_sfx_volume());
                    dsa_platform_ton_log(vmsg);
                } else if (code != 0) {
                    push_key(code);
                }
                break;
            }

            case SDL_KEYUP: {
                SDL_Scancode sc = ev.key.keysym.scancode;
                if (sc == SDL_SCANCODE_UP || sc == -0x48)    g_arrow_mask &= ~0x01;
                if (sc == SDL_SCANCODE_DOWN || sc == -0x50)  g_arrow_mask &= ~0x02;
                if (sc == SDL_SCANCODE_LEFT || sc == -0x4B)  g_arrow_mask &= ~0x04;
                if (sc == SDL_SCANCODE_RIGHT || sc == -0x4D) g_arrow_mask &= ~0x08;
                Uint16 m = ev.key.keysym.mod;
                int mods = 0;
                if (m & KMOD_RSHIFT) mods |= 0x01;
                if (m & KMOD_LSHIFT) mods |= 0x02;
                if (m & KMOD_CTRL)   mods |= 0x04;
                if (m & KMOD_ALT)    mods |= 0x08;
                g_key_mods = mods;
                break;
            }
        }
    }

    if (g_tick_hook && !g_in_tick_hook) {
        g_in_tick_hook = 1;
        g_tick_hook();
        g_in_tick_hook = 0;
    }

    uint32_t now = SDL_GetTicks();
    if (now - g_last_present_ms >= 16) {
        dsa_platform_present();
    }
    const char *max_sec = getenv("DSA_MAX_SECONDS");
    if (max_sec && max_sec[0]) {
        uint32_t limit_ms = (uint32_t)atoi(max_sec) * 1000u;
        if (limit_ms > 0 && now - g_start_ms >= limit_ms) {
            dsa_platform_shutdown();
            _exit(0);
        }
    }
    return 1;
}

void dsa_platform_get_mouse(int *x, int *y, int *buttons) {
    dsa_platform_poll_events();
    if (x) *x = g_mouse_x;
    if (y) *y = g_mouse_y;
    if (buttons) *buttons = g_mouse_buttons;
}

void dsa_platform_set_mouse_pos(int x, int y) {
    if (x < 0) x = 0;
    if (x >= DSA_SCREEN_W) x = DSA_SCREEN_W - 1;
    if (y < 0) y = 0;
    if (y >= DSA_SCREEN_H) y = DSA_SCREEN_H - 1;
    g_mouse_x = x;
    g_mouse_y = y;
}

void dsa_platform_consume_clicks(int *left_clicks, int *right_clicks) {
    if (left_clicks)  { *left_clicks  = g_left_clicks;  g_left_clicks  = 0; }
    if (right_clicks) { *right_clicks = g_right_clicks; g_right_clicks = 0; }
}

int dsa_platform_check_key(void) {
    dsa_platform_poll_events();
    if (g_key_head == g_key_tail)
        return 0;
    return (int)g_keybuf[g_key_tail];
}

int dsa_platform_get_key(void) {
    while (g_key_head == g_key_tail) {
        dsa_platform_poll_events();
        dsa_platform_present();
        if (g_replay_active) {
            g_virtual_ms += 10u;
        } else {
            SDL_Delay(10);
        }
    }
    uint16_t code = g_keybuf[g_key_tail];
    g_key_tail = (g_key_tail + 1) % KEYBUF_SIZE;
    return (int)code;
}

int dsa_platform_get_mods(void) {
    dsa_platform_poll_events();
    return g_key_mods;
}

int dsa_platform_get_arrow_mask(void) {
    return g_arrow_mask;
}

uint32_t dsa_platform_get_ticks_18hz(void) {
    uint32_t ms = (g_replay_active || g_record_fp != NULL) ? (g_present_count * 12u) : (SDL_GetTicks() - g_start_ms);
    const char *fast = getenv("DSA_FAST_VSYNC");
    if (fast && fast[0] == '1') ms = ms * 4u + g_present_count * 10u;
    return (uint32_t)((uint64_t)ms * 182u / 10000u);
}

uint32_t dsa_platform_get_ticks_ms(void) {
    uint32_t ms = (g_replay_active || g_record_fp != NULL) ? (g_present_count * 12u) : (SDL_GetTicks() - g_start_ms);
    const char *fast = getenv("DSA_FAST_VSYNC");
    if (fast && fast[0] == '1') ms = ms * 4u + g_present_count * 10u;
    return ms;
}

void dsa_platform_delay_ms(uint32_t ms) {
    if (g_replay_active) {
        for (uint32_t elapsed = 0; elapsed < ms; elapsed += 4u) {
            g_virtual_ms += 4u;
            dsa_platform_poll_events();
            dsa_platform_present();
        }
        return;
    }
    uint32_t start = SDL_GetTicks();
    while (SDL_GetTicks() - start < ms) {
        g_virtual_ms += 4u;
        dsa_platform_poll_events();
        dsa_platform_present();
        SDL_Delay(4);
    }
}

void dsa_platform_set_gamedir(const char *path) {
    if (!path || !path[0]) return;
    strncpy(g_gamedir, path, sizeof(g_gamedir) - 1);
    g_gamedir[sizeof(g_gamedir) - 1] = '\0';
}

const char *dsa_platform_get_gamedir(void) {
    return g_gamedir;
}

int dsa_platform_resolve_path(const char *dos_path, char *out_path, size_t out_size) {
    if (!dos_path || !out_path || out_size == 0) return -1;

    const char *rel = dos_path;
    if (isalpha((unsigned char)rel[0]) && rel[1] == ':') {
        rel += 2;
        if (*rel == '\\' || *rel == '/') ++rel;
    } else if (rel[0] == '\\' || rel[0] == '/') {
        ++rel;
    }

    char clean[512];
    size_t j = 0;
    for (size_t i = 0; rel[i] && j + 1 < sizeof(clean); ++i) {
        char c = rel[i];
#ifdef _WIN32
        if (c == '/') c = '\\';
#else
        if (c == '\\') c = '/';
#endif
        clean[j++] = c;
    }
    clean[j] = '\0';

#ifdef _WIN32
    snprintf(out_path, out_size, "%s\\%s", g_gamedir, clean);
#else
    snprintf(out_path, out_size, "%s/%s", g_gamedir, clean);
#endif
    return 0;
}

void dsa_platform_set_record_file(const char *path) {
    if (!path || !path[0]) return;
    if (g_record_fp) fclose(g_record_fp);
    g_record_fp = fopen(path, "wb");
    if (g_record_fp) {
        fprintf(g_record_fp, "# DSA2_REPLAY_V1\n");
        fflush(g_record_fp);
    }
}

void dsa_platform_set_replay_file(const char *path) {
    if (!path || !path[0]) return;
    FILE *fp = fopen(path, "rb");
    if (!fp) {
        fprintf(stderr, "[W4 Replay] Datei nicht gefunden: %s\n", path);
        return;
    }
    g_replay_count = 0;
    g_replay_pos   = 0;
    g_replay_active = 1;
    char line[512];
    while (fgets(line, sizeof(line), fp)) {
        char *p = line;
        while (*p == ' ' || *p == '\t') p++;
        if (*p == '#' || *p == '\r' || *p == '\n' || *p == '\0') continue;
        if (strncmp(p, "SEED ", 5) == 0) {
            g_replay_seed = (uint32_t)strtoul(p + 5, NULL, 0);
            g_replay_has_seed = 1;
            continue;
        }
        if (g_replay_count >= g_replay_cap) {
            g_replay_cap = g_replay_cap ? g_replay_cap * 2 : 256;
            g_replay_events = (struct replay_event *)realloc(g_replay_events, g_replay_cap * sizeof(struct replay_event));
        }
        struct replay_event *ev = &g_replay_events[g_replay_count];
        memset(ev, 0, sizeof(*ev));
        uint32_t fnum = 0;
        char type = 0;
        if (sscanf(p, "F %u %c", &fnum, &type) == 2) {
            ev->frame = fnum;
            ev->type  = type;
            const char *rest = strchr(p + 2, type);
            if (rest) rest++;
            while (rest && (*rest == ' ' || *rest == '\t')) rest++;
            if (type == 'M' && rest) {
                sscanf(rest, "%d %d %d %d %d %d %d",
                       &ev->mx, &ev->my, &ev->mb, &ev->lclicks, &ev->rclicks, &ev->arrows, &ev->mods);
                g_replay_count++;
            } else if (type == 'K' && rest) {
                ev->key = (uint16_t)strtoul(rest, NULL, 0);
                g_replay_count++;
            } else if (type == 'B' && rest) {
                strncpy(ev->bmp_path, rest, sizeof(ev->bmp_path) - 1);
                size_t l = strlen(ev->bmp_path);
                while (l > 0 && (ev->bmp_path[l - 1] == '\r' || ev->bmp_path[l - 1] == '\n' || ev->bmp_path[l - 1] == ' '))
                    ev->bmp_path[--l] = '\0';
                g_replay_count++;
            } else if (type == 'E') {
                g_replay_count++;
            }
        }
    }
    fclose(fp);
}

uint32_t dsa_platform_init_seed(uint32_t default_seed) {
    uint32_t s = g_replay_has_seed ? g_replay_seed : default_seed;
    const char *env_seed = getenv("DSA_SEED");
    if (env_seed && env_seed[0]) {
        s = (uint32_t)strtoul(env_seed, NULL, 0);
    }
    if (g_record_fp) {
        fprintf(g_record_fp, "SEED 0x%08X\n", s);
        fflush(g_record_fp);
    }
    return s;
}

uint32_t dsa_platform_get_frame_count(void) {
    return g_present_count;
}

void dsa_platform_save_bmp(const char *path) {
    if (!path || !path[0]) return;
    for (int i = 0; i < DSA_VRAM_SIZE; ++i) {
        g_argb_buf[i] = g_argb_pal[g_vram[i]];
    }
    write_bmp_320x200(path, g_argb_buf);
}
