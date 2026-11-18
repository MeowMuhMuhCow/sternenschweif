/* OVL060 - Overlay O060 (Segment 0x2061 in SCHWEIF.EXE): Kartenereignis-Verteiler des Dungeons 10, der Zwergenbinge
 * mit Ingerimm-Heiligtum (g_area_descs[10]; Texte BINGE2.LTX = Datei 0x402 ueber ev->txt; Ereignisse selbst
 * groesstenteils in O000 dng10_*). Ereignisobjekt ev (struct area_events), Kartenereignis mev (struct map_event).
 * dng10_handler(ev, mev): pos = ((mev->y - g_map_x0) << 8) | (mev->x - g_map_y0); MAP_EVENT_ENTER: 0x30c
 *   dng10_tools, 0x70c dng10_junk, 0x30e..0x70f dng10_sharp_object; 1 Tueren dng10_door; 0x64/0x65 Treppen
 *   (group_set_screen_pos, Dungeon 9 bzw. 11, Blickrichtung, g_cam_angle); 0x66 Kohlebecken dng10_coal_basin (0x1101,
 *   mit Ring) bzw. dng10_coal_basin_plain; 0x67 Portal dng10_portal_enter (0xc04) / dng10_portal_leave (0xd04) +
 *   cam_set_from_group; 0x68 dng10_skeleton; 0x69 dng10_ingerimm_statue (0x1108) bzw. dng10_relief (0xd13); 0x6a
 *   dng10_altar zwischen diary_unlock/diary_lock; 0x6b dng10_anvil; 0x6c Truhen 0x21..0x23 (new/delete Chest) bzw.
 *   dng10_rotten_chest; 0x6d binge_shaft (Kaminschacht); 0x6e dng10_countdown.
 * dng10_door(ev, pos, x, y): Schloss Lock 0x6c..0x76 je Feld, Kartenbild 0x2b bzw. 0x2c, bei g_lock_cell_bits != -1
 *   event_set_cell_bits + Klang 0x61 + automap_cell_write.
 * dng10_portal_enter/dng10_portal_leave: Hauptportal mit Goldschluessel (ITEM_GOLD_KEY) bzw. Silberschluessel
 *   (ITEM_SILVER_KEY); steckt der Schluessel noch im anderen Schloss (g_dng10_gold_key_in_lock /
 *   g_dng10_silver_key_in_lock), kommt er zurueck. Ohne Schluessel dng10_portal_locked (txt[7..10]: Dietriche,
 *   Foramen). Frage txt[13] bzw. txt[15] -> Gruppe durch das Portal (group_set_screen_pos, group_take_item,
 *   Schluessel
 *   steckt), sonst alle Gruppen dieses Dungeons am Portal (y 5 bzw. 3, x 12..13) auf die andere Seite versetzen.
 * dng10_skeleton: Skelett vor dem Hauptportal (txt[17..28]): Ledertasche mit Flaeschchen (ITEM_FLASK_GLASS,
 *   g_dng10_flask_taken) und Pergament (ITEM_RUNE_DOCUMENT, als inv_b4 markiert, g_dng10_parchment_taken).
 * dng10_ingerimm_statue: Ingerimmbildnis in der Nische (txt[29..30]).
 * dng10_coal_basin/dng10_coal_basin_plain: Kohlebecken (txt[31..33]) -> dng10_move_basin (txt[34..36]) bzw. Kohle
 *   durchsuchen: beim Becken mit dem Ring findet der beste in Sinnesschaerfe den Ring (txt[38]), Versuche
 *   dng10_grab_ring bis g_dng10_ring_taken (Held per get_active_hero(txt[40])).
 * dng10_move_basin: GE-Probe je Held (der erste mit misslungener Probe klemmt sich die Finger, txt[35]), txt[36].
 * dng10_grab_ring(ev, h): Koerperbeherrschung, MU+4, GE+4 - Schaden; Erfolg: Asthenilring (ITEM_ASTHENIL_RING),
 *   10 AP, FF -10 fuer einen Tag (add_effect).
 *
 * BCC -mlarge -3 -Ot -Og -Oi -c -Y -E..\bc\BIN\TASM.EXE OVL060.CPP -> 5307/5307 Bytes GLEICH (18.10.2026), cobjfix
 *   laeuft, kein eigenes _DATA. Ohne -Oi 1 Byte zu kurz (strcpy/strcat in dng10_portal_locked), ohne -Og 112 Bytes.
 *   (Namensfassung mit schweif.h: 19.10.2026 GLEICH, LINKGLEICH)
 * - Vorlage OVL061 (dng11_handler/dng11_door fast identisch, set_y_screen/set_y_v inline).
 * - Fall 0x64: [bp-7] ist ein deklariertes char-Local zwischen o und y: "old_dungeon = dungeon; t = dungeon = 9;
 *   old_dungeon = dungeon; dungeon = t;" (verschachtelter inline-Setter legt die Kopie hinter x/y).
 * - Gruppenschleifen: register i nur im aufruffreien else-Block -> CX (auf Funktionsebene SI, set_y-Kopie DI).
 * - dng10_skeleton: r/h/k im Block "else if (take_parchment)" hinter dem -Og-Zwischenwert
 *   (int)g_dng10_parchment_taken
 *   [bp-8]; inline hero::set_inv_b4(k) -> Parameterkopie [bp-12h]. dng10_coal_basin: choice ohne register ([bp-8]),
 *   done in SI.
 * - dng10_grab_ring: get_free_effect(h) hat nur h; die vorher gepushten 0, -10, 3, TICKS_PER_DAY gehoeren zu
 *   add_effect.
 * - dng10_door: letzter case ohne break vor "default: break;" (jmp short auf den Abschluss).
 * Originalfehler: dng10_portal_enter/dng10_portal_leave pruefen "x >= 12 || x <= 13" (immer wahr, gemeint &&). Fall
 *   0x64 setzt den Dungeon doppelt (old_dungeon enthaelt danach die neue Nummer 9 statt der alten); die
 *   Gruppenschleifen setzen per set_y_screen + set_y_v ebenso old_y auf die neue Position (wie O061 dng11_move_foes).
 */
