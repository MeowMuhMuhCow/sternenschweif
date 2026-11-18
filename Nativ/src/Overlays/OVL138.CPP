/* OVL138 - Overlay O138 (Segment 0x221e in SCHWEIF.EXE): Ablaufbeschreibungen fuer Gegner: Treffer, Fernangriffe,
 * Zauber (Gegenstueck zu O137; struct ani_seq mit Pfad, Partner seq->linked und Schritten ab +0x0a).
 * fig_enemy_hit_seq(f, e, seq): Drehung zu Richtung 1 (fig_enemy_turn) + FIG_FRAME_FALL.
 * fig_enemy_shoot_seq(f, e, seq): Fernkampf (enemy_is_active, sonst fig_stand_seq): Drehung zum Ziel
 *   (fight_dir_enemy), Bild g_fig_action_frames[enemy_range_kind][Richtung], Start-/Zielfeld per fight_find_cell,
 *   bei Abstand > 1 (manhattan_dist) Flugbahn bresenham_line in Bildschirmkoordinaten (g_fig_origin_x/y), jeder
 *   10. Punkt in einen neuen Pfad (mem_alloc), Geschoss fight_new_extra_anim mit eigener Folge (FIG_FIGURE_OBJ,
 *   0xcc/0xf1/0xf5 + Richtung), Rueckgabe Geschoss oder Figur. fig_enemy_spell_seq: dasselbe fuer Ziel
 *   enemy_target_enemy (nur bei g_spell_success und Ziel != e; Bild g_fig_spell_frames, Geschoss 0xef/0xf0 nach
 *   e->cur_spell ungerade/gerade).
 * fig_enemy_spell_hit_seq(f, e, seq, spell, mode): Sondergegner -> sfight_seq_spell; Wirkungsbild aus g_spell_ani
 *   (mode 2) bzw. g_mon_spell_ani: 0/2 Bild g_fig_spell_hit_frames (+ Drehung/Sturz bei g_fig_target_killed), sonst
 *   Effekt 0xd8 (Art 5: Verwandlung - Figur entfernen, fight_set_cell/fill_enemy_sheet(Art 0x10)/fig_add_enemy,
 *   renegade = 1, viewdir/attacks_left wiederherstellen).
 *
 * BCC -mlarge -3 -Ot -Og -Oi -c -Y OVL138.CPP   -> 3814/3814 Bytes GLEICH (18.10.2026), cobjfix laeuft
 *   (Namensfassung mit schweif.h: 19.10.2026 GLEICH, LINKGLEICH)
 * - -Oi fuer memset(.., 0, 0xa2) (rep stosw); C++ wegen der Bitfelder (+0x38/+0x39 mit (signed char)-Cast -> or al,al) und
 *   der inline-Setter enemy::set_viewdir/set_attacks_left (Kopien [bp-14h]/[bp-15h]).
 * - Stack: Locals des "if (dist > 1)"-Blocks (sx, ex, sy, ey, cnt, steps, step, buf, buf2, loc) liegen hinter
 *   dem -Og-Zwischenwert [bp-1ah] (Offset von &buf[i * step]); "g_fig_origin_x + (x1 + y1) * 10" (Produkt zuerst,
 *   mov dx,[g] / add dx,ax) - "(..) * 10 + g_6544" ergibt add ax,[g].
 * - Zielart als Makro ENEMY_TARGET_TYPE(e) "(signed char)((unsigned char)enemy_id < 10 ? (.. == 0 ? 5 : 2) : 1)" -> mov al,imm / cmp al,1,
 *   je Vergleich neu ausgewertet.
 * - fig_enemy_spell_hit_seq: "if (..) return ..; else if (..) {..} else return ..;" - das else nach dem ersten return erzeugt
 *   den toten Epilog dahinter; d (Block im g_fig_target_killed-Zweig) und obj_id/viewdir (Block Art 5) teilen sich [bp-12h].
 * - "f->tmpl.cbx = f->tmpl.offsetx = node->cbx" (Feld +0) ergibt les bx,a / push es / les si,p / mov al,es:[si] / pop es.
 * - Pfadkopie wortweise "path[i - 1].x = line[i * step].x; ...y" (step = 10 als Variable -> imul word [bp-..]).
 * Originalfehler: ENEMY_TARGET_TYPE() liefert nur 1, 2 oder 5 - der Zweig TARGET_SPECIAL (Zielfeld + 30) in
 *   fig_enemy_shoot_seq/fig_enemy_spell_seq
 *   ist tot. fig_enemy_spell_seq holt das Ziel fuer die Flugbahn erneut ueber
 *   enemy_target_enemy(e) statt tgt zu benutzen.
 */
