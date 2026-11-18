/* OVL219 - Overlay O219 (Segment 0x23c3 in SCHWEIF.EXE): Hilfsfunktionen der Reisekarte (Wegtabelle g_routes,
 * 10 Bytes je Weg, NR_ROUTES Wege; Ortstabelle g_places, 3 Bytes {Ort, dirs, dirs_all}; Texte g_ttxt3).
 * route_is_known(p, w): Weg w bekannt. place_count_dirs/place_count_all_dirs(p, ort): Anzahl gesetzter Bits in dirs
 * bzw. dirs_all des Orts. route_find(p, a, b): Weg von a nach b (sonst b nach a, sonst ROUTE_NONE). route_first(p, a):
 * erster Weg mit a als Start (sonst Ziel, sonst ROUTE_NONE). route_start_index(p, a): Wegpunkte des ersten Weges laden
 * (p->nr_points = route_load_points(p, w)), Rueckgabe (long) Startindex 0 bzw. nr_points - 4 je nach Richtung.
 * route_find_area(p, a, b): Ort a ueber place_area auf seine Gegend 0xe6..0xeb abbilden und deren Orte nacheinander
 * mit route_find probieren (sonst Meldung g_ttxt3[50..55]). place_next_dest/place_next_dest_all/place_last_dest(p,
 * ort[, x]): fuer jede gesetzte Richtung k route_dest_by_dir(p, ort, k) (die ersten beiden bis Ergebnis != x),
 * Ergebnis 0 -> g_ttxt3[56]. route_clear: Route p->route (40 Bytes) loeschen. place_area(p, ort): Ort -> Gegend
 * (2-4 e6, 7/9 e7, 10/11 e8, 12-15 e9, 17/18 eb, 19/20 ea, sonst unveraendert). travel_enter_town(p, ort): Stadt
 * betreten (Stadt n 1..6, Startfeld x/y, Richtung -> g_exit_dir, Gruppe town/dir/loctype, group_set_screen_pos,
 * Bildschirm loeschen, Ortsname g_ttx[0x163 + n] ueber diary_add). travel_enter_dungeon(p, ort): wie
 * travel_enter_town fuer den Dungeon 4 (group.dungeon) ohne Namen. route_dest_area(p, &ort, x): wie
 * route_find_area, aber mit route_dest_by_dir und Ergebnis ROUTE_END = nicht gefunden.
 *
 * BCC -mlarge -3 -Ot -Og -Oi -c -Y OVL219.CPP   -> 5695/5695 Bytes GLEICH (18.10.2026), cobjfix laeuft
 *   (Namensfassung mit schweif.h: 19.10.2026 GLEICH, LINKGLEICH)
 * - -Oi: memset(p->route, 0, 40) und memset(g_vga_buf, 0, 64000U) als rep stosw.
 * - Bitfelder als unsigned short b:1 (struct dir_set/dir_bits ueber PLACES; Vergleich "== 1" -> and ax,1 / cmp ax,1; unsigned char b:1
 *   ergibt cmp al,1). Die Summe der 8 Bits in place_count_dirs wird trotzdem bytebreit (add al,dl), weil das Ziel
 *   unsigned char ist.
 * - route_find/route_first: "for (i = 0, r = ROUTE_NONE; ...)" (xor cx,cx vor mov di,3e7h); i unsigned (jnc); -Og
 *   haelt den Offset von g_routes[i] in SI (route_find) bzw. DI (route_start_index), Segment neu aus ds:c7d2.
 * - C++ wegen der inline-Setter der Gruppe: set_town/set_dungeon (Byte-Kopie bzw. Konstante nicht kopiert), set_dir
 *   (int-Kopie [bp-0ah]/[bp-6]), set_loctype(set_loctype(v)) wie in O217 (Byte-Zwischenwert + Ergebniskopie).
 * - place_area: die Ortsnummer liegt ohne register von selbst in DL; Sprungtabelle mit default-Eintraegen
 *   fuer 5/6/8/16, "default: break;" ergibt das eigene jmp short.
 * - travel_enter_town: switch mit gemischter Erzeugung (Vergleiche um 12/16, Sprungtabelle fuer 1..9) entsteht von
 *   selbst aus der Quellreihenfolge 1,2,3,4,5,7,9,0x67,12..16,19,20.
 * Originalfehler: route_find/route_first lesen nach erfolgloser Suche g_routes[NR_ROUTES] (einen Eintrag hinter der
 *   Tabelle). place_count_dirs/place_next_dest/place_last_dest suchen den Ort in g_places ohne Grenze.
 *   place_last_dest: kein Abbruch nach dem ersten Treffer, die letzte gesetzte Richtung bestimmt das Ergebnis.
 *   route_start_index: "r = 0" im ersten Zweig ueberfluessig.
 */
