/* OVL001 - Overlay O001 (Segment 0x1ef3 in SCHWEIF.EXE): Ereignisse des Dungeons 16, Ingerimms Schmiede im
 * Finsterkopp (Texte BINGE7.LTX = Datei 0x45a; Fortsetzung in OVL002). Ereignisobjekt ev (struct area_events).
 * dng16_handler(ev, mev): Kartenereignis-Rueckruf (g_area_descs[16]) - Feldposition pos = (Spalte - g_map_x0) << 8 |
 *   (Zeile - g_map_y0), switch auf mev->type (MAP_EVENT_ENTER: 0x603 dng16_stone_plate, 0xe0b/0xc0b Richtung
 *   g_dng16_plates_dir, Reihe 12..14 dng16_hot_plates, 0x810..0x911 dng16_golem; 1/0x67 dng16_door; 2; 0x64..0x70
 *   Sonderfaelle, 0x68 mit pos-Unterverteiler: Truhen Chest 0x2b..0x2d, Ledertruhe, Schatztruhe, Salamanderstein),
 *   beim ersten Aufruf nach dem Betreten (g_finsterkopp_entered == 1) Einsturz des Zugangs txt[28] und Feldaenderung.
 * dng16_door(ev, pos, x, y): Tuer/Schloss an pos -> Lock-Objekt bzw. dng16_double_key, Feldbits, Klang, Automap.
 * dng16_stone_plate: Steinplatte im Schutt (Sinnesschaerfe +5), heben mit zwei Helden (KK-Summe + W5 - 1 >= 32),
 *   GE-Probe +8: Schatulle mit Messer (je 10 AP, Ablage 0x38) oder eingeklemmte Arme (Schaden).
 * dng16_hot_plates(ev, reihe): Reihe 13 - bei Frevel (g_dng16_sacrilege, Ingerimm-Ansehen < -2000) verbrennt die
 *   Gruppe, sonst stirbt, wer nicht Lederhose/-stiefel/-wams am Koerper bzw. den Asthenilring traegt; 12/14 je
 *   nach Richtung dng16_plates_turn_back; Schrittzaehler g_dng16_plate_steps.
 * dng16_plates_turn_back(ev, richtung): Platten gluehen/leuchten auf, Held ohne Schutz mit misslungener MU-Probe
 *   macht
 *   kehrt (wird in eine eigene Gruppe in g_groups versetzt). dng16_walled_passage, dng16_rubble: Texte.
 * dng16_double_key(ev, flag): Doppelbartschluessel (group_find_key), g_lock_cell_bits = 0x2000.
 * dng16_leather_chest: Lederzeug in die Truhe (Ablage 0x39) zuruecklegen bzw. nehmen schaltet Feld (2, 15).
 * dng16_treasure_chest: pluendern (Ablage 0x3a, Frevel, gespendete Heller zurueck) oder Silbertaler dazulegen
 *   (Ingerimm-Ansehen += n, g_dng16_chest_coins), Ablehnen kostet 100. dng16_salamander_chest: Salamanderstein -
 *   gegen das Asthenilschwert tauschen oder einfach nehmen (Frevel), Tagebucheintrag; dng16_exit: Zugang stuerzt ein,
 *   Gruppe verlaesst den Dungeon (unterwegs, Weg 0x6a/0x6a).
 *
 * BCC -mlarge -3 -Ot -c -Y OVL001.C   -> 5896/5896 Bytes GLEICH (18.10.2026), cobjfix laeuft (mit -Og ebenso).
 *   (Namensfassung mit schweif.h: 19.10.2026 GLEICH, LINKGLEICH)
 * - dng16_door: automap_cell_write ueber gecasteten Funktionszeiger (letzter Parameter short statt signed char),
 *   sonst legt BCC pic nicht in DI, sondern den Parameter x (17 Bytes mehr).
 *   Erste Fassung sofort GLEICH (reines C wie OVL002).
 * - dng16_handler: ein switch mit Vergleichsbaum (-2..0x69) und Sprungtabelle (0x6a..0x70) - BCC teilt selbst auf;
 *   "case 0x6c: break;" und "case 0x70: ...; break; default: break;" (zwei jmp).
 * - Register: pos (SI) vor type (DI) deklariert, Zuweisungen getrennt ("pos = .. << 8; pos |= ..").
 * - Lock-Objekt (0x2a Bytes) als struct lock_buf mit Lock_ctor/Lock_dtor in C.
 * - Block-Locals: dng16_stone_plate (h im if-Block, o im spaeteren Block teilen [bp-0eh]); dng16_treasure_chest/
 *   dng16_salamander_chest ganz ohne Locals auf Funktionsebene - "if (..) { Locals; ...; return; } { Local }"
 *   (Epilogkopie, zweiter Block ab [bp-2]).
 * Originalfehler/Auffaelliges: dng16_plates_turn_back schreibt nach der Gruppensuche auch dann in g = &g_groups[i],
 *   wenn keine Gruppe gefunden wurde (i == 6 -> Eintrag hinter der Tabelle); dng16_double_key gibt ret zurueck, das
 *   immer 0 ist;
 *   dng16_hot_plates ruft get_item_pos bis zu siebenmal mit denselben Werten auf.
 */
