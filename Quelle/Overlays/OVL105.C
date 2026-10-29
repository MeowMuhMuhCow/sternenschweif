/* OVL105 - Overlay O105 (SCHWEIF.EXE): Drucken des Tagebuchs (Drucker per biosprint oder Datei ds:35c6 bei
 * ds:253c, Zeilenbreite ds:254c, Seitenlaenge ds:254d) und Heldenliste der Datei 0x334 (Eintraege 0x17 Bytes:
 * Ende 0xff, Merker +1, Kennung +2, Name +3). f_0000(o, n): Puffer o->f4 verschluesseln (Schluessel 0x13, Summe 9,
 * Pruefwort -Summe hinter den Daten). f_005b(o, n): entschluesseln. f_00af/f_011c(o): Puffer (2 x 1000 Bytes,
 * R07_0002) und Texte (O248_02E3(ds:37e4, 0x331, 0x32)) mit Referenzzaehler +25 anlegen/freigeben (R07_0140).
 * f_01aa(o): Liste der Kapitel 0..4 (Kennung 0xa4 + i mit Merker) plus 5 und 6 nach o->f1d, Rueckgabe Anzahl.
 * f_02b2(): Namen aller Helden (O088_01DD/09FF/06C4) in die Datei eintragen (gleicher Name -> ersetzen, sonst
 * erster freier Platz mit Merker; Datei ds:b635, O243_01DD/0554/04A0/051B, neu schreiben O243_0291/04BF).
 * f_059b(o, von, bis): Seiten drucken (O104_110C laedt Seite, Statustext *ds:b556, Seitenvorschub 0x0c).
 * f_066e(o): Seite setzen (Kopf txt[0x21] per sprintf mit o->f2 + 1 und o->f0, Steuerzeichen ds:605c..6064
 * trennen Absaetze/Ueberschriften), mit Leerzeilen bis zum naechsten Drittel der Seitenlaenge auffuellen. f_08c3(o, s): zentrieren.
 * f_0980(o): Trennzeile txt[0x22]. f_09be(o, s): Zeilenumbruch (CR/LF -> Leerzeichen, '\r' am letzten
 * Leerzeichen, '@' = harter Umbruch). f_0ad4(o, s, mitte): Text zeilenweise ausgeben ('^' = Leerzeichen).
 * f_0c04(o, s): eine Zeile ausgeben (Datei O037_04D6 bzw. biosprint + R01_036B), Zeilenzaehler +18.
 *
 * BCC -mlarge -3 -Ot -Og -Oi -c -Y OVL105.C   -> 3265/3265 Bytes GLEICH (18.10.2026), cobjfix laeuft
 * - -Oi: strlen/strncmp/strcpy/strcat/memset inline. Eine register-Variable auf Funktionsebene bekommt dann
 *   nie DI (DI vom Intrinsic belegt) - in f_0ad4 steht j deshalb im inneren Block der Schleife (dort DI).
 * - Registerwahl ohne Aufrufe: f_0000 sum ohne register (SI), i register (DX); f_09be i/col/sp alle register
 *   (CX/DI/SI); f_01aa n register (SI), j ohne register (DI); f_005b "d = (c = *p) - prev" (c im Speicher, d in CL).
 * - f_01aa: "} while (buf[++j].f0 != 0xff && !found);" - mit j++ im Rumpf legt -Og &buf[j] als Zwischenwert ab.
 * - f_02b2: alle Locals ausser h im Block "if (h)" - davor liegen die -Og-Zwischenwerte [bp-6]/[bp-8]/[bp-0ah]
 *   (Offsets von h->name, &buf[i], buf[i].name); "ml = len = strlen(..)".
 * - f_066e: "g_254d / 3" zweimal statt Variable (-Og-Zwischenwert [bp-8] vor dem les).
 * - f_09be: "p = &p[i + 1]" (i + 1 + Offset, ohne -Og-Teilung mit p[i]), "p = p + sp".
 * - f_0ad4: c im Schleifenblock (hinter dem -Og-Zwischenwert), Speichern als "g_b674[j] = s[i]" (Wert vor dem les).
 * - ds:613b hat kein Public: &g_612e[13].
 * Originalfehler: f_08c3 haengt mit strcat an einen nicht abgeschlossenen new-Puffer an (nur pad Leerzeichen
 * gesetzt, kein '\0'; bei pad <= 0 ganz uninitialisiert); pad wird vorzeichenlos gerechnet (Zeile laenger als
 * ds:254c -> riesiges memset). f_02b2 prueft buf[i].name[0] vor i < 20 (liest Eintrag 20). f_09be: st bleibt
 * immer 0. f_005b: cnt unbenutzt.
 * (Namensfassung mit schweif.h, struct diary: 19.10.2026 GLEICH, LINKGLEICH) f_0000 diary_encrypt, f_005b
 *   diary_decrypt, f_00af diary_open, f_011c diary_close, f_01aa diary_chapters, f_02b2 diary_hero_names (jetzt mit
 *   dem unbenutzten Tagebuch-Argument wie beim Aufruf aus O103), f_059b diary_print_pages, f_066e diary_print_page,
 *   f_08c3 diary_print_centered, f_0980 diary_print_rule, f_09be diary_wrap, f_0ad4 diary_print_text, f_0c04
 *   diary_print_line. Namensliste Datei 0x334 als struct sel_rec.
 */
