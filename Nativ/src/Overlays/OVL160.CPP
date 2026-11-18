/* OVL160 - Overlay O160 (Segment 0x2287 in SCHWEIF.EXE): Kampfaktionen eines Gegners gegen einen Gegner (abtruennige
 * bzw. verzauberte Gegner; Gegenstueck O161 gegen Helden). Animationen in f->action_anim/action_anim2, Ende per
 * seq_done, Warteschleifen mit game_idle.
 * fig_enemy_attack_enemy(f, att, def, turned_away): Nahkampf - AT/PA ueber enemy_attack_value/enemy_parade_value (+2
 * bei parried == 1 bzw. Schnellkampf g_quick_fight), W20-Wuerfe (random_schweif): Patzer (20 + Bestaetigung >
 * at - 9) mit Patzertabelle random_interval(2, 12), sonst Treffer/Parade mit Schaden enemy_attack_damage ->
 * Lst_add/enemy_sub_le, Meldungen Lst_add2 (g_fight_list), dead -> f->atk_hit bzw. g_fig_target_killed; danach
 * Animationen (fig_enemy_attack_seq/fig_enemy_defend_seq/fig_stand_seq, fig_enemy_hit_seq) und fight_draw.
 * fig_enemy_shoot_enemy(f, att, def): Fernkampf - Schuesse shots (sonst Wuerfe throws) per inline-Setter
 * verringern, Schaden dice_code_roll(shot_dam/throw_dam) - def->rs, Animation fig_enemy_shoot_seq.
 * fig_enemy_spell_enemy(f, att, def): Zauber cur_spell von att - Text nach f->msg (strcpy aus g_text_output_buf nach
 * enemy_cast_spell), Klang g_mon_spell_descriptions[cur_spell].snd (oder 0x92), Animationen fig_enemy_spell_seq/
 * fig_enemy_spell_hit_seq (g_spell_cancelled/g_spell_success), Meldung fight_status_text oder dlg_message.
 *
 * BCC -mlarge -3 -Ot -Og -Oi -c -Y OVL160.CPP   -> 2376/2376 Bytes GLEICH (18.10.2026), cobjfix laeuft
 *   (Namensfassung mit schweif.h, Welle N11: 19.10.2026 GLEICH, cobjfix LINKGLEICH)
 * - C++ wegen Bitfeld (Original "unsigned char b:1", hier (unsigned char)-Cast) und der inline-Setter
 *   enemy::set_shots/set_throws(signed short) (Parameterkopien [bp-2]/[bp-4] in fig_enemy_shoot_enemy).
 * - -Oi noetig (strcpy als Intrinsic), -Og noetig (Epilogkopien).
 * - Fehler des Originals nachgebaut (dangling else): im Patzerzweig von fig_enemy_attack_enemy haengt "else if
 *   (k >= 9 && k <= 11)" (und das leere "else if (k == 12) {}") am inneren "if (random_schweif(20) <= pa)" statt
 *   an "if (k >= 3 && k <= 8)" - die Faelle 9..12 werden dort nie erreicht (beim Verteidiger-Patzer ist die Kette
 *   richtig, ebenso in O161).
 * - "if (g_quick_fight)" -> cmp byte; "if (!g_quick_fight)" -> mov al/cbw/or ax,ax (signed char).
 * - fight_dir_enemy liefert (Prototyp) short, fig_enemy_defend_seq nimmt short (push ax ohne cbw wie mit char).
 */
#include <string.h>
#include "schweif.h"
#include "s_plog.h"	/* Probenlog (FIX_PROBENLOG) */

#define MSG_ENEMY	1		/* Lst_add*: Objekt ist ein Gegner */
#define MON_SPELL_FLASH	7		/* Gegnerzauber mit Aufblitzen (screen_flash) */
#define SND_MON_SPELL_DEFAULT 0x92	/* Klang, wenn der Zauberklang fehlt */
#define HERO(e)		((struct hero far *)(e))	/* Lst_add/Lst_add2 nehmen Helden- oder Gegnerzeiger */

