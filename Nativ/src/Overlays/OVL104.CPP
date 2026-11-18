/* OVL104 - Overlay O104 (Segment 0x2153 in SCHWEIF.EXE): Tagebuch - Seiten anlegen, laden und anzeigen (Objekt
 * struct prn wie O103/O105: +0 Seitenzahl, +2 aktuelle Seite, +4/+8 Puffer, +10 Texte, +1a/+1b Suchbytes, +1c
 * Kapitel; Datei ds:b635 Nr. 0: Kopf Anzahl (2) + Kapitel (1), je Seite Laenge (4) + verschluesselter Text).
 * f_0000(o, s, v): Eintrag s anhaengen (Fenster sichern/setzen R20_03CE/04D8): neue Seite, wenn das Kapitel
 *   ds:b85d wechselt oder letzte Seite (f_110c, Zeilen f_133b) + Eintrag >= 20 Zeilen; Kopf (Steuerzeichen ds:605c,
 *   Ort f_1245, Datum f_12a5), Absatzart ds:605d/605e je v, Text, Ende ds:605f, Namen markieren (f_0456); bei
 *   Fortsetzung alte Seite davor (strcat(old, o->f4) + memcpy zurueck); O105_0000 verschluesseln; Rueckgabe
 *   Laenge + 2 (0 ohne s).
 * f_0456(o): Namensliste Datei 0x334 (0x17-Byte-Saetze) laden, o->f4 nach o->f8 kopieren und zeichenweise zurueck,
 *   Woerter (Buchstaben/Ziffern/Umlaute) ueber f_070c, Liste zurueckschreiben (O243_0291(.., 0x8002)/04BF).
 * f_070c(o, list, dst, word, &j): Wort gegen alle Namen (R30_008D-Kopien, strncmp mit Namenslaenge), Treffer:
 *   Kennung (+2) und Ende-Byte (+0) vor das Wort, Merker +1; Wort anhaengen und leeren.
 * f_08e3(o, page): Seite laden und ausgeben (Ueberschrift 0xfd + Trennzeile txt[13], Hervorhebung 0xf1/0xf0 fuer
 *   Absatzart ds:605e bei o->f1a == -1 bzw. Suchbytes o->f1a/f1b + folgendes Wort); fehlt die Seite: Kopf mit Ort/
 *   Datum (ds:610c). f_0d3d(o, mode, dst): Seite in Puffer (64000 Bytes bei > 80000 freiem Speicher, sonst direkt
 *   ds:b668), mode 1: Kopf lesen, letzte Seite waehlen, Palette (O269_03F8 -> O034_05F4); Hintergrund O269 (Datei
 *   0x332); mode 2: Lesezeichen-Bmp 24x24 sechsmal; Seitenzahl txt[23] bzw. txt[45] zentriert.
 * f_110c(o, page): Seite page lesen (Laengen davor ueberspringen O243_05C6), O105_005B, o->f0 = max(o->f0,
 *   Anzahl). f_1245(o, buf): Ort (*ds:b67c[0x163 + g->f20] oder txt[11]) per txt[10]. f_12a5(o, buf): Datum
 *   (ds:b85d, *ds:b67c[4 + ds:b85e], ds:b85f) bzw. txt[46] bei ds:b85d < 0. f_133b(o, s): Zeilenzahl eines Texts.
 *
 * BCC -mlarge -3 -Ot -Og -Oi -c -Y -E..\bc\BIN\TASM.EXE OVL104.CPP -> 5304/5304 Bytes GLEICH (18.10.2026), cobjfix
 *   laeuft, kein eigenes _DATA. Ohne -Og bzw. -Oi falsche Laenge.
 * - strcat-Intrinsic scannt zuerst die QUELLE (repne scasb auf src, xchg, dann Ziel): "strcat(old, o->f4)", nicht
 *   umgekehrt. memcpy(d, s, strlen(s)) = strlen mit dec cx (ohne Null), strcpy ohne dec.
 * - O269-Objekt (Pal) 0xa6 Bytes: sub sp,0b4h = Objekt + 14 Bytes Skalare (scr, old, w, Zeiger liegen darueber).
 * - f_08e3: register k (SI) nur im if-Block - das strcat-Intrinsic im else-Zweig sperrt SI/DI auf Funktionsebene;
 *   in f_133b (nur strlen/memset) bekommt k auf Funktionsebene SI.
 * - "o->f8[k - 1] = o->f8[k] = 0" -> mov al,0 / zwei Stores ([bx+si-1]). f_0456: c unsigned char (jc/jna),
 *   f_08e3/f_133b signed char (jl/jg, Umlaute als '\x84').
 * - f_0000: Block-Locals mit Initialisierung (buf = new char[100], old = 0, lines, n, len, newch) in dieser Reihen-
 *   folge; new/delete char -> R07_041C/R07_046C; new Bmp -> R42_00D8(0, ..), delete -> R42_03B3(p, 3); in f_0d3d
 *   Bmp-Zeiger und Palettenpuffer in Nachbarbloecken (teilen [bp-0eh]).
 * - "pal.load(g_b666 = 0x332, scr)"; "farcoreleft() > 80000L" (jna); "memcpy(g_b668, scr, 64000U)" -> rep movsw.
 * Auffaelligkeiten (Original): f_133b ueberspringt Steuerzeichen ds:6060..6065 samt Folgebyte, f_08e3 nur
 *   ds:6060..6064 (Zeichen 6065 wird gezaehlt anders als dargestellt). f_0000/f_070c ohne Laengenpruefung der
 *   1000-Byte-Puffer (alte Seite + neuer Eintrag per strcat).
 * (Namensfassung mit schweif.h, class Diary/struct diary: 19.10.2026 GLEICH, LINKGLEICH) Methoden von Diary: f_0000
 *   append, f_0456 mark_names, f_070c mark_word, f_08e3 show_page, f_0d3d draw, f_110c load_page, f_1245 place_str,
 *   f_12a5 date_str, f_133b count_lines; Namensliste Datei 0x334 als struct sel_rec (O225); Pal = class Pic (O269).
 */
