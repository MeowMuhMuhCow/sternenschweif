/* OVL067 - Overlay O067 (Segment 0x208a in SCHWEIF.EXE): naechtliche Begegnungen im Lager und Besucher am Lagerfeuer
 * (Aufrufe aus O072/O073; Lagerobjekt c = Camp mit Texten CAMP.LTX = Datei 0xae, Ergebnis c->penalty).
 * camp_encounter(c, a, h): g_fight_max_enemies = 0; bei W100 < 30 und a >= 0 Besucher camp_visitor(c, h), sonst
 *   Kampf:
 *   a >= 0 -> g_camp_foes[3][5] {id, count} (Zeile 0 fuer a = 0, 1 fuer 1..3, 2 fuer 4/5, Spalte W5 - 1),
 *   g_fight_max_enemies = count - W(count) * 6/7 (4/5, 5/6); a < 0 -> Staerke 20, Eintrag [(-a-1) / 5][(-a-1) % 5].
 *   Bei g_fight_max_enemies > 0 und lebenden Helden: Texte frei, g_fight_flag = 1, start_fight(id, 1), Texte neu;
 *   Rueckgabe Kampfergebnis bzw. 0.
 * camp_visitor(c, h): Orts- und Wegtabelle (Dateien 0xa1/0xa2, Laenge file_length) nach g_places/g_routes laden,
 *   Besucher r = camp_visitor_type; r != 0: schlafen alle, weckt der Held mit der geringsten IN (txt[50]; keiner ->
 *   r = 0), sonst txt[60]. switch r (Rumpfreihenfolge 0, 3, 2, 4, 1): 0 -> nur ein Geraeusch (txt[49] bzw. txt[113]
 *   mit zufaelligem Helden, set_asleep), 1..4 -> Streuner/Gaukler/Druide/Jaeger (O073); penalty == 1 -> txt[64];
 *   beide Tabellen zurueckschreiben (ofile_create/ofile_write/ofile_close) und freigeben.
 * camp_visitor_type: Weg route_find(group.route_from, route_to); nur nachts (g_daytime ausserhalb 0xa8c0..0x1a5e0)
 *   und bei schlafender Gruppe oder W100 < 70: Wege 0x40..0x52 -> 3 Gaukler, 0x54..0x85 -> 2 Druide, 0x86..0x9e
 *   -> 1 Streuner, 0xa2..0xbb oder 0x2f..0x3b -> 4 Jaeger, jeweils nur, wenn der Besucher noch nicht da war.
 * camp_reveal_area(c, mode): aktueller Weg und je Gebiet mode 1..4 Wege (route.known), Orte (place_reveal_dirs)
 *   und Richtungsbits (camp_place_set_dir) bekannt machen. camp_place_set_dir(c, id, bit): Ort mit Kennung id
 *   suchen, bei Index < 0x106 Bit (1 << bit) in travel_place.dirs setzen.
 *
 * BCC -mlarge -3 -Ot -Og -c -Y -E..\bc\BIN\TASM.EXE OVL067.CPP -> 5286/5286 Bytes GLEICH (18.10.2026), cobjfix
 *   laeuft, kein eigenes _DATA. Ohne -Og bzw. mit -2 falsche Laenge.
 *   (Namensfassung mit schweif.h: 19.10.2026 GLEICH, LINKGLEICH; Reiseobjekt als TRAVEL_OBJ = (Travel far *)g_travel)
 * - "#pragma option -rd" vor camp_place_set_dir (letzte Funktion, ohne Aufrufe): sonst laedt BCC den Parameter id in
 *   CL (cmp
 *   es:[bx],cl), das Original vergleicht mit [bp+0ah] (mov al,es:[bx] / cmp al,[bp+0ah]) und haelt i (register)
 *   in DX. Ganzes Modul mit -rd vertauscht in camp_encounter SI/DI. Erfolglos: register/volatile am Parameter, Kopie,
 *   Operandenreihenfolge, -Oe/-Z/-O/-Ol/-r-.
 * - camp_reveal_area-Rumpf (ca. 170 Aufrufe/Stores) per gen/g067.py aus der ASM erzeugt; id-Parameter von
 *   camp_place_set_dir und die
 *   Kennung im 3-Byte-Satz unsigned char (push word 0x80 statt push byte -80h). Sprungtabelle: case 1, 2, 4, 3.
 * - camp_encounter: id ohne register (DI), -Og-Zwischenwert Tabellenoffset (r * 4) in SI; "ret = flag =
 *   g_fight_max_enemies = 0"
 *   speichert g_fight_max_enemies zuerst; "(-a - 1) / 5" und "% 5" -> -Og-Zwischenwert [bp-0ah]; "r =
 *   random_schweif(5) - 1" mit
 *   unsigned char r -> add al,0ffh. start_fight hier signed short (mov [bp-2],ax ohne cbw).
 * - camp_visitor: "h = get_hero_attrib_min(..); if (h != 0)" -> cmp dword (Parameter h wird ueberschrieben).
 * - camp_visitor_type: "return res;" (int) mit char-Rueckgabe -> mov al,[bp-2] + toter Epilog.
 * Originalfehler: camp_place_set_dir sucht ohne Grenze und prueft i < 0x106 erst nach der Schleife (unbekannte
 *   Kennung ->
 *   Lesen ueber das Tabellenende hinaus).
 */
