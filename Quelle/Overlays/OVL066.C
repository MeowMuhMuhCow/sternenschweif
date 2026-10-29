/* OVL066 - Overlay O066 (Segment 0x2086 in SCHWEIF.EXE): Kartenereignis-Verteiler und Ereignisse der Dungeons 14 und
 * 15, der Binge-Ebene mit dem grossen Wasserbecken (g_area_descs[14] und [15]; 15 = Becken voll, 14 = leer, nachdem
 * in Dungeon 13 das Rad gedreht wurde, g_dng13_wheel_turned; Texte BINGE6.LTX = Datei 0x459 ueber ev->txt).
 * dng14_handler(ev, mev): MAP_EVENT_ENTER (Position ((mev->y - g_map_x0) << 8) | (mev->x - g_map_y0)), nur bei vollem
 *   Becken: 0x106/0x10d Becken verlassen (g_dng14_in_pool = 0), 0x107/0x10c ins Becken (einmalig je Bad:
 *   dng14_wash + dng14_swim), sonst im Becken dng14_dive; bei leerem Becken 0x402 dng14_floor_plate (bis die Platte
 *   gehoben ist). 0x65 Treppe nach Dungeon 16 (gesperrt, solange g_dng16_leather_taken: txt[20] Steinmauer), 0x67
 *   Treppe nach Dungeon 13.
 * dng14_swim(ev): wer nicht handlungsfaehig ist (check_hero2), bleibt als eigene Gruppe am Rand (txt[16]/[17]);
 *   Zwerge weigern sich zu baden (txt[1]), Schwimmen-Probe misslungen -> txt[2]; beide bleiben zurueck (make_group).
 * dng14_dive(ev): W100 (am Beckenrand 100) <= 10: zufaelliger Held taucht (txt[3]/[4]); 6..8 nichts zu fassen
 *   (txt[5]), 9 etwas packt ihn (txt[6]/[7]: alle verlassen das Wasser, Gruppen in Dungeon 15 nach (1, 13)), 10
 *   einmalig magischer Wasserschlauch (txt[8], ITEM_MAGIC_WATERSKIN, g_dng14_waterskin_found).
 * dng14_wash(ev): verrusste Helden (hero.sooty) waschen sich (txt[15], CH +2).
 * dng14_floor_plate(ev): Platte am Grund des leeren Beckens anheben (txt[9..14]): 1..3 Versuche
 *   (g_dng14_plate_tries), dann Hinweis auf Werkzeug (bester in Schloesser), mit Brecheisen (ITEM_CROWBAR) Treppe
 *   frei (g_dng14_plate_lifted, Feld (2, 4), Automap 0x1b).
 *
 * BCC -mlarge -3 -Ot -c -Y OVL066.C   -> 3061/3061 Bytes GLEICH (18.10.2026), cobjfix laeuft (mit -Og ebenso)
 *   (Namensfassung mit schweif.h: 19.10.2026 GLEICH, LINKGLEICH)
 * - Reines C. Toter Epilog in dng14_handler kommt schon ohne -Og (return r nach dem switch, r = 0 als Stack-Local,
 *   weil SI/DI von pos/type belegt sind).
 * - Verschachtelter switch: Positionen per Binaersuche (0x10c als Pivot), Faelle mit gleichem Rumpf zusammen.
 * - dng14_swim: lokales Feld aus 7 Heldenzeigern per Strukturkopie aus g_dng14_hero_ptrs_null (rep movsw, deshalb
 *   push si/di); make_group ist variadisch und bekommt die 7 Zeiger EINZELN (14 push word) - Uebergabe des ganzen
 *   structs als Wert erzeugt stattdessen lea/mov cx,1ch/Aufruf eines Kopier-Helfers.
 * - Bitfeld h->sooty (unsigned short, 1 Byte): Test "mov al / shr ax,2 / and ax,1 / or ax,ax", Loeschen
 *   "and byte [bx+0beh],0fbh".
 * - Gruppenschleife "for (i = 0, g = g_groups; i < 6; g = &g_groups[++i])" -> imul 49h / add ax,0c30fh / mov
 *   [bp-8],ds.
 * Originalfehler: keiner gefunden (dng14_handler liefert immer 0).
 */
#include <stdio.h>

#include "schweif.h"

/* Phase B: eigene Daten des Moduls (ds:57b4..57dc, dsana) - initialisierte Daten in Adressreihenfolge, danach der Literalpool */
struct hero_ptrs7 g_dng14_hero_ptrs_null = { { 0, 0, 0, 0, 0, 0, 0 } };

