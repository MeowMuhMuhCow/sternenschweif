/* OVL019 - Overlay O019 (Segment 0x1f67 in SCHWEIF.EXE): Zwergensiedlung am Finsterkopp - Stadt 3 (g_town_descs[3])
 * und Dungeon 9 (inneres Viertel, g_area_descs[9]); Texte FINSTER.LTX (Datei 0x21b) ueber o->txt. Position aus dem
 * Kartenereignis mev relativ zu g_map_x0/g_map_y0 als (Spalte << 8) | Zeile.
 * dng09_enter_building(id, mev): group.town = 3 (alter Wert nach old_town), Town_ctor(0, 3, 0x21b, 0x19),
 *   town_enter(o,
 *   id, mev), Town_dtor(o, 3) (new/Methode/delete als C-Aufrufe).
 * town03_handler(o, mev) / dng09_handler(o, mev): MAP_EVENT_ENTER = Zorn der Zwerge nach einem Frevel
 *   (g_finster_anger ab 3: Zwerge versammeln sich, ab 5: Angriff bis alle tot), Typ 1 = Haus betreten (Position ->
 *   g_exit_dir und Gebaeude 0x81..0x8a; nach Frevel abgewiesen: g_finster_refuse_msg bzw. txt[1]; Klopfen txt[0]);
 *   Typ 100 in town03_handler: Portalwachen weisen Elfen (Typ 10..12), Hexen (7) und Druiden (8) ab (Harmlose
 *   Gestalt,
 *   unsichtbar und Tiergestalt zaehlen nicht), sonst Dungeon 9 (Position (0x14, 0x0c)); dng09_handler: Typ 1 an
 *   0x1411 Tempel - Goldschluessel zurueckhaengen (g_binge_key_state = 3), Typ 100 Portal zurueck (Wachen fragen nach
 *   dem Goldschluessel; behalten = Frevel), Typ 0x65 Stufen hinab in die Binge (Dungeon 10, Tagebuch g_ttx[386]).
 *   Rueckgabe 1 = Ereignis behandelt.
 *
 * BCC -mlarge -3 -Ot -c -Y OVL019.C   -> 2681/2681 Bytes GLEICH (18.10.2026), cobjfix laeuft
 *   (Namensfassung mit schweif.h: 19.10.2026 GLEICH, LINKGLEICH; Town-Objekt in C als struct area_events, gleicher
 *   Aufbau; g_ttx[386] statt der Sicht g_b67c->f608)
 * - Statusbits der Helden (harmless +bd Bit 4, adler_wolf +be Bit 3) als unsigned short-Bitfelder: mov al/shr/and
 *   ax,1/or ax,ax;
 *   unsigned char-Bitfelder (C++) haengen ein mov ah,0 an.
 * - Ortswechsel-Makro doppelt angewendet: "old_town = town; t = town = 0; old_town = town; town = t;" -
 *   Zuweisungskette
 *   speichert erst town, dann t (mov al,0 / mov es:[..],al / mov [bp-..],al). Die Zwischenwerte t1/t2 in
 *   dng09_handler
 *   liegen auf [bp-9] und [bp-0ah] -> beide auf Funktionsebene deklariert (in getrennten Bloecken teilen sie sich
 *   einen Platz).
 * - Toter Epilog nach "return ret;" am Funktionsende entsteht ohne -O; switch-Faelle in Quellreihenfolge, das
 *   break hinter dem letzten Fall bleibt als jmp short stehen.
 * - dng09_enter_building steht oben -> push cs / call near aus dng09_handler.
 * - Originalfehler: der doppelte Ortswechsel ueberschreibt den gemerkten alten Wert (old_town bzw. old_dungeon) mit
 *   dem
 *   neuen; dng09_handler prueft "n == 0x8a", obwohl sein switch 0x8a nie setzt (aus town03_handler kopiert, toter
 *   Zweig).
 */

#include <stdio.h>
#include "schweif.h"

void dng09_enter_building(signed short id, struct map_event far *mev)
{
	void far *o;

	g_group->old_town = g_group->town;
	g_group->town = 3;
	o = Town_ctor(0, g_group->town, 0x21b, 0x19);
	town_enter(o, id, mev);
	Town_dtor(o, 3);
}

