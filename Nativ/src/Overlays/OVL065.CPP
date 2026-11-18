/* OVL065 - Overlay O065 (Segment 0x207f in SCHWEIF.EXE): Kartenereignis-Verteiler und Ereignisse des Dungeons 13,
 * der Binge mit Feuerschlund, Wasserrad und Pelzwaenden (g_area_descs[13]; Texte BINGE5.LTX = Datei 0x458 ueber
 * ev->txt). Ereignisobjekt ev (struct area_events), Kartenereignis mev (struct map_event).
 * dng13_handler(ev, mev): MAP_EVENT_ENTER: 0x603 dng13_ambush; bei g_dng13_wheel_turned (Wasser im Kanal) auf dem
 *   Kanal (x 8/y 2..8 bzw. y 10/x 9..17) dng13_wash; 1 Tueren dng13_door; 0x64 dng13_lava_shaft; 0x65 binge_shaft;
 *   0x66 Ebenenwechsel (group_set_screen_pos(1, 0x12): 0x12 nach Dungeon 12, 0xb12 nach 14 bzw. vor dem Rad 15);
 *   0x67 dng13_fur_wall_cold, 0x68 dng13_salt_seam, 0x69 dng13_fur_wall_burning, 0x6a binge_wall_hole(ev, 3),
 *   0x6b dng13_wheel, 0x6c dng13_gargoyles.
 * dng13_door: Schloss Lock 0x9b..0x9e, Kartenbild 0x28/0x29.
 * dng13_ambush: sechs Gestalten in Lederruestungen (txt[12], event_fight2 0x73/0x5f), gewonnen ->
 *   g_dng13_ambush_won und Ablage Stash 0x29 (leer -> g_dng13_stash29_empty); spaeter nur noch die Ablage.
 * dng13_wash: verrusste Helden (hero.sooty) waschen sich im Kanal (txt[29], CH +2).
 * dng13_lava_shaft: Loch im Boden (txt[0..11]) - etwas hineinwerfen (Ingerimm +10; Feuerpulver ITEM_FIRE_POWDER
 *   weckt den Feuerschlund: abhauen nach (4, 4) bzw. abwarten = 1W11+4 Schaden fuer alle) oder abseilen (Seil bzw.
 *   Seilzauber, txt[11]).
 * dng13_wheel: Rad drehen (txt[13..15]): g_dng13_wheel_turned - das Becken der Ebene 15 laeuft ueber den Kanal
 *   leer (Felder/Automap), alle Gruppen in Dungeon 15 kommen nach 14 (Fassung mit leerem Becken).
 * dng13_gargoyles: vier Wasserspeier-Statuen werden lebendig (txt[27], Kampf 0x61/0x60), gewonnen ->
 *   dng13_gargoyles_gone (Felder/Automap der Statuen).
 * dng13_fur_wall_cold/dng13_fur_wall_burning: Wand mit schimmerndem Pelz (txt[19]); dng13_fur_cure heilt zuerst
 *   Helden mit dem jeweils anderen Pelz (txt[16], hero.fur_hands = -1); sonst rutscht ein Held aus (GE+2) und
 *   bekommt Pelz an die Finger (txt[20]): kuehler Pelz (fur_hands 6, txt[21], Haende waschen mit dem
 *   Wasserschlauch txt[22]/[23]) bzw. brennender Pelz (fur_hands 1, txt[26]; Verlauf in O005 hero_fur_tick);
 *   gelungene Probe -> dng13_fur_burn.
 * dng13_fur_victim(ev, wall): naechster unberuehrter Held; hat er den passenden Pelz -> dng13_fur_burn, 0.
 * dng13_fur_burn(ev, wall): Frage txt[17]; ohne Fackel/Laterne/Zunder txt[28]; kalter Pelz loescht die Flamme
 *   (txt[18]), brennender Pelz verbrennt (txt[25]): Qualm g_dng13_smoke_timer = 6 (O005 dng13_smoke_damage), Felder
 *   (6, 9)/(6, 10) frei.
 * dng13_salt_seam: Salzschicht zwischen den Pelzen (txt[24]).
 *
 * BCC -mlarge -3 -Ot -Og -c -Y OVL065.CPP   -> 5265/5265 Bytes GLEICH (18.10.2026), cobjfix laeuft
 *   (Namensfassung mit schweif.h: 19.10.2026 GLEICH, LINKGLEICH)
 * - C++ wegen Lock (O108, Block-Local [bp-2ch]) und new/delete Stash (O238, Stash_ctor(0, ..), Dtor (p, 3)).
 * - dng13_handler: "dy = (y = mev->y) - g_map_x0;" (mov [bp-8],ax / sub ax / mov si,ax), dy/dx register (SI/DI);
 *   "switch (pos) { case 0x603: ..; break; }" mit nur einem case.
 * - dng13_ambush: "if (!g_dng13_ambush_won) { ..Locals..; return; }" (Epilogkopie), zweiter Block mit eigenem Zeiger
 *   auf [bp-4].
 * - dng13_door: "default: break;" erzeugt den eigenen jmp short.
 * - dng13_fur_cure/dng13_fur_victim: "(v = h->fur_hands) >= 1 && v <= 5" eingebettet (cmp ax,1 / cmp si,5);
 *   found = h -> mov eax.
 * - dng13_fur_burn: "if (..) { ..; return; }" zweimal -> Epilogkopien.
 * Originalfehler: dng13_fur_wall_cold prueft den Vorrat des Wasserschlauchs beim Traeger h2 (Index aus
 *   get_item_pos(h2, ..)), zieht ihn aber an der Stelle get_item_pos(h, ..) des Helden mit dem Pelz ab - falscher
 *   Platz (oder -1), wenn h nicht selbst der Traeger ist.
 */
