/* OVL132 - Overlay O132 (Segment 0x2202 in SCHWEIF.EXE): Ereignisse des Dungeons 1, der ersten Ebene des Gewoelbes
 * unter dem Haus in Tjolmar (Texte FINAL.LTX = Datei 0x2ad, gemeinsam mit Dungeon 2/3; g_area_descs[1]);
 * Ereignisobjekt ev (struct area_events, Texte ev->txt[65..124]), Kartenereignis mev.
 * dng01_handler(ev, mev): pos = (Zeile - g_map_x0) << 8 | (Spalte - g_map_y0). MAP_EVENT_ENTER: 0x106 angeketteter
 *   Troll auf der Truhe (FINAL[103], der Neugierigste naehert sich; Kampf 0xaa, g_dng01_troll_fought); verlaesst der
 *   Halter der Haken (g_dng01_hook_hero) das Ornament, schnappen sie zurueck (FINAL[84]); Illusionswaende 0x1811/
 *   0x1c0f schliessen sich wieder (dng01_move_groups, event_set_cell); einmalige Kaempfe (event_fight, Merker
 *   g_dng01_<gegner>_<pos>); Eisblock-Zustand 1/2 -> dng01_ice_warrior; 0x1111..0x1113 dng01_ghouls; Fallgitter
 *   dng01_grate_falls (0x1201/0x1302 beim ersten Mal, 0x1402/0x1501 danach ohne gehaltene Haken); ab
 *   g_dng01_branch_state 3 dng01_vanishing_wall. Typ 1: 0x1c00 Ausgang (FINAL[116] "wieder in das Haus hinaufgehen":
 *   view_leave, Dungeon 0, Stadt 6 auf (5, 0x0c); nach dem gewonnenen Endkampf g_final_fight_won stattdessen
 *   Spielende: swap_out_screen, g_game_state = 0x63), sonst dng01_door. 0x64: 0x1e14 dng01_scatter_trap, sonst Treppe
 *   nach Dungeon 2 auf (0x1d, 0x0b). 0x65 dng01_grate, 0x66 dng01_skull_ornament, 0x67 dng01_thorn_tree, 0x68
 *   dng01_blue_disc, 0x69 dng01_branches, 0x6a dng01_hesinde_statue (an 0x50d), 0x6b/0x70 dng01_ice_warrior, 0x6c
 *   Truhen (Chest 0x2e/0x2f), 0x6d/0x6e Schraenke (Stash 0x3c/0x3d, leer FINAL[119]; 0x6e an 0x1c0f/0x1811
 *   Illusionswand FINAL[120], an 0x1811 mit zwei Loechern FINAL[124]), 0x6f dng01_black_statue.
 * dng01_move_groups(x1, y1, x2, y2): alle Gruppen auf (x1, y1) in Ebene 1 nach (x2, y2) (inline set_pos_x/set_pos_y).
 * dng01_black_statue: schwarze Statue mit Statuette (FINAL[121]), Wandbilder 0x22/0x23.
 * dng01_scatter_trap: alle Helden der Gruppe einzeln (make_group, Vorlage g_dng01_scatter_init), Gruppen auf dem Feld
 *   auf eines von zehn Feldern g_dng01_scatter_pos versetzen (go_x/go_y, reveal_map), aktuelle Gruppe neu suchen.
 * dng01_door(ev, pos, x, y, mev): Schloesser (Lock 0xa8..0xaf, Wandbild 0x0c/0x0d); an 0x1311 bleibt der
 *   Flammenschluessel stecken (FINAL[118], group_take_item).
 * dng01_entry_fight (R12 bei g_tjolmar_vault == 1, Gewoelbe aus Tjolmar betreten; g_tjolmar_vault = 2): Kampf gegen 2
 *   x 0x40 und 3 x
 *   0x77, gewonnen -> dng01_document (FINAL[85], Pergament = Dokument 0x15a). dng01_melt_ice (O237 Hartes schmelze):
 *   Eisblock-Feld (1, 0x0c) wird frei.
 * dng01_ice_warrior (Zustand g_dng01_ice_state): 0 Eis beruehren (FINAL[110]/[113]: Frostschaeden-Probe, Wirkung);
 *   1/2 geschmolzen -> Kriegerin erwacht (Bild, FINAL[114], Ja/Nein GLOBLTXT[446]/[447]: als NSC 4),
 *   Flammenschluessel im Becken (FINAL[112]) -> 4; 4 Brunnen sprudelt (FINAL[111]).
 * dng01_hesinde_statue: ab g_god_favor_hesinde 100 - mit einem Zauberkundigen (AE >= 4) 4 AE fuer Hartes schmelze
 *   (FINAL[104]/[108]), sonst 7 LE fuer KL bzw. weniger Aberglauben (FINAL[106]/[109]); zu wenig Energie oder
 *   Ablehnung -> Zusammenbruch (FINAL[107]); 50 AP, 20 AP fuer alle; sonst FINAL[115].
 * dng01_ghouls: drei Ghule (FINAL[117]); der Held mit gelungener Totenangst-Probe (sonst der Totenaengstlichste)
 *   flieht als eigene Gruppe nach (0x15, 0x11), Kampf gegen 3 x 0x4f.
 * dng01_thorn_tree: Baeumchen im Kuebel (FINAL[86..92]) - Stich (Gift 2, 4 LE), Schluessel greifen
 *   (Koerperbeherrschung +10) -> Flammenschluessel.
 * dng01_grate: Metallgitter (FINAL[67..74]) - hochschieben nur, wenn der Haken-Halter an (0x0f, 2) steht.
 * dng01_branches / dng01_vanishing_wall: Aeste an 0x1e00, 0x1d03, 0x1f02 in dieser Folge ziehen laesst die Wand (2,
 *   0x1b) verschwinden (FINAL[93..95]); sie kehrt mit Knall zurueck (FINAL[96]) bzw. verschwindet beim Herantreten
 *   erneut (FINAL[97]).
 * dng01_skull_ornament: Totenkopf-Ornament (FINAL[75..83], Sinnesschaerfe +8/+12) - Haken in den Augenhoehlen ziehen
 *   und halten (g_dng01_hook_hero). dng01_blue_disc: blaue Scheibe (FINAL[98..102]) - Gong lockt Gegner an (Kampf
 *   0x64/0x77). dng01_grate_falls(ev, which): Fallgitter saust herunter (FINAL[65]/[66]); dem Haken-Halter an (0x0f,
 *   2) zerquetschen die Haken die Finger (FINAL[123], W4 LE).
 *
 * BCC -mlarge -3 -Ot -Og -c -Y OVL132.CPP -> 10667/10667 Bytes GLEICH (19.10.2026), cobjfix laeuft
 *   (g_dng01_scatter_init ds:6628 ohne eigenes Public -> cobjfix nimmt _g_6605+0x23). Mit -Oi ebenfalls gleich (keine
 *   Intrinsics), ohne -Og 6 Bytes zu lang, -Oe falsch. (Namensfassung mit schweif.h: 19.10.2026 GLEICH, LINKGLEICH)
 * - C++ wegen inline-Elementfunktionen der Gruppe (struct group, g_groups[6]) und Objekten: ZWEI Varianten:
 *   dng01_move_groups nutzt set_pos_x/set_pos_y (Kopien [bp-6]/[bp-8], wie O064), dng01_scatter_trap go_x/go_y
 *   (set_screen_x(sx); set_x(set_x(v)) -> zusaetzliche Ergebniskopie [bp-1a], wie O014). set_dungeon/set_town
 *   verschachtelt (char-Rueckgabe) -> -Og-Bytes [bp-9]/[bp-0a].
 * - automap_cell_write ueber AUTOMAP_CELL_WRITE_W (letzter Parameter short, wie OVL063).
 * - dng01_handler: type/pos register (DI/SI), mev->x/mev->y als -Og-Zwischenwerte [bp-0eh]/[bp-0ch] (an dng01_door
 *   weitergereicht), Chest-Objekt in zwei Geschwisterbloecken [bp-68h], Stash-Zeiger [bp-12h] im Block.
 *   "g_dng01_cultists_0806 = ret = 1" (ret zuerst gespeichert), "g_dng01_cultists_0806 = g_dng01_cultists_0e06 = 1" in
 *   dng01_blue_disc (0e06 zuerst).
 * - dng01_scatter_trap: Strukturkopie "lst = g_dng01_scatter_init" auf Funktionsebene -> keine Registervariablen
 *   (push si/di nur fuer rep movsw); "g = g_group" ist toter Store in den spaeteren Schleifenzeiger (gleicher Slot).
 * - dng01_move_groups ohne Aufrufe: i in CX, register-Parameter x2/y2 in SI/DI.
 * - dng01_blue_disc: die if/else-if-Kette ergibt dieselben Bytes, aber vertauschte Register (pos DI, ans SI) - erst
 *   die Form "if (..) { ..; return; } if (..) { ..; return; } .." gibt pos SI und ans DI (register oder nicht egal).
 * - dng01_ice_warrior: Zweige mit Epilogkopien (-Og), Bmp per new/delete (gb_new_alloc(0, ..)/gb_delete(b, 3)).
 * - dng01_entry_fight/dng01_melt_ice mit void-far-Parameter wie im Prototyp (Aufrufer R12/O237 uebergeben
 *   g_area_events).
 * Originalfehler: dng01_ice_warrior setzt im Zustand 3 nach Annahme des Flammenschluessels wieder 3 (in 1/2 wird 4
 *   gesetzt) -> der Schluessel laesst sich beliebig oft holen. dng01_hesinde_statue: der LE-Zweig prueft "!ans" statt
 *   "ans == 1" (AE-Zweig) -> wer zu wenig LE hat, bricht nur zusammen, wenn kein Held gewaehlt wurde.
 *   dng01_skull_ornament Fall 3 zeigt FINAL[79] ohne sprintf (Fall 1 setzt dort den Heldennamen ein). dng01_handler
 *   Typ 0x6e setzt ret im ersten Zweig doppelt; dng01_ghouls laesst "me" uninitialisiert, falls die aktuelle Gruppe
 *   nicht in g_groups steht; dng01_black_statue gibt das int-Ergebnis von dlg_yes_no als char zurueck (Aufrufer
 *   verwirft es).
 */