#include <stdio.h>
#include "schweif.h"

struct lock_buf {			/* Platz fuer ein Lock-Objekt (O108, 0x29 Bytes; C-Sicht mit Lock_ctor/Lock_dtor) */
	char raw[0x2a];
};

signed short dng16_door(struct area_events far *ev, signed short pos, signed short x, signed short y);
void dng16_stone_plate(struct area_events far *ev);
void dng16_hot_plates(struct area_events far *ev, signed short row);
void dng16_plates_turn_back(struct area_events far *ev, signed short flag);
void dng16_walled_passage(struct area_events far *ev);
signed short dng16_double_key(struct area_events far *ev, signed short flag);
void dng16_leather_chest(struct area_events far *ev);
void dng16_treasure_chest(struct area_events far *ev);
void dng16_salamander_chest(struct area_events far *ev);
void dng16_exit(struct area_events far *ev);
void dng16_rubble(struct area_events far *ev);

signed short dng16_handler(struct area_events far *ev, struct map_event far *mev)
{
	signed short ret = 0;
	void far *o;
	register signed short pos;
	register signed short type;

	type = mev->type;
	pos = (mev->y - g_map_x0) << 8;
	pos |= mev->x - g_map_y0;
	if (g_finsterkopp_entered == 1) {
		g_finsterkopp_entered = 2;
		/* BINGE7.LTX[28]: "Hinter Euch stuerzt mit lautem Getoese der Zugang zur Hoehle ein." */
		dlg_message(g_dialog, ev->txt[0x1c]);
		event_set_cell(ev, 3, 0, 0x16, 0, -1);
		automap_cell_write(g_map, 0x312, 0, 3, 0x31);
	}
	switch (type) {
	case MAP_EVENT_ENTER:
		switch (pos) {
		case 0x603:
			dng16_stone_plate(ev);
			break;
		case 0xe0b:
			g_dng16_plates_dir = 0;
			break;
		case 0xc0b:
			g_dng16_plates_dir = 1;
			break;
		case 0xc0c:
		case 0xc0d:
		case 0xc0e:
		case 0xd0e:
		case 0xe0c:
		case 0xe0d:
		case 0xe0e:
			dng16_hot_plates(ev, mev->y - g_map_x0);
			break;
		case 0x810:
		case 0x811:
		case 0x910:
		case 0x911:
			if (!g_dng16_sword_done && !g_dng16_forging && !g_dng16_golem_fought) {
				swap_out_screen();
				dng16_golem(ev);
			}
			break;
		}
		break;
	case 1:
	case 0x67:
		ret = dng16_door(ev, pos, mev->x, mev->y);
		break;
	case 2:
		if (g_dng16_double_key_state == 3) {
			g_dng16_double_key_state = 0;
			event_set_cell(ev, 3, 0x0d, 0, 0, -1);
		}
		break;
	case 0x64:
		if (g_dng13_wheel_turned) {
			g_group->old_dungeon = g_group->dungeon;
			g_group->dungeon = 14;
			group_set_screen_pos(4, 3);
			g_group->dir = 0;
		}
		break;
	case 0x65:
		binge_shaft(ev);
		break;
	case 0x66:
		dng16_walled_passage(ev);
		break;
	case 0x68:
		switch (pos) {
		case 0x1103:
			o = Chest_ctor(0, 0x2b, 1);
			Chest_dtor(o, 3);
			break;
		case 0xb04:
			o = Chest_ctor(0, 0x2c, 1);
			Chest_dtor(o, 3);
			break;
		case 0xa05:
			o = Chest_ctor(0, 0x2d, 1);
			Chest_dtor(o, 3);
			break;
		case 0x1209:
			dng16_leather_chest(ev);
			break;
		case 0x1210:
			dng16_treasure_chest(ev);
			break;
		case 0x1212:
			dng16_salamander_chest(ev);
			break;
		}
		break;
	case 0x69:
		binge_wall_hole(ev, 0);
		break;
	case 0x6a:
		dng16_water_basin(ev);
		break;
	case 0x6b:
		dng16_lava_wall(ev);
		break;
	case 0x6c:
		break;
	case 0x6d:
		dng16_golem_forge(ev);
		break;
	case 0x6e:
		dng16_exit(ev);
		break;
	case 0x6f:
		dng16_rubble(ev);
		break;
	case 0x70:
		dng16_stone_door(ev);
		break;
	default:
		break;
	}
	return ret;
}