#include <stdio.h>
#include <mem.h>
#include "schweif.h"

#define PLACES ((struct dir_set far *)g_places)	/* Ortstabelle mit Bitsicht der Richtungen */

signed char route_is_known(Travel far *p, unsigned char i)
{
	return g_routes[i].known;
}

unsigned char place_count_dirs(Travel far *p, unsigned char id)
{
	unsigned char i = 0;
	unsigned char n = 0;

	while (g_places[i].id != id)
		i++;
	n = PLACES[i].dirs.d0 + PLACES[i].dirs.d1 + PLACES[i].dirs.d2 + PLACES[i].dirs.d3 +
		PLACES[i].dirs.d4 + PLACES[i].dirs.d5 + PLACES[i].dirs.d6 + PLACES[i].dirs.d7;
	return n;
}

unsigned char place_count_all_dirs(Travel far *p, unsigned char id)
{
	unsigned char i = 0;
	unsigned char n = 0;

	while (g_places[i].id != id)
		i++;
	n = PLACES[i].dirs_all.d0 + PLACES[i].dirs_all.d1 + PLACES[i].dirs_all.d2 + PLACES[i].dirs_all.d3 +
		PLACES[i].dirs_all.d4 + PLACES[i].dirs_all.d5 + PLACES[i].dirs_all.d6 + PLACES[i].dirs_all.d7;
	return n;
}

signed short route_find(Travel far *p, unsigned char a, unsigned char b)
{
	unsigned short i;
	signed short r;

	for (i = 0, r = ROUTE_NONE; i < NR_ROUTES && (g_routes[i].from != a || g_routes[i].to != b); i++)
		;
#if FIX_0162
	/* Fix #162: nach erfolgloser Suche nicht hinter der Tabelle lesen */
	if (i < NR_ROUTES && g_routes[i].from == a && g_routes[i].to == b)
#else
	if (g_routes[i].from == a && g_routes[i].to == b)
#endif
		r = i;
	if (r == ROUTE_NONE) {
		for (i = 0; i < NR_ROUTES && (g_routes[i].to != a || g_routes[i].from != b); i++)
			;
#if FIX_0162
		/* Fix #162: nach erfolgloser Suche nicht hinter der Tabelle lesen */
		if (i < NR_ROUTES && g_routes[i].to == a && g_routes[i].from == b)
#else
		if (g_routes[i].to == a && g_routes[i].from == b)
#endif
			r = i;
	}
	return r;
}

signed short route_first(Travel far *p, unsigned char a);

signed long route_start_index(Travel far *p, unsigned char a)
{
	signed long r = 0;
	signed short w;

	w = route_first(p, a);
	p->nr_points = route_load_points(p, w);
	if (g_routes[w].from == a)
		r = 0;
	if (g_routes[w].to == a)
		r = p->nr_points - 4;
	return r;
}

signed short route_first(Travel far *p, unsigned char a)
{
	unsigned short i;
	signed short r;

	for (i = 0, r = ROUTE_NONE; i < NR_ROUTES && g_routes[i].from != a; i++)
		;
#if FIX_0162
	/* Fix #162: nach erfolgloser Suche nicht hinter der Tabelle lesen */
	if (i < NR_ROUTES && g_routes[i].from == a)
		r = i;
#else
	if (g_routes[i].from == a)
		r = i;
#endif
	if (r == ROUTE_NONE) {
		for (i = 0; i < NR_ROUTES && g_routes[i].to != a; i++)
			;
#if FIX_0162
		/* Fix #162: nach erfolgloser Suche nicht hinter der Tabelle lesen */
		if (i < NR_ROUTES && g_routes[i].to == a)
			r = i;
#else
		if (g_routes[i].to == a)
			r = i;
#endif
	}
	return r;
}

