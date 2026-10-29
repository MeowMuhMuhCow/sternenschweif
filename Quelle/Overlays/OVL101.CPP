/* OVL101 - Overlay O101 (Segment 0x2148 in SCHWEIF.EXE): Dialogsystem (entspricht DSA1 do_talk/load_tlk).
 * f_0000(d, id, partner, mode): Dialog id (Datei) mit Gespraechspartner partner fuehren - Zustandstabelle
 * laden (f_049c), Bild (id 0x3d5: Bild 5 nach ds:c4fb, id 0x2e4: Bild 0x2e5 in ein neues Bmp 0x50x0x50,
 * sonst O034_1A89), dann Schleife ueber Zustaende {Text (Bit 15 = Sonderfunktion O102_0000), 3 Antworten,
 * 3 Folgezustaende}: Text aufbereiten (f_0658), Auswahl ueber O190_0237 (mode 0) bzw. O190_037F mit Bild,
 * Folgezustand nach Antwort (-1 = Abbruch bei d->esc_ends), 0xff = Ende; danach Puffer freigeben.
 * f_049c(d, idx): Dialogdatei ueber das Dateiobjekt ds:35c6 (O037) lesen: Kopf (12 Bytes) und Zustandsliste
 * des Partners idx, Textblock und Zeigertabelle (O248_0245 ueber ds:37e4). f_0658(d): Text nach d->buf
 * kopieren und je Dialog/Partner/Zustand Platzhalter per sprintf fuellen (Name O013_049D, Heldenname
 * O088_016B/O088_0226, ds:281b/282b, ds:b674, Preis ds:260c * 100, ds:2620 (bei misslungener Probe
 * O076_0A65(h, 0x16, 0x0c) halbiert + 1), ds:261f).
 *
 * BCC -mlarge -3 -Ot -Og -Oi -c -Y OVL101.CPP   -> 2718/2718 Bytes GLEICH (18.10.2026; -Ov statt -Og ebenso),
 * cobjfix laeuft
 * - C++ nur wegen inline hname(h) { return h->name; }: jede Parameterkopie ist ein eigener Local, die Kopien
 *   liegen in Textreihenfolge ab [bp-4]; im Fall 0x2bc wird die zweite Kopie (O088_0226, letztes Argument)
 *   zuerst belegt. Mit eingebetteter Zuweisung (h = f())->name nimmt BCC dx:ax direkt.
 * - name[16] je in einem eigenen Block der Faelle 0x0d/0x0e (beide [bp-24h]). Ein gemeinsamer Block um den
 *   ganzen 0x90-Zweig zieht die inline-Kopien darin hinter die Kopien ausserhalb (Reihenfolge falsch).
 * - -Oi fuer strcpy (rep movsw/adc/rep movsb), -Og/-Ov fuer die Epilogkopien am Ende jedes else-if-Zweigs.
 * - "(signed long)(signed short)g_price_level * 100" -> mov al/cbw/movsx eax,ax/imul; ohne (signed short):
 *   movsx eax,byte. "g_street_thugs = (g_street_thugs >> 1) + 1" -> cbw/sar/inc al.
 * - "d->state = d->done = 0" (rechts zuerst), "ans = n = 0" (n = DI zuerst); Folgezustand per ?:.
 * - f_049c: Groesse n << 3 in unsigned register -> movzx eax; Zeigertabelle (unsigned short)(s2 << 2).
 * - Die beiden leeren Titel ds:605a/605b sind zwei getrennte ""-Literale (BCC legt gleiche Literale nicht
 *   zusammen).
 * - f_049c/f_0658 stehen unten -> far call aus f_0000.
 * (Namensfassung mit schweif.h, struct talk_desc in s_struct.h: 19.10.2026 GLEICH, LINKGLEICH) f_0000 talk,
 *   f_049c talk_load, f_0658 talk_format; hname(h) -> hero::get_name() (Parameterkopie wie bisher).
 *   Die leeren Titel ds:605a/605b haben kein eigenes Public (liegen hinter g_str_schweif_cat).
 */
#include <string.h>
#include <stdio.h>
#include "schweif.h"

#define DLG		((Dlg far *)g_dialog)
#define FILE_MAIN	((File far *)g_file_main)
#define STR_EMPTY_TITLE1	""	/* ds:605a (eigenes Literal, Phase B) */
#define STR_EMPTY_TITLE2	""	/* ds:605b (eigenes Literal, Phase B) */
#define TALK_END	0xff		/* Folgezustand: Ende */
#define TALK_SPECIAL	0x8000		/* talk_state.txt: Sonderfunktion */
#define TLK_LOW48	0x90		/* LOW48.TLK (Platzhalter in talk_format) */