#include <stdio.h>

#include "schweif.h"

/* Phase B: eigene Daten des Moduls (ds:57a8..57b4, dsana) - initialisierte Daten in Adressreihenfolge, danach der Literalpool */

/* automap_cell_write mit short als letztem Parameter (wie im Original deklariert) */
#define AUTOMAP_CELL_WRITE_W ((void (*)(void far *, signed short, signed short, signed short, signed short))automap_cell_write)

signed short dng13_handler(struct area_events far *ev, struct map_event far *mev)
{
	signed short type;
	signed short pos;
	signed short ret;
	signed short y;
	signed short x;
	register signed short dy;
	register signed short dx;

	type = mev->type;
	dy = (y = mev->y) - g_map_x0;
	dx = (x = mev->x) - g_map_y0;
	pos = dy << 8 | dx;
	ret = 0;
	switch (type) {
	case MAP_EVENT_ENTER:
		switch (pos) {
		case 0x603:
			dng13_ambush(ev);
			break;
		}
		if (g_dng13_wheel_turned != 0 && ((dx == 8 && dy >= 2 && dy <= 8) || (dy == 10 && dx >= 9 && dx <= 0x11)))
			dng13_wash(ev);
		break;
	case 1:
		ret = dng13_door(ev, pos, x, y);
		break;
	case 0x64:
		dng13_lava_shaft(ev);
		break;
	case 0x65:
		binge_shaft(ev);
		break;
	case 0x66:
		if (pos == 0x12) {
			group_set_screen_pos(1, 0x12);
			g_group->old_dungeon = g_group->dungeon;
			g_group->dungeon = 0x0c;
			g_group->dir = 3;
		} else if (pos == 0xb12) {
			group_set_screen_pos(1, 0x12);
			if (g_dng13_wheel_turned != 0) {
				g_group->old_dungeon = g_group->dungeon;
				g_group->dungeon = 0x0e;
			} else {
				g_group->old_dungeon = g_group->dungeon;
				g_group->dungeon = 0x0f;
			}
			g_group->dir = 3;
		}
		break;
	case 0x67:
		dng13_fur_wall_cold(ev);
		break;
	case 0x68:
		dng13_salt_seam(ev);
		break;
	case 0x69:
		dng13_fur_wall_burning(ev);
		break;
	case 0x6a:
		binge_wall_hole(ev, 3);
		break;
	case 0x6b:
		dng13_wheel(ev);
		break;
	case 0x6c:
		dng13_gargoyles(ev);
		break;
	}
	return ret;
}

