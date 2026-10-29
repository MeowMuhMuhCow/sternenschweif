/* OVL190 - Overlay O190 (Segment 0x2319 in SCHWEIF.EXE): Dialogfenster-Klasse Dlg (globales Objekt g_dialog ds:b59a;
 * Basis Win 0x46 Bytes mit Konstruktor/Destruktor hier, abgeleitete Klasse Dlg 0x9b Bytes mit Bmp-Member anim_bmp;
 * beide in s_class.h). Dlg(): Basis, Bmp gb_new0, f46/dragging/yes_no = 0, pic_frame = 1, anim/bmp = 0.
 * yes_no_box(s): Ja/Nein-Abfrage (Texte g_ttx[0x1be/0x1bf]) -> 1/-1/0. message(s): Meldung (Win::text_box, Maus
 * aus/an im Kampf). input(ndig, s, min, max, val): Zahleneingabe (erstes Zeichen bzw. nach '<' gross,
 * Win::num_box). choose(s, n, ...): Auswahl unter n Texten (run) bzw. Meldung ohne Auswahl. box/box_anim/box_bmp:
 * dasselbe mit Bild (Rohbild, Animation O051 oder Bmp; Groesse nach pic_w/pic_h). lock/unlock: Sperre no_move/
 * no_drag setzen/loesen. draw_pic: Bild im Fenster zeichnen (Animation, Bmp mit Rahmen bei pic_frame == 1, Rohbild
 * pic ueber lokale Bmp). print_title/print_body: Titel/Text ausgeben (R20, Schrift g_font). print_choices:
 * Zeilenpositionen der Auswahltexte (va_list choices) nach line_offs (new char[n + 1]), vorgewaehlte Zeile sel in
 * Farbe 0xf9. input_text(n, s): Meldung bzw. Texteingabe mit n Zeichen.
 * Win(): Rahmenteile aus Datei 3 (g_file_main), 0x400-Byte-Textpuffer, XMS xms_alloc_kb(0x40) -> g_dialog_xms.
 * ~Win(). lock_move..set_width: Getter/Setter (no_move, pos_x, pos_y, tiles; alter Wert zurueck).
 * num_loop(n): Tastatur-/Mausschleife fuer die Zahleneingabe O197 (Mausbereiche g_dlg_areas, Verschieben des
 * Fensters per Win::drag). overlaps(x0, y0, x1, y1): Fenster ueberlappt Rechteck? fill_area(dst): Inneres und
 * Ecken mit Farbe 0xff (do_h_line). dummy: leer. set_open: is_open = f46 = 1.
 *
 * BCC -mlarge -3 -Ot -Og -c -Y OVL190.CPP   -> 4539/4539 Bytes GLEICH (18.10.2026), cobjfix laeuft
 *   (Namensfassung mit schweif.h, Klassen Win/Dlg/NumIn in s_class.h: 19.10.2026 GLEICH, LINKGLEICH)
 * - C++: Klassen mit Basis (Konstruktor this == 0 -> mem_alloc_w(0x9b) bzw. (0x46), Basisaufruf, Member-Bmp
 *   gb_new0; Basis-Destruktor mit (flags & 1) -> mem_free_v), inline NumIn::set_cur (Kopien [bp-0eh]/[bp-10h]).
 * - Variadische Elementfunktionen: va_start -> lea/mov [bp-2],ss/mov [bp-4]; das Original sichert ap vor dem
 *   Aufruf in eine zweite va_list und stellt es danach wieder her (mov eax-Paare) - "save = ap; ...; ap = save;".
 *   va_arg(ap, char far *) ergibt mov es,[bp-8] / add word [bp-0ah],4 / es:[bx-2]/[bx-4].
 * - Methoden der Basis (Win, Code in O205) und der Auswahl (run, O189) als Elementaufrufe - gleiche Bytes wie
 *   die freien Aufrufe mit this.
 * - num_loop: k OHNE register (DI), -Og-Zwischenwert k * 10 in SI; mit register vertauscht (37 Bytes anders).
 *   "(w - nd * 15 - 42) >> 1" (sar ohne cwd), d.h. kein / 2.
 * - box_anim: Zeiger anim vor dem Aufruf in eine eigene Variable (mov eax / [bp-0ch]).
 * - "f46 = dragging = yes_no = 0", "is_open = pos_x = pos_y = 0", "pic_w = pic_h = 0x20" (rechts nach links).
 * - Nested ?: in yes_no_box ("r == 1 ? 1 : (r == -1 ? -1 : 0)") mit totem Epilog.
 * Originalfehler: choose verwirft bei n == 0 das Ergebnis von input_text und liefert immer 0. num_loop: bei
 *   nd == 0 wird bx2 fuer den OK-Bereich uninitialisiert benutzt.
 */
