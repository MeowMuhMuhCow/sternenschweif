/* OVL137 - Overlay O137 (Segment 0x2219 in SCHWEIF.EXE): Kampf - Ablaufbeschreibungen fuer Helden (Gegenstueck zu
 * O138 fuer Gegner; struct ani_seq, Ausgabe ueber fig_list_find(&f->list, h->fighter_id)->anim und seq_set).
 * fig_hit_pic(f, h, seq): Sturzbild FIG_FRAME_FALL. fig_hero_shoot_seq(f, h, seq): Fernkampf - Drehung zum Ziel
 *   (fight_dir_hero/fig_turn), Bild g_fig_action_frames[hand_weapon_class][Richtung], Start-/Zielfeld (Ziel
 *   enemy_mr_target bei target_type TARGET_ENEMY/TARGET_SPECIAL, Sondergegner -> festes Feld 4/14, sonst
 *   hero_mr_target), bei Abstand > 1 Flugbahn bresenham_line in Bildschirmkoordinaten (g_fig_origin_x/y), jeder 10.
 *   Punkt in einen eigenen Pfad, Geschoss fight_new_extra_anim (0xcc/0xf1/0xf5 + Richtung) als seq->linked
 *   (Ereignis ANI_EV_START_LINKED), Schlussbild nach Waffenart; Rueckgabe Geschoss oder Figur; nicht
 *   einsatzfaehig -> fig_stand_pic. fig_hero_spell_seq: dasselbe fuer Zauber (Geschoss nur bei g_spell_success und
 *   Ziel != TARGET_SELF; Bild g_fig_spell_frames[Hexe ? 0 : spell_id % 2][Richtung], Geschoss 0xef/0xf0).
 * fig_hero_spell_hit_seq(f, h, seq, spell, mode): Zauberwirkung am Helden nach g_spell_ani[spell].kind (mode 2) bzw.
 *   g_mon_spell_ani: 0 Bild g_fig_spell_hit_frames (+ Sturz bei g_fig_target_killed), 3 Feldobjekt 0x5b
 *   (g_fig_obj_bmps, sonst Nvf::load + new Bmp) auf hero.target_x/y mit place_obj_on_cb, 7/8 Beschwoerung
 *   (g_enemies[g_nr_enemies] per fill_enemy_sheet/fig_add_enemy aus g_spell_summon_mon/_renegade, warten
 *   game_idle/seq_done bis f->anim_removed), sonst Effekt 0xd8 (Duplicatus: hero.duplicatus auf 1 bzw. 2).
 * fig_hero_pose_seq/fig_hero_use_seq: Einzelbild g_fig_hero_pose_frames bzw. g_fig_hero_use_frames.
 * fig_hero_throw_seq(f, h, seq, item): Wurf, Geschossbild 0xe7..0xeb nach item 0xee/0xef/0x3b/0x8d, ohne Flugbahn.
 *
 * BCC -mlarge -3 -Ot -Og -Oi -c -Y OVL137.CPP   -> 5775/5775 Bytes GLEICH (18.10.2026), cobjfix laeuft
 *   (Namensfassung mit schweif.h: 19.10.2026 GLEICH, LINKGLEICH)
 * - Aufbau und Stolpersteine wie OVL138 (Block-Locals hinter dem -Og-Zwischenwert, Pfadkopie mit step = 10,
 *   "g_fig_origin_x + (x1 + y1) * 10", Ketten "f->tmpl.cbx = f->tmpl.offsetx = x1", "return res ? .. : node->anim;" + toter Epilog).
 * - Bitfeld enemy.renegade (unsigned short b:1): der inline-Setter set_renegade(signed char) erzeugt dann
 *   mov al,[bp-1fh] / cbw / and ax,1 (mit unsigned char b:1 fehlt das cbw).
 * - Bedingung in fig_hero_spell_hit_seq: "check_hero3(h) && !(signed char)h->asleep || (unsigned char)h->dead".
 * - "bmp = g_fig_obj_bmps[pic] = new Bmp(w, hg)" (Kette, R42_00D8(0, ..)), "g_fig_obj_ids[g_fig_nr_objs++] = fig_list_add(..)".
 * - fill_enemy_sheet und place_obj_on_cb haben char-Parameter (push ax ohne cbw), fig_hero_throw_seq: register-Parameter item (SI).
 * - fig_list_add(&f->list, -1) mit short-Prototyp ergibt dieselben Bytes wie die alte char-Fassung (Konstante).
 */