signed short dng13_door(struct area_events far *ev, signed short pos, signed short x, signed short y)
{
	signed short ret = 0;
	signed short nr = -1;
	signed short pic = 0x28;

	switch (pos) {
	case 0x503:
		nr = 0x9b;
		break;
	case 0x406:
		nr = 0x9c;
		pic = 0x29;
		break;
	case 0x109:
		nr = 0x9d;
		pic = 0x29;
		break;
	case 0x711:
		nr = 0x9e;
		pic = 0x29;
		break;
	default:
		break;
	}
	if (nr != -1) {
		Lock lk(nr);
		ret = 1;
		if (g_lock_cell_bits != -1) {
			event_set_cell_bits(ev, x, y, g_lock_cell_bits);
			snd_play(g_snd_fx, 0x61);
			AUTOMAP_CELL_WRITE_W(g_map, 0x30a, y - g_map_x0, x - g_map_y0, pic);
		}
	}
	return ret;
}

void dng13_ambush(struct area_events far *ev)
{
	if (!g_dng13_ambush_won) {
		signed short r;
		signed short old;
		Stash far *st;

		old = dlg_set16(g_dialog, 5);
		/* BINGE5.LTX[12]: "Ploetzlich seht Ihr Euch sechs Gestalten in zerschlissenen Lederruestungen gegenueber, die
		 * Euch angreifen." */
		dlg_message(g_dialog, ev->txt[12]);
		r = event_fight2(ev, 0x73, 1, 0, 0x5f, 5, 1);
		if (r == 0) {
			g_dng13_ambush_won = 1;
			st = new Stash(0x29, 0);
			if (!st->has_items())
				g_dng13_stash29_empty = 1;
			delete st;
		}
		dlg_set16(g_dialog, old);
		return;
	}
	if (!g_dng13_stash29_empty) {
		Stash far *st;

		st = new Stash(0x29, 0);
		if (!st->has_items())
			g_dng13_stash29_empty = 1;
		delete st;
	}
}

void dng13_wash(struct area_events far *ev)
{
	signed short old;
	struct hero far *h;

	old = dlg_set16(g_dialog, 5);
	h = get_first_hero(g_group);
	do {
		if (check_hero3(h) && h->sooty) {
			/* BINGE5.LTX[29]: "%s nutzt die Gelegenheit um sich den Russ aus den Kleidern zu entfernen und sich das Gesicht
			 * gruendlich zu waschen." */
			sprintf(g_text_output_buf, ev->txt[29], h->name);
			dlg_message(g_dialog, g_text_output_buf);
			h->sooty = 0;
			change_attrib(h, ATTRIB_CH, 2);
		}
		h = get_next_hero(g_group, h);
	} while (get_first_hero(g_group) != h);
	dlg_set16(g_dialog, old);
}

