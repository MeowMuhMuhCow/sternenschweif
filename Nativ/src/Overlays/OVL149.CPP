/* OVL149 - Overlay O149 (Segment 0x2255 in SCHWEIF.EXE): Kampf - Bewegung einer Figur entlang des Pfads g_fig_path
 * (Richtungen, Ende < 0; zwei gleiche hintereinander = Doppelschritt). fig_enemy_move(f, e): Gegner e (Figur
 * fighter_id, Richtung viewdir, BP bp, Flucht scared), auch Zweifeldgegner (Schwanz ueber
 * fig_list_find_twofield(twofielded + 20)). Richtungstabelle g_fig_dirs_enemy als lokale Strukturkopie,
 * Ablaufbeschreibung f->seq (max. 38 Schritte): Drehungen ueber fig_enemy_turn, Schrittbilder g_fig_step2_frames
 * (doppelt) bzw. g_fig_step_frames, BP -2/-1 (nicht unter 0). Ohne Schnellkampf (g_quick_fight): Markierung
 * f->mark_id entfernen, Folge abspielen (seq_set/seq_done, Abbruch ueber fig_list_exists bei scared), dann
 * fig_set_mark/fig_stand_seq/fight_draw. Ziel ueber g_fight_walkmap[y * 25 + x]: fight_cell_blocked(x, y) -> Figur
 * verlaesst das Feld (g_fight_end 5 -> 0, g_fig_enemies_escaped++, bp = 0, alte Felder per fight_set_cell
 * zuruecksetzen), sonst Feldwerte fight_obj_index(e) + FIG_ID_ENEMY / + FIG_ID_ENEMY_TAIL eintragen und Knoten
 * verschieben (cbx/cby, to_resort = 1). Am Ende fig_list_purge, nach Verlassen g_fig_redraw_all = 1 und fight_draw.
 * fig_hero_place(f, h): dasselbe fuer einen Helden (bp_left, Flucht hero.scared, g_fig_dirs_hero, fig_turn/
 *   fig_stand_pic, fight_hero_pic, Feldwert get_hero_index(h) + FIG_ID_HERO), ohne Folgenlaengengrenze, einteilig.
 *
 * BCC -mlarge -3 -Ot -Oi -Og -c -Y -E..\bc\BIN\TASM.EXE OVL149.CPP   -> 2953/2953 Bytes GLEICH (18.10.2026),
 *   cobjfix laeuft (_g_6c16/_g_6c26 -> _g_6be6+0x30/+0x40). Ohne -Oi oder ohne -Og falsche Laenge.
 *   Namensfassung mit schweif.h: 19.10.2026 GLEICH, LINKGLEICH.
 * - .CPP wie O135/O136; Bits als "unsigned short b:1", "(unsigned char)e->scared" -> shr ax,2 / and ax,1 / or al,al.
 * - "struct dirs4 d = g_fig_dirs_enemy;" (8 Worte) -> mov si / lea di / push ss / pop es / rep movsw am Funktionsanfang;
 *   d.d[dir].dx / .dy -> -Og-Zwischenwert dir * 4 ([bp-28h] bzw. [bp-18h]), .y ueber lea [bp-38h].
 * - BP-Abzug "e->bp += (signed char)-2" -> add byte 0feh (mit "+= -2" mov/add/mov und der then-Zweig wird
 *   zu lang fuer jnz short); "--" -> dec byte; Grenze "if (..) x = 0; else ;" -> toter jmp short.
 * - fig_hero_place: h->viewdir in Bedingung und Argument -> -Og-Zwischenwert [bp-15h] (kein eigener Local).
 * - "x0 = x = p->cbx" speichert erst x, dann x0; "stop = !fig_list_exists(..)" -> neg / sbb / inc.
 * - Felder als *(g_fight_walkmap + y * 25 + x) (les / add bx,ax / add bx,x).
 * - Originalfehler: fig_hero_place setzt beim Verlassen h->bp_left = 0 und prueft danach "if (h->bp_left > 12) ..."
 *   (wirkungslose Begrenzung); fig_enemy_move begrenzt die Folge auf 38 Eintraege (idx < 0x26), fig_hero_place nicht
 *   (Pfad mit Drehungen > 38 Eintraege schreibt ueber f->seq hinaus, Ende -1 ebenso).
 */
#include <mem.h>
#include "schweif.h"

/* Phase B: eigene Daten des Moduls (ds:6c16..6c36, dsana) - initialisierte Daten in Adressreihenfolge */
struct dirs4 g_fig_dirs_enemy = { { { 1, 0 }, { 0, -1 }, { -1, 0 }, { 0, 1 } } };	/* Kampffeld-Richtung 0: x+1, 1: y-1, 2: x-1, 3: y+1 */
struct dirs4 g_fig_dirs_hero = { { { 1, 0 }, { 0, -1 }, { -1, 0 }, { 0, 1 } } };	/* Kampffeld-Richtung 0: x+1, 1: y-1, 2: x-1, 3: y+1 */

