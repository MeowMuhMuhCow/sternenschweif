/* OVL189 - Overlay O189 (Segment 0x2313 in SCHWEIF.EXE): Auswahlfenster des Dialogobjekts - Methoden der Klasse Dlg
 * (s_class.h; Objekt g_dialog ds:b59a, Rest der Klasse in O190, Basis Win in O190/O205; Schrift g_font, R20).
 * count_lines(n, ap): Zeilenzahl der n Auswahltexte (font_wrap, Fensterbreite - 10). draw_choices(n, ap): Rahmen
 * zeichnen (Win::save_bg/draw_frame_row: Kopf, je 4 Zeilen ein Teil, Rest, Fuss) und vor jedem Auswahltext ein
 * Knopfbild (draw_button Bild 0x0d). draw_button(y, nr): Knopfbild nr (6x7, aus frame_gfx ueber new NvfMem /
 * new Bmp(6, 7, buf)) bei x + 15 zeichnen. mark_line(y): Markierung von Zeile mark_y auf y umsetzen (alte Zeile
 * Farbe 0xe0 bzw. 0xf9 fuer die Vorgabe sel, neue 0xff, Bild 0x0c). choose_loop(n, ap): Auswahlschleife
 * (Pfeiltasten mit Umlauf, Maus ueber line_offs, Fenster verschieben Win::drag mit Neuaufbau, Enter/Klick ->
 * Nummer, Esc/Bild ab/g_mouse_rclick -> -1, bei yes_no Tasten j/n = Ja/Nein); n == 0 -> wait_message, -1.
 * click_sound: Klickgeraeusch snd_play(g_snd_fx, 0x49). run(pic, title, s, pn, ap): Auswahl/Meldung mit Bild
 * (Animation anim oder Bmp bmp bzw. Rohbild: Textversatz text_x/text_y), Titelbreite, Aufbau (Win::prepare,
 * layout, print_title/print_body/draw_pic/print_choices aus O190), choose_loop, Aufraeumen. layout: Breite/Lage
 * aus tiles, bei mehr als 21 Zeilen tiles += 2 (ab 9: guru_meditation "Text ist viel, viel zu lange!!!!").
 * wait_message: Meldung ohne Auswahl - Warteschleife mit Zeitlimit (lines * (g_fight_speed bzw. g_text_speed * 2 + 2) *
 * 12 Ticks g_random_seed2), Verschieben, Animation (Anim_select_seq). calc_text: Textversatz neu berechnen, Zeilen
 * zaehlen, body <- text_buf.
 *
 * BCC -mlarge -3 -Ot -Og -Oi -c -Y OVL189.CPP   -> 5639/5639 Bytes GLEICH (18.10.2026), cobjfix laeuft
 *   (ohne -Og 4128 Abweichungen: Zwischenwerte total_lines [bp-10h], sel [bp-14h], pic_h [bp-0ah]/[bp-2]).
 *   (Namensfassung mit schweif.h, Methoden der Klasse Dlg: 19.10.2026 GLEICH, LINKGLEICH)
 * - Elementfunktionen der Klasse Dlg (gleiche Bytes wie die freien dlg_*-Funktionen mit this als erstem Argument);
 *   lokale Objekte NvfMem (R41) und Bmp ueber new/delete (gb_new(0, ..)/nvf_new(0), Dtor(p, 3)), Bmp-Member
 *   anim_bmp; va_list-Parameter als void far * (= va_list in STDARG.H).
 * - Reihenfolge: click_sound/layout/wait_message/calc_text stehen hinter ihren Aufrufern (far call), draw_button/
 *   mark_line/draw_choices davor.
 * - draw_choices: i/j mit register (SI/DI), sonst bekommt ln DI. "i = ln = 0" (ln zuerst gespeichert).
 * - run: strcpy-Intrinsic auf Funktionsebene -> register-Variable r nur im inneren Block (SI).
 * - calc_text: strcpy(body, text_buf) (Ziel +6a zuerst geladen).
 * - "choices = save = ap", "text_x = text_y = 0", "g_mouse_rclick = g_key = g_key_ascii = g_action = 0" (rechts
 *   zuerst). g_mouse_locked (unsigned char) als (signed char) getestet (cbw).
 * - wait_message: __abs__(dt) statt abs() (C++ inline-Kopie), "(signed long)__abs__(dt) > lim" -> movsx eax.
 * - g_font_tab0 (ds:381f) ist g_font.tabs[0], aber ein eigenes Public (eigenes Symbol noetig).
 * Originalfehler: keine gefunden.
 */
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include "schweif.h"

