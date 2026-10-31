/* OVL159 - Overlay O159 (Segment 0x2283 in SCHWEIF.EXE): Kampf - Aktion eines Helden (BE FIG_do_hero_action;
 * Gegenstueck O158 fuer Helden als Ziel). fig_do_hero_action(f, h) (Aufruf aus O128): Ziel nach target_type (1/4
 * Gegner enemy_mr_target, 2 Held hero_mr_target, 5 Held selbst); ein Zweifeldgegner wird ggf. zum Angreifer gedreht
 * (Schwanzfeld g_fig_tail_dxy auf dem Kampffeld g_fight_walkmap; belegt durch lebende Helden/Gegner oder >= 50
 * -> nicht drehen, sonst Figuren umsetzen: fight_set_cell, fig_list_purge), Anzeige unten (bottom_hero/
 * bottom_enemy), Feldcursor mark_id2 aus der Vorlage tmpl (pic_cursor), dann je action_id (2 Nahkampf, 15
 * Fernkampf, 4 Zauber, 5 Gegenstand) O158/O162 bzw. fig_hero_attack_enemy/fig_hero_shoot_enemy/
 * fig_hero_spell_enemy/fig_use_item_enemy, danach Lst_run, fight_restore_bg/fight_add_rect/fight_draw_figures/
 * fight_draw_flush. fig_hero_attack_enemy(f, h, e, turned_away): Nahkampf gegen einen Gegner (wie
 * fig_hero_attack_hero: Patzer 2W6, Parade enemy_parade_value, zahm/abtruennig -> PA 0, Bruchfaktor, Schaden
 * hero_attack_damage mit Kritisch bei Wurf < Runde / 2 + 2, enemy_sub_le), Animation fig_hero_attack_seq/
 * fig_enemy_defend_seq/fig_enemy_hit_seq. fig_hero_shoot_enemy: Fernkampf (hand_weapon_class,
 * range_attack_check_ammo, fig_hero_shoot_seq/fig_enemy_hit_seq). fig_hero_spell_enemy: Zauber (wie
 * fig_hero_spell_hero, Ziel-Animation fig_enemy_spell_hit_seq/fig_stand_seq).
 *
 * BCC -mlarge -3 -Ot -Og -Oi -c -Y OVL159.CPP   -> 5357/5357 Bytes GLEICH (18.10.2026), cobjfix laeuft.
 *   -Oi fuer strcpy/memset in fig_hero_spell_enemy; ohne -Og 5 Bytes zu lang, ohne -Oi 22 Bytes zu kurz.
 *   (Namensfassung mit schweif.h, Welle N11: 19.10.2026 GLEICH, cobjfix LINKGLEICH)
 * - fig_do_hero_action: "register is_hero = 0; register dir;" (SI/DI) vor "turned_away = 0"; Setter
 *   e->set_viewdir()/h->set_viewdir() als inline-Elementfunktionen (Parameterkopien [bp-25h..27h]); e->viewdir * 4
 *   als -Og-Zwischenwert [bp-2ah], Bitbyte t->dead als [bp-2bh]; "if (is_hero || (!is_hero && !enemy_type_check(e)))"
 *   (doppelter Test im Original).
 * - Anzeige- und Vorlagenfelder als long (Makros *_L): "= 0" -> mov dword, "= (unsigned long)t" -> mov eax.
 * - Belegungstest "!(signed char)g_enemies[c - 10].dead" -> and ax,1 / cbw / or ax,ax, Basis -0x2a2 gefaltet.
 * - Bits von Held und Gegner im Original unsigned char: bei blossen Tests (unsigned char)-Cast.
 * - fig_hero_attack_enemy: Block-Local "wait" [bp-18h] hinter dem -Og-Zwischenwert [bp-15h]; "f->round / 2" ->
 *   cwd/sub/sar.
 * Originalfehler/Auffaelliges: fig_do_hero_action - bei Zieltyp ausser 1/2/4/5 bleiben e/t uninitialisiert
 *   (fight_obj_index(e), bottom_enemy = e, fig_hero_attack_enemy(.., e, ..)); fig_hero_attack_enemy setzt
 *   f->atk_hit beim Patzer 3..8 auch ohne Schaden (wie O158).
 */