void fig_enemy_move(FIGHT_PTR f, struct enemy far *e)
{
	signed short i;
	struct fig_node far *p;
	struct fig_node far *p2;
	signed short x;
	signed short y;
	signed short x0;
	signed short y0;
	signed short x2;
	signed short y2;
	signed short x2_0;
	signed short y2_0;
	unsigned char cell;
	unsigned char old_cell;
	unsigned char cell2;
	unsigned char old_cell2;
	signed short idx;
	signed short stop;
	signed char dir;
	signed short moved;
	struct dirs4 d = g_fig_dirs_enemy;

	idx = 0;
	moved = 0;
	if (g_quick_fight != 0)
		game_idle();
	if (g_fig_path[0] < 0)
		return;
	p = fig_list_find(&f->list, e->fighter_id);
	x0 = x = p->cbx;
	y0 = y = p->cby;
	old_cell = p->obj_id;
	if (p->twofielded != FIG_TWOFIELD_NONE) {
		p2 = fig_list_find_twofield(&f->list, p->twofielded + 20);
		old_cell2 = p2->obj_id;
		x2_0 = p2->cbx;
		y2_0 = p2->cby;
	} else
		p2 = 0;
	memset(&f->seq, 0, sizeof(struct ani_seq));
	f->seq.figure = fig_enemy_pic(e);
	i = 0;
	dir = e->viewdir;
	while (g_fig_path[i] >= 0 && idx < 0x26) {
		if (g_quick_fight != 0)
			game_idle();
		if (g_fig_path[i] != dir)
			fig_enemy_turn(f, e, &dir, g_fig_path[i], &idx, &f->seq, 1);
		if (g_fig_path[i] == g_fig_path[i + 1]) {
			f->seq.step[idx].frame = g_fig_step2_frames[g_fig_path[i]];
			f->seq.step[idx].type = 0;
			idx++;
			i += 2;
			e->bp += (signed char)-2;
			if (e->bp < 0)
				e->bp = 0;
			else
				;
			x += d.d[dir].dx * 2;
			y += d.d[dir].dy * 2;
		} else {
			f->seq.step[idx].frame = g_fig_step_frames[g_fig_path[i]];
			f->seq.step[idx].type = 0;
			idx++;
			i++;
			e->bp--;
			if (e->bp < 0)
				e->bp = 0;
			else
				;
			x += d.d[dir].dx;
			y += d.d[dir].dy;
		}
	}
	f->seq.step[idx].frame = -1;
	if (!g_quick_fight) {
		if (f->mark_id != -1) {
			fig_list_remove(&f->list, f->mark_id, 0);
			f->mark_id = -1;
		}
		seq_set(p->anim, &f->seq);
		stop = 0;
		do {
			game_idle();
			if ((unsigned char)e->scared)
				stop = !fig_list_exists(&f->list, e->fighter_id);
			if (stop == 0)
				p = fig_list_find(&f->list, e->fighter_id);
		} while (stop == 0 && p->anim != 0 && seq_done(p->anim) == 0);
		if (stop == 0) {
			fig_set_mark(f, e->fighter_id);
			fig_stand_seq(f, e, &f->seq);
			fight_draw(f);
		}
	} else
		game_idle();
	cell = *(g_fight_walkmap + y * 25 + x);
	if (p2 != 0) {
		x2 = x + g_fig_tail_dxy[e->viewdir].dx;
		y2 = y + g_fig_tail_dxy[e->viewdir].dy;
		cell2 = *(g_fight_walkmap + y2 * 25 + x2);
	}
	if (fight_cell_blocked(x, y)) {
		if (g_fight_end == 5)
			g_fight_end = 0;
		g_fig_enemies_escaped++;
		e->bp = 0;
		fight_set_cell(f, x0, y0, old_cell);
		if (g_quick_fight != 0) {
			p = fig_list_find(&f->list, e->fighter_id);
			p->to_remove = 1;
			if (p2 != 0) {
				p2 = fig_list_find_twofield(&f->list, p->twofielded + 20);
				p2->to_remove = 1;
			}
		}
		if (p2 != 0)
			fight_set_cell(f, x2_0, y2_0, old_cell2);
		moved = 1;
	} else {
		p = fig_list_find(&f->list, e->fighter_id);
		fight_set_cell(f, x0, y0, old_cell);
		fight_set_cell(f, x, y, fight_obj_index(e) + FIG_ID_ENEMY);
		if (p2 != 0) {
			if (cell != fight_obj_index(e) + FIG_ID_ENEMY_TAIL && cell != fight_obj_index(e) + FIG_ID_ENEMY)
				p->obj_id = cell;
		} else {
			if (cell != fight_obj_index(e) + FIG_ID_ENEMY)
				p->obj_id = cell;
		}
		p->cbx = x;
		p->cby = y;
		p->to_resort = 1;
		if (p2 != 0) {
			p2 = fig_list_find_twofield(&f->list, p->twofielded + 20);
			fight_set_cell(f, x2_0, y2_0, old_cell2);
			fight_set_cell(f, x2, y2, fight_obj_index(e) + FIG_ID_ENEMY_TAIL);
			if (cell2 != fight_obj_index(e) + FIG_ID_ENEMY && cell2 != fight_obj_index(e) + FIG_ID_ENEMY_TAIL)
				p2->obj_id = cell2;
			p2->cbx = x2;
			p2->cby = y2;
			p2->to_resort = 1;
		}
	}
	if (!g_quick_fight) {
		fig_list_purge(&f->list);
		if (moved) {
			g_fig_redraw_all = 1;
			fight_draw(f);
		}
	} else
		game_idle();
}

