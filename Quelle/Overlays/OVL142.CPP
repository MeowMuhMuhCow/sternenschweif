/* OVL142 - Overlay O142 (Segment 0x2231 in SCHWEIF.EXE): Ablaufbeschreibungen der Sonderfigur im Sonderkampf 0x493
 * (O141; struct ani_seq, figure FIG_FIGURE_SFIGHT, Ausgabe ueber fig_list_find(&f->list, g_sfight_fig*)->anim und
 * seq_set; vier Teile g_sfight_figs[0..3], Lage im Bildraster g_sfight_pos (13 Spalten), Bildlagen
 * g_sfight_ofs_x/y bzw. g_sfight_ofs2_x/y).
 * sfight_enemy_reset(f, seq): Grundzustand - Teile je nach g_sfight_phase ein-/ausblenden, drei Standbilder (Bild 1,
 *   g_sfight_pos, 9..13 nach Spalte g_sfight_pos % 13).
 * sfight_enemy_move(f, seq, target): Weg von g_sfight_pos nach target (+-13 Zeile, +-1 Spalte, gesperrte Lagen
 *   g_sfight_blocked per is_in_word_array), je Schritt ein Eintrag (type = count = 3), Schlussbild 9..13 an Teil 2,
 *   per set_next verkettet; Rueckgabe Animation von Teil 1. sfight_enemy_end_seq: Bild 0x4f, Lage 0x74/0.
 * sfight_seq_action(f, seq, mode): Zug des Sondergegners (g_enemies[0], Feld fight_find_cell): mode 0 Bild 0x0e;
 *   mode 3 nach Feld (x << 8) + y Ziellage und bis zu vier Felder, Helden darauf (g_fight_walkmap, Werte 1..10,
 *   check_hero2) in g_fig_hit_heroes/g_fig_nr_hits, Weg (Warteschleife game_idle bis seq_done), Bild 0x72..0x76;
 *   sonst Weg nach 0x2d und Bild 0x5a/0x5b + 0x5c.
 * sfight_seq_walk_random: zufaellige freie Lage (random_interval(4, 0x4d)) -> sfight_enemy_move.
 * sfight_seq_area_attack(f, seq): Flaechenangriff - Zielspalte/-zeile bzw. Diagonale nach Helden absuchen, Ziel
 *   anlaufen, Angriffsbild, je Held ein Effekt fight_new_extra_anim (Bild 0x6c, Kette ueber set_next, erstes nach
 *   seq->linked); Rueckgabe letzter Effekt. sfight_seq_special: Bild 0x50..0x54 nach Weg zu 0x2d.
 * sfight_seq_spell(f, seq, spell): Folge aus g_sfight_spell_seq1 (27 Bytes) bzw. _seq2 (25 Bytes, Ende -1) nach
 *   (spell + 1) % 2, Bild 2/3. sfight_special_picture(f): fig_stand_seq/fight_draw, Bild 0x372 (Nvf::load) per
 *   XMS-/EMS-Auftrag g_xms_move (EMS_copy bzw. xms_move_any nach g_fight_use_ems) einblenden, sieben Ausschnitte
 *   g_sfight_pic_size (Bmp init/scale/put/free_lines) an g_sfight_pic_pos, g_fig_redraw_all = 1.
 *
 * BCC -mlarge -3 -Ot -Oi -c -Y OVL142.CPP   -> 7353/7353 Bytes GLEICH (18.10.2026), cobjfix laeuft
 *   (Namensfassung mit schweif.h: 19.10.2026 GLEICH, LINKGLEICH)
 * - -Oi fuer memset(.., 0, 0xa2) (rep stosw); -Og wird nicht gebraucht (mit -Og 22 Bytes zu lang).
 * - C++ wegen abs() (inline, je Aufruf eine Parameterkopie [bp-14h]..[bp-1ah] in sfight_enemy_move) und des inline-Setters
 *   fig_anim::set_next(struct fig_anim far *) (Parameterkopie per mov eax / mov [bp-x],eax; [bp-1eh] in sfight_enemy_move, [bp-1ah] in
 *   sfight_seq_spell, [bp-42h] in sfight_seq_area_attack).
 * - Far-Zeiger-Store ueber doppelte Dereferenzierung (les bx,o / les bx,es:[bx+0dh]) wird zu zwei Wort-Stores
 *   (Segment zuerst, auch "node->anim->next = 0"); ueber eine lokale Zeigervariable (prev->set_next(q)) mov dword.
 * - "seq->step[n - 1].event = ANI_EV_START_LINKED" faltet -1 in die Verschiebung ([bx+9]); "seq->figure = tmp.figure = FIG_FIGURE_SFIGHT" (Kette).
 * - sfight_special_picture: register x/y im Schleifenblock - auf Funktionsebene blockieren die Strukturkopien (rep movsw) SI/DI.
 * - Heldensammlung als Makro COLLECT_HERO (h = &g_heroes[c - 1] -> imul / add ax,[off] / add ax,0fa11h).
 * Originalfehler/Auffaelliges: sfight_seq_action mode 3 - fuer Schluessel 0x808/0x908 ist das zweite Feld (9, 0) statt
 *   (9, 8); ohne passenden Schluessel bleiben Ziel und Anzahl uninitialisiert. sfight_seq_area_attack - ohne passenden Schluessel
 *   ebenso (Ziel, Zeile/Spalte, Anzahl), ohne Helden in der Liste bleibt seq->linked (first) uninitialisiert, und
 *   zurueckgegeben wird q (letztes Geschoss, sonst Ergebnis von sfight_enemy_move, ggf. uninitialisiert). sfight_special_picture uebergibt
 *   Nvf::load zweimal &w als Breite und Hoehe.
 */