#include <stdio.h>
#include <string.h>

#include "schweif.h"

/* Phase B: eigene Daten des Moduls (ds:5710..5758, dsana) - initialisierte Daten in Adressreihenfolge, danach der Literalpool */

/* automap_cell_write mit short als letztem Parameter (wie im Original deklariert) */
#define AUTOMAP_CELL_WRITE_W ((void (*)(void far *, signed short, signed short, signed short, signed short))automap_cell_write)

signed short dng10_door(struct area_events far *ev, signed short pos, signed short x, signed short y);
void dng10_portal_enter(struct area_events far *ev);
void dng10_portal_leave(struct area_events far *ev);
void dng10_portal_locked(struct area_events far *ev);
void dng10_skeleton(struct area_events far *ev);
void dng10_ingerimm_statue(struct area_events far *ev);
void dng10_coal_basin(struct area_events far *ev);
void dng10_coal_basin_plain(struct area_events far *ev);
void dng10_move_basin(struct area_events far *ev);
void dng10_grab_ring(struct area_events far *ev, struct hero far *h);

signed short dng10_handler(struct area_events far *ev, struct map_event far *mev)
{
	signed short ret = 0;
	Chest far *o;
	signed char t;
	signed short y;
	signed short x;
	register signed short pos;
	register signed short type;

	type = mev->type;
	pos = ((y = mev->y) - g_map_x0) << 8;
	pos |= (x = mev->x) - g_map_y0;

	switch (type) {
	case MAP_EVENT_ENTER:
		switch (pos) {
		case 0x30c:
			dng10_tools(ev);
			break;
		case 0x70c:
			dng10_junk(ev);
			break;
		case 0x30e:
		case 0x30f:
		case 0x40e:
		case 0x40f:
		case 0x50f:
		case 0x60e:
		case 0x60f:
		case 0x70e:
		case 0x70f:
			dng10_sharp_object(ev);
			break;
		}
		break;
	case 1:
		ret = dng10_door(ev, pos, x, y);
		break;
	case 0x64:
#if FIX_0143
		/* Fix #143: Dungeonwechsel nur einmal (alten Dungeon behalten) */
		g_group->old_dungeon = g_group->dungeon;
		g_group->dungeon = 9;
#else
		g_group->old_dungeon = g_group->dungeon;
		t = g_group->dungeon = 9;
		g_group->old_dungeon = g_group->dungeon;
		g_group->dungeon = t;
#endif
		group_set_screen_pos(2, 1);
		g_group->dir = 0;
		g_cam_angle = 0x100;
		break;
	case 0x65:
		group_set_screen_pos(1, 0xe);
		g_group->old_dungeon = g_group->dungeon;
		g_group->dungeon = 11;
		g_group->dir = 3;
		break;
	case 0x66:
		if (pos == 0x1101)
			dng10_coal_basin(ev);
		else
			dng10_coal_basin_plain(ev);
		break;
	case 0x67:
		if (pos == 0xc04)
			dng10_portal_enter(ev);
		else if (pos == 0xd04)
			dng10_portal_leave(ev);
		cam_set_from_group();
		break;
	case 0x68:
		dng10_skeleton(ev);
		break;
	case 0x69:
		if (pos == 0x1108)
			dng10_ingerimm_statue(ev);
		else if (pos == 0xd13)
			dng10_relief(ev);
		break;
	case 0x6a:
		diary_unlock(g_diary);
		dng10_altar(ev);
		diary_lock(g_diary);
		break;
	case 0x6b:
		dng10_anvil(ev);
		break;
	case 0x6c:
		switch (pos) {
		case 0x103:
			o = new Chest(0x21, 1);
			delete o;
			break;
		case 0x104:
			o = new Chest(0x22, 1);
			delete o;
			break;
		case 0x106:
			o = new Chest(0x23, 1);
			delete o;
			break;
		case 0x107:
			dng10_rotten_chest(ev);
			break;
		}
		break;
	case 0x6d:
		binge_shaft(ev);
		break;
	case 0x6e:
		dng10_countdown(ev);
		break;
	}
	return ret;
}

