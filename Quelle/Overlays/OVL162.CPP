/* OVL162 - Overlay O162 (Segment 0x228e in SCHWEIF.EXE): Kampf - Gegenstand der linken Hand (inv[12]) gegen ein
 * Ziel einsetzen (BE FIG_use_item, DSA1 seg043). fig_use_item_hero(f, h, t) und fig_use_item_enemy(f, h, e): Art
 * nach der Gegenstandsbeschreibung g_items (herb_potion -> 1 = Anwenden fig_hero_use_seq, useable mit Anzahl ->
 * 2 = Wurf fig_hero_throw_seq, sonst 0); Wurfgegenstaende 0xee (5..10 Schaden), 0xef (10..25), 0x3b (1..20,
 * beim Gegner zusaetzlich RS -2), 0x8d (10..40), 0x8f (10..20 AE, nur wenn das Ziel AE hat: sub_ae_splash/
 * enemy_sub_ae) -> Text txt[37/76..79], Schaden sub_hero_le bzw. enemy_sub_le, Meldung Lst_add2(g_fight_list,
 * 0x11..0x14), dead -> g_fig_target_killed, Gegenstand verbrauchen drop_item(h, 12, 1); 0x14d..0x150 -> txt[91];
 * sonst use_item(h, 12). Danach Animation (Warteschleife game_idle/seq_done, bei g_fig_target_killed fig_hit_pic
 * bzw. fig_enemy_hit_seq), fig_stand_pic/fig_stand_seq, fight_draw, beim Helden fight_msg_names.
 * fig_use_item_enemy zusaetzlich: Gegenstand 0x61 bei g_fight_end == 3 -> Auswahlmenue txt[87] (3 Punkte): 1 =
 * group_give_item(g_group, 0x146, 1), 2 = Gegnerschaden + give_hero_new_item(h, 0x61, 1, 1), sonst g_fight_flag.
 * fig_hidden_enemies_text(f): Liste der noch nicht erschienenen Gegner (round_appear != 0) "n Name, n Name und n
 * Name." nach g_text_output_buf (txt[66]/[67], Zahl/Name txt[68] mit grammar_article(0x4000 bzw. 0x4004 fuer
 * Mehrzahl), Trenner txt[69]/[70]); Aufrufer O235 (Zauber) und O252 (Gegenstand) ueber g_fight_obj.
 *
 * BCC -mlarge -3 -Ot -Oi -Og -c -Y OVL162.CPP   -> 4132/4132 Bytes GLEICH (18.10.2026), cobjfix laeuft
 *   (_g_6c64 -> _g_af_spell_list+0x10).
 *   (Namensfassung mit schweif.h, Welle N11: 19.10.2026 GLEICH, cobjfix LINKGLEICH)
 * - STUFE D1 (wovd1): strcat(.., ".") als echtes Literal - es liegt im eigenen _DATA (ds:6c64).
 * - [bp-9] bzw. [bp-0bh] ist KEIN Local, sondern der -Og-Zwischenwert des Flagbytes g_items[w] (herb_potion und
 *   useable im selben if/else); ein Local/eine Strukturkopie laege als Klassenobjekt auf gerader Adresse.
 * - Bits des Ziels (Held/Gegner dead) im Original unsigned-char-Bitfeld: "if ((unsigned char)t->dead)" ->
 *   and ax,1 / or al,al; Gegenstandsflags unsigned short -> shr / and ax,1 / or ax,ax.
 * - "e->rs += (signed char)-2" -> add byte 0feh (-= 2 rechnet ueber al); "if (..) e->rs = 0; else ;" -> toter
 *   jmp short (ohne else 948 Abweichungen).
 * - list[n - 1].count wird dreimal verschieden adressiert (lea [bp-62h] / add ax,2 bzw. [bx+2]) - entsteht von
 *   selbst aus derselben Schreibweise.
 * Originalfehler: fig_hidden_enemies_text sucht mit "j <= n" einen Eintrag zu weit (liest das uninitialisierte
 *   list[n], bei 20 verschiedenen Gegnern sogar hinter das Feld; Treffer dort zaehlt einen Muelleintrag hoch statt
 *   neu anzulegen). fig_use_item_enemy kennt 0x14d nicht (fig_use_item_hero schon).
 */