/* Phase B: eigene Daten des Moduls (ds:6e4c..6e78, dsana) - initialisierte Daten in Adressreihenfolge, danach der Literalpool */
signed char g_loc_icons_loaded[10] = "\xfe\xfe\xfe\xfe\xfe\xfe\xfe\xfe\xfe";

#define FRAME		15		/* Rahmenbreite */
#define TILE_W		0x20		/* Rahmenteil */
#define ROW_TOP		0		/* Win::draw_frame_row: Kopf, Mitte, Fuss */
#define ROW_MID		1
#define ROW_BOTTOM	2
#define PIC_BUTTON	0x0d		/* Knopfbild vor jeder Auswahlzeile (6 x 7) */
#define PIC_BUTTON_ON	0x0c		/* Knopfbild der markierten Zeile */
#define COL_CHOICE	0xe0		/* Auswahltext */
#define COL_DEFAULT	0xf9		/* vorgewaehlte Zeile (sel) */
#define COL_MARK	0xff		/* markierte Zeile */
#define SND_CLICK	0x49		/* Klickgeraeusch */
#define MAX_LINES	0x15		/* mehr Zeilen: Fenster breiter (rows += 2) */
#define MAX_TILES	8		/* breiter geht es nicht: Fehlermeldung */
#define ACT_TITLE	1		/* g_action: Klick in die Titelleiste */
#define ACT_CLOSE	2
#define CH_ESC		0x1b
#define KEY_J		0x2c79		/* 'y' bzw. 'j' auf deutscher Tastatur: Ja */
#define KEY_N		0x316e		/* 'n': Nein */
#define ANIM		((struct anim_view far *)anim)

signed short Dlg::count_lines(signed short n, void far *ap)
{
	signed short bw;
	signed short dummy;
	signed short ix;
	signed short iy;
	signed short sum = 0;
	signed short i;

	font_get_box(g_font, &dummy, &bw, &dummy, &dummy);
	font_get_indent(g_font, &ix, &iy);
	font_set_box(g_font, -1, bw - 10, -1, -1);
	font_set_indent(g_font, 0, 0);
	for (i = 0; i < n; i++)
		sum += font_wrap(g_font, va_arg(ap, char far *));
	font_set_box(g_font, -1, bw, -1, -1);
	font_set_indent(g_font, ix, iy);
	return sum;
}

void Dlg::draw_choices(signed short n, void far *ap)
{
	signed short ln;
	signed short full;
	signed short rem;
	signed short bw;
	signed short dummy;
	signed short ix;
	signed short iy;
	register signed short i;
	register signed short j;

	save_bg();
	draw_frame_row(0, ROW_TOP, TILE_W);
	i = ln = 0;
	rem = total_lines % 4;
	full = total_lines - rem;
	if (full > 0)
		for (i = 0; i < full; i += 4)
			draw_frame_row(i + 2, ROW_MID, TILE_W);
	if (rem)
		draw_frame_row(i + 2, ROW_MID, rem << 3);
	draw_frame_row(total_lines + 2, ROW_BOTTOM, TILE_W);
	if (n) {
		font_get_box(g_font, &dummy, &bw, &dummy, &dummy);
		font_get_indent(g_font, &ix, &iy);
		font_set_box(g_font, -1, bw - 10, -1, -1);
		font_set_indent(g_font, 0, 0);
		for (j = 0; j < n; j++) {
			draw_button(((lines + ln) << 3) + y + 0x10, PIC_BUTTON);
			ln += font_wrap(g_font, va_arg(ap, char far *));
		}
		font_set_box(g_font, -1, bw, -1, -1);
		font_set_indent(g_font, ix, iy);
	}
	clear_kbd();
}

