/* OVL130 - Overlay O130 (Segment 0x21fa in SCHWEIF.EXE): Ereignisse des Dungeons 2, des Kellers unter dem Haus des
 * Namenlosen-Dieners (Texte FINAL.LTX = Datei 0x2ad, gemeinsam mit den Dungeons 1 und 3; g_area_descs[2]);
 * Ereignisobjekt ev (struct area_events, Texte ev->txt), Kartenereignis mev.
 * dng02_handler(ev, mev): pos = (Zeile - g_map_x0) << 8 | (Spalte - g_map_y0), h = erster handlungsfaehiger Held.
 *   MAP_EVENT_ENTER: g_dng02_step_toggle umschalten und dng02_amulet_timers; Felder 0x417..0x817 Bolzenfalle
 *   (FINAL[125]: rueckwaerts 1..6, sonst 3..18 LE fuer alle); dann dng02_fights. Typ 1 Tueren dng02_door; Typ 2
 *   Treppen (0x1e0b nach Dungeon 1 auf (0x1d, 0x0a), 0x131b nach Dungeon 3 auf (0x13, 0x1d) mit g_dng03_thief_pending
 *   = 1: dort wartet der Dieb des Salamandersteins, dng03_stone_thief). 0x64 Brunnen (Wasser auffuellen, FINAL[3]).
 *   0x65 Regal mit Gartengeraeten (FINAL[0..2]: Ablage 0x32, danach Sinnesschaerfe +5 -> Bronzeschluessel im alten
 *   Stiefel). 0x66 Vorraete/leere Kruege (FINAL[4..7], an 0x907 Sinnesschaerfe -> trockenes Wirselkraut). 0x67 Betten
 *   (FINAL[8]/[13]; Angelhaken unter dem Kissen 0x50b, Otterfell unter der Matratze 0x815). 0x68 Tische (Wuerfel und
 *   200 Heller an 0x91b, Wachplan/Rezept fuer Schlaftrunk an 0x51b, sonst FINAL[5]). 0x69 Truhen (Chest 0..6 je
 *   Position). 0x6a Waende: 0x502 geheimer Schalter (Schloesser +20, FINAL[11]/[17]), 0x1211 Schluesselloch
 *   (Schloesser +10, Bronzeschluessel, FINAL[18]/[19]), 0x1c1a Sonnenschloss (vier Amuletteile ergeben den
 *   Schluessel, FINAL[20]/[21]; Teile weg). Nach einem behandelten Ereignis (ausser MAP_EVENT_ENTER) laufen die
 *   Amulett-Zaehler einmal mit g_dng02_step_toggle = 0.
 * dng02_fights(ev, pos): einmalige Kaempfe (event_fight1/2/event_fight, Merker g_dng02_<gegner>_<pos>); an 0x130e,
 *   0x1e0d, 0x1617, 0x1e17 je ein Skelettkrieger (Gegner 0x5d, g_fight_end = 6) - gewonnen -> dng02_amulet_part1..4
 *   (FINAL[64]: das Skelett laesst ein Metallstueck fallen; Amuletteil, Wand auf, Klang), solange der Zaehler 0 ist.
 * dng02_amulet_timers: solange das Sonnenschloss zu ist, zaehlt jeder vorhandene Amuletteil-Zaehler bei
 *   g_dng02_step_toggle == 0 hoch; bei 75 ist das Teil wieder weg (group_drop_item, Klang 0x97) und der
 *   Skelettkrieger kampfbereit.
 * dng02_door(ev, pos, x, y): Schloss (Lock 0x7c..0x87 je Position) mit Wandbild 0x0d/0x0e bei g_lock_cell_bits != -1.
 *
 * BCC -mlarge -3 -Ot -Og -c -Y OVL130.CPP -> 5260/5260 Bytes GLEICH (18.10.2026), cobjfix laeuft (Namensfassung mit
 *   schweif.h: 19.10.2026 GLEICH, LINKGLEICH)
 * - C++ wegen der lokalen Objekte Stash (O238, 0x16 Bytes, [bp-22h]), Chest (O078, 0x55 Bytes, [bp-62h]) und Lock
 *   (O108, 0x29 Bytes) - Ktor/Dtor(.., 2) am Blockende; Stash und Chest liegen in Geschwisterbloecken und ueberlappen
 *   (beide enden bei [bp-0dh]).
 * - automap_cell_write ueber AUTOMAP_CELL_WRITE_W (letzter Parameter short, wie OVL063).
 * - dng02_handler: MAP_EVENT_ENTER springt direkt auf "mov ax,[bp-6]" (gemeinsames return) -> alles uebrige steht im
 *   else-Block samt "if (ret) { .. dng02_amulet_timers .. }"; Typ 1/2 fallen in die zweite if-Kette (0x64..0x6a).
 * - type/pos register (DI/SI) wie OVL063; "(idx = f()) != -1" eingebettet (cmp ax,0ffffh).
 * - dng02_door: nr/pic ohne register (SI/DI), pic uninitialisiert (wird nur bei nr != -1 benutzt).
 * Originalfehler: dng02_door zeichnet das Wandbild mit "x - g_map_x0" statt "x - g_map_y0" (OVL063/OVL065 nehmen
 *   g_map_y0) - falsche Spalte, sobald die beiden Kartenversaetze verschieden sind. dng02_fights gibt fuer einen
 *   erneuten Skelettkampf, der verloren geht, trotzdem 1 zurueck (wie bei allen Kaempfen).
 */