#include <stdio.h>

#include "schweif.h"

/* Phase B: eigene Daten des Moduls (ds:6600..6644, dsana) - initialisierte Daten in Adressreihenfolge */
struct point g_dng01_scatter_pos[10] = { { 6, 1 }, { 18, 3 }, { 13, 2 }, { 29, 7 }, { 6, 7 }, { 9, 8 }, { 13, 8 }, { 6, 17 }, { 7, 19 }, { 17, 17 } };
struct hero_ptrs7 g_dng01_scatter_init = { { 0, 0, 0, 0, 0, 0, 0 } };

/* automap_cell_write mit short als letztem Parameter (wie im Original deklariert) */
#define AUTOMAP_CELL_WRITE_W \
	((void (*)(void far *, signed short, signed short, signed short, signed short))automap_cell_write)

void dng01_move_groups(signed short x1, signed short y1, register signed short x2, register signed short y2)
{
	struct group far *g;
	signed short i;

	for (i = 0, g = g_groups; i < 6; g = &g_groups[++i]) {
		if (g->dungeon == 1 && g->x == x1 && g->y == y1) {
			g->set_pos_x(x2, x2 * 0xa8 + 0x54);
			g->set_pos_y(y2, y2 * 0xa8 + 0x54);
		}
	}
}

signed char dng01_black_statue(struct area_events far *ev)
{
	signed short ret = -1;

	if (g_dng01_illusion_wall1 != 2) {
		/* FINAL.LTX[121]: "Vor Euch steht eine grosse schwarze Statue in einer Nische.@In einem kleinem Fach enthaelt
		 * sie eine kleine Statuette, ein genaues Abbild der grossen.@Wollt Ihr die Statuette an Euch nehmen?" */
		ret = dlg_yes_no(g_dialog, ev->txt[121]);
		if (ret == 1) {
			group_give_item(g_group, ITEM_BLACK_STATUETTE, 1);
			snd_play(g_snd_fx, 0x90);
			timewarp_minutes(g_date, 10L);
			g_dng01_illusion_wall1 = 2;
			AUTOMAP_CELL_WRITE_W(g_map, 0x262, 0x18, 0x11, 0x23);
			AUTOMAP_CELL_WRITE_W(g_map, 0x262, 0x19, 0x11, 0x22);
		}
	}
	return ret;
}

void dng01_scatter_trap(struct area_events far *ev)
{
	struct hero_ptrs7 lst = g_dng01_scatter_init;
	struct hero far *h0 = get_first_active_hero(g_group);
	struct hero far *h = get_first_hero(g_group);
	struct group far *g = g_group;
	signed short x0 = g_group->x;
	signed short y0 = g_group->y;
	signed short idx = 0;
	signed short i = 0;

	diary_unlock(g_diary);
	do {
		if (h != 0) {
			lst.h[i] = h;
			i++;
		}
		h = get_next_hero(g_group, h);
	} while (get_first_hero(g_group) != h);

	for (i = 0; i < 7; i++) {
		if (lst.h[i]) {
			make_group(1, lst.h[i]);
			snd_play(g_snd_fx, 0x94);
		}
	}

	for (i = 0, g = g_groups; i < 6; g = &g_groups[++i]) {
		if (g->dungeon == 1 && g->x == x0 && g->y == y0) {
			idx = random_schweif(10) - 1;
			g->go_x(g_dng01_scatter_pos[idx].x, g_dng01_scatter_pos[idx].x * 0xa8 + 0x54);
			g->go_y(g_dng01_scatter_pos[idx].y, g_dng01_scatter_pos[idx].y * 0xa8 + 0x54);
			reveal_map(g_map, g->x, g->y);
		}
	}

	for (i = 0, g = g_groups; i < 6; g = &g_groups[++i]) {
		if (group_has_hero(g, h0)) {
			g_group = g;
			g_group_no = i;
			cam_set_from_group();
		}
	}
	screen_redraw(g_screen);
	diary_lock(g_diary);
}