#include <mem.h>
#include <stdlib.h>
#include "schweif.h"

/* Phase B: eigene Daten des Moduls (ds:6b30..6b9c, dsana) - initialisierte Daten in Adressreihenfolge */
struct seq27 g_sfight_spell_seq1 = {
	{ 45, 45, 45, 46, 60, 60, 72, 72, 56, 54, 52, 53, 39, 41, 43, 44, 45, 46, 45, 45, 45, 45, 45, 45, 45, 45, -1 }
};
struct seq25 g_sfight_spell_seq2 = { { 19, 19, 7, 8, 20, 31, 44, 44, 45, 45, 58, 73, 75, 61, 49, 36, 22, 20, 20, 20, 20, 20, 20, 20, -1 } };
struct points7 g_sfight_pic_size = { { { 20, 12 }, { 34, 21 }, { 62, 37 }, { 91, 55 }, { 119, 70 }, { 156, 93 }, { 216, 132 } } };
struct points7 g_sfight_pic_pos = { { { 176, 44 }, { 169, 43 }, { 154, 38 }, { 139, 36 }, { 126, 34 }, { 107, 29 }, { 78, 22 } } };

#define NVF_HEADS ((Nvf far *)g_nvf_heads)	/* NVF-Dateiobjekt (O098) */

FIGANIM_PTR sfight_enemy_reset(FIGHT_PTR f, struct ani_seq far *seq)
{
	struct fig_node far *node;
	signed short m;
	signed short img;
	register signed short i;

	if (g_sfight_phase) {
		for (i = 0; i <= 2; i++)
			fig_list_show(&f->list, g_sfight_figs[i]);
		fig_list_hide(&f->list, g_sfight_fig3);
	} else {
		for (i = 0; i <= 3; i++)
			fig_list_hide(&f->list, g_sfight_figs[i]);
	}
	node = fig_list_find(&f->list, g_sfight_fig3);
	node->anim->next = 0;
	memset(seq, 0, sizeof(struct ani_seq));
	seq->figure = FIG_FIGURE_SFIGHT;
	seq->step[0].frame = 1;
	seq->step[0].type = 3;
	seq->step[0].count = -1;
	seq->step[1].frame = -1;
	node = fig_list_find(&f->list, g_sfight_figs[0]);
	seq_set(node->anim, seq);
	memset(seq, 0, sizeof(struct ani_seq));
	seq->figure = FIG_FIGURE_SFIGHT;
	seq->step[0].frame = g_sfight_pos;
	seq->step[0].type = 3;
	seq->step[0].count = -1;
	seq->step[1].frame = -1;
	node = fig_list_find(&f->list, g_sfight_fig1);
	seq_set(node->anim, seq);
	memset(seq, 0, sizeof(struct ani_seq));
	m = g_sfight_pos % SFIGHT_GRID_W;
	seq->figure = FIG_FIGURE_SFIGHT;
	if (m >= 10)
		img = 13;
	else if (m >= 8)
		img = 12;
	else if (m >= 5)
		img = 11;
	else if (m >= 3)
		img = 10;
	else
		img = 9;
	seq->step[0].frame = img;
	seq->step[0].type = 3;
	seq->step[0].count = -1;
	seq->step[1].frame = -1;
	node = fig_list_find(&f->list, g_sfight_fig2);
	seq_set(node->anim, seq);
	return node->anim;
}