void talk(struct talk_desc far *d, signed short id, signed short partner, signed short mode)
{
	signed short old;
	signed short ans;
	signed short t;
	signed short dummy;
	struct talk_state far *st;
	struct talk_state far *states;
	void far *b;
	register signed short n;

	old = dlg_set16(g_dialog, 7);
	ans = 0;
	b = 0;
	d->id = id;
	d->partner = partner;
	talk_load(d, partner);
	d->state = d->done = 0;
	d->buf = (char far *)mem_alloc(3000L);
	states = d->states;
	if (id == 0x3d5) {
		nvfile_load(g_nvf_heads, 5, g_portrait_buf, 0, d->pic, 0, &dummy, &dummy, SCREEN_W);
	} else if (id == 0x2e4) {
		b = gb_new_alloc(0, 0x50, 0x50);
		nvfile_load(g_nvf_heads, 0x2e5, gb_data(b), 0, d->pic, 0, &dummy, &dummy, SCREEN_W);
	} else {
		load_portrait_buf(d->pic);
	}
	do {
		ans = n = 0;
		st = states + d->state;
		if (st->ans[0])
			n++;
		if (st->ans[1])
			n++;
		if (st->ans[2])
			n++;
		if (st->txt != -1) {
			t = st->txt & 0x7fff;
			d->src = d->txt[t + d->base];
			talk_format(d);
			if (mode == 0) {
				ans = dlg_box(g_dialog, g_portrait_buf, d->title >= 0 ? d->txt[d->title] : STR_EMPTY_TITLE1, d->buf, n,
					d->txt[st->ans[0] + d->base], d->txt[st->ans[1] + d->base], d->txt[st->ans[2] + d->base]);
			} else {
				g_dlg_box_flag = 0;
				ans = dlg_box_bmp(g_dialog, b, d->title >= 0 ? d->txt[d->title] : STR_EMPTY_TITLE2, d->buf, n,
					d->txt[st->ans[0] + d->base], d->txt[st->ans[1] + d->base], d->txt[st->ans[2] + d->base]);
			}
		}
		d->next = -1;
		if ((st->txt & TALK_SPECIAL) || st->txt == -1)
			talk_special(d);
		d->state = d->next == -1 ? st->next[0] : d->next;
		if (d->done == 0) {
			if (n) {
				if (ans == -1) {
					if (d->esc_ends)
						d->done = 1;
					else
						d->state = st->next[0];
				} else if (ans == 1) {
					d->state = st->next[0];
				} else if (ans == 2) {
					d->state = st->next[1];
				} else if (ans == 3) {
					d->state = st->next[2];
				}
			}
			if (d->state == TALK_END)
				d->done = 1;
		}
	} while (d->done == 0);
	if (b)
		gb_delete(b, 3);
	mem_free(d->buf);
	mem_free(d->states);
	mem_free(d->data);
	mem_free(d->txt);
	dlg_set16(g_dialog, old);
}

void talk_load(struct talk_desc far *d, signed short idx)
{
	signed short cnt;
	signed short n;
	signed short s1;
	signed short s2;
	struct talk_hdr h;
	register signed short i;
	register unsigned short sz;

	file_open(g_file_main, d->id);
	file_read(g_file_main, &cnt, 2L);
	for (i = 0; i < cnt; i++) {
		if (i == idx)
			file_read(g_file_main, &h, 12L);
		else
			file_seek(g_file_main, 12L, 1);
	}
	d->base = h.base;
	d->pic = h.pic;
	d->esc_ends = h.esc_ends;
	d->title = h.title;
	for (i = 0; i < cnt; i++) {
		file_read(g_file_main, &n, 2L);
		sz = n << 3;
		if (i == idx) {
			d->states = (struct talk_state far *)mem_alloc((signed long)sz);
			file_read(g_file_main, d->states, (signed long)sz);
		} else {
			file_seek(g_file_main, (signed long)sz, 1);
		}
	}
	file_read(g_file_main, &s1, 2L);
	file_read(g_file_main, &s2, 2L);
	d->data = (char far *)mem_alloc((signed long)s1);
	d->txt = (char far * far *)mem_alloc((signed long)(unsigned short)(s2 << 2));
	file_read(g_file_main, d->data, (signed long)s1);
	file_close(g_file_main);
	txt_split(g_font, d->txt, d->data, (signed long)s1);
}

void talk_format(struct talk_desc far *d)
{
	strcpy(d->buf, d->src);
	if (d->id == TLK_LOW48) {
		if (d->partner == 1) {
			if (d->state == 0x0d) {
				char name[16];

				talk_get_dwarf_name(d, name);
				sprintf(d->buf, d->src, name);
			} else if (d->state == 0x0e) {
				char name[16];

				talk_get_dwarf_name(d, name);
				sprintf(d->buf, d->src, name, group_count_alive(g_group));
			} else if (d->state == 0x17) {
				sprintf(d->buf, d->src, g_absent_hero1, g_absent_hero2);
			} else if (d->state == 0x18) {
				sprintf(d->buf, d->src, get_first_active_hero(g_group)->get_name());
			}
		} else if (d->partner == 2) {
			if (d->state == 0 || d->state == 3)
				sprintf(d->buf, d->src, get_first_active_hero(g_group)->get_name());
		} else if (d->partner == 3) {
			if (d->state == 2)
				sprintf(d->buf, d->src, get_first_active_hero(g_group)->get_name());
		}
	} else if (d->id == 0x2b1) {
		if (d->partner == 0 && d->state == 7)
			sprintf(d->buf, d->src, g_text_buf2);
	} else if (d->id == 0x2bc) {
		if (d->partner == 0 && d->state == 3)
			sprintf(d->buf, d->src, get_first_active_hero(g_group)->get_name(), get_second_active_hero(g_group)->get_name());
	} else if (d->id == 0x2bd) {
		if (d->partner == 0 && (d->state == 0x0c || d->state == 0x0e || d->state == 0x0f || d->state == 0x11))
			sprintf(d->buf, d->src, (signed long)(signed short)g_price_level * 100);
		if (d->partner == 1 && (d->state == 4 || d->state == 7))
			sprintf(d->buf, d->src, (signed long)(signed short)g_price_level * 100);
	} else if (d->id == 0x2e4) {
		if (d->partner == 0 && d->state == 1) {
			if (test_skill(get_first_active_hero(g_group), 0x16, 0x0c) <= 0)
				g_street_thugs = (g_street_thugs >> 1) + 1;
			sprintf(d->buf, d->src, g_street_thugs);
		}
		if (d->partner == 1 && d->state == 0)
			sprintf(d->buf, d->src, g_street_guards);
	}
}
