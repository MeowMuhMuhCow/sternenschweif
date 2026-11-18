/* OVL143 - Overlay O143 (Segment 0x2236 in SCHWEIF.EXE): Kampfende (Aufrufer O018 run(): O143_0000(this, &ret)).
 * fight_finish(f, &ret): bei ds:394b != 0 (Flucht/Abbruch) - Sonderkampf 0x493 O200_0ACE, Helden auf einem Fluchtfeld
 * (O153_0938(R02_0C5A(h) + 1)) bzw. mit Bit +bc.6/+bc.1 bekommen bei ds:394b == 7 Bit +bd.0, sonst Schaden +72
 * (O076_01F3); Anfuehrer == O088_016B -> ds:2807 = 0x63/O196_0000; ret = 1 (6 = Flucht) bzw. 2 (O074_01FE ->
 * ds:c4d4). Bei ret == 0: geflohene Gegner (O153_0938(O153_0AF8(e) + 10)) per O192_0413 entfernen, Beute f->1ad
 * (O202_0777), fig_loot_monsters, fig_split_ap, ggf. alle Gegner tot (ds:c68b). Danach O147_00D1/O023_0090, bei ret == 1 und
 * Gruppenort fight_split_escaped; Aufraeumen (O080_0298, Merker 0, 69 Bytes ds:6668, ds:3958--, Bildschirm O034_0551/
 * O042_0C9A). fight_split_escaped(f): Gruppe nach Flucht aufteilen - nicht geflohene Helden O075_0000(n, h0..h6), Anfuehrer
 * (verfuegbar, nicht +9b), bei mehreren Gruppen jeden Helden einzeln O075_0000(1, h) und neue Gruppe (ds:c30f,
 * 6 x 0x49) per O201_007F, Ort ds:0bbd[grp->dungeon] (O053_0A38/05C4/0A91), O046_0E55. fig_loot_monsters(f): Beute-Auswahl
 * (Gegenstaende f->f6b[30], Namen ueber O056_0449 in 30 x 40 Bytes ds:b6c1, Seiten zu 15 mit "weiter",
 * O202_0777, Liste nachruecken), danach Geld f->fa7 * 100 + fa9 * 10 + fab (O074_0000, txt[15], O200_0084).
 * fig_split_ap(f): Abenteuerpunkte - Gegnerarten zaehlen (Tabelle 20 x {Art, Anzahl}), AP je Gegner (300 bei 0x495,
 * bekannte Art ds:28ef: Stufe/10 (mind. 1), O192_0214: 750, sonst +22), Meldung "besiegt: ..." (sprintf/strcat
 * mit O056_0000(0x4000 bzw. 0x4004 Mehrzahl)) ausser bei 0x83/0x20, bei 0x495/0x493 feste Texte; Arten als bekannt
 * markieren; AP gleichmaessig auf lebende Helden (O074_03F1), vorher auf Vielfaches der Anzahl aufrunden.
 *
 * BCC -mlarge -3 -Ot -Og -Oi -c -Y OVL143.CPP   -> 4689/4689 Bytes GLEICH (18.10.2026), cobjfix laeuft
 *   (Namensfassung mit schweif.h, Welle N7: 19.10.2026 GLEICH, cobjfix LINKGLEICH)
 * - -Og noetig (Zwischenwerte: Byte +bc/+39 in [bp-0fh]/[bp-19h], sel + page, (long)alive); -Oi fuer memset,
 *   strcpy, strcmp, strcat (strcat-Intrinsic: repne scasb / dec di / rep movsb).
 * - C++ wegen Bitfeldern und des inline-Getters enemy::getid() (unsigned char): "tab[j].id = e->getid()" laedt
 *   den Wert VOR der Zieladresse (lea dx); direkt "e->f0" rechnet erst die Adresse (les si danach).
 * - "ap += (signed long)(unsigned long)e->f22" -> movzx eax / add [bp-4],eax; nur (unsigned long) macht die
 *   Rechnung vorzeichenlos (mov edx / add / mov), ohne Cast mov al / mov ah,0 / movsx.
 * - Struct mit 7 far-Zeigern an O075_0000 als einzelne Felder (14 push word), Ketten "first = h = ..",
 *   "g_395f = g_65e5 = g_395e = 0", "*g_b674 = found = 0", "tab[j].id = tab[j].n = 0".
 * - for mit Komma: "for (j = 0, g = g_c30f; j < 6; j++, g = &g_c30f[j])" (DS-Feld als far-Zeiger: mov [..],ds).
 * Originalfehler: fig_loot_monsters liest in "while ((id = f->ff6b[i]) != 0 && i < 30)" erst das Element und prueft dann
 *   den Index -> bei 30 Gegenstaenden wird f->ff6b[30] (= f->ffa7) gelesen (folgenlos, i < 30 bricht danach ab).
 */