signed short dng14_handler(struct area_events far *ev, struct map_event far *mev)
{
	signed short ret = 0;
	register signed short pos;
	register signed short type;

	type = mev->type;
	pos = (mev->y - g_map_x0) << 8;
	pos |= mev->x - g_map_y0;

	switch (type) {
	case MAP_EVENT_ENTER:
		switch (pos) {
		case 0x106:
		case 0x10d:
			g_dng14_in_pool = 0;
			break;
		case 0x107:
		case 0x10c:
			if (!g_dng13_wheel_turned && !g_dng14_in_pool) {
				dng14_wash(ev);
				dng14_swim(ev);
				g_dng14_in_pool = 1;
			}
			break;
		case 0x402:
			if (g_dng13_wheel_turned != 0 && !g_dng14_plate_lifted)
				dng14_floor_plate(ev);
			break;
		default:
			if (!g_dng13_wheel_turned)
				dng14_dive(ev);
			break;
		}
		break;
	case 0x65:
		if (!g_dng16_leather_taken) {
			group_set_screen_pos(15, 1);
			g_group->old_dungeon = g_group->dungeon;
			g_group->dungeon = 16;
			g_group->dir = 2;
		} else
			/* BINGE6.LTX[20]: "Die Treppe endet vor einer massiven Steinmauer. Hier kommt Ihr nicht durch!" */
			dlg_message(g_dialog, ev->txt[20]);
		break;
	case 0x67:
		group_set_screen_pos(10, 0x12);
		g_group->old_dungeon = g_group->dungeon;
		g_group->dungeon = 13;
		g_group->dir = 1;
		break;
	default:
		break;
	}
	return ret;
}

void dng14_swim(struct area_events far *ev)
{
	signed short n;
	signed short old;
	signed short cnt;
	struct hero far *h;
	struct hero far *next;
	struct hero_ptrs7 list;

	n = 0;
	old = dlg_set16(g_dialog, 5);
	cnt = group_count_check2b(g_group);
	h = get_first_hero(g_group);
	next = get_first_hero(g_group);
	list = g_dng14_hero_ptrs_null;
	if (group_get_npc(g_group))
		cnt--;
	do {
		if (!check_hero2(h)) {
			list.h[n] = h;
			n++;
		}
		h = get_next_hero(g_group, h);
	} while (get_first_hero(g_group) != h);

	if (n) {
		make_group(n, list.h[0], list.h[1], list.h[2], list.h[3], list.h[4], list.h[5], list.h[6]);
		if (n == 1) {
			h = list.h[0];
			/* BINGE6.LTX[16]: "Da %s in %s derzeitigen Zustand nicht in der Lage ist zu schwimmen, lasst Ihr %s am
			 * Beckenrand zurueck." */
			/* BINGE6.LTX[19]: "ihrem" */
			/* BINGE6.LTX[18]: "seinem" */
			sprintf(g_text_output_buf, ev->txt[16], h->name, h->sex ? ev->txt[19] : ev->txt[18], grammar_pronoun(h->sex, 2, 0));
			dlg_message(g_dialog, g_text_output_buf);
		} else
			/* BINGE6.LTX[17]: "Da einige von Euch in ihrem derzeitigen Zustand nicht in der Lage sind zu schwimmen, lasst
			 * Ihr sie am Beckenrand zurueck." */
			dlg_message(g_dialog, ev->txt[17]);
	}

	h = get_first_hero(g_group);
	do {
		next = get_next_hero(g_group, h);
		if (h->type == HERO_TYPE_DWARF || test_skill(h, TA_SCHWIMMEN, 0) <= 0) {
			if (h->type == HERO_TYPE_DWARF) {
				/* BINGE6.LTX[1]: "<Ich habe mir schon die Unsitte angewoehnt, mich monatlich zu waschen,> protestiert %s,
				 * <aber das ist einfach zuviel verlangt. - Da kriegt Ihr mich nicht hinein!>" */
				sprintf(g_text_output_buf, ev->txt[1], h->name, grammar_pronoun(h->sex, 0, 0));
				dlg_message(g_dialog, g_text_output_buf);
			} else {
				/* BINGE6.LTX[2]: "%s zuckt zurueck, als %s die Wassertemperatur ueberprueft und beschliesst, dass jetzt nicht
				 * der geeignete Zeitpunkt fuer ein Bad ist." */
				sprintf(g_text_output_buf, ev->txt[2], h->name, grammar_pronoun(h->sex, 0, 0));
				dlg_message(g_dialog, g_text_output_buf);
			}
			if (cnt > 1 || group_get_npc(g_group) == h)
				make_group(1, h);
			cnt--;
		}
		h = next;
	} while (get_first_hero(g_group) != h);
	dlg_set16(g_dialog, old);
}

