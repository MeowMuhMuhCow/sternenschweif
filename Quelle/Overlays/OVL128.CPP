/* OVL128 - Overlay O128 (Segment 0x21f1 in SCHWEIF.EXE): Kampfrunde (wie FIG_do_round in DSA 1 seg032).
 * fight_hero_turn(f): Aktion eines Helden (O079_0000 waehlt, +95--): Aufwachen (Bit +bc.1 ohne +bc.0, W100 < 75, O080_060B,
 * Standbild O135_0262), dann bei Feld/Lebenden: +a8/+a9 als Rundenzaehler (inline-Setter), sonst Schleife
 * Aktionsmenue O154_0000 -> bei Aktion 2/4/5/15 O159_0000 und ggf. Behandlung eines toten Zweifeldgegners
 * (O077_0613/0580, Schwanzfeld O153_0904/05CF, Versatz ds:6510/6512), Wiederholung bei Aktion 4 mit ds:8c03 == 1;
 * Held mit Bit +bd.0: Flucht/Bewegung O146_0500 -> O149_06CB. fight_enemy_turn(f): Aktion eines Gegners (O148_00D1, +30--),
 * Zustandsbits +38/+39 mit Chancen f->1ee + 2/5/10 (hoechstens 50) loeschen, Gegnermenue O152_084F, O161_0000,
 * dieselbe Zweifeld-Behandlung (O192_03B4); bei ds:395f == 1 Sonderzug (Bit +39.2). fight_do_round(f): Runde - Zaehler
 * O080_01A2/O147_06C0, Startseite aus f->1ea (2/1/zufaellig R37_0005), Wechsel per O148_0129, nach jeder Aktion
 * O147_004E, bei ds:65e5 alle 20 Gegner (ds:c4f7, 0x49 Bytes) pruefen (Text f->txt[0x56]), Bildreste +295/+296.
 *
 * BCC -mlarge -3 -Ot -Og -c -Y OVL128.CPP   -> 2886/2886 Bytes GLEICH (18.10.2026), cobjfix laeuft
 *   (Namensfassung mit schweif.h, Welle N7: 19.10.2026 GLEICH, cobjfix LINKGLEICH)
 * - C++ wegen der inline-Setter hero::seta8/seta9 (Parameterkopien [bp-1bh]/[bp-1ch]); -Og-Zwischenwert [bp-1dh]
 *   fuer das Byte +bc bzw. +38, in fight_enemy_turn [bp-1ch] fuer f->f1ee.
 * - fight_enemy_turn: c1/c2 register (SI/DI), c3 im inneren Block ([bp-20h] hinter den Zwischenwerten); als dritte
 *   register-Variable auf Funktionsebene bleibt sie bei [bp-1ch].
 * - "(signed char)(e->f34 < 10 ? (e->f34 == 0 ? 5 : 2) : 1) == 1" -> mov al,5/2/1 + cmp al,1; ohne Cast int-Vergleich.
 *   Ebenso "side = .. ? 1 : (.. ? 0 : R37_0005(0, 1))" auf signed char -> mov al.
 * - Bits: (unsigned char)b -> and ax,1 / or al,al; !(signed char)b -> and/cbw/or ax,ax; b = 0 -> and byte; b = 1 -> or byte.
 * - "if (g_c952) R01_036B();" vielfach (cmp byte), aber "!g_c952" -> mov al/cbw/or.
 * - e != g_c4f7 (far-Zeiger) -> mov eax / cmp eax,mem; Kette "g_395f = g_c4cd = g_65e5 = 0".
 */
#include "schweif.h"

/* Phase B: eigenes BSS des Moduls (ds:c948..cab8, 368 Bytes, Kampf-Globale; vorher Reststueck DRO131).
 * Zuordnung zu O128 vermutet: Besitzer ueber Linkreihenfolge und Zugriffe nicht entscheidbar (Fenster O104..O139),
 * bytegleich bei jedem Kandidaten. Hauptnutzer sind O141, O144, O018 und O142; im Fenster hat O128 die meisten
 * Zugriffe (17), die Kampfrunde ist der natuerliche Ort. ds:cab8 (g_dng03_last_pos) benutzt nur O131 - es bleibt
 * dort als eigenes BSS (O131 liegt in der Linkreihenfolge direkt hinter O128..O130, die kein BSS haben).
 * C++: BSS in Definitionsreihenfolge = Adressreihenfolge. */
