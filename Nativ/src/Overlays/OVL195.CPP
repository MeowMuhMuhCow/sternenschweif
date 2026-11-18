/* OVL195 - Overlay O195 (Segment 0x2342 in SCHWEIF.EXE): Ereignisse des Dungeons 4, des Altsvellt-Sumpfes
 * (Texte NATURE.LTX = Datei 0x2ca; g_area_descs[4]); Ereignisobjekt ev (struct area_events, Texte ev->txt),
 * Kartenereignis mev.
 * dng04_handler(ev, mev): pos = (Zeile - g_map_x0) << 8 | (Spalte - g_map_y0), h = erster handlungsfaehiger Held.
 *   MAP_EVENT_ENTER (je eine if-else-Kette, danach immer tiefes Wasser pruefen): 0x281d/0x271c Felder (0x1e/0x1d,
 *   0x29) einmalig auf Typ 4; Kraeuter (Pflanzenkunde: 5 Einbeeren an 0x2d1d, Belmart an 0x1409 mit Probe +5, Gulmond
 *   an 0x20f; NATURE[0]/[1]/[6]); Schlangen an 0x0c19/0x1817/0x211a/0x2419 (dng04_snake); 0x2b1b Truhe (Chest 7);
 *   0xf10 Echsendorf (Bild 0x2e5/4, NATURE[7], Gespraech SWAMP.TLK 0x12a mit den Echsen; nach dem Tod des Untiers
 *   nochmals); 0x11d Kraeutergarten (Stash 3, NATURE[8]) und 0xb1e Karren (Stash 2, NATURE[17]); 0x116 Hecke
 *   durchhauen (Sinnesschaerfe +5, NATURE[18], 1 Stunde); 0xb04 zwanzig Heidekrautstauden (nur bei g_eolan_state == 2 und
 *   g_pic2_seen == 1; NATURE[9..16]: ansehen oder ausrupfen, Nummer 0..20 - die achtzehnte ist besonders, mit Sumpfrantze
 *   jault sie auf); Sumpfrantzen-Kaempfe (Gegner 0x51, Anzahl dng04_take_from_pool aus g_dng04_rantzen_left);
 *   einmalige Kaempfe (Merker g_dng04_<gegner>_<pos>); 0x1c10/0x1c11 das Untier (0x72), wenn das Dorf besucht ist;
 *   0x1e08/0x1d08 Kampf 0x9b (bei g_eolan_state == 2). Feldtyp 10 -> dng04_deep_water. Typ 0x64: Leiche im Wasser
 *   (NATURE[2]); zu 50 % stuerzt der Totenaengstlichste in den Sumpf (NATURE[3], W6 LE, zu 20 % geht die linke Hand
 *   verloren). 0x65 Truhe (Chest 7). 0x66 Kampf 0x73. 0x67 Huette der Alten (Gespraech 0x12a, Partner 1). 0x68 Turm
 *   des Zauberers: offen nach dem Auftrag der Alten (g_dng04_tower_open) -> Kampf FIGHT_ID_SPECIAL2, danach
 *   "abgebrannt" (NATURE[20]); sonst magisch verschlossen (NATURE[23]). 0x69 Ausgaenge (NATURE[21]): je Position Ort
 *   der Reisekarte 0x2c..0x86 -> Musik aus, Dungeon 0, unterwegs (group_set_on_route, route_from/route_to); Ablehnung
 *   dreht die Gruppe um.
 * dng04_snake(ev, flag): beim ersten Betreten sicher, sonst zu 15 %; Sinnesschaerfe +3 -> Kvillotter erschlagen
 *   (NATURE[4]), sonst Biss (NATURE[5]: Gift 7 oder W4 LE).
 * dng04_take_from_pool(ev, n, pool): min(n, *pool) abziehen und liefern.
 * dng04_deep_water: misslingt jemandem die Schwimmen-Probe, droht dem schlechtesten Schwimmer (ohne In See und Fluss)
 *   W20 LE (GLOBLTXT[443] bzw. [442] ertrunken; g_in_fight = 1 waehrend sub_hero_le, g_hero_died = 1).
 *
 * BCC -mlarge -3 -Ot -c -Y -E..\bc\BIN\TASM.EXE OVL195.CPP -> 4581/4581 Bytes GLEICH (18.10.2026), cobjfix laeuft
 *   (-Og/-Oi aendern nichts; -2 falsch). (Namensfassung mit schweif.h: 19.10.2026 GLEICH, LINKGLEICH)
 * - C++ wegen Bmp/Nvf/Stash/Chest und inline-Settern der Gruppe (set_dungeon/set_route_from/set_route_to/set_dir ->
 *   Parameterkopien [bp-0ah..0eh]); "g_group->set_dungeon(g_group->dungeon = 0)" speichert 0 ins Feld UND in die
 *   Kopie.
 * - automap_cell_write ueber AUTOMAP_CELL_WRITE_W (letzter Parameter short, wie OVL063).
 * - Das Chest-Objekt (0x55 Bytes) ist ein Block-Local der beiden Faelle, liegt aber bei [bp-64h] HINTER allen
 *   Block-Locals der Geschwisterbloecke (tiefster Platz [bp-0eh]) - grosse Objekte innerer Bloecke teilen den Platz
 *   nicht mit kleinen Block-Locals.
 * - pos in zwei Schritten ("pos = (..) << 8; pos |= ..") wie in O066; x ([bp-6]) auf Funktionsebene, die Liste, das
 *   Bild und n (0xb04) als Block-Locals ([bp-0ch] bzw. [bp-0ah] geteilt).
 * - dng04_take_from_pool (ohne Aufrufe): "if ((v = c = *cnt) < n)" mit register-Parameter n (DX), r ohne register
 *   (CX), v als gewoehnliches Local ([bp-4]); mit register r tauschen CX/DX, ohne v laedt BCC c neu.
 * - dng04_deep_water: "lp = h->le; if (lp - dmg > 0)" (Flags von sub); Bitfeld see_und_fluss +be.1 (shr / and 1 /
 *   or).
 * Originalfehler: Typ 0x69 setzt erst "old_dungeon = dungeon" und danach ueber set_dungeon nochmals old_dungeon =
 *   dungeon (inzwischen 0) -> der vorige Dungeon wird mit 0 ueberschrieben (die erste Zuweisung ist wirkungslos).
 *   dng04_snake setzt *flag doppelt; dng04_take_from_pool benutzt ev nicht.
 */