signed short dng10_door(struct area_events far *ev, signed short pos, signed short x, signed short y)
{
	signed short ret = 0;
	register signed short nr = -1;
	register signed short pic = 0x2b;

	switch (pos) {
	case 0xa02:
		nr = 0x6c;
		break;
	case 0xa06:
		nr = 0x6d;
		break;
	case 0x806:
		nr = 0x6e;
		break;
	case 0xa0d:
		nr = 0x6f;
		break;
	case 0x511:
		nr = 0x70;
		pic = 0x2c;
		break;
	case 0x705:
		nr = 0x71;
		pic = 0x2c;
		break;
	case 0x404:
		nr = 0x72;
		break;
	case 0x406:
		nr = 0x73;
		break;
	case 0x607:
		nr = 0x74;
		break;
	case 0x10b:
		nr = 0x75;
		pic = 0x2c;
		break;
	case 0x111:
		nr = 0x76;
		pic = 0x2c;
	default:
		break;
	}
	if (nr != -1) {
		Lock lk(nr);
		ret = 1;
		if (g_lock_cell_bits != -1) {
			event_set_cell_bits(ev, x, y, g_lock_cell_bits);
			snd_play(g_snd_fx, 0x61);
			AUTOMAP_CELL_WRITE_W(g_map, 0x2fe, y - g_map_x0, x - g_map_y0, pic);
		}
	}
	return ret;
}

