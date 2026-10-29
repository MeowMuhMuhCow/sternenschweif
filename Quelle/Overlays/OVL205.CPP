/* OVL205 - Overlay O205 (Segment 0x2383 in SCHWEIF.EXE): Fensterfunktionen der Klasse Win (Basis des Dialogobjekts
 * Dlg, g_dialog ds:b59a; Klassen in s_class.h, Konstruktor/Destruktor in O190). save_bg/restore_bg: Hintergrund
 * zeilenweise per XMS sichern/zuruecklegen (g_xms_move, xms_move_any, Block xms); im Kampfbild (g_fight_screen_on)
 * statt dessen Kampf neu zeichnen (fight_draw). draw_frame(n): Rahmen fuer n Zeilen aus Teilbildern (draw_frame_row:
 * Kopf/Mitte/Fuss aus frame_gfx ueber NvfMem und Bmp 0x20 x h, 4 Zeilen je Mittelteil). calc_x/calc_y: Lage des
 * Fensters (Versatz pos_x/pos_y, sonst Maus bzw. Mitte bei g_win_centered, auf 320 x 200 begrenzt). save_border/
 * restore_border: Streifen unter dem Zugrahmen sichern/zurueck (R34 vga_*). drag(x, y, w, h): Fenster mit Maus bzw.
 * Pfeiltasten verschieben (Puffer 0x410, Ergebnis new_x/new_y). poll_input: Maus/Tastatur abfragen (im Kampf
 * zusaetzlich handle_options_key). wait(in): Warteschleife mit Titelleisten-Mausbereich g_dlg_areas, Verschieben
 * (Eingabefeld in mitziehen), Zeitlimit lines * (Meldungsdauer * 2 + 2) * 12 Ticks. set_no_timeout. close: Fenster
 * schliessen (Schrift/Farben und Merker zurueck). prepare(s, x, y): Fenster vorbereiten (Merker sichern, Breite
 * (tiles << 5) + 64, Zeilen font_wrap, Text kopieren). text_box(n, s): Meldung bzw. Texteingabe mit n Zeichen
 * (Input O180, atol). num_box(n, s, min, max, val): Zahleneingabe (NumIn O197).
 *
 * BCC -mlarge -3 -Ot -Og -Oi -c -Y OVL205.CPP -> 5149/5149 Bytes GLEICH (18.10.2026), cobjfix laeuft
 *   (-Oi fuer strcpy; ohne -Og 856, ohne -Oi 1150 Abweichungen; -Oe/-rd zerstoeren den Code).
 *   (Namensfassung mit schweif.h, Methoden der Klasse Win: 19.10.2026 GLEICH, LINKGLEICH)
 * - Die Funktionen sind Elementfunktionen (this = erstes Argument, gleiche Bytes wie die freien win_*-Funktionen).
 *   Win::poll_input ruft die globale poll_input als ::poll_input.
 * - calc_x/calc_y (ohne Aufrufe): OHNE jedes register-Schluesselwort ergibt BCC genau die Originalregister
 *   (calc_x: b CX, x DX, over SI; calc_y: h SI, y CX, over DI); jedes "register" verschiebt sie.
 * - g_mouse_locked ist unsigned char, das Original testet es signed: (signed char)-Cast (cbw statt mov ah,0).
 * - close: font_set_colors mit short-Farben (push word) ueber gecasteten Funktionszeiger.
 * - abs() ist in C++ inline mit Parameterkopie (wie atoi) -> in wait __abs__(d) direkt (mov ax,si / cwd).
 * - "lim = lines * (g_text_speed * 2 + 2) * 12" (push ax / mov ax,[..] / pop dx / imul dx); umgekehrt imul word
 *   es:[..]. Ebenso "n * font_char_width(..)" (push/pop) gegen "font_char_width(..) * n" (imul word [bp+0ah]).
 * - g_vga_buf als void far *, Adressrechnung "(signed long)g_vga_buf + (signed long)(y + i) * 320 + x" (mov edx).
 * - close: inline set_b76d(v) -> Parameterkopie [bp-2]. text_box/num_box: r vor dem lokalen Objekt deklariert
 *   (Initialisierung [bp-8] vor dem Konstruktor), -Og-Zwischenwert [bp-0ch] fuer tiles, return r ueber
 *   Zwischenwert [bp-0ah] vor dem Destruktor; "if (s != 0 && *s != 0) {..; return r;} return -1;".
 * - draw_frame_row: NvfMem-Objekt (0x1a Bytes, gross) zuerst deklariert -> liegt am Ende ([bp-36h]), Bmp (10 Bytes)
 *   bei den Skalaren; Teilbilder per Makro PIECE; Sprungverteilung "case 0 / case 2 / case 1" (Ruempfe in
 *   Quellreihenfolge).
 * Originalfehler/Auffaelliges: drag - "if (mx != -1)" direkt nach der Initialisierung mit -1 ist immer falsch;
 *   im Tastaturzweig wird "mx = sx" (alte Fenster-x) statt der Mausposition gemerkt - ein anschliessender
 *   Mauszug rechnet dann g_mouse_posx - mx aus Maus- und Fensterkoordinate (my bleibt unveraendert).
 */
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include "schweif.h"