#include <stdio.h>

#include "schweif.h"

/* automap_cell_write mit short als letztem Parameter (wie im Original deklariert) */
#define AUTOMAP_CELL_WRITE_W \
	((void (*)(void far *, signed short, signed short, signed short, signed short))automap_cell_write)

signed short dng04_handler(struct area_events far *ev, struct map_event far *mev)
{
	register signed short type;
	register signed short pos;
	struct hero far *h;
	signed short x;
	signed short ret;

	type = mev->type;
	pos = (mev->y - g_map_x0) << 8;
	pos |= mev->x - g_map_y0;
	h = get_first_active_hero(g_group);
	ret = 0;

	if (type == MAP_EVENT_ENTER) {
		if (!g_dng04_path_281d && pos == 0x281d) {
			event_set_cell(ev, 0x1e, 0x29, 4, 0, -1);
			AUTOMAP_CELL_WRITE_W(g_map, 0x26e, 0x29, 0x1e, 0x0f);
			g_dng04_path_281d = 1;
		} else if (!g_dng04_path_271c && pos == 0x271c) {
			event_set_cell(ev, 0x1d, 0x29, 4, 0, -1);
			AUTOMAP_CELL_WRITE_W(g_map, 0x26e, 0x29, 0x1d, 0x0f);
			g_dng04_path_271c = 1;
		} else if (!g_dng04_einbeeren_found && pos == 0x2d1d) {
			h = get_best_hero_skill(g_group, TA_PFLANZENKUNDE);
			/* NATURE.LTX[0]: "%s entdeckt 5^Einbeeren." */
			sprintf(g_text_output_buf, ev->txt[0], h->name);
			dlg_message(g_dialog, g_text_output_buf);
			group_give_item(g_group, ITEM_EINBEERE, 5);
			g_dng04_einbeeren_found = 1;
			ret = 1;
		} else if (!g_dng04_belmart_found && pos == 0x1409) {
			if (test_skill(h, TA_PFLANZENKUNDE, 5) >= 0) {
				/* NATURE.LTX[1]: "%s findet eine Portion Belmart." */
				sprintf(g_text_output_buf, ev->txt[1], h->name);
				dlg_message(g_dialog, g_text_output_buf);
				group_give_item(g_group, ITEM_BELMART, 1);
				g_dng04_belmart_found = 1;
				ret = 1;
			}
		} else if (pos == 0x0c19) {
			dng04_snake(ev, &g_dng04_snake_0c19);
			ret = 1;
		} else if (pos == 0x1817) {
			dng04_snake(ev, &g_dng04_snake_1817);
			ret = 1;
		} else if (pos == 0x211a) {
			dng04_snake(ev, &g_dng04_snake_211a);
			ret = 1;
		} else if (pos == 0x2419) {
			dng04_snake(ev, &g_dng04_snake_2419);
			ret = 1;
		} else if (pos == 0x2b1b) {
			Chest c(7, 1);
			ret = 1;
		} else if (!g_dng04_gulmond_found && pos == 0x020f) {
			/* NATURE.LTX[6]: "Ihr entdeckt eine Gulmond-Pflanze. Ohne groessere Muehe koennt Ihr die Knolle herausziehen
			 * und mitnehmen." */
			dlg_message(g_dialog, ev->txt[6]);
			group_give_item(g_group, ITEM_GULMOND_LEAF, 1);
			g_dng04_gulmond_found = 1;
			ret = 1;
		} else if (pos == 0x0f10) {
			if (!g_dng04_village_visited) {
				Bmp far *bmp;
				signed short d;
				bmp = new Bmp(0x40, 0x40);
				x = dlg_set16(g_dialog, 5);
				nvfile_load(g_nvf_heads,0x2e5, bmp->data(), 0, 4, 0, &d, &d, 0x140);
				/* NATURE.LTX[7]: "Als Ihr zwischen die seltsam aussehenden Bauten tretet, stehen Euch etwa zwei Dutzend
				 * seltsamer Wesen gegenueber. Aufrecht auf den Hinterbeinen stehend, sehen sie ansonsten wie Salamander aus,
				 * nur dass Salamander keine dreizackigen Speere in den Haenden zu halten pflegen. Mit vorsichtigen Schritten
				 * kommt das vorderste der Echsenwesen auf Euch zu und beginnt mit zischelnder und kaum verstaendlicher Stimme
				 * zu reden." */
				dlg_box_bmp(g_dialog, bmp, (char far *)0, ev->txt[7], 0);
				delete bmp;
				talk(&g_talk, 0x12a, 0, 0);
				g_dng04_village_visited = 1;
				dlg_set16(g_dialog, x);
				ret = 1;
			} else if (g_dng04_monster_killed && !g_dng04_village_thanked) {
				talk(&g_talk, 0x12a, 0, 0);
				ret = g_dng04_village_thanked = 1;
			}
		} else if (pos == 0x011d) {
			Stash far *l = new Stash(3, 1);
			if (l->has_items() == 1) {
				h = get_best_hero_skill(g_group, TA_PFLANZENKUNDE);
				/* NATURE.LTX[8]: "Ihr entdeckt einen phantastisch gepflegten Kraeutergarten!@<Seht Euch das an>, meint %s und
				 * inspiziert die Fuelle der Gewaechse." */
				sprintf(g_text_output_buf, ev->txt[8], h->name);
				dlg_message(g_dialog, g_text_output_buf);
				l->show();
				ret = 1;
			}
			delete l;
		} else if (pos == 0x0b1e) {
			Stash far *l = new Stash(2, 1);
			if (l->has_items() == 1) {
				/* NATURE.LTX[17]: "Ihr entdeckt einen Karren, der erst wenige Tage hier zu stehen scheint; von den Besitzern
				 * sind nur wenige Knochen zu entdecken. Nichts deutet darauf hin, was sich hier abgespielt haben koennte." */
				dlg_message(g_dialog, ev->txt[17]);
				l->show();
				ret = 1;
			}
			delete l;
		} else if (pos == 0x0116 && !g_dng04_hedge_cut) {
			if (test_skill(h, TA_SINNESSCHAERFE, 5) >= 0) {
				/* NATURE.LTX[18]: "<Das Gestruepp scheint an dieser Stelle etwas aermer an Unterholz zu sein. Vielleicht
				 * koennte man sich hier einen Weg durchhauen.> meint %s und deutet auf eine Hecke. Sofort macht Ihr Euch an
				 * die Arbeit und tatsaechlich, bereits nach einer Stunde habt Ihr einen Durchgang geschaffen." */
				sprintf(g_text_output_buf, ev->txt[18], h->name);
				dlg_message(g_dialog, g_text_output_buf);
				event_set_cell(ev, mev->x + 1 - g_map_y0, mev->y - g_map_x0, 2, 0, -1);
				AUTOMAP_CELL_WRITE_W(g_map, 0x26e, 1, 0x17, 0x0a);
				g_dng04_hedge_cut = 1;
				timewarp_hours(g_date, 1L);
				ret = 1;
			}
		} else if (pos == 0x0b04) {
			if (g_eolan_state == 2 && g_pic2_seen == 1) {
				signed short n;
				/* NATURE.LTX[9]: "Ihr entdeckt zwanzig Pflanzen, die irgendwie an Heidekraut erinnern, in Reih und Glied am
				 * Wasser entlang stehen.@Wollt Ihr" */
				/* NATURE.LTX[10]: "Euch eine bestimmte ansehen?" */
				/* NATURE.LTX[11]: "einfach eine herausrupfen, um sie naeher zu untersuchen?" */
				/* NATURE.LTX[12]: "Das Kraut Kraut sein lassen?" */
				if ((x = dlg_choose(g_dialog, ev->txt[9], 3, ev->txt[10], ev->txt[11],
					ev->txt[12])) != -1 &&
					(x == 1 || x == 2)) {
					/* NATURE.LTX[13]: "Welche?" */
					n = dlg_input(g_dialog, 2, ev->txt[13], 0, 0x14, 0);
					if (x == 1)
						/* NATURE.LTX[14]: "Es ist ein ausgesprochen kraeftiges Kraut." */
						dlg_message(g_dialog, ev->txt[14]);
					else {
						group_give_item(g_group, n == 0x12 ? 0x14e : 0x14f, 1);
						/* NATURE.LTX[15]: "Ihr entdeckt nichts besonderes, nur, dass es sich um sehr kraeftige Stauden handelt." */
						dlg_message(g_dialog, ev->txt[15]);
						if (n == 0x12 && group_find_item(g_group, ITEM_SWAMP_RANTZE) != -1)
							/* NATURE.LTX[16]: "Ploetzlich jault die Rantze, die Ihr herumtragt, einmal laut auf und erschlafft." */
							dlg_message(g_dialog, ev->txt[16]);
					}
				}
				ret = 1;
			}
		} else if (!g_dng04_rantzen_271b && pos == 0x271b) {
			if ((x = dng04_take_from_pool(ev, 6, &g_dng04_rantzen_left)) > 0) {
				if (event_fight1(ev, 0x51, x, 1) == 0)
					g_dng04_rantzen_271b = 1;
			}
		} else if (!g_dng04_rantzen_1413 && pos == 0x1413) {
			if ((x = dng04_take_from_pool(ev, 6, &g_dng04_rantzen_left)) > 0) {
				if (event_fight1(ev, 0x51, x, 0) == 0)
					g_dng04_rantzen_1413 = 1;
			}
		} else if (!g_dng04_rantzen_2701 && pos == 0x2701) {
			if ((x = dng04_take_from_pool(ev, 6, &g_dng04_rantzen_left)) > 0) {
				if (event_fight1(ev, 0x51, x, 1) == 0)
					g_dng04_rantzen_2701 = 1;
			}
		} else if (!g_dng04_rantzen_0405 && pos == 0x0405) {
			if ((x = dng04_take_from_pool(ev, 6, &g_dng04_rantzen_left)) > 0) {
				if (event_fight1(ev, 0x51, x, 0) == 0)
					g_dng04_rantzen_0405 = 1;
			}
		} else if (!g_dng04_rantzen_0e16 && pos == 0x0e16) {
			if ((x = dng04_take_from_pool(ev, 6, &g_dng04_rantzen_left)) > 0) {
				if (event_fight1(ev, 0x51, x, 1) == 0)
					g_dng04_rantzen_0e16 = 1;
			}
		} else if (!g_dng04_goblins_0109 && pos == 0x0109) {
			if (event_fight1(ev, 0x19, random_schweif(6) + 2, 0) == 0)
				g_dng04_goblins_0109 = 1;
			ret = 1;
		} else if (!g_dng04_goblins_040d && pos == 0x040d) {
			if (event_fight1(ev, 0x19, random_schweif(6) + 2, 0) == 0)
				g_dng04_goblins_040d = 1;
			ret = 1;
		} else if (!g_dng04_zombies_1219 && pos == 0x1219) {
			if (event_fight1(ev, 0x10, random_schweif(6) + 2, 0) == 0)
				g_dng04_zombies_1219 = 1;
			ret = 1;
		} else if (!g_dng04_zombies_2d0e && pos == 0x2d0e) {
			if (event_fight1(ev, 0x73, 2, 0) == 0)
				g_dng04_zombies_2d0e = 1;
			ret = 1;
		} else if (!g_dng04_zombies_0919 && pos == 0x0919) {
			if (event_fight1(ev, 0x10, random_schweif(6) + 2, 0) == 0)
				g_dng04_zombies_0919 = 1;
			ret = 1;
		} else if (!g_dng04_orcs_200c && pos == 0x200c) {
			if (event_fight1(ev, 0x1b, random_schweif(4) + 2, 1) == 0)
				g_dng04_orcs_200c = 1;
			ret = 1;
		} else if (!g_dng04_orcs_2510 && pos == 0x2510) {
			if (event_fight1(ev, 0x48, 2, 0) == 0)
				g_dng04_orcs_2510 = 1;
			ret = 1;
		} else if (!g_dng04_orcs_0413 && pos == 0x0413) {
			if (event_fight1(ev, 0x1b, random_schweif(4) + 2, 1) == 0)
				g_dng04_orcs_0413 = 1;
			ret = 1;
		} else if (!g_dng04_orcs_0601 && pos == 0x0601) {
			if (event_fight1(ev, 0x1b, random_schweif(4) + 2, 0) == 0)
				g_dng04_orcs_0601 = 1;
			ret = 1;
		} else if (!g_dng04_orcs_1402 && pos == 0x1402) {
			if (event_fight2(ev, 0x1b, random_schweif(6) + 2, 1, 0x19, random_schweif(6) + 2, 0) == 0)
				g_dng04_orcs_1402 = 1;
			ret = 1;
		} else if (!g_dng04_orcs_1c02 && pos == 0x1c02) {
			if (event_fight2(ev, 0x1b, random_schweif(6) + 2, 1, 0x10, random_schweif(6) + 2, 0) == 0)
				g_dng04_orcs_1c02 = 1;
			ret = 1;
		} else if (g_dng04_village_visited && !g_dng04_monster_fought && (pos == 0x1c11 || pos == 0x1c10)) {
			if (event_fight1(ev, 0x72, 1, 0) == 0)
				g_dng04_monster_fought = g_dng04_monster_killed = 1;
			ret = 1;
		} else if (g_eolan_state == 2 && !g_dng04_rantzen_1e08 && (pos == 0x1e08 || pos == 0x1d08)) {
			g_fight_end = 3;
			if (start_fight(0x9b, 1) == 0)
				g_dng04_rantzen_1e08 = 1;
		}
		if (get_cell_a(mev->x, mev->y) == 10)
			dng04_deep_water(ev);
	} else if (type == 0x64) {
		x = dlg_set16(g_dialog, 5);
		h = get_random_hero(g_group);
		/* NATURE.LTX[2]: "Ihr entdeckt im schlammigen Wasser die Leiche eines ungluecklichen Wanderers. %s kann
		 * feststellen, dass dieser Mann an Bisswunden gestorben ist. Wenn die arme Seele Ausruestungsgegenstaende bei
		 * sich hatte, so sind sie nicht mehr zu finden. Lediglich einige Kleidungsfetzen sind noch nicht dem ewig
		 * gurgelnden Sumpf anheim gefallen." */
		sprintf(g_text_output_buf, ev->txt[2], h->name);
		dlg_message(g_dialog, g_text_output_buf);
		if (random_schweif(100) <= 50) {
			h = get_hero_attrib_max(g_group, ATTRIB_TA);
			/* NATURE.LTX[3]: "Ihr spuert, dass %s beim Anblick dieser sterblichen Ueberreste ploetzlich von Totenangst
			 * beschlichen wird. Mit stierem Blick taumelt %s einige Schritte von der Leiche weg, faellt dabei ueber das
			 * Gelaender des Knueppeldammes und stuerzt in den Sumpf. Nur mit Muehe gelingt es Euch, %s den todbringenden
			 * Armen des Sumpfes wieder zu entreissen." */
			sprintf(g_text_output_buf, ev->txt[3], h->name, grammar_pronoun(h->sex, 0, 0),
				grammar_pronoun(h->sex, 2, 0));
			dlg_message(g_dialog, g_text_output_buf);
			sub_hero_le(h, random_schweif(6));
			if (random_schweif(100) <= 20)
				drop_item_at(h, INV_SLOT_LEFT_HAND);
		}
		dlg_set16(g_dialog, x);
		ret = 1;
	} else if (type == 0x65) {
		Chest c(7, 1);
		ret = 1;
	} else if (type == 0x66) {
		event_fight1(ev, 0x73, 2, 0);
		ret = 1;
	} else if (type == 0x67) {
		talk(&g_talk, 0x12a, 1, 0);
		ret = 1;
	} else if (type == 0x68) {
		if (g_dng04_tower_open) {
			if (!g_dng04_mage_killed) {
				if (start_fight(FIGHT_ID_SPECIAL2, 0) == 0)
					g_dng04_mage_killed = 1;
			} else
				/* NATURE.LTX[20]: "Der Turm ist abgebrannt. Man kann ihn nicht mehr betreten!" */
				dlg_message(g_dialog, ev->txt[20]);
		} else
			/* NATURE.LTX[23]: "Die Tuer geht nicht auf und anhand ihrer Aura koennt Ihr erahnen, dass Magie im Spiel ist." */
			dlg_message(g_dialog, ev->txt[23]);
		ret = 1;
	} else if (type == 0x69) {
		signed char tgt = 0;
		if (pos == 0x291f)
			tgt = 0x2c;
		else if (pos == 0x0006)
			tgt = 0x2d;
		else if (pos == 0x2f06)
			tgt = 0x61;
		else if (pos == 0x2300)
			tgt = 0x77;
		else if (pos == 0x181f)
			tgt = 0x84;
		else if (pos == 0x031f)
			tgt = 0x85;
		else if (pos == 0x0010)
			tgt = 0x86;
		/* NATURE.LTX[21]: "Wollt Ihr diesen Sumpf wirklich verlassen?" */
		if (dlg_yes_no(g_dialog, ev->txt[21]) == 1) {
			if (tgt != 0) {
				snd_obj_stop(g_snd_music);
				swap_out_screen();
				g_group->route_pos = 0;
#if FIX_0104
				/* Fix #104: Dungeonwechsel einmal: old_dungeon behaelt den vorigen Dungeon */
				g_group->set_dungeon(0);
#else
				g_group->old_dungeon = g_group->dungeon;
				g_group->set_dungeon(g_group->dungeon = 0);
#endif
				group_set_on_route(g_group, 1);
				g_group->set_route_from(tgt);
				g_group->set_route_to(tgt);
			}
		} else {
			g_group->set_dir((g_group->dir + 2) & 3);
			g_cam_angle = (g_group->dir << 8) + 0x100;
		}
		ret = 1;
	}
	return ret;
}

