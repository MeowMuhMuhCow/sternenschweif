/* OVL064 - Overlay O064 (Segment 0x2079 in SCHWEIF.EXE): Ereignisse des Dungeons 12 (Fortsetzung von OVL063; Texte
 * BINGE4.LTX[0x22..0x47] ueber ev->txt, Gruppen g_groups).
 * dng12_lever: Hebel g_dng12_lever - Felder (0xd,0xa)/(0xe,0xb) per event_set_cell umbauen, Wandbild 3 bzw. 0x34,
 *   aber nur wenn keine Gruppe in der Grube (0xb, 0xe, Dungeon 12) steht (sonst "Hebel laesst sich nicht bewegen").
 * dng12_pit: Grube - Held mit Seil bzw. Stabzauber 3 -> dng12_pit_recover, Frage Sprung -> dng12_pit_jump.
 * dng12_pit_jump(ev, h): Springer waehlen (GE vorgewaehlt), ggf. anseilen (GE+3 statt GE+5), sonst eigene Gruppe;
 *   Erfolg -> group_set_screen_pos(10/12, y) je g_dng12_pit_side, sonst Schaden bzw. Sturz (LE auf 0) in die Grube.
 * dng12_pit_recover(ev, h): solange eine Gruppe in der Grube steht und die Frage (Leiche bergen) bejaht wird, diese
 *   Gruppe auf die Position der aktuellen Gruppe setzen (inline group::set_pos_x/set_pos_y), groups_merge.
 * dng12_altar_inscription: Alte Sprachen +4 - Zahl der Zwerge (Inschrift fuer Tordol, den Gruender der Binge).
 * dng12_dwarf_grave(ev, y, x, axe): Kriegsbeil nehmen (g_god_favor_ingerimm -= 100, g_sacrilege) bzw. Steinhaufen
 *   durchsuchen (-300, Kampf event_fight1(0x5f)); Feld und Wandbild je nach Position (zwei Sprungtabellen).
 * dng12_dig_rubble/dng12_dig_collapse(ev, y, x): freischaufeln mit 1 bzw. 2 Schaufeln (Zeit, AP), Wandbild 7..10
 *   und Feld je nach Position. dng12_clear_debris: Schutt wegraeumen - GE-Probe gegen Steinschlag, AP bis
 *   g_dng12_debris_ap = 3. dng12_rotten_beams: einmalig (g_dng12_beams_done) Sinnesschaerfe +6 je Held (warnt),
 *   dann W100 <= 10/5 -> Balken stuerzt (GE-2).
 *
 * BCC -mlarge -3 -Ot -Og -c -Y OVL064.CPP   -> 5458/5458 Bytes GLEICH (18.10.2026), cobjfix laeuft
 *   (mit -Oi ebenfalls gleich, ohne -Og 2 Bytes zu lang).
 *   (Namensfassung mit schweif.h: 19.10.2026 GLEICH, LINKGLEICH; automap_cell_write mit variablem Bild ueber
 *   AUTOMAP_CELL_WRITE_W, sonst 17 Bytes mehr)
 * - dng12_pit_jump: "mov al,[+2e] / cbw / cmp ax,1" = inline-Getter group::get_nr_heroes() (direkter
 *   Feldvergleich ergibt cmp byte). dng12_pit_jump und dng12_pit_recover stehen unter dng12_pit -> Prototypen, far
 *   call.
 * - dng12_pit_recover: "g->set_pos_x(g_group->x, g_group->x * 0xa8 + 0x54)" mit set_pos_x(v, sx) { screen_x = sx;
 *   old_x = x; x = v; }:
 *   Kopien v [bp-0ah], pos [bp-0ch] (bzw. -0e/-10 fuer set_pos_y), Argument als -Og-Zwischenwert [bp-12h]/[bp-14h]
 *   HINTER allen inline-Kopien; "lead = g->heroes[0]" -> mov eax; "lead = 0" (Local) -> mov dword.
 * - Gruppenschleife "for (i = 0, g = g_groups; i < 6; i++, g = &g_groups[i])" wie O063.
 * Originalfehler/Auffaelligkeiten: dng12_dwarf_grave - switch (k) ohne default: fuer k > 6 bleibt pic (SI)
 *   uninitialisiert; das Ansehen g_god_favor_ingerimm sinkt, bevor feststeht, ob group_give_item das Kriegsbeil
 *   ueberhaupt geben kann; nach dem Nehmen der Axt wird f = 0 gesetzt, so dass sofort die Frage nach dem Grab folgt.
 */