#define TILE_W		0x20		/* Rahmenteil 32 Pixel breit */
#define FRAME		15		/* Rahmenbreite */
#define COL_DRAG	0xff		/* Farbe des Zugrahmens */
#define COL_TEXT	0xff
#define DRAG_STEP	8		/* Verschieben mit den Pfeiltasten */
#define DRAG_BUF	0x410		/* zwei Zeilen (2 * 320) + zwei Spalten (2 * 200) + Rest */
#define ROW_TOP		0		/* draw_frame_row: Kopf */
#define ROW_MID		1		/* Mitte (je Teil 4 Zeilen) */
#define ROW_BOTTOM	2		/* Fuss */
#define ACT_TITLE	1		/* g_action: Klick in die Titelleiste (g_dlg_areas[0]) */
#define ACT_CLOSE	2		/* Fenster schliessen */
#define CH_ESC		0x1b
#define CH_UNDERSCORE	0x5f		/* Breite eines Eingabezeichens */

/* Rahmenteil p (Teilbild der Rahmendatei frame_gfx) bei x/y zeichnen, Hoehe hoechstens h */
#define PIECE(p) \
	nvf.unpack(buf, frame_gfx, p, 0, &pw, &ph, SCREEN_W); \
	if (ph > h) \
		ph = h; \
	bmp.put_part(g_vga_buf, x, y, TILE_W, ph, 4);

void Win::save_bg()
{
	register signed short i;

	if (g_fight_screen_on == 0) {
		for (i = 0; h > i; i++) {
			g_xms_move.len = w;
			g_xms_move.src_handle = 0;
			g_xms_move.src = (signed long)g_vga_buf + (signed long)(y + i) * SCREEN_W + x;
			g_xms_move.dst_handle = xms;
			g_xms_move.dst = (signed long)i * w;
			xms_move_any(&g_xms_move);
		}
	}
}

void Win::restore_bg()
{
	signed short top;
	signed short dummy;
	signed short ix;
	signed short iy;
	register signed short i;

	if (g_fight_screen_on == 1) {
		/* im Kampfbild: Kampf neu zeichnen statt Hintergrund aus dem XMS */
		font_get_box(g_font, &dummy, &dummy, &dummy, &top);
		font_get_indent(g_font, &ix, &iy);
		font_set_box(g_font, -1, -1, -1, 0);
		font_set_indent(g_font, 0, 0);
		g_fig_redraw_all = 1;
		is_open = 0;
		fight_draw((FIGHT_PTR)g_fight_obj);
		is_open = 1;
		font_set_box(g_font, -1, -1, -1, top);
		font_set_indent(g_font, ix, iy);
	} else {
		for (i = 0; h > i; i++) {
			g_xms_move.len = w;
			g_xms_move.dst_handle = 0;
			g_xms_move.dst = (signed long)g_vga_buf + (signed long)(y + i) * SCREEN_W + x;
			g_xms_move.src_handle = xms;
			g_xms_move.src = (signed long)i * w;
			xms_move_any(&g_xms_move);
		}
	}
	g_bg_restored = 1;
}

void Win::draw_frame(signed short n)
{
	signed short rem;
	register signed short i;
	register signed short lim;

	save_bg();
	draw_frame_row(0, ROW_TOP, TILE_W);
	i = 0;
	rem = n % 4;
	lim = n - rem;
	if (lim > 0) {
		for (i = 0; i < lim; i += 4)
			draw_frame_row(i + 2, ROW_MID, TILE_W);
	}
	if (rem)
		draw_frame_row(i + 2, ROW_MID, rem << 3);
	draw_frame_row(n + 2, ROW_BOTTOM, TILE_W);
	clear_kbd();
}

