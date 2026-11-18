/* OVL110 - Overlay O110 (Segment 0x2176 in SCHWEIF.EXE): weitere Methoden des mehrzeiligen Texteingabefelds
 * Edit (Klasse 0x38 Bytes, Konstruktor/Destruktor und Rest in O111, Layout aus OVL111.CPP uebernommen).
 * f_0000 getkey: Taste holen (R18_057D), '"' wird '>'. f_0017 run(out): Eingabeschleife (alte Werte ds:8bdb/8bdf
 * sichern und loeschen, Zeichen aus der Tabelle ds:6288 einfuegen (O111_0470), Esc/Bild hoch/ds:8bb5 = Ende,
 * Backspace O111_02A6, Enter newline, Cursortasten O111_05D1/0723/0888/09DF/07F2/06DD, Mausklick ausserhalb
 * (R18_0418) = Ende; neu zeichnen je nach Merker +14/+16/+18), am Ende alle Zeilen hintereinander nach out.
 * f_05f5 prepend(d, s): Wort s vor Zeile d setzen. f_066f measure(s): Pixelbreite (Zeichenbreite + 1 je Zeichen).
 * f_06a1 wrap(l): Zeilenumbruch ab Zeile l (letztes Wort in die Folgezeile, Cursor mitfuehren). f_09e7 redraw:
 * alle Zeilen (Hintergrund aus Bild +20 per R42_04A9 oder Linien R45_00F1). f_0aff redrawline. f_0bdf curon /
 * f_0d41 curoff (Zaehler +1a, Zeichen unter dem Cursor invers). f_0e4c newline (Rest der Zeile in die Folgezeile).
 * f_0f57 join (Backspace in Spalte 0: erstes Wort an die Vorzeile haengen). f_1140 settext(s): Text mit CR/LF
 * auf die Zeilen verteilen. f_11f4 cleartails: alles hinter dem ersten Nullbyte jeder Zeile loeschen.
 *
 * BCC -mlarge -3 -Ot -Og -Oi -c -Y -E..\bc\BIN\TASM.EXE OVL110.CPP -> 4704/4704 Bytes GLEICH (18.10.2026),
 *   cobjfix laeuft. Ohne -Oi 2899 Abweichungen, ohne -Og 981.
 * - Erste Fassung bis auf curon gleich: "f1a++; if (f1a == 0)" statt "if (++f1a == 0)" (sonst inc / jz ohne
 *   neues les/cmp, 245 Bytes Verschiebung).
 * - run: "if (!(unsigned char)lines[line][col + 1])" -> mov al / mov ah,0 / or ax,ax (ohne Cast cbw, mit == 0 cmp
 *   byte). Zwei getrennte Stores "line = 0; col = 0;" (Kette waere rechts->links).
 * - wrap: "while (!*--q) k++;" ergibt das eigenartige "mov es,[bp-0ch] / mov ebx,[bp-0eh]" von selbst.
 * - newline: "*(s + col) = 0" (les ax,s / add ax,col) vs "s[col - 1]" (-Og-Zwischenwert [bp-8] fuer s + col).
 * - strcat(prev, buf) mit lokalem Puffer als Quelle: Intrinsic ohne asm.
 * Originalfehler/Auffaelligkeiten:
 * - wrap: "if (l >= nlines) { s[col--] = 0; return; }" steht in der Schleife "l < nlines" -> nie wahr (gemeint
 *   wohl die letzte Zeile; Ueberlauf landet in der Reservezeile lines[nlines]).
 * - wrap: nach memset(s + p, 0, ..) ist s[col] fuer col >= p immer 0 -> Zweig "col = len - 1; cx = x +
 *   measure(s)" tot (und er wuerde die alte Zeile statt des verschobenen Worts messen).
 * - redrawline loescht den Hintergrund ohne Bild in ds:b668 statt im Zielpuffer +24 (redraw/run nehmen +24).
 * - run: Maus-Rechteck R18_0418(x, y, x + buflen, ..) nimmt die Pufferlaenge (2 * Breite) als Pixelbreite.
 * - newline in Spalte 0 liest/schreibt s[col - 1] = s[-1] (Byte vor dem Zeilenpuffer).
 * (Namensfassung mit schweif.h, class Edit vollstaendig in s_class.h: 19.10.2026 GLEICH, LINKGLEICH) curon/curoff ->
 *   cursor_on/cursor_off, cleartails -> clear_tails; Methoden aus O111 mit ihren neuen Namen; Font-Methoden der alten
 *   Sicht als freie Funktionen font_* (g_font).
 */
#include <string.h>
#include "schweif.h"

#define FONT_SET_COLORS	((void (*)(void far *, signed short, signed short))font_set_colors)	/* Farben als short (alte Sicht) */

void Edit::getkey()
{
	poll_input();
	if (g_key_ascii == 0x22)
		g_key_ascii = 0x3e;
}