void fig_enemy_attack_enemy(Fight far *f, struct enemy far *att, struct enemy far *def, signed short turned_away)
{
	signed short hit;
	signed short r1;
	signed short r2;
	signed short at;
	signed short def_at;
	signed short def_pa;
	struct enemy far *e1;
	struct enemy far *e2;
	signed short wait;
	register signed short dmg;
	register signed short k;

	hit = 0;
	if (g_quick_fight)
		game_idle();
	def_at = enemy_attack_value(def);
	def_pa = enemy_parade_value(def);
	at = enemy_attack_value(att);
	e1 = enemy_target_enemy(att);
	if (e1->parried == 1)
		at += 2;
	if (g_quick_fight)
		at += 2;
	r1 = random_schweif(20);
	/* Probenlog: Zeile Art 6 (Gegner attackiert Gegner: AT, W20, Gegnerart des Angreifers, 2) */
#if FIX_PROBENLOG
	{
		signed short w[4];

		w[0] = at; w[1] = r1; w[2] = att->mon_id; w[3] = 2;
		probenlog(6, (char far *)0, 4, w);
	}
#endif
	if (r1 == 20 && random_schweif(20) > at - 9) {
		/* Angriffspatzer */
		Lst_add2(g_fight_list, 3, 0, MSG_ENEMY, HERO(att));
		k = random_interval(2, 12);
#if FIX_0001
		/* Fix #1: dangling else: Patzerfolgen 9..12 haengen am Test k >= 3 && k <= 8 */
		if (k >= 3 && k <= 8) {
			f->def_strikes = 1;
			if (random_schweif(20) <= def_at) {
				dmg = enemy_attack_damage(def, att, 1);
				if (dmg > 0) {
					Lst_add(g_fight_list, 0x0b, dmg, MSG_ENEMY, HERO(def), MSG_ENEMY, HERO(att));
					enemy_sub_le(att, dmg, 1);
					if ((unsigned char)att->dead)
						f->atk_hit = 1;
				}
			}
		} else if (k >= 9 && k <= 11) {
			dmg = random_schweif(6);
			Lst_add2(g_fight_list, 0x0f, dmg, MSG_ENEMY, HERO(att));
			enemy_sub_le(att, dmg, 1);
			if ((unsigned char)att->dead)
				f->atk_hit = 1;
		} else if (k == 12) {
		}
#else
		if (k >= 3 && k <= 8) {
			f->def_strikes = 1;
			if (random_schweif(20) <= def_at) {
				dmg = enemy_attack_damage(def, att, 1);
				if (dmg > 0) {
					Lst_add(g_fight_list, 0x0b, dmg, MSG_ENEMY, HERO(def), MSG_ENEMY, HERO(att));
					enemy_sub_le(att, dmg, 1);
					if ((unsigned char)att->dead)
						f->atk_hit = 1;
				}
			} else if (k >= 9 && k <= 11) {
				dmg = random_schweif(6);
				Lst_add2(g_fight_list, 0x0f, dmg, MSG_ENEMY, HERO(att));
				enemy_sub_le(att, dmg, 1);
				if ((unsigned char)att->dead)
					f->atk_hit = 1;
			} else if (k == 12) {
			}
		}
#endif
	} else if (r1 > at) {
		Lst_add0(g_fight_list, 5, 0);
	} else {
		e2 = enemy_target_enemy(att);
		if (!e2->parried) {
			r2 = random_schweif(20);
			/* Probenlog: Zeile Art 7 (Gegner pariert gegen Gegner: PA, W20, Gegnerart des Angreifers, 2) */
#if FIX_PROBENLOG
			{
				signed short w[4];

				w[0] = def_pa; w[1] = r2; w[2] = att->mon_id; w[3] = 2;
				probenlog(7, (char far *)0, 4, w);
			}
#endif
			if (r2 == 20 && random_schweif(20) > def_pa - 7) {
				/* Paradepatzer */
				hit = 1;
				Lst_add2(g_fight_list, 4, 0, MSG_ENEMY, HERO(def));
				k = random_interval(2, 12);
				if (k >= 3 && k <= 8) {
					f->atk_twice = 1;
					if (random_schweif(20) <= at) {
						dmg = enemy_attack_damage(att, def, 1);
						if (dmg > 0) {
							Lst_add(g_fight_list, 0x0b, dmg, MSG_ENEMY, HERO(att), MSG_ENEMY, HERO(def));
							enemy_sub_le(def, dmg, 1);
							if ((unsigned char)def->dead)
								g_fig_target_killed = 1;
						}
					}
				} else if (k >= 9 && k <= 11) {
					dmg = random_schweif(6);
					Lst_add2(g_fight_list, 0x10, dmg, MSG_ENEMY, HERO(def));
					enemy_sub_le(def, dmg, 1);
					if ((unsigned char)def->dead)
						g_fig_target_killed = 1;
				}
			} else if (r2 > def_pa)
				hit = 1;
			else
				Lst_add2(g_fight_list, 0x0c, 0, MSG_ENEMY, HERO(def));
		} else
			hit = 1;
		if (hit) {
			dmg = enemy_attack_damage(att, def, 1);
			if (dmg > 0) {
				Lst_add(g_fight_list, 0x0b, dmg, MSG_ENEMY, HERO(att), MSG_ENEMY, HERO(def));
				enemy_sub_le(def, dmg, 1);
				if ((unsigned char)def->dead)
					g_fig_target_killed = 1;
			} else
				Lst_add2(g_fight_list, 0x16, 0, MSG_ENEMY, HERO(def));
		}
	}
	if (!g_quick_fight) {
		wait = 0;
		f->action_anim = fig_enemy_attack_seq(f, att, &f->seq);
		if (!turned_away)
			f->action_anim2 = fig_enemy_defend_seq(f, def, &f->seq, fight_dir_enemy(f, att));
		else if (g_fig_target_killed)
			wait = 1;
		else
			f->action_anim2 = f->action_anim;
		do {
			game_idle();
			if (wait && seq_done(f->action_anim) == 1) {
				wait = 0;
				f->action_anim2 = fig_enemy_hit_seq(f, def, &f->seq);
			}
		} while (!seq_done(f->action_anim) || !seq_done(f->action_anim2));
		fig_stand_seq(f, att, &f->seq);
		fig_stand_seq(f, def, &f->seq);
		fight_draw(f);
	} else
		game_idle();
}

