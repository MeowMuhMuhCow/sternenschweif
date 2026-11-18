/* OVL141 - Overlay O141 (Segment 0x222c in SCHWEIF.EXE): Sonderkampf 0x493 gegen einen Einzelgegner (Kampfobjekt Fight,
 * Kampffeld f->field: type, cells[24][25]).
 * sfight_setup(f): Grundzustand - Art 4, fight_load_grid, Hindernisfelder aus g_sfight_walls/_walls2/_cells ({x, y},
 *   Ende x == -1) = FIG_CELL_WALL/_WALL2/_SFIGHT, Einzelfelder 0x90..0x9b, Startplaetze f->heroes aus
 *   g_sfight_starts (wie O021), NSC 2 (add_npc) versteinert, schlafende Helden behalten asleep, Gegner
 *   monsters[0] = {0x33, 4, 0x0e}, Beute/Geld 0.
 * sfight_place_enemy(f, e): vier Animationsobjekte (FigAnim_ctor, Lage 3/16 bzw. 12/12, FIG_FIGURE_SFIGHT), IDs
 *   nach g_sfight_figs[0..3], e->fighter_id = Teil 0.
 * sfight_enemy_action(f): Zug des Sondergegners (g_enemies[0]): attacks_left--, wenn aktiv und nicht geblendet
 *   (blind--) Aktion sfight_enemy_turn -> FIG_ACTION_MELEE_ATTACK/_SPELL/_RANGE_ATTACK sfight_do_action, sonst
 *   zufaellige Lage (random_interval(4, 0x4d) nicht in g_sfight_blocked) per sfight_enemy_move; bei g_sfight_won
 *   nur dead = 1.
 * sfight_do_action(f, e): Meldungsliste g_fight_list (Lst_init/Lst_run): Zauber -> sfight_special_attack, sonst
 *   Ziel-Held enemy_target_hero (tot/ohne Typ -> Abbruch), Nahkampf -> sfight_attack (Art je cur_spell: 0
 *   Einzelangriff, 3 alle, sonst Schaden = LE), Fernkampf -> sfight_area_attack (Flaechenangriff auf die Helden aus
 *   sfight_seq_area_attack: Schaden dice_code_roll(dam2) - RS, halbiert bei Gegenstand 0x11e (get_item_pos) und
 *   fireban).
 * sfight_special_attack(f, e): Zauber aus g_sfight_spells (13, zufaellig) nach cur_spell, jeder handlungsfaehige
 *   Held mit W100 < 50 wird Ziel (enemy_set_target_hero/enemy_cast_spell, Meldung aus g_text_output_buf in eigenen
 *   Puffer mem_alloc(200)), Bildfolgen sfight_seq_special/sfight_special_picture/fig_hero_spell_hit_seq, danach
 *   Meldungen per dlg_message ausgeben und freigeben.
 * sfight_fig_pos(f, n, &x, &y): Bildlage des Teils n (0/1 aus g_sfight_part_x/y, 2 aus g_sfight_grid_xy[g_sfight_pos],
 *   sonst g_sfight_ofs_x/y).
 *
 * BCC -mlarge -3 -Ot -Og -Oi -c -Y OVL141.CPP   -> 4622/4622 Bytes GLEICH (18.10.2026), cobjfix laeuft
 *   (Namensfassung mit schweif.h: 19.10.2026 GLEICH, LINKGLEICH; g_fig_target_killed muss signed char sein,
 *   sonst andere Reihenfolge in der Zuweisungskette von sfight_do_action)
 * - -Oi fuer memset (rep stosw) und strcpy in sfight_special_attack; -Og nur fuer die Epilogkopien in sfight_fig_pos (ohne: 4 Bytes anders).
 * - C++ wegen der Bitfelder (+bc, +38; Wahrheitstest mit (signed char)) und des inline-Setters
 *   enemy::set_cur_spell(signed char) in sfight_special_attack
 *   (Kopie [bp-9]: mov al,[bp-2] / mov [bp-9],al / les / mov al,[bp-9]).
 * - RS-Abzug "dam -= (signed char)(h->rs_bonus + h->rs_bonus2)" -> mov al / add al / cbw / sub di,ax; ohne Cast zwei cbw + push/pop.
 * - "g_fig_hit_anims[i] = g_fig_hit_anims[0]" erzeugt von selbst das seltsame mov eax,[ca9e] / mov dx,[ca9c] (Zeiger als FIGANIM_PTR) / mov [..],ax / [..],dx.
 * - sfight_enemy_action: char act vor dem Zeiger deklarieren ([bp-1] vor [bp-6]); "e->action_id = 12; act = 12;" getrennt.
 * - Karte als struct field mit map[24][25] ab +15: "f->field->cells[y][x]" ergibt imul 25 / les / add / x / [bx+15h].
 * Originalfehler/Auffaelliges: sfight_attack Fall cur_spell == 3 (Angriff auf alle) prueft g_fig_nr_hits > 0, aber fig_clear_hits hat
 *   den Zaehler eben auf 0 gesetzt und fig_enemy_attack_seq fuellt die Liste nicht -> Zweig ist tot, niemand nimmt Schaden.
 *   sfight_setup liest h->asleep/h->npc_id vor der Nullpruefung (wie O021). sfight_enemy_action "act = 12" sofort ueberschrieben.
 *   fig_clear_hits(void) wird mit f aufgerufen (harmlos; Makro FIG_CLEAR_HITS mit gecastetem Funktionszeiger).
 */
