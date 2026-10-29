/* OVL059 - Overlay O059 (Segment 0x205d in SCHWEIF.EXE): gemeinsame Ereignisse der Binge-Dungeons 10..16 mit eigenen
 * Texten BINGE.LTX (Datei 0x2f9, 0x3c Stueck; per txt_load_list nach g_binge_txt_buf/g_binge_txt, am Ende
 *   freigegeben).
 * binge_wall_hole(ev, hole): handtellergrosse Oeffnung in der Wand (Aufrufer dng11_handler Loch 1/2, dng13_handler
 *   Loch
 *   3), Menueschleife txt[4..6]: betrachten (txt[7]), hineingreifen - je Loch einmal: 1 Schatulle mit Muenzen und
 *   Edelstein (FF+3, ITEM_GEMS, 573 Heller, g_binge_hole1_looted), 2 Rattenfalle (FF+4: Schaden W4+2, Wundfieber,
 *   g_binge_hole2_trapped), 3 Phiole mit Heiltrank (ITEM_LE_POTION, g_binge_hole3_looted); sonst/misslungen Biss
 *   (Schaden 2, Tollwut-Probe 5 %); Fackel hineinstecken (Fackel ggf. mit ItemUse anzuenden; Quieken bzw. nichts).
 * binge_shaft(ev): Kaminschacht mit Eisenkrampen (Aufrufer dng10..dng13, dng16): hinaufsteigen -> binge_shaft_up,
 *   hinabsteigen bzw. auf Ebene 16 abseilen -> binge_shaft_down (16 = unterste Ebene des Schachts).
 * binge_shaft_up(ev, dng): Dungeon 10 - Steinplatte, lockerer Stein (Ablage Stash 0x23, einmalig 5 AP
 *   g_binge_shaft_stone_ap), zurueck voller Russ (hero.sooty, CH -2 fuer alle); Dungeon 11..16 eine Ebene hoeher
 *   (16 -> 13). binge_shaft_down(ev, dng): Dungeon 10..13 eine Ebene tiefer (13 -> 16); auf 16 mit Seil
 *   (binge_has_rope) bis zu drei Abseilstufen, bei der dritten stuerzt der Held in die Tiefe (group_set_slot -1).
 * binge_has_rope(ev): Seil (ITEM_ROPE) oder Zauberstab mit Seilzauber (get_hero_staffspell(.., 3)).
 *
 * BCC -mlarge -3 -Ot -c -Y OVL059.CPP   -> 3718/3718 Bytes GLEICH (18.10.2026), cobjfix laeuft (auch mit -Og gleich)
 *   (Namensfassung mit schweif.h: 19.10.2026 GLEICH, LINKGLEICH)
 * - C++ wegen lokaler Objekte (ItemUse im else-Block, Stash im if-Block); Bitfeld "unsigned short sooty:1" (or
 *   ax,ax).
 * - binge_wall_hole: hole ist register-Parameter (SI), sel OHNE register -> DI (mit register tauschen SI/DI).
 * - switch (hole) mit Faellen 0..3 -> Sprungtabelle im Code hinter der Funktion (0x747).
 * - Block-Locals: h2 (Fall 2) und pos/n (Menuepunkt 3) teilen sich [bp-0ah]/[bp-0ch].
 * - "group_set_slot(g_group, group_hero_pos(g_group, h), -1)": das -1 gehoert zum aeusseren Aufruf und wird VOR dem
 *   inneren gepusht (erkennbar an add sp,8 / add sp,8 statt 10 / 6).
 * - "signed char c = dng - 1;" im else-Block -> mov al,[bp+0ah] / dec al.
 * Originalfehler: keiner gefunden. Auffaellig: Fall 2 (Erfolg) laesst die Zeit zweimal 5 Minuten vergehen.
 */
#include <stdio.h>

#include "schweif.h"

/* Phase B: eigene Daten des Moduls (ds:570a..5710, dsana) - initialisierte Daten in Adressreihenfolge, danach der Literalpool */
/* Phase B: eigenes BSS (ds:c80a..c812) in Adressreihenfolge */
void far *g_binge_txt_buf;
char far * far *g_binge_txt;

