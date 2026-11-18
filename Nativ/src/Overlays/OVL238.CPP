/* OVL238 - Overlay O238 (Segment 0x2436 in SCHWEIF.EXE): Gegenstandsablage Stash (Klasse 0x16 Bytes, s_class.h;
 * Ablagen/Verstecke an Orten, benutzt von O000/O062/O065/O262..O264). Eintraege struct stash_entry {id, n} (4 Bytes).
 * Konstruktor Stash(nr, quiet): Datensatz nr aus Datei 0x2c6 (Arbeitsdatei g_ofile, O243: Laengenwort + Daten)
 * laden, cap = Laenge/4, Texte 0x2c7 ueber das Schriftobjekt g_font (txt_load_list) nach txt_buf/txt, bei
 * quiet == 0 gleich show(). Destruktor: Datensatz in die Datei zurueckschreiben (ofile_create O_RDWR | O_BINARY),
 * Puffer freigeben. set/is/add(i, id, n): Eintrag setzen, vergleichen, einfuegen (gleiche id -> Anzahl addieren,
 * sonst ab i nach hinten schieben). update(): count = Anzahl Eintraege mit id > 0, top = 0; has_items()/
 * count_items()/show(). fill(s): bis zu 10 Menuezeilen zu 40 Bytes ("<Anzahl> <Name>" ueber itoa/strcat und
 * grammar_plural mit g_item_names[id]), bei mehr als 10 Eintraegen Zeile "weiter" (txt[3]). menu(): Auswahl-
 * schleife (dlg_choose), "weiter" blaettert um 9, Anzahl erfragen (dlg_input, Breite 1-3 Stellen), ITEM_MONEY =
 * Geld (group_add_money(g_group, n * 100)), sonst group_give_item; Eintrag entfernen/verringern; Rueckgabe 1,
 * wenn etwas genommen wurde.
 *
 * BCC -mlarge -3 -Ot -Oi -Og -c -Y -E..\bc\BIN\TASM.EXE OVL238.CPP -> 2492/2492 Bytes GLEICH (18.10.2026),
 * cobjfix laeuft
 *   (Namensfassung mit schweif.h, Klasse Stash in s_class.h: 19.10.2026 GLEICH, LINKGLEICH)
 * STUFE D1 (wovd1, 19.10.2026): asm-strcat durch echtes strcat(.., " ") ersetzt - Code GLEICH, das Literal im
 *   eigenen _DATA bildet cobjfix_d.py auf ds:75c6 ab (Bytes geprueft).
 * - C++: Konstruktor (if (this==0) this = new(0x16), toter Epilog), Destruktor (flags in SI, &1 -> delete;
 *   Schleifenvariable i ohne register -> DI), Methoden mit this als erstem Parameter (O263 ruft
 *   O238_0000/0208/011D so auf).
 * - -Og noetig: Epilogkopien bei "if (..) return 1; else return 0;" (is/has_items) und "if (!update()) {..
 *   return 0; } else return menu();"; update(): "if (count) return 1; return 0;".
 * - fill(): [bp-8] ist KEINE Variable, sondern der -Og-Zwischenwert fuer i + top (Bedingung oben, Index im
 *   Rumpf, Index unten, Schleifenbedingung "i + top < count" laedt ihn zurueck).
 * - Die anderen strcat/strcpy/memset sind -Oi-Intrinsics. strcpy-Ziel als s + (more - 1) * 40 (ergibt
 *   + more*40 - 40).
 * - add(): "return;" im Zweig erzeugt die Epilogkopie vor dem letzten set(); set() steht oben -> push cs/call.
 * - menu(): w als register-Variable im inneren Block (DI); auf Funktionsebene landet sie auf dem Stack.
 *   items[k] = items[k + 1] als Strukturkopie (mov eax); items[k].n -= take (sub al).
 * - Arbeitsdatei ueber die freien Funktionen ofile_* (gleiche Bytes wie die Methodenaufrufe am Objekt).
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mem.h>
#include <fcntl.h>
#include "schweif.h"

/* Phase B: eigene Daten des Moduls (ds:75c6..75c8, dsana) - initialisierte Daten in Adressreihenfolge, danach der Literalpool */

#define FILE_STASHES	0x2c6		/* Arbeitsdatei mit den Ablagen */
#define TXT_STASH	0x2c7		/* Texte */
#define LINE_LEN	40		/* Menuezeile */
#define MAX_LINES	10

Stash::Stash(signed short n, signed short quiet)
{
	signed short len;
	register signed short i;

	nr = n;
	ofile_open(g_ofile, FILE_STASHES);
	for (i = 0; i < nr; i++) {		/* Saetze davor ueberspringen */
		ofile_read(g_ofile, &len, 2L);
		ofile_seek(g_ofile, (signed long)len, SEEK_CUR);
	}
	ofile_read(g_ofile, &len, 2L);
	items = (struct stash_entry far *)mem_alloc((signed long)len);
	ofile_read(g_ofile, items, (signed long)len);
	ofile_close(g_ofile);
	cap = (unsigned short)len >> 2;
	txt_load_list(g_font, TXT_STASH, 4, &txt_buf, &txt);
	if (!quiet)
		show();
}

