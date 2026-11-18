/* OVL267 - Overlay O267 (Segment 0x24d2 in SCHWEIF.EXE): Ereignisse des Dungeons 8, der dritten Ebene der Orkfestung
 * (Texte BLUT3.LTX = Datei 0x375; g_area_descs[8]); Ereignisobjekt ev (struct area_events, Texte ev->txt),
 * Kartenereignis mev. Unterfunktionen in O268 (dng08_door, dng08_fights, dng08_shelves, dng08_pit).
 * dng08_handler(ev, mev): pos = (Zeile - g_map_x0) << 8 | (Spalte - g_map_y0), h = erster handlungsfaehiger Held.
 *   MAP_EVENT_ENTER: Laermfelder - misslungene Schleichen-Proben erhoehen g_dng07_noise (x1..x6), blut_nop; 0xc15/
 *   0x415 Blick in die grosse Halle (BLUT3[50]; Laerm -> [51], 100 Minuten, zu 5 % Alarm [53] mit Kampf, sonst [52]);
 *   0x213 Bolzenfalle (bis entschaerft bzw. Hebel umgelegt: [78], 8..15 LE, g_dng08_noise + 10); 0xd06 Schmiede [0];
 *   0xa02 Materiallager [11]/[12]; 0x1503 [15]; 0x1602 Handbeil des Ogers [16]; 0x1907 Raum des Schamanen [26];
 *   Klappgrube 0xe15 (scharf ueber 0xd15/0xf15, nur bei Alarm -> dng08_pit); Latrine [88] (0x181b/0x818 merken,
 *   0x181c/0x918 zeigen); danach immer ret = dng08_fights(ev, pos). Typ 1 dng08_door. 0x64 Strohlager ([28] Oger,
 *   [29] Schamane, [46] zerwuehlt, [47] eingesaut mit Wirkung, [48] Wirselkraut, [58] goldenes Halsband, [66] Schnaps
 *   und Schwert). 0x65 dng08_shelves. 0x66 Treppe nach Dungeon 7 ([87]). 0x67 Truhen (Chest 0x13..0x20), Plunderkiste
 *   [44], Ablage 0x33, Kriegsbanner [13]/[14]. 0x6a Ausgang ([75], Mond-/Tageslicht [76]/[77]): wie Dungeon 6,
 *   unterwegs am Ort 0xaa. 0x6c Esse [1], 0x6d Feuerstelle [59], 0x6e Wasserfaesser ([2]; [3]/[4] auffuellen, einmal
 *   je INGAME_TIMER_DNG08_WELL LE + 1). 0x76 Gulasch ([60..65]). 0x79 Wand (0x0b, 0x19) verschwindet ("Wow!"). 0x82
 *   Gravesh-Schrein ([5..10], Goetter/Kulte +4 erkennt Ingerimm: Streitkolben nehmen = Ingerimm -50, zerschlagen =
 *   Ingerimm -1000000). 0x83 Kaestchen mit Mumienhand ([21..25], [89]: Hand nehmen W10 LE, Kupferscheibe). 0x84
 *   Schamanenlager [27]. 0x85 Sarkophag ([33..43]: untersuchen; oeffnen mit zwei Helden KK-Summe >= 38 ->
 *   Zwergenmumie, Kampf 0x5f). 0x87 Bolzenfallen (dng08_bolt_trap mit Zuschlag 3 an 0x1500, sonst 5). 0x88 Hebel
 *   ([83..86], schaltet g_dng08_lever).
 * dng08_bolt_trap(ev, mod, flag): Bolzenfalle entschaerfen ([79], Held mit vorgewaehlter Schloesser-Probe [80]):
 *   gelungen [82], sonst [81], 8..15 LE, g_dng08_noise + 2.
 *
 * BCC -mlarge -3 -Ot -Og -Oi -c -Y OVL267.CPP -> 6727/6727 Bytes GLEICH (18.10.2026), cobjfix laeuft (ohne -Og 1779
 *   Abweichungen/Epilogkopien, ohne -Oi strcat als Aufruf). (Namensfassung mit schweif.h: 19.10.2026 GLEICH,
 *   LINKGLEICH)
 * - C++ wegen lokalem Chest-Objekt ("{ Chest c(id, 1); ret = 1; }") und new/delete der Stash-Ablage (O238).
 * - automap_cell_write ueber AUTOMAP_CELL_WRITE_W; Wasserfass-Sperre g_ingame_timers[INGAME_TIMER_DNG08_WELL].
 * - Tag/Nacht-Text: "g_daytime <= 0x93a8 || g_daytime > 0x190c8 ? txt[76] : txt[77]" (jng/jng-Folge), nicht DAY ? :.
 * - Stack: NPC-Test ist "group_get_npc(g_group)->get_npc_id() == 1" mit inline-Getter (this-Kopie [bp-1ah] HINTER den
 *   set_town/set_loctype/set_dungeon-Kopien, vor dem -Og-Byte [bp-1bh]); eine Variable h3 im 0x6a-Block macht die
 *   Setter-Kopien zu Block-Locals (Reihenfolge verdreht), auf Funktionsebene liegt sie vor ihnen.
 * - "Stash far *l" im 0x67-Block (hinter allen Zwischenwerten [bp-20h]), Chest [bp-76h].
 * Originalfehler/Auffaelligkeiten: Feld 0x1308 steht zweimal in der Kette (zweiter Zweig mit Zuschlag 2 und Faktor 3
 *   ist toter Code). MAP_EVENT_ENTER: alle "ret = 1" werden vom abschliessenden ret = dng08_fights ueberschrieben.
 *   0xc15/0x415 identisch (eigene Merker). 0x213 schadet bei jedem Betreten, bis die Falle entschaerft oder der Hebel
 *   umgelegt ist. Der Laerm von Dungeon 8 landet in g_dng07_noise (wie in Dungeon 7).
 */