#include <stdio.h>
#include <string.h>
#include <mem.h>
#include "schweif.h"
#include "s_plog.h"	/* Probenlog (FIX_PROBENLOG) */

#define WEAPON_BF(h)	((h)->inv[INV_SLOT_RIGHT_HAND].bf)
#define BF_UP(h) if ((WEAPON_BF(h) == -99 ? 0 : 1) && WEAPON_BF(h) < 20) WEAPON_BF(h)++; else ;
#define MSG_ENEMY	1		/* Lst_add*: Objekt ist ein Gegner */
#define MSG_HERO	2		/* Lst_add*: Objekt ist ein Held */
#define TTX_SPELL_NAMES	0x85		/* g_ttx: Zaubernamen */
#define SND_SPELL_DEFAULT 0x97		/* Klang, wenn der Zauberklang fehlt */

/* Zeigerfelder als long beschreiben (Original: mov dword / mov eax) */
#define BOTTOM_HERO_L	(*(unsigned long far *)&f->bottom_hero)
#define BOTTOM_ENEMY_L	(*(unsigned long far *)&f->bottom_enemy)
#define TMPL_ANIM_L	(*(signed long far *)&f->tmpl.anim)
#define TMPL_GFX_L	(*(unsigned long far *)&f->tmpl.gfx)
#define PIC_CURSOR_L	(*(unsigned long far *)&f->pic_cursor)