Stash::~Stash()
{
	signed short len;
	signed short i;

	ofile_create(g_ofile, FILE_STASHES, O_RDWR | O_BINARY);
	for (i = 0; i < nr; i++) {
		ofile_read(g_ofile, &len, 2L);
		ofile_seek(g_ofile, (signed long)len, SEEK_CUR);
	}
	ofile_read(g_ofile, &len, 2L);
	ofile_write(g_ofile, items, (signed long)len);
	ofile_close(g_ofile);
	mem_free(items);
	mem_free(txt_buf);
	mem_free(txt);
}

void Stash::set(signed short i, signed short id, unsigned char n)
{
	items[i].id = id;
	items[i].n = n;
}

signed short Stash::is(signed short i, signed short id, unsigned char n)
{
	if (items[i].id == id && items[i].n == n)
		return 1;
	else
		return 0;
}

void Stash::add(register signed short i, signed short id, unsigned char n)
{
	register signed short k;

	if (items[i].id != 0) {
		if (items[i].id != id) {
			for (k = 0; k < cap; k++) {
				if (items[k].id <= 0)
					break;
			}
			for (; k > i; k--) {
				items[k].id = items[k - 1].id;
				items[k].n = items[k - 1].n;
			}
			set(i, id, n);
		} else {
			items[i].n += n;
		}
		return;
	}
	set(i, id, n);
}

signed short Stash::has_items()
{
	if (update())
		return 1;
	else
		return 0;
}

signed short Stash::count_items()
{
	update();
	return count;
}

signed short Stash::show()
{
	if (!update()) {
		dlg_message(g_dialog, txt[0]);
		return 0;
	} else
		return menu();
}

signed short Stash::update()
{
	signed short i;

	top = count = 0;
	for (i = 0; i < cap; i++) {
		if (items[i].id > 0)
			count++;
	}
	if (count)
		return 1;
	return 0;
}

signed short Stash::fill(char far *s)
{
	signed short n;
	signed short i;
	signed short id;

	i = 0;
	id = items[top].id;
	more = 0;
	if (count && i + top <= count) {
		do {
			n = items[i + top].n;
			if (n > 1) {
				itoa(n, s + i * LINE_LEN, 10);
				strcat(s + i * LINE_LEN, " ");
				strcat(s + i * LINE_LEN, grammar_plural(4, g_item_names[id], 0));
			} else {
				strcat(s + i * LINE_LEN, grammar_plural(0, g_item_names[id], 0));
			}
			i++;
			id = items[i + top].id;
		} while (i + top < count && i < MAX_LINES);
		if (count > MAX_LINES) {
			if (i < MAX_LINES)
				i++;
			more = i;
			strcpy(s + (more - 1) * LINE_LEN, txt[3]);
		}
	}
	if (i == 0)
		dlg_message(g_dialog, txt[0]);
	return i;
}

signed short Stash::menu()
{
	signed short ret;
	char far *s;
	signed short old_w;
	signed short nl;
	signed short sel;
	signed short n;
	signed short id;
	signed short take;
	register signed short k;

	ret = 0;
	s = new char[MAX_LINES * LINE_LEN];
	old_w = dlg_set16(g_dialog, 4);
	do {
		memset(s, 0, MAX_LINES * LINE_LEN);
		nl = fill(s);
		if (nl != 0) {
			sel = dlg_choose(g_dialog, txt[1], nl, s, s + 40, s + 80, s + 120, s + 160, s + 200, s + 240,
					s + 280, s + 320, s + 360);
			if (more && more == sel) {
				top += 9;
				if (top >= count)
					top = 0;
			} else if (sel != -1) {
				k = top + sel - 1;
				n = items[k].n;
				id = items[k].id;
				take = 1;
				if (n > 1) {
					register signed short w;

					w = 1;
					sprintf(g_text_output_buf, txt[2], n, grammar_plural(7, g_item_names[id], 0));
					if (n >= 100)
						w = 3;
					else if (n >= 10)
						w = 2;
					take = dlg_input(g_dialog, w, g_text_output_buf, 0, n, n);
					if (take > n)
						take = n;
				}
				if (take > 0) {
					if (id == ITEM_MONEY) {
						group_add_money(g_group, (signed long)n * 100);
						take = n;
						ret = 1;
					} else {
						take = group_give_item(g_group, id, take);
						if (take != 0)
							ret = 1;
					}
					if (take == n) {	/* Eintrag entfernen, Rest nachruecken */
						count--;
						for (; k < count; k++)
							items[k] = items[k + 1];
						memset(&items[count], 0, 4);
						if (top != 0) {
							if (top == 9 && count <= MAX_LINES)
								top = 0;
							else if (count <= top)
								top -= 9;
						}
					} else if (take != 0) {
						items[k].n -= take;
					}
				}
			}
		}
	} while (sel != -1 && nl != 0);
	dlg_set16(g_dialog, old_w);
	delete s;
	return ret;
}