FIGANIM_PTR sfight_enemy_move(FIGHT_PTR f, struct ani_seq far *seq, register signed short target)
{
	struct fig_node far *walk;
	struct fig_node far *body;
	signed short n;
	signed short cur;
	signed short m;
	signed short img;
	signed short tm;
	struct ani_seq tmp;

	n = 0;
	memset(seq, 0, sizeof(struct ani_seq));
	memset(&tmp, 0, sizeof(struct ani_seq));
	seq->figure = tmp.figure = FIG_FIGURE_SFIGHT;
	cur = g_sfight_pos;
	tm = target % SFIGHT_GRID_W;
	while (cur != target) {
		if (cur % SFIGHT_GRID_W < tm) {
			if (cur > target) {
				do {
					if (target + 13 <= cur)
						cur -= 13;
					if (cur % SFIGHT_GRID_W < tm && abs(cur - target) < 13)
						cur++;
				} while (is_in_word_array(cur, g_sfight_blocked) != -1);
			} else {
				do {
					if (target - 13 >= cur)
						cur += 13;
					if (cur % SFIGHT_GRID_W < tm && abs(cur - target) < 13)
						cur++;
				} while (is_in_word_array(cur, g_sfight_blocked) != -1);
			}
		} else {
			if (cur > target) {
				do {
					if (target + 13 <= cur)
						cur -= 13;
					if (cur % SFIGHT_GRID_W > tm && abs(cur - target) < 13)
						cur--;
				} while (is_in_word_array(cur, g_sfight_blocked) != -1);
			} else {
				do {
					if (target - 13 >= cur)
						cur += 13;
					if (cur % SFIGHT_GRID_W > tm && abs(cur - target) < 13)
						cur--;
				} while (is_in_word_array(cur, g_sfight_blocked) != -1);
			}
		}
		seq->step[n].frame = cur;
		seq->step[n].type = 3;
		seq->step[n].count = 3;
		n++;
	}
	seq->step[n].frame = cur;
	seq->step[n].type = 3;
	seq->step[n].count = -1;
	seq->step[n + 1].frame = -1;
	m = target % SFIGHT_GRID_W;
	if (m >= 10)
		img = 13;
	else if (m >= 8)
		img = 12;
	else if (m >= 5)
		img = 11;
	else if (m >= 3)
		img = 10;
	else
		img = 9;
	tmp.step[0].frame = img;
	tmp.step[0].type = 3;
	tmp.step[0].count = -1;
	tmp.step[1].frame = -1;
	walk = fig_list_find(&f->list, g_sfight_fig1);
	seq_set(walk->anim, seq);
	body = fig_list_find(&f->list, g_sfight_fig2);
	seq_set(body->anim, &tmp);
	body->anim->set_next(walk->anim);
	return walk->anim;
}

FIGANIM_PTR sfight_enemy_end_seq(FIGHT_PTR f, struct ani_seq far *seq)
{
	struct fig_node far *node;

	fig_list_hide(&f->list, g_sfight_fig2);
	fig_list_hide(&f->list, g_sfight_fig1);
	fig_list_hide(&f->list, g_sfight_figs[0]);
	fig_list_show(&f->list, g_sfight_fig3);
	node = fig_list_find(&f->list, g_sfight_fig3);
	memset(seq, 0, sizeof(struct ani_seq));
	seq->figure = FIG_FIGURE_SFIGHT;
	seq->step[0].frame = 0x4f;
	seq->step[0].type = 0;
	seq->step[0].count = 1;
	seq->step[1].frame = -1;
	seq_set(node->anim, seq);
	g_sfight_ofs_x = 0x74;
	g_sfight_ofs_y = 0;
	return node->anim;
}

#define COLLECT_HERO(c) \
	if (c > 0 && c <= 10) { \
		h = &g_heroes[c - 1]; \
		if (check_hero2(h)) { \
			g_fig_hit_heroes[g_fig_nr_hits] = h; \
			g_fig_nr_hits++; \
		} \
	}