#include <stdio.h>

#include "schweif.h"

/* Phase B: eigene Daten des Moduls (ds:57dc..5818, dsana) - initialisierte Daten in Adressreihenfolge */
struct camp_foe g_camp_foes[3][5] = {
	{ { 7, 6 },
	{ 13, 17 },
	{ 20, 6 },
	{ 28, 8 },
	{ 16, 6 } },
	{ { 5, 4 },
	{ 6, 8 },
	{ 18, 10 },
	{ 25, 8 },
	{ 14, 6 } },
	{ { 37, 8 },
	{ 39, 5 },
	{ 33, 5 },
	{ 22, 6 },
	{ 35, 5 } }
};

#define TRAVEL_OBJ	((Travel far *)g_travel)	/* Reiseobjekt in den Daten (route_find, place_reveal_dirs) */

void camp_visitor(Camp far *c, struct hero far *h);
signed char camp_visitor_type(Camp far *c);
void camp_place_set_dir(Camp far *c, unsigned char id, signed char bit);

signed short camp_encounter(Camp far *c, signed char a, struct hero far *h)
{
	signed short ret;
	signed short flag;
	unsigned char r;
	signed char x;
	signed char y;
	signed short id;

	ret = flag = g_fight_max_enemies = 0;
	if (random_schweif(100) < 30 && a >= 0) {
		camp_visitor(c, h);
	} else {
		flag = 1;
	}
	if (flag == 1) {
		if (a >= 0) {
			r = random_schweif(5) - 1;
			switch (a) {
			case 0:
				g_fight_max_enemies = g_camp_foes[0][r].count - random_schweif(g_camp_foes[0][r].count) * 6 / 7;
				id = g_camp_foes[0][r].id;
				break;
			case 1:
			case 2:
			case 3:
				g_fight_max_enemies = g_camp_foes[1][r].count - random_schweif(g_camp_foes[1][r].count) * 4 / 5;
				id = g_camp_foes[1][r].id;
				break;
			case 4:
			case 5:
				g_fight_max_enemies = g_camp_foes[2][r].count - random_schweif(g_camp_foes[2][r].count) * 5 / 6;
				id = g_camp_foes[2][r].id;
				break;
			}
		} else {
			g_fight_max_enemies = 20;
			x = (-a - 1) / 5;
			y = (-a - 1) % 5;
			id = g_camp_foes[x][y].id;
		}
	}
	if (g_fight_max_enemies > 0 && group_count_alive(g_group) >= 1) {
		mem_free(c->txt_buf);
		mem_free(c->txt);
		g_fight_flag = 1;
		ret = start_fight(id, 1);
		txt_load_list(g_font, 0xae, 0x8c, &c->txt_buf, &c->txt);
	} else {
		ret = 0;
	}
	return ret;
}