#include <ctype.h>
#include <stdarg.h>
#include "schweif.h"

#define TTX_YES		0x1be		/* g_ttx: "Ja" */
#define TTX_NO		0x1bf		/* "Nein" */
#define FILE_FRAME	3		/* Rahmenteile des Fensters */
#define FRAME		15		/* Rahmenbreite */
#define NUM_OK		12		/* Mausbereiche der Zahleneingabe: 1 Titel, 2..6 auf, 7..11 ab, 12 OK, 13 sonst */
#define NUM_OUTSIDE	13

Dlg::Dlg()
{
	f46 = dragging = yes_no = 0;
	pic_frame = 1;
	anim = 0;
	bmp = 0;
}

signed short Dlg::yes_no_box(char far *s)
{
	register signed short r;

	yes_no = 1;
	r = choose(s, 2, g_ttx[TTX_YES], g_ttx[TTX_NO]);
	yes_no = 0;
	return r == 1 ? 1 : (r == -1 ? -1 : 0);
}

signed short Dlg::message(char far *s)
{
	register signed short r;

	if (g_in_fight)
		mouse_show(0);
	if (s != 0 && *s != 0)
		r = text_box(0, s);
	else
		r = 0;
	if (g_in_fight)
		mouse_hide(0);
	return r;
}

signed short Dlg::input(signed short ndig, char far *s, signed short min, signed short max, signed short val)
{
	signed short r;

	if (g_in_fight)
		mouse_show(0);
	if ((*s = toupper(*s)) == '<')
		s[1] = toupper(s[1]);
	r = num_box(ndig, s, min, max, val);
	if (g_in_fight)
		mouse_hide(0);
	return r;
}

signed short Dlg::choose(char far *s, signed short n, ...)
{
	register signed short r = 0;
	va_list ap;
	va_list save;

	va_start(ap, n);
	if (n == 0) {
		input_text(0, s);
	} else {
		save = ap;
		r = run(0, 0, s, &n, ap);
		ap = save;
	}
	return r;
}

signed short Dlg::box(void far *p, char far *t, char far *s, signed short n, ...)
{
	register signed short r = 0;
	va_list ap;
	va_list save;

	bmp = 0;
	va_start(ap, n);
	if (p != 0)
		pic_w = pic_h = 0x20;
	if (s != 0 && *s != 0) {
		save = ap;
		r = run(p, t, s, &n, ap);
		ap = save;
	}
	return r;
}

signed short Dlg::box_anim(void far *a, char far *t, char far *s, signed short n, ...)
{
	register signed short r = 0;
	va_list ap;
	va_list save;
	void far *p;

	va_start(ap, n);
	if (s != 0 && *s != 0) {
		save = ap;
		anim = a;
		Anim_size(anim, &pic_w, &pic_h);
		p = anim;
		r = run(p, t, s, &n, ap);
		anim = 0;
		ap = save;
	}
	return r;
}

signed short Dlg::box_bmp(Bmp far *b, char far *t, char far *s, signed short n, ...)
{
	register signed short r = 0;
	va_list ap;
	va_list save;

	va_start(ap, n);
	if (s != 0 && *s != 0) {
		save = ap;
		bmp = b;
		bmp->size(&pic_w, &pic_h);
		r = run(b->data(), t, s, &n, ap);
		bmp = 0;
		ap = save;
	}
	return r;
}

signed short Dlg::lock()
{
	signed short old;

	lock_move();
	old = no_drag;
	no_drag = 1;
	return old;
}

void Dlg::unlock()
{
	unlock_move();
	no_drag = 0;
}

void Dlg::draw_pic()
{
	if (anim != 0) {
		Anim_start(anim, x + FRAME, y + FRAME, 0);
		return;
	}
	if (bmp != 0) {
		if (pic_frame == 1)
			do_border(g_vga_buf, x + FRAME, y + FRAME, pic_w + 2, pic_h + 2, 0xe0);
		bmp->put(g_vga_buf, x + FRAME + 1, y + FRAME + 1, 4);
		return;
	}
	if (pic != 0) {
		Bmp b(pic_w, pic_h, pic);

		do_border(g_vga_buf, x + FRAME, y + FRAME, pic_w + 2, pic_h + 2, 0xe0);
		b.put(g_vga_buf, x + FRAME + 1, y + FRAME + 1);
	}
}

