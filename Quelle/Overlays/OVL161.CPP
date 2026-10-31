/* OVL161 - Overlay O161 (Segment 0x228a in SCHWEIF.EXE): Kampfaktion eines Gegners (BE FIG_do_enemy_action, von
 * O128 gerufen), Gegenstueck zu O160 (dort Gegner gegen Gegner). fig_do_enemy_action(f, e): nur im Kampf
 * (g_in_fight); Merker atk_twice..atk_hit/g_fig_target_killed loeschen, Ziel ueber TARGET_KIND(e): Held
 * enemy_target_hero (tot/leer -> Ende; Schnellkampf: h->viewdir = fight_dir_enemy_straight) bzw. Gegner
 * enemy_target_enemy (beim Nahkampf ggf. Zweifeld-Ziel drehen: Schwanzfeld aus g_fig_tail_dxy, Kampffeld
 * g_fight_walkmap belegt durch lebende Helden/Gegner oder >= 50 -> turned_away; sonst Figuren umsetzen
 * fig_list_find/fig_list_find_twofield/fight_set_cell/fig_list_purge); Angreifer selbst zur Blickrichtung
 * fight_dir_enemy drehen; Anzeige vorbereiten (Vorlage tmpl, fight_find_cell/fig_list_add, fight_draw); dann je
 * nach e->action_id: 2 Nahkampf (fig_enemy_attack_hero bzw. fig_enemy_attack_enemy(.., turned_away)), 15 Fern-
 * angriff (fig_enemy_shoot_hero bzw. fig_enemy_shoot_enemy), 4 Zauber (fig_enemy_spell_hero bzw.
 * fig_enemy_spell_enemy); Abschluss Lst_run, fight_restore_bg/fight_add_rect/fight_draw_figures/fight_draw_flush.
 * fig_enemy_attack_hero(f, e, h): Nahkampf gegen einen Helden - AT/PA des Helden (hero_weapon_type == -1 ->
 * hero_at_weapon/hero_pa_weapon, sonst hero_at/hero_pa; Sichern -> PA + 3; group_mr_bonus == -5 -> je -4),
 * Gegner-AT enemy_attack_value (+2 bei schon pariert, -5 bei Chamaelioni, /2 bzw. /3 nach duplicatus, +2 im
 * Schnellkampf); Patzer des Gegners (W20 = 20 und Bestaetigung > AT - 9, nur bei check_hero(h)) mit Tabelle
 * random_interval(2, 12); Parade/Patzer des Helden, Treffer enemy_attack_damage -> sub_hero_le, Tierbiss
 * (is_animal) -> hero_disease_test(h, ILLNESS_TOLLWUT, 1); gleiche Wuerfe bei Parade: Bruchfaktor der Waffe
 * (Bruch set_item_broken bei W12 + BF > 15, sonst BF + 1 bis 20); Animation fig_enemy_attack_seq/
 * fig_hero_defend_seq, fight_msg_names. fig_enemy_shoot_hero: Fernangriff enemy_range_damage, Animation
 * fig_enemy_shoot_seq/fig_hit_pic. fig_enemy_spell_hero: Zauber enemy_cast_spell, Text nach f->msg, Klang
 * g_mon_spell_descriptions[cur_spell].snd, Animation fig_enemy_spell_seq/fig_hero_spell_hit_seq.
 *
 * BCC -mlarge -3 -Ot -Og -Oi -c -Y -E..\bc\BIN\TASM.EXE OVL161.CPP -> 4806/4806 Bytes GLEICH (18.10.2026),
 *   cobjfix laeuft. Ohne -Og 368 (Epilogkopien), ohne -Oi 3270 Abweichungen (strcpy-Intrinsic).
 *   (Namensfassung mit schweif.h, Welle N11: 19.10.2026 GLEICH, cobjfix LINKGLEICH)
 * - fig_do_enemy_action: "register" wirkt hier: turned_away/nx2 als register-Variablen (SI/DI) -> ny2 bleibt
 *   [bp-1ch]; ohne register bekommt ny2 DI und nx2 einen Platz am Ende (56 Bytes).
 * - Zuweisungskette "f->atk_twice = f->def_strikes = f->atk_hit = g_fig_target_killed = 0" (mov al,0, Stores
 *   ds:c688, +1f2, +1f1, +1f0).
 * - Zeigerfelder bottom_hero/bottom_enemy/tmpl.anim/tmpl.gfx als long (Makros *_L: mov dword 0, mov eax).
 * - inline-Setter h->set_viewdir()/e->set_viewdir() -> Parameterkopien [bp-25h]..[bp-27h] vor den -Og-
 *   Zwischenwerten [bp-2ah]/[bp-2ch]/[bp-2eh] ((int)t->viewdir, viewdir * 4).
 * - "(signed short)bf > 3" -> cbw / cmp ax,3; Bruchfaktor-Test einmal als inline is_weapon_breakable() (mov ax,1/
 *   xor ax,ax), zweimal als Ausdruck "(bf == -99 ? 0 : 1)" (mov al,0/mov al,1/or al,al), "x++; else ;" ergibt
 *   den toten jmp short (ohne "else ;" 393 Bytes Abweichung).
 * - Belegungstest "!(signed char)g_heroes[obj - 1].dead" -> and ax,1 / cbw / or ax,ax; &g_heroes[obj - 1] -> imul,
 *   Basis, -0x5ef in der Verschiebung. Bits sonst im Original unsigned char: (unsigned char)-Cast bei Tests.
 * - enemy_id unsigned gelesen ((unsigned char)-Cast in TARGET_KIND).
 * Originalfehler/Auffaelligkeiten:
 * - Der Patzerzweig ist hier richtig geklammert (else if (k >= 9 && k <= 11) haengt an "k >= 3 && k <= 8"),
 *   anders als in O160 fig_enemy_attack_enemy; Patzer 12 bleibt wie dort ein leerer Zweig.
 * - Patzer des Gegners werden nur ausgewertet, wenn check_hero(h) gilt (Zustand des Helden statt des Gegners).
 * - fig_do_enemy_action: "!turned_away" in der Bedingung vor dem Verschieben ist immer wahr (dort noch 0).
 */