#include <stdio.h>
#include <string.h>
#include "schweif.h"

/* Phase B: eigene Daten des Moduls (ds:6b9c..6be6, dsana) - initialisierte Daten in Adressreihenfolge, danach der Literalpool */
struct hero_ptrs7 g_hero_ptrs7_null = { { 0, 0, 0, 0, 0, 0, 0 } };
struct hero_ptrs7 g_hero_ptrs7_null2 = { { 0, 0, 0, 0, 0, 0, 0 } };

struct mon_count {			/* Gegnerart und Anzahl (fig_split_ap) */
	signed char mon_id;
	signed char n;
};

void fight_finish(Fight far *f, signed short far *ret)
{
	signed short x;
	signed short y;
	struct enemy far *e;
	struct hero far *h;
	signed short died = 0;
	register signed short i;

	if (g_game_state != 2) {
		if (g_game_state != 0) {
			/* Flucht bzw. Abbruch */
			g_in_fight = 1;
			if (f->id == FIGHT_ID_SPECIAL) {
				group_remove_npc(g_group, group_get_npc(g_group));
				g_sfight_seen = 1;
			}
			if (g_game_state == 7) {
				g_game_state = 6;
				h = get_first_hero(g_group);
				do {
					if ((!(signed char)h->dead && fight_find_cell(get_hero_index(h) + FIG_ID_HERO, &x, &y)) ||
						(unsigned char)h->unconscious || (unsigned char)h->asleep)
						h->scared = 1;
					h = get_next_hero(g_group, h);
				} while (get_first_hero(g_group) != h);
			} else {
				h = get_first_hero(g_group);
				do {
					if ((!(signed char)h->dead && fight_find_cell(get_hero_index(h) + FIG_ID_HERO, &x, &y)) ||
						(unsigned char)h->unconscious || (unsigned char)h->asleep)
						sub_hero_le(h, h->le);
					h = get_next_hero(g_group, h);
				} while (get_first_hero(g_group) != h);
			}
			if (get_first_active_hero(g_group) == group_get_npc(g_group)) {
				g_npc_months = 0x63;
				groups_check_npc();
			}
			g_in_fight = 0;
			if (g_game_state != 6) {
				if (count_heroes_check2()) {
					g_game_state = 0;
					died = 1;
				}
				*ret = 2;
			} else {
				*ret = 1;
				g_game_state = 0;
			}
			if (g_quick_fight)
				fight_quick_end(f, *ret);
			if (died)
				g_hero_died = 1;
		}
		if (*ret == 0) {
			/* Gegner, die noch auf dem Feld stehen, sterben */
			for (i = 0; i < NR_ENEMIES; i++) {
				e = g_enemies + i;
				if (!(signed char)e->dead && !e->round_appear && fight_find_cell(fight_obj_index(e) + FIG_ID_ENEMY, &x, &y))
					enemy_sub_le(e, e->le, 0);
			}
		}
		if (*ret == 0) {
			if (g_quick_fight)
				fight_quick_end(f, *ret);
			for (i = 0; f->dropped[i]; )
				group_give_item(g_group, f->dropped[i++], 1);
			fig_loot_monsters(f);
			fig_split_ap(f);
			if (g_fight_max_enemies && !g_fight_flag) {
				for (i = 0; i < NR_ENEMIES; i++) {
					e = g_enemies + i;
					e->dead = 1;
				}
			}
		}
		if (!g_fight_flag) {
			fig_tidy_monsters(f);
			fight_write_rec(f);
		}
		if (*ret == 1 && g_group->dungeon)
			fight_split_escaped(f);
	} else
		*ret = 2;
	fight_end_heroes(g_group);
	g_fight_end = g_fight_flee_all = g_fight_flag = 0;
	memset(&g_fight_defs_ab[0], 0, sizeof(struct fight_def));	/* Kampf 0xab (Zufallsbegegnung) leeren */
	g_dialog_depth--;
	g_refresh = g_hero_died = 1;
	g_pp20_index = -2;
	dlg_set16(g_dialog, f->old_dlg_mode);
	clear_screen_buf(g_vga_buf);
	snd_obj_stop(g_snd_music);
	game_idle();
	g_key = g_action = 0;
}