#include <stdio.h>
#include <string.h>
#include <mem.h>
#include <alloc.h>
#include "schweif.h"

/* Phase B: eigene Daten des Moduls (ds:60fe..6124, dsana) - initialisierte Daten in Adressreihenfolge, danach der Literalpool */
char g_fmt_diary_c[4] = "%c";
char g_fmt_diary_c2[4] = "%c";
char g_fmt_diary_end[6] = "@@%c";

#define STAR_NAME	(*(char far * far *)g_names_star_idx)	/* Dateiname der Arbeitsdatei 0 */
#define FILE_NAMES	0x334		/* Namensliste (struct sel_rec, Ende id == -1) */
#define PIC_DIARY	0x332		/* Hintergrund des Tagebuchs */
#define DIARY_TEXT_LEN	1000
#define COL_HL_ON	0xf1		/* Hervorhebung an/aus im Text (font_print_wrapped) */
#define COL_HL_OFF	0xf0

signed short Diary::append(char far *s, signed short v)
{
	signed short ret = 0;
	signed short x;
	signed short y;
	signed short w;
	signed short h;

	font_get_box(g_font, &x, &y, &w, &h);
	font_set_box(g_font, 0x11, 0xcc, 0x19, -1);
	if (s != 0) {
		char far *buf = new char[100];
		char far *old = 0;
		signed short lines = font_wrap(g_font, s);
		signed short n = 0;
		signed short len = 0;
		signed short newch = 0;

		memset(text, 0, DIARY_TEXT_LEN);
		if (chapter != (signed char)g_date_day) {
			newch = 1;
			lines++;
		}
		if (n_pages > 1) {
			load_page(n_pages - 1);
			n = count_lines(text);
		}
		if (n != 0 && n + lines < 20) {
			old = new char[1000];
			memcpy(old, text, strlen(text));
		} else {
			newch = 1;
			page = n_pages;
			n_pages++;
		}
		memset(text, 0, DIARY_TEXT_LEN);
		if (newch != 0) {
			chapter = (signed char)g_date_day;
			sprintf(buf, g_fmt_diary_c, g_diary_ctrl[0]);
			strcat(text, buf);
			place_str(buf);
			strcat(text, buf);
			date_str(buf);
			strcat(text, buf);
		}
		sprintf(buf, g_fmt_diary_c2, g_diary_ctrl[v ? 2 : 1]);
		strcat(text, buf);
		strcat(text, s);
		sprintf(buf, g_fmt_diary_end, g_diary_ctrl_end);
		strcat(text, buf);
		mark_names();
		if (old != 0) {
			if (newch != 0) {
				old[strlen(old) - 2] = g_diary_ctrl_end;
				old[strlen(old) - 1] = 0;
			}
			strcat(old, text);
			memcpy(text, old, strlen(old));
			delete old;
		}
		len = strlen(text) + 1;
		diary_encrypt(this, len);
		ret = len + 2;
		delete buf;
	}
	font_set_box(g_font, x, y, w, h);
	return ret;
}