#include <stdio.h>
#include <string.h>
#include "schweif.h"

#define MSG_ENEMY	1		/* Lst_add2: Objekt ist ein Gegner */
#define MSG_HERO	2		/* Lst_add2: Objekt ist ein Held */
#define HERO(e)		((struct hero far *)(e))
#define ITEM_SPECIAL_61	0x61		/* Sondergegenstand (g_fight_end == 3) */
#define ITEM_REWARD_146	0x146		/* Lohn bei Wahl 1 im Sondermenue */

enum {					/* Art des Einsatzes */
	USE_NONE = 0,
	USE_APPLY = 1,			/* fig_hero_use_seq */
	USE_THROW = 2			/* fig_hero_throw_seq */
};

struct mon_count {			/* Gegnerart und Anzahl (fig_hidden_enemies_text) */
	signed short mon_id;
	signed short count;
};

/* Gegenstand gegen einen Helden einsetzen */
void fig_use_item_hero(Fight far *f, struct hero far *h, struct hero far *t)
{
	signed short dmg;
	signed short thrown = 0;
	signed short use;
	signed short item;

	item = h->inv[INV_SLOT_LEFT_HAND].item_id;
	if (g_items[item].herb_potion)
		use = USE_APPLY;
	else if (!g_items[item].useable || h->inv[INV_SLOT_LEFT_HAND].quantity == 0)
		use = USE_NONE;
	else
		use = USE_THROW;

	*g_text_output_buf = 0;
	if (item == 0xee) {
		if (!g_quick_fight)
			strcpy(g_text_output_buf, f->txt[37]);
		dmg = random_interval(5, 10);
		sub_hero_le(t, dmg);
		Lst_add2(g_fight_list, 0x11, dmg, MSG_HERO, t);
		if ((unsigned char)t->dead)
			g_fig_target_killed = 1;
		drop_item(h, INV_SLOT_LEFT_HAND, 1);
		thrown = item;
		use = USE_THROW;
	} else if (item == 0xef) {
		if (!g_quick_fight)
			strcpy(g_text_output_buf, f->txt[76]);
		dmg = random_interval(10, 25);
		sub_hero_le(t, dmg);
		Lst_add2(g_fight_list, 0x11, dmg, MSG_HERO, t);
		if ((unsigned char)t->dead)
			g_fig_target_killed = 1;
		drop_item(h, INV_SLOT_LEFT_HAND, 1);
		thrown = item;
		use = USE_THROW;
	} else if (item == 0x3b) {
		if (!g_quick_fight)
			strcpy(g_text_output_buf, f->txt[77]);
		dmg = random_interval(1, 20);
		sub_hero_le(t, dmg);
		Lst_add2(g_fight_list, 0x11, dmg, MSG_HERO, t);
		if ((unsigned char)t->dead)
			g_fig_target_killed = 1;
		drop_item(h, INV_SLOT_LEFT_HAND, 1);
		thrown = item;
		use = USE_THROW;
	} else if (item == 0x8d) {
		if (!g_quick_fight)
			strcpy(g_text_output_buf, f->txt[78]);
		dmg = random_interval(10, 40);
		sub_hero_le(t, dmg);
		Lst_add2(g_fight_list, 0x11, dmg, MSG_HERO, t);
		if ((unsigned char)t->dead)
			g_fig_target_killed = 1;
		drop_item(h, INV_SLOT_LEFT_HAND, 1);
		thrown = item;
		use = USE_THROW;
	} else if (item == 0x8f) {
		/* raubt AE */
		if (!g_quick_fight)
			strcpy(g_text_output_buf, f->txt[79]);
		if (t->ae != 0) {
			dmg = random_interval(10, 20);
			sub_ae_splash(t, dmg);
			Lst_add2(g_fight_list, 0x12, dmg, MSG_HERO, t);
		}
		drop_item(h, INV_SLOT_LEFT_HAND, 1);
		thrown = item;
		use = USE_THROW;
	} else if (item == 0x14e || item == 0x14d || item == 0x14f || item == 0x150) {
		use = USE_NONE;
		strcpy(g_text_output_buf, f->txt[91]);
	} else {
		use_item(h, INV_SLOT_LEFT_HAND);
		*g_text_output_buf = 0;
	}

	if (use > USE_NONE) {
		if (!g_quick_fight) {
			if (use == USE_APPLY)
				f->action_anim = fig_hero_use_seq(f, h, &f->seq);
			else
				f->action_anim = fig_hero_throw_seq(f, h, &f->seq, thrown);
			f->anim_removed = 0;
			do {
				game_idle();
			} while (!f->anim_removed && !seq_done(f->action_anim));
			if (g_fig_target_killed != 0) {
				f->action_anim = fig_hit_pic(f, t, &f->seq);
				do {
					game_idle();
				} while (!seq_done(f->action_anim));
			}
			fig_stand_pic(f, h, &f->seq);
			fig_stand_pic(f, t, &f->seq);
			fight_draw(f);
			fight_msg_names(f, t, 0);
		} else {
			game_idle();
		}
	}
	dlg_message(g_dialog, g_text_output_buf);
}