#include <stdio.h>

#include "schweif.h"

/* Phase B: eigene Daten des Moduls (ds:579e..57a8, dsana) - initialisierte Daten in Adressreihenfolge, danach der Literalpool */

/* automap_cell_write mit short als letztem Parameter (wie im Original deklariert; mit signed char legt BCC
 * die Register anders) */
#define AUTOMAP_CELL_WRITE_W ((void (*)(void far *, signed short, signed short, signed short, signed short))automap_cell_write)

void dng12_pit_jump(struct area_events far *ev, struct hero far *h);
void dng12_pit_recover(struct area_events far *ev, struct hero far *h);

void dng12_lever(struct area_events far *ev)
{
	struct group far *g;
	register signed short i;
	register signed short found;

	if (!g_dng12_lever) {
		found = 0;
		for (i = 0, g = g_groups; i < 6; i++, g = &g_groups[i]) {
			if (g->x == 0x0b && g->y == 0x0e && g->dungeon == 0x0c)
				found = 1;
		}
		if (!found) {
			event_set_cell(ev, 0x0d, 0x0a, 0x12, 0, 1);
			event_set_cell(ev, 0x0e, 0x0b, 0, 0, 0);
			automap_cell_write(g_map, 0x306, 0x0b, 0x0e, 3);
			g_dng12_lever = 1;
		} else {
			/* BINGE4.LTX[34]: "Der Hebel laesst sich nicht bewegen." */
			dlg_message(g_dialog, ev->txt[0x22]);
		}
	} else {
		event_set_cell(ev, 0x0d, 0x0a, 0x12, 0, 0);
		event_set_cell(ev, 0x0e, 0x0b, 0x13, 0, 1);
		automap_cell_write(g_map, 0x306, 0x0b, 0x0e, 0x34);
		g_dng12_lever = 0;
	}
}

void dng12_pit(struct area_events far *ev)
{
	signed short old;
	struct hero far *h;
	register signed short pos;

	old = dlg_set16(g_dialog, 5);
	pos = group_find_item(g_group, ITEM_ROPE);
	h = get_hero_staffspell(g_group, 3);
	if (h == 0 && pos >= 0)
		h = g_group->heroes[pos];
	if (h != 0)
		dng12_pit_recover(ev, h);
	/* BINGE4.LTX[39]: "Wollt Ihr ueber die Grube springen?" */
	if (dlg_yes_no(g_dialog, ev->txt[0x27]) == 1)
		dng12_pit_jump(ev, h);
	dlg_set16(g_dialog, old);
}