void camp_visitor(Camp far *c, struct hero far *h)
{
	signed char r;
	signed long n2;
	signed long n1;

	n1 = file_length(g_file_main, 0xa1);
	n2 = file_length(g_file_main, 0xa2);
	g_places = (struct travel_place far *)mem_alloc(n1);
	ofile_load(g_ofile, 0xa1, g_places);
	g_routes = (struct route far *)mem_alloc(n2);
	ofile_load(g_ofile, 0xa2, g_routes);
	r = camp_visitor_type(c);
	if (r != 0) {
		if (group_all_asleep(g_group)) {
			h = get_hero_attrib_min(g_group, ATTRIB_IN);
			if (h != 0) {
				set_asleep(h, 0);
				/* CAMP.LTX[50]: "%s wird unsanft aus %s Traeumen gerissen. Eine Hand liegt auf %sr Schulter und schuettelt %s
				 * kraeftig durch." */
				sprintf(g_text_output_buf, c->txt[50], h->name, grammar_pronoun(h->sex, 3, 1),
					grammar_pronoun(h->sex, 1, 1), grammar_pronoun(h->sex, 2, 0));
				dlg_message(g_dialog, g_text_output_buf);
			} else {
				r = 0;
			}
		} else {
			/* CAMP.LTX[60]: "Eine Gestalt tritt aus dem Dunkel und kommt an das Lagerfeuer." */
			dlg_message(g_dialog, c->txt[60]);
		}
	}
	switch (r) {
	case 0:
		if (!group_all_asleep(g_group)) {
			if (group_count_check1(g_group) > 1) {
				wait_ticks(30);
				/* CAMP.LTX[49]: "Obwohl Ihr aufmerksam die Umgebung Eures Lagers beobachtet, koennt Ihr nichts gefaehrliches
				 * entdecken.@<Es werden wohl bloss irgendwelche Tiere gewesen sein>, meint %s." */
				sprintf(g_text_output_buf, c->txt[49], h->name);
				dlg_message(g_dialog, g_text_output_buf);
			}
		} else {
			struct hero far *h2;
			wait_ticks(30);
			h2 = get_random_alive_hero(g_group);
			set_asleep(h2, 0);
			/* CAMP.LTX[113]: "Ploetzlich wacht %s auf, und spuert wie irgendetwas an Eurem Gepaeck herumschnueffelt.@Als %s
			 * aufspringt, verschwindet ein Schatten im Dunkel." */
			sprintf(g_text_output_buf, c->txt[113], h2->name, grammar_pronoun(h2->sex, 0, 0));
			dlg_message(g_dialog, g_text_output_buf);
			set_asleep(h2, 1);
		}
		break;
	case 3:
		camp_visitor_juggler(c);
		break;
	case 2:
		camp_visitor_druid(c);
		break;
	case 4:
		camp_visitor_hunter(c);
		break;
	case 1:
		camp_visitor_vagabond(c);
		break;
	}
	/* CAMP.LTX[64]: "Unter lautem Fluchen verschwindet die Gestalt im Dunkel der Nacht." */
	if (c->penalty == 1) dlg_message(g_dialog, c->txt[64]);
	ofile_create(g_ofile, 0xa1, 4);
	ofile_write(g_ofile, g_places, n1);
	ofile_close(g_ofile);
	ofile_create(g_ofile, 0xa2, 4);
	ofile_write(g_ofile, g_routes, n2);
	ofile_close(g_ofile);
	mem_free(g_places);
	mem_free(g_routes);
}

signed char camp_visitor_type(Camp far *c)
{
	signed short res;
	signed long t;
	register signed short pos;

	t = g_daytime;
	res = 0;
	pos = route_find(TRAVEL_OBJ, g_group->route_from, g_group->route_to);
	if (t > 0x1a5e0L || t < 0xa8c0L) {
		if (group_all_asleep(g_group) || random_schweif(100) < 70) {
			if (pos >= 0x40 && pos <= 0x52 && !g_camp_juggler_met) res = 3;
			if (pos >= 0x54 && pos <= 0x85 && !g_camp_druid_met) res = 2;
			if (pos >= 0x86 && pos <= 0x9e && !g_camp_vagabond_met) res = 1;
			if ((pos >= 0xa2 && pos <= 0xbb || pos >= 0x2f && pos <= 0x3b) && !g_camp_hunter_met) res = 4;
		}
	}
	return res;
}