void dng14_dive(struct area_events far *ev)
{
	signed short old;
	struct hero far *h;
	struct group far *g;
	register signed short i;
	register signed short roll;

	if (g_group->x == 1 || g_group->x == 7 || g_group->y == 1 || g_group->y == 0x12)
		roll = 100;
	else
		roll = random_schweif(100);

	if (roll <= 10) {
		old = dlg_set16(g_dialog, 5);
		h = get_random_hero(g_group);
		if (roll <= 5) {
			/* BINGE6.LTX[3]: "%s haelt die Luft an und taucht, bis %s den Boden des Beckens erreicht hat. Es ist ziemlich
			 * dunkel, und %s kann nichts besonderes erkennen." */
			sprintf(g_text_output_buf, ev->txt[3], h->name, grammar_pronoun(h->sex, 0, 0), grammar_pronoun(h->sex, 0, 0));
			dlg_message(g_dialog, g_text_output_buf);
		} else {
			/* BINGE6.LTX[4]: "%s haelt die Luft an und taucht, bis %s den Boden des Beckens erreicht hat. Es ist ziemlich
			 * dunkel, und... ploetzlich fuehlt %s etwas Weiches zwischen %s Fingern." */
			sprintf(g_text_output_buf, ev->txt[4], h->name, grammar_pronoun(h->sex, 0, 0), h->name, grammar_pronoun(h->sex, 3, 1));
			dlg_message(g_dialog, g_text_output_buf);
			if (g_dng14_waterskin_found != 0 && roll == 10)
				roll = 8;
			if (roll <= 8) {
				/* BINGE6.LTX[5]: "Schnell greift %s zu, bekommt aber nichts zu fassen." */
				sprintf(g_text_output_buf, ev->txt[5], h->name);
				dlg_message(g_dialog, g_text_output_buf);
			} else if (roll == 9) {
				/* BINGE6.LTX[6]: "Schnell greift %s zu, bekommt aber nichts zu fassen, dafuer bekommt irgendetwas anderes %s
				 * zu fassen! %s schlaegt heftig um sich und ist schon halb ertrunken, als es %s endlich doch noch gelingt,
				 * freizukommen.@Erschoepft kauert %s sich am Rande der Treppe nieder und starrt schaudernd in das Wasser." */
				sprintf(g_text_output_buf, ev->txt[6], h->name, grammar_pronoun(h->sex, 2, 0), h->name,
					grammar_pronoun(h->sex, 3, 0), grammar_pronoun(h->sex, 0, 0));
				dlg_message(g_dialog, g_text_output_buf);
				/* BINGE6.LTX[7]: "Es dauert nur Sekunden bis Ihr alle das Wasser verlassen habt!" */
				dlg_message(g_dialog, ev->txt[7]);
				for (i = 0, g = g_groups; i < 6; g = &g_groups[++i])
					if (g->dungeon == 15 &&
					    ((g->x >= 2 && g->x <= 6 && g->y >= 2 && g->y <= 17) ||
					     (g->x == 1 && g->y >= 8 && g->y <= 11))) {
						group_set_screen_pos(1, 13);
						cam_set_from_group();
					}
			} else if (roll == 10) {
				/* BINGE6.LTX[8]: "Schnell greift %s zu, und bekommt das Etwas zu fassen, taucht damit auf - und betrachtet
				 * zornig den halbvollen Wasserschlauch, den %s in der Hand haelt. Demonstrativ oeffnet %s den Schlauch und
				 * schuettet das Wasser heraus.@Es dauert geraume Zeit, bis %s merkt, dass der Schlauch ueberhaupt keine
				 * Neigung zeigt, leer zu werden!@Soll %s den magischen Wasserschlauch einpacken?" */
				sprintf(g_text_output_buf, ev->txt[8], h->name, grammar_pronoun(h->sex, 0, 0), h->name,
					grammar_pronoun(h->sex, 0, 0), h->name);
				if (dlg_yes_no(g_dialog, g_text_output_buf) == 1) {
#if FIX_0361
					/* Fix #361: Schlauch bei vollem Rucksack ueber die Gruppe geben, Merker nur bei Erfolg */
					if (give_hero_new_item(h, ITEM_MAGIC_WATERSKIN, 1, 1) > 0 || group_give_item(g_group, ITEM_MAGIC_WATERSKIN, 1) > 0)
						g_dng14_waterskin_found = 1;
#else
					give_hero_new_item(h, ITEM_MAGIC_WATERSKIN, 1, 1);
					g_dng14_waterskin_found = 1;
#endif
				}
			}
		}
		dlg_set16(g_dialog, old);
	}
}

