/* OVL225 - Overlay O225 (Segment 0x23e6 in SCHWEIF.EXE): Auswahlliste Sel (s_class.h, 0x24 Bytes; Aufrufer O103
 * Tagebuchsuche) - Suchbegriffe aus Datei 0x334 (struct sel_rec, 0x17 Bytes {id, avail, Stufe, Name[20]}, Ende
 * id == -1), Rahmenbild 0x333 (154 x 117) ueber ein lokales Pic-Objekt (O269). sel_compar: qsort-Vergleich (strcmp).
 * Sel(): Konstruktor (Puffer: Eingabe 20 Bytes, Liste 350 x 0x16, Bmp 0x9a x 0x75, Bildpuffer 0x9240, Datei ganz
 * lesen ueber g_ofile). ~Sel(): Destruktor. run(titel, lo, hi): Liste fuellen, alphabetisch sortieren (qsort,
 * Vergleich ueber Overlay-Thunk 0x4d), Auswahlschleife; Ergebnis (Stufe << 8) + id bzw. -1. fill(lo, hi): Eintraege
 * mit avail und Stufe lo..hi (hi == -1 -> nur lo). loop(): Maus (poll_input mit g_mouse_areas2 = g_sel_list_areas, Zeilen
 * 8 Pixel ab y 0x44), Tasten hoch/runter/Bild/Pos1/Ende, Rollbalken-Klick, ESC/Return, Buchstaben (auch Umlaute) ->
 * type_search(), am Ende flash(). draw(): Titel zentriert mit Schatten, 10 Zeilen ab top (aktuelle in Farbe 0xff),
 * Rollbalken, Ausgabe nach g_vga_buf (82, 40). scrollbar(): Rollbalken (Hoehe/Lage aus n und top, mind. 4 Pixel).
 * flash(): Auswahl dreimal blinken (delay 80). type_search(): Tippsuche - Eingabe input ergaenzen bzw. Backspace,
 * Vergleich in Kleinbuchstaben (str_tolower) mit strncpy auf die Eingabelaenge, cur wandert zum Treffer.
 *
 * BCC -mlarge -3 -Ot -Og -Oi -c -Y OVL225.CPP   -> 4656/4656 Bytes GLEICH (18.10.2026), cobjfix laeuft
 *   (Namensfassung mit schweif.h: 19.10.2026 GLEICH, LINKGLEICH)
 * - -Oi fuer strcmp/strcpy/strncpy/strlen/memset (strncpy mit sub dx,cx / jnc / rep stosb); -Og fuer viele
 *   Zwischenwerte (n in DI, cur in [bp-0ah], cur * 0x16, top + 9, n - 10) - ohne -Og 52 Bytes laenger.
 * - C++: Konstruktor mit "this == 0 -> new(0x24)", Destruktor mit flags, new Bmp(..) -> gb_new_alloc(0, ..),
 *   delete bmp -> gb_delete(bmp, 3); delete items/input -> mem_free_v; lokales Pic (O269) mit Ktor/Dtor.
 * - loop(): "register sel" im if-Block der Mausabfrage (SI), x/y = 0x5e/0x43 im inneren Block (Stack, [bp-12h]/
 *   [bp-14h]); auf Funktionsebene bekommt sel DI und x SI. Rollbalken-Klick: Block-Variable my2 = g_mouse_posy -> CX.
 * - scrollbar(): c als register-Variable im if-Block ohne Aufrufe -> CX; der Bmp-Zeiger in einem eigenen Block
 *   (liegt dann hinter dem -Og-Zwischenwert 0x63 - bar_h, [bp-6] statt [bp-4]).
 * - draw(): "i + top" dreimal ohne Variable (-Og-Zwischenwert in DI, nach dem Aufruf font_set_colors neu berechnet);
 *   die Titel-y-Koordinate 0x0b teilt sich SI mit dem Schleifenzaehler.
 * - "g_mouse_areas2 = 0" (far-Zeiger) -> mov dword; "g_mouse_areas2 = g_sel_list_areas" -> mov [..],ds / mov word.
 * - font_set_colors(g_font, c0, c1) mit int-Variablen (push word) ueber gecasteten Funktionszeiger.
 * Originalfehler/Auffaelliges: type_search() setzt mit "input[len] = g_key_ascii = 0" auch die Taste zurueck
 *   (gewollt?); mem_alloc-/new-Ergebnisse werden nicht geprueft; die Liste fasst 350 Eintraege, fill() prueft keine
 *   Grenze.
 */
#include <stdlib.h>
#include <string.h>
#include <dos.h>
#include "schweif.h"

/* Phase B: eigene Daten des Moduls (ds:729a..72cc, dsana) - initialisierte Daten in Adressreihenfolge */
struct mouse_area g_sel_list_areas[5] = {
	{ 92, 68, 210, 147, 1 },
	{ 215, 65, 228, 73, 2 },
	{ 215, 142, 228, 150, 3 },
	{ 217, 76, 226, 139, 4 },
	{ -1, -1, -1, -1, -1 }
};