#include <stdio.h>
#include <string.h>

#include "schweif.h"

/* Phase B: eigene Daten des Moduls (ds:78cc..78d2, dsana) - initialisierte Daten in Adressreihenfolge, danach der Literalpool */

/* automap_cell_write mit short als letztem Parameter (wie im Original deklariert) */
#define AUTOMAP_CELL_WRITE_W \
	((void (*)(void far *, signed short, signed short, signed short, signed short))automap_cell_write)

#define DAY (g_daytime > 0x93a8 && g_daytime <= 0x190c8)

signed short dng08_handler(struct area_events far *ev, struct map_event far *mev)
{
	signed short type = mev->type;
	signed short pos;
	struct hero far *h;
	struct hero far *h2;
	signed short n;
	signed short ret;

	pos = (mev->y - g_map_x0) << 8;
	pos |= mev->x - g_map_y0;
	h = get_first_active_hero(g_group);
	ret = 0;

	if (type == MAP_EVENT_ENTER) {
		if (pos == 0x1c11 || pos == 0x1518 || pos == 0x101b) {
			n = count_failed_skill(g_group, TA_SCHLEICHEN, 0);
			g_dng07_noise += n * 2;
			blut_nop();
		} else if (pos == 0x070a || pos == 0x090a || pos == 0x0718) {
			n = count_failed_skill(g_group, TA_SCHLEICHEN, -2);
			g_dng07_noise += n * 2;
			blut_nop();
		} else if (pos == 0x1308 || pos == 0x0211) {
			n = count_failed_skill(g_group, TA_SCHLEICHEN, 0);
			g_dng07_noise += n * 4;
			blut_nop();
		} else if (pos == 0x1308) {
			n = count_failed_skill(g_group, TA_SCHLEICHEN, 2);
			g_dng07_noise += n * 3;
			blut_nop();
		} else if (pos == 0x1803) {
			n = count_failed_skill(g_group, TA_SCHLEICHEN, 2);
			g_dng07_noise += n * 2;
			blut_nop();
		} else if (pos == 0x020f) {
			n = count_failed_skill(g_group, TA_SCHLEICHEN, 2);
			g_dng07_noise += n * 6;
			blut_nop();
		} else if (pos == 0x0f0e) {
			n = count_failed_skill(g_group, TA_SCHLEICHEN, 3);
			g_dng07_noise += n;
			blut_nop();
		} else if (pos == 0x0f0f || pos == 0x0f10 || pos == 0x0f13) {
			n = count_failed_skill(g_group, TA_SCHLEICHEN, 0);
			g_dng07_noise += n;
			blut_nop();
		} else if (pos == 0x1905) {
			n = count_failed_skill(g_group, TA_SCHLEICHEN, 3);
			g_dng07_noise += n * 3;
			blut_nop();
		} else if (pos == 0x1019) {
			n = count_failed_skill(g_group, TA_SCHLEICHEN, 1);
			g_dng07_noise += n * 2;
			blut_nop();
		} else if (pos == 0x0a15 || pos == 0x0615 || pos == 0x0a0d || pos == 0x060d) {
			n = count_failed_skill(g_group, TA_SCHLEICHEN, 6);
			g_dng07_noise += n * 4;
			blut_nop();
		} else if (pos == 0x0616 || pos == 0x0716 || pos == 0x0916 || pos == 0x0a16 ||
			   pos == 0x060c || pos == 0x070c || pos == 0x090c || pos == 0x0a0c) {
			n = count_failed_skill(g_group, TA_SCHLEICHEN, 3);
			g_dng07_noise += n * 5;
			blut_nop();
		} else if (pos == 0x0816 || pos == 0x080c) {
			n = count_failed_skill(g_group, TA_SCHLEICHEN, 0);
			g_dng07_noise += n * 2;
			blut_nop();
		} else if (pos == 0x0c15) {
			if (!g_dng08_hall_0c15) {
				g_dng08_hall_0c15 = 1;
				/* BLUT3.LTX[50]: "Durch den Durchgang erkennt Ihr eine grosse von Feuern erleuchtete Halle, in der sich
				 * anscheinend mehrere Gestalten bewegen. Ausserdem koennt Ihr den Laerm einer rauhen orkischen Unterhaltung
				 * vernehmen." */
				dlg_message(g_dialog, ev->txt[50]);
				n = count_failed_skill(g_group, TA_SCHLEICHEN, 0);
				g_dng07_noise += n * 10;
				if (n != 0) {
					h = get_random_hero(g_group);
					/* BLUT3.LTX[51]: "%s sollte vielleicht einmal lernen, sich etwas leiser zu bewegen. Es scheint als haetten
					 * Euch die Orks in der Halle bemerkt, denn in just diesem Moment verstummt die Unterhaltung!@Ihr presst Euch
					 * an die Wand und versucht, vollkommen unbeteiligtes Mauerwerk zu spielen. Die Zeit verrinnt..." */
					sprintf(g_text_output_buf, ev->txt[51], h->name);
					timewarp_minutes(g_date, 100);
					if (random_schweif(100) <= 5) {
						/* BLUT3.LTX[53]: "ALARM! Ihr habt es tatsaechlich geschafft!@Na dann Prost!" */
						dlg_message(g_dialog, ev->txt[53]);
						g_dng08_alarm = 1;
						event_fight(ev, 0x1b, 5, 0, 0, 0x1b, 3, 0, 2, 0x48, 3, 0, 4);
					} else
						/* BLUT3.LTX[52]: "...endlich wagt Ihr, wieder zu atmen. Die Orks scheinen zu ihrem wuesten Treiben
						 * zurueckgefunden zu haben und scheinen sich nicht um irgendwelche Geraeusche aus irgendeinem Gang zu
						 * scheren." */
						dlg_message(g_dialog, ev->txt[52]);
				}
				ret = 1;
			}
		} else if (pos == 0x0415) {
			if (!g_dng08_hall_0415) {
				g_dng08_hall_0415 = 1;
				dlg_message(g_dialog, ev->txt[50]);
				n = count_failed_skill(g_group, TA_SCHLEICHEN, 0);
				g_dng07_noise += n * 10;
				if (n != 0) {
					h = get_random_hero(g_group);
					sprintf(g_text_output_buf, ev->txt[51], h->name);
					timewarp_minutes(g_date, 100);
					if (random_schweif(100) <= 5) {
						dlg_message(g_dialog, ev->txt[53]);
						g_dng08_alarm = 1;
						event_fight(ev, 0x1b, 5, 0, 0, 0x1b, 3, 0, 2, 0x48, 3, 0, 4);
					} else
						dlg_message(g_dialog, ev->txt[52]);
				}
				ret = 1;
			}
		} else if (pos == 0x0213) {
			if (!g_dng08_trap_0213_defused && !g_dng08_lever) {
				h = get_random_hero(g_group);
				/* BLUT3.LTX[78]: "%s wird schwer von einem Bolzen getroffen und schreit laut auf!" */
				sprintf(g_text_output_buf, ev->txt[78], h->name);
				dlg_message(g_dialog, g_text_output_buf);
				sub_hero_le(h, random_interval(8, 15));
				g_dng08_noise += 10;
			}
		} else if (pos == 0x0d06 && !g_dng08_smithy_seen) {
			/* BLUT3.LTX[0]: "Ihr steht in einem grossen Raum, der offensichtlich als Schmiede dient." */
			dlg_message(g_dialog, ev->txt[0]);
			g_dng08_smithy_seen = 1;
			ret = 1;
		} else if (pos == 0x0b02) {
			g_dng08_storeroom_shown = 0;
		} else if (pos == 0x0a02 && !g_dng08_storeroom_shown) {
			/* BLUT3.LTX[11]: "Ihr steht in einem kleinen Raum, in dem offensichtlich Materialien fuer die Schmiede gelagert
			 * werden: Holz, Holzkohle und mehrere Barren Roheisen. So recht von Interesse ist davon allerdings
			 * nichts.@Wollt Ihr den Raum dennoch genauer unter die Lupe nehmen." */
			if (dlg_yes_no(g_dialog, ev->txt[11]) == 1) {
				timewarp_minutes(g_date, 25);
				/* BLUT3.LTX[12]: "Ihr findet leider nichts von Interesse." */
				dlg_message(g_dialog, ev->txt[12]);
			}
			g_dng08_storeroom_shown = 1;
			ret = 1;
		} else if (pos == 0x1503) {
			/* BLUT3.LTX[15]: "Irgendetwas ist hier nicht in Ordnung..." */
			dlg_message(g_dialog, ev->txt[15]);
		} else if (pos == 0x1602 && !g_dng08_hatchet_taken) {
			/* BLUT3.LTX[16]: "Hier liegt ein kleines Handbeil eines Ogers am Boden.@Wollt Ihr es mitnehmen?" */
			if (dlg_yes_no(g_dialog, ev->txt[16]) == 1) {
				group_give_item(g_group, ITEM_BATTLE_AXE, 1);
				g_dng08_hatchet_taken = 1;
			}
		} else if (pos == 0x1806) {
			g_dng08_shaman_room_shown = 0;
		} else if (pos == 0x1907 && !g_dng08_shaman_room_shown) {
			/* BLUT3.LTX[26]: "Der unregelmaessig geformte Raum wird von einigen Fackeln schwach erleuchtet. Der Boden ist
			 * mit Teppichen und Tierfellen bedeckt. Von der Decke haengen Kraeuterbuendel, seltsame Fetische und
			 * verschiedene Metallteile. Ein strenger Geruch nach Kraeutern erfuellt den Raum." */
			dlg_message(g_dialog, ev->txt[26]);
			g_dng08_shaman_room_shown = 1;
			ret = 1;
		} else if (pos == 0x0d15) {
			g_dng08_pit_armed = 1;
		} else if (pos == 0x0f15) {
			g_dng08_pit_armed = 0;
		} else if (pos == 0x0e15 && g_dng08_pit_armed != 0) {
			if (g_dng08_alarm != 0) {
				g_dng08_pit_0e15 = 1;
				dng08_pit(ev, &g_dng08_pit_0e15);
				ret = 1;
			}
		} else if (pos == 0x181b || pos == 0x0818) {
			g_dng08_latrine_armed = 1;
		} else if ((pos == 0x181c || pos == 0x0918) && g_dng08_latrine_armed == 1) {
			/* BLUT3.LTX[88]: "Dieser Raum dient offensichtlich als Latrine, was sowohl die Loecher im Boden, als auch der
			 * Gestank bezeugen." */
			dlg_message(g_dialog, ev->txt[88]);
			g_dng08_latrine_armed = 0;
			ret = 1;
		}
#if FIX_0194
		/* Fix #194: Rueckgabe der Positionszweige nicht ueberschreiben, wenn kein Kampf stattfand */
		{
			signed short f = dng08_fights(ev, pos);
			if (f != 0)
				ret = f;
		}
#else
		ret = dng08_fights(ev, pos);
#endif
	} else if (type == 1) {
		ret = dng08_door(ev, pos, mev->x, mev->y, mev);
	} else if (type == 0x64) {
		if (pos == 0x1a0a) {
			/* BLUT3.LTX[29]: "Das Strohlager wirkt einigermassen aufgeraeumt und sogar recht komfortabel.@Es scheint die
			 * Schlafstelle des Schamanen zu sein." */
			dlg_message(g_dialog, ev->txt[29]);
		} else if (pos == 0x1813) {
			if (!g_dng08_whirlweed_found) {
				/* BLUT3.LTX[48]: "Ein Haufen ziemlich zerwuehltes Stroh. Darunter entdeckt Ihr eine Portion Wirselkraut." */
				dlg_message(g_dialog, ev->txt[48]);
				group_give_item(g_group, ITEM_WHIRLWEED, 1);
				g_dng08_whirlweed_found = 1;
			} else
				/* BLUT3.LTX[46]: "Ein Haufen ziemlich zerwuehltes Stroh." */
				dlg_message(g_dialog, ev->txt[46]);
		} else if (pos == 0x0612) {
			if (!g_dng08_straw_0612) {
				/* BLUT3.LTX[66]: "Ein Haufen ziemlich zerwuehltes Stroh. Ihr findet unter dem Stroh versteckt eine Flasche
				 * Hochprozentiges und ein gut erhaltenes Schwert." */
				dlg_message(g_dialog, ev->txt[66]);
				group_give_item(g_group, ITEM_SWORD, 1);
				group_give_item(g_group, ITEM_BRANDY, 1);
				g_dng08_straw_0612 = 1;
			} else
				dlg_message(g_dialog, ev->txt[46]);
		} else if (pos == 0x1913 || pos == 0x1a13 || pos == 0x1411 || pos == 0x1412 || pos == 0x1715 ||
			   pos == 0x1716 || pos == 0x131b || pos == 0x131d || pos == 0x0c18 || pos == 0x0d18 ||
			   pos == 0x0e18 || pos == 0x0c1a || pos == 0x0d1a || pos == 0x0c12 || pos == 0x0d12 ||
			   pos == 0x1213 || pos == 0x1114 || pos == 0x1210 || pos == 0x1211 || pos == 0x0c10 ||
			   pos == 0x100c || pos == 0x0d0a || pos == 0x0e09 || pos == 0x0a12 || pos == 0x0a11) {
			dlg_message(g_dialog, ev->txt[46]);
		} else if (pos == 0x1413) {
			h = get_random_hero(g_group);
			/* BLUT3.LTX[47]: "Ein Haufen ziemlich zerwuehltes Stroh. Mit der orkischen Sauberkeit scheint es nicht so weit
			 * her zu sein. %s hat sich ziemlich eingesaut!" */
			sprintf(g_text_output_buf, ev->txt[47], h->name);
			dlg_message(g_dialog, g_text_output_buf);
			n = get_free_effect(h);
			add_effect(h, n, 0x1fa40L, 2, -1, 0);
		} else if (pos == 0x131c) {
			if (!g_dng08_collar_131c) {
				g_dng08_collar_131c = 1;
				/* BLUT3.LTX[58]: "Ein Haufen ziemlich zerwuehltes Stroh. Ihr findet unter dem Stroh versteckt ein goldenes
				 * Halsband." */
				dlg_message(g_dialog, ev->txt[58]);
				group_give_item(g_group, ITEM_GOLD_JEWELRY, 1);
			} else
				dlg_message(g_dialog, ev->txt[46]);
		} else if (pos == 0x0d10) {
			if (!g_dng08_collar_0d10) {
				g_dng08_collar_0d10 = 1;
				dlg_message(g_dialog, ev->txt[58]);
				group_give_item(g_group, ITEM_GOLD_JEWELRY, 1);
			} else
				dlg_message(g_dialog, ev->txt[46]);
		} else {
			/* BLUT3.LTX[28]: "Hier liegt ein grosser Haufen Stroh, der notduerftig von ein paar Decken zusammengehalten
			 * wird. Offensichtlich das Lager der Oger." */
			dlg_message(g_dialog, ev->txt[28]);
		}
		ret = 1;
	} else if (type == 0x65) {
		ret = dng08_shelves(ev, pos);
	} else if (type == 0x66) {
		if (pos == 0x1a02) {
			/* BLUT3.LTX[87]: "Vorsichtig steigt Ihr die Stufen hinab..." */
			dlg_message(g_dialog, ev->txt[87]);
			g_group->set_dungeon(7);
			group_set_screen_pos(0x1a, 6);
			g_group->dir = 1;
		} else {
			dlg_message(g_dialog, ev->txt[87]);
			g_group->set_dungeon(7);
			group_set_screen_pos(0x1d, 0x10);
			g_group->dir = 1;
		}
	} else if (type == 0x67) {
		signed short id = -1;
		Stash far *l;

		switch (pos) {
		/* BLUT3.LTX[44]: "Kaum vorstellbar, dass Ihr mit dem Plunder aus dieser Kiste etwas anfangen koennt: alte
		 * Tuecher, Teile von Flaschenzuegen, eine Speerspitze, ein zerschlagener Glasspiegel, ein Hundehalsband und
		 * dergleichen mehr." */
		case 0x180e: dlg_message(g_dialog, ev->txt[44]); break;
		case 0x1511: id = 0x13; break;
		case 0x1811: id = 0x14; break;
		case 0x1515: id = 0x15; break;
		case 0x111d: id = 0x16; break;
		case 0x0d13: id = 0x17; break;
		case 0x1214: id = 0x18; break;
		case 0x1111: id = 0x19; break;
		case 0x0f0a: id = 0x1a; break;
		case 0x031d:
			l = new Stash(0x33, 0);
			delete l;
			ret = 1;
			break;
		case 0x021c: id = 0x1b; break;
		case 0x1201: id = 0x1c; break;
		case 0x1203:
			/* BLUT3.LTX[13]: "Sorgfaeltig in gewachstes Tuch eingeschlagen ruht in dieser Truhe das Kriegsbanner der Orks,
			 * eine Fahne von gut 4^Rechstschritt Groesse, die mit allerlei blutruenstigen Szenen bestickt ist.@Wollt Ihr
			 * das Banner zerstoeren?" */
			if (!g_dng08_banner_torn && dlg_yes_no(g_dialog, ev->txt[13]) == 1) {
				/* BLUT3.LTX[14]: "Genuesslich zerreisst Ihr die Fahne in kleine Fetzen." */
				dlg_message(g_dialog, ev->txt[14]);
				g_dng08_banner_torn = 1;
			}
			break;
		case 0x1d03: id = 0x1d; break;
		case 0x1c06: id = 0x1e; break;
		case 0x1b09: id = 0x1f; break;
		case 0x0901: id = 0x20; break;
		}
		if (id != -1) {
			Chest c(id, 1);
			ret = 1;
		}
	} else if (type == 0x6a) {
		/* BLUT3.LTX[75]: "Hier fuehrt eine flache Rampe hinaus. Das Tal vor Euch ist vom %s erhellt. Es scheint, als
		 * haettet Ihr es geschafft..." */
		/* BLUT3.LTX[76]: "Mondlicht" */
		/* BLUT3.LTX[77]: "Tageslicht" */
		sprintf(g_text_output_buf, ev->txt[75],
			g_daytime <= 0x93a8 || g_daytime > 0x190c8 ? ev->txt[76] : ev->txt[77]);
		dlg_message(g_dialog, g_text_output_buf);
		snd_obj_stop(g_snd_music);
		swap_out_screen();
		g_group->route_pos = 0;
		g_group->set_town(g_group->set_town(0));
		g_group->set_loctype(g_group->set_loctype(0));
		g_group->set_dungeon(g_group->set_dungeon(0));
		group_set_on_route(g_group, 1);
		g_group->route_from = 0xaa;
		g_group->route_to = 0xaa;
		if (group_get_npc(g_group)->get_npc_id() == 1)
			g_npc_months = 0x63;
		groups_check_npc();
		screen_portraits(g_screen);
	} else if (type == 0x6d) {
		/* BLUT3.LTX[59]: "Eine Feuerstelle, die offensichtlich in erster Linie zum Kochen dient." */
		dlg_message(g_dialog, ev->txt[59]);
		ret = 1;
	} else if (type == 0x6c) {
		/* BLUT3.LTX[1]: "In einem Glutbecken der Esse liegen mehrere Rohlinge, die wohl zu Speerspitzen geschmiedet
		 * werden sollen. Ueber der Esse koennt Ihr einen Rauchabzug erkennen, der jedoch hoechstens einer Ratte als
		 * Durchschlupf dienen kann." */
		dlg_message(g_dialog, ev->txt[1]);
		ret = 1;
	} else if (type == 0x6e) {
		if (pos == 0x0f01) {
			/* BLUT3.LTX[2]: "Das Wasserfass dient offensichtlich zum Abschrecken heissen Metalls." */
			dlg_message(g_dialog, ev->txt[2]);
		/* BLUT3.LTX[3]: "Dieses Fass enthaelt klares, frisches Wasser.@Wollt Ihr etwas trinken und Eure Wasserschlaeuche
		 * auffuellen?" */
		} else if (dlg_yes_no(g_dialog, ev->txt[3]) == 1) {
			timewarp_minutes(g_date, 5);
			snd_play(g_snd_fx, 0x5d);
			/* BLUT3.LTX[4]: "Tut das gut!" */
			dlg_message(g_dialog, ev->txt[4]);
			group_refill_water(g_group);
			if (g_ingame_timers[INGAME_TIMER_DNG08_WELL] == 0) {
				group_add_le(g_group, 1);
				group_add_effect(g_group, 0x546L, 0, 1, 0);
				g_ingame_timers[INGAME_TIMER_DNG08_WELL] = 0x1fa40L;
			}
		}
		ret = 1;
	} else if (type == 0x76) {
		if (pos == 0x0810) {
			/* BLUT3.LTX[60]: "In diesem Kessel schwimmen grosse Fleischstuecke in einer Sosse, die bereits auf einen halben
			 * Schritt Entfernung infernalisch scharf wirkt.@Wollt Ihr" */
			/* BLUT3.LTX[61]: "die Kochstelle untersuchen" */
			/* BLUT3.LTX[62]: "den Eintopf essen" */
			n = dlg_choose(g_dialog, ev->txt[60], 2, ev->txt[61], ev->txt[62]);
			if (n == 1) {
				timewarp_minutes(g_date, 7);
				/* BLUT3.LTX[63]: "Das Gulasch scheint aus einer Art Hylailer Feuer zubereitet - und die Fleischbrocken
				 * koennten gut und gerne aus einem Drachen herausgeschnitten sein. Ueber der Feuerstelle koennt Ihr einen
				 * schmalen Rauchabzug erkennen." */
				dlg_message(g_dialog, ev->txt[63]);
			} else if (n == 2) {
				if (!g_dng08_goulash_eaten) {
					timewarp_minutes(g_date, 5);
					/* BLUT3.LTX[64]: "Nicht schlecht, aber wirklich hoellisch scharf!" */
					dlg_message(g_dialog, ev->txt[64]);
					g_dng08_goulash_eaten = 1;
					group_clr_hunger(g_group);
					group_sub_le(g_group, 1);
				} else
					/* BLUT3.LTX[65]: "Nein danke, kein Appetit!" */
					dlg_message(g_dialog, ev->txt[65]);
			}
		}
	} else if (type == 0x79) {
		dlg_message(g_dialog, "Wow!");
		event_set_cell(ev, 0x0b, 0x19, 0, 0, -1);
		AUTOMAP_CELL_WRITE_W(g_map, 0x37f, 0x19, 0x0b, 9);
	} else if (type == 0x82) {
		/* BLUT3.LTX[5]: "Dieser Schrein ist Gravesh, dem orkischen Schmiedegott gewidmet. In einer Nische in dem
		 * Heiligtum liegt ein haesslich, aber effektiv aussehender Streitkolben.@Wollt Ihr" */
		sprintf(g_text_output_buf, ev->txt[5]);
		if (test_skill(h, TA_GOETTER_KULTE, 4) > 0)
			/* BLUT3.LTX[10]: "@Euch faellt ein, dass nach allem, was Ihr ueber den Gravesh-Kult gehoert habt, hier niemand
			 * anderes als Ingerimm verehrt wird." */
			strcat(g_text_output_buf, ev->txt[10]);
		/* BLUT3.LTX[6]: "den Streitkolben mitnehmen?" */
		/* BLUT3.LTX[7]: "diese orkische Perversion zerschlagen" */
		n = dlg_choose(g_dialog, g_text_output_buf, 2, ev->txt[6], ev->txt[7]);
		if (n == 1) {
			/* BLUT3.LTX[8]: "Ihr nehmt den Streitkolben an Euch." */
			dlg_message(g_dialog, ev->txt[8]);
			g_god_favor_ingerimm -= 50;
			group_give_item(g_group, ITEM_MACE_12A, 1);
		} else if (n == 2) {
			/* BLUT3.LTX[9]: "Unter Euren Hieben zerbirst das Goetzenbild." */
			dlg_message(g_dialog, ev->txt[9]);
			g_god_favor_ingerimm = -1000000L;
		}
		ret = 1;
	} else if (type == 0x83) {
		if (!g_dng08_copper_disc_taken) {
			/* BLUT3.LTX[21]: "Dieses kleine Kaestchen aus Edelholz ist ueber und ueber mit Kupferplaettchen beschlagen. Der
			 * Deckel fehlt und offenbart somit seinen Inhalt; eine mumifizierte Hand und eine kleine Kupferscheibe.@Wollt
			 * Ihr" */
			/* BLUT3.LTX[22]: "die Hand nehmen" */
			/* BLUT3.LTX[23]: "die Kupferscheibe nehmen" */
			n = dlg_choose(g_dialog, ev->txt[21], 2, ev->txt[22], ev->txt[23]);
			if (n == 1) {
				sub_hero_le(h, random_schweif(10));
				/* BLUT3.LTX[24]: "Ploetzlich durchfaehrt %s ein stechender Schmerz. Blitzartig laesst %s die Hand wieder
				 * fallen, die jetzt in einem unheiligen Gruen schimmert." */
				sprintf(g_text_output_buf, ev->txt[24], h->name, grammar_pronoun(h->sex, 0, 0));
				dlg_message(g_dialog, g_text_output_buf);
			} else if (n == 2) {
				/* BLUT3.LTX[25]: "Okay. Vorsichtig nehmt Ihr die Kupferscheibe aus dem Kaestchen." */
				dlg_message(g_dialog, ev->txt[25]);
				group_give_item(g_group, ITEM_COPPER_DISC, 1);
				g_dng08_copper_disc_taken = 1;
			}
		/* BLUT3.LTX[89]: "Dieses kleine Kaestchen aus Edelholz ist ueber und ueber mit Kupferplaettchen beschlagen. Der
		 * Deckel fehlt und offenbart somit seinen Inhalt; eine mumifizierte Hand.@Wollt Ihr sie mitnehmen?" */
		} else if (dlg_yes_no(g_dialog, ev->txt[89]) == 1) {
			sub_hero_le(h, random_schweif(10));
			sprintf(g_text_output_buf, ev->txt[24], h->name, grammar_pronoun(h->sex, 0, 0));
			dlg_message(g_dialog, g_text_output_buf);
		}
		ret = 1;
	} else if (type == 0x84) {
		/* BLUT3.LTX[27]: "Unter den Teppichen und in den Ritze zwischen den Matratzen koennt Ihr zwar noch einige
		 * seltsame Fetische entdecken, jedoch nichts, was Euch irgendwie von Wert scheint." */
		dlg_message(g_dialog, ev->txt[27]);
		ret = 1;
	} else if (type == 0x85) {
		if (!g_dng08_mummy_fought) {
			/* BLUT3.LTX[33]: "Dies ist ein massiver, ziemlich schmuckloser Steinsarkophag. Komisch, dass Ihr keinerlei
			 * Grabbeigaben entdecken koennt.@Wollt Ihr" */
			/* BLUT3.LTX[34]: "den Sarkophag untersuchen?" */
			/* BLUT3.LTX[35]: "den Sarkophag oeffnen?" */
			n = dlg_choose(g_dialog, ev->txt[33], 2, ev->txt[34], ev->txt[35]);
			if (n == 1) {
				timewarp_minutes(g_date, 20);
				/* BLUT3.LTX[36]: "Exzellente Steinmetzarbeit! Ihr koennt jedoch keine Inschrift oder aehnliches erkennen, was
				 * auf den <Besitzer> des Sarkophages hinweist." */
				dlg_message(g_dialog, ev->txt[36]);
			} else if (n == 2) {
				if ((h2 = get_second_active_hero(g_group)) != 0) {
					n = (signed char)(h->attrib[ATTRIB_KK].cur + h->attrib[ATTRIB_KK].mod) +
						(signed char)(h2->attrib[ATTRIB_KK].cur + h2->attrib[ATTRIB_KK].mod);
					if (n >= 0x26) {
						/* BLUT3.LTX[38]: "Unter grosser Muehe gelingt es %s und %s, den Deckel zur Seite zu schieben. Mit einem
						 * dumpfen Poltern faellt er auf den Steinboden und gibt den Blick auf das Innere des Sarkophages frei..." */
						sprintf(g_text_output_buf, ev->txt[38], h->name, h2->name);
						dlg_message(g_dialog, g_text_output_buf);
						/* BLUT3.LTX[39]: "Eine Mumie!" */
						dlg_message(g_dialog, ev->txt[39]);
						/* BLUT3.LTX[40]: "Offensichtlich die eines Zwergen." */
						dlg_message(g_dialog, ev->txt[40]);
						/* BLUT3.LTX[41]: "Die sich gerade erhebt." */
						dlg_message(g_dialog, ev->txt[41]);
						/* BLUT3.LTX[42]: "Was meint Ihr wohl, warum die Orks die Schutzzeichen an die Tuer gemalt haben?" */
						dlg_message(g_dialog, ev->txt[42]);
						g_dng08_sarcophagus_opened = 1;
						if (event_fight1(ev, 0x5f, 1, 0) == 0)
							g_dng08_mummy_fought = 1;
					} else
						/* BLUT3.LTX[37]: "Der Deckel ist zu schwer!" */
						dlg_message(g_dialog, ev->txt[37]);
				} else
					dlg_message(g_dialog, ev->txt[37]);
			}
		} else
			/* BLUT3.LTX[43]: "Der Sarkophag ist aufgebrochen und leer!" */
			dlg_message(g_dialog, ev->txt[43]);
		ret = 1;
	} else if (type == 0x87) {
		if (pos == 0x1500)
			dng08_bolt_trap(ev, 3, &g_lock_trap_off);
		else
			dng08_bolt_trap(ev, 5, &g_dng08_trap_0213_defused);
	} else if (type == 0x88) {
		/* BLUT3.LTX[83]: "Wollt Ihr den Hebel umlegen?" */
		if (dlg_yes_no(g_dialog, ev->txt[83]) == 1) {
			/* BLUT3.LTX[84]: "Baeng! Der Hebel ist nun tot..." */
			dlg_message(g_dialog, ev->txt[84]);
			/* BLUT3.LTX[85]: "Moment mal, nein, so geht das nicht. Also nochmal.@Wollt Ihr den Hebel in seiner
			 * Schaltposition veraendern?" */
			if (dlg_yes_no(g_dialog, ev->txt[85]) == 1) {
				/* BLUT3.LTX[86]: "Ihr legt den Hebel um, aber nichts weiter geschieht." */
				dlg_message(g_dialog, ev->txt[86]);
				g_dng08_lever ^= 1;
			}
		}
	}
	return ret;
}