void dng12_pit_jump(struct area_events far *ev, struct hero far *h)
{
	signed short mod = 5;
	signed short sv = g_pp20_index;
	signed short old;
	signed short alone;
	struct hero far *hh;
	struct group far *og;
	struct group far *g;
	register signed short i;
	register signed short flag;

	old = dlg_set16(g_dialog, 5);
	flag = 0;
	alone = 0;
	hh = get_first_active_hero(g_group);
	og = g_group;
	if (g_group->get_nr_heroes() == 1) {
		alone = 1;
		h = 0;
	} else {
		preselect_best_attrib(g_group, ATTRIB_GE);
		/* BINGE4.LTX[40]: "Wer soll den Sprung wagen?" */
		hh = get_active_hero(g_group, ev->txt[0x28]);
		if (group_count_check3(g_group) == 1)
			h = 0;
	}
	if (hh != 0) {
		if (h != 0) {
			/* BINGE4.LTX[41]: "Wollt Ihr %s anseilen?" */
			sprintf(g_text_output_buf, ev->txt[0x29], hh->name);
			if (dlg_yes_no(g_dialog, g_text_output_buf) == 1) {
				flag = 1;
				mod = 3;
				/* BINGE4.LTX[42]: "%s schlingt sich ein Seil um die Hueften, die uebrigen halten das andere Ende. Dann laeuft
				 * %s zurueck, soweit das Seil reicht und spurtet dann ploetzlich auf die Grube zu." */
				sprintf(g_text_output_buf, ev->txt[0x2a], hh->name, grammar_pronoun(hh->sex, 0, 0));
				dlg_message(g_dialog, g_text_output_buf);
			} else {
				/* BINGE4.LTX[43]: "Mit einem ueberheblichen Lachen weist %s das Seil zurueck, drei Schritt sind schliesslich
				 * keine Entfernung!" */
				sprintf(g_text_output_buf, ev->txt[0x2b], hh->name);
				dlg_message(g_dialog, g_text_output_buf);
			}
		}
		if (!flag && !alone) {
			g_pp20_index = -1;
			diary_unlock(g_diary);
			make_group(1, hh);
			for (i = 0, g = g_groups; i < 6; i++, g = &g_groups[i]) {
				if (group_has_hero(g, hh)) {
					g_group = g;
					g_group_no = i;
				}
			}
			diary_lock(g_diary);
			g_pp20_index = sv;
			screen_redraw(g_screen);
		}
		if (test_attrib(hh, ATTRIB_GE, mod) > 0) {
			/* BINGE4.LTX[44]: "Mit einem eleganten Sprung ueberwindet %s die Grube." */
			sprintf(g_text_output_buf, ev->txt[0x2c], hh->name);
			dlg_message(g_dialog, g_text_output_buf);
			if (flag) {
				/* BINGE4.LTX[45]: "Waehrend %s das Seil im Tuerrahmen befestigt, tut Ihr es %s am diesseitigen Ende nach, so
				 * dass Ihr Euch bequem hinueberhangeln koennt." */
				sprintf(g_text_output_buf, ev->txt[0x2d], hh->name, grammar_pronoun(hh->sex, 3, 0));
				dlg_message(g_dialog, g_text_output_buf);
			}
			if (g_dng12_pit_side == 0x0c)
				group_set_screen_pos(0x0a, g_group->y);
			else
				group_set_screen_pos(0x0c, g_group->y);
			cam_set_from_group();
		} else if (flag) {
			/* BINGE4.LTX[46]: "Kurz vor dem Grubenrand strauchelt %s und huepft mit der Eleganz eines liebeskranken
			 * Riesenspringegels mitten in die Grube. Ein kraeftiger Ruck am Seil und ein unterdruecktes Aechzen bedeuten
			 * Euch, dass %s inzwischen eine der Seitenwaende erreicht hat. Mit etwas Muehe koennt Ihr %s wieder nach oben
			 * ziehen." */
			sprintf(g_text_output_buf, ev->txt[0x2e], hh->name, grammar_pronoun(hh->sex, 0, 0), grammar_pronoun(hh->sex, 2, 0));
			dlg_message(g_dialog, g_text_output_buf);
			sub_hero_le(hh, random_schweif(2) + 1);
		} else {
			/* BINGE4.LTX[47]: "Kurz vor dem Grubenrand strauchelt %s. Ein leises, seltsam dumpfes Geraeusch belehrt Euch,
			 * dass auch einfache Hindernisse hin- und wieder ein ernsthaftes Problem sein koennen." */
			sprintf(g_text_output_buf, ev->txt[0x2f], hh->name);
			dlg_message(g_dialog, g_text_output_buf);
			sub_hero_le(hh, hh->le);
			group_set_screen_pos(0x0b, 0x0e);
			g_group = og;
			g_group_no = get_group_index(og);
		}
	}
	dlg_set16(g_dialog, old);
}