int sel_compar(const void far *a, const void far *b)
{
	return strcmp((char far *)a, (char far *)b);
}

Sel::Sel()
{
	signed long len;
	Pic o;

	input = new char[0x14];
	items = new struct sel_item[350];
	bmp = new Bmp(0x9a, 0x75);
	buf = mem_alloc(0x9240);
	o.load(0x333, bmp->data());
	ofile_open(g_ofile, 0x334);
	len = ofile_length(g_ofile);
	recs = (struct sel_rec far *)mem_alloc(len);
	ofile_read(g_ofile, recs, len);
	ofile_close(g_ofile);
	top = cur = 0;
	done = 0;
}

Sel::~Sel()
{
	delete bmp;
	delete items;
	delete input;
	mem_free(buf);
	mem_free(recs);
}

signed short Sel::run(char far *t, signed char lo, signed char hi)
{
	title = t;
	n = fill(lo, hi);
	qsort(items, n, sizeof(struct sel_item), sel_compar);
	return loop();
}

signed short Sel::fill(signed char lo, signed char hi)
{
	signed short i = 0;
	signed short k = 0;

	if (hi == -1)
		hi = lo;
	while (recs[i].id != -1) {
#if FIX_0108
		/* Fix #108: Liste (350 Eintraege) nicht ueberfuellen */
		if (k < 350 && recs[i].lvl >= lo && recs[i].lvl <= hi && recs[i].avail) {
#else
		if (recs[i].lvl >= lo && recs[i].lvl <= hi && recs[i].avail) {
#endif
			items[k].id = recs[i].id;
			items[k].lvl = recs[i].lvl;
			strcpy(items[k].name, recs[i].name);
			k++;
		}
		i++;
	}
	return k;
}

signed short Sel::loop()
{
	signed short my = g_mouse_posy;
	signed short ret = -1;
	signed short c1, c0;

	font_get_colors(g_font, &c0, &c1);
	font_set_colors(g_font, 0xe0, -1);
	g_refresh = 1;
	while (!done) {
		if (g_refresh) {
			draw();
			g_refresh = 0;
		}
		g_mouse_areas2 = g_sel_list_areas;
		poll_input();
		g_mouse_areas2 = 0;
		if (my != g_mouse_posy && is_mouse_in_rect(0x5c, 0x44, 0xd2, 0x93)) {
			register signed short sel;

			sel = top + ((g_mouse_posy - 0x44) >> 3);
			if (cur != sel && sel >= 0 && n > sel) {
				signed short x;
				signed short y;

				x = 0x5e;
				y = 0x43;

				mouse_hide(0);
				font_print(g_font, items[cur].name, x, y + ((cur - top) << 3));
				cur = sel;
				font_set_colors(g_font, 0xff, -1);
				font_print(g_font, items[cur].name, x, y + ((cur - top) << 3));
				font_restore_colors(g_font);
				mouse_show(0);
				my = g_mouse_posy;
			}
		}
		if (g_action == 2 || g_key == 0x4800) {
			if (cur != 0) {
				g_refresh = 1;
				cur--;
				if (cur < top) {
					top -= 9;
					if (top < 0)
						top = 0;
				}
			}
		} else if (g_action == 3 || g_key == 0x5000) {
			if (cur < n - 1) {
				g_refresh = 1;
				cur++;
				if (cur > top + 9) {
					if (top < n - 10)
						top += 9;
				}
			}
		} else if (g_action == 4) {
			signed short my2 = g_mouse_posy;

			if (bar_y + (bar_h >> 1) + 0x28 >= my2) {
				if (g_mouse_ldclick)
					g_key = 0x4700;
				else
					g_key = 0x4900;
			} else {
				if (g_mouse_ldclick)
					g_key = 0x4f00;
				else
					g_key = 0x5100;
			}
			g_mouse_ldclick = 0;
		} else if (g_mouse_rclick || g_key == 0x11b) {
			done = 1;
			cur = -1;
		} else if (g_key == 0x1c0d || g_action == 1) {
			done = 1;
		} else if ((g_key_ascii >= 'A' && g_key_ascii <= 'Z') || (g_key_ascii >= 'a' && g_key_ascii <= 'z') ||
			g_key_ascii == 0x84 || g_key_ascii == 0x94 || g_key_ascii == 0x81 || g_key_ascii == 0x8e ||
			g_key_ascii == 0x99 || g_key_ascii == 0x9a || g_key_ascii == 0xe1 || g_key_ascii == 8)
			type_search();
		if (g_key == 0x4900) {
			if (top != 0) {
				top -= 9;
				if (top < 0)
					top = 0;
				if (cur > top + 9)
					cur = top + 9;
			} else
				cur = 0;
			g_refresh = 1;
		} else if (g_key == 0x5100) {
			if (top < n - 10) {
				top += 9;
				if (top > n - 10)
					top = n - 10;
				if (cur < top)
					cur = top;
			} else
				cur = n - 1;
			g_refresh = 1;
		} else if (g_key == 0x4700) {
			if (top != 0)
				top = 0;
			g_refresh = 1;
			cur = 0;
		} else if (g_key == 0x4f00) {
			if (top < n - 10)
				top = n - 10;
			g_refresh = 1;
			cur = n - 1;
		}
	}
	if (cur != -1) {
		ret = (items[cur].lvl << 8) + items[cur].id;
		flash();
	}
	((void (*)(void far *, signed short, signed short))font_set_colors)(g_font, c0, c1);
	return ret;
}

void Sel::draw()
{
	signed short x;
	void far *old;
	Bmp far *b;
	register signed short i;

	x = 0x4d - (font_text_width(g_font, title) >> 1);
	i = 0x0b;
	old = font_set_screen(g_font, buf);
	gb_blit(bmp, buf, 0, 0);
	font_set_colors(g_font, 0xe7, -1);
	font_print(g_font, title, x + 1, i + 1);
	font_restore_colors(g_font);
	font_set_colors(g_font, 0xe0, -1);
	font_print(g_font, title, x, i);
	font_restore_colors(g_font);
	for (i = 0; i < 10; i++) {
		if (i + top < n) {
			if (i + top == cur) {
				font_set_colors(g_font, 0xff, -1);
				font_print(g_font, items[i + top].name, 0x0c, (i << 3) + 0x1b);
				font_restore_colors(g_font);
			} else
				font_print(g_font, items[i + top].name, 0x0c, (i << 3) + 0x1b);
		}
	}
	font_set_screen(g_font, old);
	scrollbar();
	b = new Bmp(0x9a, 0x75);
	gb_save(b, buf, 0, 0);
	mouse_hide(0);
	gb_blit_m(b, g_vga_buf, 0x52, 0x28, 4);
	mouse_show(0);
	delete b;
}

void Sel::scrollbar()
{
	register signed short d;

	if (n > 10) {
		register signed short c;

		d = n * 100L / 10;
		if (d == 0)
			d = 1;
		bar_h = 620000L / d;
		bar_h /= 100;
		bar_h++;
		if (bar_h < 4)
			bar_h = 4;
		c = n * 100L / 62;
		if (c == 0)
			c = 1;
		bar_y = top * 10000L / c;
		bar_y /= 100;
		bar_y += 0x24;
		if (bar_y > 0x63 - bar_h)
			bar_y = 0x63 - bar_h;
	} else {
		bar_h = 0x3e;
		bar_y = 0x24;
	}
	{
		Bmp far *b = new Bmp(8, bar_h);

		gb_fill(b, 0xe7);
		gb_blit(b, buf, 0x88, bar_y);
		delete b;
	}
	do_h_line(buf, 0x88, bar_y, 7, 0xe5);
	do_v_line(buf, 0x88, bar_y, bar_h - 1, 0xe5);
	do_h_line(buf, 0x89, bar_y + bar_h - 1, 7, 0xe8);
	do_v_line(buf, 0x8f, bar_y + 1, bar_h - 1, 0xe8);
}

void Sel::flash()
{
	signed short y;
	register signed short i;
	register signed short x;

	x = 0x5e;
	y = ((cur - top) << 3) + 0x43;
	mouse_hide(0);
	for (i = 0; i < 3; i++) {
		delay(0x50);
		font_print(g_font, items[cur].name, x, y);
		delay(0x50);
		font_set_colors(g_font, 0xff, -1);
		font_print(g_font, items[cur].name, x, y);
		font_restore_colors(g_font);
	}
	delay(0x50);
	mouse_show(0);
}

void Sel::type_search()
{
	signed short old = cur;
	signed short len = strlen(input);
	signed short stop = 0;
	char a[20];
	char b[20];

	memset(a, 0, 20);
	memset(b, 0, 20);
	cur = 0;
	if (g_key_ascii == 8 && len != 0) {
		len--;
		input[len] = g_key_ascii = 0;
		strncpy(a, input, len);
		str_tolower(a);
		strncpy(b, items[cur].name, len);
		str_tolower(b);
	} else if (len < 0x13) {
		strncpy(a, input, len);
		a[len] = g_key_ascii;
		str_tolower(a);
		len++;
		strncpy(b, items[cur].name, len);
		str_tolower(b);
	} else
		stop = 1;
	while (!stop) {
		if (strcmp(a, b) < 0) {
			if (cur != 0) {
				cur--;
				strncpy(b, items[cur].name, len);
				str_tolower(b);
			} else
				stop = 1;
		} else if (strcmp(a, b) == 0) {
			strcpy(input, a);
			stop = 1;
		} else {
			strncpy(b, items[cur + 1].name, len);
			str_tolower(b);
			if (strcmp(a, b) >= 0) {
				if (cur < n - 1)
					cur++;
				else
					stop = 1;
			} else
				stop = 1;
		}
	}
	if (cur != old) {
		g_refresh = 1;
		while (cur < top)
			top -= 9;
		if (top < 0)
			top = 0;
		while (cur > top + 9)
			top += 9;
		if (top > n - 10)
			top = n - 10;
	}
}