signed short route_find_area(Travel far *p, unsigned char place, unsigned char dest)
{
	signed short r = ROUTE_NONE;

	place = place_area(p, place);
	switch (place) {
	case 0xe6:
		place = 2;
		r = route_find(p, place, dest);
		if (r == ROUTE_NONE) {
			place = 3;
			r = route_find(p, place, dest);
			if (r == ROUTE_NONE) {
				place = 4;
				r = route_find(p, place, dest);
				if (r == ROUTE_NONE)
					dlg_message(g_dialog, g_ttxt3[50]);
			}
		}
		break;
	case 0xe7:
		place = 7;
		r = route_find(p, place, dest);
		if (r == ROUTE_NONE) {
			place = 9;
			r = route_find(p, place, dest);
			if (r == ROUTE_NONE)
				dlg_message(g_dialog, g_ttxt3[51]);
		}
		break;
	case 0xe8:
		place = 10;
		r = route_find(p, place, dest);
		if (r == ROUTE_NONE) {
			place = 11;
			r = route_find(p, place, dest);
			if (r == ROUTE_NONE)
				dlg_message(g_dialog, g_ttxt3[52]);
		}
		break;
	case 0xe9:
		place = 12;
		r = route_find(p, place, dest);
		if (r == ROUTE_NONE) {
			place = 13;
			r = route_find(p, place, dest);
			if (r == ROUTE_NONE) {
				place = 14;
				r = route_find(p, place, dest);
				if (r == ROUTE_NONE) {
					place = 15;
					r = route_find(p, place, dest);
					if (r == ROUTE_NONE)
						dlg_message(g_dialog, g_ttxt3[53]);
				}
			}
		}
		break;
	case 0xea:
		place = 19;
		r = route_find(p, place, dest);
		if (r == ROUTE_NONE) {
			place = 20;
			r = route_find(p, place, dest);
			if (r == ROUTE_NONE)
				dlg_message(g_dialog, g_ttxt3[54]);
		}
		break;
	case 0xeb:
		place = 17;
		r = route_find(p, place, dest);
		if (r == ROUTE_NONE) {
			place = 18;
			r = route_find(p, place, dest);
			if (r == ROUTE_NONE)
				dlg_message(g_dialog, g_ttxt3[55]);
		}
		break;
	}
	return r;
}

unsigned char place_next_dest(Travel far *p, unsigned char place, unsigned char back)
{
	unsigned char found = 0;
	unsigned char r = 0;
	unsigned char i = 0;

	while (g_places[i].id != place)
		i++;
	if (PLACES[i].dirs.d0 == 1) {
		r = route_dest_by_dir(p, place, 0);
		if (r != back)
			found = 1;
	}
	if (found != 1 && PLACES[i].dirs.d1 == 1) {
		r = route_dest_by_dir(p, place, 1);
		if (r != back)
			found = 1;
	}
	if (found != 1 && PLACES[i].dirs.d2 == 1) {
		r = route_dest_by_dir(p, place, 2);
		if (r != back)
			found = 1;
	}
	if (found != 1 && PLACES[i].dirs.d3 == 1) {
		r = route_dest_by_dir(p, place, 3);
		if (r != back)
			found = 1;
	}
	if (found != 1 && PLACES[i].dirs.d4 == 1) {
		r = route_dest_by_dir(p, place, 4);
		if (r != back)
			found = 1;
	}
	if (found != 1 && PLACES[i].dirs.d5 == 1) {
		r = route_dest_by_dir(p, place, 5);
		if (r != back)
			found = 1;
	}
	if (found != 1 && PLACES[i].dirs.d6 == 1) {
		r = route_dest_by_dir(p, place, 6);
		if (r != back)
			found = 1;
	}
	if (found != 1 && PLACES[i].dirs.d7 == 1) {
		r = route_dest_by_dir(p, place, 7);
		if (r != back)
			found = 1;
	}
	if (r == 0)
		dlg_message(g_dialog, g_ttxt3[56]);
	return r;
}