void Dlg::draw_button(signed short by, signed short nr)
{
	signed short pw;
	signed short ph;
	void far *buf;
	Bmp far *b;
	NvfMem far *nvf;

	buf = mem_alloc(0x2aL);
	b = new Bmp(6, 7, buf);
	nvf = new NvfMem();
	nvf->unpack(buf, frame_gfx, nr, 0, &pw, &ph, SCREEN_W);
	b->put_part(g_vga_buf, x + FRAME, by, pw, ph, 4);
	delete nvf;
	delete b;
	mem_free(buf);
}

void Dlg::mark_line(register signed short my)
{
	if (mark_y != my) {
		mouse_hide(0);
		if (mark_y != -1) {
			/* alte Zeile in ihrer Farbe zurueck */
			if (g_254a == 0 && old_sel - 1 == sel)
				font_set_colors(g_font, COL_DEFAULT, -1);
			else
				font_set_colors(g_font, COL_CHOICE, -1);
			draw_button(mark_y, PIC_BUTTON);
			font_print(g_font, mark_text, x + 0x1c, mark_y - 1);
		}
		font_set_colors(g_font, COL_MARK, -1);
		draw_button(my, PIC_BUTTON_ON);
		font_print(g_font, new_text, x + 0x1c, my - 1);
		mouse_show(0);
		mark_y = my;
		mark_text = new_text;
	}
}

signed short Dlg::choose_loop(signed short n, void far *ap)
{
	signed short tx;
	signed short ty;
	signed short my;
	signed short by;
	signed short ret = 0;
	signed short done = 0;
	signed short moved = 0;
	va_list save;
	signed short i;
	signed short k;

	if (n) {
		if (g_in_fight == 0)
			wait_ticks(7);
		clear_input();
		mouse_clear_left();
		mouse_clear_right();
		g_mouse_ldclick = g_mouse_event = 0;
		old_sel = -1;
		cur_sel = 1;
		save = ap;
		mark_y = -1;
		by = (lines << 3) + y + 0x10;
		my = g_mouse_posy;
		for (i = 0; cur_sel > i; i++)
			new_text = va_arg(ap, char far *);
		mark_line(by + line_offs[cur_sel - 1]);
		do {
			ap = save;
			g_dlg_areas[0].x0 = x;
			g_dlg_areas[0].x1 = x + w;
			g_dlg_areas[0].y0 = y + 1;
			g_dlg_areas[0].y1 = y + 0x0e;
			if (!g_win_centered && (g_action == ACT_TITLE || g_key_shift != 0)) {
				/* Fenster verschieben und neu aufbauen */
				if (no_drag == 0) {
					if (g_mouse_ldown != 0 || g_key_shift != 0) {
						click_sound();
						drag(x, y, w, h);
						if (new_x != x || new_y != y) {
							mouse_hide(0);
							restore_bg();
							mark_y = -1;
							x = new_x;
							y = new_y;
							calc_text();
							font_set_tab(g_font, x + w - 0x24, 0);
							font_set_colors(g_font, COL_MARK, -1);
							font_set_box(g_font, x + FRAME, w - 2 * FRAME, y + FRAME, 0);
							font_set_indent(g_font, text_x, text_y);
							if (title != 0)
								lines += font_wrap(g_font, title) + 1;
							if (lines < text_y)
								lines = text_y;
							draw_choices(n, ap);
							print_title();
							print_body();
							draw_pic();
							tx = x + 0x1c;
							ty = (lines << 3) + y + 0xf;
							font_set_indent(g_font, 0, 0);
							for (i = 0; i < n; i++) {
								if (g_254a == 0 && sel == i)
									font_set_colors(g_font, COL_DEFAULT, -1);
								else
									font_set_colors(g_font, COL_CHOICE, -1);
								k = font_print(g_font, va_arg(ap, char far *), tx, ty);
								ty += k << 3;
							}
							mouse_show(0);
							by = (lines << 3) + y + 0x10;
							my = -1;
							mark_line(by + line_offs[cur_sel - 1]);
						}
					}
				} else
					mouse_clear_left();
			} else {
				if (g_key == KEY_UP) {
					old_sel = cur_sel;
					if (--cur_sel == 0)
						cur_sel = n;
					for (i = 0; cur_sel > i; i++)
						new_text = va_arg(ap, char far *);
					mark_line(by + line_offs[cur_sel - 1]);
				} else if (g_key == KEY_DOWN) {
					old_sel = cur_sel;
					if (cur_sel++ == n)
						cur_sel = 1;
					for (i = 0; cur_sel > i; i++)
						new_text = va_arg(ap, char far *);
					mark_line(by + line_offs[cur_sel - 1]);
				} else if (!(signed char)g_mouse_locked && g_mouse_posy != my) {
					/* Maus: Zeile unter dem Zeiger markieren */
					my = g_mouse_posy;
					for (i = 1; i <= n; i++) {
						if (line_offs[i] + by > my) {
							if (cur_sel != i) {
								moved = 1;
								old_sel = cur_sel;
								cur_sel = i;
								for (k = 0; cur_sel > k; k++)
									new_text = va_arg(ap, char far *);
								mark_line(by + line_offs[i - 1]);
							}
							break;
						}
					}
				}
				if (g_key == KEY_RETURN || g_mouse_lclick != 0) {
					click_sound();
					if (g_mouse_lclick != 0 && moved != 0) {
						my = g_mouse_clicky;
						for (i = 1; i <= n; i++) {
							if (line_offs[i] + by > my) {
								cur_sel = i;
								break;
							}
						}
					}
					clear_input();
					mouse_clear_left();
					mouse_clear_right();
					ret = cur_sel;
					g_mouse_ldclick = g_mouse_event = 0;
					done = 1;
				}
				if (yes_no != 0) {
					if (g_key == KEY_J) {
						click_sound();
						cur_sel = 1;
						done = 1;
					} else if (g_key == KEY_N) {
						click_sound();
						cur_sel = 2;
						done = 1;
					}
				}
			}
			if (g_mouse_rclick != 0 || g_key == KEY_ESC || g_key == KEY_PGDN) {
				ret = -1;
				done = 1;
				g_mouse_rclick = g_key = g_key_ascii = g_action = 0;
			}
			g_mouse_areas2 = g_dlg_areas;
			poll_input();
			g_mouse_areas2 = 0;
		} while (done == 0);
		font_set_tab(g_font, old_tab, 0);
	} else {
		wait_message();
		ret = -1;
	}
	pic_frame = 1;
	return ret;
}