void fig_enemy_shoot_enemy(Fight far *f, struct enemy far *att, struct enemy far *def)
{
	register signed short dmg;
	register signed short kind;

	if (att->shots > 0) {
		kind = 4;
		att->set_shots(att->shots - 1);
	} else {
		kind = 5;
		att->set_throws(att->throws - 1);
	}
	dmg = dice_code_roll(kind == 4 ? att->shot_dam : att->throw_dam) - def->rs;
	if (dmg > 0) {
		Lst_add(g_fight_list, 0x0b, dmg, MSG_ENEMY, HERO(att), MSG_ENEMY, HERO(def));
		enemy_sub_le(def, dmg, 1);
		if ((unsigned char)def->dead)
			g_fig_target_killed = 1;
	} else
		Lst_add2(g_fight_list, 0x16, 0, MSG_ENEMY, HERO(def));
	if (!g_quick_fight) {
		f->action_anim = fig_enemy_shoot_seq(f, att, &f->seq);
		f->anim_removed = 0;
		do
			game_idle();
		while (!f->anim_removed && !seq_done(f->action_anim));
		if (g_fig_target_killed) {
			f->action_anim = fig_enemy_hit_seq(f, def, &f->seq);
			do
				game_idle();
			while (!seq_done(f->action_anim));
		}
		fig_stand_seq(f, att, &f->seq);
		fight_draw(f);
	} else
		game_idle();
}

void fig_enemy_spell_enemy(Fight far *f, struct enemy far *att, struct enemy far *def)
{
	signed short sp;

	*g_text_output_buf = 0;
	sp = att->cur_spell;
	g_spell_no_sound = 1;
	enemy_cast_spell(att, 0);
	strcpy(f->msg, g_text_output_buf);
	if (!g_quick_fight) {
		if (!g_spell_cancelled) {
			f->action_anim = fig_enemy_spell_seq(f, att, &f->seq);
			f->anim_removed = 0;
			do
				game_idle();
			while (!f->anim_removed && !seq_done(f->action_anim));
			if (arc_exists(g_arc_fx, g_mon_spell_descriptions[sp].snd))
				snd_play2(g_snd_fx, g_mon_spell_descriptions[sp].snd);
			else
				snd_play2(g_snd_fx, SND_MON_SPELL_DEFAULT);
			if (g_spell_success) {
				f->action_anim = fig_enemy_spell_hit_seq(f, def, &f->seq, sp, 1);
				f->anim_removed = 0;
				if (sp == MON_SPELL_FLASH)
					screen_flash();
				do
					game_idle();
				while (!f->anim_removed && !seq_done(f->action_anim));
			}
			fig_stand_seq(f, att, &f->seq);
			fig_stand_seq(f, def, &f->seq);
			fight_draw(f);
		}
		if (all_heroes_down(g_group))
			fight_status_text(f->msg);
		else
			dlg_message(g_dialog, f->msg);
	} else
		game_idle();
}