void fight_split_escaped(Fight far *f)
{
	signed short n = 0;
	struct hero far *first;
	struct hero far *h;
	struct hero far *leader;
	struct view_area far *loc;
	signed short i;
	signed short j;
	signed short cnt;
	struct group far *g;
	signed short escaped;
	struct hero_ptrs7 stay = g_hero_ptrs7_null;
	struct hero_ptrs7 all = g_hero_ptrs7_null2;

	escaped = 0;
	diary_unlock(g_diary);
	loc = &g_dng_view_areas[g_group->dungeon - 1];
	first = h = get_first_hero(g_group);
	leader = 0;
	do {
		if (!(signed char)h->scared) {
			stay.h[n] = h;
			n++;
			if (check_hero(h) && !h->npc_id)
				leader = h;
		} else
			escaped = 1;
		h = get_next_hero(g_group, h);
	} while (h != first);
	if (n && escaped)
		make_group(n, stay.h[0], stay.h[1], stay.h[2], stay.h[3], stay.h[4], stay.h[5], stay.h[6]);
	if (escaped) {
		cnt = g_group->nr_heroes;
		if (leader == 0)
			leader = get_first_active_hero(g_group);
		if (cnt == 1) {
			automap_init(g_map, loc->map);
			automap_random_cell(g_map);
			automap_free_cells(g_map);
		} else {
			all.h[0] = get_first_hero(g_group);
			for (i = 1; i < cnt; i++)
				all.h[i] = get_next_hero(g_group, all.h[i - 1]);
			for (i = 0; i < cnt; i++)
				make_group(1, all.h[i]);
			for (i = 0; i < cnt; i++) {
				for (j = 0, g = g_groups; j < NR_GROUPS; j++, g = &g_groups[j]) {
					if (group_has_hero(g, all.h[i])) {
						g_group = g;
						automap_init(g_map, loc->map);
						automap_random_cell(g_map);
						automap_free_cells(g_map);
					}
				}
			}
		}
		for (i = 0, g = g_groups; i < NR_GROUPS; i++, g = &g_groups[i]) {
			if (group_has_hero(g, leader)) {
				g_group = g;
				g_group_no = i;
				break;
			}
		}
		cam_set_from_group();
	}
	diary_lock(g_diary);
}

void fig_loot_monsters(Fight far *f)
{
	signed short i;
	signed short id;
	signed short cnt;
	signed short sel;
	signed short more;
	signed short page = 0;
	signed short n;
	signed long money;
	signed short old_mode;
	char far *buf;

	buf = (char far *)mem_alloc(0x4b0);
	do {
		for (i = 0; i < 30; i++)
			g_menu_texts[i] = buf + i * 40;
		i = cnt = 0;
		while ((id = f->loot[i]) != 0 && i < 30) {
			strcpy(g_menu_texts[i++], grammar_name(g_item_names[id]));
			cnt++;
		}
		if (cnt) {
			if (cnt > 15) {
				if (page == 0) {
					more = 15;
					n = 16;
				} else {
					more = cnt;
					n = cnt - 14;
				}
				strcpy(g_menu_texts[more], f->txt[11]);
			} else {
				page = 0;
				more = 0;
				n = cnt;
			}
			old_mode = dlg_set16(g_dialog, 6);
			mouse_show(0);
			sel = dlg_choose(g_dialog, f->txt[14], n, g_menu_texts[page], g_menu_texts[page + 1],
				g_menu_texts[page + 2], g_menu_texts[page + 3], g_menu_texts[page + 4], g_menu_texts[page + 5],
				g_menu_texts[page + 6], g_menu_texts[page + 7], g_menu_texts[page + 8], g_menu_texts[page + 9],
				g_menu_texts[page + 10], g_menu_texts[page + 11], g_menu_texts[page + 12],
				g_menu_texts[page + 13], g_menu_texts[page + 14], g_menu_texts[page + 15]) - 1;
			mouse_hide(0);
			dlg_set16(g_dialog, old_mode);
			if (more && n - 1 == sel) {
				/* "weiter": umblaettern */
				if (page == 0)
					page = 15;
				else
					page = 0;
			}
			if (sel == -2 && f->id == FIGHT_ID_SPECIAL2)
				sel = 0;
			if (sel != -2) {
				if (more == 0 || (more != 0 && n - 1 != sel)) {
					if (!group_give_item(g_group, f->loot[sel + page], 1))
						sel = -2;
					else {
						f->loot[sel + page] = 0;
						for (i = sel + page; i < 29; i++) {
							f->loot[i] = f->loot[i + 1];
							f->loot[i + 1] = 0;
						}
					}
				}
			}
		} else
			sel = -2;
	} while (sel != -2);
	money = f->ducats * 100;
	money += f->silver * 10;
	money += f->heller;
	if (money > 0) {
		money_to_string(g_text_buf2, money);
		sprintf(g_text_output_buf, f->txt[15], g_text_buf2);
		mouse_show(0);
		dlg_message(g_dialog, g_text_output_buf);
		mouse_hide(0);
		group_add_money(g_group, money);
	}
	mem_free(buf);
}