void Dlg::click_sound()
{
	snd_play(g_snd_fx, SND_CLICK);
}

signed short Dlg::run(void far *p, char far *t, char far *s, signed short far *pn, void far *ap)
{
	va_list save;
	signed short ret;
	signed short prev_tiles;

	if (g_in_fight)
		mouse_show(0);
	ret = 0;
	f7c = 0x11;
	f7e = 0x0b;
	pic = p;
	title = t;
	body = s;
	choices = save = ap;
	n_choices = *pn;
	if ((*s = toupper(*s)) == '<')
		s[1] = toupper(s[1]);
	strcpy(text_buf, s);
	text_x = text_y = 0;
	if (bmp != 0 || anim != 0) {
		text_x = pic_w + 6;
		text_y = pic_h / 8 + (pic_h % 8 ? 2 : 1);
	} else if (p != 0) {
		text_x = 0x26;
		text_y = 5;
	}
	prev_tiles = tiles;
	if (t != 0) {
		register signed short r;

		r = (font_text_width(g_font, t) + text_x) / 32;
		if (r > MAX_TILES)
			r = MAX_TILES;
		if (r > prev_tiles)
			set_width(r);
	}
	old_tiles = tiles;
	prepare(s, text_x, text_y);
	layout();
	h = (total_lines << 3) + 0x1c;
	y = calc_y(h);
	font_set_colors(g_font, COL_MARK, -1);
	font_set_box(g_font, x + FRAME, -1, y + FRAME, 0);
	mouse_hide(0);
	ap = save;
	draw_choices(*pn, ap);
	if (anim != 0) {
		/* Animation: Hintergrund sichern und als Bmp uebergeben */
		Anim_size(anim, &pic_w, &pic_h);
		anim_buf = mem_alloc((signed long)pic_w * (signed long)pic_h);
		anim_bmp.init(pic_w, pic_h, anim_buf);
		mouse_hide(0);
		anim_bmp.get(g_vga_buf, x + FRAME, y + 0x12);
		mouse_show(0);
		Anim_setbg(anim, &anim_bmp);
	}
	print_title();
	print_body();
	draw_pic();
	print_choices();
	mouse_show(0);
	font_set_colors(g_font, COL_MARK, -1);
	ap = save;
	ret = choose_loop(*pn, ap);
	if (anim != 0) {
		Anim_stop(anim);
		anim_buf = mem_free(anim_buf);
		anim_bmp.free_lines();
	}
	tiles = old_tiles;
	font_set_tab(g_font, old_tab, 0);
	close();
	if (line_offs != 0)
		mem_free_v(line_offs);
	set_width(prev_tiles);
	if (g_in_fight)
		mouse_hide(0);
	pic_frame = 1;
	return ret;
}

