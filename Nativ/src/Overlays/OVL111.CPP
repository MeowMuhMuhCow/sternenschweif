/* OVL111 - Overlay O111 (Segment 0x217d in SCHWEIF.EXE): mehrzeiliges Texteingabefeld, Klasse Edit (0x38 Bytes,
 * STUFE D1 (wovd1, 19.10.2026): asm-strcat durch echtes strcat(.., "..") ersetzt - Code GLEICH, das Literal
 *   im eigenen _DATA bildet wovd1/cobjfix_d.py auf ds:62e2 ab (Bytes geprueft). Hinweise unten zum asm sind alt.
 * weitere Methoden in O110). Konstruktor f_0000(nl, w, x, y, p, scr, clip): Zeilenpuffer (nl + 1 Zeilen zu 2 * w
 * Bytes, R07_0002), Zeichenhoehe/-masse ueber den Zeichensatz ds:37e4 (O248_01DA, R20_05BD, R20_0407,
 * R20_0C40 = Zielpuffer setzen), Ziel scr oder ds:b668, optional Clip-Rechteck ds:3303 sichern und setzen.
 * Destruktor f_0205: Puffer freigeben, alten Zielpuffer und ggf. Clip-Rechteck (Strukturkopie, SCOPY@) zurueck.
 * f_02a6: Zeichen links vom Cursor loeschen (memmove, Cursor-x um Zeichenbreite + 1 zurueck), Wortlaenge der
 * Folgezeile messen (ohne Wirkung, s.u.); in Spalte 0 O110_0F57. f_0470(c): Zeichen einfuegen (O110_06A1).
 * f_052f(s): letzte Leerstelle, f_055c(s): Wortlaenge. f_057f/f_05ad: Zeile vor/zurueck. f_05d1: Cursor
 * links (ggf. ans Ende der Vorzeile), f_06dd: Pos1, f_0723: Cursor rechts (ggf. Anfang der Folgezeile),
 * f_07f2: Ende, f_0888/f_09df: Zeile hoch/runter mit Spaltenerhalt, f_0b43: Puffer nach ds:b668 kopieren
 * (Maus aus/an R18_001A/R18_0009), wenn das Feld in einen eigenen Puffer zeichnet.
 *
 * BCC -mlarge -3 -Ot -Oi -Og -c -Y -E..\bc\BIN\TASM.EXE OVL111.CPP   -> 2951/2951 Bytes GLEICH (18.10.2026),
 *   cobjfix laeuft (_g_62e2 -> _g_6288+0x5a). Ohne -Oi 444 Abweichungen, ohne -Og 155 Bytes zu kurz.
 * - C++: Konstruktor mit operator new (R07_041C, 0x38), Destruktor mit delete (R07_046C), Methoden ueber this.
 * - -Oi: memset/memmove-Aufrufe bzw. strlen/strncpy/memcpy inline. f_0b43: Kopie des Bildschirms (64000 Bytes)
 *   ist memcpy(g_b668, scr, 64000U) -> push ds / mov ds,ax / rep movsw; die Strukturzuweisung far->far
 *   ergibt dagegen SCOPY@ mit cx = 0xfa00.
 * - Inline-Assembler (einzige Stelle): strcat(buf, " ") mit 1-Zeichen-Literal ds:62e2 (bekannte Laenge:
 *   repne scasb / dec di / mov cx,2 / rep movsb) geht nur mit eigenem Literal im _DATA des Moduls (dann
 *   ebenfalls GLEICH, aber eigene Daten) -> wie OVL238 als asm, xor ax,ax als db (TASM kodiert 31 c0).
 * - -Og: memmove(l + col - 1, l + col, ..) legt den Offset von l + col als Zwischenwert [bp-0ch] ab
 *   (f_0470 in DI); "line = col = 0" speichert rechts->links; "if ((len = strlen(l)) >= col)" -> mov ax,cx.
 * - Leerer Vergleich "if (O110_066f(l) + O110_066f(buf) < width) ;" erzeugt cmp / jnl auf die Folgezeile.
 * - Originalfehler: f_0470 "if (++col == old) return 1;" ist nie wahr (old = col vor dem Erhoehen);
 *   f_0888/f_09df: strncpy(tmp, l, col) in frisch allozierten Puffer ohne Nullterminator, wenn die Zeile
 *   mindestens col Zeichen hat -> O110_066F misst ueber das Ende hinaus; f_02a6: Wortlaenge/Breitenvergleich
 *   der Folgezeile ohne Wirkung (unfertiger Zeilenumbruch).
 * (Namensfassung mit schweif.h, class Edit vollstaendig in s_class.h: 19.10.2026 GLEICH, LINKGLEICH) Methoden: f_02a6
 *   del_char, f_0470 insert_char, f_052f last_space, f_055c word_len, f_057f next_line, f_05ad prev_line, f_05d1
 *   cursor_left, f_06dd home, f_0723 cursor_right, f_07f2 end, f_0888 line_up, f_09df line_down, f_0b43 flush;
 *   Font-Methoden der alten Sicht als freie Funktionen font_* (g_font).
 */
#include <string.h>
#include "schweif.h"

/* Phase B: eigene Daten des Moduls (ds:62e2..62e4, dsana) - initialisierte Daten in Adressreihenfolge, danach der Literalpool */

#define FONT_SET_COLORS	((void (*)(void far *, signed short, signed short))font_set_colors)	/* Farben als short (alte Sicht) */