void fig_do_hero_action(Fight far *f, struct hero far *h)
{
	register signed short is_hero = 0;
	register signed short dir;
	struct enemy far *e;
	struct hero far *t;
	struct fig_node far *fig_t;
	struct fig_node far *fig;
	struct fig_node far *tail;
	signed short turned_away = 0;
	signed short c;
	signed short x;
	signed short y;
	signed short x2;
	signed short y2;
	signed short px;
	signed short py;

	if (g_in_fight != 0) {
		if (g_quick_fight)
			game_idle();
		Lst_init(g_fight_list);
		f->atk_twice = f->def_strikes = g_fig_action_flag = f->atk_hit = g_fig_target_killed = 0;
		if (target_type(h) == TARGET_ENEMY || target_type(h) == TARGET_SPECIAL) {
			if (target_type(h) == TARGET_SPECIAL)
				turned_away = 1;
			e = enemy_mr_target(h);
			e->asleep = 0;
			if (!e->mon_id || ((unsigned char)e->dead &&
			    (h->action_id != FIG_ACTION_SPELL || h->spell_id != SP_SKELETTARIUS)))
				return;
			if (enemy_is_twofielded(e) && !turned_away && !e->parried && h->action_id == FIG_ACTION_MELEE_ATTACK) {
				/* Zweifeldgegner zum Angreifer drehen, wenn das neue Schwanzfeld frei ist */
				dir = fight_dir_hero_straight(f, h);
				if (e->viewdir != dir) {
					fight_find_cell(fight_obj_index(e) + FIG_ID_ENEMY, &px, &py);
					x = px + g_fig_tail_dxy[dir].dx;
					y = py + g_fig_tail_dxy[dir].dy;
					c = *(g_fight_walkmap + y * FIG_MAP_PITCH + x);
					if (c >= 50 ||
					    (c < FIG_ID_ENEMY && c >= FIG_ID_HERO && !(signed char)g_heroes[c - FIG_ID_HERO].dead) ||
					    (c >= FIG_ID_ENEMY && c < FIG_ID_ENEMY_TAIL && !(signed char)g_enemies[c - FIG_ID_ENEMY].dead) ||
					    (c >= FIG_ID_ENEMY_TAIL && c < 50 && !(signed char)g_enemies[c - FIG_ID_ENEMY_TAIL].dead)) {
						turned_away = 1;
					} else {
						x2 = px + g_fig_tail_dxy[e->viewdir].dx;
						y2 = py + g_fig_tail_dxy[e->viewdir].dy;
						fig = fig_list_find(&f->list, e->fighter_id);
						tail = fig_list_find_twofield(&f->list, fig->twofielded + 20);
						fight_set_cell(f, x2, y2, tail->obj_id);
						fight_set_cell(f, x, y, fight_obj_index(e) + FIG_ID_ENEMY_TAIL);
						tail->obj_id = c;
						tail->cbx = x;
						tail->cby = y;
						tail->to_resort = 1;
						fig->to_resort = 1;
						fig_list_purge(&f->list);
					}
				}
			}
			if (turned_away == 0 && !e->parried && h->action_id == FIG_ACTION_MELEE_ATTACK && g_quick_fight)
				e->set_viewdir(fight_dir_hero_straight(f, h));
		} else if (target_type(h) == TARGET_HERO) {
			t = hero_mr_target(h);
			if ((unsigned char)t->dead || !t->type)
				return;
			if ((unsigned char)t->asleep) {
				/* Ziel wacht auf */
				set_asleep(t, 0);
				fig_t = fig_list_find(&f->list, t->fighter_id);
				fig_stand_pic(f, t, &f->seq);
				seq_set(fig_t->anim, &f->seq);
			}
			if (g_quick_fight && !t->parried)
				t->set_viewdir(fight_dir_hero_straight(f, h));
			is_hero = 1;
#if FIX_0147
		/* Fix #147: unbekannte Zielart: Aktion abbrechen statt Muellzeiger e/t */
		} else if (target_type(h) == TARGET_SELF) {
			t = h;
			is_hero = 1;
		} else
			return;
#else
		} else if (target_type(h) == TARGET_SELF) {
			t = h;
			is_hero = 1;
		}
#endif
		if (g_quick_fight)
			h->set_viewdir(fight_dir_hero(f, h));
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
				fight_find_cell(get_hero_index(t) + FIG_ID_HERO, &px, &py);
				BOTTOM_HERO_L = (unsigned long)t;
			} else {
				fight_find_cell(fight_obj_index(e) + FIG_ID_ENEMY, &px, &py);
				BOTTOM_ENEMY_L = (unsigned long)e;
			}
			if (is_hero || (!is_hero && !enemy_type_check(e))) {
				f->tmpl.obj_pic = 0;
				f->tmpl.cbx = px;
				f->tmpl.cby = py;
				f->tmpl.offsetx = 0;
				f->tmpl.offsety = 6;
				f->tmpl.kind = FIG_KIND_OBJ;
				TMPL_ANIM_L = 0;
				TMPL_GFX_L = PIC_CURSOR_L;
				f->tmpl.z = 2;
				f->tmpl.visible = 1;
				f->tmpl.twofielded = FIG_TWOFIELD_NONE;
				f->mark_id2 = fig_list_add(&f->list, -1);
			}
			fight_draw(f);
			BOTTOM_ENEMY_L = 0;
			BOTTOM_HERO_L = 0;
		}
		if (h->action_id == FIG_ACTION_MELEE_ATTACK) {
			if (is_hero)
				fig_hero_attack_hero(f, h, t);
			else
				fig_hero_attack_enemy(f, h, e, turned_away);
		} else if (h->action_id == FIG_ACTION_RANGE_ATTACK) {
			if (is_hero)
				fig_hero_shoot_hero(f, h, t);
			else
				fig_hero_shoot_enemy(f, h, e);
		} else if (h->action_id == FIG_ACTION_SPELL) {
			if (is_hero)
				fig_hero_spell_hero(f, h, t);
			else
				fig_hero_spell_enemy(f, h, e);
		} else if (h->action_id == FIG_ACTION_USE_ITEM) {
			if (is_hero)
				fig_use_item_hero(f, h, t);
			else
				fig_use_item_enemy(f, h, e);
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
			fight_add_rect(0, 0x91, 100, 199);
			fight_draw_figures(f);
			fight_draw_flush(f);
		}
	}
}

