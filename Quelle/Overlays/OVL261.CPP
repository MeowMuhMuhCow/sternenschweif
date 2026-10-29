/* OVL261 - Overlay O261 (Segment 0x24b9 in SCHWEIF.EXE): Ereignisse des Dungeons 6, der ersten Ebene der Orkfestung
 * in den Blutzinnen (Texte BLUT1.LTX = Datei 0x373; g_area_descs[6]); Ereignisobjekt ev (struct area_events, Texte
 * ev->txt), Kartenereignis mev. Unterfunktionen in O262 (dng06_*, blut_*).
 * dng06_handler(ev, mev): pos = (Zeile - g_map_x0) << 8 | (Spalte - g_map_y0), h = erster handlungsfaehiger Held.
 *   MAP_EVENT_ENTER: Laermfelder - misslungene Schleichen-Proben (count_failed_skill, Zuschlag 0..2) erhoehen
 *   g_dng06_noise (x1/x2/x3/x5), danach blut_nop; an 0x1b04.. und 0xe10/0xe11 dabei einmalig Kampf gegen einen Ork
 *   (0x1b); 0x70f/0x711 schalten die Bolzenwand 0x710 (BLUT1[114], scharf [115]: 8..24 LE fuer alle); 0x914/0x714 die
 *   Schroetergrube 0x814 (dng06_beetle_pit); 0x1819 Alarmdraht (Sinnesschaerfe +4, [75]); 0x90a Topfhelm ([17]);
 *   Klappgruben 0x1305/0x1519 (dng06_pit); Steinschlag 0x307/0x709/0x30c bzw. 0x30a/0x60c (blut_rockfall, [37]);
 *   0x410 Zwergenskelett (dng06_dwarf_skeleton); 0x1a0c Ersatzteile (Stash 0x0e, [55]); 0x170f Hundegebell ([59],
 *   solange der Kaefig zu ist); 0x180e Hundefutter ([67]/[68]); 0x1a0e Hundeharnische ([69]); 0x1715 schwere Waffen
 *   der Oger zerschlagen ([73]/[74], Laerm +5); 0x161d Latrine ([86], zu 10 % Ork-Ueberfall -> dng06_latrine_ambush);
 *   0x1012 Waffenlager (Stash 0x0f, [92]); 0x1117 grosse Halle ([93]); danach immer ret = dng06_fights(ev, pos). Typ
 *   1 dng06_door. 0x64 dng06_straw. 0x65 Pechfass ([0]) bzw. an 0x1e1c Heu und Pferdefutter ([78..85]: anzuenden mit
 *   Fackel/Laterne/Zunderkaestchen -> g_dng06_hay_state = 1, g_dng06_alarm_delay = 0x1c2, Qualm treibt die Gruppe nach (0x18,
 *   0x19)). 0x66 Treppe nach Dungeon 7 ([5]). 0x67 Truhen (Chest 9..13). 0x68 dng06_stores. 0x69 dng06_weapon_rack.
 *   0x6a Ausgang ([14] mit Mond-/Tageslicht [15]/[16]): Musik aus, Dungeon/Stadt/Ortsart 0, unterwegs am Ort 0xab;
 *   NSC 1 verabschiedet sich (g_npc_months = 0x63, groups_check_npc). 0x6b dng06_brazoragh. 0x6c Esse ([48]), 0x6d
 *   Feuerstelle ([47]), 0x6e Wasserfass ([50]). 0x6f Brunnen/Traenken ([51]/[76]/[109]): Wasser auffuellen, einmal je
 *   INGAME_TIMER_DNG06_WELL LE + 1 und Wirkung. 0x70 an 0x1807 dng06_hornets, sonst Kisten mit Rohlingen ([53]/[54]).
 *   0x71 Gravesh-Schrein ([56..58]: Dolch, Ingerimm -50). 0x72 Kaefig der Kriegshunde ([59..66]: oeffnen -> Kampf 4 x
 *   0x5e, durch das Gitter toeten, sonst Biss). 0x73 Pferdebox ([77]). 0x74 Schacht ([91]). 0x75 dng06_beer_barrel.
 *   0x76 Kessel ([100..108]: Eintopf/Gulasch untersuchen oder essen, einmal Hunger 0). 0x77 Tisch ([110]). 0x78 Altar
 *   ([113]). 0x79 Schluessellocher an 0xa14/0xb0e ([119..121], Knochen- oder Gusseisenschluessel oeffnet die Wand).
 * dng06_fights(ev, pos): Kaempfe an 0x1d02, 0x1503, 0xf03, 0x402, 0x1009 (Orkveteran 0x78 bzw. Orks 2/0x1b), Zahl
 *   nach g_dng06_alarm, sonst bei Tag (g_daytime in (0x93a8, 0x190c8]) bzw. Nacht; Merker g_dng06_<gegner>_<pos>.
 * dng06_latrine_ambush: Laerm += Kampfrunden, g_dng06_alarm_delay = 0x708; Frage [87..89]: Leiche in den Schacht werfen ([90],
 *   Gruppe nach (0x16, 0x1e), g_dng06_alarm_delay = 0).
 *
 * BCC -mlarge -3 -Ot -Og -c -Y OVL261.CPP -> 7000/7000 Bytes GLEICH (18.10.2026), cobjfix laeuft (mit -Oi ebenso
 *   GLEICH, ohne -Og nicht). (Namensfassung mit schweif.h: 19.10.2026 GLEICH, LINKGLEICH)
 * - C++ wegen Stash (O238) / Chest (O078, 0x55 Bytes) als Blocklocals und new/delete, inline-Setter der Gruppe
 *   (set_town/set_loctype/set_dungeon/set_route_from/set_route_to) und inline-Getter hero::get_npc_id().
 * - automap_cell_write ueber AUTOMAP_CELL_WRITE_W (letzter Parameter short, wie OVL063); die Brunnen-Sperre als
 *   g_ingame_timers[INGAME_TIMER_DNG06_WELL] (g_ingame_timers + 0x88, kein eigenes Public).
 * - Stack: die Parameterkopien der verschachtelten Setter (Typ 0x6a) liegen direkt hinter den Funktionslocals
 *   ([bp-0dh]/[bp-10h]/[bp-11h]), dann die this-Kopie von "group_get_npc(g)->get_npc_id()" ([bp-16h], 4 Bytes), dann
 *   das -Og-Byte loctype ([bp-17h]), erst danach die Blocklocals der Geschwisterbloecke ([bp-1ah]/[bp-1ch]) und die
 *   grossen Objekte (Stash [bp-32h], Chest [bp-72h], ueberlappend). Eine echte Variable statt des Getters landet vor
 *   den Kopien (Funktionsebene) bzw. verschiebt alle Blocklocals (Blockebene).
 * - "g_dng06_noise += r" (char += int) -> mov al,[g] / add al,[r] / mov; dng06_latrine_ambush braucht "g_dng06_noise
 *   = g_dng06_noise + g_fight_rounds" (mit += zwischen zwei char: mov al,[8b68] / add [265c],al).
 * Originalfehler: dng06_handler MAP_EVENT_ENTER - die in den Positionszweigen gesetzten Rueckgaben (ret = 1, ret =
 *   blut_rockfall(..)) werden am Ende durch "ret = dng06_fights(ev, pos)" ueberschrieben.
 */