signed short dng16_door(struct area_events far *ev, signed short pos, signed short x, signed short y)
{
	signed short ret = 0;
	register signed short id = -1;
	register signed short pic = 0x33;
	struct lock_buf lk;

	switch (pos) {
	case 0x902:
		id = 0x9f;
		pic = 0x34;
		break;
	case 0xe04:
		id = 0xa0;
		break;
	case 0xc07:
		id = 0xa1;
		break;
	case 0x1008:
		id = 0xa2;
		pic = 0x34;
		break;
	case 0xf09:
		id = 0xa3;
		break;
	case 0x110f:
		id = 0xa4;
		pic = 0x34;
		break;
	case 0xd02:
		id = 0xa5;
		pic = 0x35;
		break;
	case 0xd03:
		id = 0xa6;
		pic = 2;
		break;
	case -1:
		id = 0xa7;
		pic = 2;
	default:
		break;
	}
	if (id != -1) {
		if (id == 0xa5)
			dng16_double_key(ev, 1);
		else if (id == 0xa6)
			dng16_double_key(ev, 0);
		else if (id == 0xa7)
			g_lock_cell_bits = 0x2000;
		else {
			Lock_ctor(&lk, id);
			Lock_dtor(&lk, 2);
		}
		ret = 1;
		if (g_lock_cell_bits != -1) {
			event_set_cell_bits(ev, x, y, g_lock_cell_bits);
			snd_play(g_snd_fx, 0x61);
			((void (*)(void far *, signed short, signed short, signed short, signed short))automap_cell_write)
				(g_map, 0x312, y - g_map_x0, x - g_map_y0, pic);
		}
	}
	return ret;
}