signed short dng01_handler(struct area_events far *ev, struct map_event far *mev)
{
	register signed short type;
	register signed short pos;
	signed short ret;
	signed short ans;
	struct hero far *h;

	type = mev->type;
	pos = (mev->y - g_map_x0) << 8;
	pos |= mev->x - g_map_y0;
	ret = 0;

	if (type == MAP_EVENT_ENTER) {
		if (!g_dng01_troll_fought && pos == 0x0106) {
			h = get_hero_attrib_max(g_group, ATTRIB_NG);
			/* FINAL.LTX[103]: "Ploetzlich erhebt sich vor Euch ein riesiger Troll, der gerade noch auf einer Truhe gesessen
			 * hat, an die er mit einer armdicken Kette angekettet ist.@%s Neugier besiegt %s Angst vor dem maechtigen
			 * Gegner und %s naehert sich diesem langsam.@Derartig provoziert reisst der Troll wie tollwuetig an der Kette,
			 * die an einem rostigen Kettenglied auseinanderbricht." */
			sprintf(g_text_output_buf, ev->txt[103], hero_name_gen(h, 1), grammar_pronoun(h->sex, 1, 1),
				grammar_pronoun(h->sex, 0, 0));
			dlg_message(g_dialog, g_text_output_buf);
			if (start_fight(0xaa, 1) == 0)
				g_dng01_troll_fought = 1;
			ret = 1;
		}
		if (g_dng01_hooks_held == 1 && g_dng01_skull_state == 3 &&
			(pos == 0x1002 || pos == 0x1101 || pos == 0x1102 || pos == 0x1103)) {
			if (g_dng01_hook_hero != 0 && group_has_hero(g_group, g_dng01_hook_hero)) {
				g_dng01_hooks_held = 0;
				/* FINAL.LTX[84]: "Als %s %s Finger aus den Schaedeln nimmt, schnappen die Haken zurueck." */
				sprintf(g_text_output_buf, ev->txt[84], g_dng01_hook_hero->name,
					grammar_pronoun(g_dng01_hook_hero->sex, 1, 1));
				dlg_message(g_dialog, g_text_output_buf);
				g_dng01_hook_hero = 0;
			}
			ret = 1;
		}
		if (g_dng01_illusion_wall1 == 1 && pos == 0x1a11) {
			dng01_move_groups(0x18, 0x11, 0x19, 0x11);
			event_set_cell(ev, 0x11, 0x18, 0x0a, 0, -1);
			g_dng01_illusion_wall1 = 0;
		}
		if (g_dng01_illusion_wall2 == 1 && (pos == 0x1a0f || pos == 0x1d0e || pos == 0x1b0e || pos == 0x1b10)) {
			if (pos == 0x1d0e)
				dng01_move_groups(0x1c, 0x0f, 0x1d, 0x0f);
			else
				dng01_move_groups(0x1c, 0x0f, 0x1b, 0x0f);
			event_set_cell(ev, 0x0f, 0x1c, 0x0a, 0, -1);
			snd_play(g_snd_fx, 0x7c);
			g_dng01_illusion_wall2 = 0;
		}
		if ((pos == 0x0806 || pos == 0x0807) && !g_dng01_cultists_0806 &&
			event_fight(ev, 0x36, 1, 0, 0, 0x64, 1, 2, 3, 0x77, 1, 2, 1) == 0)
			g_dng01_cultists_0806 = ret = 1;
		if ((pos == 0x0e06 || pos == 0x0e07) && !g_dng01_cultists_0e06 &&
			event_fight(ev, 0x36, 1, 0, 0, 0x64, 1, 2, 3, 0x77, 1, 2, 1) == 0)
			g_dng01_cultists_0e06 = ret = 1;
		if ((g_dng01_ice_state == 1 || g_dng01_ice_state == 2) && (pos == 0x010c || pos == 0x020c || pos == 0x030c ||
			pos == 0x020d || pos == 0x030d || pos == 0x010e || pos == 0x020e || pos == 0x030e)) {
			dng01_ice_warrior(ev);
			ret = 1;
		}
		if (!g_dng01_ghouls_fought && (pos == 0x1111 || pos == 0x1112 || pos == 0x1113)) {
			dng01_ghouls(ev);
			ret = 1;
		}
		if (!g_dng01_cave_spiders_1808 && (pos == 0x1808 || pos == 0x1908 || pos == 0x1a08 || pos == 0x1907)) {
			if (event_fight(ev, 0x26, 4, 0, 0, 0x27, 3, 0, 0, 0x27, 4, 2, 5) == 0)
				g_dng01_cave_spiders_1808 = 1;
			ret = 1;
		}
		if (!g_dng01_cave_spiders_0101 && (pos == 0x0101 || pos == 0x0102 || pos == 0x0201)) {
			if (event_fight(ev, 0x25, 3, 0, 0, 0x27, 2, 0, 0, 0, 0, 0, 0) == 0)
				g_dng01_cave_spiders_0101 = 1;
			ret = 1;
		}
		if (pos == 0x1201 || pos == 0x1302) {
			if (g_dng01_grate_first == 0)
				dng01_grate_falls(ev, 1);
			ret = 1;
		}
		if (pos == 0x1402 || pos == 0x1501) {
			if (g_dng01_grate_first == 1 && g_dng01_hooks_held == 0)
				dng01_grate_falls(ev, 2);
			ret = 1;
		}
		if (g_dng01_branch_state >= 3) {
			dng01_vanishing_wall(ev, pos);
			ret = 1;
		}
	} else if (type == 1) {
		if (pos == 0x1c00) {
			/* FINAL.LTX[116]: "Wollt Ihr wieder in das Haus hinaufgehen?" */
			ans = dlg_yes_no(g_dialog, ev->txt[116]);
			if (ans == 1) {
				if (!g_final_fight_won) {
					view_leave();
					g_group->set_dungeon(g_group->set_dungeon(0));
					g_group->set_town(g_group->set_town(6));
					group_set_screen_pos(5, 0x0c);
					g_group->dir = 0;
					g_pp20_index = -1;
					g_refresh = 1;
					g_loaded_dungeon = -1;
				} else {
					swap_out_screen();
					g_game_state = 0x63;
				}
			} else
				group_set_screen_pos(g_group->x, 1);
			ret = 1;
		} else
			ret = dng01_door(ev, pos, mev->x, mev->y, mev);
	} else if (type == 0x64) {
		if (pos == 0x1e14)
			dng01_scatter_trap(ev);
		else {
			group_set_screen_pos(0x1d, 0x0b);
			g_group->old_dungeon = g_group->dungeon;
			g_group->dungeon = 2;
			g_group->dir = 1;
		}
		ret = 1;
	} else if (type == 0x65 && pos == 0x1301) {
		dng01_grate(ev);
		ret = 1;
	} else if (type == 0x66 && (pos == 0x0f01 || pos == 0x0f03)) {
		dng01_skull_ornament(ev);
		ret = 1;
	} else if (type == 0x67 && pos == 0x1102 && g_dng01_thorn_tree_state < 2) {
		dng01_thorn_tree(ev);
		ret = 1;
	} else if (type == 0x68) {
		dng01_blue_disc(ev, pos);
		ret = 1;
	} else if (type == 0x69) {
		dng01_branches(ev, pos);
		ret = 1;
	} else if (type == 0x6a && !g_dng01_hesinde_done) {
		if (pos == 0x050d && g_group->x == 5) {
			dng01_hesinde_statue(ev);
			ret = 1;
		} else
			timewarp_minutes(g_date, 10L);
	} else if (type == 0x6b && g_dng01_ice_state <= 4) {
		dng01_ice_warrior(ev);
		ret = 1;
	} else if (type == 0x6c) {
		if (pos == 0x0208) {
			Chest c(0x2e, 1);
		} else if (pos == 0x1114) {
			Chest c(0x2f, 1);
		}
		ret = 1;
	} else if (type == 0x6d) {
		Stash far *l;
		if (pos == 0x0703) {
			l = new Stash(0x3c, 1);
			if (l->has_items() == 1)
				l->show();
			else
				/* FINAL.LTX[119]: "Ihr habt die Schraenke bereits leergeraeumt." */
				dlg_message(g_dialog, ev->txt[119]);
			delete l;
		} else if (pos == 0x0602) {
			l = new Stash(0x3c, 1);
			if (l->has_items() == 1)
				l->show();
			else
				dlg_message(g_dialog, ev->txt[119]);
			delete l;
		} else {
			l = new Stash(0x3d, 1);
			if (l->has_items() == 1)
				l->show();
			else
				dlg_message(g_dialog, ev->txt[119]);
			delete l;
		}
		timewarp_minutes(g_date, 10L);
		ret = 1;
	} else if (type == 0x6e) {
		Stash far *l;
		if (pos == 0x0702) {
			l = new Stash(0x3c, 1);
			if (l->has_items() == 1)
				l->show();
			else
				dlg_message(g_dialog, ev->txt[119]);
			delete l;
		}
		h = get_first_active_hero(g_group);
		if (pos == 0x1c0f && g_dng01_illusion_wall2 == 0) {
			g_dng01_illusion_wall2 = 1;
			/* FINAL.LTX[120]: "<Nanu, da ist ja gar keine Wand>, meint %s ploetzlich.@<Alles nur eine Illusion.>" */
			sprintf(g_text_output_buf, ev->txt[120], h->name);
			dlg_message(g_dialog, g_text_output_buf);
			event_set_cell(ev, 0x0f, 0x1c, 7, 0, -1);
			ret = 1;
		} else if (pos == 0x1811 && g_dng01_illusion_wall1 == 0) {
			g_dng01_illusion_wall1 = 1;
			sprintf(g_text_output_buf, ev->txt[120], h->name);
			dlg_message(g_dialog, g_text_output_buf);
			event_set_cell(ev, 0x11, 0x18, 7, 0, -1);
			AUTOMAP_CELL_WRITE_W(g_map, 0x262, 0x18, 0x11, 0x23);
			AUTOMAP_CELL_WRITE_W(g_map, 0x262, 0x19, 0x11, 0x22);
			view_frame();
			/* FINAL.LTX[124]: "An der Stelle, an der gerade noch die Wand zu sehen war, koennt Ihr zwei kleine Loecher am
			 * Boden erkennen, die sich gegenueberliegen." */
			dlg_message(g_dialog, ev->txt[124]);
		}
		ret = 1;
	} else if (type == 0x6f) {
		dng01_black_statue(ev);
		ret = 1;
	} else if (type == 0x70 && g_dng01_ice_state <= 4) {
		dng01_ice_warrior(ev);
		ret = 1;
	}
	return ret;
}