#include <string.h>
#include "schweif.h"

/* Phase B: eigene Daten des Moduls (ds:6882..6b30, dsana) - initialisierte Daten in Adressreihenfolge */
unsigned char g_sfight_figs[1] = { 0xff };
unsigned char g_sfight_fig1 = 0xff;
unsigned char g_sfight_fig2 = 0xff;
unsigned char g_sfight_fig3 = 0xff;
signed short g_sfight_part_x[4] = { 133, 150, 150, 150 };
signed short g_sfight_part_y[4] = { 0, 1, -24, 0 };
signed short g_sfight_pos = 4;
signed short g_sfight_phase = 0;
struct point g_sfight_grid_xy[78] = {
	{ -1, -1 },
	{ -1, -1 },
	{ -1, -1 },
	{ -1, -1 },
	{ 150, -24 },
	{ 153, -24 },
	{ 156, -24 },
	{ 159, -24 },
	{ 162, -24 },
	{ -1, -1 },
	{ -1, -1 },
	{ -1, -1 },
	{ -1, -1 },
	{ -1, -1 },
	{ -1, -1 },
	{ 144, -19 },
	{ 147, -19 },
	{ 150, -19 },
	{ 153, -19 },
	{ 156, -19 },
	{ 159, -19 },
	{ 162, -19 },
	{ 165, -19 },
	{ 168, -19 },
	{ -1, -1 },
	{ -1, -1 },
	{ 138, -14 },
	{ 141, -14 },
	{ 144, -14 },
	{ 147, -14 },
	{ 150, -14 },
	{ 153, -14 },
	{ 156, -14 },
	{ 159, -14 },
	{ 162, -14 },
	{ 165, -14 },
	{ 168, -14 },
	{ 171, -14 },
	{ 175, -14 },
	{ 132, -9 },
	{ 136, -9 },
	{ 140, -9 },
	{ 144, -9 },
	{ 148, -9 },
	{ 152, -9 },
	{ 156, -9 },
	{ 160, -9 },
	{ 164, -9 },
	{ 168, -9 },
	{ 172, -9 },
	{ 176, -9 },
	{ 180, -9 },
	{ 126, -4 },
	{ 131, -4 },
	{ 136, -4 },
	{ 141, -4 },
	{ 146, -4 },
	{ 151, -4 },
	{ 156, -4 },
	{ 161, -4 },
	{ 166, -4 },
	{ 171, -4 },
	{ 176, -4 },
	{ 181, -4 },
	{ 186, -4 },
	{ 120, 1 },
	{ 126, 1 },
	{ 132, 1 },
	{ 138, 1 },
	{ 144, 1 },
	{ 150, 1 },
	{ 156, 1 },
	{ 162, 1 },
	{ 168, 1 },
	{ 174, 1 },
	{ 180, 1 },
	{ 186, 1 },
	{ 192, 1 }
};
signed short g_sfight_blocked[13] = { 0, 1, 2, 3, 9, 10, 11, 12, 13, 14, 24, 25, -1 };
struct cxy g_sfight_walls[23] = {
	{ 0, 0 },
	{ 0, 1 },
	{ 0, 3 },
	{ 0, 9 },
	{ 0, 10 },
	{ 0, 11 },
	{ 0, 12 },
	{ 1, 0 },
	{ 1, 10 },
	{ 1, 11 },
	{ 6, 19 },
	{ 6, 20 },
	{ 6, 21 },
	{ 6, 22 },
	{ 6, 23 },
	{ 7, 19 },
	{ 7, 20 },
	{ 7, 21 },
	{ 7, 22 },
	{ 7, 23 },
	{ 8, 22 },
	{ 8, 23 },
	{ -1, -1 }
};
struct cxy g_sfight_walls2[10] = {
	{ 10, 0 },
	{ 10, 1 },
	{ 10, 2 },
	{ 10, 3 },
	{ 11, 0 },
	{ 11, 1 },
	{ 11, 2 },
	{ 16, 4 },
	{ 16, 5 },
	{ -1, -1 }
};
struct cxy g_sfight_cells[101] = {
	{ 5, 14 },
	{ 5, 13 },
	{ 4, 13 },
	{ 3, 13 },
	{ 3, 14 },
	{ 3, 15 },
	{ 4, 15 },
	{ 5, 15 },
	{ 6, 15 },
	{ 6, 14 },
	{ 6, 13 },
	{ 6, 12 },
	{ 5, 12 },
	{ 4, 12 },
	{ 3, 12 },
	{ 2, 12 },
	{ 2, 13 },
	{ 2, 14 },
	{ 2, 15 },
	{ 2, 16 },
	{ 3, 16 },
	{ 4, 16 },
	{ 5, 16 },
	{ 6, 16 },
	{ 7, 16 },
	{ 7, 15 },
	{ 7, 14 },
	{ 7, 13 },
	{ 7, 12 },
	{ 6, 11 },
	{ 5, 11 },
	{ 4, 11 },
	{ 3, 11 },
	{ 2, 11 },
	{ 1, 12 },
	{ 1, 13 },
	{ 1, 14 },
	{ 1, 15 },
	{ 1, 16 },
	{ 1, 17 },
	{ 2, 17 },
	{ 3, 17 },
	{ 4, 17 },
	{ 5, 17 },
	{ 8, 15 },
	{ 8, 14 },
	{ 8, 13 },
	{ 8, 11 },
	{ 7, 11 },
	{ 7, 10 },
	{ 6, 10 },
	{ 5, 10 },
	{ 4, 10 },
	{ 3, 10 },
	{ 0, 13 },
	{ 0, 14 },
	{ 0, 15 },
	{ 0, 16 },
	{ 0, 17 },
	{ 0, 18 },
	{ 1, 18 },
	{ 2, 18 },
	{ 3, 18 },
	{ 4, 18 },
	{ 6, 9 },
	{ 5, 9 },
	{ 4, 9 },
	{ 3, 19 },
	{ 2, 19 },
	{ 1, 19 },
	{ 0, 19 },
	{ 0, 20 },
	{ 0, 21 },
	{ 0, 22 },
	{ 0, 23 },
	{ 1, 20 },
	{ 1, 21 },
	{ 1, 22 },
	{ 1, 23 },
	{ 2, 20 },
	{ 2, 21 },
	{ 2, 22 },
	{ 2, 23 },
	{ 3, 20 },
	{ 3, 21 },
	{ 3, 22 },
	{ 3, 23 },
	{ 4, 19 },
	{ 4, 20 },
	{ 4, 21 },
	{ 4, 22 },
	{ 4, 23 },
	{ 5, 18 },
	{ 5, 19 },
	{ 5, 20 },
	{ 5, 21 },
	{ 5, 22 },
	{ 5, 23 },
	{ 8, 9 },
	{ 8, 10 },
	{ -1, -1 }
};
signed short g_sfight_seq_on = 0;
struct sfight_starts g_sfight_starts = {
	{ { 4, 0, 3, 0 },
	{ 8, 3, 3, 0 },
	{ 12, 4, 2, 0 },
	{ 17, 7, 2, 0 },
	{ 19, 5, 2, 0 },
	{ 19, 11, 2, 0 },
	{ 9, 11, 1, 0 } }
};
struct sfight_spells g_sfight_spells = { { 7, 3, 9, 10, 11, 9, 8, 9, 9, 10, 10, 7, 7 } };