void dng04_snake(struct area_events far *ev, signed char far *flag)
{
	register signed short go = 0;
	struct hero far *h = get_first_active_hero(g_group);

	if (!*flag)
		*flag = go = 1;
	else if (random_schweif(100) <= 15)
		go = 1;
	if (go) {
		if (test_skill(h, TA_SINNESSCHAERFE, 3) >= 0)
			/* NATURE.LTX[4]: "Vor Euch im Wasser schlaengelt sich eine Kvillotter! %s kann sie geistesgegenwaertig
			 * erschlagen, aber um ein Haar waeret Ihr auf sie getreten." */
			sprintf(g_text_output_buf, ev->txt[4], h->name);
		else {
			/* NATURE.LTX[5]: "%s wird von einer Schlange gebissen!@Ihr koennt nur noch das Ende des Tieres im brackigen
			 * Wasser verschwinden sehen, ohne genaueres zu erkennen." */
			sprintf(g_text_output_buf, ev->txt[5], h->name);
			if (random_schweif(100) > 50)
				hero_get_poison(h, 7);
			else
				sub_hero_le(h, random_schweif(4));
		}
		dlg_message(g_dialog, g_text_output_buf);
		*flag = 1;
	}
}

signed short dng04_take_from_pool(struct area_events far *ev, register signed short n, signed char far *cnt)
{
	signed char c;
	signed short v;
	signed short r;

	if ((v = c = *cnt) < n) {
		r = v;
		*cnt = 0;
	} else {
		r = n;
		*cnt = c - n;
	}
	return r;
}