void dng10_portal_enter(struct area_events far *ev)
{
	signed short old;
	struct group far *g;

	old = dlg_set16(g_dialog, 5);
	if (g_dng10_gold_key_in_lock != 0) {
		group_give_item(g_group, ITEM_GOLD_KEY, 1);
		g_dng10_gold_key_in_lock = 0;
	}
	if (group_find_key(g_group, ITEM_GOLD_KEY) < 0) {
		dng10_portal_locked(ev);
	} else {
		/* BINGE2.LTX[11]: "Ein leises Klicken bestaetigt, dass das Schloss entriegelt ist, gleichzeitig rastet der
		 * Schluessel im Schloss ein und laesst sich nicht mehr bewegen." */
		dlg_message(g_dialog, ev->txt[11]);
		/* BINGE2.LTX[12]: "Beinahe geraeuschlos gleitet der rechte Fluegel nach innen auf, dann ist erneut ein leises
		 * Klicken zu vernehmen und der Fluegel bewegt sich wieder auf Euch zu." */
		dlg_message(g_dialog, ev->txt[12]);
		/* BINGE2.LTX[13]: "Leise schliesst sich der Portalfluegel vor Euch, mit einem Klick verriegelt sich das Schloss
		 * und gibt den Schluessel wieder frei. Wollt Ihr die Binge betreten?" */
		if (dlg_yes_no(g_dialog, ev->txt[13]) == 1) {
			group_set_screen_pos(0xc, 5);
			/* BINGE2.LTX[14]: "Wieder rastet der Schluessel im Schloss ein und der rechte Fluegel gleitet nach innen auf.
			 * Schnell schluepft Ihr durch die entstandene Luecke, als sich das Portal auch schon wieder hinter Euch
			 * schliesst." */
			dlg_message(g_dialog, ev->txt[14]);
			group_take_item(g_group, ITEM_GOLD_KEY);
			g_dng10_gold_key_in_lock = 1;
			if (g_dng10_silver_key_in_lock != 0) {
				group_give_item(g_group, ITEM_SILVER_KEY, 1);
				g_dng10_silver_key_in_lock = 0;
			}
		} else {
			register signed short i;

			for (i = 0, g = g_groups; i < 6; g = &g_groups[++i]) {
#if FIX_0143
				/* Fix #143: nur Gruppen auf x 12..13 versetzen (&& statt ||), old_y behaelt die alte Zeile */
				if (g->dungeon == g_loaded_dungeon && g->y == 5 && (g->x >= 12 && g->x <= 13))
					g->set_y_screen(3);
#else
				if (g->dungeon == g_loaded_dungeon && g->y == 5 && (g->x >= 12 || g->x <= 13))
					g->set_y_v(g->set_y_screen(3));
#endif
			}
		}
	}
	dlg_set16(g_dialog, old);
}

void dng10_portal_leave(struct area_events far *ev)
{
	signed short old;
	struct group far *g;

	old = dlg_set16(g_dialog, 5);
	if (g_dng10_silver_key_in_lock != 0) {
		group_give_item(g_group, ITEM_SILVER_KEY, 1);
		g_dng10_silver_key_in_lock = 0;
	}
	if (group_find_key(g_group, ITEM_SILVER_KEY) < 0) {
		dng10_portal_locked(ev);
	/* BINGE2.LTX[15]: "Wollt Ihr die Binge verlassen?" */
	} else if (dlg_yes_no(g_dialog, ev->txt[15]) == 1) {
		group_set_screen_pos(0xd, 3);
		/* BINGE2.LTX[16]: "Ein leises Klicken bestaetigt, dass das Schloss entriegelt ist, gleichzeitig rastet der
		 * Schluessel im Schloss ein und laesst sich nicht mehr bewegen. Beinahe geraeuschlos gleitet der rechte Fluegel
		 * nach aussen auf, dann ist erneut ein mechanisches Klicken zu vernehmen und der Fluegel bewegt sich wieder auf
		 * Euch zu. Ihr muesst Euch beeilen, um hindurchzukommen." */
		dlg_message(g_dialog, ev->txt[16]);
		group_take_item(g_group, ITEM_SILVER_KEY);
		g_dng10_silver_key_in_lock = 1;
		if (g_dng10_gold_key_in_lock != 0) {
			group_give_item(g_group, ITEM_GOLD_KEY, 1);
			g_dng10_gold_key_in_lock = 0;
		}
	} else {
		register signed short i;

		for (i = 0, g = g_groups; i < 6; g = &g_groups[++i]) {
#if FIX_0143
			/* Fix #143: nur Gruppen auf x 12..13 versetzen (&& statt ||), old_y behaelt die alte Zeile */
			if (g->dungeon == g_loaded_dungeon && g->y == 3 && (g->x >= 12 && g->x <= 13))
				g->set_y_screen(5);
#else
			if (g->dungeon == g_loaded_dungeon && g->y == 3 && (g->x >= 12 || g->x <= 13))
				g->set_y_v(g->set_y_screen(5));
#endif
		}
	}
	dlg_set16(g_dialog, old);
}