#include <stdio.h>

#include "schweif.h"

/* automap_cell_write mit short als letztem Parameter (wie im Original deklariert) */
#define AUTOMAP_CELL_WRITE_W \
	((void (*)(void far *, signed short, signed short, signed short, signed short))automap_cell_write)

signed short dng02_handler(struct area_events far *ev, struct map_event far *mev)
{
	struct hero far *h;
	signed short ret;
	struct hero far *h2;
	signed short idx;
	register signed short type;
	register signed short pos;

	type = mev->type;
	pos = (mev->y - g_map_x0) << 8;
	pos |= mev->x - g_map_y0;
	h = get_first_active_hero(g_group);
	ret = 0;
	if (type == MAP_EVENT_ENTER) {
		g_dng02_step_toggle ^= 1;
		dng02_amulet_timers(ev);
		if (pos == 0x417 || pos == 0x517 || pos == 0x617 || pos == 0x717 || pos == 0x817) {
			/* FINAL.LTX[125]: "Irgendwie schafft Ihr es, stockvoll in eine Falle zu laufen. Aus allen Richtungen fliegen
			 * ploetzlich Armbrustbolzen um Euch herum und verletzen jeden Einzelnen. Leider konntet Ihr nicht ausmachen,
			 * woher genau sie kamen!" */
			dlg_message(g_dialog, ev->txt[125]);
			if (g_move_flags & 2)
				group_sub_le_range(g_group, 1, 6);
			else
				group_sub_le_range(g_group, 3, 0x12);
		}
		ret = dng02_fights(ev, pos);
	} else {
		if (type == 1) {
			ret = dng02_door(ev, pos, mev->x, mev->y);
		} else if (type == 2) {
			if (pos == 0x1e0b) {
				group_set_screen_pos(0x1d, 0x0a);
				g_group->old_dungeon = g_group->dungeon;
				g_group->dungeon = 1;
				g_group->dir = 1;
			} else if (pos == 0x131b) {
				group_set_screen_pos(0x13, 0x1d);
				g_group->old_dungeon = g_group->dungeon;
				g_group->dungeon = 3;
				g_group->dir = 0;
				g_dng03_thief_pending = 1;
			}
			ret = 1;
		}
		if (type == 0x64) {
			/* FINAL.LTX[3]: "Das Wasser im Brunnen scheint frisch zu sein.@Wollt Ihr Eure Wasserflaschen auffuellen?" */
			if (dlg_yes_no(g_dialog, ev->txt[3]) == 1)
				group_refill_water(g_group);
			ret = 1;
		} else if (type == 0x65) {
			if (!g_dng02_tools_state) {
				/* FINAL.LTX[0]: "Ihr seht eine Vielzahl von Gartengeraeten im Regal liegen.@Wollt Ihr einige als improvisierte
				 * Waffen mitnehmen?" */
				if (dlg_yes_no(g_dialog, ev->txt[0]) == 1) {
					Stash l(0x32, 0);
					g_dng02_tools_state++;
				}
				ret = 1;
			} else if (g_dng02_tools_state == 1) {
				if (test_skill(h, TA_SINNESSCHAERFE, 5) >= 0) {
					/* FINAL.LTX[1]: "In einem alten Stiefel entdeckt %s einen bronzenen Schluessel!" */
					sprintf(g_text_output_buf, ev->txt[1], h->name);
					dlg_message(g_dialog, g_text_output_buf);
					group_give_item(g_group, ITEM_BRONZE_KEY, 1);
					g_dng02_tools_state++;
				} else {
					/* FINAL.LTX[2]: "Ihr seht eine Vielzahl von Gartengeraeten im Regal liegen." */
					dlg_message(g_dialog, ev->txt[2]);
				}
				ret = 1;
			} else {
				dlg_message(g_dialog, ev->txt[2]);
				ret = 1;
			}
		} else if (type == 0x66) {
			if (pos == 0x1405 || pos == 0x1407) {
				/* FINAL.LTX[4]: "Hier lagern jede Menge Lebensmittel, wie Mehl, Erbsen und Aehnliches; jedoch muss alles
				 * gekocht werden, bevor es geniessbar ist. Auf alle Faelle aber scheint dieser Vorrat fuer viele Leute
				 * ausreichend zu sein..." */
				dlg_message(g_dialog, ev->txt[4]);
			} else if (pos == 0x604 || pos == 0x808 || pos == 0xc05) {
				/* FINAL.LTX[6]: "Es stehen etliche Kruege und Gefaesse herum, die jedoch allesamt leer sind." */
				dlg_message(g_dialog, ev->txt[6]);
			} else if (pos == 0x907) {
				if (!g_dng02_jugs_state) {
					dlg_message(g_dialog, ev->txt[6]);
					g_dng02_jugs_state++;
				} else if (g_dng02_jugs_state == 1) {
					if (test_skill(h, TA_SINNESSCHAERFE, 5) >= 0) {
						g_dng02_jugs_state++;
						/* FINAL.LTX[7]: "Doch Halt! Hinter einem groesseren Krug findet sich ein Buendel trockenen
						 * Wirselkrautes.@Wollt Ihr es mitnehmen?" */
						if (dlg_yes_no(g_dialog, ev->txt[7]) == 1) {
							group_give_item(g_group, ITEM_DRIED_WHIRLWEED, 1);
							g_dng02_jugs_state++;
						}
					} else {
						dlg_message(g_dialog, ev->txt[6]);
					}
				} else if (g_dng02_jugs_state == 2) {
					if (dlg_yes_no(g_dialog, ev->txt[7]) == 1) {
						group_give_item(g_group, ITEM_DRIED_WHIRLWEED, 1);
						g_dng02_jugs_state++;
					}
				} else {
					dlg_message(g_dialog, ev->txt[6]);
				}
				ret = 1;
			}
		} else if (type == 0x67) {
			/* FINAL.LTX[14]: "Unter dem Kissen findet Ihr einen Angelhaken.@Wollt Ihr ihn mitnehmen?" */
			if (pos == 0x50b && !g_dng02_fishhook_taken && dlg_yes_no(g_dialog, ev->txt[14]) == 1) {
				group_give_item(g_group, ITEM_FISHHOOK, 1);
				g_dng02_fishhook_taken++;
			}
			if (pos == 0x815 && !g_dng02_otter_fur_taken) {
				/* FINAL.LTX[15]: "Gut versteckt unter der Matratze entdeckt Ihr beim Durchstoebern ein schoenes
				 * Otterfell.@Moechtet Ihr es mitnehmen?" */
				if (dlg_yes_no(g_dialog, ev->txt[15]) == 1) {
					group_give_item(g_group, ITEM_OTTER_PELT, 1);
					g_dng02_otter_fur_taken++;
				}
			} else {
				/* FINAL.LTX[13]: "Dieses Bett sieht irgendwie ungepflegt und siffig aus!" */
				/* FINAL.LTX[8]: "Dieses Bett ist eindeutig in Gebrauch!" */
				dlg_message(g_dialog, mev->x > 0x10 ? ev->txt[13] : ev->txt[8]);
			}
			ret = 1;
		} else if (type == 0x68) {
			if (pos == 0x91b && !g_dng02_dice_money_taken) {
				/* FINAL.LTX[9]: "Auf dem Tisch liegen einige Wuerfel und etwas Kleingeld." */
				dlg_message(g_dialog, ev->txt[9]);
				group_add_money(g_group, 200L);
				g_dng02_dice_money_taken++;
			} else if (pos == 0x51b) {
				if (!g_dng02_recipe_taken && test_skill(h, TA_SINNESSCHAERFE, 5) >= 0) {
					/* FINAL.LTX[16]: "Holla! Unter all den Papieren hier entdeckt %s doch tatsaechlich ein Rezept fuer einen
					 * Schlaftrunk!@Wollt Ihr es mitnehmen?" */
					sprintf(g_text_output_buf, ev->txt[16], h->name);
					if (dlg_yes_no(g_dialog, g_text_output_buf) == 1) {
						group_give_item(g_group, ITEM_RECIPE_SLEEPING_POTION, 1);
						g_dng02_recipe_taken++;
					}
				} else {
					/* FINAL.LTX[10]: "Einige Papiere, offensichtlich etwas wie ein Wachplan und ein Kohlestift liegen auf dem
					 * Tisch." */
					dlg_message(g_dialog, ev->txt[10]);
				}
			} else {
				/* FINAL.LTX[5]: "Auf dem Tisch stehen keinerlei Geraetschaften mehr, jedoch sieht man Wasserraender, Brand-
				 * und Aetzflecken." */
				dlg_message(g_dialog, ev->txt[5]);
			}
			ret = 1;
		} else if (type == 0x69) {
			if (pos == 0x51e) {
				Chest c(1, 1);
			} else if (pos == 0xb0e) {
				Chest c(2, 1);
			} else if (pos == 0xb11) {
				Chest c(3, 1);
			} else if (pos == 0xb1a) {
				Chest c(4, 1);
			} else if (pos == 0x1114) {
				Chest c(5, 1);
			} else if (pos == 0x1117) {
				Chest c(6, 1);
			} else {
				Chest c(0, 1);
			}
			ret = 1;
		} else if (type == 0x6a) {
			if (pos == 0x502) {
				if (!g_dng02_secret_switch) {
					if (test_skill(h, TA_SCHLOESSER, 0x14) >= 0) {
						/* FINAL.LTX[11]: "Nach einem genauen Blick auf die Wand vor Euch, entdeckt %s etwas und tatsaechlich, es
						 * gelingt %s, einen geheimen Schalter zu entdecken, der beim Betaetigen einen Durchgang vor Euch freigibt." */
						sprintf(g_text_output_buf, ev->txt[11], h->name,
							grammar_pronoun(h->sex, 3, 0));
						dlg_message(g_dialog, g_text_output_buf);
						event_set_cell(ev, 2, 5, 9, 0, -1);
						AUTOMAP_CELL_WRITE_W(g_map, 0x266, 5, 2, 0x3b);
						event_set_cell_bits(ev, g_map_y0 + 2, g_map_x0 + 5, 0x2000);
						snd_play(g_snd_fx, 0xac);
						g_dng02_secret_switch = 1;
					} else {
						/* FINAL.LTX[17]: "%s erkennt zwar einige Unebenheiten an der Wand und meint auch, eine Geheimtuer entdeckt
						 * zu haben, doch es gelingt %s nicht, sie zu oeffnen." */
						sprintf(g_text_output_buf, ev->txt[17], h->name,
							grammar_pronoun(h->sex, 3, 0));
						dlg_message(g_dialog, g_text_output_buf);
					}
					ret = 1;
				}
			} else if (pos == 0x1211) {
				if (!g_dng02_keyhole_door) {
					if (test_skill(h, TA_SCHLOESSER, 10) >= 0) {
						if ((idx = group_find_key(g_group, ITEM_BRONZE_KEY)) != -1) {
							h = g_group->heroes[idx];
							if ((idx = hero_find_key(h, ITEM_BRONZE_KEY)) != -1) {
								if (!check_hero3(h))
									h = get_random_hero(g_group);
								/* FINAL.LTX[19]: "Nach einigem Abtasten der Ritzen dieser Wand, entdeckt %s ein Schluesselloch, und nach
								 * kurzem Durchprobieren Eurer Schluessel, schnappt auch das Schloss auf und die Tuere oeffnet sich." */
								sprintf(g_text_output_buf, ev->txt[19], h->name);
								dlg_message(g_dialog, g_text_output_buf);
								event_set_cell(ev, 0x11, 0x12, 9, 0, -1);
								AUTOMAP_CELL_WRITE_W(g_map, 0x266, 0x12, 0x11, 0x3b);
								event_set_cell_bits(ev, g_map_y0 + 0x11,
									g_map_x0 + 0x12, 0x2000);
								snd_play(g_snd_fx, 0xac);
								g_dng02_keyhole_door = 1;
							} else {
								/* FINAL.LTX[18]: "Nach einigem Abtasten der Ritzen dieser Wand, entdeckt %s ein Schluesselloch, doch
								 * leider habt Ihr keinen passenden Schluessel bei Euch." */
								sprintf(g_text_output_buf, ev->txt[18], h->name);
								dlg_message(g_dialog, g_text_output_buf);
							}
						} else {
							sprintf(g_text_output_buf, ev->txt[18], h->name);
							dlg_message(g_dialog, g_text_output_buf);
						}
					} else {
						sprintf(g_text_output_buf, ev->txt[17], h->name,
							grammar_pronoun(h->sex, 3, 0));
						dlg_message(g_dialog, g_text_output_buf);
					}
					ret = 1;
				}
			} else if (pos == 0x1c1a) {
				if (!g_dng02_sun_lock_open) {
					if (group_find_item(g_group,
						ITEM_AMULET_PART1) != -1 && group_find_item(g_group,
						ITEM_AMULET_PART2) != -1 &&
					    group_find_item(g_group,
						ITEM_AMULET_PART3) != -1 && group_find_item(g_group,
						ITEM_AMULET_PART4) != -1) {
						h2 = get_random_hero(g_group);
						/* FINAL.LTX[21]: "%s untersucht die Wand und entdeckt tatsaechlich nach Kurzem ein grosses Schloss, fuer das
						 * ein sonnenfoermiger Schluessel benoetigt wird.@<Hier, die Amulette!> ruft %s aufgeregt, kramt sie zusammen
						 * und fuegt sie zu einem sonnenfoermigen Schluessel zusammen, der tatsaechlich in das Schloss passt. Kurz
						 * darauf tut sich vor Euch ein schmaler Durchgang auf!" */
						sprintf(g_text_output_buf, ev->txt[21], h->name, h2->name);
						dlg_message(g_dialog, g_text_output_buf);
						event_set_cell(ev, 0x1a, 0x1c, 9, 0, -1);
						AUTOMAP_CELL_WRITE_W(g_map, 0x266, 0x1c, 0x1a, 0x3c);
						event_set_cell_bits(ev, g_map_y0 + 0x1a, g_map_x0 + 0x1c, 0x2000);
						snd_play(g_snd_fx, 0xac);
						g_dng02_sun_lock_open = 1;
						group_drop_item(g_group, ITEM_AMULET_PART1);
						group_drop_item(g_group, ITEM_AMULET_PART2);
						group_drop_item(g_group, ITEM_AMULET_PART3);
						group_drop_item(g_group, ITEM_AMULET_PART4);
					} else {
						/* FINAL.LTX[20]: "%s untersucht die Wand und entdeckt tatsaechlich nach Kurzem ein grosses Schloss, fuer das
						 * ein sonnenfoermiger Schluessel benoetigt wird." */
						sprintf(g_text_output_buf, ev->txt[20], h->name,
							grammar_pronoun(h->sex, 3, 0));
						dlg_message(g_dialog, g_text_output_buf);
					}
					ret = 1;
				}
			}
		}
		if (ret) {
			g_dng02_step_toggle = 0;
			dng02_amulet_timers(ev);
			g_dng02_step_toggle = 1;
		}
	}
	return ret;
}