void dng13_lava_shaft(struct area_events far *ev)
{
	signed short old;
	signed short rope;
	struct hero far *h;
	signed short item;
	register signed short sel = 0;
	register signed short slot;

	old = dlg_set16(g_dialog, 5);
	rope = group_find_item(g_group, ITEM_ROPE);
	h = get_hero_staffspell(g_group, 3);
	if (h != 0 || rope >= 0) {
		/* BINGE5.LTX[0]: "Wollt Ihr" */
		/* BINGE5.LTX[1]: "etwas in das Loch werfen?" */
		/* BINGE5.LTX[2]: "oder Euch abseilen?" */
		sel = dlg_choose(g_dialog, ev->txt[0], 2, ev->txt[1], ev->txt[2]);
	} else {
		sprintf(g_text_output_buf, "%s %s", ev->txt[0], ev->txt[1]);
		if (dlg_yes_no(g_dialog, g_text_output_buf) == 1)
			sel = 1;
	}
	if (sel == 1) {
		/* BINGE5.LTX[3]: "Wer soll etwas in das Loch werfen?" */
		h = get_active_hero(g_group, ev->txt[3]);
		if (h != 0) {
			/* BINGE5.LTX[4]: "Was soll %s hinunterwerfen?" */
			sprintf(g_text_output_buf, ev->txt[4], h->name);
			slot = select_item(h, g_text_output_buf, 0);
			if (slot != -1) {
				item = h->inv[slot].item_id;
				if (drop_item(h, slot, 1) == 1) {
					g_god_favor_ingerimm += 10;
					if (item == ITEM_FIRE_POWDER) {
						/* BINGE5.LTX[5]: "Ihr hoert brodelnde zischende Geraeusche aus dem Loch, dann wird es heiss im Raum. Im
						 * Schacht steigt gluehende Lava auf." */
						dlg_message(g_dialog, ev->txt[5]);
						/* BINGE5.LTX[6]: "abhauen?" */
						/* BINGE5.LTX[7]: "oder abwarten?" */
						sel = dlg_choose(g_dialog, ev->txt[0], 2, ev->txt[6], ev->txt[7]);
						if (sel == 1) {
							group_set_screen_pos(4, 4);
							cam_set_from_group();
							view_frame();
							/* BINGE5.LTX[8]: "Das war knapp, hinter Euch prasseln Brocken gluehender Lava im Raum nieder." */
							dlg_message(g_dialog, ev->txt[8]);
						} else {
							/* BINGE5.LTX[9]: "Urploetzlich wirft der Feuerschlund Brocken gluehender Lava in den Raum. Ihr habt keine
							 * Zeit, um zu reagieren. Das gluehende Gestein fuegt Euch schwere Verbrennungen zu." */
							dlg_message(g_dialog, ev->txt[9]);
							h = get_first_hero(g_group);
							do {
								sub_hero_le(h, random_schweif(11) + 4);
								h = get_next_hero(g_group, h);
							} while (get_first_hero(g_group) != h);
						}
					}
				} else {
					/* BINGE5.LTX[10]: "Es passiert nichts." */
					dlg_message(g_dialog, ev->txt[10]);
				}
			}
		}
	} else if (sel == 2) {
		if (h == 0)
			h = g_group->heroes[rope];
		if (!check_hero3(h))
			h = get_first_active_hero(g_group);
		/* BINGE5.LTX[11]: "Je tiefer sich %s abseilt, desto waermer wird es. Die Waende des Schachts bestehen aus
		 * Lavagestein. Da %s am Seil nicht weiter nach unten kommt, macht %s kehrt und zieht sich wieder nach oben." */
		sprintf(g_text_output_buf, ev->txt[11], h->name, grammar_pronoun(h->sex, 0, 0), grammar_pronoun(h->sex, 0, 0));
		dlg_message(g_dialog, g_text_output_buf);
	}
	dlg_set16(g_dialog, old);
}