signed short dng01_door(struct area_events far *ev, signed short pos, signed short x, signed short y,
	struct map_event far *mev)
{
	register signed short pic = 0;
	signed short ret = 0;
	register signed short idx = -1;

	switch (pos) {
	case 0x0409: idx = 0xa8; pic = 0x0c; break;
	case 0x0b06: idx = 0xa9; pic = 0x0c; break;
	case 0x1209: idx = 0xaa; pic = 0x0c; break;
	case 0x1b0b: idx = 0xab; pic = 0x0c; break;
	case 0x130b: idx = 0xac; pic = 0x0c; break;
	case 0x0b0b: idx = 0xad; pic = 0x0c; break;
	case 0x1311: idx = 0xae; pic = 0x0d; break;
	case 0x1d0b: idx = 0xaf; pic = 0x0c; break;
	}
	if (idx != -1) {
		Lock lk(idx);
		ret = 1;
		if (g_lock_cell_bits != -1) {
			if (pos == 0x1311) {
				/* FINAL.LTX[118]: "Es gelingt Euch zwar die Tuer zu oeffnen, aber der Schluessel ist nicht mehr aus dem
				 * Schloss zu bekommen." */
				dlg_message(g_dialog, ev->txt[118]);
				group_take_item(g_group, 0x15b);
			}
			event_set_cell_bits(ev, x, y, g_lock_cell_bits);
			AUTOMAP_CELL_WRITE_W(g_map, 0x262, mev->y - g_map_x0, mev->x - g_map_y0, pic);
			snd_play(g_snd_fx, 0x61);
		}
	}
	return ret;
}

void dng01_entry_fight(void far *ev)
{
	g_tjolmar_vault = 2;
	if (event_fight2(ev, 0x40, 2, 0, 0x77, 3, 2) == 0)
		dng01_document((struct area_events far *)g_area_events);
}

void dng01_document(struct area_events far *ev)
{
	/* FINAL.LTX[85]: "Beim genaueren Dursuchen Eurer Gegner entdeckt Ihr ein Pergament, das Ihr an Euch nehmt." */
	dlg_message(g_dialog, ev->txt[85]);
	group_give_item(g_group, ITEM_DOCUMENT_15A, 1);
}

void dng01_melt_ice(void far *ev)
{
	event_set_cell(ev, 0x0c, 1, 0x0d, 0, -1);
	AUTOMAP_CELL_WRITE_W(g_map, 0x262, 1, 0x0c, 0x45);
	timewarp_minutes(g_date, 10L);
}