unsigned char place_next_dest_all(Travel far *p, unsigned char place, unsigned char back)
{
	unsigned char found = 0;
	unsigned char r = 0;
	unsigned char i = 0;

	while (g_places[i].id != place)
		i++;
	if (PLACES[i].dirs_all.d0 == 1) {
		r = route_dest_by_dir(p, place, 0);
		if (r != back)
			found = 1;
	}
	if (found != 1 && PLACES[i].dirs_all.d1 == 1) {
		r = route_dest_by_dir(p, place, 1);
		if (r != back)
			found = 1;
	}
	if (found != 1 && PLACES[i].dirs_all.d2 == 1) {
		r = route_dest_by_dir(p, place, 2);
		if (r != back)
			found = 1;
	}
	if (found != 1 && PLACES[i].dirs_all.d3 == 1) {
		r = route_dest_by_dir(p, place, 3);
		if (r != back)
			found = 1;
	}
	if (found != 1 && PLACES[i].dirs_all.d4 == 1) {
		r = route_dest_by_dir(p, place, 4);
		if (r != back)
			found = 1;
	}
	if (found != 1 && PLACES[i].dirs_all.d5 == 1) {
		r = route_dest_by_dir(p, place, 5);
		if (r != back)
			found = 1;
	}
	if (found != 1 && PLACES[i].dirs_all.d6 == 1) {
		r = route_dest_by_dir(p, place, 6);
		if (r != back)
			found = 1;
	}
	if (found != 1 && PLACES[i].dirs_all.d7 == 1) {
		r = route_dest_by_dir(p, place, 7);
		if (r != back)
			found = 1;
	}
	if (r == 0)
		dlg_message(g_dialog, g_ttxt3[56]);
	return r;
}

unsigned char place_last_dest(Travel far *p, unsigned char place)
{
	unsigned char r = 0;
	unsigned char i = 0;

	while (g_places[i].id != place)
		i++;
	if (PLACES[i].dirs.d0 == 1)
		r = route_dest_by_dir(p, place, 0);
	if (PLACES[i].dirs.d1 == 1)
		r = route_dest_by_dir(p, place, 1);
	if (PLACES[i].dirs.d2 == 1)
		r = route_dest_by_dir(p, place, 2);
	if (PLACES[i].dirs.d3 == 1)
		r = route_dest_by_dir(p, place, 3);
	if (PLACES[i].dirs.d4 == 1)
		r = route_dest_by_dir(p, place, 4);
	if (PLACES[i].dirs.d5 == 1)
		r = route_dest_by_dir(p, place, 5);
	if (PLACES[i].dirs.d6 == 1)
		r = route_dest_by_dir(p, place, 6);
	if (PLACES[i].dirs.d7 == 1)
		r = route_dest_by_dir(p, place, 7);
	if (r == 0)
		dlg_message(g_dialog, g_ttxt3[56]);
	return r;
}

void route_clear(Travel far *p)
{
	memset(p->route, 0, ROUTE_MAX);
}

unsigned char place_area(Travel far *p, unsigned char a)
{
	switch (a) {
	case 2: a = 0xe6; break;
	case 3: a = 0xe6; break;
	case 4: a = 0xe6; break;
	case 7: a = 0xe7; break;
	case 9: a = 0xe7; break;
	case 10: a = 0xe8; break;
	case 11: a = 0xe8; break;
	case 12: a = 0xe9; break;
	case 13: a = 0xe9; break;
	case 14: a = 0xe9; break;
	case 15: a = 0xe9; break;
	case 17: a = 0xeb; break;
	case 18: a = 0xeb; break;
	case 19: a = 0xea; break;
	case 20: a = 0xea; break;
	default: break;
	}
	return a;
}

void travel_enter_town(Travel far *p, unsigned char place)
{
	signed short x;
	signed short y;
	signed short town = 0;
	signed short dir;

	switch (place) {
	case 1: town = 1; x = 16; y = 16; dir = 2; break;
	case 2: town = 2; x = 13; y = 2; dir = 0; break;
	case 3: town = 2; x = 9; y = 29; dir = 2; break;
	case 4: town = 2; x = 17; y = 18; dir = 3; break;
	case 5: town = 3; x = 29; y = 12; dir = 3; break;
	case 7: town = 4; x = 12; y = 42; dir = 3; break;
	case 9: town = 4; x = 6; y = 5; dir = 0; break;
	case 0x67: town = 4; x = 23; y = 19; dir = 0; break;
	case 12: town = 5; x = 23; y = 35; dir = 2; break;
	case 13: town = 5; x = 23; y = 7; dir = 0; break;
	case 14: town = 5; x = 32; y = 7; dir = 3; break;
	case 15: town = 5; x = 10; y = 6; dir = 1; break;
	case 16: town = 5; x = 14; y = 8; dir = 2; break;
	case 19: town = 6; x = 13; y = 20; dir = 1; break;
	case 20: town = 6; x = 17; y = 4; dir = 0; break;
	}
	if (town != 0) {
		g_exit_dir = dir;
		g_group->set_town(town);
		g_group->set_dir(dir);
		group_set_screen_pos(x, y);
		g_group->set_loctype(g_group->set_loctype(place == 0x67 ? 0x3a : 0));
		mouse_hide(0);
		memset(g_vga_buf, 0, 64000U);
		mouse_show(0);
		g_refresh = 1;
		g_pp20_index = -1;
		sprintf(g_text_output_buf, g_ttx[0x17a], g_ttx[town + 0x163]);
		diary_add(g_diary, g_text_output_buf, 0);
	}
}