void dng12_pit_recover(struct area_events far *ev, struct hero far *h)
{
	struct group far *g;
	struct hero far *lead;
	register signed short i;
	register signed short r;

	do {
		lead = 0;
		for (i = 0, g = g_groups; i < 6; i++, g = &g_groups[i]) {
			if (g->x == 0x0b && g->y == 0x0e && g->dungeon == 0x0c) {
				lead = g->heroes[0];
				break;
			}
		}
		if (lead != 0) {
			/* BINGE4.LTX[48]: "Wollt Ihr %s Leiche bergen?" */
			sprintf(g_text_output_buf, ev->txt[0x30], hero_name_gen(lead, 1));
			r = dlg_yes_no(g_dialog, g_text_output_buf);
			if (r == 1) {
				g->set_pos_x(g_group->x, g_group->x * 0xa8 + 0x54);
				g->set_pos_y(g_group->y, g_group->y * 0xa8 + 0x54);
				diary_unlock(g_diary);
				groups_merge();
				diary_lock(g_diary);
				/* BINGE4.LTX[49]: "%s holt %s Leiche aus der Grube. Vielleicht laesst eine der Gottheiten ja mit sich
				 * handeln..." */
				sprintf(g_text_output_buf, ev->txt[0x31], h->name, hero_name_gen(lead, 1));
				dlg_message(g_dialog, g_text_output_buf);
			}
		}
	} while (lead != 0 && r == 1);
}

void dng12_altar_inscription(struct area_events far *ev)
{
	signed short old;
	signed short cnt;
	struct hero far *h;
	struct hero far *m;

	old = dlg_set16(g_dialog, 5);
	/* BINGE4.LTX[55]: "Der Altar enthaelt eine Tafel mit einer Inschrift in runenaehnlichen Schriftzeichen.@Wollt Ihr
	 * versuchen, die Inschrift zu entziffern?" */
	if (dlg_yes_no(g_dialog, ev->txt[0x37]) == 1) {
		cnt = 0;
		h = get_best_hero_skill(g_group, TA_ALTE_SPRACHEN);
		m = get_first_hero(g_group);
		do {
			if (check_hero3(m) && m->type == HERO_TYPE_DWARF)
				cnt++;
			m = get_next_hero(g_group, m);
		} while (get_first_hero(g_group) != m);
		if (test_skill(h, TA_ALTE_SPRACHEN, 4 - cnt) <= 0) {
			/* BINGE4.LTX[56]: "Obwohl Ihr Euch geraume Zeit mit der Inschrift beschaeftigt, gelingt es Euch nicht, die
			 * Runen zu entziffern." */
			dlg_message(g_dialog, ev->txt[0x38]);
		} else {
			/* BINGE4.LTX[57]: "%s kann die Inschrift nicht vollstaendig entziffern, ist aber sicher, dass sowohl eine
			 * Widmung an Tordol, den Gruender der Binge, als auch das Gedenken an die Opfer eines Minenungluecks im Text
			 * enthalten sind." */
			sprintf(g_text_output_buf, ev->txt[0x39], h->name);
			dlg_message(g_dialog, g_text_output_buf);
		}
	}
	dlg_set16(g_dialog, old);
}