void dng10_portal_locked(struct area_events far *ev)
{
	/* BINGE2.LTX[7]: "Keiner Eurer Schluessel passt in das Schloss." */
	strcpy(g_text_output_buf, ev->txt[7]);
	if (group_find_item(g_group, ITEM_PICKLOCKS) >= 0) {
		/* BINGE2.LTX[8]: "Auch mit Dietrichen ist das Portal nicht zu oeffnen." */
		sprintf(g_text_buf2, " %s", ev->txt[8]);
		strcat(g_text_output_buf, g_text_buf2);
	}
	if (get_mage_hero(g_group)) {
		/* BINGE2.LTX[9]: "Schliesslich muesst Ihr feststellen, dass auch ein Foramen das Portal nicht zu bewegen
		 * vermag." */
		sprintf(g_text_buf2, " %s", ev->txt[9]);
		strcat(g_text_output_buf, g_text_buf2);
	}
	/* BINGE2.LTX[10]: "Es bleibt Euch nichts anderes uebrig, als nach dem passenden Schluessel zu suchen." */
	sprintf(g_text_buf2, " %s", ev->txt[10]);
	strcat(g_text_output_buf, g_text_buf2);
	dlg_message(g_dialog, g_text_output_buf);
}

void dng10_skeleton(struct area_events far *ev)
{
	signed short old;
	signed short take_parchment;
	signed short take_flask;
	register signed short done;
	register signed short choice;

	old = dlg_set16(g_dialog, 5);
	done = 0;
	while (!done) {
		take_parchment = take_flask = 0;
		/* BINGE2.LTX[17]: "Ein Skelett liegt direkt vor dem Hauptportal.@Der Verwesungsgeruch ist laengst verflogen,
		 * ausser ein paar Knochen und einer kleinen Ledertasche ist nichts von dem Toten uebriggeblieben." */
		/* BINGE2.LTX[0]: "Wollt Ihr" */
		sprintf(g_text_output_buf, "%s@%s", ev->txt[17], ev->txt[0]);
		/* BINGE2.LTX[18]: "das Skelett untersuchen?" */
		/* BINGE2.LTX[19]: "die Tasche inspizieren?" */
		/* BINGE2.LTX[20]: "oder Boron lassen, was des Boron ist?" */
		choice = dlg_choose(g_dialog, g_text_output_buf, 3, ev->txt[18], ev->txt[19], ev->txt[20]);
		switch (choice) {
		case 1:
			timewarp_minutes(g_date, 5L);
			/* BINGE2.LTX[21]: "Ausser den Knochen und besagter Ledertasche koennt Ihr nichts finden." */
			dlg_message(g_dialog, ev->txt[21]);
			break;
		case 2:
			timewarp_minutes(g_date, 5L);
			if (!g_dng10_parchment_taken && !g_dng10_flask_taken) {
				/* BINGE2.LTX[22]: "Die Tasche ist besser erhalten als Ihr Besitzer.@Ihr findet ein leeres Flaeschchen und
				 * einen Fetzen Pergament." */
				sprintf(g_text_output_buf, "%s@%s", ev->txt[22], ev->txt[0]);
				/* BINGE2.LTX[26]: "das Flaeschchen untersuchen?" */
				/* BINGE2.LTX[27]: "das Pergament mitnehmen?" */
				choice = dlg_choose(g_dialog, g_text_output_buf, 2, ev->txt[26], ev->txt[27]);
				if (choice == 1)
					take_flask = 1;
				else if (choice == 2)
					take_parchment = 1;
				else
					done = 1;
			} else if (!g_dng10_parchment_taken) {
				/* BINGE2.LTX[23]: "In der Tasche ist noch der Pergamentfetzen." */
				sprintf(g_text_output_buf, "%s@%s %s", ev->txt[23], ev->txt[0], ev->txt[27]);
				if (dlg_yes_no(g_dialog, g_text_output_buf) == 1)
					take_parchment = 1;
				else
					done = 1;
			} else if (!g_dng10_flask_taken) {
				/* BINGE2.LTX[24]: "In der Tasche ist noch das leere Flaeschchen." */
				sprintf(g_text_output_buf, "%s@%s %s", ev->txt[24], ev->txt[0], ev->txt[26]);
				if (dlg_yes_no(g_dialog, g_text_output_buf) == 1)
					take_flask = 1;
				else
					done = 1;
			} else {
				/* BINGE2.LTX[25]: "In der Tasche ist nichts mehr zu finden." */
				dlg_message(g_dialog, ev->txt[25]);
				done = 1;
			}
			break;
		default:
			done = 1;
			break;
		}
		if (take_flask != 0) {
			timewarp_minutes(g_date, 5L);
			/* BINGE2.LTX[28]: "Das Flaeschchen ist leer, dem Geruch nach zu urteilen muss frueher einmal ein Heiltrank
			 * darin gewesen sein. Wollt Ihr das Flaeschchen an Euch nehmen?" */
			if (dlg_yes_no(g_dialog, ev->txt[28]) == 1 && group_give_item(g_group, ITEM_FLASK_GLASS, 1))
				g_dng10_flask_taken = 1;
		} else if (take_parchment != 0) {
			signed short r;
			struct hero far *h;
			signed short k;

			if (group_give_item(g_group, ITEM_RUNE_DOCUMENT, 1)) {
				r = group_find_item(g_group, ITEM_RUNE_DOCUMENT);
				if (r != -1) {
					h = g_group->heroes[r];
					if (h != 0) {
						k = get_item_pos(h, ITEM_RUNE_DOCUMENT);
						if (k != -1) {
							h->set_inv_b4(k);
							g_dng10_parchment_taken = 1;
						}
					}
				}
			}
		}
	}
	dlg_set16(g_dialog, old);
}