void Win::draw_frame_row(signed short row, signed short type, signed short h)
{
	signed short p_left;
	signed short p_mid1;
	signed short p_mid2;
	signed short p_right;
	signed short pw;
	signed short ph;
	signed short y;
	void far *buf;
	register signed short x;
	register signed short k;
	NvfMem nvf;
	Bmp bmp(TILE_W, h, 0);

	buf = mem_alloc(0x400);
	bmp.set_base(buf);
	switch (type) {
	case ROW_TOP:
		p_left = 0;
		p_mid1 = 1;
		p_mid2 = 2;
		p_right = 3;
		break;
	case ROW_BOTTOM:
		p_left = 4;
		p_mid1 = 5;
		p_mid2 = 6;
		p_right = 7;
		break;
	case ROW_MID:
		p_left = 8;
		p_mid1 = 10;
		p_mid2 = 11;
		p_right = 9;
		break;
	}
	x = this->x;
	y = this->y + (row << 3);
	PIECE(p_left);
	x += TILE_W;
	for (k = tiles; k > 0; k--) {
		PIECE(p_mid1);
		x += TILE_W;
		k--;
		if (k > 0) {
			PIECE(p_mid2);
			x += TILE_W;
		}
	}
	PIECE(p_right);
	mem_free(buf);
}

signed short Win::calc_x(signed short bw)
{
	signed short x;
	signed short over;

	if (pos_x)
		x = ((SCREEN_W - bw) >> 1) + pos_x;
	else if (!(signed char)g_mouse_locked && !g_win_centered)
		x = g_mouse_posx - bw + 16;
	else
		x = (SCREEN_W - bw) >> 1;
	over = x + bw - (SCREEN_W - 1);
	if (over > 0)
		x -= over;
	if (x < 0)
		x = 0;
	return x;
}

signed short Win::calc_y(signed short bh)
{
	signed short y;
	signed short over;

	if (pos_y)
		y = ((SCREEN_H - bh) >> 1) + pos_y;
	else if (!(signed char)g_mouse_locked && !g_win_centered)
		y = g_mouse_posy - (bh >> 1);
	else
		y = (SCREEN_H - bh) >> 1;
	over = y + bh - (SCREEN_H - 1);
	if (over > 0)
		y -= over;
	if (y < 0)
		y = 0;
	return y;
}

void Win::save_border(register signed short x, register signed short y, signed short bw, signed short bh, char far *buf)
{
	vga_save_row(y * SCREEN_W, buf);
	vga_save_row((y + bh) * SCREEN_W - SCREEN_W, buf + SCREEN_W);
	vga_save_col(x, buf + 2 * SCREEN_W);
	vga_save_col(x + bw - 1, buf + 2 * SCREEN_W + SCREEN_H);
}

void Win::restore_border(register signed short x, register signed short y, signed short bw, signed short bh,
	char far *buf)
{
	vga_restore_row_part(y * SCREEN_W + x, buf + x, bw);
	vga_restore_row_part((y + bh) * SCREEN_W + x - SCREEN_W, buf + x + SCREEN_W, bw);
	g_bg_restored = 1;
	vga_restore_col_part(y * SCREEN_W + x, buf + y + 2 * SCREEN_W, bh);
	vga_restore_col_part(y * SCREEN_W + x + bw - 1, buf + y + 2 * SCREEN_W + SCREEN_H, bh);
	g_bg_restored = 1;
}

