/* OVL135 - Overlay O135 (Segment 0x2211 in SCHWEIF.EXE): Kampf - Ablaufbeschreibungen (struct ani_seq, 0xa2 Bytes:
 * figure = fight_hero_pic(h), Schritte {frame, type, count, event}, Ende ANI_FRAME_END) fuer Helden; Ausgabe an das
 * Animationsobjekt der Figur fig_list_find(&f->list, h->fighter_id)->anim per seq_set.
 * fig_turn(f, h, from, to, &idx, seq, no_stand): Drehung von Blickrichtung from nach to (fig_turn_steps = Schritte
 *   +-1/2, Bilder base + 2 * Richtung, base 1/0xb3/0xbb je nach hand_weapon_class 4/6), danach viewdir = to; bei
 *   from == to nur ein Standbild (Art 3, Wert 4).
 * fig_stand_pic(f, h, seq): Standbild je nach Zustand (tot 9, schlafend/bewusstlos g_fig_hero_sleep_frames,
 *   versteinert g_fig_stone_frames, sonst g_fig_hero_stand_* nach Waffenart).
 * fig_hero_attack_seq(f, h, seq): Angriff - Drehung zum Ziel (fight_dir_hero), Bild g_fig_hero_attack_frames
 *   [Waffenart][Richtung] bzw. g_fig_attack_frames (hand_weapon_class_melee, Gegenstand 0x85 = waffenlos), ggf.
 *   zweimal (f->atk_twice), Sturzbild bei f->atk_hit; nicht einsatzfaehig -> fig_stand_pic.
 * fig_hero_defend_seq(f, h, seq, dir): Drehung nach dir_opposite(f, dir), Paradebild g_fig_action_frames bzw.
 *   g_fig_parry_frames (nur beim ersten Mal je Runde, hero.parried), Gegenschlag bei f->def_strikes, bei
 *   g_fig_target_killed Sturz (Drehung zu 1, FIG_FRAME_FALL) bzw. bewusstlos (viewdir + 0xaf); leer -> fig_stand_pic.
 *
 * BCC -mlarge -3 -Ot -Og -Oi -c -Y OVL135.CPP   -> 2495/2495 Bytes GLEICH (18.10.2026), cobjfix laeuft
 *   (Namensfassung mit schweif.h: 19.10.2026 GLEICH, LINKGLEICH)
 * - C++ wegen inline-Setter hero::set_viewdir(signed char) (Parameterkopie [bp-5] in fig_turn); -Oi fuer memset(a, 0, 0xa2).
 * - -Og: [bp-6] in fig_turn ist der Zwischenwert (char)(base + von) (zweimal hintereinander gespeichert),
 *   [bp-5]/[bp-0dh] in fig_stand_pic/fig_hero_defend_seq die Kopie von Byte +bc (Zustandsbits).
 * - "if ((from = from - 1) < 0)" / "if ((from = from + 1) > 7)" -> mov ax/dec/mov/or bzw. cmp (mit --from:
 *   dec word [..] / jnl).
 * - seq->step[(*idx)++].type = 0 ueber den Zeiger (inc word es:[bx]), lokal dagegen "seq->step[idx].type = 0; idx++;".
 * - fig_turn: "if (von != nach) { ..; return; }" - Epilogkopie vor dem kurzen Zweig.
 * - Bitabfragen: (unsigned char)h->dead -> and ax,1 / or al,al, !(signed char)h->asleep -> cbw / or ax,ax.
 * - fig_hero_defend_seq: register row in den beiden inneren Bloecken (DI); auf Funktionsebene kommt row wegen memset
 *   (rep stosw mit DI) auf den Stack. fig_list_find hat einen char-Parameter (push ax ohne cbw).
 * - "if (..) { ..; return x; } else return fig_stand_pic(..);" -> Epilogkopien + tote Epiloge wie im Original.
 */
#include <mem.h>
#include "schweif.h"

