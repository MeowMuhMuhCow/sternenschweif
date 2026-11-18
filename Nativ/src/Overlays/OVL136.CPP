/* OVL136 - Overlay O136 (Segment 0x2215 in SCHWEIF.EXE): Ablaufbeschreibungen (struct ani_seq) fuer Gegner (Gegenstueck
 * zu O135). fig_enemy_turn(f, e, &dir, to, &idx, seq, no_stand): Drehung von *dir nach to (fig_turn_steps, Richtungen
 *   * 2, Ueberlauf 0..7, Bilder 1 + 2 * Richtung), am Ende Ereignis ANI_EV_VIEWDIR + to im letzten Schritt; im
 *   Schnellkampf (g_quick_fight) nur viewdir und *dir setzen; gleiche Richtung -> ein Standbild (count 4, type 3).
 * fig_stand_seq(f, e, seq): Standbild (Sondergegner enemy_type_check -> sfight_enemy_reset; Art 0x34 nichts; Pilz
 *   FIG_FIGURE_OBJ/0xd9; sonst fig_enemy_pic und Bild 9 tot, g_fig_stone_frames versteinert, g_fig_stand_frames).
 * fig_enemy_attack_seq(f, e, seq): Drehung nach fight_dir_enemy, g_fig_attack_frames (doppelt bei f->atk_twice,
 *   Sturzbild bei f->atk_hit); Sondergegner sfight_seq_action(cur_spell); sonst fig_stand_seq.
 * fig_enemy_defend_seq(f, e, seq, dir): Drehung nach dir_opposite(f, dir), Paradebild g_fig_parry_frames (nur wenn
 *   e->parried noch 0, dann = 1), Gegenschlag bei f->def_strikes, Sturz bei g_fig_target_killed; leer -> fig_stand_seq.
 *
 * BCC -mlarge -3 -Ot -Og -Oi -c -Y OVL136.CPP   -> 2198/2198 Bytes GLEICH (18.10.2026), cobjfix laeuft
 *   (Namensfassung mit schweif.h: 19.10.2026 GLEICH, LINKGLEICH; Schritte ab +9 als Teilsicht struct ani_seq9)
 * - C++ wegen der Bitfelder (+0x38: "(signed char)e->mushroom" -> or al,al) und des inline-Setters
 *   enemy::set_viewdir(signed char) (Parameterkopie [bp-2] im Schnellmodus-Zweig von fig_enemy_turn).
 * - -Oi noetig: memset(seq, 0, 0xa2) als Intrinsic (xor ax,ax / mov ah,al / rep stosw).
 * - -Og noetig: [bp-4] = (short)d, [bp-5] = d + 1, [bp-9] = Byte +0x38 und SI = (short)d2 in fig_enemy_attack_seq sind
 *   -Og-Zwischenwerte; Epilogkopien und tote Epiloge hinter "return r" bzw. am Funktionsende.
 * - "n *= 2" ergibt mov dx,2 / imul dx (n = n * 2 dagegen shl ax,1).
 * - Eintraege: "e[(*idx)++].type = 0" (inc direkt nach dem Laden), in fig_enemy_attack_seq/fig_enemy_defend_seq dagegen "step[idx].type = 0; idx++;".
 */
#include <mem.h>
#include "schweif.h"

#define SEQ9 ((struct ani_seq9 far *)seq)	/* Schritte ab +9: e[n].frame = step[n].frame, e[n].prev_event = step[n - 1].event */

void fig_enemy_turn(FIGHT_PTR f, struct enemy far *e, signed char far *dirp, signed short dir,
	signed short far *idx, void far *seq, register signed short no_stand)
{
	signed char d;
	register signed short steps;

	d = *dirp;
	if (g_quick_fight) {
		e->set_viewdir(dir);
		*dirp = dir;
		return;
	}
	if (d != dir) {
		steps = fig_turn_steps(f, d, dir);
		d = d * 2;
		steps *= 2;
		if (steps == -2) {
			if (!no_stand) {
				SEQ9->e[*idx].frame = d + 1;
				SEQ9->e[(*idx)++].type = 0;
				SEQ9->e[*idx].frame = d + 1;
				SEQ9->e[(*idx)++].type = 0;
			}
			if ((d = d - 1) < 0)
				d = 7;
			SEQ9->e[*idx].frame = d + 1;
			SEQ9->e[(*idx)++].type = 0;
			SEQ9->e[*idx].frame = dir * 2 + 1;
			SEQ9->e[(*idx)++].type = 0;
		} else {
			if (steps == 2 && !no_stand) {
				SEQ9->e[*idx].frame = d + 1;
				SEQ9->e[(*idx)++].type = 0;
				SEQ9->e[*idx].frame = d + 1;
				SEQ9->e[(*idx)++].type = 0;
			}
			do {
				if ((d = d + 1) > 7)
					d = 0;
				SEQ9->e[*idx].frame = d + 1;
				SEQ9->e[(*idx)++].type = 0;
			} while (--steps > 0);
		}
		*dirp = dir;
		SEQ9->e[*idx].prev_event = dir + ANI_EV_VIEWDIR;
	} else {
		if (!no_stand) {
			d = d * 2;
			SEQ9->e[*idx].frame = d + 1;
			SEQ9->e[*idx].count = 4;
			SEQ9->e[(*idx)++].type = 3;
		}
	}
}