void fig_hero_place(FIGHT_PTR f, struct hero far *h)
{
	signed short i;
	struct fig_node far *p;
	signed short x;
	signed short y;
	signed short x0;
	signed short y0;
	unsigned char cell;
	unsigned char old_cell;
	signed short idx;
	signed short stop;
	struct dirs4 d = g_fig_dirs_hero;

	idx = 0;
	if (g_quick_fight != 0)
		game_idle();
	if (g_fig_path[0] < 0)
		return;
	p = fig_list_find(&f->list, h->fighter_id);
	x0 = x = p->cbx;
	y0 = y = p->cby;
	old_cell = p->obj_id;
	memset(&f->seq, 0, sizeof(struct ani_seq));
	f->seq.figure = fight_hero_pic(h);
#if FIX_0029
	/* Fix #29: Laenge der Animationsfolge begrenzen wie in fig_enemy_move */
	i = 0;
	while (g_fig_path[i] >= 0 && idx < 0x26) {
		if (g_quick_fight != 0)
			game_idle();
		if (h->viewdir != g_fig_path[i])
#else
	i = 0;
	while (g_fig_path[i] >= 0) {
		if (g_quick_fight != 0)
			game_idle();
		if (h->viewdir != g_fig_path[i])
#endif
			fig_turn(f, h, h->viewdir, g_fig_path[i], &idx, &f->seq, 1);
		if (g_fig_path[i] == g_fig_path[i + 1]) {
			f->seq.step[idx].frame = g_fig_step2_frames[g_fig_path[i]];
			f->seq.step[idx].type = 0;
			idx++;
			i += 2;
			h->bp_left += (signed char)-2;
			if (h->bp_left < 0)
				h->bp_left = 0;
			else
				;
			x += d.d[h->viewdir].dx * 2;
			y += d.d[h->viewdir].dy * 2;
		} else {
			f->seq.step[idx].frame = g_fig_step_frames[g_fig_path[i]];
			f->seq.step[idx].type = 0;
			idx++;
			i++;
			h->bp_left--;
			if (h->bp_left < 0)
				h->bp_left = 0;
			else
				;
			x += d.d[h->viewdir].dx;
			y += d.d[h->viewdir].dy;
		}
	}
	f->seq.step[idx].frame = -1;
	if (!g_quick_fight) {
		if (f->mark_id != -1) {
			fig_list_remove(&f->list, f->mark_id, 0);
			f->mark_id = -1;
		}
		seq_set(p->anim, &f->seq);
		stop = 0;
		do {
			game_idle();
			if ((unsigned char)h->scared)
				stop = !fig_list_exists(&f->list, h->fighter_id);
			if (stop == 0)
				p = fig_list_find(&f->list, h->fighter_id);
		} while (stop == 0 && p->anim != 0 && seq_done(p->anim) == 0);
		if (stop == 0) {
			fig_set_mark(f, h->fighter_id);
			fig_stand_pic(f, h, &f->seq);
			fight_draw(f);
		}
	} else
		game_idle();
	cell = *(g_fight_walkmap + y * 25 + x);
	if (fight_cell_blocked(x, y)) {
		if (g_fight_end == 5)
			g_fight_end = 0;
		fight_set_cell(f, x0, y0, old_cell);
		h->bp_left = 0;
		if (h->bp_left > 12)
			h->bp_left = 12;
		else
			;
		if (g_quick_fight != 0) {
			p = fig_list_find(&f->list, h->fighter_id);
			p->to_remove = 1;
		}
	} else {
		p = fig_list_find(&f->list, h->fighter_id);
		fight_set_cell(f, x0, y0, old_cell);
		fight_set_cell(f, x, y, get_hero_index(h) + FIG_ID_HERO);
		if (cell != get_hero_index(h) + FIG_ID_HERO)
			p->obj_id = cell;
		p->cbx = x;
		p->cby = y;
		p->to_resort = 1;
	}
	if (!g_quick_fight)
		fig_list_purge(&f->list);
	else
		game_idle();
}