/* Nahkampf Held gegen Gegner; turned_away: Gegner konnte sich nicht zum Angreifer drehen (keine Parade, AT +2) */
void fig_hero_attack_enemy(Fight far *f, struct hero far *h, struct enemy far *e, signed char turned_away)
{
	signed short wtype;
	signed short crit;
	signed short hit;
	signed short roll1;
	signed short roll2;
	signed short pa;
	struct enemy far *p1;
	struct enemy far *p2;
	register signed short dmg;
	register signed short at;

	hit = 0;
	if (g_quick_fight)
		game_idle();
	wtype = hero_weapon_type(h);
	if (wtype == -1)
		at = hero_at_weapon(h, 0);
	else
		at = hero_at(h);
	if (group_mr_bonus(g_group) == -5)
		at -= 4;
	p1 = enemy_mr_target(h);
	if (p1->parried == 1 || turned_away != 0)
		at += 2;
	at += 4;
	if (g_quick_fight)
		at += 2;
	roll1 = random_schweif(20);
	/* Probenlog: Zeile Art 6 (Held attackiert Gegner: AT, W20, Gegnerart, 0) */
#if FIX_PROBENLOG
	{
		signed short w[4];

		w[0] = at; w[1] = roll1; w[2] = e->mon_id; w[3] = 0;
		probenlog(6, h->name, 4, w);
	}
#endif
	if (roll1 == 20 && random_schweif(20) > at - 9) {
		/* Angriffspatzer */
		dmg = 0;
		Lst_add2(g_fight_list, 1, 0, MSG_HERO, h);
		crit = random_interval(2, 12);
		if (crit == 2 && wtype != -1) {
			if (set_item_broken(h, INV_SLOT_RIGHT_HAND, 1))
				Lst_add0(g_fight_list, 6, 0);
		} else if (crit >= 3 && crit <= 8 && !turned_away) {
			f->def_strikes = 1;
			if (random_schweif(20) <= enemy_attack_value(e))
				dmg = enemy_attack_damage(e, h, 0);
#if FIX_0147
			/* Fix #147: Folge nur nach Schaden (wie O158) */
			if (dmg > 0) {
				Lst_add(g_fight_list, 8, dmg, MSG_ENEMY, (struct hero far *)e, MSG_HERO, h);
				sub_hero_le(h, dmg);
				if ((unsigned char)h->dead)
					f->atk_hit = 1;
			}
#else
			if (dmg > 0) {
				Lst_add(g_fight_list, 8, dmg, MSG_ENEMY, (struct hero far *)e, MSG_HERO, h);
				sub_hero_le(h, dmg);
			}
			if ((unsigned char)h->dead)
				f->atk_hit = 1;
#endif
		} else if (crit >= 9 && crit <= 11) {
			dmg = random_schweif(6);
			Lst_add2(g_fight_list, 0xd, dmg, MSG_HERO, h);
			sub_hero_le(h, dmg);
			if ((unsigned char)h->dead)
				f->atk_hit = 1;
		}
	} else if (roll1 > at) {
		Lst_add0(g_fight_list, 5, 0);
	} else {
		p2 = enemy_mr_target(h);
		if (!p2->parried && !turned_away) {
			/* Parade des Gegners */
			pa = enemy_parade_value(e);
			if ((unsigned char)e->tame || (unsigned char)e->renegade)
				pa = 0;
			roll2 = random_schweif(20);
			/* Probenlog: Zeile Art 7 (Gegner pariert gegen den Helden: PA, W20, Gegnerart, 0) */
#if FIX_PROBENLOG
			{
				signed short w[4];

				w[0] = pa; w[1] = roll2; w[2] = e->mon_id; w[3] = 0;
				probenlog(7, h->name, 4, w);
			}
#endif
			if (roll2 == 20 && random_schweif(20) > pa - 7) {
				/* Paradepatzer */
				hit = 1;
				dmg = 0;
				Lst_add2(g_fight_list, 4, 0, MSG_ENEMY, (struct hero far *)e);
				crit = random_interval(2, 12);
				if (crit >= 3 && crit <= 8) {
					f->atk_twice = 1;
					if (random_schweif(20) <= at) {
						dmg = hero_attack_damage(h, e, 0);
						if (dmg > 0) {
							Lst_add(g_fight_list, 0xb, dmg, MSG_HERO, h, MSG_ENEMY, (struct hero far *)e);
							enemy_sub_le(e, dmg, 0);
							if ((unsigned char)e->dead)
								g_fig_target_killed = 1;
						}
					}
				} else if (crit >= 9 && crit <= 11) {
					dmg = random_schweif(6);
					Lst_add2(g_fight_list, 0x10, dmg, MSG_ENEMY, (struct hero far *)e);
					enemy_sub_le(e, dmg, 1);
					if ((unsigned char)e->dead)
						g_fig_target_killed = 1;
				}
			} else if (roll2 > pa) {
				hit = 1;
			} else {
				/* pariert; gleiche Wuerfe: Bruchprobe */
				Lst_add2(g_fight_list, 0xc, 0, MSG_ENEMY, (struct hero far *)e);
				if (roll2 == roll1) {
					if ((signed short)WEAPON_BF(h) > 3) {
						if (random_schweif(12) + WEAPON_BF(h) > 15) {
							if (set_item_broken(h, INV_SLOT_RIGHT_HAND, 1))
								Lst_add2(g_fight_list, 6, 0, MSG_HERO, h);
						} else {
							BF_UP(h)
						}
					} else {
						BF_UP(h)
					}
				}
			}
		} else
			hit = 1;
		if (hit) {
			if (f->round / 2 + 2 > roll1)
				dmg = hero_attack_damage(h, e, 2);
			else
				dmg = hero_attack_damage(h, e, 0);
			if (dmg > 0) {
				Lst_add(g_fight_list, 0xb, dmg, MSG_HERO, h, MSG_ENEMY, (struct hero far *)e);
				enemy_sub_le(e, dmg, 0);
				if ((unsigned char)e->dead)
					g_fig_target_killed = 1;
			} else
				Lst_add2(g_fight_list, 0x16, 0, MSG_ENEMY, (struct hero far *)e);
		}
	}
	if (!g_quick_fight) {
		signed short wait = 0;

		f->action_anim = fig_hero_attack_seq(f, h, &f->seq);
		if (!turned_away) {
			f->action_anim2 = fig_enemy_defend_seq(f, e, &f->seq, h->viewdir);
		} else if (g_fig_target_killed) {
			wait = 1;
		} else {
			f->action_anim2 = f->action_anim;
		}
		do {
			game_idle();
			if (wait && seq_done(f->action_anim) == 1) {
				wait = 0;
				f->action_anim2 = fig_enemy_hit_seq(f, e, &f->seq);
			}
		} while (!seq_done(f->action_anim) || !seq_done(f->action_anim2));
		fig_stand_pic(f, h, &f->seq);
		fig_stand_seq(f, e, &f->seq);
		fight_draw(f);
		fight_msg_names(f, h, 0);
	} else
		game_idle();
}