void Diary::mark_names()
{
	signed long size;
	signed short i = 0;
	signed short j = 0;
	signed short k = 0;
	char far *word = new char[0x40];
	signed short len;
	unsigned char c;
	struct sel_rec far *list;

	len = strlen(text);
	c = text[0];
	ofile_open(g_ofile, FILE_NAMES);
	size = ofile_length(g_ofile);
	list = (struct sel_rec far *)mem_alloc(size);
	ofile_read(g_ofile, list, size);
	ofile_close(g_ofile);
	memset(buf, 0, DIARY_TEXT_LEN);
	strcpy(buf, text);
	memset(text, 0, DIARY_TEXT_LEN);
	while (i <= len) {
		if (c == g_diary_ctrl[0]) {
			do {
				text[j] = c;
				j++;
				i++;
				c = buf[i];
			} while (c != g_diary_ctrl_entry && c != g_diary_ctrl_own);
			text[j] = c;
			j++;
			i++;
			c = buf[i];
		}
		if (c >= 'A' && c <= 'Z' || c >= 'a' && c <= 'z' || c >= '0' && c <= '9' ||
		    c == 0x84 || c == 0x94 || c == 0x81 || c == 0x8e || c == 0x99 || c == 0x9a || c == 0xe1) {
			word[k] = c;
			k++;
			i++;
		} else if (word[0] != 0) {
			mark_word(list, text, word, &j);
			k = 0;
		} else {
			text[j] = c;
			j++;
			i++;
		}
		c = buf[i];
	}
	if (word[0] != 0)
		mark_word(list, text, word, &j);
	ofile_create(g_ofile, FILE_NAMES, 0x8002);
	ofile_write(g_ofile, list, size);
	ofile_close(g_ofile);
	mem_free(list);
	delete word;
}

void Diary::mark_word(struct sel_rec far *list, char far *dst, char far *word, signed short far *pj)
{
	signed short len;
	signed short e = 0;
	signed short found = 0;
	char far *a = new char[0x40];
	char far *b = new char[0x40];

	while (list[e].id != -1 && found == 0) {
		strcpy(a, list[e].name);
		strcpy(b, word);
		str_tolower(a);
		str_tolower(b);
		if ((len = strlen(list[e].name)) > 0 && strncmp(a, b, len) == 0) {
			dst[(*pj)++] = list[e].lvl;
			dst[(*pj)++] = list[e].id;
			list[e].avail = found = 1;
		}
		e++;
	}
	len = strlen(word);
	for (e = 0; e < len; e++)
		dst[(*pj)++] = word[e];
	memset(word, 0, 0x40);
	delete b;
	delete a;
}