#include <mem.h>
#include "schweif.h"

#define NVF_HEADS ((Nvf far *)g_nvf_heads)	/* NVF-Dateiobjekt (O098) */

FIGANIM_PTR fig_hit_pic(FIGHT_PTR f, struct hero far *h, struct ani_seq far *seq)
{
	struct fig_node far *node;

	memset(seq, 0, sizeof(struct ani_seq));
	seq->figure = fight_hero_pic(h);
	seq->step[0].frame = FIG_FRAME_FALL;
	seq->step[0].type = 0;
	seq->step[1].frame = ANI_FRAME_END;
	node = fig_list_find(&f->list, h->fighter_id);
	seq_set(node->anim, seq);
	return node->anim;
}

FIGANIM_PTR fig_hero_shoot_seq(FIGHT_PTR f, struct hero far *h, struct ani_seq far *seq)
{
	signed short wc;
	signed short n = 0;
	signed short d;
	signed short to;
	struct fig_node far *node;
	signed short x1;
	signed short y1;
	signed short x2;
	signed short y2;
	signed short dist;
	FIGANIM_PTR res = 0;
	struct enemy far *tgt;

	if (check_hero3(h)) {
		wc = hand_weapon_class(h);
		memset(seq, 0, sizeof(struct ani_seq));
		seq->figure = fight_hero_pic(h);
		d = h->viewdir;
		to = fight_dir_hero(f, h);
		fig_turn(f, h, d, to, &n, seq, 0);
		seq->step[n].frame = g_fig_action_frames[wc][to];
		seq->step[n].type = 0;
		node = fig_list_find(&f->list, h->fighter_id);
		fight_find_cell(get_hero_index(h) + FIG_ID_HERO, &x1, &y1);
		tgt = 0;
		if (target_type(h) == TARGET_ENEMY || target_type(h) == TARGET_SPECIAL)
			tgt = enemy_mr_target(h);
		if (tgt != 0 && enemy_type_check(tgt)) {
			x2 = 4;
			y2 = 14;
		} else if (target_type(h) == TARGET_ENEMY)
			fight_find_cell(fight_obj_index(tgt) + FIG_ID_ENEMY, &x2, &y2);
		else if (target_type(h) == TARGET_SPECIAL)
			fight_find_cell(fight_obj_index(tgt) + FIG_ID_ENEMY_TAIL, &x2, &y2);
		else
			fight_find_cell(get_hero_index(hero_mr_target(h)) + FIG_ID_HERO, &x2, &y2);
		dist = manhattan_dist(x1, y1, x2, y2);
		if (dist > 1) {
			signed short sx;
			signed short ex;
			signed short sy;
			signed short ey;
			signed short cnt;
			signed short steps;
			signed short step;
			struct point far *line;
			struct point far *path;
			register signed short i;
			struct ani_seq shot;

			line = (struct point far *)mem_alloc(0x514);
			sx = g_fig_origin_x + (x1 + y1) * 10;
			sy = g_fig_origin_y + (x1 - y1) * 5;
			ex = g_fig_origin_x + (x2 + y2) * 10;
			ey = g_fig_origin_y + (x2 - y2) * 5;
			cnt = bresenham_line(sx, sy, ex, ey, line) - 1;
			step = 10;
			steps = cnt / step;
			path = (struct point far *)mem_alloc(steps * 4);
			for (i = 1; i <= steps; i++) {
				path[i - 1].x = line[i * step].x;
				path[i - 1].y = line[i * step].y;
			}
			mem_free(line);
			memset(&shot, 0, sizeof(struct ani_seq));
			res = fight_new_extra_anim(f);
			shot.figure = FIG_FIGURE_OBJ;
			shot.path = path;
			shot.step[0].frame = (wc == 4 ? 0xcc : wc == 5 ? 0xf1 : 0xf5) + to;
			shot.step[0].type = 3;
			shot.step[0].event = ANI_EV_END;
			shot.step[0].count = steps;
			f->tmpl.anim = res;
			f->tmpl.cbx = f->tmpl.offsetx = x1;
			f->tmpl.cby = f->tmpl.offsety = y1;
			f->tmpl.kind = FIG_KIND_OBJ;
			f->tmpl.figure = FIG_FIGURE_OBJ;
			f->tmpl.gfx = 0;
			f->tmpl.z = 100;
			f->tmpl.visible = 1;
			f->tmpl.twofielded = FIG_TWOFIELD_NONE;
			fig_list_add(&f->list, -1);
			seq_set(res, &shot);
			res->delay_fix = 0;
			seq->step[n].event = ANI_EV_START_LINKED;
			seq->linked = res;
		}
		n++;
		if (wc == 4)
			seq->step[n].frame = g_fig_hero_stand_wc4[to];
		else if (wc == 6)
			seq->step[n].frame = g_fig_hero_stand_wc6[to];
		else
			seq->step[n].frame = g_fig_stand_frames[0][to];
		seq->step[n].type = 0;
		seq->step[n].count = 1;
		n++;
		seq->step[n].frame = ANI_FRAME_END;
		seq_set(node->anim, seq);
		return res ? res : node->anim;
	} else
		return fig_stand_pic(f, h, seq);
}