void dng13_wheel(struct area_events far *ev)
{
	signed short old;
	struct hero far *h;
	struct group far *g;
	register signed short i;

	old = dlg_set16(g_dialog, 5);
	h = get_first_active_hero(g_group);
	if (!g_dng13_wheel_turned) {
		/* BINGE5.LTX[13]: "%s schaut sich das Rad genauer an, betrachtet dann die naehere Umgebung und laesst endlich %s
		 * Blicke durch den ganzen Raum schweifen, kann aber nichts erkennen, was mit dem Rad zusammenhaengen
		 * koennte...@Wollt Ihr das Rad drehen?" */
		sprintf(g_text_output_buf, ev->txt[13], h->name, grammar_pronoun(h->sex, 1, 1));
		if (dlg_yes_no(g_dialog, g_text_output_buf) == 1) {
			/* BINGE5.LTX[14]: "Zunaechst passiert nichts, doch nach ein paar Umdrehungen vernehmt Ihr ein aechzendes
			 * Geraeusch. %s dreht weiter an dem Rad bis es am Anschlag angekommen ist. Zwar koennt Ihr im ganzen Raum keine
			 * Veraenderung erkennen, aber aus der Ferne ist ein leises Rauschen zu vernehmen." */
			sprintf(g_text_output_buf, ev->txt[14], h->name);
			dlg_message(g_dialog, g_text_output_buf);
			g_dng13_wheel_turned = 1;
			event_set_cell(ev, 8, 1, 0x0c, 0, 0);
			event_set_cell(ev, 8, 9, 0x0c, 0, 0);
			event_set_cell(ev, 0x12, 10, 0x0c, 0, 1);
			event_set_cell(ev, 8, 10, 0x0d, 0, 0);
			for (i = 2; i <= 8; i++)
				event_set_cell(ev, 8, i, 0x0b, 0, 0);
			for (i = 9; i <= 0x11; i++)
				event_set_cell(ev, i, 10, 0x0b, 0, 1);
			automap_cell_write(g_map, 0x30a, 1, 8, 0x16);
			automap_cell_write(g_map, 0x30a, 9, 8, 0x17);
			automap_cell_write(g_map, 0x30a, 10, 0x12, 0x18);
			automap_cell_write(g_map, 0x30a, 10, 8, 0x19);
			for (i = 2; i <= 8; i++)
				automap_cell_write(g_map, 0x30a, i, 8, 0x14);
			for (i = 9; i <= 0x11; i++)
				automap_cell_write(g_map, 0x30a, 10, i, 0x15);
			for (i = 0, g = g_groups; i < 6; i++, g = &g_groups[i]) {
				if (g->dungeon == 0x0f) {
					g->old_dungeon = g->dungeon;
					g->dungeon = 0x0e;
				}
			}
			timewarp_minutes(g_date, 5L);
		}
	} else {
		/* BINGE5.LTX[15]: "Trotz groesster Anstrengungen gelingt es %s nicht das Rad zurueckzudrehen!" */
		sprintf(g_text_output_buf, ev->txt[15], h->name);
		dlg_message(g_dialog, g_text_output_buf);
	}
	dlg_set16(g_dialog, old);
}

void dng13_gargoyles(struct area_events far *ev)
{
	signed short old;
	register signed short r = 0;

	old = dlg_set16(g_dialog, 5);
	/* BINGE5.LTX[27]: "Als Ihr die Statuen genauer betrachtet, werden diese ploetzlich lebendig. Noch bevor Ihr
	 * begriffen habt, was los ist, greifen die vier Wasserspeier an." */
	dlg_message(g_dialog, ev->txt[27]);
	dlg_set16(g_dialog, old);
	r = event_fight2(ev, 0x61, 1, 0, 0x60, 3, 0);
	if (r == 0)
		dng13_gargoyles_gone((struct area_events far *)g_area_events);
}

void dng13_gargoyles_gone(struct area_events far *ev)
{
	event_set_cell(ev, 0x0c, 3, 0x13, 0, 0);
	event_set_cell(ev, 0x0c, 5, 0x13, 0, 1);
	event_set_cell(ev, 0x0e, 5, 0x13, 0, 2);
	event_set_cell(ev, 0x0e, 3, 0x13, 0, 3);
	automap_cell_write(g_map, 0x30a, 3, 0x0c, 0x2b);
	automap_cell_write(g_map, 0x30a, 5, 0x0c, 0x2c);
	automap_cell_write(g_map, 0x30a, 5, 0x0e, 0x2d);
	automap_cell_write(g_map, 0x30a, 3, 0x0e, 0x2e);
}