void Win::drag(register signed short x, register signed short y, signed short bw, signed short bh)
{
	char far *buf;
	signed short mx = -1;
	signed short my = -1;
	signed short px = -1;
	signed short py = -1;
	signed short sx = -1;
	signed short sy = -1;

	if (!no_move) {
		if ((buf = (char far *)mem_alloc(DRAG_BUF)) != 0) {
			dragging = 1;
			mouse_hide(0);
			if (mx != -1)
				restore_border(px, py, bw, bh, buf);
			save_border(x, y, bw, bh, buf);
			do_border(g_vga_buf, x, y, bw, bh, COL_DRAG);
			mouse_show(0);
			px = x;
			py = y;
			mx = g_mouse_posx;
			my = g_mouse_posy;
			while (g_mouse_ldown || g_key_shift) {
				poll_input();
				if (g_key_shift) {
					/* Pfeiltasten */
					sx = x;
					sy = y;
					if (g_action == KEY_RIGHT)
						x += DRAG_STEP;
					else if (g_action == KEY_LEFT)
						x -= DRAG_STEP;
					else if (g_action == KEY_DOWN)
						y += DRAG_STEP;
					else if (g_action == KEY_UP)
						y -= DRAG_STEP;
					else if (g_action == KEY_ESC || g_action == KEY_PGDN)
						break;
					if (x < 0)
						x = 0;
					if (y < 0)
						y = 0;
					if (x + bw > SCREEN_W - 1)
						x = SCREEN_W - 1 - bw;
					if (y + bh > SCREEN_H - 1)
						y = SCREEN_H - 1 - bh;
					if (sx != x || sy != y) {
						mouse_hide(0);
						if (mx != -1)
							restore_border(px, py, bw, bh, buf);
						save_border(x, y, bw, bh, buf);
						do_border(g_vga_buf, x, y, bw, bh, COL_DRAG);
						mouse_show(0);
						px = x;
						py = y;
#if FIX_0136
						/* Fix #136: Mausposition merken statt alter Fenster-x (kein Sprung beim naechsten Mauszug) */
						mx = g_mouse_posx;
						my = g_mouse_posy;
#else
						mx = sx;
#endif
					}
				} else {
					/* Maus */
					if (mx != g_mouse_posx || my != g_mouse_posy) {
						mouse_hide(0);
						if (mx != -1)
							restore_border(px, py, bw, bh, buf);
						save_border(x, y, bw, bh, buf);
						do_border(g_vga_buf, x, y, bw, bh, COL_DRAG);
						mouse_show(0);
						px = x;
						py = y;
						if (mx != -1) {
							x += g_mouse_posx - mx;
							y += g_mouse_posy - my;
							if (x < 0)
								x = 0;
							if (y < 0)
								y = 0;
							if (x + bw > SCREEN_W - 1)
								x = SCREEN_W - 1 - bw;
							if (y + bh > SCREEN_H - 1)
								y = SCREEN_H - 1 - bh;
						}
						mx = g_mouse_posx;
						my = g_mouse_posy;
					}
				}
			}
			dragging = 0;
			mouse_hide(0);
			restore_border(new_x = px, new_y = py, bw, bh, buf);
			mouse_show(0);
			buf = (char far *)mem_free(buf);
		}
	}
	mouse_clear_left();
	g_mouse_event = 0;
}

void Win::poll_input()
{
	::poll_input();
	if (g_in_fight)
		handle_options_key();
}

signed short Win::wait(Input far *in)
{
	signed long lim;
	signed long t0;
	signed short iy;
	register signed short d;
	register signed short ix;

	t0 = g_random_seed2;
	if (g_in_fight && all_heroes_down(g_group))
		lim = lines * (g_fight_speed * 2 + 2) * 12;
	else
		lim = lines * (g_text_speed * 2 + 2) * 12;
	do {
		g_dlg_areas[0].x0 = x;
		g_dlg_areas[0].x1 = x + w;
		g_dlg_areas[0].y0 = y + 1;
		g_dlg_areas[0].y1 = y + 14;
		g_mouse_areas2 = g_dlg_areas;
		poll_input();
		g_mouse_areas2 = 0;
		if (g_key == KEY_PGUP || g_key == KEY_PGDN || g_mouse_rclick) {
			g_action = ACT_CLOSE;
			g_key_ascii = CH_ESC;
			g_mouse_rclick = 0;
		}
		if (!g_win_centered && (g_action == ACT_TITLE || g_key_shift) && (g_mouse_ldown || g_key_shift)) {
			drag(x, y, w, h);
			if (new_x != x || new_y != y) {
				mouse_hide(0);
				restore_bg();
				if (in) {
					ix = in->get_x() - (x - new_x);
					in->set_x(ix);
					iy = in->get_y() - (y - new_y);
					in->set_y(iy);
				}
				x = new_x;
				y = new_y;
				font_set_colors(g_font, COL_TEXT, -1);
				font_set_box(g_font, x + FRAME, -1, y + FRAME, 1);
				draw_frame(lines);
				font_print_wrapped(g_font, text);
				if (in)
					in->redraw();
				g_action = 0;
				mouse_show(0);
			}
		}
		if (no_timeout == 0 && in == 0) {
			d = t0 - g_random_seed2;
			if (__abs__(d) > lim)
				break;
		}
	} while (g_action != ACT_CLOSE && g_key_ascii == 0);
	no_timeout = 0;
	return g_key_ascii;
}