void Diary::show_page(unsigned short page)
{
	signed char c;
	signed short pos;
	signed short len;
	signed short hl;

	if (load_page(page)) {
		register signed short k;

		k = 0;
		pos = 0;
		len = strlen(text);
		hl = 0;
		font_set_box(g_font, 0x19, 0xce, 0x1e, -1);
		memset(buf, 0, DIARY_TEXT_LEN);
		while (pos < len) {
			c = text[pos];
			pos++;
			if (c == g_diary_ctrl[0]) {
				if (buf[0] != 0) {
					buf[k] = 0;
					k = 0;
					font_set_colors(g_font, 0xfe, -1);
					font_print_wrapped(g_font, buf);
				}
				while (c != g_diary_ctrl_entry && c != g_diary_ctrl_own) {
					c = text[pos];
					pos++;
					buf[k] = c;
					k++;
				}
				buf[k - 1] = buf[k] = 0;
				k = 0;
				font_set_colors(g_font, 0xfd, -1);
				font_print_wrapped(g_font, buf);
				font_set_colors(g_font, 0xbb, -1);
				font_set_y(g_font, font_get_y(g_font) - 2);
				font_print_wrapped(g_font, txt[13]);
				font_set_y(g_font, font_get_y(g_font) + 2);
				pos--;
			} else if (find_tag == -1 && c == g_diary_ctrl_own) {
				buf[k] = COL_HL_ON;
				k++;
				hl = 1;
			} else if (c == g_diary_ctrl_end && hl != 0) {
				buf[k] = COL_HL_OFF;
				k++;
				hl = 0;
			} else if (find_tag != 0 && find_tag == c) {
				c = text[pos];
				pos++;
				if (find_id == c) {
					buf[k] = COL_HL_ON;
					k++;
					c = text[pos];
					pos++;
					while (c >= 'A' && c <= 'Z' || c >= 'a' && c <= 'z' || c == '\x84' || c == '\x94' ||
					       c == '\x81' || c == '\xe1' || c == '\x8e' || c == '\x99' || c == '\x9a') {
						buf[k] = c;
						k++;
						c = text[pos];
						pos++;
					}
					buf[k] = COL_HL_OFF;
					k++;
					pos--;
				}
			} else if (c >= g_diary_tag[0] && c <= g_diary_tag4) {
				pos++;
			} else if (c != g_diary_ctrl_entry && c != g_diary_ctrl_own && c != g_diary_ctrl_end) {
				buf[k] = c;
				k++;
			}
		}
		buf[k] = 0;
		font_set_colors(g_font, 0xfe, -1);
		font_print_wrapped(g_font, buf);
	} else {
		font_set_colors(g_font, 0xfd, -1);
		font_set_box(g_font, 0x19, 0xce, 0x1e, -1);
		place_str(g_text_output_buf);
		sprintf(text, "%s", g_text_output_buf);
		date_str(g_text_output_buf);
		strcat(text, g_text_output_buf);
		font_print_wrapped(g_font, text);
		font_set_colors(g_font, 0xbb, -1);
		font_set_y(g_font, 0x24);
		font_print_wrapped(g_font, txt[13]);
	}
}

void Diary::draw(signed short mode, void far *dst)
{
	Pic pal;
	void far *scr;
	void far *old;
	signed short w;

	if (dst == 0) {
		if (farcoreleft() > 80000L)
			scr = mem_alloc(64000L);
		else
			scr = g_vga_buf;
	} else {
		scr = dst;
	}
	old = font_set_screen(g_font, scr);
	if (mode == 1) {
		STAR_NAME = "DIARY.DAT";
		if (ofile_create(g_ofile, 0, 0x8001) != -1) {
			ofile_read(g_ofile, this, 2L);
			ofile_read(g_ofile, &chapter, 1L);
			ofile_close(g_ofile);
		}
		STAR_NAME = 0;
		if (n_pages != 0)
			page = n_pages - 1;
		else
			page = 0;
	}
	if (scr == g_vga_buf)
		mouse_hide(0);
	pal.load(g_pp20_index = PIC_DIARY, scr);
	if (mode == 2) {
		Bmp far *b = new Bmp(0x18, 0x18);
		read_record(b->data(), 8);
		b->put(scr, 0x121, 0x0e);
		b->clear();
		b->put(scr, 0x121, 0x2d);
		b->put(scr, 0x121, 0x4c);
		b->put(scr, 0x121, 0x6b);
		b->put(scr, 0x121, 0x8a);
		b->put(scr, 0x121, 0xa9);
		delete b;
	}
	if (scr == g_vga_buf)
		mouse_show(0);
	show_page(page);
	font_set_colors(g_font, 0xfe, -1);
	if (mode == 2)
		sprintf(g_text_output_buf, txt[45]);
	else
		sprintf(g_text_output_buf, txt[23], page + 1, n_pages);
	w = font_text_width(g_font, g_text_output_buf);
	font_set_box(g_font, 0x82 - (w >> 1), w + 1, 0x10, -1);
	font_print_wrapped(g_font, g_text_output_buf);
	font_set_screen(g_font, old);
	if (scr != g_vga_buf) {
		mouse_hide(0);
		memcpy(g_vga_buf, scr, 64000U);
		mouse_show(0);
	}
	if (mode == 1) {
		void far *p = mem_alloc(0x300L);
		memcpy(p, pal.get_palette(), 0x300);
		pal_fade_in(p);
		mem_free(p);
	}
	if (dst == 0 && scr != g_vga_buf)
		mem_free(scr);
}