void fig_turn(FIGHT_PTR f, struct hero far *h, signed short from, signed short to, signed short far *idx,
	struct ani_seq far *seq, register signed short no_stand)
{
	signed short wc;
	signed short base;
	register signed short steps;

	base = 1;
	wc = hand_weapon_class(h);
	if (wc == 4)
		base = 0xb3;
	else if (wc == 6)
		base = 0xbb;
	if (from != to) {
		steps = fig_turn_steps(f, from, to);
		from *= 2;
		steps *= 2;
		if (steps == -2) {
			if (!no_stand) {
				seq->step[*idx].frame = base + from;
				seq->step[(*idx)++].type = 0;
				seq->step[*idx].frame = base + from;
				seq->step[(*idx)++].type = 0;
			}
			if ((from = from - 1) < 0)
				from = 7;
			seq->step[*idx].frame = base + from;
			seq->step[(*idx)++].type = 0;
			seq->step[*idx].frame = base + to * 2;
			seq->step[(*idx)++].type = 0;
		} else {
			if (steps == 2 && !no_stand) {
				seq->step[*idx].frame = base + from;
				seq->step[(*idx)++].type = 0;
				seq->step[*idx].frame = base + from;
				seq->step[(*idx)++].type = 0;
			}
			do {
				if ((from = from + 1) > 7)
					from = 0;
				seq->step[*idx].frame = base + from;
				seq->step[(*idx)++].type = 0;
			} while (--steps > 0);
		}
		h->set_viewdir(to);
		return;
	}
	if (!no_stand) {
		from *= 2;
		seq->step[*idx].frame = base + from;
		seq->step[*idx].count = 4;
		seq->step[(*idx)++].type = 3;
	}
}

FIGANIM_PTR fig_stand_pic(FIGHT_PTR f, struct hero far *h, struct ani_seq far *seq)
{
	struct fig_node far *node;
	register signed short wc;

	memset(seq, 0, sizeof(struct ani_seq));
	seq->figure = fight_hero_pic(h);
	if ((unsigned char)h->dead) {
		seq->step[0].frame = 9;
	} else if ((unsigned char)h->asleep || (unsigned char)h->unconscious) {
		seq->step[0].frame = g_fig_hero_sleep_frames[h->viewdir];
	} else if ((unsigned char)h->petrified) {
		seq->step[0].frame = g_fig_stone_frames[h->viewdir];
	} else {
		wc = hand_weapon_class(h);
		if (wc == 4)
			seq->step[0].frame = g_fig_hero_stand_wc4[h->viewdir];
		else if (wc == 6)
			seq->step[0].frame = g_fig_hero_stand_wc6[h->viewdir];
		else
			seq->step[0].frame = g_fig_stand_frames[0][h->viewdir];
	}
	seq->step[0].type = 4;
	seq->step[0].count = -1;
	seq->step[1].frame = ANI_FRAME_END;
	node = fig_list_find(&f->list, h->fighter_id);
	seq_set(node->anim, seq);
	return node->anim;
}

FIGANIM_PTR fig_hero_attack_seq(FIGHT_PTR f, struct hero far *h, struct ani_seq far *seq)
{
	signed short wc;
	signed short idx;
	signed short from;
	struct fig_node far *node;
	register signed short to;

	idx = 0;
	if (check_hero3(h) || (unsigned char)h->dead) {
		wc = hand_weapon_class_melee(h);
		if (h->inv[INV_SLOT_RIGHT_HAND].item_id == 0x85)	/* Gegenstand 0x85: wie waffenlos */
			wc = -1;
		memset(seq, 0, sizeof(struct ani_seq));
		seq->figure = fight_hero_pic(h);
		from = h->viewdir;
		to = fight_dir_hero(f, h);
		fig_turn(f, h, from, to, &idx, seq, 0);
		if (wc != -1)
			seq->step[idx].frame = g_fig_hero_attack_frames[wc][to];
		else
			seq->step[idx].frame = g_fig_attack_frames[to];
		seq->step[idx].type = 0;
		idx++;
		if (f->atk_twice) {
			if (wc != -1)
				seq->step[idx].frame = g_fig_hero_attack_frames[wc][to];
			else
				seq->step[idx].frame = g_fig_attack_frames[to];
			seq->step[idx].type = 0;
			idx++;
		}
		if (f->atk_hit) {
			seq->step[idx].frame = FIG_FRAME_FALL;
			seq->step[idx].type = 0;
			idx++;
		}
		seq->step[idx].frame = ANI_FRAME_END;
		node = fig_list_find(&f->list, h->fighter_id);
		seq_set(node->anim, seq);
		return node->anim;
	} else
		return fig_stand_pic(f, h, seq);
}