/* Fernkampf Held gegen Gegner */
void fig_hero_shoot_enemy(Fight far *f, struct hero far *h, struct enemy far *e)
{
	signed short wclass;
	register signed short dmg;

	wclass = hand_weapon_class(h);
	if (wclass != -1) {
		if (range_attack_check_ammo(f, h, 1) == 0)
			return;
		dmg = hero_attack_damage(h, e, 0);
		if (dmg > 0) {
			Lst_add(g_fight_list, 0xb, dmg, MSG_HERO, h, MSG_ENEMY, (struct hero far *)e);
			enemy_sub_le(e, dmg, 0);
			if ((unsigned char)e->dead)
				g_fig_target_killed = 1;
		} else
			Lst_add2(g_fight_list, 0x16, 0, MSG_ENEMY, (struct hero far *)e);
		if (!g_quick_fight) {
			f->action_anim = fig_hero_shoot_seq(f, h, &f->seq);
			f->anim_removed = 0;
			do
				game_idle();
			while (!f->anim_removed && !seq_done(f->action_anim));
			if (g_fig_target_killed) {
				f->action_anim = fig_enemy_hit_seq(f, e, &f->seq);
				do
					game_idle();
				while (!seq_done(f->action_anim));
			}
			range_attack_check_ammo(f, h, 0);
			fig_stand_pic(f, h, &f->seq);
			fight_draw(f);
			fight_msg_names(f, h, 0);
		} else {
			game_idle();
			range_attack_check_ammo(f, h, 0);
		}
	}
}