signed short dng02_fights(struct area_events far *ev, register signed short pos)
{
	register signed short ret = 0;

	if (pos == 0x1c05 && !g_dng02_druids_1c05) {
		if (!event_fight1(ev, 0x12, 2, 0))
			g_dng02_druids_1c05 = 1;
		ret = 1;
	} else if (pos == 0x802 && !g_dng02_cultists_0802) {
		if (!event_fight2(ev, 0x37, 1, 1, 0x5e, 2, 0))
			g_dng02_cultists_0802 = 1;
		ret = 1;
	} else if (pos == 0x402 && !g_dng02_cultists_0402) {
		if (!event_fight2(ev, 0x36, 2, 0, 0x13, 1, 2))
			g_dng02_cultists_0402 = 1;
		ret = 1;
	} else if (pos == 0x20b && !g_dng02_cultists_020b) {
		if (!event_fight1(ev, 0x36, 2, 0))
			g_dng02_cultists_020b = 1;
		ret = 1;
	} else if (pos == 0x70d && !g_dng02_cultists_070d) {
		if (!event_fight(ev, 0x35, 2, 0, 0, 0x37, 1, 0, 0, 0x13, 1, 2, 3))
			g_dng02_cultists_070d = 1;
		ret = 1;
	} else if (pos == 0x41c && !g_dng02_cultists_041c) {
		if (!event_fight(ev, 0x37, 6, 0, 0, 0x38, 3, 0, 0, 0x57, 1, 2, 0))
			g_dng02_cultists_041c = 1;
		ret = 1;
	} else if (pos == 0xb17 && !g_dng02_sabretooth_0b17) {
		if (!event_fight1(ev, 5, 2, 0))
			g_dng02_sabretooth_0b17 = 1;
		ret = 1;
	} else if (pos == 0xf0c && !g_dng02_sabretooth_0f0c) {
		if (!event_fight1(ev, 5, 1, 0))
			g_dng02_sabretooth_0f0c = 1;
		ret = 1;
	} else if (pos == 0xf1c && !g_dng02_sabretooth_0f1c) {
		if (!event_fight1(ev, 5, 1, 0))
			g_dng02_sabretooth_0f1c = 1;
		ret = 1;
	} else if (pos == 0x150c && !g_dng02_cave_spiders_150c) {
		if (!event_fight1(ev, 0x27, 5, 0))
			g_dng02_cave_spiders_150c = 1;
		ret = 1;
	} else if (pos == 0x171d && !g_dng02_cultists_171d) {
		if (!event_fight2(ev, 0x37, 2, 1, 0x5e, 4, 0))
			g_dng02_cultists_171d = 1;
		ret = 1;
	} else if (pos == 0x1c1b && !g_dng02_cultists_1c1b) {
		if (!event_fight(ev, 0x37, 3, 0, 0, 0x38, 2, 0, 0, 0x57, 1, 2, 0))
			g_dng02_cultists_1c1b = 1;
		ret = 1;
	} else if (pos == 0x130e && !g_dng02_amulet1_timer) {
		g_fight_end = 6;
		if (!event_fight1(ev, 0x5d, 1, 0))
			dng02_amulet_part1((struct area_events far *)g_area_events);
		ret = 1;
	} else if (pos == 0x1e0d && !g_dng02_amulet2_timer) {
		g_fight_end = 6;
		if (!event_fight1(ev, 0x5d, 1, 0))
			dng02_amulet_part2((struct area_events far *)g_area_events);
		ret = 1;
	} else if (pos == 0x1617 && !g_dng02_amulet3_timer) {
		g_fight_end = 6;
		if (!event_fight1(ev, 0x5d, 1, 0))
			dng02_amulet_part3((struct area_events far *)g_area_events);
		ret = 1;
	} else if (pos == 0x1e17 && !g_dng02_amulet4_timer) {
		g_fight_end = 6;
		if (!event_fight1(ev, 0x5d, 1, 0))
			dng02_amulet_part4((struct area_events far *)g_area_events);
		ret = 1;
	}
	return ret;
}