#include <stdio.h>

#include "schweif.h"

/* automap_cell_write mit short als letztem Parameter (wie im Original deklariert) */
#define AUTOMAP_CELL_WRITE_W \
	((void (*)(void far *, signed short, signed short, signed short, signed short))automap_cell_write)

signed short dng06_handler(struct area_events far *ev, struct map_event far *mev)
{
	register signed short type;
	register signed short pos;
	struct hero far *h;
	signed short r;
	signed short ret;
	char far *msg;

	type = mev->type;
	pos = (mev->y - g_map_x0) << 8;
	pos |= mev->x - g_map_y0;
	h = get_first_active_hero(g_group);
	ret = 0;
	if (type == MAP_EVENT_ENTER) {
		if (pos == 0x1b04 || pos == 0x1c04 || pos == 0x1b05 || pos == 0x1c05) {
			r = count_failed_skill(g_group, TA_SCHLEICHEN, 2);
			g_dng06_noise += r;
			blut_nop();
			if (r != 0 && !g_dng06_orcs_1b04) {
				g_dng06_noise += 5;
				if (!event_fight1(ev, 0x1b, 1, 0))
					g_dng06_orcs_1b04 = 1;
			}
		} else if (pos == 0x1505) {
			g_dng06_noise += count_failed_skill(g_group, TA_SCHLEICHEN, 0);
			blut_nop();
		} else if (pos == 0x0404) {
			r = count_failed_skill(g_group, TA_SCHLEICHEN, 1);
			g_dng06_noise += r;
			blut_nop();
		} else if (pos == 0x0b05) {
			r = count_failed_skill(g_group, TA_SCHLEICHEN, 0);
			g_dng06_noise += r * 2;
			blut_nop();
		} else if (pos == 0x0f05) {
			r = count_failed_skill(g_group, TA_SCHLEICHEN, 1);
			g_dng06_noise += r * 2;
			blut_nop();
		} else if (pos == 0x1209) {
			r = count_failed_skill(g_group, TA_SCHLEICHEN, 2);
			g_dng06_noise += r * 3;
			blut_nop();
		} else if (pos == 0x1317) {
			r = count_failed_skill(g_group, TA_SCHLEICHEN, 2);
			g_dng06_noise += r * 3;
			blut_nop();
		} else if (pos == 0x1619) {
			r = count_failed_skill(g_group, TA_SCHLEICHEN, 2);
			g_dng06_noise += r;
			blut_nop();
		} else if (pos == 0x141b) {
			r = count_failed_skill(g_group, TA_SCHLEICHEN, 0);
			g_dng06_noise += r * 2;
			blut_nop();
		} else if (pos == 0x181a || pos == 0x181b || pos == 0x1919 || pos == 0x191a) {
			r = count_failed_skill(g_group, TA_SCHLEICHEN, 1);
			g_dng06_noise += r;
			blut_nop();
		} else if (pos == 0x0b14) {
			r = count_failed_skill(g_group, TA_SCHLEICHEN, 0);
			g_dng06_noise += r * 5;
			blut_nop();
		} else if (pos == 0x0e11) {
			if (count_failed_skill(g_group, TA_SCHLEICHEN, 2) > 0) {
				g_dng06_noise += 3;
				blut_nop();
				if (!g_dng06_orcs_0e10) {
					if (!event_fight1(ev, 0x1b, 1, 0))
						g_dng06_orcs_0e10 = 1;
				}
			} else
				blut_nop();
		} else if (!g_dng06_orcs_0e10 && pos == 0x0e10) {
			if (!event_fight1(ev, 0x1b, 1, 0))
				g_dng06_orcs_0e10 = 1;
		} else if (pos == 0x070f) {
			g_dng06_bolt_wall_armed = 1;
		} else if (pos == 0x0711) {
			g_dng06_bolt_wall_armed = 0;
		} else if (pos == 0x0710) {
			/* BLUT1.LTX[114]: "In der Wand koennt Ihr in einem scheinbar zufaelligen Muster verteilt etwa ein Dutzend
			 * Loecher fuer Armbrustbolzen oder aehnliches erkennen. Einen Mechanismus entdeckt Ihr jedoch nicht." */
			dlg_message(g_dialog, ev->txt[0x72]);
			if (g_dng06_bolt_wall_armed != 0) {
				/* BLUT1.LTX[115]: "Kaum wendet Ihr den Blick ab, werdet Ihr auch schon von Armbrustbolzen getroffen." */
				dlg_message(g_dialog, ev->txt[0x73]);
				group_sub_le_range(g_group, 8, 0x18);
			}
		} else if (pos == 0x0914) {
			g_dng06_beetle_pit_armed = 1;
		} else if (pos == 0x0714) {
			g_dng06_beetle_pit_armed = 0;
		} else if (pos == 0x0814) {
			dng06_beetle_pit(ev, h);
			ret = 1;
		} else if (!g_dng06_tripwire_seen && pos == 0x1819) {
			if (test_skill(h, TA_SINNESSCHAERFE, 4) >= 0) {
				/* BLUT1.LTX[75]: "<Hoppla!> meint %s und deutet auf einen hauchduennen Stolperdraht, der nur wenige
				 * Fingerbreit ueber dem Boden gespannt ist.@<Da waeren wir beinahe ueber einen Alarmdraht gestolpert!>" */
				sprintf(g_text_output_buf, ev->txt[0x4b], h->name);
				dlg_message(g_dialog, g_text_output_buf);
				g_dng06_tripwire_seen = 1;
			} else
				g_dng06_tripwire_hit = 1;
		} else if (!g_dng06_helmet_taken && pos == 0x090a) {
			/* BLUT1.LTX[17]: "Hier liegt ein verbeulter, aber offensichtlich noch brauchbarer Topfhelm.@Wollt Ihr ihn
			 * mitnehmen?" */
			if (dlg_yes_no(g_dialog, ev->txt[0x11]) == 1) {
				group_give_item(g_group, ITEM_POT_HELMET, 1);
				g_dng06_helmet_taken = 1;
			}
		} else if (pos == 0x1305) {
			dng06_pit(ev, &g_dng06_pit_1305);
		} else if (pos == 0x1519) {
			dng06_pit(ev, &g_dng06_pit_1519);
		} else if (pos == 0x0307 || pos == 0x0709 || pos == 0x030c) {
			/* BLUT1.LTX[37]: "Von der Decke prasseln Steinbrocken auf Euch herab!" */
			ret = blut_rockfall(ev, &g_dng06_rockfall1, -5, 3, 1, 3, 5, 10, ev->txt[0x25]);
		} else if (pos == 0x030a || pos == 0x060c) {
			ret = blut_rockfall(ev, &g_dng06_rockfall2, -1, 1, 2, 5, 8, 12, ev->txt[0x25]);
		} else if (!g_dng06_skeleton_dug && pos == 0x0410) {
			dng06_dwarf_skeleton(ev);
			ret = 1;
		} else if (pos == 0x1a0c) {
			Stash far *l = new Stash(0x0e, 1);
			/* BLUT1.LTX[55]: "Hier lagern eine Menge Speerspitzen, einige Dolche ohne Griff, eine Spitzhacke und ein
			 * gebrauchsfertiger Saebel.@Wollt Ihr etwas davon an Euch nehmen?" */
			if (l->has_items() == 1 && dlg_yes_no(g_dialog, ev->txt[0x37]) == 1)
				l->menu();
			delete l;
			ret = 1;
		} else if (pos == 0x170f) {
			if (!g_dng06_dogs_state) {
				snd_play(g_snd_fx, 0x7f);
				/* BLUT1.LTX[59]: "Ihr habt kaum einen Schritt in den Raum getan, als in einiger Entfernung mehrere Hunde wild
				 * zu bellen und zu knurren beginnen." */
				dlg_message(g_dialog, ev->txt[0x3b]);
				ret = 1;
			}
		} else if (pos == 0x180e) {
			/* BLUT1.LTX[67]: "Hier lagert ein Haufen Hundefutter von zweifelhaftem Geruch und noch weit zweifelhafterer
			 * Herkunft.@Wollt Ihr den haufen naeher untersuchen?" */
			if (dlg_yes_no(g_dialog, ev->txt[0x43]) == 1) {
				timewarp_minutes(g_date, 2);
				/* BLUT1.LTX[68]: "Ihr bemerkt sehr schnell, dass hier wohl nichts zu finden ist." */
				dlg_message(g_dialog, ev->txt[0x44]);
			}
			ret = 1;
		} else if (pos == 0x1a0e) {
			/* BLUT1.LTX[69]: "Hier liegt ein Stapel mit fuer die Hunde zugeschnittenen Lederharnischen." */
			dlg_message(g_dialog, ev->txt[0x45]);
			ret = 1;
		} else if (pos == 0x1715 && !g_dng06_ogre_weapons_smashed) {
			/* BLUT1.LTX[73]: "Hier haben die Oger ihr <schweres Geraet> gelagert: monstroese Kriegshaemmer und riesige
			 * Keulen, ueberdimensionale Aexte und Kriegsflegel. Alle Waffen sind zu schwer, um sie zu transportieren oder
			 * gar zu benutzen.@Wollt Ihr die Waffen zerstoeren?" */
			if (dlg_yes_no(g_dialog, ev->txt[0x49]) == 1) {
				timewarp_minutes(g_date, 0x19);
				g_dng06_noise += 5;
				/* BLUT1.LTX[74]: "Es dauert zwar eine Weile, aber schliesslich sind die schweren Waffen der Oger nur noch ein
				 * Haufen nutzloser Schrott." */
				dlg_message(g_dialog, ev->txt[0x4a]);
				g_dng06_ogre_weapons_smashed = 1;
			}
			ret = 1;
		} else if (pos == 0x161d) {
			if (!g_dng06_latrine_seen) {
				/* BLUT1.LTX[86]: "Dieser Raum dient offensichtlich als Latrine, was sowohl die Loecher im Boden, als auch der
				 * bestialische Gestank bezeugen." */
				dlg_message(g_dialog, ev->txt[0x56]);
				g_dng06_latrine_seen = 1;
			}
			if (random_schweif(100) <= 10 && !event_fight1(ev, 0x1b, 1, 0))
				dng06_latrine_ambush((struct area_events far *)g_area_events);
			ret = 1;
		} else if (pos == 0x1012) {
			/* BLUT1.LTX[92]: "Hier lagern eine Menge Waffen.@Wollt Ihr Euch bedienen?" */
			if (dlg_yes_no(g_dialog, ev->txt[0x5c]) == 1) {
				Stash l(0x0f, 0);
			}
			ret = 1;
		} else if (!g_dng06_hall_seen && pos == 0x1117) {
			/* BLUT1.LTX[93]: "Ihr kommt in eine grosse Halle, die vom Schein mehrerer Feuer und Fackeln erhellt wird." */
			dlg_message(g_dialog, ev->txt[0x5d]);
			g_dng06_hall_seen = 1;
		}
#if FIX_0182
		/* Fix #182: Rueckgabe der Positionszweige nicht ueberschreiben, wenn kein Kampf stattfand */
		{
			signed short f = dng06_fights(ev, pos);
			if (f != 0)
				ret = f;
		}
#else
		ret = dng06_fights(ev, pos);
#endif
	} else if (type == 1) {
		ret = dng06_door(ev, pos, mev->x, mev->y, mev);
	} else if (type == 0x64) {
		dng06_straw(ev, h, pos);
		ret = 1;
	} else if (type == 0x65) {
		msg = 0;
		if (pos == 0x1e1c) {
			if (!g_dng06_hay_state) {
				/* BLUT1.LTX[78]: "In diesem Fass wird offensichtlich Korn als Pferdefutter gelagert. Neben dem Fass koennt Ihr
				 * mehrere grosse Ballen Heu erkennen.@Wollt Ihr" */
				/* BLUT1.LTX[79]: "das Heu anzuenden" */
				/* BLUT1.LTX[80]: "Feuer legen" */
				/* BLUT1.LTX[81]: "das Heu Heu sein lassen" */
				r = dlg_choose(g_dialog, ev->txt[0x4e], 3, ev->txt[0x4f], ev->txt[0x50],
					ev->txt[0x51]);
				if (r == 1 || r == 2) {
					if (group_find_item(g_group, ITEM_TORCH_ON) >= 0 || group_find_item(g_group,
						ITEM_LANTERN_ON) >= 0 ||
						group_find_item(g_group, ITEM_TINDERBOX) >= 0) {
						g_dng06_hay_state = 1;
						g_dng06_alarm_delay = 0x1c2;
						group_set_screen_pos(0x18, 0x19);
						/* BLUT1.LTX[83]: "Nach kurzer Zeit flammt das Heu auf. Beissender Qualm zwingt Euch, den Raum zu verlassen." */
						msg = ev->txt[0x53];
					} else
						/* BLUT1.LTX[82]: "Ihr habt nichts bei Euch, um ein Feuer zu entzuenden." */
						msg = ev->txt[0x52];
				}
			} else if (g_dng06_hay_state == 1)
				/* BLUT1.LTX[85]: "Das Fass und das Heu brennt noch immer lichterloh und verbeitet einen hoellischen Qualm." */
				msg = ev->txt[0x55];
			else
				/* BLUT1.LTX[84]: "Hier hat es vor Kurzem bereits gebrannt." */
				msg = ev->txt[0x54];
		} else
			/* BLUT1.LTX[0]: "Vor Euch steht ein grosses Fass, offensichtlich bis zum Rand mit zaehfluessigem Pech
			 * gefuellt." */
			msg = ev->txt[0];
		if (msg != 0)
			dlg_message(g_dialog, msg);
		ret = 1;
	} else if (type == 0x66) {
		/* BLUT1.LTX[5]: "Leise schleicht Ihr die Treppe hinauf..." */
		dlg_message(g_dialog, ev->txt[5]);
		if (pos == 0x0101) {
			g_group->old_dungeon = g_group->dungeon;
			g_group->dungeon = 7;
			group_set_screen_pos(0x14, 2);
			g_group->dir = 0;
		} else {
			g_group->old_dungeon = g_group->dungeon;
			g_group->dungeon = 7;
			group_set_screen_pos(0x1d, 0x12);
			g_group->dir = 3;
		}
	} else if (type == 0x67) {
		signed short id = -1;
		switch (pos) {
		case 0x0502:
			id = 9;
			break;
		case 0x0904:
			id = 10;
			break;
		case 0x1b0e:
			id = 11;
			break;
		case 0x0517:
			id = 12;
			break;
		case 0x0a0d:
			id = 13;
			break;
		}
		if (id != -1) {
			Chest c(id, 1);
			ret = 1;
		}
	} else if (type == 0x68) {
		ret = dng06_stores(ev, pos);
	} else if (type == 0x69) {
		ret = dng06_weapon_rack(ev, pos);
	} else if (type == 0x6a) {
		/* BLUT1.LTX[14]: "Hier fuehrt eine flache Rampe hinaus. Das Tal vor Euch ist vom %s erhellt. Es scheint, als
		 * haettet Ihr es geschafft..." */
		/* BLUT1.LTX[15]: "Mondlicht" */
		/* BLUT1.LTX[16]: "Tageslicht" */
		sprintf(g_text_output_buf, ev->txt[0x0e],
			g_daytime <= 0x93a8L || g_daytime > 0x190c8L ? ev->txt[0x0f] : ev->txt[0x10]);
		dlg_message(g_dialog, g_text_output_buf);
		snd_obj_stop(g_snd_music);
		swap_out_screen();
		g_group->route_pos = 0;
		g_group->set_town(g_group->set_town(0));
		g_group->set_loctype(g_group->set_loctype(0));
		g_group->set_dungeon(g_group->set_dungeon(0));
		group_set_on_route(g_group, 1);
		g_group->set_route_from(0xab);
		g_group->set_route_to(0xab);
		if (group_get_npc(g_group)->get_npc_id() == 1)
			g_npc_months = 0x63;
		groups_check_npc();
		screen_portraits(g_screen);
	} else if (type == 0x6b) {
		dng06_brazoragh(ev);
	} else if (type == 0x6c) {
		/* BLUT1.LTX[48]: "Im Glutbecken der Esse liegt ein Rohling, der wohl zu einem Schwert geschmiedet werden soll.
		 * Ueber der Esse koennt Ihr einen Rauchabzug erkennen, der jedoch hoechstens einer Ratte als Durchschlupf dienen
		 * kann." */
		dlg_message(g_dialog, ev->txt[0x30]);
		ret = 1;
	} else if (type == 0x6d) {
		/* BLUT1.LTX[47]: "Ueber der Feuerstelle scheint sich eine schmale Ritze im Fels zu befinden, durch die der Rauch
		 * abziehen kann - viel zu schmal allerdings fuer einen von Euch. Ueber dem Feuer dreht sich am Spiess etwas, das
		 * wohl frueher einmal Euresgleichen war. Auch die abgenagten Knochen in der Naehe der Feuerstelle sprechen eine
		 * deutliche Sprache..." */
		dlg_message(g_dialog, ev->txt[0x2f]);
		ret = 1;
	} else if (type == 0x6e) {
		/* BLUT1.LTX[50]: "Dieses Wasserfass dient offensichtlich zum Abschrecken heissen Metalls." */
		dlg_message(g_dialog, ev->txt[0x32]);
		ret = 1;
	} else if (type == 0x6f) {
		if (pos == 0x170b) {
			/* BLUT1.LTX[51]: "Dies ist ein Wasserbecken, das mit einer Pumpe gefuellt werden kann.@Wollt Ihr von dem Wasser
			 * trinken und Eure Wasserschlaeuche wieder auffuellen?" */
			if (dlg_yes_no(g_dialog, ev->txt[0x33]) == 1) {
				timewarp_minutes(g_date, 5);
				snd_play(g_snd_fx, 0x5d);
				/* BLUT1.LTX[52]: "Ihr fuehlt Euch herrlich erfrischt." */
				dlg_message(g_dialog, ev->txt[0x34]);
				group_refill_water(g_group);
				if (g_ingame_timers[INGAME_TIMER_DNG06_WELL] == 0) {
					group_add_le(g_group, 1);
					group_add_effect(g_group, 0x546, 0, 1, 0);
					g_ingame_timers[INGAME_TIMER_DNG06_WELL] = 0x1fa40L;
				}
			}
		} else if (pos == 0x1b19) {
			/* BLUT1.LTX[76]: "Dies ist ein lange Traenke, die mit einer Pumpe gefuellt werden kann.@Wollt Ihr von dem
			 * Wasser trinken und Eure Wasserschlaeuche wieder auffuellen?" */
			if (dlg_yes_no(g_dialog, ev->txt[0x4c]) == 1) {
				timewarp_minutes(g_date, 5);
				snd_play(g_snd_fx, 0x5d);
				dlg_message(g_dialog, ev->txt[0x34]);
				group_refill_water(g_group);
				if (g_ingame_timers[INGAME_TIMER_DNG06_WELL] == 0) {
					group_add_le(g_group, 1);
					group_add_effect(g_group, 0x546, 0, 1, 0);
					g_ingame_timers[INGAME_TIMER_DNG06_WELL] = 0x1fa40L;
				}
			}
		} else if (pos == 0x091c) {
			/* BLUT1.LTX[109]: "Hier befindet sich ein kleiner Brunnen.@Wollt Ihr davon trinken und Eure Wasserschlaeuche
			 * auffuellen?" */
			if (dlg_yes_no(g_dialog, ev->txt[0x6d]) == 1) {
				timewarp_minutes(g_date, 5);
				snd_play(g_snd_fx, 0x5d);
				dlg_message(g_dialog, ev->txt[0x34]);
				group_refill_water(g_group);
				if (g_ingame_timers[INGAME_TIMER_DNG06_WELL] == 0) {
					group_add_le(g_group, 1);
					group_add_effect(g_group, 0x546, 0, 1, 0);
					g_ingame_timers[INGAME_TIMER_DNG06_WELL] = 0x1fa40L;
				}
			}
		}
	} else if (type == 0x70) {
		if (!g_dng06_hornets_destroyed && pos == 0x1807)
			dng06_hornets(ev);
		/* BLUT1.LTX[53]: "In dieser Ecke werden eiserne Rohlinge und Kohle in grossen Holzkisten gelagert.@Wollt Ihr die
		 * Kisten naeher untersuchen?" */
		else if (dlg_yes_no(g_dialog, ev->txt[0x35]) == 1)
			/* BLUT1.LTX[54]: "So sehr Ihr Euch auch abmueht, Ihr koennt nichts von Interesse entdecken." */
			dlg_message(g_dialog, ev->txt[0x36]);
		ret = 1;
	} else if (type == 0x71) {
		if (!g_dng06_dagger_taken) {
			/* BLUT1.LTX[56]: "Dieser Schrein ist wohl Gravesh, dem orkischen Schmiedegott gewidmet. In einer kleinen Nische
			 * liegt ein exzellent gearbeiteter Dolch.@Wollt Ihr ihn mitnehmen?" */
			if (dlg_yes_no(g_dialog, ev->txt[0x38]) == 1) {
				/* BLUT1.LTX[57]: "Ihr packt den Dolch ein." */
				dlg_message(g_dialog, ev->txt[0x39]);
				group_give_item(g_group, ITEM_HEAVY_DAGGER, 1);
				g_dng06_dagger_taken = 1;
				g_god_favor_ingerimm -= 50;
			}
		} else
			/* BLUT1.LTX[58]: "Dieser Schrein ist wohl Gravesh, dem orkischen Schmiedegott gewidmet." */
			dlg_message(g_dialog, ev->txt[0x3a]);
	} else if (type == 0x72) {
		if (!g_dng06_dogs_state) {
			snd_play(g_snd_fx, 0x7f);
			/* BLUT1.LTX[60]: "In diesem Kaefig aus stabilem Holz werden orkische Kriegshunde gefangen gehalten, die trotz
			 * der Gitterstaebe wild nach Euch schnappen.@Wollt Ihr" */
			/* BLUT1.LTX[61]: "das Gitter oeffnen" */
			/* BLUT1.LTX[62]: "die Hunde durch das Gitter umbringen" */
			r = dlg_choose(g_dialog, ev->txt[0x3c], 2, ev->txt[0x3d], ev->txt[0x3e]);
			if (r == 1) {
				if (!event_fight1(ev, 0x5e, 4, 0)) {
					struct obj3d far *t;
					g_dng06_dogs_state = 1;
					t = g_cell_objs[19];
					t->nfaces -= 2;
				}
			} else if (r == 2) {
				struct obj3d far *t;
				g_dng06_dogs_state = 2;
				snd_play(g_snd_fx, 0xa3);
				/* BLUT1.LTX[66]: "Ihr benoetigt einige Minuten fuer Euer blutiges Handwerk. Es ist jetzt unheimlich still." */
				dlg_message(g_dialog, ev->txt[0x42]);
				snd_play(g_snd_fx, 0xa3);
				t = g_cell_objs[19];
				t->nfaces -= 2;
			} else {
				h = get_random_hero(g_group);
				/* BLUT1.LTX[65]: "%s ist fuer einen Moment unachtsam und wird von einem der Hunde in den Unterarm gebissen." */
				sprintf(g_text_output_buf, ev->txt[0x41], h->name);
				sub_hero_le(h, dice_roll(1, 6, 2));
				dlg_message(g_dialog, g_text_output_buf);
			}
		} else if (g_dng06_dogs_state == 1)
			/* BLUT1.LTX[63]: "Der Kaefig ist leer." */
			dlg_message(g_dialog, ev->txt[0x3f]);
		else
			/* BLUT1.LTX[64]: "In diesem Kaefig liegen die niedergemetzelten orkischen Kriegshunde." */
			dlg_message(g_dialog, ev->txt[0x40]);
		ret = 1;
	} else if (type == 0x73) {
		/* BLUT1.LTX[77]: "Hier steht eine grob gezimmerte Pferdebox. Es sind jedoch nirgendwo Pferde zu entdecken." */
		dlg_message(g_dialog, ev->txt[0x4d]);
		ret = 1;
	} else if (type == 0x74) {
		/* BLUT1.LTX[91]: "Hier gaehnt ein grosses, finsteres Loch in der Wand. Dahinter fuehrt ein Schacht steil nach
		 * unten in die Dunkelheit. Der Gestank aus dem Loch ist schier unertraeglich." */
		dlg_message(g_dialog, ev->txt[0x5b]);
		ret = 1;
	} else if (type == 0x75) {
		dng06_beer_barrel(ev, h, pos);
		ret = 1;
	} else if (type == 0x76) {
		if (pos == 0x0a17) {
			/* BLUT1.LTX[100]: "In diesem Kessel blubbert irgendeine Art Eintopf vor sich hin.@Wollt Ihr" */
			/* BLUT1.LTX[101]: "die Kochstelle untersuchen" */
			/* BLUT1.LTX[102]: "den Eintopf essen" */
			r = dlg_choose(g_dialog, ev->txt[0x64], 2, ev->txt[0x65], ev->txt[0x66]);
			if (r == 1) {
				timewarp_minutes(g_date, 7);
				/* BLUT1.LTX[103]: "Der Eintopf sieht unverfaenglich, wenn auch nicht besonders schmackhaft aus. Ueber der
				 * Feuerstelle koennt Ihr einen schmalen Rauchabzug erkennen." */
				dlg_message(g_dialog, ev->txt[0x67]);
			} else if (r == 2) {
				if (!g_dng06_stew_eaten) {
					timewarp_minutes(g_date, 5);
					/* BLUT1.LTX[104]: "Der Eintopf saettigt wie eine Wegzehrung." */
					dlg_message(g_dialog, ev->txt[0x68]);
					g_dng06_stew_eaten = 1;
					group_clr_hunger(g_group);
				} else
					/* BLUT1.LTX[105]: "Nein danke, kein Appetit!" */
					dlg_message(g_dialog, ev->txt[0x69]);
			}
		} else if (pos == 0x0917) {
			/* BLUT1.LTX[106]: "In diesem Kessel schwimmen grosse Fleischstuecke in einer Sosse, die bereits auf einen
			 * halben Schritt Entfernung infernalisch scharf wirkt.@Wollt Ihr" */
			r = dlg_choose(g_dialog, ev->txt[0x6a], 2, ev->txt[0x65], ev->txt[0x66]);
			if (r == 1) {
				timewarp_minutes(g_date, 7);
				/* BLUT1.LTX[107]: "Das Gulasch scheint aus einer Art Hylailer Feuer zubereitet - und die Fleischbrocken
				 * koennten gut und gerne aus einem Drachen herausgeschnitten sein. Ueber der Feuerstelle koennt Ihr einen
				 * schmalen Rauchabzug erkennen." */
				dlg_message(g_dialog, ev->txt[0x6b]);
			} else if (r == 2) {
				if (!g_dng06_stew_eaten) {
					timewarp_minutes(g_date, 5);
					/* BLUT1.LTX[108]: "Nicht schlecht, aber wirklich hoellisch scharf!" */
					dlg_message(g_dialog, ev->txt[0x6c]);
					g_dng06_stew_eaten = 1;
					group_clr_hunger(g_group);
					group_sub_le(g_group, 1);
				} else
					dlg_message(g_dialog, ev->txt[0x69]);
			}
		}
		ret = 1;
	} else if (type == 0x77) {
		/* BLUT1.LTX[110]: "Auf dem Tisch liegen einige Kochgeraete, ein paar Gewuerze, aber nichts von wirklichem
		 * Interesse." */
		dlg_message(g_dialog, ev->txt[0x6e]);
		ret = 1;
	} else if (type == 0x78) {
		/* BLUT1.LTX[113]: "Der Altar scheint aus einem Block gehauen zu sein. Auf der Oberseite erkennt Ihr eine flache
		 * Mulde, in der sich Reste von getrocknete Blut befinden." */
		dlg_message(g_dialog, ev->txt[0x71]);
		ret = 1;
	} else if (type == 0x79) {
		if (pos == 0x0a14) {
			if (test_skill(h, TA_SINNESSCHAERFE, 10) >= 0) {
				/* BLUT1.LTX[119]: "%s wirft einen Blick auf die Wand und meint dann ploetzlich <Das ist doch ein
				 * Schluesselloch hier!>@Mit dem Finger deutet %s auf eine kleine Oeffnung zwischen zwei Steinen." */
				sprintf(g_text_output_buf, ev->txt[0x77], h->name, grammar_pronoun(h->sex, 0, 0));
				dlg_message(g_dialog, g_text_output_buf);
				if (group_find_key(g_group, 0x10a) >= 0 || group_find_key(g_group, 0x111) >= 0) {
					/* BLUT1.LTX[121]: "Nach kurzem hat %s herausgefunden, welcher Schluessel in das Schloss passt und kaum dreht
					 * %s ihn im Schloss, loest sich die Wand vor Euch, wie von Geisterhand auf." */
					sprintf(g_text_output_buf, ev->txt[0x79], h->name, grammar_pronoun(h->sex,
						0, 0));
					dlg_message(g_dialog, g_text_output_buf);
					event_set_cell(ev, 0x14, 0x0a, 1, 0, -1);
					AUTOMAP_CELL_WRITE_W(g_map, 0x377, 0x14, 0x0a, 8);
				} else
					/* BLUT1.LTX[120]: "Leider habt Ihr keinen Schluessel bei Euch, der hier passt." */
					dlg_message(g_dialog, ev->txt[0x78]);
			}
		} else if (pos == 0x0b0e) {
			if (test_skill(h, TA_SINNESSCHAERFE, 10) >= 0) {
				sprintf(g_text_output_buf, ev->txt[0x77], h->name, grammar_pronoun(h->sex, 0, 0));
				dlg_message(g_dialog, g_text_output_buf);
				if (group_find_key(g_group, 0x10a) >= 0 || group_find_key(g_group, 0x111) >= 0) {
					sprintf(g_text_output_buf, ev->txt[0x79], h->name, grammar_pronoun(h->sex,
						0, 0));
					dlg_message(g_dialog, g_text_output_buf);
					event_set_cell(ev, 0x0e, 0x0b, 1, 0, -1);
					AUTOMAP_CELL_WRITE_W(g_map, 0x377, 0x0b, 0x0e, 8);
				} else
					dlg_message(g_dialog, ev->txt[0x78]);
			}
		}
	}
	return ret;
}