void binge_wall_hole(struct area_events far *ev, register signed short hole)
{
	signed short old;
	signed short done;
	struct hero far *h;
	signed short sel;

	old = dlg_set16(g_dialog, 5);
	done = 0;
	h = get_first_active_hero(g_group);
	txt_load_list(g_font, 0x2f9, 0x3c, &g_binge_txt_buf, &g_binge_txt);
	while (!done) {
		/* BINGE.LTX[3]: "Zu Euren Fuessen entdeckt Ihr halb in der Wand, halb im Boden eine etwa handtellergrosse
		 * Oeffnung." */
		/* BINGE.LTX[19]: "Wollt Ihr" */
		sprintf(g_text_output_buf, "%s@%s", g_binge_txt[3], g_binge_txt[19]);
		/* BINGE.LTX[4]: "die Oeffnung genauer betrachten?" */
		/* BINGE.LTX[5]: "hineingreifen?" */
		/* BINGE.LTX[6]: "oder eine Fackel hineinstecken?" */
		sel = dlg_choose(g_dialog, g_text_output_buf, 3, g_binge_txt[4], g_binge_txt[5], g_binge_txt[6]);
		if (sel == 1) {
			/* BINGE.LTX[7]: "Das Loch ist dunkel. Ihr koennt nichts weiter erkennen." */
			dlg_message(g_dialog, g_binge_txt[7]);
		} else if (sel == 2) {
			if (hole == 1) {
				if (g_binge_hole1_looted)
					hole = 0;
			} else if (hole == 2) {
				if (g_binge_hole2_trapped)
					hole = 0;
			} else if (hole == 3) {
				if (g_binge_hole3_looted)
					hole = 0;
			}
			switch (hole) {
			case 0:
				if (random_schweif(3) == 1) {
					/* BINGE.LTX[8]: "%s tastet sich in der Oeffnung vorwaerts. Ploetzlich zieht %s die Hand ruckartig aus dem
					 * Loch. Irgendetwas hat %s gebissen." */
					sprintf(g_text_output_buf, g_binge_txt[8], h->name, grammar_pronoun(h->sex, 0, 0),
						grammar_pronoun(h->sex, 2, 0));
					dlg_message(g_dialog, g_text_output_buf);
					sub_hero_le(h, 2);
					hero_disease_test(h, ILLNESS_TOLLWUT, 5);
				} else {
					/* BINGE.LTX[9]: "%s tastet sich in der Oeffnung vorwaerts, bis %s Arm ganz darin verschwunden ist, findet
					 * aber nichts." */
					sprintf(g_text_output_buf, g_binge_txt[9], h->name, grammar_pronoun(h->sex, 1, 0));
					dlg_message(g_dialog, g_text_output_buf);
				}
				timewarp_minutes(g_date, 5L);
				break;
			case 1:
				if (test_attrib(h, ATTRIB_FF, 3) > 0) {
					/* BINGE.LTX[10]: "%s ertastet mit den Fingerspitzen einen Gegenstand. Mit viel Geschick gelingt es %s eine
					 * kleine Schatulle aus dem Loch zu ziehen. In ihr befinden sich ein paar Muenzen und ein kleiner Edelstein." */
					sprintf(g_text_output_buf, g_binge_txt[10], h->name, grammar_pronoun(h->sex, 3, 0));
					dlg_message(g_dialog, g_text_output_buf);
					give_hero_new_item(h, ITEM_GEMS, 1, 1);
					group_add_money(g_group, 0x23dL);
					g_binge_hole1_looted = 1;
				} else {
					/* BINGE.LTX[11]: "%s ertastet mit den Fingerspitzen einen Gegenstand, aber es gelingt %s nicht, das Ding zu
					 * greifen. Ploetzlich zieht %s die Hand ruckartig aus dem Loch. Irgendetwas hat %s gebissen." */
					sprintf(g_text_output_buf, g_binge_txt[11], h->name, grammar_pronoun(h->sex, 3, 0),
						grammar_pronoun(h->sex, 0, 0), grammar_pronoun(h->sex, 2, 0));
					dlg_message(g_dialog, g_text_output_buf);
					sub_hero_le(h, 2);
					hero_disease_test(h, ILLNESS_TOLLWUT, 5);
				}
				timewarp_minutes(g_date, 5L);
				break;
			case 2:
				if (test_attrib(h, ATTRIB_FF, 4) > 0) {
					struct hero far *h2;

					/* BINGE.LTX[12]: "%s ertastet mit den Fingerspitzen einen Gegenstand. Ploetzlich zieht %s die Hand ruckartig
					 * aus dem Loch. %s ist in eine Rattenfalle getappt. Der verrostete Buegel hat sich ins Handfleisch
					 * eingeschnitten und es schmerzt fuerchterlich." */
					sprintf(g_text_output_buf, g_binge_txt[12], h->name, grammar_pronoun(h->sex, 0, 0), h->name);
					dlg_message(g_dialog, g_text_output_buf);
					sub_hero_le(h, random_schweif(4) + 2);
					hero_disease_test(h, ILLNESS_WUNDFIEBER, 0x32);
					g_binge_hole2_trapped = 1;
					h2 = get_second_active_hero(g_group);
					if (h2 == 0)
						/* BINGE.LTX[13]: "Mit etwas Muehe biegt %s den Buegel der Falle zurueck, und zieht erleichtert die laedierte
						 * Hand aus der Falle." */
						sprintf(g_text_output_buf, g_binge_txt[13], h->name);
					else
						/* BINGE.LTX[14]: "Ohne viel Muehe biegt %s den Buegel der Falle zurueck, und %s zieht erleichtert die
						 * laedierte Hand aus der Falle." */
						sprintf(g_text_output_buf, g_binge_txt[14], h2->name, h->name);
					timewarp_minutes(g_date, 5L);
					dlg_message(g_dialog, g_text_output_buf);
				} else {
					/* BINGE.LTX[15]: "%s ertastet irgendeinen Gegenstand mit den Fingerspitzen, aber es gelingt %s nicht, das
					 * Ding zu greifen. Ploetzlich zieht %s die Hand ruckartig aus dem Loch. Irgendetwas hat %s gebissen." */
					sprintf(g_text_output_buf, g_binge_txt[15], h->name, grammar_pronoun(h->sex, 3, 0),
						grammar_pronoun(h->sex, 0, 0), grammar_pronoun(h->sex, 2, 0));
					dlg_message(g_dialog, g_text_output_buf);
					sub_hero_le(h, 2);
					hero_disease_test(h, ILLNESS_TOLLWUT, 5);
				}
				timewarp_minutes(g_date, 5L);
				break;
			case 3:
				/* BINGE.LTX[16]: "%s findet eine kleine Phiole mit Heiltrank." */
				sprintf(g_text_output_buf, g_binge_txt[16], h->name);
				dlg_message(g_dialog, g_text_output_buf);
				group_give_item(g_group, ITEM_LE_POTION, 1);
				g_binge_hole3_looted = 1;
				timewarp_minutes(g_date, 5L);
				break;
			}
		} else if (sel == 3) {
			signed short pos;
			signed short slot;

			if (group_find_item(g_group, ITEM_TORCH_ON) < 0) {
				pos = group_find_item(g_group, ITEM_TORCH_OFF);
				if (pos < 0)
					/* BINGE.LTX[20]: "Welche Fackel denn, bitteschoen?" */
					dlg_message(g_dialog, g_binge_txt[20]);
				else {
					h = g_group->heroes[pos];
					slot = get_item_pos(h, ITEM_TORCH_OFF);
					ItemUse u(h, 0, slot, ITEM_TORCH_OFF, 1);
					u.use();
				}
			}
			if (group_find_item(g_group, ITEM_TORCH_ON) >= 0) {
				if (random_schweif(3) == 1)
					/* BINGE.LTX[17]: "Es passiert nichts." */
					dlg_message(g_dialog, g_binge_txt[17]);
				else
					/* BINGE.LTX[18]: "Ein Quieken ist zu hoeren." */
					dlg_message(g_dialog, g_binge_txt[18]);
			}
			timewarp_minutes(g_date, 5L);
		} else
			done = 1;
		if (g_refresh_portraits)
			screen_portraits(g_screen);
	}
	dlg_set16(g_dialog, old);
	mem_free(g_binge_txt_buf);
	mem_free(g_binge_txt);
}