void dng02_amulet_timers(struct area_events far *ev)
{
	if (!g_dng02_sun_lock_open) {
		if (g_dng02_amulet1_timer != 0) {
			if (!g_dng02_step_toggle)
				g_dng02_amulet1_timer++;
			if (g_dng02_amulet1_timer == 0x4b) {
				g_dng02_amulet1_timer = 0;
				group_drop_item(g_group, ITEM_AMULET_PART1);
				snd_play(g_snd_fx, 0x97);
			}
		}
		if (g_dng02_amulet2_timer != 0) {
			if (!g_dng02_step_toggle)
				g_dng02_amulet2_timer++;
			if (g_dng02_amulet2_timer == 0x4b) {
				g_dng02_amulet2_timer = 0;
				group_drop_item(g_group, ITEM_AMULET_PART2);
				snd_play(g_snd_fx, 0x97);
			}
		}
		if (g_dng02_amulet3_timer != 0) {
			if (!g_dng02_step_toggle)
				g_dng02_amulet3_timer++;
			if (g_dng02_amulet3_timer == 0x4b) {
				g_dng02_amulet3_timer = 0;
				group_drop_item(g_group, ITEM_AMULET_PART3);
				snd_play(g_snd_fx, 0x97);
			}
		}
		if (g_dng02_amulet4_timer != 0) {
			if (!g_dng02_step_toggle)
				g_dng02_amulet4_timer++;
			if (g_dng02_amulet4_timer == 0x4b) {
				g_dng02_amulet4_timer = 0;
				group_drop_item(g_group, ITEM_AMULET_PART4);
				snd_play(g_snd_fx, 0x97);
			}
		}
	}
}