#include <string.h>
#include "schweif.h"
#include "s_plog.h"	/* Probenlog (FIX_PROBENLOG) */

#define MSG_ENEMY	1		/* Lst_add*: Objekt ist ein Gegner */
#define MSG_HERO	2		/* Lst_add*: Objekt ist ein Held */
#define MON_SPELL_FLASH	7		/* Gegnerzauber mit Aufblitzen (screen_flash) */
#define SND_MON_SPELL_DEFAULT 0x92	/* Klang, wenn der Zauberklang fehlt */
#define HERO(e)		((struct hero far *)(e))	/* Lst_add/Lst_add2 nehmen Helden- oder Gegnerzeiger */

/* Zielart des Gegners: 1 = Gegner (TARGET_ENEMY), 2 = Held (TARGET_HERO), 5 = keins */
#define TARGET_KIND(e) ((signed char)((unsigned char)(e)->enemy_id < TARGET_ID_ENEMY ? \
	((unsigned char)(e)->enemy_id == 0 ? TARGET_SELF : TARGET_HERO) : TARGET_ENEMY))

#define WEAPON_BF(h)	((h)->inv[INV_SLOT_RIGHT_HAND].bf)

/* Zeigerfelder als long beschreiben (Original: mov dword / mov eax) */
#define BOTTOM_HERO_L	(*(unsigned long far *)&f->bottom_hero)
#define BOTTOM_ENEMY_L	(*(unsigned long far *)&f->bottom_enemy)
#define TMPL_ANIM_L	(*(signed long far *)&f->tmpl.anim)
#define TMPL_GFX_L	(*(unsigned long far *)&f->tmpl.gfx)
#define PIC_CURSOR_L	(*(unsigned long far *)&f->pic_cursor)