void dng10_ingerimm_statue(struct area_events far *ev)
{
	signed short old;

	old = dlg_set16(g_dialog, 5);
	/* BINGE2.LTX[29]: "In einer breiten Nische ist ein Ingerimmbildnis von etwa zweieinhalb Schritt Groesse in die
	 * Wand gemeisselt.@Wollt Ihr das Bildnis untersuchen?" */
	if (dlg_yes_no(g_dialog, ev->txt[29]) == 1) {
		timewarp_minutes(g_date, 5L);
		/* BINGE2.LTX[30]: "Zweifelsohne ein solides Kunstwerk." */
		dlg_message(g_dialog, ev->txt[30]);
	}
	dlg_set16(g_dialog, old);
}

void dng10_coal_basin(struct area_events far *ev)
{
	struct hero far *h;
	signed short old;
	signed short choice;
	register signed short done;

	old = dlg_set16(g_dialog, 5);
	/* BINGE2.LTX[31]: "Das Kohlebecken ist bis zum Rand mit verbrannter Holzkohle gefuellt.@Wollt Ihr" */
	/* BINGE2.LTX[32]: "das Becken untersuchen?" */
	/* BINGE2.LTX[33]: "die Kohle durchsuchen?" */
	choice = dlg_choose(g_dialog, ev->txt[31], 2, ev->txt[32], ev->txt[33]);
	switch (choice) {
	case 1:
		dng10_move_basin(ev);
		break;
	case 2:
		timewarp_minutes(g_date, 5L);
		h = get_best_hero_skill(g_group, TA_SINNESSCHAERFE);
		if (test_skill(h, TA_SINNESSCHAERFE, 0) <= 0 || g_dng10_ring_taken) {
			/* BINGE2.LTX[37]: "Gewoehnliche, verbrannte Holzkohle. Nach mehreren Minuten habt Ihr alle schwarze Finger,
			 * aber nichts entdeckt." */
			dlg_message(g_dialog, ev->txt[37]);
		} else {
			done = 0;
			/* BINGE2.LTX[38]: "Mit einem eher gelangweilten Blick durchwuehlt %s die Holzkohle, doch ploetzlich tritt %s
			 * naeher und beginnt aufgeregt darin zu wuehlen. In wenigen Augenblicken legt %s einen kleinen Ring aus
			 * dunkelrot schimmerndem Metall frei." */
			sprintf(g_text_buf2, ev->txt[38], h->name, grammar_pronoun(h->sex, 0, 0), grammar_pronoun(h->sex, 0, 0));
			/* BINGE2.LTX[39]: "Wollt Ihr den Ring an Euch nehmen?" */
			sprintf(g_text_output_buf, "%s@%s", g_text_buf2, ev->txt[39]);
			if (dlg_yes_no(g_dialog, g_text_output_buf) == 1)
				dng10_grab_ring(ev, h);
			else
				done = 1;
			while (!g_dng10_ring_taken && !done) {
				if (g_refresh_portraits != 0)
					screen_portraits(g_screen);
				if (dlg_yes_no(g_dialog, ev->txt[39]) == 1) {
					preselect_agile_hero(g_group);
					/* BINGE2.LTX[40]: "Wer soll es diesmal versuchen?" */
					h = get_active_hero(g_group, ev->txt[40]);
					if (h != 0)
						dng10_grab_ring(ev, h);
					else
						done = 1;
				} else {
					done = 1;
				}
			}
			if (!done)
				/* BINGE2.LTX[46]: "Der Ring besteht offensichtlich aus dem gleichen Material wie das Kohlebecken. Obwohl er in
				 * den Flammen gelegen hat, fuehlt sich das Metall kuehl an." */
				dlg_message(g_dialog, ev->txt[46]);
		}
		break;
	default:
		break;
	}
	dlg_set16(g_dialog, old);
}