#include <mem.h>
#include <string.h>
#include <stdio.h>
#include <bios.h>
#include "schweif.h"

/* Phase B: eigene Daten des Moduls (ds:6124..613e, dsana) - initialisierte Daten in Adressreihenfolge */
char g_str_diary_dat5[10] = "DIARY.DAT";
char g_str_diary_txt2[16] = "..\\DIARY.TXT\0\r\n";	/* Dateiname, dahinter STR_CRLF */

#define STAR_NAME	(*(char far * far *)g_names_star_idx)	/* Dateiname der Arbeitsdatei 0 */
#define FILE_NAMES	0x334		/* Namensliste (struct sel_rec, Ende id == -1) */
#define TXT_DIARY	0x331		/* Texte des Tagebuchs (0x32 Eintraege) */
#define DIARY_TEXT_LEN	1000
#define STR_CRLF	(g_str_diary_txt2 + 13)	/* ds:613b "\r\n" (kein eigenes Public) */

void diary_encrypt(struct diary far *o, register signed short n)
{
	unsigned char far *p;
	unsigned char acc;
	signed short sum;
	unsigned char k;
	register signed short i;

	p = (unsigned char far *)o->text;
	sum = 0;
	k = 0x13;
	acc = 9;
	for (i = 0; i < n; i++) {
		k ^= *p;
		acc += k;
		*p = acc;
		p++;
		if ((i & 3) == 3)
			sum += k;
	}
	*(signed short far *)p = -sum;
}

void diary_decrypt(struct diary far *o, register signed short n)
{
	unsigned char far *p;
	signed short cnt;
	unsigned char prev;
	unsigned char key;
	unsigned char c;
	unsigned char d;
	signed short i;

	p = (unsigned char far *)o->text;
	cnt = 0;
	prev = 9;
	key = 0x13;
	for (i = 0; i < n; i++) {
		d = (c = *p) - prev;
		prev = c;
		*p = d ^ key;
		p++;
		key = d;
	}
}

void diary_open(struct diary far *o)
{
	if (o->refs == 0) {
		o->text = mem_alloc(1000);
		o->buf = mem_alloc(1000);
		txt_load_list(g_font, TXT_DIARY, 0x32, &o->txt_buf, &o->txt);
	}
	o->refs++;
}

void diary_close(struct diary far *o)
{
	if (o->refs == 1) {
		o->txt_buf = mem_free(o->txt_buf);
		o->txt = mem_free(o->txt);
		o->text = mem_free(o->text);
		o->buf = mem_free(o->buf);
	}
	o->refs--;
}