void Dlg::print_title()
{
	signed short box_x;
	signed short box_w;
	signed short box_y;
	signed short box_h;
	signed short y2;
	register signed short n;

	if (title != 0) {
		font_set_colors(g_font, 0xfd, -1);
		font_get_box(g_font, &box_x, &box_w, &box_y, &box_h);
		font_set_indent(g_font, text_x, text_y);
		font_set_box(g_font, -1, w - FRAME, y + FRAME, -1);
		n = font_print_wrapped(g_font, title) + 1;
		font_get_box(g_font, &box_h, &box_h, &y2, &box_h);
		font_set_box(g_font, box_x, box_w, y2 + n * 8 - 2, box_h);
		font_set_colors(g_font, 0xff, -1);
		text_y -= n;
	}
}

void Dlg::print_body()
{
	font_set_indent(g_font, text_x, text_y);
	if (lines != 0)
		font_print_wrapped(g_font, body);
	font_set_indent(g_font, 0, 0);
}

void Dlg::print_choices()
{
	signed short y0;
	signed short py;
	signed short px;
	va_list ap;
	register signed short i;
	register signed short h;

	line_offs = 0;
	if (n_choices != 0) {
		line_offs = (unsigned char far *)new char[n_choices + 1];
		px = x + 28;
		y0 = py = lines * 8 + y + FRAME;
		ap = choices;
		for (i = 0; i < n_choices; i++) {
			if (g_254a == 0 && sel == i)
				font_set_colors(g_font, 0xf9, -1);
			else
				font_set_colors(g_font, 0xe0, -1);
			h = font_print(g_font, va_arg(ap, char far *), px, py);
			line_offs[i] = py - y0;
			py += h * 8;
		}
		line_offs[i] = 199 - y0;
	}
}

signed short Dlg::input_text(signed short n, char far *s)
{
	signed short r;

	if (g_in_fight)
		mouse_show(0);
	*s = toupper(*s);
	r = text_box(n, s);
	if (g_in_fight)
		mouse_hide(0);
	return r;
}

Win::Win()
{
	signed long n;

	n = file_length(g_file_main, FILE_FRAME);
	frame_gfx = mem_alloc(n);
	file_load(g_file_main, FILE_FRAME, frame_gfx);
	text_buf = (char far *)mem_alloc(0x400L);
	xms = xms_alloc_kb(0x40);
	g_dialog_xms = xms;
	is_open = pos_x = pos_y = 0;
	sel = -1;
}

Win::~Win()
{
	xms_free_block(xms);
	mem_free(frame_gfx);
	mem_free(text_buf);
}

signed short Dlg::lock_move()
{
	signed short old;

	old = no_move;
	no_move = 1;
	return old;
}

void Dlg::unlock_move()
{
	no_move = 0;
}

signed short Dlg::set_pos_x(signed short v)
{
	signed short old;

	old = pos_x;
	pos_x = v;
	return old;
}

signed short Dlg::set_pos_y(signed short v)
{
	signed short old;

	old = pos_y;
	pos_y = v;
	return old;
}

signed short Dlg::set_width(signed short v)
{
	signed short old;

	old = tiles;
	tiles = v;
	return old;
}

