/* OVL181 - Overlay O181 (Segment 0x22e6 in SCHWEIF.EXE): Textbildschirm struct text_screen (0x18 Bytes, s_struct.h;
 * Basis der Bildfolgen O039/O040/O041 und O166, dort als Klasse "Win" mit open/close/text). tscr_open_anim(p, ani,
 * id, cnt) / tscr_open_texts(p, id, cnt) / tscr_open(p): Zustand der Schrift g_font (Kasten, Farben) nach p sichern,
 * Farben 0xff/0, Textliste id/cnt laden (txt_load_list nach txt_buf/txt), Vollbild ungueltig (g_pp20_index = -1),
 * Bildschirm loeschen (clear_screen_buf); tscr_open_anim startet zusaetzlich die Hintergrund-Animation ani
 * (tscr_set_anim). tscr_close(p, keep): Textliste freigeben (ausser keep), Schrift zurueck, g_refresh = 1.
 * tscr_print/tscr_print_txt/tscr_print_font/tscr_print_txt_font(p, s bzw. txt[i], [Schrift,] h, x, w): einen
 * Streifen der Hoehe h am unteren Bildrand ab x - 2 (Breite w + 4) ueber ein lokales Bmp loeschen und den Text im
 * Kasten (x, w, 200 - h) ausgeben, Schrift g_font bzw. Parameter. tscr_set_anim(p, ani, mode): Animation ani in das
 * Bildobjekt g_screen_pic laden (ani_load), Lage (0x3c, 10), abspielen (anim_reset).
 *
 * BCC -mlarge -3 -Ot -c -Y OVL181.C   -> 1276/1276 Bytes GLEICH (18.10.2026), cobjfix laeuft
 *   (Namensfassung mit schweif.h: 19.10.2026 GLEICH, LINKGLEICH)
 * - Reines C, beim ersten Versuch gleich; tscr_set_anim steht unten -> far call aus tscr_open_anim.
 * - Lokales Bmp-Objekt (10 Bytes, [bp-0ah], C-Struktur struct Bmp aus s_class.h) mit expliziten Konstruktor-/
 *   Destruktoraufrufen gb_new_alloc/gb_delete(.., 2) wie in OVL193.
 * - tscr_close: font_set_colors mit short-Farben (push word) ueber gecasteten Funktionszeiger.
 * - h/x landen automatisch in SI/DI (Parameter, mehrfach benutzt); x - 2 ergibt add ax,0fffeh.
 */
#include "schweif.h"

#define COL_FG		0xff		/* Schriftfarben waehrend der Bildfolge */
#define COL_BG		0
#define ANI_X		0x3c		/* Lage der Hintergrund-Animation */
#define ANI_Y		0x0a

void tscr_set_anim(struct text_screen far *p, signed short ani, signed short mode);

void tscr_open_anim(struct text_screen far *p, signed short ani, signed short id, signed short cnt)
{
	font_get_colors(g_font, &p->old_fg, &p->old_bg);
	font_get_box(g_font, &p->old_box[0], &p->old_box[1], &p->old_box[2], &p->old_box[3]);
	font_set_colors(g_font, COL_FG, COL_BG);
	txt_load_list(g_font, id, cnt, &p->txt_buf, &p->txt);
	g_pp20_index = -1;
	clear_screen_buf(g_vga_buf);
	poll_input();
	tscr_set_anim(p, ani, 0);
}

void tscr_open_texts(struct text_screen far *p, signed short id, signed short cnt)
{
	font_get_colors(g_font, &p->old_fg, &p->old_bg);
	font_get_box(g_font, &p->old_box[0], &p->old_box[1], &p->old_box[2], &p->old_box[3]);
	font_set_colors(g_font, COL_FG, COL_BG);
	txt_load_list(g_font, id, cnt, &p->txt_buf, &p->txt);
	g_pp20_index = -1;
	clear_screen_buf(g_vga_buf);
}

void tscr_open(struct text_screen far *p)
{
	font_get_colors(g_font, &p->old_fg, &p->old_bg);
	font_get_box(g_font, &p->old_box[0], &p->old_box[1], &p->old_box[2], &p->old_box[3]);
	font_set_colors(g_font, COL_FG, COL_BG);
	g_pp20_index = -1;
	clear_screen_buf(g_vga_buf);
}

void tscr_close(struct text_screen far *p, signed short keep)
{
	if (keep == 0) {
		mem_free(p->txt_buf);
		mem_free(p->txt);
	}
	font_set_box(g_font, p->old_box[0], p->old_box[1], p->old_box[2], p->old_box[3]);
	((void (*)(void far *, signed short, signed short))font_set_colors)(g_font, p->old_fg, p->old_bg);
	g_refresh = 1;
}

void tscr_print(struct text_screen far *p, char far *s, signed short h, signed short x, signed short w)
{
	struct Bmp b;

	gb_new_alloc(&b, w + 4, h);
	gb_clear(&b);
	font_set_box(g_font, x, w, SCREEN_H - h, 1);
	gb_blit(&b, g_vga_buf, x - 2, SCREEN_H - h);
	poll_input();
	font_print_wrapped(g_font, s);
	gb_delete(&b, 2);
}

void tscr_print_txt(struct text_screen far *p, signed short i, signed short h, signed short x, signed short w)
{
	struct Bmp b;

	gb_new_alloc(&b, w + 4, h);
	gb_clear(&b);
	font_set_box(g_font, x, w, SCREEN_H - h, 1);
	gb_blit(&b, g_vga_buf, x - 2, SCREEN_H - h);
	poll_input();
	font_print_wrapped(g_font, p->txt[i]);
	gb_delete(&b, 2);
}

void tscr_print_font(struct text_screen far *p, char far *s, void far *font, signed short h, signed short x,
	signed short w)
{
	struct Bmp b;

	gb_new_alloc(&b, w + 4, h);
	gb_clear(&b);
	font_set_box(font, x, w, SCREEN_H - h, 1);
	gb_blit(&b, g_vga_buf, x - 2, SCREEN_H - h);
	poll_input();
	font_print_wrapped(font, s);
	gb_delete(&b, 2);
}

void tscr_print_txt_font(struct text_screen far *p, signed short i, void far *font, signed short h, signed short x,
	signed short w)
{
	struct Bmp b;

	gb_new_alloc(&b, w + 4, h);
	gb_clear(&b);
	font_set_box(font, x, w, SCREEN_H - h, 1);
	gb_blit(&b, g_vga_buf, x - 2, SCREEN_H - h);
	poll_input();
	font_print_wrapped(font, p->txt[i]);
	gb_delete(&b, 2);
}

void tscr_set_anim(struct text_screen far *p, signed short ani, signed short mode)
{
	ani_load(g_screen_pic, ani);
	ani_set_pos(g_screen_pic, ANI_X, ANI_Y);
	anim_reset(g_screen_pic, mode);
	poll_input();
}