void dng16_stone_plate(struct area_events far *ev)
{
	signed short old;
	struct hero far *h1;
	struct hero far *h2;
	register signed short t;
	register signed short flag;

	old = dlg_set16(g_dialog, 5);
	h1 = get_first_active_hero(g_group);
	h2 = get_second_active_hero(g_group);
	flag = 0;
	if (!g_dng16_plate_found) {
		struct hero far *h;

		h = get_first_hero(g_group);
		do {
			if (check_hero3(h) && test_skill(h, TA_SINNESSCHAERFE, 5) > 0) {
				timewarp_minutes(g_date, 5L);
				g_dng16_plate_found = 1;
				break;
			}
			h = get_next_hero(g_group, h);
		} while (get_first_hero(g_group) != h);
		if (g_dng16_plate_found) {
			/* BINGE7.LTX[2]: "%s beginnt ploetzlich im Schutt herumzufingern und legt nach kurzer Zeit eine Steinplatte mit
			 * zwei rostigen Eisengriffen frei. Wollt Ihr die Platte beiseite heben?" */
			sprintf(g_text_output_buf, ev->txt[2], h->name);
			if (dlg_yes_no(g_dialog, g_text_output_buf) == 1)
				flag = 1;
		}
	}
	/* BINGE7.LTX[3]: "Im Boden ist eine Steinplatte mit zwei rostigen Eisengriffen eingelassen. Wollt Ihr die Platte
	 * beiseite heben?" */
	if (g_dng16_plate_found && !g_dng16_plate_lifted && (flag || dlg_yes_no(g_dialog, ev->txt[3]) == 1)) {
		if (h2 == 0) {
			/* BINGE7.LTX[4]: "Die Platte ist einfach zu schwer. %s hat alleine keine Chance." */
			sprintf(g_text_output_buf, ev->txt[4], h1->name);
			dlg_message(g_dialog, g_text_output_buf);
		} else {
			t = random_schweif(5) - 1;
			t += (signed char)(h1->attrib[ATTRIB_KK].cur + h1->attrib[ATTRIB_KK].mod);
			t += (signed char)(h2->attrib[ATTRIB_KK].cur + h2->attrib[ATTRIB_KK].mod);
			if (t < 0x20) {
				/* BINGE7.LTX[5]: "%s und %s muehen sich mit der Platte ab, aber das Ding ist einfach zu schwer." */
				sprintf(g_text_output_buf, ev->txt[5], h1->name, h2->name);
			} else {
				/* BINGE7.LTX[6]: "Die Platte ist sehr schwer, aber schliesslich gelingt es %s und %s, sie beiseite zu
				 * schieben. Darunter befindet sich eine kleine Grube von etwa einem Schritt Tiefe. Auf dem Boden liegt eine
				 * Schatulle." */
				sprintf(g_text_output_buf, ev->txt[6], h1->name, h2->name);
				g_dng16_plate_lifted = 1;
			}
			dlg_message(g_dialog, g_text_output_buf);
			timewarp_minutes(g_date, 5L);
		}
	}
	if (g_dng16_plate_lifted) {
		/* BINGE7.LTX[7]: "Wollt Ihr die Schatulle bergen?" */
		if (!g_dng16_casket_taken && !g_dng16_pit_trap && dlg_yes_no(g_dialog, ev->txt[7]) == 1) {
			if (!g_dng16_casket_taken) {
				if (test_attrib(h1, ATTRIB_GE, 8) > 0) {
					/* BINGE7.LTX[8]: "%s haengt sich in die Grube, wobei %s %s an den Beinen festhaelt. Auf diese Weise gelingt
					 * es den beiden, die Schatulle zu bergen. In der Schatulle liegt ein zierliches Messer aus dunkelrot
					 * schimmerndem Metall, ausserdem fuenf gepraegte Muenzen aus dem gleichen Material." */
					sprintf(g_text_output_buf, ev->txt[8], h1->name, h2->name, grammar_pronoun(h1->sex, 2, 0));
					dlg_message(g_dialog, g_text_output_buf);
					add_hero_ap(h1, 10);
					add_hero_ap(h2, 10);
					g_dng16_casket_taken = 1;
				} else {
					/* BINGE7.LTX[9]: "%s haengt sich in die Grube, wobei %s %s an den Beinen festhaelt. Als %s an die Grubenwand
					 * stoesst, schnellen ploetzlich zwei Steinbloecke aus den Seitenwaenden und klemmen %s Arme ein. Unter
					 * starken Schmerzen gelingt es %s, die Arme wieder zu befreien. Aber selbst mit groesster Kraftanstrengung
					 * lassen sich die Bloecke nicht mehr zurueckbewegen. Die Schatulle muss wohl bleiben, wo sie ist." */
					sprintf(g_text_output_buf, ev->txt[9], h1->name, h2->name, grammar_pronoun(h1->sex, 2, 0),
						grammar_pronoun(h1->sex, 0, 0), grammar_pronoun(h1->sex, 1, 1), grammar_pronoun(h1->sex, 3, 0));
					dlg_message(g_dialog, g_text_output_buf);
					sub_hero_le(h1, random_schweif(4) + 4);
					add_hero_ap(h1, 5);
					g_dng16_pit_trap = 1;
				}
			}
			timewarp_minutes(g_date, 10L);
		}
		if (g_dng16_casket_taken) {
			void far *o;

			o = Stash_ctor(0, 0x38, 0);
			Stash_dtor(o, 3);
		}
	}
	dlg_set16(g_dialog, old);
}