signed short diary_chapters(struct diary far *o)
{
	signed short i;
	signed char c;
	signed long size;
	signed short found;
	struct sel_rec far *buf;
	register signed short n;
	signed short j;

	n = 0;
	found = 0;
	ofile_open(g_ofile, FILE_NAMES);
	size = ofile_length(g_ofile);
	buf = mem_alloc(size);
	ofile_read(g_ofile, buf, size);
	ofile_close(g_ofile);
	for (i = 0; i <= 4; i++) {
		found = j = 0;
		c = i + 0xa4;
		do {
			if (buf[j].lvl == c && buf[j].avail) {
				o->chapters[n] = i;
				n++;
				found = 1;
			}
		} while (buf[++j].id != -1 && !found);
	}
	o->chapters[n] = 5;
	n++;
	o->chapters[n] = 6;
	n++;
	mem_free(buf);
	return n;
}

void diary_hero_names(struct diary far *o)
{
	struct hero far *h;

	h = get_first_hero(g_group);
	if (h) {
		signed short i;
		signed short idx;
		signed short ml;
		signed short l2;
		signed short len;
		signed long size;
		signed short neu;
		struct sel_rec far *buf;

		ofile_open(g_ofile, FILE_NAMES);
		size = ofile_length(g_ofile);
		buf = mem_alloc(size);
		ofile_read(g_ofile, buf, size);
		ofile_close(g_ofile);
		do {
			neu = 0;
			idx = -1;
			ml = len = strlen(h->name);
			for (i = 0; i < 20; i++) {
				l2 = strlen(buf[i].name);
				if (l2 && l2 < len)
					ml = l2;
				if (!strncmp(h->name, buf[i].name, ml)) {
					if (ml == len)
						idx = i;
					else
						neu = 1;
					break;
				} else if (ml == l2 && !strncmp(h->name, buf[i].name, ml)) {
					idx = i;
					break;
				}
			}
			if (idx != -1) {
				strcpy(buf[idx].name, h->name);
			} else if (!neu) {
#if FIX_0057
				/* Fix #57: Grenze vor dem Zugriff pruefen */
				for (i = 0; i < 20 && buf[i].name[0]; i++)
#else
				for (i = 0; buf[i].name[0] && i < 20; i++)
#endif
					;
				if (i != 20) {
					strcpy(buf[i].name, h->name);
					buf[i].avail = 1;
				}
			}
			h = get_next_hero(g_group, h);
		} while (h != group_get_npc(g_group) && h != get_first_hero(g_group));
		ofile_create(g_ofile, FILE_NAMES, 0x8002);
		ofile_write(g_ofile, buf, size);
		ofile_close(g_ofile);
		mem_free(buf);
	}
}

void diary_print_pages(struct diary far *o, unsigned short from, register unsigned short to)
{
	unsigned short save;

	save = o->page;
	mouse_set_cursor(5, 1);
	for (o->page = from; o->page <= to; o->page++) {
		STAR_NAME = g_str_diary_dat5;
		if (diary_load_page(o, o->page)) {
			STAR_NAME = g_str_diary_txt2;
			diary_print_page(o);
		}
	}
	if (!g_print_to_file && g_print_page_len > o->lines)
		biosprint(0, 0x0c, 0);
	STAR_NAME = 0;
	o->page = save;
	diary_load_page(o, o->page);
	mouse_reset();
}

void diary_print_page(struct diary far *o)
{
	signed char c;
	signed short pos;
	signed short len;
	register signed short n;

	n = 0;
	pos = 0;
	len = strlen(o->text);
	o->lines = 0;
	memset(o->buf, 0, 1000);
	sprintf(g_text_output_buf, o->txt[0x21], o->page + 1, o->n_pages);
	diary_print_text(o, g_text_output_buf, 1);
	diary_print_rule(o);
	while (pos <= len) {
		c = o->text[pos];
		pos++;
		if (c == g_diary_ctrl[0]) {
			if (o->buf[0]) {
				o->buf[n] = 0;
				n = 0;
				diary_print_text(o, o->buf, 0);
				diary_print_line(o, 0);
			}
			while (c != g_diary_ctrl_entry && c != g_diary_ctrl_own) {
				c = o->text[pos];
				pos++;
				o->buf[n] = c;
				n++;
			}
			o->buf[n - 1] = o->buf[n] = 0;
			n = 0;
			diary_print_line(o, 0);
			diary_print_text(o, o->buf, 0);
			diary_print_line(o, 0);
		} else if (c >= g_diary_tag[0] && c <= g_diary_tag4) {
			pos++;
		} else if (c != g_diary_ctrl_entry && c != g_diary_ctrl_own && c != g_diary_ctrl_end) {
			o->buf[n] = c;
			n++;
		}
	}
	o->buf[n] = 0;
	diary_print_text(o, o->buf, 0);
	while (g_print_page_len / 3 < o->lines)
		o->lines -= g_print_page_len / 3;
	while (g_print_page_len / 3 > o->lines)
		diary_print_line(o, 0);
}