FIGANIM_PTR sfight_seq_action(FIGHT_PTR f, struct ani_seq far *seq, signed short mode)
{
	signed short n;
	struct fig_node far *node;
	FIGANIM_PTR walk_anim;
	signed short m;
	signed short img;
	signed short target;
	signed short dx;
	signed short dy;
	signed short x;
	signed short y;
	struct enemy far *e;
	struct hero far *h;
	signed short xs[4];
	signed short ys[4];
	signed short cnt;
	signed short j;
	signed short c;
	register signed short key;	/* (x << 8) + y */

	n = 0;
	e = g_enemies;
	fight_find_cell(e->enemy_id, &x, &y);
	if (mode == 0) {
		fig_list_show(&f->list, g_sfight_fig3);
		node = fig_list_find(&f->list, g_sfight_fig3);
		memset(seq, 0, sizeof(struct ani_seq));
		seq->figure = FIG_FIGURE_SFIGHT;
		seq->step[0].frame = 0x0e;
		seq->step[0].type = 0;
		seq->step[0].count = 1;
		seq->step[1].frame = -1;
		seq_set(node->anim, seq);
		g_sfight_ofs_x = 0xe0;
		g_sfight_ofs_y = 0x16;
	} else if (mode == 3) {
		key = (x << 8) + y;
		if (key == 0x109 || key == 0x209 || key == 0x309) {
			target = 0x41;
			cnt = 3;
			xs[0] = 1; ys[0] = 9;
			xs[1] = 2; ys[1] = 9;
			xs[2] = 3; ys[2] = 9;
		} else if (key == 0x308 || key == 0x408 || key == 0x508) {
			target = 0x42;
			cnt = 3;
			xs[0] = 3; ys[0] = 8;
			xs[1] = 4; ys[1] = 8;
			xs[2] = 5; ys[2] = 8;
		} else if (key == 0x608 || key == 0x708 || key == 0x709) {
			target = 0x45;
			cnt = 3;
			xs[0] = 6; ys[0] = 8;
			xs[1] = 7; ys[1] = 8;
			xs[2] = 7; ys[2] = 9;
		} else if (key == 0x808 || key == 0x908) {
			target = 0x46;
			cnt = 2;
			xs[0] = 8; ys[0] = 8;
#if FIX_0189
			/* Fix #189: zweites Feld (9, 8) wie der Schluessel 0x908 */
			xs[1] = 9; ys[1] = 8;
#else
			xs[1] = 9; ys[1] = 0;
#endif
		} else if (key == 0x909 || key == 0x90a) {
			target = 0x48;
			cnt = 2;
			xs[0] = 9; ys[0] = 9;
			xs[1] = 9; ys[1] = 10;
		} else if (key == 0x90b || key == 0x90c || key == 0x80c || key == 0x90d) {
			target = 0x4a;
			cnt = 4;
			xs[0] = 9; ys[0] = 11;
			xs[1] = 9; ys[1] = 12;
			xs[2] = 8; ys[2] = 12;
			xs[3] = 9; ys[3] = 13;
		} else if (key == 0x90e || key == 0x90f) {
			target = 0x4c;
			cnt = 2;
			xs[0] = 9; ys[0] = 14;
			xs[1] = 9; ys[1] = 15;
		} else if (key == 0x810 || key == 0x910) {
			target = 0x4d;
			cnt = 2;
			xs[0] = 8; ys[0] = 16;
			xs[1] = 9; ys[1] = 16;
		}
		for (j = 0; j < cnt; j++) {
			c = *(g_fight_walkmap + ys[j] * 25 + xs[j]);
			COLLECT_HERO(c)
		}
		if (target != g_sfight_pos) {
			walk_anim = sfight_enemy_move(f, seq, target);
			do {
				game_idle();
			} while (!seq_done(walk_anim));
		}
		fig_list_hide(&f->list, g_sfight_fig2);
		fig_list_show(&f->list, g_sfight_fig3);
		node = fig_list_find(&f->list, g_sfight_fig3);
		memset(seq, 0, sizeof(struct ani_seq));
		m = g_sfight_pos % SFIGHT_GRID_W;
		if (m >= 10) {
			img = 0x76; dx = 5; dy = 8;
		} else if (m >= 8) {
			img = 0x75; dx = 9; dy = 0x10;
		} else if (m >= 5) {
			img = 0x74; dx = 10; dy = 15;
		} else if (m >= 3) {
			img = 0x73; dx = -3; dy = 13;
		} else {
			img = 0x72; dx = -15; dy = 13;
		}
		seq->figure = FIG_FIGURE_SFIGHT;
		seq->step[0].frame = img;
		seq->step[0].type = 0;
		seq->step[0].count = 1;
		g_sfight_ofs_x = g_sfight_grid_xy[g_sfight_pos].x + dx;
		g_sfight_ofs_y = g_sfight_grid_xy[g_sfight_pos].y + dy;
		seq->step[1].frame = -1;
		seq_set(node->anim, seq);
	} else {
		if (g_sfight_pos != 0x2d) {
			walk_anim = sfight_enemy_move(f, seq, 0x2d);
			do {
				game_idle();
			} while (!seq_done(walk_anim));
		}
		fig_list_hide(&f->list, g_sfight_fig2);
		fig_list_hide(&f->list, g_sfight_fig1);
		fig_list_hide(&f->list, g_sfight_figs[0]);
		fig_list_show(&f->list, g_sfight_fig3);
		node = fig_list_find(&f->list, g_sfight_fig3);
		memset(seq, 0, sizeof(struct ani_seq));
		seq->figure = FIG_FIGURE_SFIGHT;
		if (mode == 1) {
			seq->step[n].frame = 0x5a;
			g_sfight_ofs_x = 0x63;
			g_sfight_ofs_y = 1;
		} else {
			seq->step[n].frame = 0x5b;
			g_sfight_ofs_x = 0x7d;
			g_sfight_ofs_y = 1;
		}
		seq->step[n].type = 0;
		seq->step[n].event = 0x26;
		seq->step[n].count = 1;
		n++;
		seq->step[n].frame = 0x5c;
		seq->step[n].type = 0;
		seq->step[n].count = 1;
		n++;
		seq->step[n].frame = -1;
		seq_set(node->anim, seq);
		g_sfight_ofs2_x = 0xa3;
		g_sfight_ofs2_y = 1;
	}
	return node->anim;
}