/* Zauber Held auf Gegner */
void fig_hero_spell_enemy(Fight far *f, struct hero far *h, struct enemy far *e)
{
	signed short sp;
	signed short idx;
	signed short dir;
	struct fig_node far *fig;

	g_spell_no_sound = 1;
	sp = h->spell_id;
	use_spell(h, 0, 0);
	if (g_spell_cancelled == 0) {
		h->bp_left += (signed char)-5;
		if (h->bp_left < 0) h->bp_left = 0; else ;
	}
	strcpy(f->msg, g_text_output_buf);
	if (h->fb5 && !g_quick_fight) {
		sprintf(g_text_buf2, f->txt[49], h->name, g_ttx[h->spell_id + TTX_SPELL_NAMES]);
		fight_status_text(g_text_buf2);
	}
	if (!g_quick_fight) {
		if (g_spell_cancelled == 0) {
			register signed short dir_old;

			/* zum Ziel drehen */
			idx = 0;
			dir_old = h->viewdir;
			dir = fight_dir_hero(f, h);
			if (dir_old != dir) {
				memset(&f->seq, 0, sizeof(struct ani_seq));
				f->seq.figure = fight_hero_pic(h);
				fig_turn(f, h, dir_old, dir, &idx, &f->seq, 0);
				fig = fig_list_find(&f->list, h->fighter_id);
				f->seq.step[idx].frame = ANI_FRAME_END;
				seq_set(fig->anim, &f->seq);
				do
					game_idle();
				while (!seq_done(fig->anim));
			}
			if (g_speech_on == 1 && speech_active(g_snd_speech)) {
				snd_speech(g_snd_speech, h->sex == 0 ? sp + 0x1e : sp + 0x73);
				Speech_wait(g_snd_speech);
			}
			f->action_anim = fig_hero_spell_seq(f, h, &f->seq);
			f->anim_removed = 0;
			do
				game_idle();
			while (!f->anim_removed && !seq_done(f->action_anim));
			if (arc_exists(g_arc_fx, g_spell_snd[sp].snd))
				snd_play2(g_snd_fx, g_spell_snd[sp].snd);
			else
				snd_play2(g_snd_fx, SND_SPELL_DEFAULT);
			if (g_spell_success) {
				f->action_anim = fig_enemy_spell_hit_seq(f, e, &f->seq, sp, 2);
				f->anim_removed = 0;
				if (sp == SP_BLITZ)
					screen_flash();
				do
					game_idle();
				while (!f->anim_removed && !seq_done(f->action_anim));
				fig_stand_seq(f, e, &f->seq);
			}
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