void dng16_hot_plates(struct area_events far *ev, register signed short row)
{
	signed short old;
	struct hero far *h;

	old = dlg_set16(g_dialog, 5);
	if (row == 13) {
		if (g_god_favor_ingerimm < -2000L)
			g_dng16_sacrilege = 1;
		if (g_dng16_sacrilege) {
			/* BINGE7.LTX[10]: "Die Platten sind unglaublich heiss - noch bevor Ihr mitbekommt, was los ist, steht Eure
			 * Kleidung in Flammen. Um Euch herum entsteht ein Flammenmeer, das alles verschlingt, dann wird es Euch schwarz
			 * vor den Augen." */
			dlg_message(g_dialog, ev->txt[10]);
			group_kill_all(g_group);
		} else {
			h = get_first_hero(g_group);
			do {
				if ((get_item_pos(h, ITEM_LEATHER_TROUSERS) == -1 || get_item_pos(h, ITEM_LEATHER_TROUSERS) >= 15 ||
				     get_item_pos(h, ITEM_LEATHER_BOOTS) == -1 || get_item_pos(h, ITEM_LEATHER_BOOTS) >= 15 ||
				     get_item_pos(h, ITEM_LEATHER_JERKIN) == -1 || get_item_pos(h, ITEM_LEATHER_JERKIN) >= 15) &&
				    get_item_pos(h, ITEM_ASTHENIL_RING) == -1)
					sub_hero_le(h, h->le);
				h = get_next_hero(g_group, h);
			} while (get_first_hero(g_group) != h);
		}
	} else if ((row == 12 && g_dng16_plates_dir == 1) || (row == 14 && g_dng16_plates_dir == 0))
		dng16_plates_turn_back(ev, g_dng16_plates_dir);
	g_dng16_plate_steps++;
	dlg_set16(g_dialog, old);
}

void dng16_plates_turn_back(struct area_events far *ev, signed short flag)
{
	signed short n = 9;
	signed short old;
	struct hero far *h;
	struct group far *g;
	register signed short i;
	register signed short cnt;

	old = dlg_set16(g_dialog, 5);
	h = get_first_hero(g_group);
	n -= g_dng16_plate_steps;
	if (flag == 0)
		/* BINGE7.LTX[11]: "Die Platten auf dem Boden gluehen schwach, als Ihr Euch im Korridor vorwaertsbewegt." */
		dlg_message(g_dialog, ev->txt[11]);
	else {
		/* BINGE7.LTX[12]: "Die Platten auf dem Boden leuchten in gleissendem orange auf, als Ihr Euch im Korridor
		 * vorwaertsbewegt." */
		dlg_message(g_dialog, ev->txt[12]);
		n += 8;
	}
	do {
		if (get_item_pos(h, ITEM_STONE_MEDALLION) < 0 && get_item_pos(h, ITEM_ASTHENIL_RING) < 0 && check_hero3(h) && test_attrib(h, ATTRIB_MU, n) <= 0) {
			cnt = group_count_check2b(g_group);
			if (group_get_npc(g_group))
				cnt--;
			/* BINGE7.LTX[13]: "%s macht lieber kehrt." */
			sprintf(g_text_output_buf, ev->txt[13], h->name);
			dlg_message(g_dialog, g_text_output_buf);
			if (cnt > 1)
				make_group(1, h);
			for (i = 0, g = g_groups; i < 6; i++, g = &g_groups[i]) {
				if (group_has_hero(g, h))
					break;
			}
#if FIX_0164
			/* Fix #164: nur in eine gefundene Gruppe schreiben (nicht g_groups[6]) */
			if (i < 6) {
				g->screen_y = 0x78c;
				g->old_y = g->y;
				g->y = 11;
				if (g == g_group)
					cam_set_from_group();
			}
#else
			g->screen_y = 0x78c;
			g->old_y = g->y;
			g->y = 11;
			if (g == g_group)
				cam_set_from_group();
#endif
			break;
		}
		h = get_next_hero(g_group, h);
	} while (get_first_hero(g_group) != h);
	dlg_set16(g_dialog, old);
}