FIGANIM_PTR fig_hero_defend_seq(FIGHT_PTR f, struct hero far *h, struct ani_seq far *seq, signed short dir)
{
	signed short idx;
	signed short from;
	signed short to;
	struct fig_node far *node;
	signed short no_parry;
	register signed short wc;

	idx = 0;
	no_parry = 0;
	if ((check_hero3(h) && !(signed char)h->asleep) || (unsigned char)h->dead || (unsigned char)h->unconscious) {
		wc = hand_weapon_class_melee(h);
		if (h->inv[INV_SLOT_RIGHT_HAND].item_id == 0x85)
			wc = -1;
		memset(seq, 0, sizeof(struct ani_seq));
		seq->figure = fight_hero_pic(h);
		from = h->viewdir;
		to = dir_opposite(f, dir);
		if (!h->parried) {
			h->parried = 1;
			fig_turn(f, h, from, to, &idx, seq, 0);
			if (wc != -1)
				seq->step[idx].frame = g_fig_action_frames[wc][to];
			else
				seq->step[idx].frame = g_fig_parry_frames[to];
			seq->step[idx].type = 0;
			idx++;
		} else
			no_parry = 1;
		if (f->def_strikes) {
			no_parry = 0;
			from = h->viewdir;
			if (from != to)
				fig_turn(f, h, from, to, &idx, seq, 0);
			if (wc != -1)
				seq->step[idx].frame = g_fig_hero_attack_frames[wc][to];
			else
				seq->step[idx].frame = g_fig_attack_frames[to];
			seq->step[idx].type = 0;
			idx++;
		}
		if (g_fig_target_killed) {
			if (no_parry) {
				register signed short row;

				row = 0;
				wc = hand_weapon_class(h);
				if (wc == 4)
					row = 1;
				seq->step[idx].frame = g_fig_stand_frames[row][h->viewdir];
				seq->step[idx].count = 8;
				seq->step[idx].type = 3;
				idx++;
			}
			if (h->viewdir != 1)
				fig_turn(f, h, h->viewdir, 1, &idx, seq, 0);
			seq->step[idx].frame = FIG_FRAME_FALL;
			seq->step[idx].type = 0;
			idx++;
		} else if ((unsigned char)h->unconscious) {
			if (no_parry) {
				register signed short row;

				row = 0;
				wc = hand_weapon_class(h);
				if (wc == 4)
					row = 1;
				seq->step[idx].frame = g_fig_stand_frames[row][h->viewdir];
				seq->step[idx].count = 8;
				seq->step[idx].type = 3;
				idx++;
			}
			seq->step[idx].frame = h->viewdir + 0xaf;	/* bewusstlos je Richtung */
			seq->step[idx].type = 0;
			idx++;
		}
		seq->step[idx].frame = ANI_FRAME_END;
		if (idx != 0) {
			node = fig_list_find(&f->list, h->fighter_id);
			seq_set(node->anim, seq);
			return node->anim;
		} else
			return fig_stand_pic(f, h, seq);
	} else
		return fig_stand_pic(f, h, seq);
}