void dng12_dwarf_grave(struct area_events far *ev, signed short y, signed short x, signed short f)
{
	signed short old;
	signed short k;
	register signed short pic;

	old = dlg_set16(g_dialog, 5);
	k = -1;
	switch (x) {
	case 0x0c:
		k = y - 3;
		break;
	case 0x0d:
		k = 0;
		break;
	case 0x0e:
		k = y;
		break;
	}
	if (k != -1) {
		if (f == 1) {
			/* BINGE4.LTX[50]: "Steine wurden zu einem kleinen Haufen aufgeschichtet. Obenauf liegt eine Kriegsaxt." */
			dlg_message(g_dialog, ev->txt[0x32]);
			/* BINGE4.LTX[51]: "Wollt Ihr die Axt nehmen?" */
			if (dlg_yes_no(g_dialog, ev->txt[0x33]) == 1) {
#if FIX_0151
				/* Fix #151: Gunst und Frevel erst, wenn die Axt wirklich genommen wurde */
				if (group_give_item(g_group, ITEM_WAR_AXE, 1)) {
					g_god_favor_ingerimm -= 100;
					g_sacrilege = 1;
#else
				g_god_favor_ingerimm -= 100;
				g_sacrilege = 1;
				if (group_give_item(g_group, ITEM_WAR_AXE, 1)) {
#endif
					event_set_cell(ev, x, y, 0x18, 0, 3);
					/* BINGE4.LTX[53]: "Als Ihr die Axt nehmt, bebt die Erde fuer einen kurzen Moment." */
					dlg_message(g_dialog, ev->txt[0x35]);
					view_frame();
					f = 0;
					switch (k) {
					case 0:
					case 1:
						pic = 0x3d;
						break;
					case 2:
					case 3:
						pic = 0x3e;
						break;
					case 4:
						pic = 0x3f;
						break;
#if FIX_0151
					/* Fix #151: Wandbild vorbelegen */
					case 5:
					case 6:
						pic = 0x3c;
						break;
					default:
						pic = 0x3d;
						break;
					}
#else
					case 5:
					case 6:
						pic = 0x3c;
						break;
					}
#endif
					AUTOMAP_CELL_WRITE_W(g_map, 0x306, y, x, pic);
				}
			}
		}
		if (f == 0) {
			/* BINGE4.LTX[52]: "Wollt Ihr den Steinhaufen durchsuchen?" */
			if (dlg_yes_no(g_dialog, ev->txt[0x34]) == 1) {
				g_god_favor_ingerimm -= 300;
				g_sacrilege = 1;
				event_set_cell(ev, x, y, 0, 0, 3);
				/* BINGE4.LTX[54]: "Ihr findet die Gebeine eines Zwergen. Waehrend Ihr im Grab herumwuehlt, bebt die Erde fuer
				 * einen kurzen Moment." */
				dlg_message(g_dialog, ev->txt[0x36]);
				switch (k) {
				case 0:
				case 1:
					pic = 2;
					break;
				case 2:
				case 3:
					pic = 3;
					break;
				case 4:
					pic = 4;
					break;
#if FIX_0151
				/* Fix #151: Wandbild vorbelegen */
				case 5:
				case 6:
					pic = 1;
					break;
				default:
					pic = 2;
					break;
				}
#else
				case 5:
				case 6:
					pic = 1;
					break;
				}
#endif
				AUTOMAP_CELL_WRITE_W(g_map, 0x306, y, x, pic);
				event_fight1(ev, 0x5f, 1, 0);
			}
		}
	}
	dlg_set16(g_dialog, old);
}

void dng12_dig_rubble(struct area_events far *ev, signed short y, signed short x)
{
	signed short pic;
	signed short r;
	signed short old;
	signed short done;
	struct hero far *h;
	struct hero far *h2;

	old = dlg_set16(g_dialog, 5);
	done = 0;
	h = get_first_active_hero(g_group);
	h2 = get_second_active_hero(g_group);
	/* BINGE4.LTX[58]: "Der Boden ist mehr als kniehoch mit Geroell bedeckt, das aus einem verschuetteten Stollen
	 * stammt.@Wollt Ihr den Stollen freischaufeln?" */
	if (dlg_yes_no(g_dialog, ev->txt[0x3a]) == 1) {
		r = group_count_item(g_group, ITEM_SHOVEL);
		if (r == 2 && h2 == 0)
			r = 1;
		if (r == 0) {
			/* BINGE4.LTX[59]: "Es dauert nicht lange, bis Ihr feststellt, dass hier ohne Schaufel wohl nichts zu machen
			 * ist." */
			dlg_message(g_dialog, ev->txt[0x3b]);
			timewarp_minutes(g_date, 10L);
		} else if (r == 1) {
			/* BINGE4.LTX[60]: "%s macht sich an die Arbeit." */
			sprintf(g_text_output_buf, ev->txt[0x3c], h->name);
			dlg_message(g_dialog, g_text_output_buf);
			/* BINGE4.LTX[62]: "Es ist zwar aeusserst muehsam, aber Stueck fuer Stueck laesst sich der Einbruch auf diese
			 * Weise freilegen." */
			dlg_message(g_dialog, ev->txt[0x3e]);
			timewarp_minutes(g_date, 0x5aL);
			add_hero_ap(h, 3);
			done = 1;
		} else if (r >= 2) {
			/* BINGE4.LTX[61]: "%s und %s machen sich an die Arbeit." */
			sprintf(g_text_output_buf, ev->txt[0x3d], h->name, h2->name);
			dlg_message(g_dialog, g_text_output_buf);
			dlg_message(g_dialog, ev->txt[0x3e]);
			timewarp_minutes(g_date, 0x2dL);
			add_hero_ap(h, 2);
			add_hero_ap(h2, 2);
			done = 1;
		}
		if (done) {
			if ((y == 3 && x == 2) || (y == 0x0d && x == 5) || (y == 1 && x == 6) || (y == 0x0d && x == 3))
				pic = 7;
			else if (y == 0x0f && x == 4)
				pic = 10;
			else
				pic = 8;
			AUTOMAP_CELL_WRITE_W(g_map, 0x306, y, x, pic);
			if ((y == 2 && x == 7) || (y == 9 && x == 1) || (y == 8 && x == 5) || (y == 0x0a && x == 5))
				event_set_cell(ev, x, y, 1, 0, 1);
			else
				event_set_cell(ev, x, y, 1, 0, 0);
		}
	}
	dlg_set16(g_dialog, old);
}