void dng16_walled_passage(struct area_events far *ev)
{
	signed short old;

	old = dlg_set16(g_dialog, 5);
	/* BINGE7.LTX[14]: "An dieser Stelle war frueher wohl mal ein Durchgang, der jetzt allerdings zugemauert ist. Ihr
	 * schaut Euch das betreffende Wandstueck genauer an und klopft auch probeweise ein paar mal dagegen." */
	dlg_message(g_dialog, ev->txt[14]);
	/* BINGE7.LTX[15]: "Hier wurde ganze Arbeit geleistet. Das Wandstueck ist mehr als massiv und mit Euren Mitteln
	 * habt Ihr keine Chance, hier durchzukommen." */
	dlg_message(g_dialog, ev->txt[15]);
	dlg_set16(g_dialog, old);
}

signed short dng16_double_key(struct area_events far *ev, signed short flag)
{
	signed short old;
	signed short ret;
	struct hero far *h;
	register signed short idx;
	struct lock_buf lk;

	old = dlg_set16(g_dialog, 5);
	idx = group_find_key(g_group, ITEM_DOUBLE_KEY);
	ret = 0;
	h = 0;
	if (idx != -1)
		h = g_group->heroes[idx];
	if (!check_hero3(h) || h == 0)
		h = get_first_active_hero(g_group);
	if (group_find_key(g_group, ITEM_DOUBLE_KEY) != -1) {
		if (!g_dng16_double_key_state) {
			/* BINGE7.LTX[16]: "Nach einer kurzen Untersuchung steckt %s den Doppelbartschluessel in das Schloss und dreht
			 * ihn um." */
			sprintf(g_text_output_buf, ev->txt[16], h->name);
			dlg_message(g_dialog, g_text_output_buf);
		}
#if FIX_0164
		/* Fix #164: Rueckgabe 1, wenn der Schluessel benutzt wurde */
		g_lock_cell_bits = 0x2000;
		ret = 1;
		if (flag)
#else
		g_lock_cell_bits = 0x2000;
		if (flag)
#endif
			g_dng16_double_key_state = 1;
		else
			g_dng16_double_key_state = 2;
	} else {
		Lock_ctor(&lk, 0xa5);
		Lock_dtor(&lk, 2);
	}
	dlg_set16(g_dialog, old);
	return ret;
}

void dng16_leather_chest(struct area_events far *ev)
{
	signed short r = 0;
	void far *o;
	signed short k;
	signed short slot;
	struct hero far *h;
	register signed short item;
	register signed short i;

	o = Stash_ctor(0, 0x39, 1);
	if ((group_find_item(g_group, ITEM_LEATHER_TROUSERS) != -1 || group_find_item(g_group, ITEM_LEATHER_BOOTS) != -1 || group_find_item(g_group, ITEM_LEATHER_JERKIN) != -1) &&
	    /* BINGE7.LTX[18]: "Wollt Ihr das Lederzeug zuruecklegen?" */
	    (r = dlg_yes_no(g_dialog, ev->txt[18])) == 1) {
		for (i = 0; i < 3; i++) {
			switch (i) {
			case 0:
				item = ITEM_LEATHER_TROUSERS;
				break;
			case 1:
				item = ITEM_LEATHER_BOOTS;
				break;
			case 2:
				item = ITEM_LEATHER_JERKIN;
				break;
			}
			while ((slot = group_find_item(g_group, item)) != -1) {
				h = g_group->heroes[slot];
				if ((k = get_item_pos(h, item)) != -1) {
					drop_item(h, k, 1);
					stash_add(o, i, item, 1);
				}
			}
		}
		if (stash_is(o, 0, ITEM_LEATHER_TROUSERS, 7) && stash_is(o, 1, ITEM_LEATHER_BOOTS, 7) && stash_is(o, 2, ITEM_LEATHER_JERKIN, 7) && g_dng16_leather_taken) {
			event_set_cell(ev, 2, 0x0f, 5, 0, -1);
			automap_cell_write(g_map, 0x312, 0x0f, 2, 0x0e);
			g_dng16_leather_taken = 0;
			/* BINGE7.LTX[73]: "Als Ihr die Kleidung wieder in die Truhe gelegt habt, koennt Ihr aus der Ferne ein lautes,
			 * mahlendes Geraeusch vernehmen." */
			dlg_message(g_dialog, ev->txt[0x49]);
		}
	}
	if (r == 0 && stash_has_items(o)) {
		if (!g_dng16_sign_read) {
			/* BINGE7.LTX[17]: "Die Truhe ist unverschlossen. Auf der Frontseite ist eine Tafel angebracht:@ <Nehmt des
			 * Gottes, solange Ihr dessen beduerft, doch bedient Euch seiner nicht laenger, als die Not gebietet!>" */
			dlg_message(g_dialog, ev->txt[17]);
			g_dng16_sign_read = 1;
		}
		if (stash_show(o) && !g_dng16_leather_taken) {
			event_set_cell(ev, 2, 0x0f, 2, 0, -1);
			automap_cell_write(g_map, 0x312, 0x0f, 2, 0x0b);
			g_dng16_leather_taken = 1;
			/* BINGE7.LTX[74]: "Als Ihr die Kleidung aus der Truhe genommen habt, hoert Ihr ploetzlich aus der Ferne einen
			 * donnernden Knall." */
			dlg_message(g_dialog, ev->txt[0x4a]);
		}
	}
	Stash_dtor(o, 3);
}