signed short dng02_door(struct area_events far *ev, signed short pos, signed short x, signed short y)
{
	signed short ret = 0;
	signed short nr = -1;
	signed short pic;

	switch (pos) {
	case 0x1902:
		nr = 0x7c;
		pic = 0x0e;
		break;
	case 0x1c06:
		nr = 0x7d;
		pic = 0x0d;
		break;
	case 0x1603:
		nr = 0x7e;
		pic = 0x0d;
		break;
	case 0x1003:
		nr = 0x7f;
		pic = 0x0d;
		break;
	case 0x0a03:
		nr = 0x80;
		pic = 0x0d;
		break;
	case 0x0209:
		nr = 0x81;
		pic = 0x0d;
		break;
	case 0x030d:
		nr = 0x82;
		pic = 0x0e;
		break;
	case 0x0313:
		nr = 0x83;
		pic = 0x0e;
		break;
	case 0x031c:
		nr = 0x84;
		pic = 0x0e;
		break;
	case 0x150d:
		nr = 0x85;
		pic = 0x0d;
		break;
	case 0x1514:
		nr = 0x86;
		pic = 0x0d;
		break;
	case 0x181d:
		nr = 0x87;
		pic = 0x0e;
		break;
	}
	if (nr != -1) {
		Lock lk(nr);
		ret = 1;
		if (g_lock_cell_bits != -1) {
			event_set_cell_bits(ev, x, y, g_lock_cell_bits);
#if FIX_0139
			/* Fix #139: Spalte mit g_map_y0 (wie O063/O065) */
			AUTOMAP_CELL_WRITE_W(g_map, 0x266, y - g_map_x0, x - g_map_y0, pic);
#else
			AUTOMAP_CELL_WRITE_W(g_map, 0x266, y - g_map_x0, x - g_map_x0, pic);
#endif
			snd_play(g_snd_fx, 0x61);
		}
	}
	return ret;
}

