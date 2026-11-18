/* SEG020 - Textausgabe mit Zeichensatz (Proportionalschrift), Objekt struct font (g_font): Zeichensatz
 * einrichten (0x58 Zeichen, Zuordnung Tabellennummer -> Zeichencode in g_font_charmap), Zeichenbreiten,
 * Textbreite messen, Zentrieren, Zeilenumbruch (font_wrap), Text ausgeben mit Steuerzeichen (font_print:
 * CR/LF/@ = neue Zeile, ~ = Tabulator, 0xf1..0xf4 = Farbsatz, 0xf5/0xf0 = Schatten an/aus, ^ = Leerzeichen,
 * Grossschreibung nach . ! ?), Farb-/Kasten-/Einzug-Einstellungen der Schriftstruktur.
 * (seg020 in SCHWEIF.EXE, 3220 Bytes, 36 Funktionen)
 *
 * C++! BCC -mlarge -3 -Ot -Og -Oi -c -Y SEG020.CPP   -> 3220/3220 Bytes GLEICH (18.10.2026)
 *   (Namensfassung mit schweif.h: 19.10.2026 GLEICH)
 * (gleichwertig: zusaetzlich -Ol und/oder -Oc; ohne -Og oder -Oi oder mit -1/-2 nicht gleich)
 *
 * Schluessel-Erkenntnisse:
 * - font_init: die Locals [bp-0Ch]/[bp-0Dh]/[bp-0Eh] sind die Parameterkopien der inline-Memberfunktion
 *   glyph::set(off, idx, width); [bp-10h] und DI sind -Og-Zwischenwerte (int)h bzw. (int)w; x muss
 *   "register" sein, der Zeiger auf den Tabellenplatz ist ein echter Local (vor dem Allokator-Aufruf berechnet).
 * - -Oi wegen memcpy (5-Byte-Farbsatz sichern/zuruecksetzen wird zu rep movsw/movsb eingebaut);
 *   eine Strukturzuweisung erzeugt dagegen einen SCOPY-Aufruf.
 * - font_glyph: "return *w = 0;" (xor ax,ax / mov es:[bx],ax); toter Sprung nach dem ersten return = else if.
 * - font_rect: "!F->transp ? 0 : 4" erzwingt die int-Erweiterung (mov ah,0 / or ax,ax).
 * - font_center_x: Zeichen als int-Local c; Ergebnis ueber "width -= n; width >>= 1; width += x" in DI.
 * - font_wrap: i/space brauchen "register", sonst nimmt der -Og-Zwischenwert &p[i] das DI;
 *   Vorruecken als p = &p[i + 1] (nicht p + i + 1); if (s) {...return} else return 0 (zwei tote Epiloge).
 * - font_print: [bp-0Ch..-18h] sind -Og-Zwischenwerte der Tabulator-Vergleiche; leerer then-Zweig
 *   in der Grossschreib-Abfrage (Sprung auf Sprung im Original).
 * - Die Schriftstruktur ist gepackt (Felder an ungeraden Offsets); alles als freie Funktionen mit dem
 *   Schriftobjekt (void far * wie in s_proto.h, Zugriff ueber den Cast-Makro F) als erstem Parameter
 *   (gleiches Aufrufbild wie Memberfunktionen). In C++ muss jede Definition exakt dem Prototyp entsprechen
 *   (sonst zweite Funktion) - die R20-Prototypen stehen daher fest in proto_fest.tsv.
 * - font_rect1: lokales Bmp-Objekt (Konstruktor Bmp(w, h, p) = gb_new, Destruktor mit Flag 2 = gb_delete);
 *   Bmp::put = gb_blit_m (Modus short, seit N5 auch im freien Prototyp).
 */
#include <mem.h>
#include "schweif.h"

/* Phase B: eigene Daten des Moduls (ds:75d8..7638, dsana) - initialisierte Daten in Adressreihenfolge, danach der Literalpool */
char g_str_empty_75d8[6] = "";