void fig_do_enemy_action(Fight far *f, struct enemy far *e)
{
	struct hero far *h;
	struct enemy far *t;
	signed char is_hero;
	signed short obj;
	signed short ex;
	signed short ey;
	signed short dir;
	signed short x;
	signed short y;
	signed short nx;
	signed short ny;
	signed short ny2;
	struct fig_node far *fig;
	struct fig_node far *tail;
	register signed short turned_away;
	register signed short nx2;

	turned_away = 0;
	if (g_in_fight != 0) {
		if (g_quick_fight)
			game_idle();
		Lst_init(g_fight_list);
		f->atk_twice = f->def_strikes = f->atk_hit = g_fig_target_killed = 0;
		if (TARGET_KIND(e) == TARGET_HERO) {
			h = enemy_target_hero(e);
			if ((unsigned char)h->dead || !h->type)
				return;
			if (g_quick_fight && !h->parried)
				h->set_viewdir(fight_dir_enemy_straight(f, e));
			is_hero = 1;
		} else {
			t = enemy_target_enemy(e);
			if ((unsigned char)t->dead || !t->mon_id)
				return;
			is_hero = 0;
			if (enemy_is_twofielded(t) && !turned_away && !t->parried && e->action_id == FIG_ACTION_MELEE_ATTACK) {
				/* Zweifeld-Ziel zum Angreifer drehen, wenn das neue Schwanzfeld frei ist */
				fight_find_cell(fight_obj_index(t) + FIG_ID_ENEMY, &x, &y);
				dir = fight_dir_enemy_straight(f, e);
				if (t->viewdir != dir) {
					nx = x + g_fig_tail_dxy[dir].dx;
					ny = y + g_fig_tail_dxy[dir].dy;
					obj = *(g_fight_walkmap + ny * FIG_MAP_PITCH + nx);
					if (obj >= 50 ||
						(obj < FIG_ID_ENEMY && obj >= FIG_ID_HERO && !(signed char)g_heroes[obj - FIG_ID_HERO].dead) ||
						(obj >= FIG_ID_ENEMY && obj < FIG_ID_ENEMY_TAIL && !(signed char)g_enemies[obj - FIG_ID_ENEMY].dead) ||
						(obj >= FIG_ID_ENEMY_TAIL && obj < 50 && !(signed char)g_enemies[obj - FIG_ID_ENEMY_TAIL].dead))
						turned_away = 1;
					else {
						nx2 = x + g_fig_tail_dxy[t->viewdir].dx;
						ny2 = y + g_fig_tail_dxy[t->viewdir].dy;
						fig = fig_list_find(&f->list, t->fighter_id);
						tail = fig_list_find_twofield(&f->list, fig->twofielded + 20);
						fight_set_cell(f, nx2, ny2, tail->obj_id);
						fight_set_cell(f, nx, ny, fight_obj_index(t) + FIG_ID_ENEMY_TAIL);
						tail->obj_id = obj;
						tail->cbx = nx;
						tail->cby = ny;
						tail->to_resort = 1;
						fig->to_resort = 1;
						fig_list_purge(&f->list);
					}
				}
			}
			if (!turned_away && !t->parried && e->action_id == FIG_ACTION_MELEE_ATTACK && g_quick_fight)
				t->set_viewdir(fight_dir_enemy_straight(f, e));
		}
		/* Angreifer (ggf. mit Schwanzfeld) zum Ziel drehen */
		dir = fight_dir_enemy(f, e);
		if (enemy_is_twofielded(e) && e->viewdir != dir) {
			fight_find_cell(fight_obj_index(e) + FIG_ID_ENEMY, &ex, &ey);
			nx = ex + g_fig_tail_dxy[dir].dx;
			ny = ey + g_fig_tail_dxy[dir].dy;
			obj = *(g_fight_walkmap + ny * FIG_MAP_PITCH + nx);
			nx2 = ex + g_fig_tail_dxy[e->viewdir].dx;
			ny2 = ey + g_fig_tail_dxy[e->viewdir].dy;
			fig = fig_list_find(&f->list, e->fighter_id);
			tail = fig_list_find_twofield(&f->list, fig->twofielded + 20);
			fight_set_cell(f, nx2, ny2, tail->obj_id);
			fight_set_cell(f, nx, ny, fight_obj_index(e) + FIG_ID_ENEMY_TAIL);
			tail->obj_id = obj;
			tail->cbx = nx;
			tail->cby = ny;
			tail->to_resort = 1;
			fig->to_resort = 1;
			fig_list_purge(&f->list);
		}
		if (g_quick_fight)
			e->set_viewdir(dir);
		if (!g_quick_fight) {
			/* Ziel anzeigen und mit dem Feldcursor markieren */
			BOTTOM_ENEMY_L = 0;
			BOTTOM_HERO_L = 0;
			g_fig_redraw_bottom = 1;
			if (f->mark_id2 != -1) {
				fig_list_remove(&f->list, f->mark_id2, 0);
				f->mark_id2 = -1;
			}
			if (is_hero) {
				fight_find_cell(get_hero_index(h) + FIG_ID_HERO, &x, &y);
				BOTTOM_HERO_L = (unsigned long)h;
			} else {
				fight_find_cell(fight_obj_index(t) + FIG_ID_ENEMY, &x, &y);
				BOTTOM_ENEMY_L = (unsigned long)t;
			}
			f->tmpl.obj_pic = 0;
			f->tmpl.cbx = x;
			f->tmpl.cby = y;
			f->tmpl.offsetx = 0;
			f->tmpl.offsety = 6;
			f->tmpl.kind = FIG_KIND_OBJ;
			TMPL_ANIM_L = 0;
			TMPL_GFX_L = PIC_CURSOR_L;
			f->tmpl.z = 2;
			f->tmpl.visible = 1;
			f->tmpl.twofielded = FIG_TWOFIELD_NONE;
			f->mark_id2 = fig_list_add(&f->list, -1);
			fight_draw(f);
			BOTTOM_ENEMY_L = 0;
			BOTTOM_HERO_L = 0;
		}
		if (e->action_id == FIG_ACTION_MELEE_ATTACK) {
			if (is_hero)
				fig_enemy_attack_hero(f, e, h);
			else
				fig_enemy_attack_enemy(f, e, t, turned_away);
		} else if (e->action_id == FIG_ACTION_RANGE_ATTACK) {
			if (is_hero)
				fig_enemy_shoot_hero(f, e, h);
			else
				fig_enemy_shoot_enemy(f, e, t);
		} else if (e->action_id == FIG_ACTION_SPELL) {
			if (is_hero)
				fig_enemy_spell_hero(f, e, h);
			else
				fig_enemy_spell_enemy(f, e, t);
		}
		Lst_run(g_fight_list);
		if (!g_quick_fight) {
			if (f->mark_id2 != -1) {
				fig_list_remove(&f->list, f->mark_id2, 0);
				f->mark_id2 = -1;
			}
			BOTTOM_ENEMY_L = 0;
			BOTTOM_HERO_L = 0;
			g_fig_redraw_bottom = 1;
			fight_restore_bg(f);
			fight_add_rect(0, 0x91, 100, 0xc7);
			fight_draw_figures(f);
			fight_draw_flush(f);
		}
	}
}