void dng14_wash(struct area_events far *ev)
{
	signed short old;
	struct hero far *h;

	old = dlg_set16(g_dialog, 5);
	h = get_first_hero(g_group);
	do {
		if (check_hero3(h) && h->sooty) {
			/* BINGE6.LTX[15]: "%s nutzt die Gelegenheit um sich den Russ aus den Kleidern zu entfernen und sich das Gesicht
			 * gruendlich zu waschen." */
			sprintf(g_text_output_buf, ev->txt[15], h->name);
			dlg_message(g_dialog, g_text_output_buf);
			h->sooty = 0;
			change_attrib(h, ATTRIB_CH, 2);
		}
		h = get_next_hero(g_group, h);
	} while (get_first_hero(g_group) != h);
	dlg_set16(g_dialog, old);
}

void dng14_floor_plate(struct area_events far *ev)
{
	signed short old;
	struct hero far *h;
	register signed short choice;

	old = dlg_set16(g_dialog, 5);
	/* BINGE6.LTX[9]: "Die Platte ist so gross, dass Ihr sie selbst mit vereinten Kraeften wahrscheinlich nicht
	 * hochheben koennt. Wollt Ihr trotzdem versuchen, die Platte anzuheben?" */
	if (!g_dng14_plate_tried && dlg_yes_no(g_dialog, ev->txt[9]) == 1) {
		/* BINGE6.LTX[10]: "Ihr strengt Euch zwar maechtig an, aber es gelingt Euch nicht, die Platte hochzuheben." */
		dlg_message(g_dialog, ev->txt[10]);
		g_dng14_plate_tried = 1;
		g_dng14_plate_tries = random_schweif(3) + 1;
	}
	if (g_dng14_plate_tried != 0) {
		while (g_dng14_plate_tries > 1) {
			/* BINGE6.LTX[0]: "Wollt Ihr" */
			/* BINGE6.LTX[11]: "nochmal versuchen, die Platte anzuheben?" */
			sprintf(g_text_output_buf, "%s %s", ev->txt[0], ev->txt[11]);
			if (dlg_yes_no(g_dialog, g_text_output_buf) == 1) {
				dlg_message(g_dialog, ev->txt[10]);
				g_dng14_plate_tries--;
			} else
				break;
		}
		if (g_dng14_plate_tries == 1) {
			h = get_best_hero_skill(g_group, TA_SCHLOESSER);
			/* BINGE6.LTX[13]: "<So geht es nicht!> meint %s, <wir muessten ein geeignetes Werkzeug haben!>" */
			sprintf(g_text_output_buf, ev->txt[13], h->name);
			dlg_message(g_dialog, g_text_output_buf);
			g_dng14_plate_tries--;
		}
		if (g_dng14_plate_tries == 0) {
			choice = 0;
			if (group_find_item(g_group, ITEM_CROWBAR) != -1) {
				/* BINGE6.LTX[12]: "oder lieber eine Brechstange verwenden?" */
				choice = dlg_choose(g_dialog, ev->txt[0], 2, ev->txt[11], ev->txt[12]);
				if (choice == 2) {
					/* BINGE6.LTX[14]: "Mit der Brechstange gelingt es Euch auf Anhieb, die Platte an den Eisengriffen
					 * hochzuhebeln, darunter befindet sich eine Treppe, die weiter in die Tiefe fuehrt." */
					dlg_message(g_dialog, ev->txt[14]);
					g_dng14_plate_lifted = 1;
					group_set_screen_pos(4, 3);
					event_set_cell(ev, 2, 4, 9, 0, 0);
					automap_cell_write(g_map, 0x316, 4, 2, 0x1b);
				} else
					dlg_message(g_dialog, ev->txt[10]);
			} else {
				sprintf(g_text_output_buf, "%s %s", ev->txt[0], ev->txt[11]);
				if (dlg_yes_no(g_dialog, g_text_output_buf) == 1)
					dlg_message(g_dialog, ev->txt[10]);
			}
		}
	}
	dlg_set16(g_dialog, old);
}