void Edit::run(char far *out)
{
	signed short i;
	signed short len;
	signed short done;
	void far *s1;
	void far *s2;
	struct rect r;

	done = 0;
	line = 0;
	col = 0;
	cx = x;
	cy = y;
	s1 = g_mouse_areas;
	s2 = g_mouse_areas2;
	g_mouse_areas = 0;
	g_mouse_areas2 = 0;
	mouse_clear_left();
	cursor_cnt = -1;
	redraw();
	cursor_on();
	flush();
	do {
		need_redraw = 0;
		need_line = 0;
		need_char = 0;
		getkey();
		if (is_in_byte_array(g_key_ascii, g_edit_chars) != -1) {
			cursor_off();
			insert_char(g_key_ascii);
			cursor_on();
			flush();
		}
		if (g_action == KEY_ESC || g_action == KEY_PGUP || g_mouse_rclick != 0)
			done = 1;
		else if (g_key_ascii == 8)
			del_char();
		else if (g_key_ascii == 0x0d) {
			if (line + 1 < nlines)
				newline();
		} else if (g_action == KEY_LEFT)
			cursor_left();
		else if (g_action == KEY_RIGHT)
			cursor_right();
		else if (g_action == KEY_UP)
			line_up();
		else if (g_action == KEY_DOWN)
			line_down();
		else if (g_action == KEY_END)
			end();
		else if (g_action == KEY_HOME)
			home();
		if (g_mouse_lclick) {
#if FIX_0109
			/* Fix #109: Mausbereich mit der Breite in Pixeln */
			if (!is_mouse_in_rect(x, y, x + width, y + nlines * line_h))
#else
			if (!is_mouse_in_rect(x, y, x + buflen, y + nlines * line_h))
#endif
				done = 1;
			mouse_clear_left();
		}
		if (need_redraw == 1) {
			redraw();
			flush();
		} else if (need_line == 1) {
			redraw_line();
			flush();
		} else if (need_char == 1) {
			if (!(unsigned char)lines[line][col + 1]) {
				cursor_off();
				mouse_hide(0);
				r = g_clip_rect;
				g_clip_rect.x0 = cx;
				g_clip_rect.x1 = cx + font_char_width(g_font, g_key_ascii);
				if (bg_pic)
					gb_blit(bg_pic, scr, 0, cy);
				else
					for (i = 0; i < line_h; i++)
						do_h_line(scr, x, cy + i, width, col_bg);
				g_clip_rect = r;
				font_put_char(g_font, g_key_ascii, cx, cy);
				mouse_show(0);
				cx += font_char_width(g_font, g_key_ascii) + 1;
				cursor_on();
				flush();
			} else {
				cursor_off();
				mouse_hide(0);
				if (bg_pic)
					gb_blit(bg_pic, scr, 0, cy);
				else
					for (i = 0; i < line_h; i++)
						do_h_line(scr, x, cy + i, width, col_bg);
				font_print(g_font, lines[line], x, cy);
				mouse_show(0);
				cx += font_char_width(g_font, g_key_ascii) + 1;
				cursor_on();
				flush();
			}
		}
	} while (!done);
	cursor_off();
	g_mouse_areas = s1;
	g_mouse_areas2 = s2;
	for (i = 0; i < nlines; i++) {
		len = strlen(lines[i]) + 1;
		strcpy(out, lines[i]);
		out += len;
	}
}

void Edit::prepend(char far *d, char far *s)
{
	signed short n;

	if (*s == ' ')
		s++;
	n = strlen(s);
	memmove(d + n + 1, d, strlen(d));
	if (d[n + 1] != 0)
		d[n] = ' ';
	while (*s != 0) {
		*d = *s;
		s++;
		d++;
	}
}

signed short Edit::measure(char far *s)
{
	signed short n;

	n = 0;
	while (*s != 0)
		n += font_char_width(g_font, *s++) + 1;
	return n;
}

void Edit::wrap(signed short l)
{
	signed short p;
	signed short len;
	signed short flag;
	char far *s;
	char far *q;
	signed short k;
	char buf[40];
	char buf2[80];

	flag = 0;
	for (; l < nlines; l++) {
		s = lines[l];
		while (measure(s) > width) {
			if (l >= nlines) {
				s[col--] = 0;
				return;
			}
			p = last_space(s);
			if (p != 0) {
				strcpy(buf, s + p + 1);
				memset(s + p, 0, buflen - p);
				len = strlen(buf);
				if (len + strlen(lines[l + 1]) < buflen)
					prepend(lines[l + 1], buf);
				if (line == l) {
					if (col >= p) {
						next_line();
						if (*(s + col) != 0) {
							col = len - 1;
							cx = x + measure(s);
						} else {
							k = 0;
							memset(buf2, 0, 80);
							q = s + col;
							while (!*--q)
								k++;
							if (l == nlines - 1) {
								col -= k + 1;
								cx = x + measure(s);
							} else {
								col = k - 1;
								strncpy(buf2, lines[l + 1], k);
								cx = x + measure(buf2);
							}
						}
						flag = 1;
					} else if (!flag) {
						cx += font_char_width(g_font, *(s + col));
						flag = 1;
					}
				}
				need_redraw = 1;
			} else {
				col--;
				s[strlen(s) - 1] = 0;
				need_line = 1;
			}
		}
	}
}