void dng08_bolt_trap(struct area_events far *ev, signed char mod, signed char far *flag)
{
	struct hero far *h;

	/* BLUT3.LTX[79]: "Ihr entdeckt eine Bolzenfalle.@Wollt Ihr versuchen, sie zu entschaerfen?" */
	if (!*flag && dlg_yes_no(g_dialog, ev->txt[79]) == 1) {
		preselect_best_skill(g_group, TA_SCHLOESSER);
		/* BLUT3.LTX[80]: "Wer soll sein Glueck an dieser Falle versuchen?" */
		h = get_active_hero(g_group, ev->txt[80]);
		if (h != 0) {
			if (test_skill(h, TA_SCHLOESSER, mod) >= 0) {
				/* BLUT3.LTX[82]: "Vorsichtig geht %s zu Werke und nach Kurzem ist die Falle entschaerft." */
				sprintf(g_text_output_buf, ev->txt[82], h->name);
				dlg_message(g_dialog, g_text_output_buf);
				*flag = 1;
				return;
			}
			/* BLUT3.LTX[81]: "Bei dem Versuch, die Fall zu entschaerfen, loest sie ploetzlich aus und verletzt %s schwer am
			 * Arm. Mit zusammengepressten Zaehnen versucht %s einen Schmerzenslaut zu unterdruecken, was auch beinahe
			 * gelingt!" */
			sprintf(g_text_output_buf, ev->txt[81], h->name, grammar_pronoun(h->sex, 0, 0));
			dlg_message(g_dialog, g_text_output_buf);
			sub_hero_le(h, random_interval(8, 15));
			g_dng08_noise += 2;
		}
	}
}