/* Nahkampf Gegner gegen Held */
void fig_enemy_attack_hero(Fight far *f, struct enemy far *e, struct hero far *h)
{
	signed short k;
	signed short wtype;
	signed short hit;
	signed short r1;
	signed short r2;
	signed short at;
	signed short pa;
	struct hero far *t1;
	struct hero far *t2;
	signed short dmg;
	signed short e_at;

	hit = 0;
	if (g_quick_fight)
		game_idle();
	wtype = hero_weapon_type(h);
	if (wtype == -1) {
		at = hero_at_weapon(h, 0);
		pa = hero_pa_weapon(h, 0);
	} else {
		at = hero_at(h);
		pa = hero_pa(h);
	}
	if (h->action_id == FIG_ACTION_GUARD)
		pa += 3;
	if (group_mr_bonus(g_group) == -5) {
		at -= 4;
		pa -= 4;
	}
	e_at = enemy_attack_value(e);
	t1 = enemy_target_hero(e);
	if (t1->parried == 1)
		e_at += 2;
	if ((unsigned char)h->chamaelioni == 1)
		e_at -= 5;
	if (h->duplicatus == 1)
		e_at /= 2;
	if (h->duplicatus == 2)
		e_at /= 3;
	if (g_quick_fight)
		e_at += 2;
	r1 = random_schweif(20);
	/* Probenlog: Zeile Art 6 (Gegner attackiert Helden: AT, W20, Gegnerart, 1) */
#if FIX_PROBENLOG
	{
		signed short w[4];

		w[0] = e_at; w[1] = r1; w[2] = e->mon_id; w[3] = 1;
		probenlog(6, h->name, 4, w);
	}
#endif
	if (r1 == 20 && random_schweif(20) > e_at - 9) {
		/* Angriffspatzer des Gegners */
		if (check_hero(h)) {
			Lst_add2(g_fight_list, 3, 0, MSG_ENEMY, HERO(e));
			k = random_interval(2, 12);
			if (k >= 3 && k <= 8) {
				f->def_strikes = 1;
				if (random_schweif(20) <= at) {
					dmg = hero_attack_damage(h, e, 0);
					if (dmg > 0) {
						Lst_add(g_fight_list, 0x0b, dmg, MSG_HERO, h, MSG_ENEMY, HERO(e));
						enemy_sub_le(e, dmg, 1);
						if ((unsigned char)e->dead)
							f->atk_hit = 1;
					}
				}
			} else if (k >= 9 && k <= 11) {
				dmg = random_schweif(6);
				Lst_add2(g_fight_list, 0x0f, dmg, MSG_ENEMY, HERO(e));
				enemy_sub_le(e, dmg, 1);
				if ((unsigned char)e->dead)
					f->atk_hit = 1;
			} else if (k == 12) {
			}
		}
	} else if (r1 > e_at) {
		Lst_add0(g_fight_list, 5, 0);
	} else {
		t2 = enemy_target_hero(e);
		if (!t2->parried && check_hero(h)) {
			/* Parade des Helden */
			r2 = random_schweif(20);
			/* Probenlog: Zeile Art 7 (Held pariert gegen den Gegner: PA, W20, Gegnerart, 1) */
#if FIX_PROBENLOG
			{
				signed short w[4];

				w[0] = pa; w[1] = r2; w[2] = e->mon_id; w[3] = 1;
				probenlog(7, h->name, 4, w);
			}
#endif
			if (r2 == 20 && random_schweif(20) > pa - 7) {
				/* Paradepatzer */
				hit = 1;
				Lst_add2(g_fight_list, 2, 0, MSG_HERO, h);
				k = random_interval(2, 12);
				if (k >= 3 && k <= 8) {
					f->atk_twice = 1;
					if (random_schweif(20) <= e_at) {
						dmg = enemy_attack_damage(e, h, 0);
						if (dmg > 0) {
							Lst_add(g_fight_list, 8, dmg, MSG_ENEMY, HERO(e), MSG_HERO, h);
							sub_hero_le(h, dmg);
							if ((unsigned char)h->dead)
								g_fig_target_killed = 1;
							else if (e->is_animal)
								hero_disease_test(h, ILLNESS_TOLLWUT, 1);
						}
					}
				} else if (k >= 9 && k <= 11) {
					dmg = random_schweif(6);
					Lst_add2(g_fight_list, 0x0e, dmg, MSG_HERO, h);
					sub_hero_le(h, dmg);
					if ((unsigned char)h->dead)
						g_fig_target_killed = 1;
				}
			} else if (r2 > pa) {
				hit = 1;
			} else {
				/* pariert; gleiche Wuerfe: Bruchprobe der Waffe */
				Lst_add2(g_fight_list, 9, 0, MSG_HERO, h);
				if (r2 == r1 && h->is_weapon_breakable()) {
					if ((signed short)WEAPON_BF(h) > 3) {
						if (random_schweif(12) + WEAPON_BF(h) > 15) {
							if (set_item_broken(h, INV_SLOT_RIGHT_HAND, 1))
								Lst_add2(g_fight_list, 6, 0, MSG_HERO, h);
						} else if ((WEAPON_BF(h) == -99 ? 0 : 1) && WEAPON_BF(h) < 20)
							WEAPON_BF(h)++;
						else ;
					} else if ((WEAPON_BF(h) == -99 ? 0 : 1) && WEAPON_BF(h) < 20)
						WEAPON_BF(h)++;
					else ;
				}
			}
		} else
			hit = 1;
		if (hit) {
			dmg = enemy_attack_damage(e, h, 0);
			if (dmg > 0) {
				Lst_add(g_fight_list, 8, dmg, MSG_ENEMY, HERO(e), MSG_HERO, h);
				sub_hero_le(h, dmg);
				if ((unsigned char)h->dead)
					g_fig_target_killed = 1;
				else if (e->is_animal)
					hero_disease_test(h, ILLNESS_TOLLWUT, 1);
			} else
				Lst_add2(g_fight_list, 0x16, 0, MSG_HERO, h);
		}
	}
	if (!g_quick_fight) {
		f->action_anim = fig_enemy_attack_seq(f, e, &f->seq);
		f->action_anim2 = fig_hero_defend_seq(f, h, &f->seq, fight_dir_enemy(f, e));
		do
			game_idle();
		while (!seq_done(f->action_anim) || !seq_done(f->action_anim2));
		fig_stand_seq(f, e, &f->seq);
		fig_stand_pic(f, h, &f->seq);
		fight_draw(f);
		fight_msg_names(f, h, 0);
	} else
		game_idle();
}