FIGANIM_PTR fig_hero_spell_seq(FIGHT_PTR f, struct hero far *h, struct ani_seq far *seq)
{
	signed short n = 0;
	signed short row;
	signed short wc;
	signed short d;
	signed short to;
	struct fig_node far *node;
	signed short x1;
	signed short y1;
	signed short x2;
	signed short y2;
	signed short dist;
	FIGANIM_PTR res = 0;
	struct enemy far *tgt;

	if (check_hero3(h)) {
		memset(seq, 0, sizeof(struct ani_seq));
		seq->figure = fight_hero_pic(h);
		d = h->viewdir;
		to = fight_dir_hero(f, h);
		fig_turn(f, h, d, to, &n, seq, 0);
		row = h->type == HERO_TYPE_WITCH ? 0 : h->spell_id % 2;
		seq->step[n].frame = g_fig_spell_frames[row][to];
		seq->step[n].type = 0;
		node = fig_list_find(&f->list, h->fighter_id);
		if (g_spell_success != 0 && target_type(h) != TARGET_SELF) {
			fight_find_cell(get_hero_index(h) + FIG_ID_HERO, &x1, &y1);
			tgt = 0;
			if (target_type(h) == TARGET_ENEMY || target_type(h) == TARGET_SPECIAL)
				tgt = enemy_mr_target(h);
			if (tgt != 0 && enemy_type_check(tgt)) {
				x2 = 4;
				y2 = 14;
			} else if (target_type(h) == TARGET_ENEMY)
				fight_find_cell(fight_obj_index(tgt) + FIG_ID_ENEMY, &x2, &y2);
			else if (target_type(h) == TARGET_SPECIAL)
				fight_find_cell(fight_obj_index(tgt) + FIG_ID_ENEMY_TAIL, &x2, &y2);
			else
				fight_find_cell(get_hero_index(hero_mr_target(h)) + FIG_ID_HERO, &x2, &y2);
			dist = manhattan_dist(x1, y1, x2, y2);
			if (dist > 1) {
				signed short sx;
				signed short ex;
				signed short sy;
				signed short ey;
				signed short cnt;
				signed short steps;
				signed short step;
				struct point far *line;
				struct point far *path;
				register signed short i;
				struct ani_seq shot;

				line = (struct point far *)mem_alloc(0x514);
				sx = g_fig_origin_x + (x1 + y1) * 10;
				sy = g_fig_origin_y + (x1 - y1) * 5;
				ex = g_fig_origin_x + (x2 + y2) * 10;
				ey = g_fig_origin_y + (x2 - y2) * 5;
				cnt = bresenham_line(sx, sy, ex, ey, line) - 1;
				step = 10;
				steps = cnt / step;
				path = (struct point far *)mem_alloc(steps * 4);
				for (i = 1; i <= steps; i++) {
					path[i - 1].x = line[i * step].x;
					path[i - 1].y = line[i * step].y;
				}
				mem_free(line);
				memset(&shot, 0, sizeof(struct ani_seq));
				res = fight_new_extra_anim(f);
				shot.figure = FIG_FIGURE_OBJ;
				shot.path = path;
				shot.step[0].frame = row == 0 ? 0xef : 0xf0;
				shot.step[0].type = 3;
				shot.step[0].event = ANI_EV_END;
				shot.step[0].count = steps;
				f->tmpl.anim = res;
				f->tmpl.cbx = f->tmpl.offsetx = x1;
				f->tmpl.cby = f->tmpl.offsety = y1;
				f->tmpl.kind = FIG_KIND_OBJ;
				f->tmpl.figure = FIG_FIGURE_OBJ;
				f->tmpl.gfx = 0;
				f->tmpl.z = 100;
				f->tmpl.visible = 1;
				f->tmpl.twofielded = FIG_TWOFIELD_NONE;
				fig_list_add(&f->list, -1);
				seq_set(res, &shot);
				res->delay_fix = 0;
				seq->step[n].event = ANI_EV_START_LINKED;
				seq->linked = res;
			}
		}
		n++;
		wc = hand_weapon_class(h);
		if (wc == 4)
			seq->step[n].frame = g_fig_hero_stand_wc4[to];
		else if (wc == 6)
			seq->step[n].frame = g_fig_hero_stand_wc6[to];
		else
			seq->step[n].frame = g_fig_stand_frames[0][to];
		seq->step[n].type = 0;
		seq->step[n].count = 1;
		n++;
		seq->step[n].frame = ANI_FRAME_END;
		seq_set(node->anim, seq);
		return res ? res : node->anim;
	} else
		return fig_stand_pic(f, h, seq);
}