signed short g_lock_cell_bits;			/* ds:c948 Feldbits nach dem Schloss (0x2000 = offen, -1 = nichts; O108) */
void far *g_anim_small_buf;			/* ds:c94a Puffer fuer kleine Figurenbilder (0x550 Bytes, O016) */
signed short g_fight_id;			/* ds:c94e Kampfnummer (= Fight.id) */
signed short g_nr_enemies;			/* ds:c950 Zahl der Gegner im Kampf (BE NR_OF_ENEMIES) */
signed char g_quick_fight;			/* ds:c952 Schnellkampf (doppelter Schaden, kein Bild) */
BMP_PTR far *g_fig_obj_bmps;			/* ds:c953 Standbilder der Feldobjekte je Bildnummer (150 Bmp-Zeiger) */
signed char far *g_fig_obj_ids;			/* ds:c957 Figur-IDs der Feldobjekte (150) */
FIGLIST_PTR g_fig_list;				/* ds:c95b Figurenliste des Kampfes (= &Fight.list) */
signed short g_fig_nr_objs;			/* ds:c95f Zahl der Feldobjekte in g_fig_obj_ids */
FIGANIM_PTR far *g_fig_extra_anims;		/* ds:c961 zehn zusaetzliche Animationsobjekte (Treffer-/Zauberbilder) */
struct fig_node far *g_fig_tmpl;		/* ds:c965 Vorlage fuer fig_list_add/-remove (zeigt auf Fight.tmpl) */
signed char g_fig_path[50];			/* ds:c969 Weg der Bewegung: Richtungen 0..3, Ende -1, -2 = zu wenig BP */
unsigned char far *g_fight_walkmap;		/* ds:c99b Begehbarkeitskarte des Kampffelds (25 x 24) */
struct dirty_rect g_fig_dirty[20];		/* ds:c99f neu zu zeichnende Rechtecke (Ende x0 == -1) */
signed char g_fig_nr_dirty;			/* ds:ca67 Zahl der Eintraege in g_fig_dirty */
signed short g_fig_redraw_all;			/* ds:ca68 Kampfbild: alles neu zeichnen */
signed short g_fig_dirty_full;			/* ds:ca6a Vollbild neu (g_fig_dirty enthaelt nur noch den Bildschirm) */
char far *g_fig_text;				/* ds:ca6c Textzeile des Kampfbilds (100 Bytes) */
signed short g_fig_redraw_top;			/* ds:ca70 Anzeige oben (Akteur) neu zeichnen */
signed short g_fig_redraw_bottom;		/* ds:ca72 Anzeige unten (Ziel) neu zeichnen */
signed short g_fig_redraw_text;			/* ds:ca74 Textzeile neu zeichnen */
signed short g_fig_target_x;			/* ds:ca76 Zielfeld der letzten Wegsuche x (fight_path_search) */
signed short g_fig_target_y;			/* ds:ca78 Zielfeld der letzten Wegsuche y */
signed short g_sfight2_x;			/* ds:ca7a Sonderkampf 0x495: Bildlage des Gegners x */
signed short g_sfight2_y;			/* ds:ca7c Sonderkampf 0x495: Bildlage des Gegners y */
signed short g_fig_nr_hits;			/* ds:ca7e Zahl der getroffenen Helden einer Flaechen-/Mehrfachaktion */
struct hero far *g_fig_hit_heroes[7];		/* ds:ca80 getroffene Helden */
FIGANIM_PTR g_fig_hit_anims[7];			/* ds:ca9c Animationen je getroffenem Held */