signed short dng06_fights(struct area_events far *ev, signed short pos)
{
	register signed short a = 0;
	register signed short b = 0;
	signed short c = 0;
	signed short ret = 0;

	if (pos == 0x1d02 && !g_dng06_orc_veteran_1d02) {
		if (g_dng06_alarm != 0)
			a = 4;
		else
			a = 1;
		if (!event_fight1(ev, 0x78, a, 2))
			g_dng06_orc_veteran_1d02 = 1;
	}
	if (pos == 0x1503 && !g_dng06_orcs_1503) {
		if (g_dng06_alarm != 0) {
			a = 6;
			b = 2;
			c = 5;
		} else if (g_daytime > 0x93a8L && g_daytime <= 0x190c8L) {
			a = 2;
			b = 2;
			c = 4;
		} else {
			a = 1;
			b = 1;
			c = 5;
		}
		if (a != 0 || b != 0 || c != 0) {
			if (!event_fight(ev, 2, a, 0, 0, 0x1b, b, 0, 0, 0x1b, c, 0, 5))
				g_dng06_orcs_1503 = 1;
			ret = 1;
		}
	} else if (pos == 0x0f03 && !g_dng06_orcs_0f03) {
		if (g_dng06_alarm != 0) {
			a = 5;
			b = 3;
			c = 6;
		} else if (g_daytime > 0x93a8L && g_daytime <= 0x190c8L) {
			a = 1;
			b = 3;
			c = 4;
		} else {
			a = 2;
			b = 1;
			c = 5;
		}
		if (a != 0 || b != 0 || c != 0) {
			if (!event_fight(ev, 2, a, 0, 0, 0x1b, b, 0, 0, 0x1b, c, 0, 5))
				g_dng06_orcs_0f03 = 1;
			ret = 1;
		}
	} else if (pos == 0x0402 && !g_dng06_orcs_0402) {
		if (g_dng06_alarm != 0) {
			a = 1;
			b = 4;
			c = 5;
		} else {
			a = 1;
			b = 1;
			c = 3;
		}
		if (a != 0 || b != 0 || c != 0) {
			if (!event_fight(ev, 2, a, 0, 0, 0x1b, b, 0, 0, 0x1b, c, 0, 5))
				g_dng06_orcs_0402 = 1;
			ret = 1;
		}
	} else if (pos == 0x1009 && !g_dng06_orcs_1009) {
		if (g_dng06_alarm != 0) {
			a = 5;
			b = 2;
			c = 5;
		} else if (g_daytime > 0x93a8L && g_daytime <= 0x190c8L) {
			a = 2;
			b = 2;
			c = 3;
		} else {
			a = 1;
			b = 1;
			c = 4;
		}
		if (a != 0 || b != 0 || c != 0) {
			if (!event_fight(ev, 2, a, 0, 0, 0x1b, b, 0, 0, 0x1b, c, 0, 4))
				g_dng06_orcs_1009 = 1;
			ret = 1;
		}
	}
	return ret;
}

void dng06_latrine_ambush(struct area_events far *ev)
{
	g_dng06_noise = g_dng06_noise + g_fight_rounds;
	g_dng06_alarm_delay = 0x708;
	/* BLUT1.LTX[87]: "Vor Euren Fuessen liegt nun ein toter Ork." */
	/* BLUT1.LTX[88]: "Wollt Ihr ihn in den Schacht werfen?" */
	/* BLUT1.LTX[89]: "Na und?" */
	if (dlg_choose(g_dialog, ev->txt[0x57], 2, ev->txt[0x58], ev->txt[0x59]) == 1) {
		g_dng06_alarm_delay = 0;
		snd_play(g_snd_fx, 0x70);
		/* BLUT1.LTX[90]: "Ihr zieht den Ork zu einem grossen Schacht in der Suedwand und lasst die Leiche dort
		 * verschwinden. Nach wenigen Sekunden hoert Ihr einen dumpfen Aufprall." */
		dlg_message(g_dialog, ev->txt[0x5a]);
		group_set_screen_pos(0x16, 0x1e);
	}
}