FIGANIM_PTR fig_hero_spell_hit_seq(FIGHT_PTR f, struct hero far *h, struct ani_seq far *seq, signed short spell,
	signed short mode)
{
	struct fig_node far *node;
	FIGANIM_PTR res = 0;
	BMP_PTR bmp;
	signed short fx;
	signed short fy;
	signed short pic;
	signed short w;
	signed short hg;
	signed short n = 0;
	signed char kind;
	struct enemy far *e;
	struct ani_seq eff;

	if (check_hero3(h) && !(signed char)h->asleep || (unsigned char)h->dead) {
		if (mode == 2)
			kind = g_spell_ani[spell].kind;
		else
			kind = g_mon_spell_ani[spell].kind;
		node = fig_list_find(&f->list, h->fighter_id);
		if (kind == 0) {
			memset(seq, 0, sizeof(struct ani_seq));
			seq->figure = fight_hero_pic(h);
			seq->step[n].frame = g_fig_spell_hit_frames[h->viewdir];
			seq->step[n].type = 0;
			n++;
			if (g_fig_target_killed != 0) {
				seq->step[n].frame = FIG_FRAME_FALL;
				seq->step[n].type = 0;
				n++;
			}
			seq->step[n].frame = ANI_FRAME_END;
			seq_set(node->anim, seq);
			return node->anim;
		} else if (kind == 3) {
			pic = 0x5b;
			fx = h->target_x;
			fy = h->target_y;
			if (g_fig_obj_bmps[pic]) {
				bmp = g_fig_obj_bmps[pic];
				bmp->size(&w, &hg);
			} else {
				NVF_HEADS->load(0x47, f->buf, 0, pic, 0, &w, &hg, 0x140);
				bmp = g_fig_obj_bmps[pic] = new Bmp(w, hg);
				bmp->load(f->buf);
			}
			f->tmpl.obj_pic = pic;
			f->tmpl.cbx = fx;
			f->tmpl.cby = fy;
			f->tmpl.offsetx = g_fig_obj_dxy[pic].dx;
			f->tmpl.offsety = g_fig_obj_dxy[pic].dy;
			f->tmpl.kind = FIG_KIND_OBJ;
			f->tmpl.anim = 0;
			f->tmpl.gfx = bmp;
			f->tmpl.z = 50;
			f->tmpl.visible = 1;
			f->tmpl.twofielded = FIG_TWOFIELD_NONE;
			g_fig_obj_ids[g_fig_nr_objs++] = fig_list_add(&f->list, -1);
			place_obj_on_cb(f, fx, fy, pic + FIG_CELL_OBJ, pic, 0);
			return fig_stand_pic(f, h, seq);
		} else if (kind == 7 || kind == 8) {
			e = g_enemies + g_nr_enemies;
			fx = h->target_x;
			fy = h->target_y;
			fill_enemy_sheet(f, g_nr_enemies, g_spell_summon_mon, 0);
			fig_add_enemy(f, e, fx, fy);
			e->set_renegade(g_spell_summon_renegade);
			e->set_viewdir(h->viewdir);
			place_obj_on_cb(f, fx, fy, g_nr_enemies + FIG_ID_ENEMY, e->gfx_id, e->viewdir);
			f->enemy_le_sum += e->le;
			f->enemy_ae_sum += e->ae;
			fig_stand_seq(f, e, seq);
			node = fig_list_find(&f->list, e->fighter_id);
			fig_list_hide(&f->list, node->id);
			g_nr_enemies++;
			memset(&eff, 0, sizeof(struct ani_seq));
			res = fight_new_extra_anim(f);
			eff.figure = FIG_FIGURE_OBJ;
			eff.step[0].frame = g_spell_summon_mon == 0x50 ? (random_schweif(100) < 50 ? 0xed : 0xee) : 0xec;
			eff.step[0].type = 0;
			eff.step[0].event = ANI_EV_END;
			f->tmpl.anim = res;
			f->tmpl.cbx = f->tmpl.offsetx = node->cbx;
			f->tmpl.cby = f->tmpl.offsety = node->cby;
			f->tmpl.kind = FIG_KIND_OBJ;
			f->tmpl.figure = FIG_FIGURE_OBJ;
			f->tmpl.gfx = 0;
			f->tmpl.z = 100;
			f->tmpl.visible = 1;
			f->tmpl.twofielded = FIG_TWOFIELD_NONE;
			fig_list_add(&f->list, -1);
			seq_set(res, &eff);
			res->started = 1;
			f->anim_removed = 0;
			do
				game_idle();
			while (!f->anim_removed && !seq_done(res));
			fig_list_show(&f->list, node->id);
			return node->anim;
		} else {
			if (g_cast_spell_id[0] == SP_DUPLICATUS) {
				if (h->duplicatus != 0)
					h->duplicatus = 2;
				else
					h->duplicatus = 1;
			}
			memset(&eff, 0, sizeof(struct ani_seq));
			res = fight_new_extra_anim(f);
			eff.figure = FIG_FIGURE_OBJ;
			eff.step[0].frame = 0xd8;
			eff.step[0].type = 0;
			eff.step[0].event = ANI_EV_END;
			f->tmpl.anim = res;
			f->tmpl.cbx = f->tmpl.offsetx = node->cbx;
			f->tmpl.cby = f->tmpl.offsety = node->cby;
			f->tmpl.kind = FIG_KIND_OBJ;
			f->tmpl.figure = FIG_FIGURE_OBJ;
			f->tmpl.gfx = 0;
			f->tmpl.z = 100;
			f->tmpl.visible = 1;
			f->tmpl.twofielded = FIG_TWOFIELD_NONE;
			fig_list_add(&f->list, -1);
			seq_set(res, &eff);
			res->started = 1;
			return res;
		}
	} else
		return fig_stand_pic(f, h, seq);
}