signed short Diary::load_page(unsigned short page)
{
	signed short ok = 0;
	unsigned short cnt = 0;
	signed long len;
	register unsigned short i;

	memset(text, 0, DIARY_TEXT_LEN);
	STAR_NAME = "DIARY.DAT";
	if (ofile_create(g_ofile, 0, 0x8001) != -1) {
		ofile_read(g_ofile, &cnt, 2L);
		if (cnt != 0 && page < cnt) {
			ofile_read(g_ofile, &chapter, 1L);
			for (i = 0; i < page; i++) {
				ofile_read(g_ofile, &len, 4L);
				ofile_seek(g_ofile, len, 1);
			}
			ofile_read(g_ofile, &len, 4L);
			ofile_read(g_ofile, text, len);
			diary_decrypt(this, (signed short)len - 2);
			ok = 1;
		}
		ofile_close(g_ofile);
	}
	STAR_NAME = 0;
	if (n_pages < cnt)
		n_pages = cnt;
	return ok;
}

void Diary::place_str(char far *buf)
{
	sprintf(buf, txt[10], g_group->town ? g_ttx[g_group->town + 0x163] : txt[11]);
}

void Diary::date_str(char far *buf)
{
	if ((signed char)g_date_day < 0)
		sprintf(buf, txt[46], (signed char)g_date_day + 6, (signed char)g_date_year);
	else
		sprintf(buf, n_pages > 1 ? txt[9] : txt[8], (signed char)g_date_day, g_ttx[(signed char)g_date_month + 4], (signed char)g_date_year);
}

signed short Diary::count_lines(char far *s)
{
	signed char c;
	signed short pos;
	signed short len;
	signed short n;
	char far *buf;
	register signed short k;

	k = 0;
	pos = 0;
	len = strlen(s);
	n = 0;
	buf = new char[1000];
	font_set_box(g_font, 0x19, 0xce, 0x1e, -1);
	memset(buf, 0, DIARY_TEXT_LEN);
	while (pos < len) {
		c = s[pos];
		pos++;
		if (c == g_diary_ctrl[0]) {
			if (buf[0] != 0) {
				buf[k] = 0;
				k = 0;
				n += font_wrap(g_font, buf);
			}
			while (c != g_diary_ctrl_entry && c != g_diary_ctrl_own) {
				c = s[pos];
				pos++;
				buf[k] = c;
				k++;
			}
			buf[k - 1] = buf[k] = 0;
			k = 0;
			n += font_wrap(g_font, buf);
			n++;
			pos--;
#if FIX_0144
		/* Fix #144: Seitenzaehlung mit denselben Steuerzeichen wie die Anzeige (0xa4..0xa8) */
		} else if (c >= g_diary_tag[0] && c <= g_diary_tag4) {
#else
		} else if (c >= g_diary_tag[0] && c <= g_diary_tag5) {
#endif
			pos++;
		} else if (c != g_diary_ctrl_entry && c != g_diary_ctrl_own && c != g_diary_ctrl_end) {
			buf[k] = c;
			k++;
		}
	}
	buf[k] = 0;
	n += font_wrap(g_font, buf) - 1;
	delete buf;
	return n;
}
