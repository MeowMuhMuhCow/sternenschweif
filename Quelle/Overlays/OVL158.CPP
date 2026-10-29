/* OVL158 - Overlay O158 (Segment 0x2280 in SCHWEIF.EXE): Kampf - Aktion eines Helden gegen einen Helden
 * (verzauberter/verwirrter Held; Gegenstueck O159 gegen Gegner). fig_hero_attack_hero(f, a, d): Nahkampf - AT/PA
 * ueber hero_at/hero_at_weapon bzw. hero_pa/hero_pa_weapon (ohne Waffe hero_weapon_type == -1), Malus -4 bei
 * group_mr_bonus == -5, Patzer (W20 == 20 und zweiter W20 > AT - 9) mit 2W6-Tabelle (2: Waffe bricht set_item_broken,
 * 3..8: Gegenangriff, 9..11: W6 Selbstschaden), Paradepatzer (> PA - 7), gleiche Wuerfe -> Bruchfaktor +1 bzw.
 * Bruchprobe (W12 + BF > 15), Treffer hero_attack_damage / Meldungen Lst_add/Lst_add2/Lst_add0 (g_fight_list);
 * danach Animationen fig_hero_attack_seq/fig_hero_defend_seq und Warten auf seq_done. fig_hero_shoot_hero(f, a, d):
 * Fernkampf (hand_weapon_class, range_attack_check_ammo, Animation fig_hero_shoot_seq/fig_hit_pic).
 * fig_hero_spell_hero(f, a, d): Zauber - use_spell, BP -5, Meldung strcpy(f->msg, g_text_output_buf), ggf.
 * Zaubername (g_ttx[0x85 + spell_id]), Drehung (fig_turn), Sprachausgabe, Klang (g_spell_snd, Pruefung
 * arc_exists), Animation fig_hero_spell_seq / fig_hero_spell_hit_seq (g_spell_success), Blitz -> screen_flash.
 *
 * BCC -mlarge -3 -Ot -Og -Oi -c -Y -E..\bc\BIN\TASM.EXE OVL158.CPP   -> 3425/3425 Bytes GLEICH (18.10.2026),
 *   cobjfix laeuft. Ohne -Og 5 Bytes zu kurz; -Oi fuer strcpy/memset (rep movsw/stosw).
 *   (Namensfassung mit schweif.h, Welle N11: 19.10.2026 GLEICH, cobjfix LINKGLEICH)
 * - Bruchfaktor der Waffe (inv[3].bf, +1ff): inline hero::is_weapon_breakable() im && -> xor ax,ax / mov ax,1 /
 *   or ax,ax; im Makro BF_UP direkt geschrieben -> mov al,0 / mov al,1 / or al,al, dazu "else ;" (toter jmp
 *   short). "(signed short)bf > 3" -> cbw / cmp ax,3.
 * - fig_hero_spell_hero: Drehrichtung als register-Variable im INNEREN Block (SI); auf Funktionsebene kommt sie
 *   wegen strcpy/memset (SI/DI) auf den Stack.
 * - Warteschleifen "do game_idle(); while (!f->anim_removed && !seq_done(p));".
 * - Originalfehler/Auffaelliges: fig_hero_attack_hero holt fuer die Parade p2 = hero_mr_target(a) vom ANGREIFER
 *   (wie p1, gemeint war wohl der Verteidiger d) und meldet den Paradepatzer (Code 2) mit dem Angreifer; beim
 *   Angriffspatzer 3..8 wird f->atk_hit auch ohne Schaden gesetzt (dead ausserhalb von "if (dmg > 0)");
 *   fig_hero_spell_hero rechnet den Index g_ttx[0x85 + h->spell_id] neu statt sp zu nehmen (harmlos).
 */
#include <stdio.h>
#include <string.h>
#include <mem.h>
#include "schweif.h"

#define WEAPON_BF(h)	((h)->inv[INV_SLOT_RIGHT_HAND].bf)
#define BF_UP(h) if ((WEAPON_BF(h) == -99 ? 0 : 1) && WEAPON_BF(h) < 20) WEAPON_BF(h)++; else ;
#define MSG_HERO	2		/* Lst_add*: Objekt ist ein Held */
#define TTX_SPELL_NAMES	0x85		/* g_ttx: Zaubernamen */
#define SND_SPELL_DEFAULT 0x97		/* Klang, wenn der Zauberklang fehlt */