void binge_shaft(struct area_events far *ev)
{
	signed short old;
	register signed short sel;
	register signed short dng;

	old = dlg_set16(g_dialog, 5);
	dng = g_group->dungeon;
	txt_load_list(g_font, 0x2f9, 0x3c, &g_binge_txt_buf, &g_binge_txt);
	if (dng == 16)
		/* BINGE.LTX[44]: "Der Schacht reicht in beide Richtungen weiter als der Schein Eures Lichtes. Von unten her
		 * bemerkt Ihr deutlich Zugluft. Links und rechts sind grosse Oesen in die Wand geschlagen, nach oben hin sind in
		 * regelmaessigen Abstaenden Eisenkrampen in die Wand geschlagen.@Wollt Ihr" */
		/* BINGE.LTX[35]: "den Schacht hinaufsteigen?" */
		/* BINGE.LTX[45]: "oder Euch abseilen?" */
		sel = dlg_choose(g_dialog, g_binge_txt[44], 2, g_binge_txt[35], g_binge_txt[45]);
	else
		/* BINGE.LTX[34]: "Direkt unter und ueber Euch sind grosse Eisenkrampen in regelmaessigen Abstaenden in dem
		 * Schacht befestigt, so dass sich eine provisorische Stiege ergibt. Der Schacht reicht in beide Richtungen
		 * weiter als der Schein Eures Lichtes. @Wollt Ihr" */
		/* BINGE.LTX[36]: "oder hinabsteigen?" */
		sel = dlg_choose(g_dialog, g_binge_txt[34], 2, g_binge_txt[35], g_binge_txt[36]);
	if (sel == 1)
		binge_shaft_up(ev, dng);
	else if (sel == 2)
		binge_shaft_down(ev, dng);
	dlg_set16(g_dialog, old);
	mem_free(g_binge_txt_buf);
	mem_free(g_binge_txt);
}