void dng01_ice_warrior(struct area_events far *ev)
{
	Bmp far *b;
	struct hero far *h;
	signed short w;
	signed short n;
	signed short ans;

	if (g_dng01_ice_state == 0) {
		/* FINAL.LTX[110]: "Das Wasser, das einst in dem Brunnen gesprudelt hat, ist zu langen Eiszapfen gefroren. In dem
		 * Becken koennt Ihr einen flammendroten Schluessel erkennen.@Dahinter steht ein riesiger Eisblock in dem der
		 * Koerper einer Kriegerin zu sehen ist.@Wollt Ihr das Eis beruehren?" */
		ans = dlg_yes_no(g_dialog, ev->txt[110]);
		if (ans == 1) {
			h = get_first_active_hero(g_group);
			/* FINAL.LTX[113]: "Als %s vorsichtig das Eis befuehlt, bemerkt %s, dass das Eis ueberhaupt nicht kalt ist, aber
			 * %s Hand langsam ganz eisig wird und sich kaum noch bewegen laesst.@Schnell zieht %s %s Hand zurueck." */
			sprintf(g_text_output_buf, ev->txt[113], h->name, grammar_pronoun(h->sex, 0, 0),
				grammar_pronoun(h->sex, 1, 1),
				h->name, grammar_pronoun(h->sex, 1, 1));
			dlg_message(g_dialog, g_text_output_buf);
			hero_disease_test(h, ILLNESS_FROSTSCHAEDEN, 0x14);
			add_effect(h, get_free_effect(h), 0x1518L, 3, -2, 0);
		}
		timewarp_minutes(g_date, 10L);
	} else if (g_dng01_ice_state == 1 || g_dng01_ice_state == 2) {
		b = new Bmp(0x20, 0x20);
		n = nvfile_count(g_nvf_heads, 5);
		nvfile_load(g_nvf_heads, 5, b->data(), 0, n - 8, 0, &w, &w, 0x140);
		/* FINAL.LTX[114]: "Mit einem leichten Stoehnen kommt die Kriegerin zu sich:@<Habt Dank, Freunde, Ihr habt mich
		 * aus meiner monatelangen Gefangenschaft befreit. Ein Untertan des Namenlosen Gottes hat mich hier mit einem
		 * Wunder in den Eisblock gebannt. Ich weiss nicht genau, was Euch in diese unfreundlichen Gewoelbe fuehrt, aber
		 * wenn Ihr es wuenscht, werde ich Euch begleiten und meinen Dank ueber die Befreiung mit Taten
		 * unterstreichen.>@Wollt Ihr die Kriegerin in Eure Reihen aufnehmen?" */
		/* GLOBLTXT.LTX[446]: "Ja" */
		/* GLOBLTXT.LTX[447]: "Nein" */
		ans = dlg_box_bmp(g_dialog, b, (char far *)0, ev->txt[114], 2, g_ttx[446], g_ttx[447]);
		delete b;
		if (ans == 1) {
			add_npc(4);
			screen_portraits(g_screen);
		}
		/* FINAL.LTX[112]: "Der Brunnen sprudelt in kleinen Fontaenen. In dem Becken seht Ihr einen flammendroten
		 * Schluessel.@Wollt Ihr ihn an Euch nehmen?" */
		ans = dlg_yes_no(g_dialog, ev->txt[112]);
		if (ans == 1) {
			group_give_item(g_group, ITEM_FLAME_KEY, 1);
			g_dng01_ice_state = 4;
		} else
			g_dng01_ice_state = 3;
		timewarp_minutes(g_date, 15L);
	} else if (g_dng01_ice_state == 3) {
		ans = dlg_yes_no(g_dialog, ev->txt[112]);
		if (ans == 1) {
#if FIX_0203
			/* Fix #203: nach dem Nehmen Zustand 4 wie in Zustand 1/2 (Schluessel nur einmal) */
			group_give_item(g_group, ITEM_FLAME_KEY, 1);
			g_dng01_ice_state = 4;
#else
			group_give_item(g_group, ITEM_FLAME_KEY, 1);
			g_dng01_ice_state = 3;
#endif
			timewarp_minutes(g_date, 5L);
		}
	} else if (g_dng01_ice_state == 4)
		/* FINAL.LTX[111]: "Der Brunnen sprudelt in kleinen Fontaenen, sonst ist an ihm nichts auffaelliges zu
		 * entdecken." */
		dlg_message(g_dialog, ev->txt[111]);
}