void dng04_deep_water(struct area_events far *ev)
{
	signed short lp;
	struct hero far *h;
	register signed short dmg;

	if (count_failed_skill(g_group, TA_SCHWIMMEN, 0)) {
		h = get_worst_hero_skill(g_group, TA_SCHWIMMEN);
		if (!h->see_und_fluss) {
			dmg = random_schweif(20);
			lp = h->le;
			if (lp - dmg > 0)
				/* GLOBLTXT.LTX[443]: "%s droht in den Fluten unterzugehen, bleibt jedoch mit grosser Muehe an der
				 * Wasseroberflaeche." */
				sprintf(g_text_output_buf, g_ttx[443], h->name);
			else
				/* GLOBLTXT.LTX[442]: "%s ist soeben in den Fluten untergegangen! Eine Sekunde konntet Ihr %s Hand noch
				 * hilfesuchend aus dem Wasser ragen sehen, doch dann verschwand sie in den Fluten. Als Ihr %s bergen koennt,
				 * ist %s bereits ertrunken!" */
				sprintf(g_text_output_buf, g_ttx[442], h->name, grammar_pronoun(h->sex, 1, 1),
					grammar_pronoun(h->sex, 2, 0), grammar_pronoun(h->sex, 0, 0));
			dlg_message(g_dialog, g_text_output_buf);
			g_in_fight = 1;
			sub_hero_le(h, dmg);
			g_in_fight = 0;
			g_hero_died = 1;
		}
	}
}