#define F	((struct font far *)font)
#define G_FONT	((struct font far *)g_font)

void font_init(void far *font)
{
	signed short i;
	unsigned char code;
	unsigned char w;
	unsigned char h;
	struct glyph far * far *g;
	register signed short x;

	F->max_w = F->max_h = x = i = 0;
	for (; i < NR_FONT_CHARS; i++) {
		w = F->file->e[i].w;
		h = F->file->e[i].h;
		code = " ABCDEFGHIJKLMNOPQRSTUVWXYZ\x8e\x99\x9a""1234567890abcdefghijklmnopqrstuvwxyz\x84\x94\x81\xe1()!?:<>/_,.'-[];&"[i];
		g = &F->glyphs[code];
		*g = (struct glyph far *)mem_alloc(sizeof(struct glyph));
		(*g)->set(x, i, w);
		if (w > F->max_w)
			F->max_w = w;
		if (h > F->max_h)
			F->max_h = h;
		x += w;
	}
}

signed short font_glyph(void far *font, unsigned char c, signed short far *w)
{
	struct glyph far * far *g = &F->glyphs[c];

	if (*g) {
		*w = (*g)->width;
		return (*g)->idx;
	}
	if (c == FONT_CH_TAB || c == 0xf1 || c == 0xf2 || c == 0xf3 || c == 0xf4) {	/* Steuerzeichen */
		return *w = 0;
	} else if (c == FONT_CH_SHADOW_ON || c == FONT_CH_NORMAL) {
		return *w = 0;
	}
	*w = 5;		/* nicht im Zeichensatz */
	return 0;
}

void font_dummy1(void)
{
}

unsigned char far *font_glyph_bits(void far *font, unsigned char c)
{
	unsigned char far *p = (unsigned char far *)&F->file->e[F->bits_start];
	signed short off = F->glyphs[c]->off;

	p += off * F->max_h;
	return p;
}

void font_dummy2(void)
{
}

void font_draw_glyph(void far *font, unsigned char c, signed short w, signed short x, signed short y)
{
	unsigned char far *dst;
	unsigned char far *src;
	unsigned char far *pal;

	src = font_glyph_bits(font, c) + 1;
	dst = F->screen + x + y * SCREEN_W;
	pal = &F->bg + F->colsel;
	if (F->transp)
		glyph_blit(dst, src, pal, w, F->max_h);
	else
		glyph_blit_bg(dst, src, pal, w, F->max_h, F->bg);
}

signed short font_put_char(void far *font, unsigned char c, signed short x, signed short y)
{
	signed short w;

	font_glyph(font, c, &w);
	font_draw_glyph(font, c, w, x, y);
	F->x = x + w + 1;
	return w + 1;
}

void font_set_x(void far *font, signed short x)
{
	F->x = x;
}

void font_set_y(void far *font, signed short y)
{
	F->y = y;
}

void font_set_xy(void far *font, signed short x, signed short y)
{
	font_set_x(font, x);
	font_set_y(font, y);
}

signed short font_get_y(void far *font)
{
	return F->y;
}

signed short font_char_width(void far *font, unsigned char c)
{
	signed short w;

	font_glyph(font, c, &w);
	return w;
}

signed short font_text_width(void far *font, char far *s)
{
	unsigned char c;
	signed short w;
	signed short n = 0;

	while ((c = *s++) != 0) {
		font_glyph(font, c, &w);
		n += w + 1;
	}
	return n;
}

signed short font_height(void far *font)
{
	return F->max_h;
}

void font_get_box(void far *font, signed short far *x, signed short far *w, signed short far *y, signed short far *align)
{
	*x = F->box_x;
	*w = F->box_w;
	*y = F->box_y;
	*align = F->align;
}

void font_set_colors(void far *font, unsigned char fg, signed short bg)
{
	F->transp = 0;
	F->old_bg = F->bg;
	F->old_fg = F->cs.fg;
	if (bg == -1) {
		bg = 0;
		F->transp = 1;
	}
	F->cs.fg = fg;
	F->bg = bg;
}