void dng16_treasure_chest(struct area_events far *ev)
{
	if (!g_dng16_chest_looted) {
		signed short old;
		register signed short r;
		register signed short n;

		old = dlg_set16(g_dialog, 5);
		/* BINGE7.LTX[19]: "Die Truhe ist fast bis zum Rand mit Schmuck, Edelsteinen und Muenzen gefuellt. Der Inhalt ist
		 * sicher mehr wert als alles, was Ihr je gesehen habt. @Wollt Ihr" */
		/* BINGE7.LTX[20]: "die Truhe pluendern?" */
		/* BINGE7.LTX[21]: "oder ein paar Muenzen dazu legen?" */
		r = dlg_choose(g_dialog, ev->txt[19], 2, ev->txt[20], ev->txt[21]);
		if (r == 1) {
			void far *o;

			o = Stash_ctor(0, 0x3a, 1);
			if (stash_show(o) == 1) {
				if (!stash_has_items(o))
					g_dng16_chest_looted = 1;
				g_dng16_sacrilege = 1;
				if (g_dng16_chest_coins) {
					money_to_string(g_text_buf2, g_dng16_chest_coins);
					/* BINGE7.LTX[78]: "Ihr findet noch %s." */
					sprintf(g_text_output_buf, ev->txt[0x4e], g_text_buf2);
					dlg_message(g_dialog, g_text_output_buf);
					group_add_money(g_group, g_dng16_chest_coins);
				}
			}
			Stash_dtor(o, 3);
		} else if (r == 2) {
			signed long m;

			m = g_group->money;
			if (m == 0)
				/* BINGE7.LTX[1]: "Ihr habt doch gar kein Geld mehr!" */
				dlg_message(g_dialog, ev->txt[1]);
			else {
				n = m / 10;
				if (n > 9999)
					n = 9999;
				/* BINGE7.LTX[22]: "Wieviele Silbertaler wollt Ihr in die Truhe legen?" */
				m = dlg_input(g_dialog, 4, ev->txt[22], 0, n, n < 100 ? n : 100);
				if (m > 0) {
					g_god_favor_ingerimm += m;
					m *= 10;
					g_dng16_chest_coins += m;
					group_sub_money(g_group, m);
					snd_play(g_snd_fx, 0x47);
				}
			}
		} else
			g_god_favor_ingerimm -= 100;
		dlg_set16(g_dialog, old);
		return;
	}
	{
		void far *o;

		o = Stash_ctor(0, 0x3a, 0);
		Stash_dtor(o, 3);
	}
}