void Dlg::layout()
{
	va_list ap;
	signed short again;

	do {
		w = (tiles << 5) + 0x40;
		x = calc_x(w);
		font_set_box(g_font, x + FRAME, w - 2 * FRAME, -1, -1);
		calc_text();
		old_tab = g_font_tab0;
		font_set_tab(g_font, x + w - 0x24, 0);
		if (title != 0)
			lines += font_wrap(g_font, title) + 1;
		if (lines < text_y)
			lines = text_y;
		ap = choices;
		choice_lines = count_lines(n_choices, ap);
		total_lines = lines + choice_lines;
		again = 0;
		if (total_lines > MAX_LINES) {
			if ((tiles += 2) < 9)
				again = 1;
			else
				guru_meditation("Text ist viel, viel zu lange!!!!", (unsigned char huge *)body);
		}
	} while (again);
}

void Dlg::wait_message()
{
	signed long lim;
	signed long t0;
	signed short dt;

	t0 = g_random_seed2;
	if (g_in_fight && all_heroes_down(g_group))
		lim = lines * (g_fight_speed * 2 + 2) * 12;
	else
		lim = lines * (g_text_speed * 2 + 2) * 12;
	do {
		g_dlg_areas[0].x0 = x;
		g_dlg_areas[0].x1 = x + w;
		g_dlg_areas[0].y0 = y + 1;
		g_dlg_areas[0].y1 = y + 0x0e;
		g_mouse_areas2 = g_dlg_areas;
		poll_input();
		g_mouse_areas2 = 0;
		if (g_key == KEY_PGUP || g_key == KEY_PGDN || g_mouse_rclick != 0) {
			g_action = ACT_CLOSE;
			g_key_ascii = CH_ESC;
			g_mouse_rclick = 0;
		}
		if (!g_win_centered && (g_action == ACT_TITLE || g_key_shift != 0) && (g_mouse_ldown != 0 || g_key_shift != 0)) {
			drag(x, y, w, h);
			if (new_x != x || new_y != y) {
				mouse_hide(0);
				restore_bg();
				x = new_x;
				y = new_y;
				calc_text();
				font_set_colors(g_font, COL_MARK, -1);
				font_set_box(g_font, x + FRAME, w - 2 * FRAME, y + FRAME, 0);
				font_set_indent(g_font, text_x, text_y);
				draw_choices(n_choices, choices);
				print_title();
				print_body();
				draw_pic();
				print_choices();
				font_set_indent(g_font, 0, 0);
				g_action = 0;
				mouse_show(0);
			}
		}
		dt = t0 - g_random_seed2;
		if ((signed long)__abs__(dt) > lim)
			return;
		if (anim != 0 && !(Anim_running(anim) & 1) && ANIM->nr_seqs > 1)
			Anim_select_seq(anim, 1);
	} while (g_action != ACT_CLOSE && g_key_ascii == 0);
}

void Dlg::calc_text()
{
	text_x = text_y = 0;
	if (bmp != 0 || anim != 0) {
		text_x = pic_w + 6;
		text_y = pic_h / 8 + (pic_h % 8 ? 2 : 1);
	} else if (pic != 0) {
		text_x = 0x26;
		text_y = 5;
	}
	if (title != 0)
		text_y -= font_wrap(g_font, title) + 1;
	font_set_indent(g_font, text_x, text_y);
	lines = font_wrap(g_font, body);
	strcpy(body, text_buf);
	if (title != 0)
		text_y += font_wrap(g_font, title) + 1;
	font_set_indent(g_font, text_x, text_y);
}