void camp_reveal_area(Camp far *c, signed char mode)
{
	register signed short i;

	i = route_find(TRAVEL_OBJ, g_group->route_from, g_group->route_to);
	g_routes[i].known = 1;
	switch (mode) {
	case 1:
		for (i = 0x87; i <= 0x92; i++) {
			if (i != 0x8e && i != 0x91) g_routes[i].known = 1;
		}
		for (i = 0x1e; i <= 0x24; i++) g_routes[i].known = 1;
		for (i = 0x3c; i <= 0x3f; i++) g_routes[i].known = 1;
		for (i = 0x98; i <= 0x9a; i++) g_routes[i].known = 1;
		place_reveal_dirs(TRAVEL_OBJ, 0x0e);
		place_reveal_dirs(TRAVEL_OBJ, 0x11);
		place_reveal_dirs(TRAVEL_OBJ, 0x1f);
		place_reveal_dirs(TRAVEL_OBJ, 0x26);
		place_reveal_dirs(TRAVEL_OBJ, 0x2e);
		place_reveal_dirs(TRAVEL_OBJ, 0x31);
		place_reveal_dirs(TRAVEL_OBJ, 0x32);
		place_reveal_dirs(TRAVEL_OBJ, 0x34);
		place_reveal_dirs(TRAVEL_OBJ, 0x49);
		place_reveal_dirs(TRAVEL_OBJ, 0x79);
		place_reveal_dirs(TRAVEL_OBJ, 0x7a);
		place_reveal_dirs(TRAVEL_OBJ, 0x81);
		camp_place_set_dir(c, 0x23, 6);
		camp_place_set_dir(c, 0x33, 3);
		camp_place_set_dir(c, 0x33, 7);
		camp_place_set_dir(c, 0x35, 3);
		camp_place_set_dir(c, 0x35, 6);
		camp_place_set_dir(c, 0x4a, 2);
		camp_place_set_dir(c, 0x4a, 6);
		camp_place_set_dir(c, 0x4b, 1);
		camp_place_set_dir(c, 0x4b, 5);
		camp_place_set_dir(c, 0x7d, 4);
		camp_place_set_dir(c, 0x7d, 6);
		camp_place_set_dir(c, 0x7e, 1);
		camp_place_set_dir(c, 0x7e, 7);
		camp_place_set_dir(c, 0x80, 2);
		camp_place_set_dir(c, 0x80, 6);
		camp_place_set_dir(c, 0x36, 6);
		camp_place_set_dir(c, 0x36, 2);
		camp_place_set_dir(c, 0x36, 4);
		camp_place_set_dir(c, 0x78, 0);
		camp_place_set_dir(c, 0x78, 2);
		camp_place_set_dir(c, 0x78, 6);
		camp_place_set_dir(c, 0x7b, 0);
		camp_place_set_dir(c, 0x7b, 2);
		camp_place_set_dir(c, 0x7b, 4);
		break;
	case 2:
		g_routes[0x54].known = 1;
		g_routes[0x55].known = 1;
		g_routes[0xfa].known = 1;
		g_routes[0xfb].known = 1;
		g_routes[0x101].known = 1;
		for (i = 0x25; i <= 0x2e; i++) g_routes[i].known = 1;
		for (i = 0x57; i <= 0x5d; i++) g_routes[i].known = 1;
		for (i = 0x61; i <= 0x65; i++) g_routes[i].known = 1;
		for (i = 0x72; i <= 0x78; i++) g_routes[i].known = 1;
		place_reveal_dirs(TRAVEL_OBJ, 0x17);
		place_reveal_dirs(TRAVEL_OBJ, 0x18);
		place_reveal_dirs(TRAVEL_OBJ, 0x19);
		place_reveal_dirs(TRAVEL_OBJ, 0x20);
		place_reveal_dirs(TRAVEL_OBJ, 0x38);
		place_reveal_dirs(TRAVEL_OBJ, 0x3a);
		place_reveal_dirs(TRAVEL_OBJ, 0x3b);
		place_reveal_dirs(TRAVEL_OBJ, 0x3d);
		place_reveal_dirs(TRAVEL_OBJ, 0x58);
		place_reveal_dirs(TRAVEL_OBJ, 0x5b);
		place_reveal_dirs(TRAVEL_OBJ, 0x5f);
		place_reveal_dirs(TRAVEL_OBJ, 0x6f);
		place_reveal_dirs(TRAVEL_OBJ, 0x70);
		place_reveal_dirs(TRAVEL_OBJ, 0xbe);
		place_reveal_dirs(TRAVEL_OBJ, 0xbf);
		camp_place_set_dir(c, 0x04, 3);
		camp_place_set_dir(c, 0x1a, 3);
		camp_place_set_dir(c, 0x39, 0);
		camp_place_set_dir(c, 0x39, 4);
		camp_place_set_dir(c, 0x3c, 2);
		camp_place_set_dir(c, 0x3c, 6);
		camp_place_set_dir(c, 0x59, 0);
		camp_place_set_dir(c, 0x59, 6);
		camp_place_set_dir(c, 0x5d, 1);
		camp_place_set_dir(c, 0x5d, 4);
		camp_place_set_dir(c, 0x5e, 2);
		camp_place_set_dir(c, 0x5e, 6);
		camp_place_set_dir(c, 0x62, 0);
		camp_place_set_dir(c, 0x62, 4);
		camp_place_set_dir(c, 0x63, 0);
		camp_place_set_dir(c, 0x63, 5);
		camp_place_set_dir(c, 0x6b, 2);
		camp_place_set_dir(c, 0x6b, 5);
		camp_place_set_dir(c, 0x6c, 1);
		camp_place_set_dir(c, 0x6c, 5);
		camp_place_set_dir(c, 0x6d, 3);
		camp_place_set_dir(c, 0x6d, 7);
		camp_place_set_dir(c, 0x6e, 3);
		camp_place_set_dir(c, 0x6e, 7);
		camp_place_set_dir(c, 0xc0, 0);
		camp_place_set_dir(c, 0xc0, 6);
		camp_place_set_dir(c, 0xc1, 0);
		camp_place_set_dir(c, 0xc1, 4);
		camp_place_set_dir(c, 0x37, 7);
		camp_place_set_dir(c, 0x37, 4);
		camp_place_set_dir(c, 0x37, 3);
		break;
	case 4:
		g_routes[0xaa].known = 1;
		g_routes[0xae].known = 1;
		g_routes[0xaf].known = 1;
		for (i = 0x2f; i <= 0x31; i++) g_routes[i].known = 1;
		for (i = 0x34; i <= 0x3b; i++) g_routes[i].known = 1;
		for (i = 0xa4; i <= 0xa7; i++) g_routes[i].known = 1;
		for (i = 0xb7; i <= 0xba; i++) g_routes[i].known = 1;
		place_reveal_dirs(TRAVEL_OBJ, 0x09);
		place_reveal_dirs(TRAVEL_OBJ, 0x2b);
		place_reveal_dirs(TRAVEL_OBJ, 0x40);
		place_reveal_dirs(TRAVEL_OBJ, 0x8c);
		camp_place_set_dir(c, 0x1a, 1);
		camp_place_set_dir(c, 0x1a, 6);
		camp_place_set_dir(c, 0x3e, 2);
		camp_place_set_dir(c, 0x3e, 5);
		camp_place_set_dir(c, 0x3f, 1);
		camp_place_set_dir(c, 0x3f, 5);
		camp_place_set_dir(c, 0x43, 3);
		camp_place_set_dir(c, 0x43, 6);
		camp_place_set_dir(c, 0x44, 2);
		camp_place_set_dir(c, 0x44, 6);
		camp_place_set_dir(c, 0x45, 2);
		camp_place_set_dir(c, 0x45, 6);
		camp_place_set_dir(c, 0x46, 1);
		camp_place_set_dir(c, 0x46, 5);
		camp_place_set_dir(c, 0x47, 0);
		camp_place_set_dir(c, 0x47, 4);
		camp_place_set_dir(c, 0x48, 0);
		camp_place_set_dir(c, 0x48, 4);
		camp_place_set_dir(c, 0x88, 2);
		camp_place_set_dir(c, 0x88, 7);
		camp_place_set_dir(c, 0x89, 2);
		camp_place_set_dir(c, 0x89, 6);
		camp_place_set_dir(c, 0x8a, 2);
		camp_place_set_dir(c, 0x8a, 6);
		camp_place_set_dir(c, 0x8b, 2);
		camp_place_set_dir(c, 0x8b, 7);
		camp_place_set_dir(c, 0x8d, 3);
		camp_place_set_dir(c, 0x8d, 7);
		camp_place_set_dir(c, 0x90, 1);
		camp_place_set_dir(c, 0x90, 4);
		camp_place_set_dir(c, 0x96, 4);
		camp_place_set_dir(c, 0x96, 1);
		camp_place_set_dir(c, 0x97, 0);
		camp_place_set_dir(c, 0x97, 5);
		camp_place_set_dir(c, 0x1b, 0);
		camp_place_set_dir(c, 0x1b, 2);
		camp_place_set_dir(c, 0x1b, 4);
		camp_place_set_dir(c, 0x1c, 0);
		camp_place_set_dir(c, 0x1c, 5);
		camp_place_set_dir(c, 0x1c, 3);
		break;
	case 3:
		g_routes[0xfe].known = 1;
		for (i = 0x1e; i <= 0x24; i++) g_routes[i].known = 1;
		for (i = 0x40; i <= 0x4a; i++) g_routes[i].known = 1;
		for (i = 0x4e; i <= 0x51; i++) g_routes[i].known = 1;
		place_reveal_dirs(TRAVEL_OBJ, 0x15);
		place_reveal_dirs(TRAVEL_OBJ, 0x1f);
		place_reveal_dirs(TRAVEL_OBJ, 0x21);
		place_reveal_dirs(TRAVEL_OBJ, 0x23);
		place_reveal_dirs(TRAVEL_OBJ, 0x25);
		place_reveal_dirs(TRAVEL_OBJ, 0x34);
		place_reveal_dirs(TRAVEL_OBJ, 0x4c);
		place_reveal_dirs(TRAVEL_OBJ, 0x4d);
		place_reveal_dirs(TRAVEL_OBJ, 0x4f);
		place_reveal_dirs(TRAVEL_OBJ, 0x50);
		place_reveal_dirs(TRAVEL_OBJ, 0x52);
		place_reveal_dirs(TRAVEL_OBJ, 0x54);
		place_reveal_dirs(TRAVEL_OBJ, 0x5c);
		place_reveal_dirs(TRAVEL_OBJ, 0xc3);
		camp_place_set_dir(c, 0x04, 1);
		camp_place_set_dir(c, 0x31, 2);
		camp_place_set_dir(c, 0x31, 6);
		camp_place_set_dir(c, 0x32, 2);
		camp_place_set_dir(c, 0x32, 7);
		camp_place_set_dir(c, 0x33, 3);
		camp_place_set_dir(c, 0x33, 7);
		camp_place_set_dir(c, 0x35, 3);
		camp_place_set_dir(c, 0x35, 6);
		camp_place_set_dir(c, 0x36, 2);
		camp_place_set_dir(c, 0x36, 6);
		camp_place_set_dir(c, 0x4e, 1);
		camp_place_set_dir(c, 0x4e, 4);
		camp_place_set_dir(c, 0x51, 0);
		camp_place_set_dir(c, 0x51, 6);
		camp_place_set_dir(c, 0x55, 1);
		camp_place_set_dir(c, 0x55, 5);
		camp_place_set_dir(c, 0x56, 1);
		camp_place_set_dir(c, 0x56, 5);
		break;
	}
}

#pragma option -rd
void camp_place_set_dir(Camp far *c, unsigned char id, signed char bit)
{
	register signed short i;

#if FIX_0142
	/* Fix #142: Grenze in die Suchschleife */
	for (i = 0; i < 0x106 && g_places[i].id != id; i++) ;
#else
	for (i = 0; g_places[i].id != id; i++) ;
#endif
	if (i < 0x106) {
		switch (bit) {
		case 0: g_places[i].dirs |= 1; break;
		case 1: g_places[i].dirs |= 2; break;
		case 2: g_places[i].dirs |= 4; break;
		case 3: g_places[i].dirs |= 8; break;
		case 4: g_places[i].dirs |= 0x10; break;
		case 5: g_places[i].dirs |= 0x20; break;
		case 6: g_places[i].dirs |= 0x40; break;
		case 7: g_places[i].dirs |= 0x80; break;
		}
	}
}