void fight_hero_turn(Fight far *f)
{
	struct fig_node far *fig;
	signed char act;
	struct hero far *h;
	struct enemy far *e;
	signed short x2;
	signed short y2;
	signed short x3;
	signed short y3;
	signed short x;
	signed short y;
	register signed short done;

	if (g_quick_fight)
		game_idle();
	h = fig_choose_next_hero(g_group);
	h->actions--;
	if ((unsigned char)h->asleep && !(signed char)h->dead && random_schweif(100) < 75) {
		/* aufwachen */
		set_asleep(h, 0);
		if (!g_quick_fight) {
			fig = fig_list_find(&f->list, h->fighter_id);
			fig_stand_pic(f, h, &f->seq);
		}
	}
	if (fight_find_cell(get_hero_index(h) + FIG_ID_HERO, &x, &y) && h->bp_left > 0) {
		if (check_hero3(h) && !(signed char)h->scared && !(signed char)h->asleep) {
			if (h->blind != 0)
				h->set_blind(h->blind - 1);
			else {
				if (h->ecliptifactus != 0)
					h->set_ecliptifactus(h->ecliptifactus - 1);
				do {
					if (g_quick_fight)
						game_idle();
					f->top_hero = h;
					g_fig_redraw_top = 1;
					done = 1;
					fight_find_cell(get_hero_index(h) + FIG_ID_HERO, &x, &y);
					act = fig_menu(f, h, x, y);
					if (act == FIG_ACTION_MELEE_ATTACK || act == FIG_ACTION_SPELL || act == FIG_ACTION_USE_ITEM ||
						act == FIG_ACTION_RANGE_ATTACK) {
						fig_do_hero_action(f, h);
						if (act == FIG_ACTION_SPELL && g_spell_cancelled == 1 && !h->fb5)
							done = 0;
						else if (target_type(h) == TARGET_ENEMY || target_type(h) == TARGET_SPECIAL) {
							e = enemy_mr_target(h);
							if (g_fight_end == 5 && e != g_enemies)
								g_fight_end = 0;
							if ((unsigned char)e->dead) {
								/* toten Gegner hinlegen, Schwanzfeld eines Zweifeldgegners nachziehen */
								fight_find_cell(fight_obj_index(e) + FIG_ID_ENEMY, &x2, &y2);
								e->viewdir = 1;
								fig = fig_list_find(&f->list, e->fighter_id);
								fig->z = 5;
								fig->to_resort = 1;
								if (enemy_is_twofielded(e) && fight_find_cell(fight_obj_index(e) + FIG_ID_ENEMY_TAIL, &x3, &y3)) {
									fig = fig_list_find_twofield(&f->list, fig->twofielded + 20);
									fight_set_cell(f, x3, y3, fig->obj_id);
									if (x2 == x3 && y2 == y3)
										fight_set_cell(f, x2, y2, fight_obj_index(e) + FIG_ID_ENEMY);
									fig->cbx = x2 + g_fig_tail_dxy[1].dx;
									fig->cby = y2 + g_fig_tail_dxy[1].dy;
									fig->z = 5;
									fig->to_resort = 1;
								}
								fig_list_purge(&f->list);
							}
						}
					}
					f->top_hero = 0;
					g_fig_redraw_top = 1;
				} while (!done);
			}
		} else if ((unsigned char)h->scared) {
			/* Flucht */
			if (g_quick_fight)
				game_idle();
			if (check_hero3(h)) {
				if ((unsigned char)h->tied)
					h->scared = 0;
#if FIX_0364
				/* Fix #364: ohne Fluchtweg endet die Angst (sonst kann der Kampf nie enden) */
				else if (fight_path_search(f, h, x, y, 7, 1))
					fig_hero_place(f, h);
				else
					h->scared = 0;
#else
				else if (fight_path_search(f, h, x, y, 7, 1))
					fig_hero_place(f, h);
#endif
			} else
				h->scared = 0;
		}
	}
	if (g_quick_fight)
		game_idle();
}