void Win::set_no_timeout()
{
	no_timeout = 1;
}

inline void set_b76d(signed short v) { g_status_on = v; }

void Win::close()
{
	mouse_hide(0);
	mouse_reset();
	restore_bg();
	mouse_show(0);
	/* Farben als short uebergeben (push word), Prototyp hat unsigned char */
	((void (*)(void far *, signed short, signed short))font_set_colors)(g_font, old_fg, old_bg);
	font_set_box(g_font, old_box[0], old_box[1], old_box[2], old_box[3]);
	sel = -1;
	is_open = 0;
	g_clock_update = old_clock;
	set_b76d(old_b76d);
	g_mouse_handler_on = old_mouse_cb;
}

void Win::prepare(char far *s, signed short ix, signed short iy)
{
	old_b76d = g_status_on;
	g_status_on = 0;
	old_mouse_cb = g_mouse_handler_on;
	g_mouse_handler_on = 0;
	mouse_set_cursor(0, 1);
	text = s;
	old_clock = g_clock_update;
	g_clock_update = 0;
	is_open = 1;
	font_get_colors(g_font, &old_fg, &old_bg);
	font_get_box(g_font, &old_box[0], &old_box[1], &old_box[2], &old_box[3]);
	w = (tiles << 5) + 0x40;
	x = calc_x(w);
	font_set_box(g_font, x + FRAME, w - 2 * FRAME, -1, -1);
	font_set_indent(g_font, ix, iy);
	lines = font_wrap(g_font, s);
	strcpy(text, text_buf);
}

signed short Win::text_box(signed short n, char far *s)
{
	signed short bw;
	signed short need;
	signed short old_tiles;
	signed short r = 0;
	Input in(n);

	if (s != 0 && *s != 0) {
		if ((*s = toupper(*s)) == '<')
			s[1] = toupper(s[1]);
		strcpy(text_buf, s);
		bw = font_char_width(g_font, CH_UNDERSCORE) * n + 0x20;
		need = bw / 32;
		old_tiles = tiles;
		if (tiles < need)
			tiles = need;
		prepare(s, 0, 0);
		if (n)
			lines += 2;
		h = (lines << 3) + 0x1c;
		y = calc_y(h);
		font_set_colors(g_font, COL_TEXT, -1);
		font_set_box(g_font, -1, -1, y + FRAME, 1);
		mouse_hide(0);
		draw_frame(lines);
		font_print_wrapped(g_font, s);
		mouse_show(0);
		if (n) {
			/* Texteingabe: Ergebnis als Zahl */
			r = in.run(g_b690, x + ((w - n * font_char_width(g_font, CH_UNDERSCORE)) >> 1),
				(lines << 3) + y + 8, 0);
			if (r != -1)
				r = atol(g_b690);
		} else {
			if (!g_in_fight)
				wait_ticks(7);
			clear_input();
			mouse_clear_left();
			mouse_clear_right();
			g_mouse_ldclick = g_mouse_event = 0;
			wait(0);
		}
		close();
		g_mouse_ldclick = g_mouse_event = 0;
		clear_input();
		mouse_clear_left();
		mouse_clear_right();
		tiles = old_tiles;
		return r;
	}
	return -1;
}

signed short Win::num_box(signed short n, char far *s, signed short min, signed short max, signed short val)
{
	signed short bw;
	signed short need;
	signed short old_tiles;
	signed short r = 0;
	NumIn in(n, frame_gfx, min, max, val);

	if (s != 0 && *s != 0) {
		if ((*s = toupper(*s)) == '<')
			s[1] = toupper(s[1]);
		strcpy(text_buf, s);
		bw = n * 15 + 0x20;
		need = bw / 32;
		old_tiles = tiles;
		if (tiles < need)
			tiles = need;
		prepare(s, 0, 0);
		lines += 5;
		h = (lines << 3) + 0x1c;
		y = calc_y(h);
		font_set_colors(g_font, COL_TEXT, -1);
		font_set_box(g_font, -1, -1, y + FRAME, 1);
		mouse_hide(0);
		draw_frame(lines);
		font_print_wrapped(g_font, s);
		mouse_show(0);
		r = in.run(x + ((w - n * 14 - 50) >> 1), (lines << 3) + y - 8);
		close();
		g_mouse_ldclick = g_mouse_event = 0;
		clear_input();
		mouse_clear_left();
		mouse_clear_right();
		tiles = old_tiles;
		return r;
	}
	return -1;
}