FIGANIM_PTR fig_stand_seq(FIGHT_PTR f, struct enemy far *e, struct ani_seq far *seq)
{
	struct fig_node far *node;
	FIGANIM_PTR anim;

	if (enemy_type_check(e))
		anim = sfight_enemy_reset(f, &f->seq);
	else if (e->mon_id != 0x34) {
		memset(seq, 0, sizeof(struct ani_seq));
		if ((signed char)e->mushroom) {
			seq->figure = FIG_FIGURE_OBJ;
			seq->step[0].frame = 0xd9;
		} else {
			seq->figure = fig_enemy_pic(e);
			if ((signed char)e->dead)
				seq->step[0].frame = 9;
			else if ((signed char)e->petrified)
				seq->step[0].frame = g_fig_stone_frames[e->viewdir];
			else
				seq->step[0].frame = g_fig_stand_frames[0][e->viewdir];
		}
		seq->step[0].type = 4;
		seq->step[0].count = 0xff;
		seq->step[1].frame = ANI_FRAME_END;
		node = fig_list_find(&f->list, e->fighter_id);
		seq_set(node->anim, seq);
		anim = node->anim;
	}
	return anim;
}

FIGANIM_PTR fig_enemy_attack_seq(FIGHT_PTR f, struct enemy far *e, struct ani_seq far *seq)
{
	signed short idx;
	signed char d;
	signed char to;
	struct fig_node far *node;
	FIGANIM_PTR anim;

	idx = 0;
	if (enemy_is_active(e) || (signed char)e->dead) {
		if (enemy_type_check(e))
			anim = sfight_seq_action(f, &f->seq, e->cur_spell);
		else if (e->mon_id != 0x34) {
			memset(seq, 0, sizeof(struct ani_seq));
			seq->figure = fig_enemy_pic(e);
			d = e->viewdir;
			to = fight_dir_enemy(f, e);
			fig_enemy_turn(f, e, &d, to, &idx, seq, 0);
			seq->step[idx].frame = g_fig_attack_frames[to];
			seq->step[idx].type = 0;
			idx++;
			if (f->atk_twice) {
				seq->step[idx].frame = g_fig_attack_frames[to];
				seq->step[idx].type = 0;
				idx++;
			}
			if (f->atk_hit) {
				if (d != 1)
					fig_enemy_turn(f, e, &d, 1, &idx, seq, 0);
				seq->step[idx].frame = FIG_FRAME_FALL;
				seq->step[idx].type = 0;
				idx++;
			}
			seq->step[idx].frame = ANI_FRAME_END;
			node = fig_list_find(&f->list, e->fighter_id);
			seq_set(node->anim, seq);
			return node->anim;
		}
		return anim;
	} else
		return fig_stand_seq(f, e, seq);
}

FIGANIM_PTR fig_enemy_defend_seq(FIGHT_PTR f, struct enemy far *e, struct ani_seq far *seq, signed short dir)
{
	signed short idx;
	signed char d;
	signed char to;
	struct fig_node far *node;
	FIGANIM_PTR anim;
	register signed short no_parry;

	idx = 0;
	no_parry = 0;
	if (enemy_is_active(e) || (signed char)e->dead) {
		if (enemy_type_check(e))
			anim = sfight_seq_walk_random(f, seq);
		else {
			memset(seq, 0, sizeof(struct ani_seq));
			seq->figure = fig_enemy_pic(e);
			d = e->viewdir;
			to = dir_opposite(f, dir);
			if (!e->parried) {
				e->parried = 1;
				fig_enemy_turn(f, e, &d, to, &idx, seq, 0);
				seq->step[idx].frame = g_fig_parry_frames[to];
				seq->step[idx].type = 0;
				idx++;
			} else
				no_parry = 1;
			if (f->def_strikes) {
				no_parry = 0;
				if (d != to)
					fig_enemy_turn(f, e, &d, to, &idx, seq, 0);
				seq->step[idx].frame = g_fig_attack_frames[to];
				seq->step[idx].type = 0;
				idx++;
			}
			if (g_fig_target_killed) {
				if (no_parry) {
					seq->step[idx].frame = g_fig_stand_frames[0][d];
					seq->step[idx].count = 6;
					seq->step[idx].type = 3;
					idx++;
				}
				if (d != 1)
					fig_enemy_turn(f, e, &d, 1, &idx, seq, 0);
				seq->step[idx].frame = FIG_FRAME_FALL;
				seq->step[idx].type = 0;
				idx++;
			}
			seq->step[idx].frame = ANI_FRAME_END;
			if (idx) {
				node = fig_list_find(&f->list, e->fighter_id);
				seq_set(node->anim, seq);
				return node->anim;
			} else
				return fig_stand_seq(f, e, seq);
		}
		return anim;
	} else
		return fig_stand_seq(f, e, seq);
}