FIGANIM_PTR sfight_seq_walk_random(FIGHT_PTR f, struct ani_seq far *seq)
{
	register signed short t;

	do {
		t = random_interval(4, 0x4d);
	} while (is_in_word_array(t, g_sfight_blocked) != -1);
	return sfight_enemy_move(f, seq, t);
}

FIGANIM_PTR sfight_seq_area_attack(FIGHT_PTR f, struct ani_seq far *seq)
{
	signed short n;
	signed short key;
	signed short i;
	signed short col;
	signed short k;
	signed short cnt;
	struct fig_node far *node;
	FIGANIM_PTR q;
	FIGANIM_PTR first;
	FIGANIM_PTR prev;
	signed short m;
	signed short img;
	signed short target;
	signed short img2;
	signed short dx;
	signed short dy;
	signed short x;
	signed short y;
	signed short dx2;
	signed short dy2;
	signed short flag;
	struct enemy far *e;
	struct hero far *t;
	struct hero far *h;
	register signed short c;
	struct ani_seq tmp;

	n = 0;
	flag = 1;
	e = g_enemies;
	fight_find_cell(e->enemy_id, &x, &y);
	t = enemy_target_hero(e);
	m = -1;
	if (x == 7) {
		target = 0x0f;
		img2 = 0x63;
		dx2 = 0x45;
		dy2 = 0x44;
		for (i = 0; i < 8; i++) {
			c = *(g_fight_walkmap + i * 25 + x);
			COLLECT_HERO(c)
		}
	} else if (x == 8) {
		target = 0x1e;
		img2 = 0x63;
		dx2 = 0x56;
		dy2 = 0x47;
		for (i = 0; i < 8; i++) {
			c = *(g_fight_walkmap + i * 25 + x);
			COLLECT_HERO(c)
		}
	} else if (x == 9) {
		target = 0x2b;
		img2 = 0x63;
		dx2 = 0x52;
		dy2 = 0x52;
		for (i = 0; i < 8; i++) {
			c = *(g_fight_walkmap + i * 25 + x);
			COLLECT_HERO(c)
		}
	} else if (x == 11 && y == 11) {
		flag = 0;
		target = 0x17;
		g_fig_hit_heroes[g_fig_nr_hits] = t;
		g_fig_nr_hits++;
	} else if (x == 10 && y == 12) {
		flag = 0;
		target = 0x26;
		g_fig_hit_heroes[g_fig_nr_hits] = t;
		g_fig_nr_hits++;
	} else if (y == 10) {
		target = 0x15;
		img2 = 0x65;
		dx2 = 0xbf;
		dy2 = 0x45;
		for (col = 10; col < 0x15; col++) {
			c = *(g_fight_walkmap + y * 25 + col);
			COLLECT_HERO(c)
		}
	} else {
		img2 = 0x64;
		key = (x << 8) + y;
		if (key == 0xa05 || key == 0xb04 || key == 0xc03 || key == 0xd02 || key == 0xe01 || key == 0xf00) {
			target = 0x42; dx2 = 0x8c; dy2 = 0x50; col = 10; i = 5; cnt = 6;
		} else if (key == 0xa06 || key == 0xb05 || key == 0xc04 || key == 0xd03 || key == 0xe02 || key == 0xf01 || key == 0x1000) {
			target = 0x36; dx2 = 0x96; dy2 = 0x50; col = 10; i = 6; cnt = 7;
		} else if (key == 0xa07 || key == 0xb06 || key == 0xc05 || key == 0xd04 || key == 0xe03 || key == 0xf02 || key == 0x1001) {
			target = 0x2a; dx2 = 0xa0; dy2 = 0x50; col = 10; i = 7; cnt = 7;
		} else if (key == 0xa08 || key == 0xb07 || key == 0xc06 || key == 0xd05 || key == 0xe04 || key == 0xf03 || key == 0x1002) {
			target = 5; dx2 = 0xaa; dy2 = 0x50; col = 10; i = 8; cnt = 7;
		} else if (key == 0xa09 || key == 0xb08 || key == 0xc07 || key == 0xd06 || key == 0xe05 || key == 0xf04 || key == 0x1003) {
			target = 8; dx2 = 0xb4; dy2 = 0x50; col = 10; i = 9; cnt = 7;
		} else if (key == 0xb09 || key == 0xc08 || key == 0xd07 || key == 0xe06 || key == 0xf05 || key == 0x1004 || key == 0x1103) {
			target = 0x3e; dx2 = 0xbe; dy2 = 0x50; col = 11; i = 9; cnt = 7;
		}
		m = 1;
		img = 0x60;
		dx = 0x16;
		dy = 0x1a;
		for (k = 0; k < cnt; k++) {
			c = *(g_fight_walkmap + (i - k) * 25 + col + k);
			COLLECT_HERO(c)
		}
	}
	if (target != g_sfight_pos) {
		q = sfight_enemy_move(f, seq, target);
		do {
			game_idle();
		} while (!seq_done(q));
	}
	fig_list_hide(&f->list, g_sfight_fig2);
	fig_list_show(&f->list, g_sfight_fig3);
	node = fig_list_find(&f->list, g_sfight_fig3);
	memset(seq, 0, sizeof(struct ani_seq));
	if (m == -1) {
		m = g_sfight_pos % SFIGHT_GRID_W;
		if (m >= 10) {
			img = 0x62; dx = 0x1b; dy = 0x1a;
		} else if (m >= 8) {
			img = 0x61; dx = 0x17; dy = 0x1a;
		} else if (m >= 5) {
			img = 0x60; dx = 0x16; dy = 0x1a;
		} else if (m >= 3) {
			img = 0x5f; dx = 0x16; dy = 0x1a;
		} else {
			img = 0x5e; dx = 0x23; dy = 0x1a;
		}
	}
	seq->figure = FIG_FIGURE_SFIGHT;
	seq->step[n].frame = img;
	seq->step[n].type = 0;
	seq->step[n].count = 1;
	seq->step[n].event = 0x28;
	n++;
	g_sfight_ofs_x = g_sfight_grid_xy[g_sfight_pos].x - dx + 0x1f;
	g_sfight_ofs_y = g_sfight_grid_xy[g_sfight_pos].y - dy + 0x28;
	if (flag) {
		seq->step[n].frame = img2;
		seq->step[n].type = 0;
		seq->step[n].count = 1;
		n++;
		g_sfight_ofs2_x = dx2;
		g_sfight_ofs2_y = dy2;
	}
	for (k = 0; k < g_fig_nr_hits; k++) {
		h = g_fig_hit_heroes[k];
		fight_find_cell(get_hero_index(h) + 1, &x, &y);
		memset(&tmp, 0, sizeof(struct ani_seq));
		q = fight_new_extra_anim(f);
		tmp.figure = FIG_FIGURE_SFIGHT;
		tmp.step[0].frame = 0x6c;
		tmp.step[0].type = 0;
		tmp.step[0].event = 0x20;
		f->tmpl.anim = q;
		f->tmpl.cbx = f->tmpl.offsetx = x;
		f->tmpl.cby = f->tmpl.offsety = y;
		f->tmpl.kind = 0;
		f->tmpl.figure = 0x91;
		f->tmpl.gfx = 0;
		f->tmpl.z = 100;
		f->tmpl.visible = 1;
		f->tmpl.twofielded = -1;
		fig_list_add(&f->list, -1);
		seq_set(q, &tmp);
		q->delay_fix = 0;
		if (k > 0)
			prev->set_next(q);
		else
			first = q;
		prev = q;
	}
	seq->step[n - 1].event = 0x1f;
	seq->linked = first;
	seq->step[n].frame = -1;
	seq_set(node->anim, seq);
	return q;
}