/* Gegenstand gegen einen Gegner einsetzen */
void fig_use_item_enemy(Fight far *f, struct hero far *h, struct enemy far *e)
{
	signed short dmg;
	signed short thrown = 0;
	signed short use;
	signed short item;
	signed short special = 0;

	item = h->inv[INV_SLOT_LEFT_HAND].item_id;
	if (g_items[item].herb_potion)
		use = USE_APPLY;
	else if (!g_items[item].useable || h->inv[INV_SLOT_LEFT_HAND].quantity == 0)
		use = USE_NONE;
	else
		use = USE_THROW;

	*g_text_output_buf = 0;
	if (item == 0xee) {
		if (!g_quick_fight)
			strcpy(g_text_output_buf, f->txt[37]);
		dmg = random_interval(5, 10);
		enemy_sub_le(e, dmg, 0);
		Lst_add2(g_fight_list, 0x13, dmg, MSG_ENEMY, HERO(e));
		if ((unsigned char)e->dead)
			g_fig_target_killed = 1;
		drop_item(h, INV_SLOT_LEFT_HAND, 1);
		thrown = item;
		use = USE_THROW;
	} else if (item == 0xef) {
		if (!g_quick_fight)
			strcpy(g_text_output_buf, f->txt[76]);
		dmg = random_interval(10, 25);
		enemy_sub_le(e, dmg, 0);
		Lst_add2(g_fight_list, 0x13, dmg, MSG_ENEMY, HERO(e));
		if ((unsigned char)e->dead)
			g_fig_target_killed = 1;
		drop_item(h, INV_SLOT_LEFT_HAND, 1);
		thrown = item;
		use = USE_THROW;
	} else if (item == 0x3b) {
		if (!g_quick_fight)
			strcpy(g_text_output_buf, f->txt[77]);
		dmg = random_interval(1, 20);
		enemy_sub_le(e, dmg, 0);
		e->rs += (signed char)-2;
		if (e->rs < 0)
			e->rs = 0;
		else ;
		Lst_add2(g_fight_list, 0x13, dmg, MSG_ENEMY, HERO(e));
		if ((unsigned char)e->dead)
			g_fig_target_killed = 1;
		drop_item(h, INV_SLOT_LEFT_HAND, 1);
		thrown = item;
		use = USE_THROW;
	} else if (item == 0x8d) {
		if (!g_quick_fight)
			strcpy(g_text_output_buf, f->txt[78]);
		dmg = random_interval(10, 40);
		enemy_sub_le(e, dmg, 0);
		Lst_add2(g_fight_list, 0x13, dmg, MSG_ENEMY, HERO(e));
		if ((unsigned char)e->dead)
			g_fig_target_killed = 1;
		drop_item(h, INV_SLOT_LEFT_HAND, 1);
		thrown = item;
		use = USE_THROW;
	} else if (item == 0x8f) {
		/* raubt AE */
		if (!g_quick_fight)
			strcpy(g_text_output_buf, f->txt[79]);
		if (e->ae > 0) {
			dmg = random_interval(10, 20);
			enemy_sub_ae(e, dmg);
			Lst_add2(g_fight_list, 0x14, dmg, MSG_ENEMY, HERO(e));
		}
		drop_item(h, INV_SLOT_LEFT_HAND, 1);
		thrown = item;
		use = USE_THROW;
	} else if (item == ITEM_SPECIAL_61 && g_fight_end == 3) {
		drop_item(h, INV_SLOT_LEFT_HAND, 1);
		special = 1;
		use = USE_THROW;
#if FIX_0076
	/* Fix #76: 0x14d wie in fig_use_item_hero */
	} else if (item == 0x14e || item == 0x14d || item == 0x14f || item == 0x150) {
#else
	} else if (item == 0x14e || item == 0x14f || item == 0x150) {
#endif
		use = USE_NONE;
		strcpy(g_text_output_buf, f->txt[91]);
	} else {
		use_item(h, INV_SLOT_LEFT_HAND);
		*g_text_output_buf = 0;
	}

	if (use > USE_NONE) {
		if (!g_quick_fight) {
			if (use == USE_APPLY)
				f->action_anim = fig_hero_use_seq(f, h, &f->seq);
			else {
				if (enemy_type_check(e))
					thrown = 0;
				f->action_anim = fig_hero_throw_seq(f, h, &f->seq, thrown);
			}
			f->anim_removed = 0;
			do {
				game_idle();
			} while (!f->anim_removed && !seq_done(f->action_anim));
			if (g_fig_target_killed != 0) {
				f->action_anim = fig_enemy_hit_seq(f, e, &f->seq);
				do {
					game_idle();
				} while (!seq_done(f->action_anim));
			}
			fig_stand_pic(f, h, &f->seq);
			fig_stand_seq(f, e, &f->seq);
			fight_draw(f);
		} else {
			game_idle();
		}
	}
	if (special == 1) {
		register signed short sel;
		struct enemy far *first;

		/* Sondermenue: Kampf endet */
		g_in_fight = 0;
		do {
			sel = dlg_choose(g_dialog, f->txt[87], 3, f->txt[88], f->txt[89], f->txt[90]);
		} while (sel == -1);
		if (sel == 1) {
			first = g_enemies;
			enemy_sub_le(first, first->le, 0);
			group_give_item(g_group, ITEM_REWARD_146, 1);
		} else if (sel == 2) {
			first = g_enemies;
			enemy_sub_le(first, first->le, 0);
			give_hero_new_item(h, ITEM_SPECIAL_61, 1, 1);
		} else {
			g_fight_flag = 1;
			give_hero_new_item(h, ITEM_SPECIAL_61, 1, 1);
		}
	} else
		dlg_message(g_dialog, g_text_output_buf);
}