void dng02_amulet_part1(struct area_events far *ev)
{
	g_dng02_amulet1_timer = 1;
	snd_play(g_snd_fx, 0x9f);
	event_set_cell(ev, 0x10, 0x19, 0, 0, -1);
	AUTOMAP_CELL_WRITE_W(g_map, 0x266, 0x19, 0x10, 0x3b);
	/* FINAL.LTX[64]: "Das Skelett haelt mitten in seiner Bewegung inne und aus seinen knochigen Fingern faellt ein
	 * Stueck Metall zu Boden, direkt vor Eure Fuesse. Dann faellt es in sich zusammen und regt sich nicht mehr.
	 * Vorsichtig hebt ihr das Metall auf." */
	dlg_message(g_dialog, ev->txt[64]);
	group_give_item(g_group, ITEM_AMULET_PART1, 1);
}

void dng02_amulet_part2(struct area_events far *ev)
{
	g_dng02_amulet2_timer = 1;
	snd_play(g_snd_fx, 0x9f);
	event_set_cell(ev, 0x14, 0x1d, 0, 0, -1);
	AUTOMAP_CELL_WRITE_W(g_map, 0x266, 0x1d, 0x14, 0x3c);
	/* FINAL.LTX[64]: "Das Skelett haelt mitten in seiner Bewegung inne und aus seinen knochigen Fingern faellt ein
	 * Stueck Metall zu Boden, direkt vor Eure Fuesse. Dann faellt es in sich zusammen und regt sich nicht mehr.
	 * Vorsichtig hebt ihr das Metall auf." */
	dlg_message(g_dialog, ev->txt[64]);
	group_give_item(g_group, ITEM_AMULET_PART2, 1);
}