/* fig_clear_hits(void) wird mit f aufgerufen (Original) */
#define FIG_CLEAR_HITS(f) ((void (*)(FIGHT_PTR))fig_clear_hits)(f)

void sfight_setup(FIGHT_PTR f)
{
	signed short n = 0;
	signed short i;
	struct hero far *h;
	struct sfight_starts starts = g_sfight_starts;

	f->field->type = 4;
	fight_load_grid(f);
	for (n = 0; g_sfight_walls[n].x != -1; n++)
		f->field->cells[g_sfight_walls[n].y][g_sfight_walls[n].x] = FIG_CELL_WALL;
	for (n = 0; g_sfight_walls2[n].x != -1; n++)
		f->field->cells[g_sfight_walls2[n].y][g_sfight_walls2[n].x] = FIG_CELL_WALL2;
	for (n = 0; g_sfight_cells[n].x != -1; n++)
		f->field->cells[g_sfight_cells[n].y][g_sfight_cells[n].x] = FIG_CELL_SFIGHT;
	f->field->cells[6][0] = 0x90;
	f->field->cells[6][1] = 0x91;
	f->field->cells[6][2] = 0x92;
	f->field->cells[16][11] = 0x93;
	f->field->cells[11][12] = 0x94;
	f->field->cells[12][12] = 0x95;
	f->field->cells[12][13] = 0x96;
	f->field->cells[6][16] = 0x97;
	f->field->cells[4][3] = 0x98;
	f->field->cells[5][4] = 0x99;
	f->field->cells[7][5] = 0x9a;
	f->field->cells[5][6] = 0x9b;
	f->field->cells[14][14] = 0x9a;
	f->field->cells[17][12] = 0x99;
	memset(f->heroes, 0, sizeof(f->heroes));
	add_npc(2);
	h = group_get_npc(g_group);
	h->petrified = 1;
	for (i = 0; i < 7; i++) {
		f->heroes[i].x = starts.p[i].x;
		f->heroes[i].y = starts.p[i].y;
		f->heroes[i].viewdir = starts.p[i].viewdir;
	}
	h = get_first_hero(g_group);
	n = 0;
#if FIX_0106
	/* Fix #106: Nullpruefung vor dem ersten Zugriff (wie O021) */
	if (h != 0) {
		do {
			if ((signed char)h->asleep)
				f->heroes[h->npc_id ? 6 : n].asleep = 1;
			h = get_next_hero(g_group, h);
			n++;
		} while (get_first_hero(g_group) != h && get_first_hero(g_group) != 0);
	}
#else
	do {
		if ((signed char)h->asleep)
			f->heroes[h->npc_id ? 6 : n].asleep = 1;
		h = get_next_hero(g_group, h);
		n++;
	} while (get_first_hero(g_group) != h && get_first_hero(g_group) != 0);
#endif
	memset(f->monsters, 0, sizeof(f->monsters));
	f->monsters[0].mon_id = 0x33;
	f->monsters[0].x = 4;
	f->monsters[0].y = 0x0e;
	memset(f->loot, 0, 0x1e);
	f->ducats = f->silver = f->heller = 0;
}