void diary_print_centered(struct diary far *o, char far *s)
{
	signed short pad;
	char far *tmp;

#if FIX_0056
	/* Fix #56: pad mit Vorzeichen, zu breite Zeilen unveraendert lassen */
	pad = ((signed short)g_print_width - (signed short)strlen(s)) / 2;
	if (pad > 0) {
		tmp = mem_alloc_w(g_print_width + 1);
		memset(tmp, ' ', pad);
		strcat(tmp, s);
		strcpy(s, tmp);
		mem_free_v(tmp);
	}
#else
	pad = (g_print_width - strlen(s)) / 2;
	tmp = mem_alloc_w(g_print_width + 1);
	if (pad > 0)
		memset(tmp, ' ', pad);
	strcat(tmp, s);
	strcpy(s, tmp);
	mem_free_v(tmp);
#endif
}

void diary_print_rule(struct diary far *o)
{
	sprintf(g_text_output_buf, o->txt[0x22]);
	diary_print_text(o, g_text_output_buf, 1);
}

void diary_wrap(struct diary far *o, char far *s)
{
	char far *p;
	signed short st;
	register signed short i;
	register signed short col;
	register signed short sp;

	i = 0;
	sp = 0;
	st = 0;
	col = 0;
	if (s) {
		for (p = s; *p; p++)
			if (*p == '\r' || *p == '\n')
				*p = ' ';
		p = s;
		while (p[i]) {
			col++;
			if (col >= g_print_width) {
				if (sp != st) {
					p[sp] = '\r';
					p = p + sp;
				} else {
					p[i] = '\r';
					p = &p[i + 1];
				}
				i = sp = st = col = 0;
			}
			if (p[i] == ' ')
				sp = i;
			if (p[i] == '@') {
				p = &p[i + 1];
				i = -1;
				sp = st = col = 0;
			}
			i++;
		}
		if (col >= g_print_width) {
			if (sp == st) {
				p[i - 1] = 0;
				return;
			}
			p[sp] = '\r';
		}
	}
}

void diary_print_text(struct diary far *o, char far *s, signed short center)
{
	signed short len;
	register signed short i;

	i = 0;
	len = strlen(s);
	diary_wrap(o, s);
	while (i < len) {
		signed char c;
		register signed short j;

		j = 0;
		c = s[i];
		while (c != '@' && c != '\r' && i < len) {
			if (c == '^') {
				g_text_buf2[j] = ' ';
				j++;
			} else {
				g_text_buf2[j] = s[i];
				j++;
			}
			i++;
			c = s[i];
		}
		g_text_buf2[j] = 0;
		if (center)
			diary_print_centered(o, g_text_buf2);
		diary_print_line(o, g_text_buf2);
		if (c == '@' || c == '\r') {
			i++;
			c = s[i];
		}
		while (c == '@' || c == '\r') {
			diary_print_line(o, 0);
			i++;
			c = s[i];
		}
		while (c == ' ' || c == '^') {
			i++;
			c = s[i];
		}
	}
}

void diary_print_line(struct diary far *o, char far *s)
{
	char far *nl;

	if (g_print_to_file) {
		nl = STR_CRLF;
		if (s)
			File_write(g_file_main, s, strlen(s));
		File_write(g_file_main, nl, strlen(nl));
	} else {
		if (s)
			while (*s) {
				biosprint(0, *s++, 0);
				game_idle();
			}
		biosprint(0, '\r', 0);
		biosprint(0, '\n', 0);
	}
	o->lines++;
}