void dng10_coal_basin_plain(struct area_events far *ev)
{
	signed short old;
	signed short choice;

	old = dlg_set16(g_dialog, 5);
	/* BINGE2.LTX[31]: "Das Kohlebecken ist bis zum Rand mit verbrannter Holzkohle gefuellt.@Wollt Ihr" */
	/* BINGE2.LTX[32]: "das Becken untersuchen?" */
	/* BINGE2.LTX[33]: "die Kohle durchsuchen?" */
	choice = dlg_choose(g_dialog, ev->txt[31], 2, ev->txt[32], ev->txt[33]);
	switch (choice) {
	case 1:
		dng10_move_basin(ev);
		break;
	case 2:
		timewarp_minutes(g_date, 5L);
		/* BINGE2.LTX[37]: "Gewoehnliche, verbrannte Holzkohle. Nach mehreren Minuten habt Ihr alle schwarze Finger, aber
		 * nichts entdeckt." */
		dlg_message(g_dialog, ev->txt[37]);
		break;
	default:
		break;
	}
	dlg_set16(g_dialog, old);
}

void dng10_move_basin(struct area_events far *ev)
{
	struct hero far *h;
	struct hero far *h2;
	register signed short found;

	h = get_first_hero(g_group);
	found = 0;
	timewarp_minutes(g_date, 5L);
	/* BINGE2.LTX[34]: "Das Metall ist Euch voellig unbekannt und das Becken ist wesentlich schwerer als Ihr vermutet
	 * habt, aber mit vereinten Kraeften koenntet Ihr es wahrscheinlich beiseite schieben. Wollt Ihr es versuchen?" */
	if (dlg_yes_no(g_dialog, ev->txt[34]) == 1) {
		do {
			if (check_hero3(h) && test_attrib(h, ATTRIB_GE, 0) <= 0) {
				found = 1;
				break;
			}
			h = get_next_hero(g_group, h);
		} while (get_first_hero(g_group) != h);
		if (found) {
			/* BINGE2.LTX[35]: "Beim Verruecken des Kohlebeckens hat sich %s die Finger eingeklemmt." */
			sprintf(g_text_output_buf, ev->txt[35], h->name);
			dlg_message(g_dialog, g_text_output_buf);
		}
		timewarp_minutes(g_date, 5L);
		h2 = get_first_active_hero(g_group);
		/* BINGE2.LTX[36]: "Ihr schiebt das Becken beiseite. An der Stelle, wo das Becken zuvor gestanden hat, zeichnet
		 * sich ein Fleck hell gegenueber dem uebrigen Gestein ab. %s untersucht den Fleck eingehend, kann aber nichts
		 * Besonderes erkennen. Noch einmal bemueht Ihr Eure Kraefte, dann steht das Becken wieder da, wo es hingehoert." */
		sprintf(g_text_output_buf, ev->txt[36], h2->name);
		dlg_message(g_dialog, g_text_output_buf);
	}
}