void dng13_fur_wall_cold(struct area_events far *ev)
{
	signed short old;
	signed short ok;
	struct hero far *h;
	struct hero far *h2;
	signed short k;
	signed short k2;
	register signed short pos;

	old = dlg_set16(g_dialog, 5);
	ok = dng13_fur_cure(ev, 0);
	h = 0;
	if (!g_dng13_smoke_timer) {
		if (ok != 0)
			h = dng13_fur_victim(ev, 0);
		if (h != 0) {
			if (test_attrib(h, ATTRIB_GE, 2) <= 0) {
				pos = group_find_item(g_group, ITEM_WATERSKIN);
				/* BINGE5.LTX[19]: "Es scheint so, als sei die Wand mit einem duennen Pelz ueberzogen, wohl der Ursprung des
				 * eigentuemlichen Farbenspiels." */
				dlg_message(g_dialog, ev->txt[19]);
				/* BINGE5.LTX[20]: "Als %s die Wand genauer betrachten will rutscht %s auf dem feuchten Boden aus und muss sich
				 * an der Wand abstuetzen." */
				sprintf(g_text_output_buf, ev->txt[20], h->name, grammar_pronoun(h->sex, 0, 0));
				dlg_message(g_dialog, g_text_output_buf);
				h->fur_hands = 6;
				if (pos >= 0) {
					h2 = g_group->heroes[pos];
					k = get_item_pos(h2, ITEM_WATERSKIN);
					if (h2->inv[k].quantity != 0) {
						/* BINGE5.LTX[21]: "Ein Teil des Pelzes bleibt an %s Fingern haengen, die jetzt in den gleichen Farben wie
						 * die Wand schimmern und rasch kuehl werden." */
						sprintf(g_text_buf2, ev->txt[21], hero_name_gen(h, 1));
						/* BINGE5.LTX[22]: "Wollt Ihr die Haende reinigen?" */
						sprintf(g_text_output_buf, "%s@%s", g_text_buf2, ev->txt[22]);
						if (dlg_yes_no(g_dialog, g_text_output_buf) == 1) {
							/* BINGE5.LTX[23]: "Der Pelz zeigt sich hartnaeckig und laesst sich nicht abwaschen. - Im uebrigen empfindet
							 * %s den Zustand seiner Haende zwar als unangenehm, aber es schmerzt nicht." */
							sprintf(g_text_output_buf, ev->txt[23], h->name);
							dlg_message(g_dialog, g_text_output_buf);
							k2 = get_item_pos(h, ITEM_WATERSKIN);
#if FIX_0140
							/* Fix #140: Wasser beim Traeger h2 an seiner eigenen Stelle k abziehen */
							h2->inv[k].quantity--;
#else
							h2->inv[k2].quantity--;
#endif
						}
					} else {
						pos = -1;
					}
				}
				if (pos == -1) {
					sprintf(g_text_output_buf, ev->txt[21], hero_name_gen(h, 1));
					dlg_message(g_dialog, g_text_output_buf);
				}
			} else {
				dlg_message(g_dialog, ev->txt[19]);
				dng13_fur_burn(ev, 0);
			}
		}
	}
	dlg_set16(g_dialog, old);
}

void dng13_salt_seam(struct area_events far *ev)
{
	signed short old;

	old = dlg_set16(g_dialog, 5);
	/* BINGE5.LTX[24]: "Wo die beiden Schichten aufeinander treffen ist eine dicke Salzschicht entstanden." */
	dlg_message(g_dialog, ev->txt[24]);
	dlg_set16(g_dialog, old);
}

void dng13_fur_wall_burning(struct area_events far *ev)
{
	signed short old;
	signed short ok;
	struct hero far *h;

	old = dlg_set16(g_dialog, 5);
	ok = dng13_fur_cure(ev, 1);
	h = 0;
	if (!g_dng13_smoke_timer) {
		if (ok != 0)
			h = dng13_fur_victim(ev, 1);
		if (h != 0) {
			if (test_attrib(h, ATTRIB_GE, 2) <= 0) {
				/* BINGE5.LTX[19]: "Es scheint so, als sei die Wand mit einem duennen Pelz ueberzogen, wohl der Ursprung des
				 * eigentuemlichen Farbenspiels." */
				dlg_message(g_dialog, ev->txt[19]);
				/* BINGE5.LTX[20]: "Als %s die Wand genauer betrachten will rutscht %s auf dem feuchten Boden aus und muss sich
				 * an der Wand abstuetzen." */
				sprintf(g_text_output_buf, ev->txt[20], h->name, grammar_pronoun(h->sex, 0, 0));
				dlg_message(g_dialog, g_text_output_buf);
				/* BINGE5.LTX[26]: "Ein Teil des Pelzes bleibt an %s Fingern haengen, die jetzt in den gleichen Farben wie die
				 * Wand schimmern." */
				sprintf(g_text_output_buf, ev->txt[26], hero_name_gen(h, 1));
				dlg_message(g_dialog, g_text_output_buf);
				h->fur_hands = 1;
			} else {
				dlg_message(g_dialog, ev->txt[19]);
				dng13_fur_burn(ev, 1);
			}
		}
	}
	dlg_set16(g_dialog, old);
}