signed short Dlg::num_loop(NumIn far *n)
{
	signed short i;
	signed short bx;
	signed short bx2;
	signed short by;
	signed short nd;
	signed short redo;
	signed short k = 0;

	redo = 1;
	nd = n->ndig;
	do {
		if (redo) {			/* Mausbereiche: Titel, Pfeile je Ziffer, OK, Rest */
			redo = 0;
			k = 0;
			g_dlg_areas[k].x0 = x;
			g_dlg_areas[k].x1 = x + w;
			g_dlg_areas[k].y0 = y + 1;
			g_dlg_areas[k].y1 = y + 14;
			k++;
#if FIX_0099
			/* Fix #99: bx2 vorbelegen (bei nd == 0 sonst uninitialisiert) */
			by = lines * 8 + y - 12;
			bx2 = x + ((w - nd * 15 - 42) >> 1) - 8;
			for (i = 0; i < nd; i++) {
#else
			by = lines * 8 + y - 12;
			for (i = 0; i < nd; i++) {
#endif
				bx = x + ((w - nd * 15 - 42) >> 1) - 8;
				bx += i * 14;
				bx2 = bx + 14;
				g_dlg_areas[k].x0 = bx;
				g_dlg_areas[k].x1 = bx2;
				g_dlg_areas[k].y0 = by - 13;
				g_dlg_areas[k].y1 = g_dlg_areas[k].y0 + 13;
				g_dlg_areas[k].action = i + 2;
				k++;
				g_dlg_areas[k].x0 = bx;
				g_dlg_areas[k].x1 = bx2;
				g_dlg_areas[k].y0 = by + 17;
				g_dlg_areas[k].y1 = g_dlg_areas[k].y0 + 13;
				g_dlg_areas[k].action = i + 7;
				k++;
			}
			g_dlg_areas[k].x0 = bx2 + 16;
			g_dlg_areas[k].x1 = bx2 + 50;
			g_dlg_areas[k].y0 = by;
			g_dlg_areas[k].y1 = by + 16;
			g_dlg_areas[k].action = NUM_OK;
			k++;
			g_dlg_areas[k].x0 = 0;
			g_dlg_areas[k].x1 = SCREEN_W - 1;
			g_dlg_areas[k].y0 = 0;
			g_dlg_areas[k].y1 = SCREEN_H - 1;
			g_dlg_areas[k].action = NUM_OUTSIDE;
		}
		g_mouse_areas2 = g_dlg_areas;
		poll_input();
		g_mouse_areas2 = 0;
		if (g_key == KEY_PGUP || g_key == KEY_PGDN || g_mouse_rclick != 0) {
			g_action = NUM_OK;
			g_key_ascii = 0x1b;
			g_mouse_rclick = 0;
		}
		if (!g_win_centered && (g_action == 1 || g_key_shift != 0) && (g_mouse_ldown != 0 || g_key_shift != 0)) {
			drag(x, y, w, h);
			if (new_x != x || new_y != y) {
				signed short dx;
				signed short dy;

				mouse_hide(0);
				restore_bg();
				dx = n->get_x() - (x - new_x);
				n->set_x(dx);
				dy = n->get_y() - (y - new_y);
				n->set_y(dy);
				x = new_x;
				y = new_y;
				font_set_box(g_font, x + FRAME, -1, y + FRAME, 1);
				draw_frame(lines);
				font_print_wrapped(g_font, text);
				n->draw_frame();
				n->draw_digits();
				g_action = 0;
				mouse_show(0);
				redo = 1;
			}
		}
		if (g_action >= 2 && g_action <= 6) {		/* Pfeil auf ueber Ziffer */
			n->set_cur(g_action - 2);
			g_action = KEY_UP;
		}
		if (g_action >= 7 && g_action <= 11) {		/* Pfeil ab */
			n->set_cur(g_action - 7);
			g_action = KEY_DOWN;
		}
		if (g_action == KEY_RIGHT || g_action == KEY_LEFT || g_action == KEY_UP || g_action == KEY_DOWN)
			g_key_ascii = 1;
	} while (g_action != NUM_OK && g_key_ascii == 0);
	g_dlg_areas[1].x0 = 0;
	g_dlg_areas[1].x1 = SCREEN_W - 1;
	g_dlg_areas[1].y0 = 0;
	g_dlg_areas[1].y1 = SCREEN_H - 1;
	g_dlg_areas[1].action = 2;
	return g_key_ascii;
}

signed short Dlg::overlaps(register signed short x0, signed short y0, signed short x1, signed short y1)
{
	register signed short r = 0;

	if (is_open == 0)
		return r;
	if (((x <= x0 && x + w >= x0) || (x <= x1 && x + w >= x1)) &&
		((y <= y0 && y + h >= y0) || (y <= y1 && y + h >= y1)))
		r = 1;
	return r;
}

void Dlg::fill_area(void far *dst)
{
	signed short len;
	signed short x2;
	signed short y2;
	register signed short py;
	register signed short px;

	x2 = x + w;
	y2 = y + h;
	px = x + 6;
	len = w - 12;
	for (py = y + 7; py <= y + h - 3; py++)		/* Inneres */
		do_h_line(dst, px, py, len, 0xff);
	px = x + 3;					/* Ecke links oben */
	len = 10;
	for (py = y + 4; py <= y + 6; py++)
		do_h_line(dst, px, py, len, 0xff);
	px = x + 3;
	len = 3;
	for (py = y + 7; py <= y + 13; py++)
		do_h_line(dst, px, py, len, 0xff);
	px = x + 3;					/* links unten */
	len = 10;
	for (py = y2 - 3; py <= y2; py++)
		do_h_line(dst, px, py, len, 0xff);
	px = x + 3;
	len = 3;
	for (py = y2 - 9; py <= y2 - 4; py++)
		do_h_line(dst, px, py, len, 0xff);
	px = x2 - 13;					/* rechts oben */
	len = 10;
	for (py = y + 4; py <= y + 6; py++)
		do_h_line(dst, px, py, len, 0xff);
	px = x2 - 6;
	len = 3;
	for (py = y + 7; py <= y + 13; py++)
		do_h_line(dst, px, py, len, 0xff);
	px = x2 - 13;					/* rechts unten */
	len = 10;
	for (py = y2 - 3; py <= y2; py++)
		do_h_line(dst, px, py, len, 0xff);
	px = x2 - 6;
	len = 3;
	for (py = y2 - 9; py <= y2 - 4; py++)
		do_h_line(dst, px, py, len, 0xff);
}

void Dlg::dummy()
{
}

void Dlg::set_open()
{
	is_open = 1;
	f46 = 1;
}