FIGANIM_PTR sfight_seq_special(FIGHT_PTR f, struct ani_seq far *seq)
{
	struct fig_node far *node;
	FIGANIM_PTR walk_anim;
	signed short m;
	signed short img;
	signed short dy;
	register signed short dx;

	if (g_sfight_pos != 0x2d) {
		walk_anim = sfight_enemy_move(f, seq, 0x2d);
		do {
			game_idle();
		} while (!seq_done(walk_anim));
	}
	fig_list_hide(&f->list, g_sfight_fig2);
	fig_list_show(&f->list, g_sfight_fig3);
	node = fig_list_find(&f->list, g_sfight_fig3);
	memset(seq, 0, sizeof(struct ani_seq));
	m = g_sfight_pos % SFIGHT_GRID_W;
	if (m >= 10) {
		img = 0x54; dx = 0x1b; dy = 0x1a;
	} else if (m >= 8) {
		img = 0x53; dx = 0x17; dy = 0x1a;
	} else if (m >= 5) {
		img = 0x52; dx = 0x16; dy = 0x1a;
	} else if (m >= 3) {
		img = 0x51; dx = 0x13; dy = 0x1a;
	} else {
		img = 0x50; dx = 0x1c; dy = 0x1a;
	}
	seq->figure = FIG_FIGURE_SFIGHT;
	seq->step[0].frame = img;
	seq->step[0].type = 0;
	seq->step[0].count = 1;
	seq->step[1].frame = -1;
	seq_set(node->anim, seq);
	g_sfight_ofs_x = g_sfight_grid_xy[g_sfight_pos].x - dx + 0x1f;
	g_sfight_ofs_y = g_sfight_grid_xy[g_sfight_pos].y - dy + 0x28;
	return node->anim;
}