void dng10_grab_ring(struct area_events far *ev, struct hero far *h)
{
	if (test_skill(h, TA_KOERPERBEHERRSCHUNG, 0) <= 0) {
		/* BINGE2.LTX[41]: "%s schreit laut auf und zieht blitzartig %s verbrannten Finger aus der Glut." */
		sprintf(g_text_buf2, ev->txt[41], h->name, grammar_pronoun(h->sex, 1, 1));
		/* BINGE2.LTX[45]: "Ploetzlich flammt die Kohle im Becken auf." */
		sprintf(g_text_output_buf, "%s %s", ev->txt[45], g_text_buf2);
		dlg_message(g_dialog, g_text_output_buf);
		sub_hero_le(h, random_schweif(2) + 2);
	} else if (test_attrib(h, ATTRIB_MU, 4) <= 0) {
		/* BINGE2.LTX[42]: "%s zieht erschrocken %s Finger zurueck." */
		sprintf(g_text_buf2, ev->txt[42], h->name, grammar_pronoun(h->sex, 1, 1));
		sprintf(g_text_output_buf, "%s %s", ev->txt[45], g_text_buf2);
		dlg_message(g_dialog, g_text_output_buf);
		sub_hero_le(h, 1);
	} else {
		if (test_attrib(h, ATTRIB_GE, 4) <= 0) {
			/* BINGE2.LTX[43]: "%s verzieht unter Schmerzen das Gesicht und versucht, den Ring aus der gluehenden Kohle zu
			 * ziehen, muss jedoch vor den Schmerzen kapitulieren. Schreiend zieht %s die Finger aus dem lodernden Feuer." */
			sprintf(g_text_buf2, ev->txt[43], h->name, grammar_pronoun(h->sex, 0, 0));
			sprintf(g_text_output_buf, "%s %s", ev->txt[45], g_text_buf2);
			dlg_message(g_dialog, g_text_output_buf);
		} else {
			/* BINGE2.LTX[44]: "Mit zusammengebissenen Zaehnen zieht %s den Ring aus der gluehenden Kohle, %s Haende
			 * schmerzen fuerchterlich." */
			sprintf(g_text_buf2, ev->txt[44], h->name, grammar_pronoun(h->sex, 1, 1));
			sprintf(g_text_output_buf, "%s %s", ev->txt[45], g_text_buf2);
			dlg_message(g_dialog, g_text_output_buf);
#if FIX_0361
			/* Fix #361: Ring bei vollem Rucksack ueber die Gruppe geben (fragt nach Ablegen), Merker nur bei Erfolg */
			if (give_hero_new_item(h, ITEM_ASTHENIL_RING, 1, 1) > 0 || group_give_item(g_group, ITEM_ASTHENIL_RING, 1) > 0)
				g_dng10_ring_taken = 1;
#else
			give_hero_new_item(h, ITEM_ASTHENIL_RING, 1, 1);
			g_dng10_ring_taken = 1;
#endif
			add_hero_ap(h, 10);
		}
		sub_hero_le(h, random_schweif(3) + 3);
		add_effect(h, get_free_effect(h), TICKS_PER_DAY, ATTRIB_FF, -10, EFFECT_ATTRIB);
	}
	timewarp_minutes(g_date, 5L);
}