/* Nahkampf Held gegen Held */
void fig_hero_attack_hero(Fight far *f, struct hero far *att, struct hero far *def)
{
	signed short wt_att;
	signed short wt_def;
	signed short hit;
	signed short roll1;
	signed short roll2;
	signed short at;
	signed short pa;
	signed short at_def;
	struct hero far *p1;
	struct hero far *p2;
	register signed short dmg;
	register signed short crit;

	hit = 0;
	if (g_quick_fight)
		game_idle();
	wt_att = hero_weapon_type(att);
	wt_def = hero_weapon_type(def);
	if (wt_att == -1)
		at = hero_at_weapon(att, 0);
	else
		at = hero_at(att);
	if (wt_def == -1)
		at_def = hero_at_weapon(def, 0);
	else
		at_def = hero_at(def);
	if (group_mr_bonus(g_group) == -5) {
		at -= 4;
		at_def -= 4;
	}
	p1 = hero_mr_target(att);
	if (p1->parried == 1)
		at += 2;
	at += 4;
	roll1 = random_schweif(20);
	if (roll1 == 20 && random_schweif(20) > at - 9) {
		/* Angriffspatzer */
		dmg = 0;
		Lst_add2(g_fight_list, 1, 0, MSG_HERO, att);
		crit = random_interval(2, 12);
		if (crit == 2 && wt_att != -1 && att->is_weapon_breakable()) {
			if (set_item_broken(att, INV_SLOT_RIGHT_HAND, 1))
				Lst_add2(g_fight_list, 6, 0, MSG_HERO, att);
		} else if (crit >= 3 && crit <= 8) {
			f->def_strikes = 1;
			if (random_schweif(20) <= at_def)
				dmg = hero_attack_damage(def, att, 1);
#if FIX_0042
			/* Fix #42: Folge nur nach Schaden (Abfrage in den Schadensblock) */
			if (dmg > 0) {
				Lst_add(g_fight_list, 8, dmg, MSG_HERO, def, MSG_HERO, att);
				sub_hero_le(att, dmg);
				if ((unsigned char)att->dead)
					f->atk_hit = 1;
			}
#else
			if (dmg > 0) {
				Lst_add(g_fight_list, 8, dmg, MSG_HERO, def, MSG_HERO, att);
				sub_hero_le(att, dmg);
			}
			if ((unsigned char)att->dead)
				f->atk_hit = 1;
#endif
		} else if (crit >= 9 && crit <= 11) {
			dmg = random_schweif(6);
			Lst_add2(g_fight_list, 0xd, dmg, MSG_HERO, att);
			sub_hero_le(att, dmg);
			if ((unsigned char)att->dead)
				f->atk_hit = 1;
		}
	} else if (roll1 > at) {
		Lst_add0(g_fight_list, 5, 0);
	} else {
		p2 = hero_mr_target(att);
		if (!p2->parried) {
			/* Parade */
			if (wt_def == -1)
				pa = hero_pa_weapon(def, 0);
			else
				pa = hero_pa(def);
			if (def->action_id == FIG_ACTION_GUARD)
				pa += 3;
			if (group_mr_bonus(g_group) == -5)
				pa -= 4;
			if (pa < 0)
				pa = 0;
			roll2 = random_schweif(20);
			if (roll2 == 20 && random_schweif(20) > pa - 7) {
				/* Paradepatzer */
				hit = 1;
				dmg = 0;
#if FIX_0041
				/* Fix #41: Paradepatzer nennt den Verteidiger def */
				Lst_add2(g_fight_list, 2, 0, MSG_HERO, def);
				crit = random_interval(2, 12);
#else
				Lst_add2(g_fight_list, 2, 0, MSG_HERO, att);
				crit = random_interval(2, 12);
#endif
				if (crit >= 3 && crit <= 8) {
					f->atk_twice = 1;
					if (random_schweif(20) <= at) {
						dmg = hero_attack_damage(att, def, 1);
						if (dmg > 0) {
							Lst_add(g_fight_list, 8, dmg, MSG_HERO, att, MSG_HERO, def);
							sub_hero_le(def, dmg);
							if ((unsigned char)def->dead)
								g_fig_target_killed = 1;
						}
					}
				} else if (crit >= 9 && crit <= 11) {
					dmg = random_schweif(6);
					Lst_add2(g_fight_list, 0xe, dmg, MSG_HERO, def);
					sub_hero_le(def, dmg);
					if ((unsigned char)def->dead)
						g_fig_target_killed = 1;
				}
			} else if (roll2 > pa) {
				hit = 1;
			} else {
				/* pariert; gleiche Wuerfe: Bruchprobe beider Waffen */
				Lst_add2(g_fight_list, 9, 0, MSG_HERO, def);
				if (roll2 == roll1) {
					if ((signed short)WEAPON_BF(att) > 3) {
						if (random_schweif(12) + WEAPON_BF(att) > 15) {
							if (set_item_broken(att, INV_SLOT_RIGHT_HAND, 1))
								Lst_add2(g_fight_list, 6, 0, MSG_HERO, att);
						} else {
							BF_UP(att)
						}
					} else {
						BF_UP(att)
					}
					if ((signed short)WEAPON_BF(def) > 3) {
						if (random_schweif(12) + WEAPON_BF(def) > 15) {
							if (set_item_broken(def, INV_SLOT_RIGHT_HAND, 1))
								Lst_add2(g_fight_list, 6, 0, MSG_HERO, def);
						} else {
							BF_UP(def)
						}
					} else {
						BF_UP(def)
					}
				}
			}
		} else
			hit = 1;
		if (hit) {
			dmg = hero_attack_damage(att, def, 1);
			if (dmg > 0) {
				Lst_add(g_fight_list, 8, dmg, MSG_HERO, att, MSG_HERO, def);
				sub_hero_le(def, dmg);
				if ((unsigned char)def->dead)
					g_fig_target_killed = 1;
			} else
				Lst_add2(g_fight_list, 0x16, 0, MSG_HERO, def);
		}
	}
	if (!g_quick_fight) {
		f->action_anim = fig_hero_attack_seq(f, att, &f->seq);
		f->action_anim2 = fig_hero_defend_seq(f, def, &f->seq, att->viewdir);
		do
			game_idle();
		while (!seq_done(f->action_anim) || !seq_done(f->action_anim2));
		fig_stand_pic(f, att, &f->seq);
		fig_stand_pic(f, def, &f->seq);
		fight_draw(f);
		fight_msg_names(f, att, def);
	} else
		game_idle();
}