signed short dng13_fur_cure(struct area_events far *ev, register signed short wall)
{
	struct hero far *h = 0;
	struct hero far *found = 0;
	signed short ret = 1;
	register signed short v;

	do {
		found = 0;
		h = get_first_hero(g_group);
		do {
			if (check_hero3(h)) {
				if (wall == 0) {
					if ((v = h->fur_hands) >= 1 && v <= 5)
						found = h;
				} else {
					if ((v = h->fur_hands) == 6)
						found = h;
				}
			}
			h = get_next_hero(g_group, h);
		} while (get_first_hero(g_group) != h && found == 0);
		if (found != 0) {
			/* BINGE5.LTX[16]: "Als %s die Wand beruehrt, beginnt es heftig in %s Haenden zu Kribbeln. Danach ist der
			 * seltsame Pelz verschwunden und ein weisses Pulver bleibt zurueck, das sich leicht abreiben laesst." */
			sprintf(g_text_output_buf, ev->txt[16], found->name, grammar_pronoun(found->sex, 3, 1));
			dlg_message(g_dialog, g_text_output_buf);
			found->fur_hands = -1;
			ret = 0;
		}
	} while (found != 0);
	return ret;
}

struct hero far *dng13_fur_victim(struct area_events far *ev, register signed short wall)
{
	struct hero far *h;
	register signed short v;

	h = get_first_hero(g_group);
	do {
		if ((v = h->fur_hands) == 0 && check_hero3(h))
			break;
		h = get_next_hero(g_group, h);
	} while (get_first_hero(g_group) != h);
	if ((v = h->fur_hands) != 0) {
		do {
			if ((v = h->fur_hands) == -1 && !check_hero3(h))
				break;
			h = get_next_hero(g_group, h);
		} while (get_first_hero(g_group) != h);
	}
	if (check_hero3(h)) {
		if ((wall == 0 && (v = h->fur_hands) == 6) || (wall == 1 && (v = h->fur_hands) >= 1 && v <= 5)) {
			dng13_fur_burn(ev, wall);
			h = 0;
		}
	}
	return h;
}

void dng13_fur_burn(struct area_events far *ev, register signed short wall)
{
	/* BINGE5.LTX[17]: "Wollt Ihr das Zeug abfackeln?" */
	if (dlg_yes_no(g_dialog, ev->txt[17]) == 1) {
		if (group_find_item(g_group, ITEM_TORCH_ON) < 0 && group_find_item(g_group, ITEM_LANTERN_ON) < 0 &&
		    group_find_item(g_group, ITEM_TINDERBOX) < 0) {
			/* BINGE5.LTX[28]: "Mit welcher Fackel denn, bitteschoen?" */
			dlg_message(g_dialog, ev->txt[28]);
			return;
		}
		if (wall == 0) {
			/* BINGE5.LTX[18]: "Als Ihr die Fackel an die Wand haltet, erlischt die Flamme." */
			dlg_message(g_dialog, ev->txt[18]);
			return;
		}
		if (wall == 1) {
			/* BINGE5.LTX[25]: "Das Zeug scheint ziemlich zaeh zu sein. Ihr muesst die Fackel schon eine ganze Weile auf
			 * einen Fleck halten, bis sich die Wand dort zunaechst dunkelblau, dann schwarz verfaerbt. Doch dann geht es
			 * schneller als Euch lieb ist: Der Fleck wird rasch groesser und die pelzige Schicht auf der Wand gibt einen
			 * beissenden Qualm von sich, der sich innerhalb kuerzester Zeit im ganzen Raum verteilt. Ihr koennt kaum
			 * atmen." */
			dlg_message(g_dialog, ev->txt[25]);
			g_dng13_smoke_timer = 6;
			event_set_cell(ev, 6, 9, 1, 0, -1);
			event_set_cell(ev, 6, 10, 1, 0, -1);
			automap_cell_write(g_map, 0x30a, 9, 6, 6);
			automap_cell_write(g_map, 0x30a, 10, 6, 6);
		}
	}
}