void dng16_salamander_chest(struct area_events far *ev)
{
	if (!g_dng16_stone_taken && !g_dng16_artifact_traded) {
		signed short old;
		void far *o;
		struct hero far *h;
		register signed short r;
		register signed short k;

		old = dlg_set16(g_dialog, 5);
		o = Stash_ctor(0, 0x3b, 1);
		if (!g_dng16_golem_fought) {
			/* BINGE7.LTX[23]: "In der Truhe liegt ein ikosaederfoermiges, faustgrosses Stueck Stein, das von feinen gelben
			 * und roten Erzadern durchzogen und in silbernes Metall gefasst ist.@Wollt Ihr den Stein nehmen?" */
			if (dlg_yes_no(g_dialog, ev->txt[23]) == 1) {
				/* BINGE7.LTX[24]: "<Das ist der Salamanderstein.@Schoenes Stueck, nicht wahr?!>,@toent es von nebenan,@<Ihr
				 * koennt es gegen Euer Artefakt tauschen, wenn Ihr wollt.> @Wollt Ihr" */
				/* BINGE7.LTX[25]: "tauschen?" */
				/* BINGE7.LTX[26]: "oder den Stein einfach mitnehmen?" */
				r = dlg_choose(g_dialog, ev->txt[24], 2, ev->txt[25], ev->txt[26]);
#if FIX_0357
				/* Fix #357: ESC bei der Frage tauschen/mitnehmen bricht ab, statt den Stein ohne Folgen nehmen zu lassen */
				if (r != -1 && stash_show(o) == 1) {
#else
				if (stash_show(o) == 1) {
#endif
					if (r == 1) {
						k = group_find_item(g_group, ITEM_ASTHENIL_SWORD);
						if (k != -1) {
							h = g_group->heroes[k];
							k = get_item_pos(h, ITEM_ASTHENIL_SWORD);
							if (k != -1)
								drop_item(h, k, 1);
						}
						g_dng16_artifact_traded = 1;
					} else if (r == 2) {
						/* BINGE7.LTX[27]: "<Ihr solltet nur nehmen, was das eurige ist>, sagt der Golem aus dem Hintergrund, trifft
						 * aber keine Anstalten, Euren Frevel zu verhindern." */
						dlg_message(g_dialog, ev->txt[27]);
						g_dng16_sacrilege = 1;
					}
					g_dng16_stone_taken = 1;
					/* BINGE7.LTX[75]: "Wir haben den Salamanderstein endlich gefunden - tief im Inneren des Finsterkopps. Nun
					 * gilt es, wieder heil aus dieser Binge herauszukommen..." */
					sprintf(g_text_output_buf, ev->txt[0x4b]);
					diary_add(g_diary, g_text_output_buf, 0);
				}
			}
		} else {
			if (stash_show(o) == 1)
				g_dng16_stone_taken = 1;
		}
		dlg_set16(g_dialog, old);
		Stash_dtor(o, 3);
		return;
	}
	if (g_dng16_artifact_traded && g_dng16_sword_done) {
		void far *o;

		o = Stash_ctor(0, 0x3e, 1);
		if (stash_show(o) == 1)
			g_dng16_sacrilege = 1;
		Stash_dtor(o, 3);
		return;
	}
	/* BINGE7.LTX[79]: "Hier gibt es nichts mehr zu holen." */
	dlg_message(g_dialog, ev->txt[0x4f]);
}

void dng16_exit(struct area_events far *ev)
{
	signed short old;

	old = dlg_set16(g_dialog, 5);
	/* BINGE7.LTX[28]: "Hinter Euch stuerzt mit lautem Getoese der Zugang zur Hoehle ein." */
	dlg_message(g_dialog, ev->txt[0x1c]);
	g_finsterkopp_entered = 2;
	snd_obj_stop(g_snd_music);
	swap_out_screen();
	g_group->route_pos = 0;
	g_group->old_dungeon = g_group->dungeon;
	g_group->dungeon = 0;
	g_group->old_town = g_group->town;
	g_group->town = 0;
	group_set_on_route(g_group, 1);
	g_group->route_from = 0x6a;
	g_group->route_to = 0x6a;
	dlg_set16(g_dialog, old);
}

void dng16_rubble(struct area_events far *ev)
{
	signed short old;

	old = dlg_set16(g_dialog, 5);
	/* BINGE7.LTX[29]: "Mit Euren Mitteln habt Ihr keine Chance, hier durchzukommen." */
	dlg_message(g_dialog, ev->txt[0x1d]);
	dlg_set16(g_dialog, old);
}