void sfight_place_enemy(FIGHT_PTR f, struct enemy far *e)
{
	char id;
	register signed short i;

	for (i = 0; i <= 3; i++) {
		f->tmpl.anim = f->anims[f->nr_anims++] = (FIGANIM_PTR)FigAnim_ctor(0, &f->seq, 1);
		f->tmpl.cbx = f->tmpl.offsetx = i == 3 ? 12 : 3;
		f->tmpl.cby = f->tmpl.offsety = i == 3 ? 12 : 16;
		f->tmpl.obj_id = 0;
		f->tmpl.twofielded = FIG_TWOFIELD_NONE;
		f->tmpl.kind = FIG_KIND_ENEMY;
		f->tmpl.figure = FIG_FIGURE_SFIGHT;
		f->tmpl.gfx = 0;
		f->tmpl.z = 0x63;
		f->tmpl.visible = 0;
		g_sfight_figs[i] = fig_list_add(&f->list, -1);
	}
	id = g_sfight_figs[0];
	e->fighter_id = id;
}

void sfight_enemy_action(FIGHT_PTR f)
{
	signed char act;
	struct enemy far *e;
	register signed short pos;

	if (g_quick_fight)
		game_idle();
	e = g_enemies;
	e->attacks_left--;
	if (!g_sfight_won) {
		if (enemy_is_active(e)) {
			if (e->blind)
				e->blind--;
			else {
				f->top_enemy = e;
				fight_draw(f);
				e->action_id = FIG_ACTION_WAIT;
				act = FIG_ACTION_WAIT;
				act = sfight_enemy_turn(f);
				if (act == FIG_ACTION_MELEE_ATTACK || act == FIG_ACTION_SPELL || act == FIG_ACTION_RANGE_ATTACK)
					sfight_do_action(f, e);
				else if (!g_sfight_won) {
					do {
						pos = random_interval(4, 0x4d);
					} while (is_in_word_array(pos, g_sfight_blocked) != -1);
					sfight_enemy_move(f, &f->seq, pos);
				}
				f->top_enemy = 0;
			}
		}
	} else
		e->dead = 1;
	if (g_quick_fight)
		game_idle();
}