void binge_shaft_up(struct area_events far *ev, signed short dng)
{
	signed short sel;
	struct hero far *h;

	h = get_first_active_hero(g_group);
	timewarp_minutes(g_date, 15L);
	if (dng == 10) {
		/* BINGE.LTX[37]: "Es geht scheinbar endlos nach oben und das Klettern ermuedet. Nachdem Ihr drei- bis
		 * vierhundert Stiegen bewaeltigt hat, endet die Stiege vor einer massiven Steinplatte, die den Schacht nach oben
		 * hin vollstaendig abschliesst.@Wollt Ihr" */
		/* BINGE.LTX[38]: "wieder hinabklettern?" */
		/* BINGE.LTX[39]: "oder die Platte untersuchen?" */
		sel = dlg_choose(g_dialog, g_binge_txt[37], 2, g_binge_txt[38], g_binge_txt[39]);
		if (sel == 2) {
			timewarp_minutes(g_date, 5L);
			Stash st(0x23, 1);
			if (st.has_items()) {
				/* BINGE.LTX[40]: "Sicherlich koenntet Ihr selbst mit vereinten Kraeften die Platte nicht von der Stelle
				 * bewegen, aber als %s sich umschaut, entdeckt %s eine Spalte in der Felswand. Tatsaechlich wurde hier ein
				 * Stein aus dem Fels geschlagen, der sich mit einiger Muehe herausziehen laesst." */
				sprintf(g_text_output_buf, g_binge_txt[40], h->name, grammar_pronoun(h->sex, 0, 0));
				dlg_message(g_dialog, g_text_output_buf);
			}
			st.show();
			if (!g_binge_shaft_stone_ap) {
				g_binge_shaft_stone_ap = 1;
				add_hero_ap(h, 5);
			}
		}
		timewarp_minutes(g_date, 10L);
		/* BINGE.LTX[41]: "Abwaerts geht es zwar etwas leichter, trotzdem seid Ihr ziemlich geschafft, als Ihr ueber und
		 * ueber mit Russ bedeckt wieder die Schachtoeffnung erreicht." */
		dlg_message(g_dialog, g_binge_txt[41]);
		h = get_first_hero(g_group);
		do {
			if (!h->sooty) {
				h->sooty = 1;
				change_attrib(h, ATTRIB_CH, -2);
			}
			h = get_next_hero(g_group, h);
		} while (get_first_hero(g_group) != h);
		return;
	}
	if (dng >= 11 && dng <= 16) {
		/* BINGE.LTX[42]: "Es geht ein gehoeriges Stueck nach oben und das Klettern ermuedet. Schliesslich erreicht Ihr
		 * eine Oeffnung, durch die Ihr den Schacht verlassen koennt." */
		dlg_message(g_dialog, g_binge_txt[42]);
		if (dng == 16) {
			g_group->old_dungeon = g_group->dungeon;
			g_group->dungeon = 13;
		} else {
			signed char c = dng - 1;

			g_group->old_dungeon = g_group->dungeon;
			g_group->dungeon = c;
		}
		group_set_screen_pos(2, 1);
		g_group->dir = 3;
	}
}