void font_push_colset(void far *font, unsigned char fg, unsigned char c1, unsigned char c2, unsigned char c3, unsigned char c4)
{
	memcpy(&F->save, &F->cs, sizeof(struct colset));
	F->cs.fg = fg;
	F->cs.c1 = c1;
	F->cs.c2 = c2;
	F->cs.c3 = c3;
	F->cs.c4 = c4;
}

void font_pop_colset(void far *font)
{
	memcpy(&F->cs, &F->save, sizeof(struct colset));
}

void font_set_shadow(void far *font, unsigned char v)
{
	F->shadow = v;
}

/* -1 = unveraendert */
void font_set_box(void far *font, signed short x, signed short w, signed short y, signed short align)
{
	if (x != -1)
		F->box_x = F->x = x;
	if (w != -1)
		F->box_w = w;
	if (y != -1)
		F->box_y = F->y = y;
	if (align != -1)
		F->align = align;
}

signed short font_get_tab(void far *font, signed short i)
{
	return F->tabs[i];
}

void font_set_tab(void far *font, signed short x, signed short i)
{
	F->tabs[i] = x;
}

void font_set_indent(void far *font, signed short indent, signed short line)
{
	if (indent != -1)
		F->indent = indent;
	if (line != -1)
		F->indent_line = line;
}

void font_get_indent(void far *font, signed short far *indent, signed short far *line)
{
	*indent = F->indent;
	*line = F->indent_line;
}

void font_restore_colors(void far *font)
{
	F->cs.fg = F->old_fg;
	F->bg = F->old_bg;
}

void font_get_colors(void far *font, signed short far *fg, signed short far *bg)
{
	*fg = F->cs.fg;
	*bg = F->bg;
}

signed short font_get_shadow(void far *font)
{
	return F->shadow;
}

/* Rechteck h x w aus dem Hintergrundbild an (x, y); Blitmodus 4 = durchsichtig */
void font_rect(void far *font, signed short x, signed short y, signed short h, signed short w)
{
	signed short mode;

	mode = !F->transp ? 0 : 4;
	font_rect1(font, x, y, h, w, F->bg_pic, mode);
	if (F->shadow)
		font_rect1(font, x + 1, y, h, w, F->bg_pic, 4);
}

void font_rect1(void far *font, signed short x, signed short y, signed short h, signed short w, void far *pic,
		signed short mode)
{
	Bmp bmp(w, h, pic);

	bmp.put(F->screen, x, y, mode);
}

signed short font_center_x(void far *font, char far *s, signed short x, signed short width)
{
	signed short c;
	signed short w;
	signed short n = 0;

	while ((c = *s) != 0 && c != FONT_CH_NEWLINE && c != '\r' && c != '\n') {
		font_glyph(font, *s++, &w);
		n += w + 1;
	}
	width -= n;
	width >>= 1;
	width += x;
	return width;
}

signed short font_text_width0(void far *font, char far *s)
{
	signed short w;
	signed short n = 0;

	while (*s) {
		font_glyph(font, *s++, &w);
		n += w;
	}
	return n;
}