/* Fernkampf Held gegen Held */
void fig_hero_shoot_hero(Fight far *f, struct hero far *att, struct hero far *def)
{
	signed short wclass;
	register signed short dmg;

	wclass = hand_weapon_class(att);
	if (wclass != -1) {
		if (range_attack_check_ammo(f, att, 1) == 0)
			return;
		dmg = hero_attack_damage(att, def, 1);
		if (dmg > 0) {
			Lst_add(g_fight_list, 8, dmg, MSG_HERO, att, MSG_HERO, def);
			sub_hero_le(def, dmg);
			if ((unsigned char)def->dead)
				g_fig_target_killed = 1;
		} else
			Lst_add2(g_fight_list, 0x16, 0, MSG_HERO, def);
		if (!g_quick_fight) {
			f->action_anim = fig_hero_shoot_seq(f, att, &f->seq);
			f->anim_removed = 0;
			do
				game_idle();
			while (!f->anim_removed && !seq_done(f->action_anim));
			if (g_fig_target_killed) {
				f->action_anim = fig_hit_pic(f, def, &f->seq);
				do
					game_idle();
				while (!seq_done(f->action_anim));
			}
			range_attack_check_ammo(f, att, 0);
			fig_stand_pic(f, att, &f->seq);
			fig_stand_pic(f, def, &f->seq);
			fight_draw(f);
			fight_msg_names(f, att, def);
		} else {
			range_attack_check_ammo(f, att, 0);
			game_idle();
		}
	}
}

/* Zauber Held auf Held */
void fig_hero_spell_hero(Fight far *f, struct hero far *att, struct hero far *def)
{
	signed short sp;
	signed short idx;
	signed short dir;
	struct fig_node far *fig;

	g_spell_no_sound = 1;
	sp = att->spell_id;
	use_spell(att, 0, 0);
	if (g_spell_cancelled == 0) {
		att->bp_left += (signed char)-5;
		if (att->bp_left < 0) att->bp_left = 0; else ;
	}
	strcpy(f->msg, g_text_output_buf);
	if (att->fb5 && !g_quick_fight) {
		sprintf(g_text_buf2, f->txt[49], att->name, g_ttx[att->spell_id + TTX_SPELL_NAMES]);
		fight_status_text(g_text_buf2);
	}
	if (!g_quick_fight) {
		if (g_spell_cancelled == 0) {
			register signed short dir_old;

			/* zum Ziel drehen */
			idx = 0;
			dir_old = att->viewdir;
			dir = fight_dir_hero(f, att);
			if (dir_old != dir) {
				memset(&f->seq, 0, sizeof(struct ani_seq));
				f->seq.figure = fight_hero_pic(att);
				fig_turn(f, att, dir_old, dir, &idx, &f->seq, 0);
				fig = fig_list_find(&f->list, att->fighter_id);
				f->seq.step[idx].frame = ANI_FRAME_END;
				seq_set(fig->anim, &f->seq);
				do
					game_idle();
				while (!seq_done(fig->anim));
			}
			if (g_speech_on == 1 && speech_active(g_snd_speech)) {
				snd_speech(g_snd_speech, att->sex == 0 ? sp + 0x1e : sp + 0x73);
				Speech_wait(g_snd_speech);
			}
			f->action_anim = fig_hero_spell_seq(f, att, &f->seq);
			f->anim_removed = 0;
			do
				game_idle();
			while (!f->anim_removed && !seq_done(f->action_anim));
			if (arc_exists(g_arc_fx, g_spell_snd[sp].snd))
				snd_play2(g_snd_fx, g_spell_snd[sp].snd);
			else
				snd_play2(g_snd_fx, SND_SPELL_DEFAULT);
			if (g_spell_success) {
				f->action_anim = fig_hero_spell_hit_seq(f, def, &f->seq, sp, 2);
				f->anim_removed = 0;
				if (sp == SP_BLITZ)
					screen_flash();
				do
					game_idle();
				while (!f->anim_removed && !seq_done(f->action_anim));
			}
			fig_stand_pic(f, att, &f->seq);
			fig_stand_pic(f, def, &f->seq);
			fight_draw(f);
		}
		if (all_heroes_down(g_group))
			fight_status_text(f->msg);
		else
			dlg_message(g_dialog, f->msg);
		fight_msg_names(f, att, def);
	} else
		game_idle();
}