void fig_split_ap(Fight far *f)
{
	signed long ap = 0;
	signed short i;
	signed short j;
	signed short k;
	signed short alive;
	struct enemy far *e;
	struct hero far *h;
	signed short found;
	signed short unknown = 0;
	struct mon_count tab[20];

	memset(tab, 0, sizeof(tab));
	for (i = 0; i < NR_ENEMIES; i++) {
		e = g_enemies + i;
		if (e->mon_id) {
			if (!g_mon_known[e->mon_id])
				unknown = 1;
			if ((unsigned char)e->f39_b5 && (unsigned char)e->renegade) {
				ap++;
				continue;
			}
			found = 0;
			for (j = 0; j < 20; j++) {
				if (tab[j].mon_id == e->mon_id) {
					found = 1;
					tab[j].n++;
					break;
				}
			}
			if (!found) {
				for (j = 0; tab[j].mon_id; j++)
					;
				tab[j].mon_id = e->get_mon_id();
				tab[j].n = 1;
			}
			if (f->id == FIGHT_ID_SPECIAL2)
				ap += 300;
			else if (g_mon_known[e->mon_id]) {
				/* bekannte Art: ein Zehntel */
				k = (unsigned char)e->first_ap / 10;
				ap += k == 0 ? 1L : k;
			} else if (enemy_type_check(e))
				ap += 750;
			else
				ap += (signed long)(unsigned long)(unsigned char)e->first_ap;
		}
	}
	if (f->id != 0x83 && f->id != 0x20) {
		/* Tagebuch: besiegte Gegner */
		if (f->id == FIGHT_ID_SPECIAL2)
			diary_add(g_diary, f->txt[100], 0);
		else if (f->id == FIGHT_ID_SPECIAL)
			diary_add(g_diary, f->txt[104], 0);
		else if (unknown) {
			for (i = 0; i < 20; i++) {
				if (tab[i].n) {
					for (j = i + 1; j < 20; j++) {
						if (tab[j].n) {
							sprintf(g_text_output_buf, "%s", grammar_article(0x4000, tab[i].mon_id, 1));
							sprintf(g_text_buf2, "%s", grammar_article(0x4000, tab[j].mon_id, 1));
							if (!strcmp(g_text_output_buf, g_text_buf2)) {
								tab[i].n += tab[j].n;
								tab[j].mon_id = tab[j].n = 0;
							}
						}
					}
				}
			}
			*g_text_buf2 = found = 0;
			for (i = 0; i < 20; i++)
				if (tab[i].mon_id)
					found++;
			k = found;
			for (i = 0; i < 20; i++) {
				if (tab[i].mon_id) {
					k--;
					if (found - 1 != k) {
						if (k == 0) {
							sprintf(g_text_output_buf, " %s ", g_ttx[422]);
							strcat(g_text_buf2, g_text_output_buf);
						} else
							strcat(g_text_buf2, g_ttx[420]);
					}
					sprintf(g_text_output_buf, "%d %s", tab[i].n,
						grammar_article(0x4000 + (tab[i].n == 1 ? 0 : 4), tab[i].mon_id, 1));
					strcat(g_text_buf2, g_text_output_buf);
				}
			}
			sprintf(g_text_output_buf, f->txt[84], g_text_buf2);
			diary_add(g_diary, g_text_output_buf, 0);
		}
	}
	for (i = 0; i < NR_ENEMIES; i++) {
		e = g_enemies + i;
		if (e->mon_id)
			g_mon_known[e->mon_id] = 1;
	}
	/* AP gleichmaessig auf die lebenden Helden */
	alive = 0;
	h = get_first_hero(g_group);
	do {
		if (!(signed char)h->dead && !(signed char)h->scared)
			alive++;
		h = get_next_hero(g_group, h);
	} while (get_first_hero(g_group) != h);
	if (alive) {
		if (alive > ap)
			ap = alive;
		else
			while (ap % alive)
				ap++;
		h = get_first_hero(g_group);
		do {
			if (!(signed char)h->dead && !(signed char)h->scared)
				add_hero_ap(h, ap / alive);
			h = get_next_hero(g_group, h);
		} while (get_first_hero(g_group) != h);
	}
}