void fight_enemy_turn(Fight far *f)
{
	struct fig_node far *fig;
	signed char act;
	struct enemy far *e;
	struct enemy far *target;
	signed short x2;
	signed short y2;
	signed short x3;
	signed short y3;
	signed short x;
	signed short y;
	register signed short chance1;
	register signed short chance2;

	if (f->id == FIGHT_ID_SPECIAL) {
		sfight_enemy_action(f);
		return;
	}
	if (f->id == FIGHT_ID_EMPTY)
		return;
	if (g_quick_fight)
		game_idle();
	e = fig_choose_next_enemy(f);
	e->attacks_left--;
	if (fight_find_cell(fight_obj_index(e) + FIG_ID_ENEMY, &x, &y)) {
		signed short chance3;

		/* Zustaende loesen sich mit der Rundenzahl (hoechstens 50 %) */
		chance1 = f->round + 2;
		if (chance1 > 50)
			chance1 = 50;
		chance2 = f->round + 5;
		if (chance2 > 50)
			chance2 = 50;
		chance3 = f->round + 10;
		if (chance3 > 50)
			chance3 = 50;
		if (g_quick_fight)
			game_idle();
		if ((unsigned char)e->asleep && !(signed char)e->dead && random_schweif(100) < chance3)
			e->asleep = 0;
		if ((unsigned char)e->petrified && random_schweif(100) <= chance1)
			e->petrified = 0;
		if ((unsigned char)e->mushroom && random_schweif(100) <= chance1)
			e->mushroom = 0;
		if ((unsigned char)e->tame && random_schweif(100) <= chance2)
			e->tame = 0;
		if ((unsigned char)e->dancing && random_schweif(100) <= chance2)
			e->dancing = 0;
		if ((unsigned char)e->tied && random_schweif(100) <= chance2)
			e->tied = 0;
		if (enemy_is_active(e)) {
			if (e->blind != 0)
				e->blind--;
			else {
				f->top_enemy = e;
				g_fig_redraw_top = 1;
				if ((unsigned char)e->renegade && random_schweif(100) <= chance1)
					e->renegade = 0;
				act = enemy_turn(f, e, x, y);
				if (act == FIG_ACTION_MELEE_ATTACK || act == FIG_ACTION_SPELL || act == FIG_ACTION_USE_ITEM ||
					act == FIG_ACTION_RANGE_ATTACK) {
					if (g_quick_fight)
						game_idle();
					fig_do_enemy_action(f, e);
					if ((signed char)((unsigned char)e->enemy_id < 10 ? ((unsigned char)e->enemy_id == 0 ? 5 : 2) : 1) == 1) {
						/* Ziel ist ein Gegner: toten Gegner hinlegen */
						target = enemy_target_enemy(e);
						if ((unsigned char)target->dead) {
							fight_find_cell(fight_obj_index(target) + FIG_ID_ENEMY, &x2, &y2);
							target->viewdir = 1;
							fig = fig_list_find(&f->list, target->fighter_id);
							fig->z = 5;
							fig->to_resort = 1;
							if (enemy_is_twofielded(target) &&
								fight_find_cell(fight_obj_index(target) + FIG_ID_ENEMY_TAIL, &x3, &y3)) {
								fig = fig_list_find_twofield(&f->list, fig->twofielded + 20);
								fight_set_cell(f, x3, y3, fig->obj_id);
								if (x2 == x3 && y2 == y3)
									fight_set_cell(f, x2, y2, fight_obj_index(target) + FIG_ID_ENEMY);
								fig->cbx = x2 + g_fig_tail_dxy[1].dx;
								fig->cby = y2 + g_fig_tail_dxy[1].dy;
								fig->z = 5;
								fig->to_resort = 1;
							}
							fig_list_purge(&f->list);
						}
					}
					if (g_fight_end == 1 && e->attacks_left == 0) {
						e->scared = 1;
						enemy_turn(f, e, x, y);
						g_game_state = 7;
						g_in_fight = 0;
					}
				}
				f->top_enemy = 0;
			}
		}
	}
	if (g_quick_fight)
		game_idle();
}

void fight_do_round(Fight far *f)
{
	signed short nr_heroes;
	signed short nr_enemies;
	signed short left;
	signed char side;
	struct enemy far *e;
	register signed short i;
	register signed short found;

	if (g_quick_fight)
		game_idle();
	if (!fig_fight_continues(f))
		g_in_fight = 0;
	nr_heroes = fight_round_heroes(g_group);
	nr_enemies = fig_round_enemies(f, g_nr_enemies);
	left = 0;
	side = f->initiative == 2 ? 1 : (f->initiative == 1 ? 0 : random_interval(0, 1));
	f->initiative = 0;
	while (g_in_fight != 0 && nr_heroes + nr_enemies > 0) {
		if (g_quick_fight)
			game_idle();
		if (left == 0)
			fig_next_side(f, &side, &left, nr_heroes, nr_enemies);
		if (side == 0) {
			fight_hero_turn(f);
			if (g_in_fight && !fig_fight_continues(f))
				g_in_fight = 0;
			nr_heroes--;
			left--;
		} else {
			fight_enemy_turn(f);
			if (g_in_fight && !fig_fight_continues(f))
				g_in_fight = 0;
			nr_enemies--;
			left--;
		}
		if (g_fight_flee_all) {
			/* alle handlungsfaehigen Gegner fliehen */
			found = 0;
			for (i = 0; i < NR_ENEMIES; i++) {
				e = g_enemies + i;
				if (enemy_is_active(e)) {
					e->scared = 1;
					found = 1;
				}
			}
			if (found) {
				if (g_quick_fight)
					fight_quick_stop(f);
				dlg_message(g_dialog, f->txt[0x56]);
			}
			g_fight_end = g_in_fight = g_fight_flee_all = 0;
		}
		if (f->mark_id != -1) {
			fig_list_remove(&f->list, f->mark_id, 0);
			f->mark_id = -1;
		}
		if (f->mark_id2 != -1) {
			fig_list_remove(&f->list, f->mark_id2, 0);
			f->mark_id2 = -1;
		}
		if (g_quick_fight)
			game_idle();
	}
}