void sfight_do_action(FIGHT_PTR f, struct enemy far *e)
{
	struct hero far *h;

	if (g_in_fight) {
		if (g_quick_fight)
			game_idle();
		Lst_init(g_fight_list);
		f->atk_twice = f->def_strikes = f->atk_hit = g_fig_target_killed = 0;
		if (e->action_id == FIG_ACTION_SPELL)
			sfight_special_attack(f, e);
		else {
			h = enemy_target_hero(e);
			if ((signed char)h->dead || !h->type)
				return;
			if (e->action_id == FIG_ACTION_MELEE_ATTACK)
				sfight_attack(f, e, h);
			else if (e->action_id == FIG_ACTION_RANGE_ATTACK)
				sfight_area_attack(f, e, h);
		}
		Lst_run(g_fight_list);
	}
}

void sfight_attack(FIGHT_PTR f, struct enemy far *e, struct hero far *h)
{
	signed short kind;
	struct hero far *h2;
	register signed short i;
	register signed short dam;

	kind = e->cur_spell;
	FIG_CLEAR_HITS(f);
	if (kind == 0) {
		dam = dice_code_roll(e->dam1);
		dam -= (signed char)(h->rs_bonus + h->rs_bonus2);
		if ((signed char)h->petrified)
			dam = 0;
		if (dam > 0) {
			sub_hero_le(h, dam);
			Lst_add(g_fight_list, 8, dam, 1, (struct hero far *)e, 2, h);
			if ((signed char)h->dead)
				g_fig_target_killed = 1;
		}
		f->action_anim = fig_enemy_attack_seq(f, e, &f->seq);
		do
			game_idle();
		while (!seq_done(f->action_anim));
		fig_stand_seq(f, e, &f->seq);
		fig_stand_pic(f, h, &f->seq);
	} else if (kind == 3) {
		f->action_anim = fig_enemy_attack_seq(f, e, &f->seq);
		do
			game_idle();
#if FIX_0106
		/* Fix #106: Angriff auf alle: Trefferliste mit den handlungsfaehigen Helden fuellen (war immer leer) */
		while (!seq_done(f->action_anim));
		for (i = 0; i <= 6; i++) {
			h2 = g_group->heroes[i];
			if (h2 != 0 && check_hero2(h2) && g_fig_nr_hits < 7) {
				g_fig_hit_heroes[g_fig_nr_hits] = h2;
				g_fig_nr_hits++;
			}
		}
		if (g_fig_nr_hits > 0) {
			for (i = 0; i < g_fig_nr_hits; i++) {
				h2 = g_fig_hit_heroes[i];
				dam = dice_code_roll(e->dam1);
#else
		while (!seq_done(f->action_anim));
		if (g_fig_nr_hits > 0) {
			for (i = 0; i < g_fig_nr_hits; i++) {
				h2 = g_fig_hit_heroes[i];
				dam = dice_code_roll(e->dam1);
#endif
				dam -= (signed char)(h2->rs_bonus + h2->rs_bonus2);
				if ((signed char)h2->petrified)
					dam = 0;
				g_fig_target_killed = 0;
				if (dam > 0) {
					sub_hero_le(h2, dam);
					Lst_add(g_fight_list, 8, dam, 1, (struct hero far *)e, 2, h2);
					if ((signed char)h2->dead)
						g_fig_target_killed = 1;
				}
				if (g_fig_target_killed)
					g_fig_hit_anims[i] = fig_hit_pic(f, h2, &f->seq);
				else
					g_fig_hit_anims[i] = fig_stand_pic(f, h2, &f->seq);
			}
			for (i = g_fig_nr_hits; i < 7; i++)
				g_fig_hit_anims[i] = g_fig_hit_anims[0];
			do
				game_idle();
			while (!seq_done(g_fig_hit_anims[0]) || !seq_done(g_fig_hit_anims[1]) || !seq_done(g_fig_hit_anims[2]) ||
				!seq_done(g_fig_hit_anims[3]) || !seq_done(g_fig_hit_anims[4]) || !seq_done(g_fig_hit_anims[5]) ||
				!seq_done(g_fig_hit_anims[6]));
		}
		fig_stand_seq(f, e, &f->seq);
		for (i = 0; i < g_fig_nr_hits; i++)
			fig_stand_pic(f, g_fig_hit_heroes[i], &f->seq);
	} else {
		dam = h->le;
		sub_hero_le(h, dam);
		Lst_add(g_fight_list, 8, dam, 1, (struct hero far *)e, 2, h);
		f->action_anim = fig_enemy_attack_seq(f, e, &f->seq);
		do
			game_idle();
		while (!seq_done(f->action_anim));
		fig_stand_seq(f, e, &f->seq);
	}
	fight_draw(f);
}

void sfight_area_attack(FIGHT_PTR f, struct enemy far *e, struct hero far *h)
{
	struct hero far *h2;
	register signed short i;
	register signed short dam;

	FIG_CLEAR_HITS(f);
	f->action_anim = sfight_seq_area_attack(f, &f->seq);
	f->anim_removed = 0;
	do
		game_idle();
	while (!f->anim_removed && !seq_done(f->action_anim));
	if (g_fig_nr_hits > 0) {
		for (i = 0; i < g_fig_nr_hits; i++) {
			h2 = g_fig_hit_heroes[i];
			dam = dice_code_roll(e->dam2);
			dam -= (signed char)(h2->rs_bonus + h2->rs_bonus2);
			if (get_item_pos(h2, 0x11e) != -1)
				dam /= 2;
			if (h2->fireban)
				dam /= 2;
			if ((signed char)h2->petrified)
				dam = 0;
			g_fig_target_killed = 0;
			if (dam > 0) {
				sub_hero_le(h2, dam);
				Lst_add(g_fight_list, 8, dam, 1, (struct hero far *)e, 2, h2);
				if ((signed char)h2->dead)
					g_fig_target_killed = 1;
			}
			if (g_fig_target_killed)
				g_fig_hit_anims[i] = fig_hit_pic(f, h2, &f->seq);
			else
				g_fig_hit_anims[i] = fig_stand_pic(f, h2, &f->seq);
		}
		for (i = g_fig_nr_hits; i < 7; i++)
			g_fig_hit_anims[i] = g_fig_hit_anims[0];
		do
			game_idle();
		while (!seq_done(g_fig_hit_anims[0]) || !seq_done(g_fig_hit_anims[1]) || !seq_done(g_fig_hit_anims[2]) ||
			!seq_done(g_fig_hit_anims[3]) || !seq_done(g_fig_hit_anims[4]) || !seq_done(g_fig_hit_anims[5]) ||
			!seq_done(g_fig_hit_anims[6]));
	}
	fig_stand_seq(f, e, &f->seq);
	for (i = 0; i < g_fig_nr_hits; i++)
		fig_stand_pic(f, g_fig_hit_heroes[i], &f->seq);
	fight_draw(f);
}

void sfight_special_attack(FIGHT_PTR f, struct enemy far *e)
{
	signed short spell;
	signed short i;
	struct hero far *h;
	signed short ok[7];
	char far *msg[7];
	struct sfight_spells spells = g_sfight_spells;

	spell = spells.sp[random_schweif(13) - 1];
	e->set_cur_spell(spell);
	FIG_CLEAR_HITS(f);
	for (i = 0; i < 7; i++) {
		ok[i] = 0;
		msg[i] = 0;
	}
	h = get_first_hero(g_group);
	do {
		if (check_hero(h) && random_schweif(100) < 50) {
			*g_text_output_buf = 0;
			g_fig_hit_heroes[g_fig_nr_hits] = h;
			enemy_set_target_hero(e, h);
			g_spell_no_sound = 1;
			ok[g_fig_nr_hits] = enemy_cast_spell(e, 0);
			if (ok[g_fig_nr_hits]) {
				msg[g_fig_nr_hits] = (char far *)mem_alloc(200);
				strcpy(msg[g_fig_nr_hits], g_text_output_buf);
			}
			g_fig_nr_hits++;
		}
		h = get_next_hero(g_group, h);
	} while (get_first_hero(g_group) != h);
	f->action_anim = sfight_seq_special(f, &f->seq);
	do
		game_idle();
	while (!seq_done(f->action_anim));
	sfight_special_picture(f);
	if (g_fig_nr_hits > 0) {
		for (i = 0; i < g_fig_nr_hits; i++) {
			if (ok[i]) {
				g_fig_target_killed = 0;
				if ((signed char)g_fig_hit_heroes[i]->dead)
					g_fig_target_killed = 1;
				g_fig_hit_anims[i] = fig_hero_spell_hit_seq(f, g_fig_hit_heroes[i], &f->seq, spell, 1);
			} else
				g_fig_hit_anims[i] = fig_stand_pic(f, g_fig_hit_heroes[i], &f->seq);
		}
		for (i = g_fig_nr_hits; i < 7; i++)
			g_fig_hit_anims[i] = g_fig_hit_anims[0];
		do
			game_idle();
		while (!seq_done(g_fig_hit_anims[0]) || !seq_done(g_fig_hit_anims[1]) || !seq_done(g_fig_hit_anims[2]) ||
			!seq_done(g_fig_hit_anims[3]) || !seq_done(g_fig_hit_anims[4]) || !seq_done(g_fig_hit_anims[5]) ||
			!seq_done(g_fig_hit_anims[6]));
	}
	fig_stand_seq(f, e, &f->seq);
	for (i = 0; i < g_fig_nr_hits; i++)
		fig_stand_pic(f, g_fig_hit_heroes[i], &f->seq);
	fight_draw(f);
	for (i = 0; i < g_fig_nr_hits; i++) {
		if (msg[i]) {
			dlg_message(g_dialog, msg[i]);
			mem_free(msg[i]);
		}
	}
}

void sfight_fig_pos(FIGHT_PTR f, signed short n, signed short far *x, signed short far *y)
{
	if (n == 0 || n == 1) {
		*x = g_sfight_part_x[n];
		*y = g_sfight_part_y[n];
	} else if (n == 2) {
		*x = g_sfight_grid_xy[g_sfight_pos].x;
		*y = g_sfight_grid_xy[g_sfight_pos].y;
	} else {
		*x = g_sfight_ofs_x;
		*y = g_sfight_ofs_y;
	}
}