signed short font_print(void far *font, char far *s, signed short x, signed short y)
{
	signed short lines = 0;
	signed short i = 0;
	signed short x0;
	signed short cap = 0;		/* naechster Buchstabe gross (nach . ! ?) */
	unsigned char c;

	screensaver_reset(&g_saver);
	if (F->screen == g_vga_buf)
		mouse_hide(0);
	if (F->align == FONT_ALIGN_CENTER)
		x = font_center_x(font, s, x, F->box_w);
	else if (F->indent)
		x += F->indent;
	x0 = x;
	while ((c = s[i++]) != 0) {
		if (c == '\r' || c == '\n' || c == FONT_CH_NEWLINE) {
			if (F->indent_line == ++lines)
				x0 -= F->indent;
			y += F->line_h;
			x = (F->align == FONT_ALIGN_CENTER) ? font_center_x(font, s + i, F->box_x, F->box_w) : x0;
		} else if (c == FONT_CH_TAB) {
			if (F->tabs[0] > x)
				x = F->tabs[0];
			else if (F->tabs[1] > x)
				x = F->tabs[1];
			else if (F->tabs[2] > x)
				x = F->tabs[2];
			else if (F->tabs[3] > x)
				x = F->tabs[3];
			else if (F->tabs[4] > x)
				x = F->tabs[4];
			else if (F->tabs[5] > x)
				x = F->tabs[5];
			else if (F->tabs[6] > x)
				x = F->tabs[6];
		} else if (c == FONT_CH_COLSET1 || c == 0xf2 || c == 0xf3 || c == FONT_CH_COLSET4) {
			F->colsel = c + 0x10;
		} else if (c == FONT_CH_SHADOW_ON) {
			F->shadow = 1;
		} else if (c == FONT_CH_NORMAL) {
			F->shadow = 0;
			F->colsel = 0;
		} else {
			if (c == FONT_CH_SPACE)
				c = ' ';
			if (cap) {
				if (c < 0x21 || c == FONT_CH_NEWLINE || c == '.' || c == '!' || c == '?' || c > FONT_CH_NORMAL) {
				} else {
					cap = 0;
					c = char_toupper(c);
				}
			} else if (c == '.' || c == '!' || c == '?') {
				cap = 1;
			}
			x += font_put_char(font, c, x, y);
		}
	}
	if (F->screen == g_vga_buf)
		mouse_show(0);
	return lines + 1;
}

signed short font_print_wrapped(void far *font, char far *s)
{
	signed short n = 1;

	if (F->screen == g_vga_buf)
		mouse_hide(0);
	n = font_wrap(font, s);
	font_print(font, s, F->box_x, F->y);
	F->y += n * F->line_h;
	if (F->screen == g_vga_buf)
		mouse_show(0);
	return n;
}

signed short font_wrap(void far *font, char far *s)
{
	char far *p;
	signed short line_start;
	signed short lines = 0;
	signed short box_w_bak;
	signed short w;
	signed short width;
	register signed short i;
	register signed short space;	/* letztes Leerzeichen der Zeile */

	if (s) {
		for (p = s; *p; p++)
			if (*p == '\r' || *p == '\n')
				*p = ' ';
		p = s;
		i = space = line_start = 0;
		box_w_bak = F->box_w;
		if (F->indent)
			F->box_w -= F->indent;
		for (width = 0; p[i]; i++) {
			font_glyph(font, p[i], &w);
			width += w + 1;
			if (F->box_w <= width) {
				if (space != line_start) {
					p[space] = '\r';
					p = &p[space];
				} else {
					p[i] = '\r';
					p = &p[i + 1];
				}
				if (F->indent_line == ++lines)
					F->box_w += F->indent;
				line_start = i = space = width = 0;
			}
			if (p[i] == ' ')
				space = i;
			if (p[i] == FONT_CH_NEWLINE) {
				p = &p[i + 1];
				i = -1;
				line_start = space = width = 0;
				if (F->indent_line == ++lines)
					F->box_w += F->indent;
			}
		}
		if (F->box_w <= width) {
			if (line_start == space) {
				p[i - 1] = 0;
			} else {
				p[space] = '\r';
				if (F->indent_line == ++lines)
					F->box_w += F->indent;
			}
		}
		F->box_w = box_w_bak;
		return ++lines;
	} else {
		return 0;
	}
}

void far *font_set_screen(void far *font, void far *screen)
{
	unsigned char far *old = F->screen;

	F->screen = (unsigned char far *)screen;
	return old;
}

/* Text mit g_font im Textkasten ausgeben, danach Zeilenumbruch und x an den Kastenanfang */
signed short print_text(char far *s)
{
	font_print_wrapped(g_font, s);
	font_print_wrapped(g_font, "@");
	font_set_x(g_font, G_FONT->box_x);
	return 1;
}