void dng12_dig_collapse(struct area_events far *ev, signed short y, signed short x)
{
	signed short pic;
	signed short r;
	signed short old;
	signed short done;
	struct hero far *h;
	struct hero far *h2;

	old = dlg_set16(g_dialog, 5);
	done = 0;
	h = get_first_active_hero(g_group);
	h2 = get_second_active_hero(g_group);
	/* BINGE4.LTX[63]: "Der Gang ist hier eingestuerzt, offensichtlich ist einer der Deckenbalken zusammengebrochen
	 * und einige Gesteinsbrocken sind von oben nachgerutscht. @Wollt Ihr den Stollen freischaufeln?" */
	if (dlg_yes_no(g_dialog, ev->txt[0x3f]) == 1) {
		r = group_count_item(g_group, ITEM_SHOVEL);
		if (r == 2 && h2 == 0)
			r = 1;
		if (r == 0) {
			/* BINGE4.LTX[59]: "Es dauert nicht lange, bis Ihr feststellt, dass hier ohne Schaufel wohl nichts zu machen
			 * ist." */
			dlg_message(g_dialog, ev->txt[0x3b]);
			timewarp_minutes(g_date, 10L);
		} else if (r == 1) {
			/* BINGE4.LTX[60]: "%s macht sich an die Arbeit." */
			sprintf(g_text_output_buf, ev->txt[0x3c], h->name);
			dlg_message(g_dialog, g_text_output_buf);
			/* BINGE4.LTX[62]: "Es ist zwar aeusserst muehsam, aber Stueck fuer Stueck laesst sich der Einbruch auf diese
			 * Weise freilegen." */
			dlg_message(g_dialog, ev->txt[0x3e]);
			timewarp_minutes(g_date, 0xb4L);
			add_hero_ap(h, 3);
			done = 1;
		} else if (r >= 2) {
			/* BINGE4.LTX[61]: "%s und %s machen sich an die Arbeit." */
			sprintf(g_text_output_buf, ev->txt[0x3d], h->name, h2->name);
			dlg_message(g_dialog, g_text_output_buf);
			dlg_message(g_dialog, ev->txt[0x3e]);
			add_hero_ap(h, 2);
			add_hero_ap(h2, 2);
			timewarp_minutes(g_date, 0x5aL);
			done = 1;
		}
		if (done) {
			if ((y == 1 && x == 4) || (y == 1 && x == 5) || (y == 0x0d && x == 4))
				pic = 7;
			else if ((y == 0x0e && x == 4) || (y == 0x0c && x == 1) || (y == 0x0b && x == 1) ||
				 (y == 0x0a && x == 1) || (y == 2 && x == 3) || (y == 9 && x == 5))
				pic = 8;
			else if (y == 1 && x == 3)
				pic = 9;
			else
				pic = 10;
			AUTOMAP_CELL_WRITE_W(g_map, 0x306, y, x, pic);
			if ((y == 0x0a && x == 1) || (y == 0x0b && x == 1) || (y == 0x0c && x == 1) ||
			    (y == 0x0e && x == 4) || (y == 9 && x == 5) || (y == 2 && x == 3))
				event_set_cell(ev, x, y, 1, 0, 1);
			else
				event_set_cell(ev, x, y, 1, 0, 0);
		}
	}
	dlg_set16(g_dialog, old);
}