FIGANIM_PTR sfight_seq_spell(FIGHT_PTR f, struct ani_seq far *seq, signed short spell)
{
	struct fig_node far *walk;
	struct fig_node far *body;
	FIGANIM_PTR walk_anim;
	signed short n;
	signed short odd;
	signed char far *s;
	signed short target;
	struct seq27 seq1;
	struct seq25 seq2;

	n = 0;
	seq1 = g_sfight_spell_seq1;
	seq2 = g_sfight_spell_seq2;
	odd = (spell + 1) % 2;
	target = odd ? 0x2d : 0x13;
	if (target != g_sfight_pos) {
		walk_anim = sfight_enemy_move(f, seq, target);
		do {
			game_idle();
		} while (!seq_done(walk_anim));
	}
	s = odd ? seq1.frame : seq2.frame;
	n = 0;
	memset(seq, 0, sizeof(struct ani_seq));
	while (*s != -1) {
		seq->figure = FIG_FIGURE_SFIGHT;
		seq->step[n].frame = *s;
		seq->step[n].type = 0;
		seq->step[n].count = 1;
		n++;
		s++;
	}
	seq->step[n].frame = -1;
	walk = fig_list_find(&f->list, g_sfight_fig1);
	seq_set(walk->anim, seq);
	fig_list_hide(&f->list, g_sfight_fig2);
	fig_list_show(&f->list, g_sfight_fig3);
	body = fig_list_find(&f->list, g_sfight_fig3);
	memset(seq, 0, sizeof(struct ani_seq));
	seq->figure = FIG_FIGURE_SFIGHT;
	seq->step[0].frame = odd ? 2 : 3;
	seq->step[0].type = 0;
	seq->step[0].count = 1;
	seq->step[1].frame = -1;
	seq_set(body->anim, seq);
	body->anim->set_next(walk->anim);
	if (odd) {
		g_sfight_ofs_x = 0x8c;
		g_sfight_ofs_y = 5;
	} else {
		g_sfight_ofs_x = 0xa1;
		g_sfight_ofs_y = -10;
	}
	return body->anim;
}