/* Fernangriff Gegner gegen Held */
void fig_enemy_shoot_hero(Fight far *f, struct enemy far *e, struct hero far *h)
{
	signed short dmg;

	dmg = enemy_range_damage(e, h);
	if (dmg > 0) {
		Lst_add(g_fight_list, 8, dmg, MSG_ENEMY, HERO(e), MSG_HERO, h);
		sub_hero_le(h, dmg);
		if ((unsigned char)h->dead)
			g_fig_target_killed = 1;
	} else
		Lst_add2(g_fight_list, 0x16, 0, MSG_HERO, h);
	if (!g_quick_fight) {
		f->action_anim = fig_enemy_shoot_seq(f, e, &f->seq);
		f->anim_removed = 0;
		do
			game_idle();
		while (!f->anim_removed && !seq_done(f->action_anim));
		if (g_fig_target_killed) {
			f->action_anim = fig_hit_pic(f, h, &f->seq);
			do
				game_idle();
			while (!seq_done(f->action_anim));
		}
		fig_stand_seq(f, e, &f->seq);
		fig_stand_pic(f, h, &f->seq);
		fight_draw(f);
		fight_msg_names(f, h, 0);
	} else
		game_idle();
}

/* Zauber Gegner auf Held */
void fig_enemy_spell_hero(Fight far *f, struct enemy far *e, struct hero far *h)
{
	signed short sp;

	*g_text_output_buf = 0;
	g_spell_no_sound = 1;
	sp = e->cur_spell;
	enemy_cast_spell(e, 0);
	strcpy(f->msg, g_text_output_buf);
	if (!g_quick_fight) {
		if (!g_spell_cancelled) {
			f->action_anim = fig_enemy_spell_seq(f, e, &f->seq);
			f->anim_removed = 0;
			do
				game_idle();
			while (!f->anim_removed && !seq_done(f->action_anim));
			if (arc_exists(g_arc_fx, g_mon_spell_descriptions[sp].snd))
				snd_play2(g_snd_fx, g_mon_spell_descriptions[sp].snd);
			else
				snd_play2(g_snd_fx, SND_MON_SPELL_DEFAULT);
			if (g_spell_success) {
				f->action_anim = fig_hero_spell_hit_seq(f, h, &f->seq, sp, 1);
				f->anim_removed = 0;
				if (sp == MON_SPELL_FLASH)
					screen_flash();
				do
					game_idle();
				while (!f->anim_removed && !seq_done(f->action_anim));
			}
			fig_stand_seq(f, e, &f->seq);
			fig_stand_pic(f, h, &f->seq);
			fight_draw(f);
		}
		if (all_heroes_down(g_group))
			fight_status_text(f->msg);
		else
			dlg_message(g_dialog, f->msg);
		fight_msg_names(f, h, 0);
	} else
		game_idle();
}