Edit::Edit(signed short nl, signed short w, signed short px, signed short py,
	void far *p, void far *s, signed short c)
{
	signed short i;

	nlines = nl;
	width = w;
	x = px;
	y = py;
	buflen = w * 2;
	line_h = font_set_line_h(g_font, -1);
	font_get_colors(g_font, &col_fg, &col_bg);
	lines = (char far * far *)mem_alloc((nl + 1) * sizeof(char far *));
	for (i = 0; i < nl + 1; i++)
		lines[i] = (char far *)mem_alloc(buflen);
	cursor_cnt = -1;
	line = col = 0;
	cx = x;
	cy = y;
	bg_pic = p;
	if (bg_pic != 0)
		FONT_SET_COLORS(g_font, col_fg, -1);
	scr = s == 0 ? g_vga_buf : s;
	old_dst = font_set_screen(g_font, scr);
	if (c) {
		clip.x0 = g_clip_rect.x0;
		clip.y0 = g_clip_rect.y0;
		clip.x1 = g_clip_rect.x1;
		clip.y1 = g_clip_rect.y1;
		g_clip_rect.x0 = px;
		g_clip_rect.y0 = py;
		g_clip_rect.x1 = px + w + font_char_width(g_font, ' ') - 1;
		g_clip_rect.y1 = py + nl * line_h;
	} else
		clip.x0 = -1;
}

Edit::~Edit()
{
	signed short i;

	for (i = 0; i < nlines + 1; i++)
		mem_free(lines[i]);
	mem_free(lines);
	font_set_screen(g_font, old_dst);
	if (clip.x0 != -1)
		g_clip_rect = clip;
}

void Edit::del_char()
{
	char c;
	char far *l;
	char far *n;
	char buf[80];

	if (col != 0) {
		memset(buf, 0, 80);
		l = lines[line];
		c = l[col - 1];
		memmove(l + col - 1, l + col, buflen - col);
		l[buflen - 1] = 0;
		col--;
		cursor_off();
		cx -= font_char_width(g_font, c) + 1;
		cursor_on();
		need_line = 1;
		n = lines[line + 1];
		c = word_len(n);
		strncpy(buf, n, c);
		strcat(buf, " ");
		if (measure(l) + measure(buf) < width)
			;
	} else
		join();
}

signed short Edit::insert_char(char c)
{
	signed short old;
	char far *l;

	old = col;
	l = lines[line];
	memmove(l + col + 1, l + col, buflen - col - 1);
	l[col] = c;
	wrap(line);
	if (++col == old)
		return 1;
	else {
		need_char = 1;
		return 0;
	}
}

signed short Edit::last_space(char far *s)
{
	signed short n;

	n = strlen(s);
	while (s[n] != ' ' && n != 0)
		n--;
	return n;
}

signed short Edit::word_len(char far *s)
{
	signed short n;

	n = 0;
	while (*s != 0 && *s != ' ') {
		s++;
		n++;
	}
	return n;
}

void Edit::next_line()
{
	if (line < nlines - 1) {
		line++;
		cy += line_h;
	}
}

void Edit::prev_line()
{
	if (line != 0) {
		line--;
		cy -= line_h;
	}
}

void Edit::cursor_left()
{
	char c;

	cursor_off();
	if (col != 0) {
		if (!(c = lines[line][col - 1]))
			c = ' ';
		cx -= font_char_width(g_font, c) + 1;
		col--;
	} else {
		prev_line();
		col = strlen(lines[line]);
		cx = x + measure(lines[line]);
	}
	cursor_on();
	flush();
}

void Edit::home()
{
	cursor_off();
	col = 0;
	cx = x;
	cursor_on();
	flush();
}

void Edit::cursor_right()
{
	char c;

	c = lines[line][col];
	cursor_off();
	if (strlen(lines[line]) > col) {
		cx += font_char_width(g_font, c) + 1;
		col++;
	} else {
		col = 0;
		next_line();
		cx = x;
	}
	cursor_on();
	flush();
}

void Edit::end()
{
	cursor_off();
	col = strlen(lines[line]);
	cx = x + measure(lines[line]);
	cursor_on();
	flush();
}

void Edit::line_up()
{
	signed short len;
	char far *tmp;
	char far *l;

	tmp = (char far *)mem_alloc(buflen);
	if (line != 0) {
		cursor_off();
		prev_line();
		l = lines[line];
#if FIX_0027
		/* Fix #27: Endnull nach strncpy */
		strncpy(tmp, l, col);
		tmp[col] = 0;
		if ((len = strlen(l)) >= col)
#else
		strncpy(tmp, l, col);
		if ((len = strlen(l)) >= col)
#endif
			cx = x + measure(tmp);
		else {
			col = len;
			cx = x + measure(l);
		}
		cursor_on();
	}
	mem_free(tmp);
	flush();
}

void Edit::line_down()
{
	char far *tmp;
	char far *l;

	tmp = (char far *)mem_alloc(buflen);
	if (line < nlines - 1) {
		cursor_off();
		next_line();
		l = lines[line];
#if FIX_0027
		/* Fix #27: Endnull nach strncpy */
		strncpy(tmp, l, col);
		tmp[col] = 0;
		if (strlen(l) >= col)
#else
		strncpy(tmp, l, col);
		if (strlen(l) >= col)
#endif
			cx = x + measure(tmp);
		else {
			col = strlen(l);
			cx = x + measure(l);
		}
		cursor_on();
	}
	mem_free(tmp);
	flush();
}

void Edit::flush()
{
	if (scr != g_vga_buf) {
		mouse_hide(0);
		memcpy(g_vga_buf, scr, 64000U);
		mouse_show(0);
	}
}