void binge_shaft_down(struct area_events far *ev, signed short dng)
{
	struct hero far *h;
	register signed short sel;
	register signed short ok;

	h = get_first_active_hero(g_group);
	timewarp_minutes(g_date, 15L);
	if (dng >= 10 && dng <= 13) {
		/* BINGE.LTX[43]: "Ihr klettert ueber die provisorische Stiege aus Eisenkrampen nach unten und erreicht einen
		 * Absatz, von dem aus ein enges Loch ostwaerts aus dem Kamin fuehrt." */
		dlg_message(g_dialog, g_binge_txt[43]);
		if (dng == 13) {
			g_group->old_dungeon = g_group->dungeon;
			g_group->dungeon = 16;
		} else {
			signed char c = dng + 1;

			g_group->old_dungeon = g_group->dungeon;
			g_group->dungeon = c;
		}
		group_set_screen_pos(2, 1);
		g_group->dir = 3;
		return;
	}
	if (dng == 16 && binge_has_rope(ev)) {
		ok = 1;
		/* BINGE.LTX[46]: "%s seilt sich ein Stueck in den Schacht ab, kann aber nichts auffaelliges entdecken.@Wollt
		 * Ihr" */
		sprintf(g_text_output_buf, g_binge_txt[46], h->name);
		/* BINGE.LTX[47]: "%s weiter abseilen?" */
		sprintf(g_text_output_buf + 200, g_binge_txt[47], h->name);
		/* BINGE.LTX[48]: "oder %s wieder heraufholen?" */
		sprintf(g_text_output_buf + 400, g_binge_txt[48], grammar_pronoun(h->sex, 2, 0));
		sel = dlg_choose(g_dialog, g_text_output_buf, 2, g_text_output_buf + 200, g_text_output_buf + 400);
		if (sel == 1) {
			/* BINGE.LTX[49]: "Weiter als acht Schritt geht es kaum, denn %s hat Muehe sich am schwankenden Seil
			 * festzuhalten.@ Wollt Ihr" */
			sprintf(g_text_output_buf, g_binge_txt[49], h->name);
			sel = dlg_choose(g_dialog, g_text_output_buf, 2, g_text_output_buf + 200, g_text_output_buf + 400);
			if (sel == 1) {
				/* BINGE.LTX[50]: "%s ist jetzt nahezu zehn Schritt tief und schon mehrmals gegen die Schachtwaende
				 * geprallt.@Wollt Ihr" */
				sprintf(g_text_output_buf, g_binge_txt[50], h->name);
				sel = dlg_choose(g_dialog, g_text_output_buf, 2, g_text_output_buf + 200, g_text_output_buf + 400);
				if (sel == 1) {
					/* BINGE.LTX[51]: "Ploetzlich schreit %s laut auf und Ihr hoert dumpfe polternde Geraeusche, die in der Tiefe
					 * verklingen." */
#if FIX_0363
					/* Fix #363: der letzte Held der Gruppe stuerzt nicht ab (leere Gruppe haengt das Spiel auf), er steigt wieder heraus */
					if ((signed char)g_group->nr_heroes - (group_get_npc(g_group) != 0) > 1) {
						sprintf(g_text_output_buf, g_binge_txt[51], h->name);
						dlg_message(g_dialog, g_text_output_buf);
						ok = 0;
						group_set_slot(g_group, group_hero_pos(g_group, h), -1);
						screen_portraits(g_screen);
					}
#else
					sprintf(g_text_output_buf, g_binge_txt[51], h->name);
					dlg_message(g_dialog, g_text_output_buf);
					ok = 0;
					group_set_slot(g_group, group_hero_pos(g_group, h), -1);
					screen_portraits(g_screen);
#endif
				}
			}
			if (ok) {
				/* BINGE.LTX[52]: "%s hat zwar einige Muehe, kann aber wohlbehalten wieder aus dem Loch steigen." */
				sprintf(g_text_output_buf, g_binge_txt[52], h->name);
				dlg_message(g_dialog, g_text_output_buf);
			}
		}
	}
}

signed short binge_has_rope(struct area_events far *ev)
{
	struct hero far *h;
	register signed short ok;
	register signed short pos;

	h = get_hero_staffspell(g_group, 3);
	pos = group_find_item(g_group, ITEM_ROPE);
	ok = 0;
	if (h != 0 || pos >= 0)
		ok = 1;
	if (ok) {
		if (pos == -1) {
			/* BINGE.LTX[54]: "%s verwandelt %s Zauberstab in ein Seil, dann kann es losgehen." */
			sprintf(g_text_output_buf, g_binge_txt[54], h->name, grammar_pronoun(h->sex, 3, 1));
			dlg_message(g_dialog, g_text_output_buf);
		}
	} else
		/* BINGE.LTX[53]: "Ihr habt doch gar kein Seil dabei!" */
		dlg_message(g_dialog, g_binge_txt[53]);
	return ok;
}