void sfight_special_picture(FIGHT_PTR f)
{
	Bmp pic(0x133, 0xb6, f->buf);
	Bmp part;
	signed short w;
	signed short i;
	struct points7 size = g_sfight_pic_size;
	struct points7 pos = g_sfight_pic_pos;

	fig_stand_seq(f, g_enemies, &f->seq);
	fight_draw(f);
	g_xms_move.len = 0xfa00L;
	g_xms_move.dst_handle = g_fight_xms;
	g_xms_move.dst = 0x2c880L;
	g_xms_move.src_handle = 0;
	g_xms_move.src = (signed long)g_vga_buf;
	if (g_fight_use_ems == 1)
		EMS_copy(&g_xms_move);
	else
		xms_move_any(&g_xms_move);
	NVF_HEADS->load(0x372, f->buf, 0, 0, 0, &w, &w, 0x140);
	g_xms_move.len = 0xfa00L;
	g_xms_move.dst_handle = g_fight_xms;
	g_xms_move.dst = 0xfa00L;
	g_xms_move.src_handle = 0;
	g_xms_move.src = (signed long)f->buf;
	if (g_fight_use_ems == 1)
		EMS_copy(&g_xms_move);
	else
		xms_move_any(&g_xms_move);
	for (i = 0; i <= 6; i++) {
		register signed short x;
		register signed short y;

		x = size.p[i].x;
		y = size.p[i].y;
		part.init(x, y, f->buf);
		g_xms_move.len = 0xfa00L;
		g_xms_move.dst_handle = 0;
		g_xms_move.dst = (signed long)f->buf;
		g_xms_move.src_handle = g_fight_xms;
		g_xms_move.src = 0xfa00L;
		if (g_fight_use_ems == 1)
			EMS_copy(&g_xms_move);
		else
			xms_move_any(&g_xms_move);
		pic.scale(&part);
		g_xms_move.len = 0xfa00L;
		g_xms_move.dst_handle = 0;
		g_xms_move.dst = (signed long)g_vga_buf;
		g_xms_move.src_handle = g_fight_xms;
		g_xms_move.src = 0x2c880L;
		if (g_fight_use_ems == 1)
			EMS_copy(&g_xms_move);
		else
			xms_move_any(&g_xms_move);
		part.put(g_vga_buf, pos.p[i].x, pos.p[i].y, 4);
		part.free_lines();
	}
	g_xms_move.len = 0xfa00L;
	g_xms_move.dst_handle = 0;
	g_xms_move.dst = (signed long)f->buf;
	g_xms_move.src_handle = g_fight_xms;
	g_xms_move.src = 0xfa00L;
	if (g_fight_use_ems == 1)
		EMS_copy(&g_xms_move);
	else
		xms_move_any(&g_xms_move);
	g_xms_move.len = 0xfa00L;
	g_xms_move.dst_handle = 0;
	g_xms_move.dst = (signed long)g_vga_buf;
	g_xms_move.src_handle = g_fight_xms;
	g_xms_move.src = 0x2c880L;
	if (g_fight_use_ems == 1)
		EMS_copy(&g_xms_move);
	else
		xms_move_any(&g_xms_move);
	pic.put(g_vga_buf, 0x29, 8, 4);
	g_xms_move.len = 0xfa00L;
	g_xms_move.dst_handle = 0;
	g_xms_move.dst = (signed long)g_vga_buf;
	g_xms_move.src_handle = g_fight_xms;
	g_xms_move.src = 0x2c880L;
	if (g_fight_use_ems == 1)
		EMS_copy(&g_xms_move);
	else
		xms_move_any(&g_xms_move);
	g_fig_redraw_all = 1;
}