void travel_enter_dungeon(Travel far *p, unsigned char place)
{
	signed short x;
	signed short y;
	signed short dir = -1;

	switch (place) {
	case 0x2c: x = 41; y = 31; dir = 2; break;
	case 0x2d: x = 0; y = 6; dir = 0; break;
	case 0x61: x = 47; y = 6; dir = 1; break;
	case 0x77: x = 35; y = 0; dir = 0; break;
	case 0x84: x = 24; y = 31; dir = 2; break;
	case 0x85: x = 3; y = 31; dir = 2; break;
	case 0x86: x = 0; y = 16; dir = 0; break;
	}
	if (dir != -1) {
		g_exit_dir = dir;
		g_group->set_dungeon(4);
		g_group->set_dir(dir);
		group_set_screen_pos(x, y);
		g_group->set_loctype(g_group->set_loctype(0));
		mouse_hide(0);
		memset(g_vga_buf, 0, 64000U);
		mouse_show(0);
		g_refresh = 1;
		g_pp20_index = -1;
	}
}

unsigned char route_dest_area(Travel far *p, unsigned char far *pplace, unsigned char dir)
{
	unsigned char r = 0;

	switch (*pplace) {
	case 0xe6:
		*pplace = 2;
		r = route_dest_by_dir(p, *pplace, dir);
		if (r == ROUTE_END) {
			*pplace = 3;
			r = route_dest_by_dir(p, *pplace, dir);
			if (r == ROUTE_END) {
				*pplace = 4;
				r = route_dest_by_dir(p, *pplace, dir);
				if (r == ROUTE_END)
					dlg_message(g_dialog, g_ttxt3[50]);
			}
		}
		break;
	case 0xe7:
		*pplace = 7;
		r = route_dest_by_dir(p, *pplace, dir);
		if (r == ROUTE_END) {
			*pplace = 9;
			r = route_dest_by_dir(p, *pplace, dir);
			if (r == ROUTE_END)
				dlg_message(g_dialog, g_ttxt3[51]);
		}
		break;
	case 0xe8:
		*pplace = 10;
		r = route_dest_by_dir(p, *pplace, dir);
		if (r == ROUTE_END) {
			*pplace = 11;
			r = route_dest_by_dir(p, *pplace, dir);
			if (r == ROUTE_END)
				dlg_message(g_dialog, g_ttxt3[52]);
		}
		break;
	case 0xe9:
		*pplace = 12;
		r = route_dest_by_dir(p, *pplace, dir);
		if (r == ROUTE_END) {
			*pplace = 13;
			r = route_dest_by_dir(p, *pplace, dir);
			if (r == ROUTE_END) {
				*pplace = 14;
				r = route_dest_by_dir(p, *pplace, dir);
				if (r == ROUTE_END) {
					*pplace = 15;
					r = route_dest_by_dir(p, *pplace, dir);
					if (r == ROUTE_END)
						dlg_message(g_dialog, g_ttxt3[53]);
				}
			}
		}
		break;
	case 0xea:
		*pplace = 19;
		r = route_dest_by_dir(p, *pplace, dir);
		if (r == ROUTE_END) {
			*pplace = 20;
			r = route_dest_by_dir(p, *pplace, dir);
			if (r == ROUTE_END)
				dlg_message(g_dialog, g_ttxt3[54]);
		}
		break;
	case 0xeb:
		*pplace = 17;
		r = route_dest_by_dir(p, *pplace, dir);
		if (r == ROUTE_END) {
			*pplace = 18;
			r = route_dest_by_dir(p, *pplace, dir);
			if (r == ROUTE_END)
				dlg_message(g_dialog, g_ttxt3[55]);
		}
		break;
	}
	return r;
}