#include <mem.h>
#include "schweif.h"

/* Zielart des Gegners aus enemy_id (wie target_type beim Helden), je Vergleich neu ausgewertet */
#define ENEMY_TARGET_TYPE(e) ((signed char)((unsigned char)(e)->enemy_id < TARGET_ID_ENEMY ? \
	((unsigned char)(e)->enemy_id == 0 ? TARGET_SELF : TARGET_HERO) : TARGET_ENEMY))

FIGANIM_PTR fig_enemy_hit_seq(FIGHT_PTR f, struct enemy far *e, struct ani_seq far *seq)
{
	struct fig_node far *node;
	signed short n = 0;
	signed char d;
	signed char to;

	memset(seq, 0, sizeof(struct ani_seq));
	seq->figure = fig_enemy_pic(e);
	d = e->viewdir;
	to = 1;
	fig_enemy_turn(f, e, &d, to, &n, seq, 0);
	seq->step[n].frame = FIG_FRAME_FALL;
	seq->step[n].type = 0;
	n++;
	seq->step[n].frame = ANI_FRAME_END;
	node = fig_list_find(&f->list, e->fighter_id);
	seq_set(node->anim, seq);
	return node->anim;
}

FIGANIM_PTR fig_enemy_shoot_seq(FIGHT_PTR f, struct enemy far *e, struct ani_seq far *seq)
{
	signed short kind;
	signed short n = 0;
	signed char d;
	signed char to;
	struct fig_node far *node;
	signed short x1;
	signed short y1;
	signed short x2;
	signed short y2;
	signed short dist;
	FIGANIM_PTR res = 0;

	if (enemy_is_active(e)) {
		kind = enemy_range_kind(e);
		memset(seq, 0, sizeof(struct ani_seq));
		seq->figure = fig_enemy_pic(e);
		d = e->viewdir;
		to = fight_dir_enemy(f, e);
		fig_enemy_turn(f, e, &d, to, &n, seq, 0);
		seq->step[n].frame = g_fig_action_frames[kind][to];
		seq->step[n].type = 0;
		node = fig_list_find(&f->list, e->fighter_id);
		fight_find_cell(fight_obj_index(e) + FIG_ID_ENEMY, &x1, &y1);
		if (ENEMY_TARGET_TYPE(e) == TARGET_ENEMY)
			fight_find_cell(fight_obj_index(enemy_target_enemy(e)) + FIG_ID_ENEMY, &x2, &y2);
		else if (ENEMY_TARGET_TYPE(e) == TARGET_SPECIAL)
			fight_find_cell(fight_obj_index(enemy_target_enemy(e)) + FIG_ID_ENEMY_TAIL, &x2, &y2);
		else
			fight_find_cell(get_hero_index(enemy_target_hero(e)) + FIG_ID_HERO, &x2, &y2);
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
			shot.step[0].frame = (kind == 4 ? 0xcc : kind == 5 ? 0xf1 : 0xf5) + to;
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
		seq->step[n].frame = g_fig_stand_frames[0][to];
		seq->step[n].type = 0;
		seq->step[n].count = 1;
		n++;
		seq->step[n].frame = ANI_FRAME_END;
		seq_set(node->anim, seq);
		return res ? res : node->anim;
	} else
		return fig_stand_seq(f, e, seq);
}