FIGANIM_PTR fig_hero_pose_seq(FIGHT_PTR f, struct hero far *h, struct ani_seq far *seq)
{
	struct fig_node far *node;

	memset(seq, 0, sizeof(struct ani_seq));
	seq->figure = fight_hero_pic(h);
	seq->step[0].frame = g_fig_hero_pose_frames[h->viewdir];
	seq->step[0].type = 0;
	seq->step[1].frame = ANI_FRAME_END;
	node = fig_list_find(&f->list, h->fighter_id);
	seq_set(node->anim, seq);
	return node->anim;
}

FIGANIM_PTR fig_hero_use_seq(FIGHT_PTR f, struct hero far *h, struct ani_seq far *seq)
{
	struct fig_node far *node;

	if (check_hero3(h)) {
		memset(seq, 0, sizeof(struct ani_seq));
		seq->figure = fight_hero_pic(h);
		seq->step[0].frame = g_fig_hero_use_frames[h->viewdir];
		seq->step[0].type = 0;
		seq->step[1].frame = ANI_FRAME_END;
		node = fig_list_find(&f->list, h->fighter_id);
		seq_set(node->anim, seq);
		return node->anim;
	} else
		return fig_stand_pic(f, h, seq);
}

FIGANIM_PTR fig_hero_throw_seq(FIGHT_PTR f, struct hero far *h, struct ani_seq far *seq, register signed short item)
{
	struct fig_node far *node;
	signed short n = 0;
	FIGANIM_PTR res = 0;
	signed short d;
	signed short to;
	signed short x;
	signed short y;
	struct enemy far *tgt;
	struct ani_seq shot;

	if (check_hero3(h)) {
		memset(seq, 0, sizeof(struct ani_seq));
		seq->figure = fight_hero_pic(h);
		d = h->viewdir;
		to = fight_dir_hero(f, h);
		fig_turn(f, h, d, to, &n, seq, 0);
		seq->step[n].frame = g_fig_hero_throw_frames[h->viewdir];
		seq->step[n].type = 0;
		node = fig_list_find(&f->list, h->fighter_id);
		if (item != 0) {
			tgt = 0;
			if (target_type(h) == TARGET_ENEMY || target_type(h) == TARGET_SPECIAL)
				tgt = enemy_mr_target(h);
			if (target_type(h) == TARGET_ENEMY)
				fight_find_cell(fight_obj_index(tgt) + FIG_ID_ENEMY, &x, &y);
			else if (target_type(h) == TARGET_SPECIAL)
				fight_find_cell(fight_obj_index(tgt) + FIG_ID_ENEMY_TAIL, &x, &y);
			else
				fight_find_cell(get_hero_index(hero_mr_target(h)) + FIG_ID_HERO, &x, &y);
			memset(&shot, 0, sizeof(struct ani_seq));
			res = fight_new_extra_anim(f);
			shot.figure = FIG_FIGURE_OBJ;
			shot.step[0].frame = item == 0xee ? 0xe7 : item == 0xef ? 0xe9 : item == 0x3b ? 0xe8 : item == 0x8d ? 0xea : 0xeb;
			shot.step[0].type = 0;
			shot.step[0].event = ANI_EV_END;
			shot.step[0].count = 1;
			f->tmpl.anim = res;
			f->tmpl.cbx = f->tmpl.offsetx = x;
			f->tmpl.cby = f->tmpl.offsety = y;
			f->tmpl.kind = FIG_KIND_OBJ;
			f->tmpl.figure = FIG_FIGURE_OBJ;
			f->tmpl.gfx = 0;
			f->tmpl.z = 100;
			f->tmpl.visible = 1;
			f->tmpl.twofielded = FIG_TWOFIELD_NONE;
			fig_list_add(&f->list, -1);
			seq_set(res, &shot);
			res->delay_fix = 0;
			seq->step[n].event = ANI_EV_START_LINKED;
			seq->linked = res;
		}
		n++;
		seq->step[n].frame = ANI_FRAME_END;
		seq_set(node->anim, seq);
		return res ? res : node->anim;
	} else
		return fig_stand_pic(f, h, seq);
}