signed short town03_handler(struct area_events far *o, struct map_event far *mev)
{
	struct hero far *h;
	signed short ret;
	signed short n;
	signed char flag;
	signed char t;
	register signed short pos;
	register signed short type;

	type = mev->type;
	pos = (mev->y - g_map_x0) << 8;
	pos |= mev->x - g_map_y0;
	h = get_first_active_hero(g_group);
	n = 0;
	ret = 0;
	if (type == MAP_EVENT_ENTER) {
		if (g_finster_anger >= 3 && g_finster_anger_stage == 0) {
			/* FINSTER.LTX[13]: "Ihr merkt, dass sich einige Zwerge mit Kettenhemden und Aexten versammeln. Sollte dieser
			 * Aufstand etwa Euch gelten? Habt Ihr womoeglich den Zorn der Zwerge auf Euch gelenkt." */
			dlg_message(g_dialog, o->txt[13]);
			g_finster_anger_stage = 1;
		}
		if (g_finster_anger >= 5 && g_finster_anger_stage == 1) {
			/* FINSTER.LTX[14]: "Unvermittelt seht Ihr Euch einer groesseren Schar von erbosten Zwergen gegenueber. Als Ihr
			 * Euch umdreht, stehen auch dort ein gutes Dutzend des kleinen Volkes, die Aexte locker in der Hand.@<Mit
			 * Gotteslaesterern machen wir kurzen Prozess> hoert Ihr noch, und dann erfahrt Ihr, welch grauenvolle Wirkung
			 * scharfe Aexte in kundiger Hand auszurichten vermoegen..." */
			dlg_message(g_dialog, o->txt[14]);
			while (group_count_alive(g_group))
				group_sub_le(g_group, 0x14);
			g_finster_anger = 0;
			g_finster_anger_stage = 0;
		}
		if (pos == 0x1e0c) {
			town_enter(o, 0x8a, mev);
			ret = 1;
		}
	} else if (type == 1) {
		n = 0;
		switch (pos) {
		case 0x1208:
			g_exit_dir = 3;
			n = 0x82;
			break;
		case 0x1515:
			g_exit_dir = 0;
			n = 0x83;
			break;
		case 0x121d:
			g_exit_dir = 3;
			n = 0x87;
			break;
		case 0x1e0c:
			g_exit_dir = 1;
			n = 0x8a;
			break;
		}
		if (g_sacrilege != 0) {
			if (n == 0x8a) {
				town_enter(o, n, mev);
			} else {
				if (g_speech_ext_avail != 0) {
					g_finster_refuse_msg = 1;
					town_enter(o, 0x0e, mev);
				} else {
					/* FINSTER.LTX[1]: "Es ist wohl niemand zu Hause." */
					dlg_message(g_dialog, o->txt[1]);
				}
				timewarp_minutes(g_date, 3L);
			}
		} else if (n == 0) {
			/* FINSTER.LTX[0]: "Auf Euer Klopfen ruehrt sich nichts. Wollt Ihr es noch einmal und mit etwas mehr Nachdruck
			 * versuchen?" */
			if (dlg_yes_no(g_dialog, o->txt[0]) == 1) {
				if ((g_group->x + g_group->y) % 4) {
					dlg_message(g_dialog, o->txt[1]);
				} else {
					town_enter(o, 0x0e, mev);
				}
			}
		} else {
			town_enter(o, n, mev);
		}
		ret = 1;
	} else if (type == 100) {
		flag = 0;
		h = get_first_hero(g_group);
		do {
			if (check_hero3(h) && !h->harmless && !h->invisible && !h->adler_wolf) {
				if (h->type == 12) {
					n = 1;
				} else if (h->type == 10) {
					n = 1;
				} else if (h->type == 11) {
					n = 1;
				} else if (h->type == 7) {
					n = 2;
				} else if (h->type == 8) {
					n = 3;
				} else {
					flag = 1;
				}
			}
			if (n == 0)
				h = get_next_hero(g_group, h);
		} while (get_first_hero(g_group) != h && n == 0);
		if (n != 0) {
			/* FINSTER.LTX[8]: "Zwei grimmige, schwer bewaffnete Wachen an dem Portal mustern Euch von oben bis unten.
			 * Obwohl sie Euch koerperlich unterlegen scheinen, lassen Sie keinen Zweifel daran, dass sie Euch mit Ihren
			 * Aexten ohne zu zoegern aufschlitzen wuerden." */
			dlg_message(g_dialog, o->txt[8]);
			switch (n) {
			case 1:
				/* FINSTER.LTX[9]: "Einer tritt Euch entgegen, hebt imperativ die Hand und meint@<HALT! Elfen kommen hier nicht
				 * rein!>@Er deutet auf %s und schickt Euch wieder weg." */
				sprintf(g_text_output_buf, o->txt[9], h->name);
				break;
			case 2:
				/* FINSTER.LTX[10]: "Einer tritt Euch entgegen, hebt imperativ die Hand und meint@<HALT! Hexen kommen hier
				 * nicht rein!>@Er deutet auf %s und schickt Euch wieder weg." */
				sprintf(g_text_output_buf, o->txt[10], h->name);
				break;
			case 3:
				/* FINSTER.LTX[11]: "Einer tritt Euch entgegen, hebt imperativ die Hand und meint@<HALT! Druiden kommen hier
				 * nicht rein!>@Er deutet auf %s und schickt Euch wieder weg." */
				sprintf(g_text_output_buf, o->txt[11], h->name);
				break;
			}
			dlg_message(g_dialog, g_text_output_buf);
			n = 0;
		} else {
			if (flag) {
				dlg_message(g_dialog, o->txt[8]);
				/* FINSTER.LTX[17]: "<Ihr duerft passieren, aber bedenkt: Ihr betretet eine dem Ingerimm geweihte Staette.
				 * Verhaltet Euch deshalb so, wie es <Ihm> gebuehrt!> meint eine der Wachen und tritt beiseite." */
				dlg_message(g_dialog, o->txt[17]);
			}
			g_pp20_index = -1;
			g_refresh = 1;
#if FIX_0031
			/* Fix #31: Ortswechsel nur einmal: alten Ort behalten */
			g_group->old_town = g_group->town;
			g_group->town = 0;
#else
			g_group->old_town = g_group->town;
			t = g_group->town = 0;
			g_group->old_town = g_group->town;
			g_group->town = t;
#endif
			g_group->old_dungeon = g_group->dungeon;
			g_group->dungeon = 9;
			group_set_screen_pos(0x14, 0x0c);
			g_group->dir = 0;
			g_loaded_town = -1;
		}
		ret = 1;
	}
	return ret;
}