void dng01_hesinde_statue(struct area_events far *ev)
{
	struct hero far *h;
	register signed short ans;
	register signed short found = 0;

	if (g_god_favor_hesinde >= 100) {
		h = get_mage_hero(g_group);
		if (h != 0) {
			do {
				if (check_hero3(h) && h->ae >= 4)
					found = 1;
				h = get_next_mage(g_group, h);
			} while (!found && get_mage_hero(g_group) != h);
		}
		if (found) {
			/* FINAL.LTX[104]: "<Seid Ihr willens der Hesinde unwiederbringlich 4 Astralpunkte zu opfern?>, hoert Ihr
			 * ploetzlich eine weibliche Stimme scheinbar aus der Statue sprechen." */
			ans = dlg_yes_no(g_dialog, ev->txt[104]);
			if (ans == 1) {
				/* FINAL.LTX[105]: "Wer soll sich zu dem Opfer bereiterklaeren (die Zahl gibt die aktuelle Anzahl der Energie
				 * an)?" */
				h = select_hero_ae(g_group, ev->txt[105]);
				if (h == 0) {
					h = get_mage_hero(g_group);
					ans = 0;
				}
				if (h->ae < 4 && ans == 1) {
					/* FINAL.LTX[107]: "<Ihr Wuermer wollt Euch mit der Goettin der Weisheit anlegen!@Wie Ihr wollt!>@%s bricht
					 * schreiend vor der Statue zusammen.@<Lasst Euch das eine Lehre sein!>" */
					sprintf(g_text_output_buf, ev->txt[107], h->name);
					dlg_message(g_dialog, g_text_output_buf);
					sub_hero_le(h, h->le - 1);
					sub_ae_max(h, 4);
					change_attrib_normal(h, ATTRIB_AG, 1);
					g_dng01_hesinde_done = 1;
				} else {
					/* FINAL.LTX[108]: "%s spuert, wie die mentale Kraft %s Koerper verlaesst und ein Gefuehl der Mattheit
					 * zuruecklaesst. Allerdings meint %s auch, ein besseres Verstaendnis von einem Zauber zu haben." */
					sprintf(g_text_output_buf, ev->txt[108], h->name, grammar_pronoun(h->sex, 3,
						1),
						grammar_pronoun(h->sex, 0, 0));
					dlg_message(g_dialog, g_text_output_buf);
					sub_ae_max(h, 4);
					if (h->spells[SP_HARTES_SCHMELZE] <= 12)
						h->spells[SP_HARTES_SCHMELZE] = 15;
					else if (h->spells[SP_HARTES_SCHMELZE] <= 16)
						h->spells[SP_HARTES_SCHMELZE] += (signed char)2;
					if (h->attrib[ATTRIB_AG].normal > 2)
						change_attrib_normal(h, ATTRIB_AG, -1);
					g_dng01_hesinde_done = 1;
				}
				add_hero_ap(h, 0x32);
			}
		} else {
			/* FINAL.LTX[106]: "<Seid Ihr willens der Hesinde unwiederbringlich 7 Lebensenergiepunkte zu opfern?>, hoert Ihr
			 * ploetzlich eine weibliche Stimme scheinbar aus der Statue sprechen." */
			ans = dlg_yes_no(g_dialog, ev->txt[106]);
			if (ans == 1) {
				h = select_hero_le(g_group, ev->txt[105]);
				if (h == 0) {
					h = get_random_hero(g_group);
					ans = 0;
				}
#if FIX_0204
				/* Fix #204: zu wenig LE bestraft jeden Helden (nicht nur den Ersatzhelden) */
				if (h->le < 7) {
#else
				if (h->le < 7 && !ans) {
#endif
					sprintf(g_text_output_buf, ev->txt[107], h->name);
					dlg_message(g_dialog, g_text_output_buf);
					sub_hero_le(h, h->le - 1);
					change_attrib_normal(h, ATTRIB_AG, 1);
					g_dng01_hesinde_done = 1;
				} else {
					/* FINAL.LTX[109]: "%s spuert, wie die Kraft %s Koerper verlaesst. Gleichzeitig aber fuehlt %s, wie ein Teil
					 * der Weisheit Hesindes %s eigenes Wissen erweitert." */
					sprintf(g_text_output_buf, ev->txt[109], h->name, grammar_pronoun(h->sex, 3,
						1),
						grammar_pronoun(h->sex, 0, 0), grammar_pronoun(h->sex, 1, 0));
					dlg_message(g_dialog, g_text_output_buf);
					sub_le_max(h, 7);
					if (h->attrib[ATTRIB_KL].normal < 0x11)
						change_attrib_normal(h, ATTRIB_KL, 2);
					else if (h->attrib[ATTRIB_AG].normal > 2)
						change_attrib_normal(h, ATTRIB_AG, -1);
					g_dng01_hesinde_done = 1;
				}
				add_hero_ap(h, 0x32);
			}
		}
#if FIX_0368
		/* Fix #368: 20 AP je Held nur nach einem Opfer (Merker gesetzt), nicht bei jeder Ablehnung */
		if (g_dng01_hesinde_done)
			group_add_ap_each(g_group, 0x14);
#else
		group_add_ap_each(g_group, 0x14);
#endif
	} else
		/* FINAL.LTX[115]: "<Ihr Wuermer seid Hesindes Hilfe nicht wuerdig!>@hoert Ihr ploetzlich eine weibliche Stimme
		 * scheinbar aus der Statue sprechen." */
		dlg_message(g_dialog, ev->txt[115]);
	timewarp_minutes(g_date, 15L);
}

void dng01_ghouls(struct area_events far *ev)
{
	struct hero far *h;
	struct hero far *sel;
	signed short me;
	struct group far *old;
	struct group far *g;
	register signed short i;
	register signed short best = 0;

	sel = 0;
	h = get_first_hero(g_group);
	do {
		if (h != 0 && check_hero3(h)) {
			i = test_attrib(h, ATTRIB_TA, -2);
			if (i > 0 && best < 1) {
				sel = h;
				best = i;
			}
		}
		h = get_next_hero(g_group, h);
	} while (get_first_hero(g_group) != h);

	if (sel == 0 && group_count_check3(g_group) > 1)
		sel = get_hero_attrib_max(g_group, ATTRIB_TA);
	if (sel != 0) {
		/* FINAL.LTX[117]: "Aus dem Daemmerlicht des Raumes loesen sich ploetzlich drei haessliche Gestalten, die langsam
		 * auf Euch zuwanken.@<Ghule>@ruft %s; %s ist durch die Haesslichkeit und den Leichengestank der Ghule so
		 * veraengstigt, dass %s sich davon macht." */
		sprintf(g_text_output_buf, ev->txt[117], sel->name, grammar_pronoun(sel->sex, 0, 0),
			grammar_pronoun(sel->sex, 0, 0));
		dlg_message(g_dialog, g_text_output_buf);
		for (i = 0, g = g_groups; i < 6; g = &g_groups[++i]) {
			if (g == g_group)
				me = i;
		}
		diary_unlock(g_diary);
		make_group(1, sel);
		old = g_group;
		for (i = 0, g = g_groups; i < 6; g = &g_groups[++i]) {
			if (group_has_hero(g, sel)) {
				g_group = g;
				g_group_no = i;
			}
		}
		group_set_screen_pos(0x15, 0x11);
		reveal_map(g_map, g_group->x, g_group->y);
		g_group = old;
		g_group_no = me;
		diary_lock(g_diary);
		screen_redraw(g_screen);
	}
	if (event_fight1(ev, 0x4f, 3, 0) == 0)
		g_dng01_ghouls_fought = 1;
}

void dng01_thorn_tree(struct area_events far *ev)
{
	struct hero far *h;
	register signed short ans;

	if (g_dng01_thorn_tree_state == 0) {
		/* FINAL.LTX[86]: "Der Verrueckte, der diese Keller bewohnt hat, hat doch tatsaechlich ein Baeumchen in einem
		 * Kuebel herumstehen!@Das merkwuerdigste aber ist, dass der Baum keine Blaetter hat, sondern Nadeln und Dornen!" */
		dlg_message(g_dialog, ev->txt[86]);
		/* FINAL.LTX[87]: "Wollt Ihr den Baum naeher untersuchen?" */
		ans = dlg_yes_no(g_dialog, ev->txt[87]);
		if (ans == 1) {
			g_dng01_thorn_tree_state = 1;
			h = get_first_active_hero(g_group);
			/* FINAL.LTX[88]: "%s hat sich an einer Dorne gestochen! Sofort breitet sich eine leichte Mattheit in %s Koerper
			 * aus. Wenigstens konnte %s noch sehen, dass in dem Kuebel ein Schluessel schimmert, der gut unter den Dornen
			 * versteckt ist." */
			sprintf(g_text_output_buf, ev->txt[88], h->name, grammar_pronoun(h->sex, 3, 0),
				grammar_pronoun(h->sex, 0, 0));
			dlg_message(g_dialog, g_text_output_buf);
			hero_get_poison(h, 2);
			sub_hero_le(h, 4);
			screen_portraits(g_screen);
			timewarp_minutes(g_date, 5L);
		}
	}
	if (g_dng01_thorn_tree_state == 1) {
		/* FINAL.LTX[89]: "Wollt Ihr versuchen den Schluessel zu nehmen?" */
		ans = dlg_yes_no(g_dialog, ev->txt[89]);
		if (ans == 1) {
			preselect_best_skill(g_group, TA_KOERPERBEHERRSCHUNG);
			/* FINAL.LTX[90]: "Wer soll es versuchen?" */
			h = get_active_hero(g_group, ev->txt[90]);
			if (h != 0) {
				if (test_skill(h, TA_KOERPERBEHERRSCHUNG, 0x0a) <= 0) {
					/* FINAL.LTX[91]: "Auch %s kommt nicht an den Schluessel heran, ohne sich vorher zu stechen!@Trotzdem gelingt
					 * es %s beim Zurueckziehen der Hand den Schluessel festzuhalten." */
					sprintf(g_text_output_buf, ev->txt[91], h->name, grammar_pronoun(h->sex, 3,
						0));
					dlg_message(g_dialog, g_text_output_buf);
					hero_get_poison(h, 2);
					sub_hero_le(h, 3);
					screen_portraits(g_screen);
				} else {
					/* FINAL.LTX[92]: "%s gelingt es auf sehr geschickte Weise den Schluessel zu greifen, ohne die Dornen zu
					 * beruehren und den Schluessel herauszuziehen." */
					sprintf(g_text_output_buf, ev->txt[92], h->name);
					dlg_message(g_dialog, g_text_output_buf);
				}
				group_give_item(g_group, ITEM_FLAME_KEY, 1);
				g_dng01_thorn_tree_state = 2;
				timewarp_minutes(g_date, 10L);
			}
		}
	}
}

void dng01_grate(struct area_events far *ev)
{
	struct group far *g;
	signed short sel;
	register signed short i;

	/* FINAL.LTX[67]: "Der Weg ist Euch durch ein massives Metallgitter versperrt.@Wollt Ihr" */
	/* FINAL.LTX[68]: "es hochschieben" */
	/* FINAL.LTX[69]: "den Verschlussmechanismus knacken" */
	/* FINAL.LTX[70]: "einen Foramen sprechen" */
	sel = dlg_choose(g_dialog, ev->txt[67], 3, ev->txt[68], ev->txt[69], ev->txt[70]);
	switch (sel) {
	case 1:
		if (g_dng01_hooks_held == 1 && g_dng01_hook_hero != 0) {
			for (i = 0, g = g_groups; i < 6; g = &g_groups[++i]) {
				if (group_has_hero(g, g_dng01_hook_hero))
					break;
			}
			if (group_has_hero(g, g_dng01_hook_hero) && g->x == 0x0f && g->y == 2) {
				/* FINAL.LTX[74]: "Tatsaechlich koennt Ihr das Gitter nach oben schieben, bis es in der Decke einrastet." */
				dlg_message(g_dialog, ev->txt[74]);
				event_set_cell(ev, 1, 0x13, 0x0d, 0, -1);
				AUTOMAP_CELL_WRITE_W(g_map, 0x262, 0x13, 1, 0x41);
				g_dng01_hooks_held = 0;
			}
		} else
			/* FINAL.LTX[73]: "Das Gitter bewegt sich keinen fingerbreit nach oben, irgendein Mechanismus muss es verriegelt
			 * haben." */
			dlg_message(g_dialog, ev->txt[73]);
		break;
	case 2:
		/* FINAL.LTX[71]: "Ihr koennt nicht erkennen, wie das Gitter verschlossen ist." */
		dlg_message(g_dialog, ev->txt[71]);
		break;
	case 3:
		/* FINAL.LTX[72]: "Der Zauber laesst sich nur auf Schloesser anwenden; hier ist aber keines zu erkennen." */
		dlg_message(g_dialog, ev->txt[72]);
		break;
	}
	timewarp_minutes(g_date, 10L);
}

void dng01_vanishing_wall(struct area_events far *ev, signed short pos)
{
	struct group far *g;
	register signed short i;
	register signed short flag = 0;

	if ((pos == 0x1901 || pos == 0x1903 || pos == 0x1a04 || pos == 0x1a03 || pos == 0x1a01) &&
		g_dng01_branch_state != 4) {
		for (i = 0, g = g_groups; i < 6; g = &g_groups[++i]) {
			if (g->dungeon == 1 && g->y == 2 && (g->x == 0x1b || g->x == 0x1a))
				flag = -1;
		}
		if (flag != -1) {
			g_dng01_branch_state = 4;
			flag = 1;
		}
	}
	if ((pos == 0x1d01 || pos == 0x1d02 || pos == 0x1c01) && g_dng01_branch_state == 5) {
		for (i = 0, g = g_groups; i < 6; g = &g_groups[++i]) {
			if (g->dungeon == 1 && g->y == 2 && (g->x == 0x1b || g->x == 0x1a))
				flag = -1;
		}
		if (flag != -1) {
			g_dng01_branch_state = 0;
			flag = 1;
		}
	}
	if (flag == 1) {
		/* FINAL.LTX[96]: "Ihr hoert einen lauten Knall, der von der Stelle kam, wo vorher das Mauerstueck verschwunden
		 * ist." */
		dlg_message(g_dialog, ev->txt[96]);
		event_set_cell(ev, 2, 0x1b, 0x18, 0, 0);
		snd_play(g_snd_fx, 0x7c);
		AUTOMAP_CELL_WRITE_W(g_map, 0x262, 0x1b, 2, 0x4c);
		AUTOMAP_CELL_WRITE_W(g_map, 0x262, 0x1c, 2, 0x3c);
		AUTOMAP_CELL_WRITE_W(g_map, 0x262, 0x1a, 2, 0x3a);
	}
	if (g_dng01_branch_state == 4 && pos == 0x1a02) {
		g_dng01_branch_state = 5;
		/* FINAL.LTX[97]: "Als Ihr an das Mauerstueck herantretet verschwindet dieses mit einem reibenden Geraeusch." */
		dlg_message(g_dialog, ev->txt[97]);
		event_set_cell(ev, 2, 0x1b, 0, 0, 1);
		AUTOMAP_CELL_WRITE_W(g_map, 0x262, 0x1b, 2, 0x39);
		AUTOMAP_CELL_WRITE_W(g_map, 0x262, 0x1c, 2, 0x37);
		AUTOMAP_CELL_WRITE_W(g_map, 0x262, 0x1a, 2, 0x31);
	}
}

void dng01_branches(struct area_events far *ev, register signed short pos)
{
	if ((g_dng01_branch_state == 0 || g_dng01_branch_state == 4) && pos == 0x1e00) {
		g_dng01_branch_state = 1;
		/* FINAL.LTX[93]: "Als Ihr die Wand untersucht und dabei an einem der Aeste zieht hoert Ihr ein seltsames
		 * reibendes Geraeusch in der Wand." */
		dlg_message(g_dialog, ev->txt[93]);
	} else if (g_dng01_branch_state == 1 && pos == 0x1d03) {
		g_dng01_branch_state = 2;
		dlg_message(g_dialog, ev->txt[93]);
	} else if (g_dng01_branch_state == 2 && pos == 0x1f02) {
		g_dng01_branch_state = 3;
		/* FINAL.LTX[95]: "Als Ihr auch an dieser Wand an einen der Aeste zieht hoert Ihr, wie sich die hinter Euch
		 * liegende Wand bewegt.@Als Ihr Euch umschaut ist sie verschwunden." */
		dlg_message(g_dialog, ev->txt[95]);
		event_set_cell(ev, 2, 0x1b, 0, 0, 1);
		AUTOMAP_CELL_WRITE_W(g_map, 0x262, 0x1b, 2, 0x39);
		AUTOMAP_CELL_WRITE_W(g_map, 0x262, 0x1c, 2, 0x37);
		AUTOMAP_CELL_WRITE_W(g_map, 0x262, 0x1a, 2, 0x31);
	} else if (g_group->x >= 0x1c && g_group->y <= 2) {
		g_dng01_branch_state = 0;
		/* FINAL.LTX[94]: "Als Ihr die Wand untersucht bemerkt Ihr einen Ast der sich ein Stueck herausziehen laesst,
		 * aber nichts passiert." */
		dlg_message(g_dialog, ev->txt[94]);
	}
	timewarp_minutes(g_date, 10L);
}

void dng01_skull_ornament(struct area_events far *ev)
{
	struct hero far *h;
	register signed short ans;

	switch (g_dng01_skull_state) {
	case 0:
		h = get_random_hero(g_group);
		if (test_skill(h, TA_SINNESSCHAERFE, 8) <= 0) {
			/* FINAL.LTX[75]: "%s betrachtet das Ornament, kann aber nichts feststellen." */
			sprintf(g_text_output_buf, ev->txt[75], h->name);
			dlg_message(g_dialog, g_text_output_buf);
		} else {
			/* FINAL.LTX[76]: "%s hat das Gefuehl, dass mit dem abgebildeten Totenkopf etwas nicht stimmt, kann aber nicht
			 * genau sagen, was die Ursache dafuer ist." */
			sprintf(g_text_output_buf, ev->txt[76], h->name);
			dlg_message(g_dialog, g_text_output_buf);
			g_dng01_skull_state = 1;
		}
		break;
	case 1:
		h = get_random_hero(g_group);
		if (test_skill(h, TA_SINNESSCHAERFE, 0x0c) <= 0) {
			/* FINAL.LTX[77]: "Selbst bei genauerem Untersuchen kann %s nichts Auffaelliges entdecken." */
			sprintf(g_text_output_buf, ev->txt[77], h->name);
			dlg_message(g_dialog, g_text_output_buf);
		} else {
			/* FINAL.LTX[78]: "%s bemerkt, dass %s der Kopf aus seltsamen leeren und tiefen Augen anstarrt, und als %s mit
			 * der Hand darueberstreicht, stellt %s fest, dass es sich tatsaechlich um tiefe Loecher handelt." */
			sprintf(g_text_output_buf, ev->txt[78], h->name, grammar_pronoun(h->sex, 2, 0),
				grammar_pronoun(h->sex, 0, 0),
				grammar_pronoun(h->sex, 0, 0));
			dlg_message(g_dialog, g_text_output_buf);
			g_dng01_skull_state = 2;
			/* FINAL.LTX[79]: "Wollt Ihr versuchen mit den Fingern in den Augenhoehlen des Ornamentes etwas zu ertasten?" */
			sprintf(g_text_output_buf, ev->txt[79], h->name);
			ans = dlg_yes_no(g_dialog, g_text_output_buf);
			if (ans == 1) {
				/* FINAL.LTX[83]: "Wer soll es versuchen?" */
				h = get_active_hero(g_group, ev->txt[83]);
				if (h != 0) {
					/* FINAL.LTX[80]: "Tatsaechlich fuehlt %s einen kleinen Haken, der sich bewegen laesst. Nach einem kurzen Ruck
					 * hoert Ihr, wie sich in der Decke ueber dem Gitter etwas entriegelt. Aber das Gitter hat sich nicht bewegt
					 * und %s versucht das Gleiche mit dem Kopf gegenueber. Wieder gibt es dieses Geraeusch oberhalb des Gitters
					 * und wieder bewegt sich nichts." */
					sprintf(g_text_output_buf, ev->txt[80], grammar_pronoun(h->sex, 0, 0),
						h->name);
					dlg_message(g_dialog, g_text_output_buf);
					g_dng01_hooks_held = 1;
					g_dng01_hook_hero = h;
				}
			} else
				g_dng01_skull_state = 3;
		}
		break;
	case 2:
		/* FINAL.LTX[81]: "Wollt Ihr nocheinmal an den Haken ziehen?" */
		ans = dlg_yes_no(g_dialog, ev->txt[81]);
		if (ans == 1) {
			h = get_active_hero(g_group, ev->txt[83]);
			if (h != 0) {
				/* FINAL.LTX[82]: "Wieder greift %s mit den Fingern in die Augenhoehlen und wieder gibt es die Geraeusche ueber
				 * dem Gitter, als %s die Haken zurueckzieht." */
				sprintf(g_text_output_buf, ev->txt[82], h->name, grammar_pronoun(h->sex, 0, 0));
				dlg_message(g_dialog, g_text_output_buf);
				g_dng01_hooks_held = 1;
				g_dng01_skull_state = 3;
				g_dng01_hook_hero = h;
			}
		}
		break;
	case 3:
		ans = dlg_yes_no(g_dialog, ev->txt[79]);
		if (ans == 1) {
			h = get_active_hero(g_group, ev->txt[83]);
			if (h != 0) {
				sprintf(g_text_output_buf, ev->txt[80], grammar_pronoun(h->sex, 0, 0), h->name);
				dlg_message(g_dialog, g_text_output_buf);
				g_dng01_hooks_held = 1;
				g_dng01_hook_hero = h;
			}
		} else
			g_dng01_skull_state = 3;
		break;
	}
	timewarp_minutes(g_date, 10L);
}

void dng01_blue_disc(struct area_events far *ev, register signed short pos)
{
	signed short ans;

	/* FINAL.LTX[98]: "Ihr tretet vor eine grosse blaue Scheibe, die an der Wand haengt. Sie hat einen metallischen
	 * blauen Schimmer und scheppert, wenn man dagegen tippt. Wollt Ihr einmal richtig kraeftig dagegen schlagen?" */
	ans = dlg_yes_no(g_dialog, ev->txt[98]);
	if (ans == 1) {
		if (pos == 0x0d00) {
			/* FINAL.LTX[99]: "Es gibt nur ein ganz leises gedaempftes Geraeusch. Wollt Ihr die Scheibe naeher untersuchen?" */
			ans = dlg_yes_no(g_dialog, ev->txt[99]);
			if (ans == 1)
				/* FINAL.LTX[100]: "Trotz gemeinsamer Bemuehungen koennt Ihr nichts auffaelliges feststellen. Die Scheibe sitzt
				 * fest an der Wand, laesst sich nicht bewegen und auch dahinter scheint nichts besonderes zu sein." */
				dlg_message(g_dialog, ev->txt[100]);
			return;
		}
		if (g_dng01_cultists_0806 != 1 && g_dng01_cultists_0e06 != 1 && (pos == 0x0a00 || pos == 0x0a03 || pos == 0x0c03)) {
			snd_play(g_snd_fx, 0x96);
			/* FINAL.LTX[101]: "Es gibt einen ohrenbetaeubenden Laerm, der vermutlich jedes Lebewesen, das sich hier
			 * irgendwo befindet auf Euch aufmerkam macht." */
			dlg_message(g_dialog, ev->txt[101]);
			if (!g_dng01_cultists_0806 && !g_dng01_cultists_0e06) {
				if (event_fight2(ev, 0x64, 4, 0, 0x77, 2, 2) == 0)
					g_dng01_cultists_0806 = g_dng01_cultists_0e06 = 1;
			} else {
				if (event_fight2(ev, 0x64, 2, 0, 0x77, 1, 2) == 0)
					g_dng01_cultists_0806 = g_dng01_cultists_0e06 = 1;
			}
			return;
		}
		snd_play(g_snd_fx, 0x96);
		/* FINAL.LTX[102]: "Es gibt einen ohrenbetaeubenden Laerm, doch nachdem er abgeklungen ist, herrscht wieder
		 * absolute Stille." */
		dlg_message(g_dialog, ev->txt[102]);
	}
}

void dng01_grate_falls(struct area_events far *ev, signed char which)
{
	struct group far *g;
	register signed short i;

	g_dng01_hooks_held = 2;
	event_set_cell(ev, 1, 0x13, 0x16, 0, -1);
	snd_play(g_snd_fx, 0xa5);
	AUTOMAP_CELL_WRITE_W(g_map, 0x262, 0x13, 1, 0x3e);
	if (which == 1) {
		g_dng01_grate_first = 1;
		/* FINAL.LTX[65]: "Ploetzlich hoert Ihr hinter Euch das laute Rasseln einer Kette und als Ihr zurueckschaut
		 * koennt Ihr gerade noch sehen, wie ein schweres Gitter heruntersaust und den Weg versperrt." */
		dlg_message(g_dialog, ev->txt[65]);
		return;
	}
	/* FINAL.LTX[66]: "Ploetzlich hoert Ihr wieder das Rasseln der Kette und prompt saust auch das Gitter wieder
	 * herunter." */
	dlg_message(g_dialog, ev->txt[66]);
	if (g_dng01_hook_hero != 0) {
		for (i = 0, g = g_groups; i < 6; g = &g_groups[++i]) {
			if (group_has_hero(g, g_dng01_hook_hero))
				break;
		}
		if (group_has_hero(g, g_dng01_hook_hero) && g->x == 0x0f && g->y == 2) {
			/* FINAL.LTX[123]: "Bei %s schnappen die Haken zurueck und %s kann gerade noch die Finger aus den Augenhoehlen
			 * ziehen, bevor sie %s zerquetscht werden." */
			sprintf(g_text_output_buf, ev->txt[123], g_dng01_hook_hero->name,
				grammar_pronoun(g_dng01_hook_hero->sex, 0, 0),
				grammar_pronoun(g_dng01_hook_hero->sex, 3, 0));
			dlg_message(g_dialog, g_text_output_buf);
			sub_hero_le(g_dng01_hook_hero, random_schweif(4));
			g_dng01_hook_hero = 0;
		}
	}
}