/* Liste der noch nicht erschienenen Gegner nach g_text_output_buf */
void fig_hidden_enemies_text(Fight far *f)
{
	signed short n = 0;
	signed short i;
	signed short j;
	signed short mon;
	signed short found;
	struct enemy far *e;
	struct mon_count list[NR_ENEMIES];

	for (i = 0; i < g_nr_enemies; i++) {
		e = &g_enemies[i];
		if (e->round_appear != 0) {
			mon = e->mon_id;
			found = 0;
#if FIX_0075
			/* Fix #75: nur die n belegten Eintraege durchsuchen */
			for (j = 0; j < n; j++) {
#else
			for (j = 0; j <= n; j++) {
#endif
				if (list[j].mon_id == mon) {
					list[j].count++;
					found = 1;
					break;
				}
			}
			if (found == 0) {
				list[n].mon_id = mon;
				list[n].count = 1;
				n++;
			}
		}
	}
	if (n != 0) {
		strcpy(g_text_output_buf, f->txt[66]);
		for (i = 0; n - 1 > i; i++) {
			sprintf(g_text_buf2, f->txt[68], list[i].count,
				grammar_article((list[i].count > 1 ? 4 : 0) + 0x4000, list[i].mon_id, 1));
			strcat(g_text_output_buf, g_text_buf2);
			if (n - 2 > i)
				strcat(g_text_output_buf, f->txt[69]);
		}
		if (n > 1)
			strcat(g_text_output_buf, f->txt[70]);
		sprintf(g_text_buf2, f->txt[68], list[n - 1].count,
			grammar_article((list[n - 1].count > 1 ? 4 : 0) + 0x4000, list[n - 1].mon_id, 1));
		strcat(g_text_output_buf, g_text_buf2);
		strcat(g_text_output_buf, ".");
	} else {
		strcpy(g_text_output_buf, f->txt[67]);
	}
}