signed short dng09_handler(struct area_events far *o, struct map_event far *mev)
{
	signed char r;
	signed short ret;
	signed short n;
	signed short m;
	signed char t1;
	signed char t2;
	register signed short type;
	register signed short pos;

	type = mev->type;
	pos = (mev->y - g_map_x0) << 8;
	pos |= mev->x - g_map_y0;
	ret = 0;
	m = 0;
	if (type == MAP_EVENT_ENTER) {
		if (g_finster_anger >= 3 && g_finster_anger_stage == 0) {
			/* FINSTER.LTX[13]: "Ihr merkt, dass sich einige Zwerge mit Kettenhemden und Aexten versammeln. Sollte dieser
			 * Aufstand etwa Euch gelten? Habt Ihr womoeglich den Zorn der Zwerge auf Euch gelenkt." */
			dlg_message(g_dialog, o->txt[13]);
			g_finster_anger_stage = 1;
		}
		if (g_finster_anger >= 5 && g_finster_anger_stage == 1) {
			/* FINSTER.LTX[14]: "Unvermittelt seht Ihr Euch einer groesseren Schar von erbosten Zwergen gegenueber. Als Ihr
			 * Euch umdreht, stehen auch dort ein gutes Dutzend des kleinen Volkes, die Aexte locker in der Hand.@<Mit
			 * Gotteslaesterern machen wir kurzen Prozess> hoert Ihr noch, und dann erfahrt Ihr, welch grauenvolle Wirkung
			 * scharfe Aexte in kundiger Hand auszurichten vermoegen..." */
			dlg_message(g_dialog, o->txt[14]);
			while (group_count_alive(g_group))
				group_sub_le(g_group, 0x14);
			g_finster_anger = 0;
			g_finster_anger_stage = 0;
		}
	} else if (type == 1) {
		n = 0;
		switch (pos) {
		case 0x1411:
			g_exit_dir = 2;
			n = 0x81;
			break;
		case 0x0506:
			g_exit_dir = 2;
			n = 0x84;
			break;
		case 0x0a1c:
			g_exit_dir = 3;
			n = 0x85;
			break;
		case 0x0e09:
			g_exit_dir = 0;
			n = 0x86;
			break;
		case 0x1812:
			g_exit_dir = 1;
			n = 0x88;
			break;
		case 0x0617:
			g_exit_dir = 2;
			n = 0x89;
			break;
		}
		if (g_sacrilege != 0) {
			if (n == 0x8a) {
				dng09_enter_building(n, mev);
			} else if (n == 0x81 && g_binge_key_state == 3 && group_find_key(g_group, ITEM_GOLD_KEY) != -1) {
				m = 1;
			} else {
				if (g_speech_ext_avail != 0) {
					g_finster_refuse_msg = 1;
					dng09_enter_building(0x0e, mev);
				} else {
					/* FINSTER.LTX[1]: "Es ist wohl niemand zu Hause." */
					dlg_message(g_dialog, o->txt[1]);
				}
				timewarp_minutes(g_date, 3L);
			}
		} else if (n == 0) {
			/* FINSTER.LTX[0]: "Auf Euer Klopfen ruehrt sich nichts. Wollt Ihr es noch einmal und mit etwas mehr Nachdruck
			 * versuchen?" */
			if (dlg_yes_no(g_dialog, o->txt[0]) == 1) {
				if ((g_group->x + g_group->y) % 4) {
					dlg_message(g_dialog, o->txt[1]);
				} else {
					dng09_enter_building(0x0e, mev);
				}
			}
		} else if (n == 0x81 && g_binge_key_state == 3 && group_find_key(g_group, ITEM_GOLD_KEY) != -1) {
			m = 1;
		} else {
			dng09_enter_building(n, mev);
		}
		if (m) {
			/* FINSTER.LTX[19]: "Als Ihr den Tempel betreten wollt, hoert Ihr den Priester von innen rufen:@<Haengt den
			 * Schluessel an den Haken und verschwindet; ich habe zu tun.>@Wollt Ihr den Schluessel zurueckgeben?" */
			r = dlg_yes_no(g_dialog, o->txt[19]);
			if (r == 1) {
				group_take_item(g_group, ITEM_GOLD_KEY);
				/* FINSTER.LTX[20]: "Ihr haengt den Schluessel an den Haken." */
				dlg_message(g_dialog, o->txt[20]);
				g_binge_key_state = 3;
			} else {
				/* FINSTER.LTX[21]: "Ihr haltet Euer Gepaeck zusammen und denkt Euch Euren Teil." */
				dlg_message(g_dialog, o->txt[21]);
			}
		}
		ret = 1;
	} else if (type == 100) {
		if (group_find_key(g_group, ITEM_GOLD_KEY) != -1) {
			/* FINSTER.LTX[15]: "Zwei grimmige, schwer bewaffnete Wachen an dem Portal mustern Euch von oben bis
			 * unten.@<Habt Ihr den Schluessel zur Binge zurueckgegeben.>" */
			r = dlg_yes_no(g_dialog, o->txt[15]);
			if (r == 0) {
				/* FINSTER.LTX[16]: "<Dann solltet Ihr das noch tun!>" */
				dlg_message(g_dialog, o->txt[16]);
			} else {
				r = 3;
				if (!g_finster_anger) {
					g_sacrilege = 1;
					g_finster_anger = 1;
				}
			}
		} else {
			r = 3;
		}
		if (r == 3) {
			view_leave();
#if FIX_0031
			/* Fix #31: Ortswechsel nur einmal: alten Dungeon behalten */
			g_group->old_dungeon = g_group->dungeon;
			g_group->dungeon = 0;
#else
			g_group->old_dungeon = g_group->dungeon;
			t1 = g_group->dungeon = 0;
			g_group->old_dungeon = g_group->dungeon;
			g_group->dungeon = t1;
#endif
			g_group->old_town = g_group->town;
			g_group->town = 3;
			group_set_screen_pos(0x14, 0x0a);
			g_group->dir = 2;
			g_pp20_index = -1;
			g_refresh = 1;
			g_loaded_dungeon = -1;
		}
		ret = 1;
	} else if (type == 0x65) {
		/* FINSTER.LTX[18]: "Ihr oeffnet die Tuer und seht unzaehlige Stufen, die in die Tiefe des Berges fuehren. Wollt
		 * Ihr hinabsteigen?" */
		r = dlg_yes_no(g_dialog, o->txt[18]);
		if (r == 1) {
#if FIX_0031
			/* Fix #31: Ortswechsel nur einmal: alten Dungeon behalten */
			g_group->old_dungeon = g_group->dungeon;
			g_group->dungeon = 0x0a;
#else
			g_group->old_dungeon = g_group->dungeon;
			t2 = g_group->dungeon = 0x0a;
			g_group->old_dungeon = g_group->dungeon;
			g_group->dungeon = t2;
#endif
			group_set_screen_pos(9, 1);
			g_group->dir = 0;
			/* GLOBLTXT.LTX[386]: "Wir haben heute die Finsterkoppenbinge betreten." */
			diary_add(g_diary, g_ttx[386], 0);
			g_pp20_index = -1;
			g_refresh = 1;
			g_loaded_dungeon = -1;
			timewarp_minutes(g_date, 15L);
		} else {
			group_set_screen_pos(2, 1);
			g_group->dir = 0;
			g_cam_angle = 0x100;
		}
		ret = 1;
	}
	return ret;
}