FIGANIM_PTR fig_enemy_spell_seq(FIGHT_PTR f, struct enemy far *e, struct ani_seq far *seq)
{
	signed short n = 0;
	signed char d;
	signed char to;
	struct fig_node far *node;
	signed short x1;
	signed short y1;
	signed short x2;
	signed short y2;
	signed short dist;
	FIGANIM_PTR res = 0;
	struct enemy far *tgt;

	if (enemy_is_active(e)) {
		memset(seq, 0, sizeof(struct ani_seq));
		seq->figure = fig_enemy_pic(e);
		tgt = enemy_target_enemy(e);
		d = e->viewdir;
		if (e != tgt) {
			to = fight_dir_enemy(f, e);
			fig_enemy_turn(f, e, &d, to, &n, seq, 0);
		} else
			to = d;
		seq->step[n].frame = g_fig_spell_frames[0][to];
		seq->step[n].type = 0;
		node = fig_list_find(&f->list, e->fighter_id);
		if (g_spell_success != 0 && tgt != e) {
			fight_find_cell(fight_obj_index(e) + FIG_ID_ENEMY, &x1, &y1);
			if (ENEMY_TARGET_TYPE(e) == TARGET_ENEMY)
				fight_find_cell(fight_obj_index(enemy_target_enemy(e)) + FIG_ID_ENEMY, &x2, &y2);
			else if (ENEMY_TARGET_TYPE(e) == TARGET_SPECIAL)
				fight_find_cell(fight_obj_index(enemy_target_enemy(e)) + FIG_ID_ENEMY_TAIL, &x2, &y2);
			else
				fight_find_cell(get_hero_index(enemy_target_hero(e)) + FIG_ID_HERO, &x2, &y2);
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
				shot.step[0].frame = e->cur_spell % 2 ? 0xef : 0xf0;
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
		seq->step[n].frame = g_fig_stand_frames[0][to];
		seq->step[n].type = 0;
		seq->step[n].count = 1;
		n++;
		seq->step[n].frame = ANI_FRAME_END;
		seq_set(node->anim, seq);
		return res ? res : node->anim;
	} else
		return fig_stand_seq(f, e, seq);
}

FIGANIM_PTR fig_enemy_spell_hit_seq(FIGHT_PTR f, struct enemy far *e, struct ani_seq far *seq, signed short spell,
	signed short mode)
{
	struct fig_node far *node;
	FIGANIM_PTR res = 0;
	signed short n = 0;
	signed char kind;
	signed char attacks_left;
	signed short fx;
	signed short fy;
	struct ani_seq eff;

	if (enemy_type_check(e))
		return sfight_seq_spell(f, seq, spell);
	else if (enemy_is_active(e) || (signed char)e->dead || (signed char)e->petrified || (signed char)e->tame ||
		(signed char)e->asleep) {
		if (mode == 2)
			kind = g_spell_ani[spell].kind;
		else
			kind = g_mon_spell_ani[spell].kind;
		node = fig_list_find(&f->list, e->fighter_id);
		if (kind == 0 || kind == 2) {
			memset(seq, 0, sizeof(struct ani_seq));
			seq->figure = fig_enemy_pic(e);
			seq->step[n].frame = g_fig_spell_hit_frames[e->viewdir];
			seq->step[n].type = 0;
			n++;
			if (g_fig_target_killed != 0) {
				signed char d;
				d = e->viewdir;
				if (d != 1)
					fig_enemy_turn(f, e, &d, 1, &n, seq, 0);
				seq->step[n].frame = FIG_FRAME_FALL;
				seq->step[n].type = 0;
				n++;
			}
			seq->step[n].frame = ANI_FRAME_END;
			seq_set(node->anim, seq);
			return node->anim;
		} else {
			if (kind == 5) {
				signed char obj_id;
				signed char viewdir;
				fx = node->cbx;
				fy = node->cby;
				obj_id = node->obj_id;
				viewdir = e->viewdir;
				if (node->twofielded != FIG_TWOFIELD_NONE)
					fig_list_remove(&f->list, fig_list_find_twofield(&f->list, node->twofielded + 20)->id, 0);
				fig_list_remove(&f->list, e->fighter_id, 0);
				attacks_left = e->attacks_left;
				fight_set_cell(f, fx, fy, obj_id);
				fill_enemy_sheet(f, fight_obj_index(e), 0x10, 0);
				fig_add_enemy(f, e, fx, fy);
				e->set_viewdir(viewdir);
				fight_set_cell(f, fx, fy, fight_obj_index(e) + FIG_ID_ENEMY);
				e->renegade = 1;
				e->set_attacks_left(attacks_left);
				node = fig_list_find(&f->list, e->fighter_id);
				fig_stand_seq(f, e, seq);
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
		return fig_stand_seq(f, e, seq);
}