void dng02_amulet_part3(struct area_events far *ev)
{
	g_dng02_amulet3_timer = 1;
	snd_play(g_snd_fx, 0x5b);
	/* FINAL.LTX[64]: "Das Skelett haelt mitten in seiner Bewegung inne und aus seinen knochigen Fingern faellt ein
	 * Stueck Metall zu Boden, direkt vor Eure Fuesse. Dann faellt es in sich zusammen und regt sich nicht mehr.
	 * Vorsichtig hebt ihr das Metall auf." */
	dlg_message(g_dialog, ev->txt[64]);
	group_give_item(g_group, ITEM_AMULET_PART3, 1);
}

void dng02_amulet_part4(struct area_events far *ev)
{
	g_dng02_amulet4_timer = 1;
	snd_play(g_snd_fx, 0x9f);
	event_set_cell(ev, 0x17, 0x19, 0, 0, -1);
	AUTOMAP_CELL_WRITE_W(g_map, 0x266, 0x19, 0x17, 0x3b);
	/* FINAL.LTX[64]: "Das Skelett haelt mitten in seiner Bewegung inne und aus seinen knochigen Fingern faellt ein
	 * Stueck Metall zu Boden, direkt vor Eure Fuesse. Dann faellt es in sich zusammen und regt sich nicht mehr.
	 * Vorsichtig hebt ihr das Metall auf." */
	dlg_message(g_dialog, ev->txt[64]);
	group_give_item(g_group, ITEM_AMULET_PART4, 1);
}