void dng12_clear_debris(struct area_events far *ev)
{
	signed short old;
	struct hero far *h;

	old = dlg_set16(g_dialog, 5);
	h = get_first_active_hero(g_group);
	/* BINGE4.LTX[64]: "Der Stollen ist vollkommen eingestuerzt. Gesteinsbrocken und Schutt versperren Euch den Weg.
	 * Wollt Ihr den Schutt wegraeumen?" */
	if (dlg_yes_no(g_dialog, ev->txt[0x40]) == 1) {
		timewarp_minutes(g_date, 0x14L);
		if (test_attrib(h, ATTRIB_GE, 0) > 0) {
			/* BINGE4.LTX[66]: ". %s kann gerade noch einem grossen Brocken ausweichen." */
			sprintf(g_text_buf2, ev->txt[0x42], h->name);
			/* BINGE4.LTX[65]: "Das ist zwecklos! Von oben fallen weitere Gesteinsbrocken nach" */
			sprintf(g_text_output_buf, "%s%s", ev->txt[0x41], g_text_buf2);
			dlg_message(g_dialog, g_text_output_buf);
			if (g_dng12_debris_ap < 3) {
				add_hero_ap(h, 5);
				g_dng12_debris_ap++;
			}
		} else {
			/* BINGE4.LTX[67]: ", einer davon trifft mit ganzer Wucht %s." */
			sprintf(g_text_buf2, ev->txt[0x43], h->name);
			sprintf(g_text_output_buf, "%s%s", ev->txt[0x41], g_text_buf2);
			dlg_message(g_dialog, g_text_output_buf);
			sub_hero_le(h, random_schweif(3) + 3);
			if (g_dng12_debris_ap < 3) {
				add_hero_ap(h, 10);
				g_dng12_debris_ap++;
			}
		}
	}
	dlg_set16(g_dialog, old);
}

void dng12_rotten_beams(struct area_events far *ev)
{
	signed short old;
	struct hero far *h;
	register signed short chance;

	if (!g_dng12_beams_done) {
		chance = 10;
		old = dlg_set16(g_dialog, 5);
		h = get_first_hero(g_group);
		do {
			if (!g_dng12_beams_seen && check_hero3(h) && test_skill(h, TA_SINNESSCHAERFE, 6) > 0) {
				add_hero_ap(h, 10);
				g_dng12_beams_seen = 1;
				/* BINGE4.LTX[68]: "%s deutet auf einen der Deckenbalken ueber Euch. Das Gebaelk ist hier offensichtlich
				 * besonders morsch, und Ihr muesst Euch mit aeusserster Vorsicht bewegen." */
				sprintf(g_text_output_buf, ev->txt[0x44], h->name);
				dlg_message(g_dialog, g_text_output_buf);
			}
			if (g_dng12_beams_seen != 0)
				chance = 5;
			if (random_schweif(100) <= chance && check_hero3(h)) {
				if (test_attrib(h, ATTRIB_GE, -2) <= 0) {
					/* BINGE4.LTX[69]: "Ploetzlich bricht einer der Deckenbalken krachend herab und begraebt %s unter groesseren
					 * Gesteinsbrocken, die von oben nachrutschen." */
					sprintf(g_text_output_buf, ev->txt[0x45], h->name);
					dlg_message(g_dialog, g_text_output_buf);
					sub_hero_le(h, random_schweif(5) + 3);
					/* BINGE4.LTX[70]: "Es dauert nicht lange, bis Ihr %s befreit habt, %s ist von einigen Steinen getroffen
					 * worden und blutet heftig. Es bleibt Euch noch genuegend Platz, um an der Einsturzstelle vorbeizukommen." */
					sprintf(g_text_output_buf, ev->txt[0x46], h->name, grammar_pronoun(h->sex, 0, 0));
					dlg_message(g_dialog, g_text_output_buf);
				} else {
					/* BINGE4.LTX[71]: "Ploetzlich bricht einer der Deckenbalken krachend herab und %s springt geistesgegenwaertig
					 * beiseite. Von oben rutschen groessere Gesteinsbrocken nach, aber es bleibt genuegend Platz, um daran
					 * vorbeizukommen." */
					sprintf(g_text_output_buf, ev->txt[0x47], h->name);
					dlg_message(g_dialog, g_text_output_buf);
				}
				add_hero_ap(h, 10);
				g_dng12_beams_done = 1;
				break;
			}
			h = get_next_hero(g_group, h);
		} while (get_first_hero(g_group) != h);
		dlg_set16(g_dialog, old);
	}
}