void Edit::redraw()
{
	signed short i;
	signed short yo;
	signed short j;

	clear_tails();
	cursor_off();
	mouse_hide(0);
	for (i = yo = 0; i < nlines; i++, yo += line_h) {
		if (bg_pic)
			gb_blit(bg_pic, scr, 0, y + yo);
		else
			for (j = 0; j < line_h; j++)
				do_h_line(scr, x, y + yo + j, width, col_bg);
		font_print(g_font, lines[i], x, y + yo);
	}
	cursor_on();
	mouse_show(0);
}

void Edit::redraw_line()
{
	signed short j;

	cursor_off();
	mouse_hide(0);
	if (bg_pic)
		gb_blit(bg_pic, scr, 0, cy);
	else
		for (j = 0; j < line_h; j++)
#if FIX_0109
			/* Fix #109: Hintergrund im eigenen Zielpuffer scr loeschen */
			do_h_line(scr, x, cy + j, width, col_bg);
#else
			do_h_line(g_vga_buf, x, cy + j, width, col_bg);
#endif
	font_print(g_font, lines[line], x, cy);
	mouse_show(0);
	cursor_on();
}

void Edit::cursor_on()
{
	char c;
	struct rect r;

	cursor_cnt++;
	if (cursor_cnt == 0) {
		if (!(c = lines[line][col]))
			c = ' ';
		FONT_SET_COLORS(g_font, col_bg, col_fg);
		mouse_hide(0);
		if (bg_pic) {
			r = g_clip_rect;
			g_clip_rect.x0 = cx;
			g_clip_rect.x1 = cx + font_char_width(g_font, c);
			gb_blit(bg_pic, scr, 0, cy);
			g_clip_rect = r;
		}
		font_put_char(g_font, c, cx, cy);
		mouse_show(0);
		if (bg_pic) {
			FONT_SET_COLORS(g_font, col_fg, -1);
			return;
		}
		FONT_SET_COLORS(g_font, col_fg, col_bg);
	}
}

void Edit::cursor_off()
{
	char c;
	struct rect r;

	if (cursor_cnt == 0) {
		if (!(c = lines[line][col]))
			c = ' ';
		mouse_hide(0);
		if (bg_pic) {
			r = g_clip_rect;
			g_clip_rect.x0 = cx;
			g_clip_rect.x1 = cx + font_char_width(g_font, c);
			gb_blit(bg_pic, scr, 0, cy);
			g_clip_rect = r;
		}
		font_put_char(g_font, c, cx, cy);
		mouse_show(0);
	}
	cursor_cnt--;
}

void Edit::newline()
{
	char far *s;

	s = lines[line];
	if (line + 1 < nlines) {
		prepend(lines[line + 1], s + col);
		wrap(line + 1);
	}
	*(s + col) = 0;
#if FIX_0109
	/* Fix #109: in Spalte 0 nicht s[-1] lesen/schreiben */
	if (col > 0 && s[col - 1] == ' ')
		s[col - 1] = 0;
#else
	if (s[col - 1] == ' ')
		s[col - 1] = 0;
#endif
	cursor_off();
	cx = x;
	next_line();
	col = 0;
	cursor_on();
	need_redraw = 1;
}

void Edit::join()
{
	signed short wl;
	signed short w;
	signed short n;
	char far *prev;
	char far *cur;
	char buf[80];

	if (line != 0) {
		memset(buf, 0, 80);
		prev = lines[line - 1];
		cur = lines[line];
		n = strlen(prev);
		w = measure(prev);
		wl = word_len(cur);
		if (*prev != 0) {
			buf[0] = ' ';
			strncpy(buf + 1, cur, wl);
		} else
			strncpy(buf, cur, wl);
		memmove(cur, cur + wl + 1, buflen - wl - 1);
		strcat(prev, buf);
		wrap(line - 1);
		cursor_off();
		cx = x + w;
		col = n;
		prev_line();
		cursor_on();
		need_redraw = 1;
	}
}

void Edit::settext(char far *s)
{
	signed short i;
	char far *t;

	for (i = 0; i < nlines; i++) {
		t = s;
		while (*s != 0 && *s != 0x0d && *s != 0x0a)
			s++;
		*s = 0;
		s++;
		strcpy(lines[i], t);
	}
	cursor_off();
	redraw();
	flush();
}

void Edit::clear_tails()
{
	signed short i;
	char far *s;
	signed short flag;
	signed short j;

	for (i = 0; i < nlines; i++) {
		s = lines[i];
		for (j = flag = 0; j < buflen; j++) {
			if (!s[j])
				flag = 1;
			if (flag)
				s[j] = 0;
		}
	}
}
