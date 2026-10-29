/* OVL164 - Overlay O164 (Segment 0x2296 in SCHWEIF.EXE): Sonderereignisse unterwegs (Reiseobjekt o = Travel, Texte
 * g_tev_txt/g_tev_loc_txt, Bilder nvfile_load(g_nvf_heads, 0x196..0x198, ..)). group_has_special_item(): -1, wenn kein
 * Held einen von acht Gegenstaenden (group_find_item) besitzt, sonst 0. travel_scene_31(): vier Texte mit Bild 0x31.
 * shrine_donation(): Spendenschrein - zwei Zustaende g_shrine_state/g_shrine_paid, Spende je Held (get_active_hero,
 * Gott h->god) dlg_input bis 9999, Ansehen g_god_favor[god], Geld group_sub_money; Ergebnis 1 nach Spende.
 * guide_offer(o, n): Fuehrer-Angebot (g_guide_avail, hoechstens 5x g_guide_count) mit Gegnerzahl g_fight_max_enemies = 4/8 je
 * group_sum_ap < 25, Antwort 1: o->guide, 2: Kampf/Wechsel travel_leave/start_fight(0x1b/0x1e/0x1f/0x2b je n)/
 * fight_result/travel_enter, sonst Abbruch (o->route loeschen, group_sub_le_range); bzw. bei g_trial_lost Fragekette mit
 * Tod aller Helden (group_count_alive/group_sub_le, g_hero_died). random_encounter(o, n, k): Zufallsbegegnung
 * (start_fight 0x50..0x53). show_once_anim(o): einmalig (g_once_anim) Bild 0x1b + Animation 0x193.
 * armed_hero_fight(o, n): Held mit Waffe/Gegenstand (get_random_hero_weapon_type/get_random_hero_with_item) nach
 * vorn tauschen (hero_swap), Begegnung 0x33/0x34, zuruecktauschen. npc6_encounter(o): einmalige Szene g_npc6_met mit
 * Menues (ggf. add_npc(6)), sonst bei o->from == 0x2a show_once_anim. spring_event(): Animation 0x199, Menue (1/3:
 * group_refill_water/group_clr_thirst, 3 faellt in 2: Talentprobe TA_WILDNISLEBEN, W6+W6 Gegenstaende 0x122/0x121).
 *
 * BCC -mlarge -3 -Ot -Og -Oi -c -Y OVL164.CPP   -> 5341/5341 Bytes GLEICH (18.10.2026), cobjfix laeuft.
 *   -Oi fuer memset(o->route, 0, 0x28) (rep stosw, daher push di); ohne -Oi 9 Bytes zu lang, ohne -Og 1 Byte zu kurz.
 *   (Namensfassung mit schweif.h: 19.10.2026 GLEICH, LINKGLEICH)
 * - Erster Entwurf sofort GLEICH (Aufbau wie OVL113/OVL114).
 * - group_has_special_item: "if (a >= 0 || .. || h >= 0) return 0; else return -1;" -> erst jl/jmp near, dann
 *   jnl short, zwei tote Epiloge.
 * - Lokale Bmp-Objekte mitten im Rumpf (nach txt_load_list) bzw. je if-Zweig ([bp-22h] geteilt); new Bmp(w, h) mit
 *   Groesse aus nvfile_info, delete -> gb_delete(b, 3).
 * - npc6_encounter: Initialisierungsreihenfolge [bp-4] vor [bp-3] -> "signed char flag = 0;" bei der Deklaration,
 *   "ans = -1;" als Anweisung. spring_event: case 1, case 3 (faellt durch), case 2 -> Rumpfreihenfolge 1, 3, 2.
 * - show_once_anim: Parameter o unbenutzt (Aufruf aus npc6_encounter mit o).
 * - Die Animationsobjekte (O051) werden als C-Aufrufe Anim_ctor(0, ..)/Anim_dtor(p, 3) angelegt und zerstoert.
 * Originalfehler/Auffaelliges: shrine_donation zweiter Zustand zeigt Text g_tev_txt[135] im falschen Zweig (bei
 *   Teilzahlung statt bei Zahlung des ganzen Geldes wie im ersten Zustand); ganze Zahlung erhoeht das Ansehen um
 *   2 * Geld/10, Teilzahlung nur um n; "ans = 1; switch (ans)". guide_offer/random_encounter: n bzw. k ausserhalb
 *   1..4 -> res uninitialisiert an fight_result.
 */
#include <stdio.h>
#include <string.h>
#include "schweif.h"

/* Phase B: eigene Daten des Moduls (ds:6c6c..6c74, dsana) - initialisierte Daten in Adressreihenfolge, danach der Literalpool */

#define NVF_HEADS ((Nvf far *)g_nvf_heads)	/* NVF-Dateiobjekt (O098) */

signed char group_has_special_item(void)
{
	if (group_find_item(g_group, 0x0f) >= 0 || group_find_item(g_group, 0x2b) >= 0 || group_find_item(g_group, 0x36) >= 0 ||
	    group_find_item(g_group, 0x50) >= 0 || group_find_item(g_group, 0x52) >= 0 || group_find_item(g_group, 0x53) >= 0 ||
	    group_find_item(g_group, 0x116) >= 0 || group_find_item(g_group, 0x127) >= 0)
		return 0;
	else
		return -1;
}

void travel_scene_31(void)
{
	signed short w;

	txt_load_list(g_font, 0x13e, 0x8c, &g_tev_buf, &g_tev_txt);
	Bmp b(0x40, 0x40);
	NVF_HEADS->load(0x197, b.data(), 0, 0, 0x31, 0, &w, &w, 320);
	dlg_box_bmp(g_dialog, &b, (char far *)0, g_tev_txt[13], 0);
	dlg_box_bmp(g_dialog, &b, (char far *)0, g_tev_txt[14], 0);
	dlg_box_bmp(g_dialog, &b, (char far *)0, g_tev_txt[15], 0);
	dlg_box_bmp(g_dialog, &b, (char far *)0, g_tev_txt[16], 0);
	g_tev_buf = mem_free(g_tev_buf);
	mem_free(g_tev_txt);
}

signed char shrine_donation(void)
{
	signed long money;
	signed long amt;
	signed long amt10;
	signed short w;
	signed char ans;
	signed char god;
	signed char old;
	struct hero far *h;
	signed char ret = 0;

	txt_load_list(g_font, 0x13e, 0x8c, &g_tev_buf, &g_tev_txt);
	txt_load_list(g_font, 0x144, 0x41, &g_tev_loc_buf, &g_tev_loc_txt);
	old = dlg_set16(g_dialog, 5);
	if (g_shrine_state == 0) {
		Bmp b(0x40, 0x40);
		NVF_HEADS->load(0x198, b.data(), 0, 0, 3, 0, &w, &w, 320);
		ans = dlg_box_bmp(g_dialog, &b, (char far *)0, g_tev_loc_txt[28], 2, g_tev_loc_txt[34], g_tev_loc_txt[35]);
		switch (ans) {
		case 1:
			if (g_shrine_paid == 0)
				ans = dlg_yes_no(g_dialog, g_tev_loc_txt[33]);
			else
				ans = 0;
			while (ans == 1) {
				if ((money = g_group->money) == 0) {
					dlg_message(g_dialog, g_tev_loc_txt[36]);
				} else {
					h = get_active_hero(g_group, g_tev_loc_txt[37]);
					if (h != 0) {
						god = h->god;
						money_to_string(g_text_buf2, money);
						sprintf(g_text_output_buf, g_tev_loc_txt[38], h->name, g_ttx[god + 4], g_text_buf2);
						amt = dlg_input(g_dialog, 4, g_text_output_buf, 0, 9999, 0);
						amt10 = amt * 10;
						if (amt10 > 0) {
							if (amt10 >= money) {
								dlg_message(g_dialog, g_tev_txt[135]);
								g_god_favor[god] += money / 10 * 2;
								money = g_group->money;
							} else {
								g_god_favor[god] += amt;
								money = amt10;
							}
							group_sub_money(g_group, money);
						}
					}
				}
				if (g_group->money == 0)
					ans = 0;
				else
					ans = dlg_yes_no(g_dialog, g_tev_loc_txt[39]);
			}
			ret = 1;
			g_shrine_state = 1;
			diary_add(g_diary, g_tev_txt[124], 0);
			break;
		case 2:
			break;
		}
	} else if (g_shrine_state == 1) {
		Bmp b(0x40, 0x40);
		NVF_HEADS->load(0x198, b.data(), 0, 0, 4, 0, &w, &w, 320);
		dlg_box_bmp(g_dialog, &b, (char far *)0, g_tev_loc_txt[29], 0);
		ans = 1;
		switch (ans) {
		case 1:
			group_get_ill(g_group, 4, 2);
			if (g_shrine_paid == 0)
				ans = dlg_yes_no(g_dialog, g_tev_loc_txt[40]);
			else
				ans = 0;
			while (ans == 1) {
				if ((money = g_group->money) == 0) {
					dlg_message(g_dialog, g_tev_loc_txt[36]);
					ans = 0;
				} else {
					h = get_active_hero(g_group, g_tev_loc_txt[37]);
					if (h != 0) {
						god = h->god;
						money_to_string(g_text_buf2, money);
						sprintf(g_text_output_buf, g_tev_loc_txt[38], h->name, g_ttx[god + 4], g_text_buf2);
						amt = dlg_input(g_dialog, 4, g_text_output_buf, 0, 9999, 0);
						amt10 = amt * 10;
						if (amt10 > 0) {
							if (amt10 >= money) {
#if FIX_0146
								/* Fix #146: Text 135 bei Zahlung des ganzen Geldes (wie im ersten Zustand) */
								dlg_message(g_dialog, g_tev_txt[135]);
								g_god_favor[god] += money / 10 * 2;
								money = g_group->money;
							} else {
								g_god_favor[god] += amt;
#else
								g_god_favor[god] += money / 10 * 2;
								money = g_group->money;
							} else {
								dlg_message(g_dialog, g_tev_txt[135]);
								g_god_favor[god] += amt;
#endif
								money = amt10;
							}
							group_sub_money(g_group, money);
						}
					}
				}
				if (g_group->money == 0)
					ans = 0;
				else
					ans = dlg_yes_no(g_dialog, g_tev_loc_txt[39]);
			}
			ret = 1;
			g_shrine_paid = 1;
			g_shrine_state = 0;
			diary_add(g_diary, g_tev_txt[125], 0);
			break;
		case 2:
			break;
		}
	}
	dlg_set16(g_dialog, old);
	g_tev_buf = mem_free(g_tev_buf);
	mem_free(g_tev_txt);
	mem_free(g_tev_loc_buf);
	mem_free(g_tev_loc_txt);
	if (ret == 1)
		return 1;
	else
		return 0;
}

void guide_offer(Travel far *o, signed char n)
{
	signed char ans;
	signed char old;
	signed char res;
	signed short w;
	signed short ww;
	signed short hh;
	Bmp far *b;
	signed long l = 0;

	if (g_guide_avail == 1 && g_guide_count < 5) {
		g_guide_won = 0;
		g_guide_count += 1;
		nvfile_info(g_nvf_heads, 0x196, 9, &ww, &hh);
		b = new Bmp(ww, hh);
		NVF_HEADS->load(0x196, b->data(), 0, 0, 9, 0, &w, &w, 320);
		l = group_sum_ap(g_group);
		if (l < 25)
			g_fight_max_enemies = 4;
		else
			g_fight_max_enemies = 8;
		sprintf(g_text_output_buf, g_tev_txt[72], g_fight_max_enemies);
		g_dlg_box_flag = 0;
		old = dlg_set16(g_dialog, 6);
		ans = dlg_box_bmp(g_dialog, b, " ", g_text_output_buf, 3, g_tev_txt[73], g_tev_txt[74], g_tev_txt[75]);
		dlg_set16(g_dialog, old);
		delete b;
		switch (ans) {
		case 1:
			o->action = 1;
			res = 1;
			o->ev_half = 0;
			break;
		case 2:
			travel_leave(o);
			g_fight_flag = 1;
			switch (n) {
			case 1:
				res = start_fight(0x1b, 0);
				break;
			case 2:
				res = start_fight(0x1e, 0);
				break;
			case 3:
				res = start_fight(0x1f, 0);
				break;
#if FIX_0146
			/* Fix #146: n ausserhalb 1..4: kein Kampf, kein fight_result mit Muellwert */
			case 4:
				res = start_fight(0x2b, 0);
				break;
			default:
				res = -2;
				break;
			}
			if (res != -2)
				fight_result(o, res);
#else
			case 4:
				res = start_fight(0x2b, 0);
				break;
			}
			fight_result(o, res);
#endif
			travel_enter(o);
			if (res == 0)
				g_guide_won = 1;
			break;
		default:
			dlg_message(g_dialog, g_tev_txt[76]);
			g_travel_state = -1;
			g_gashok_arrested = 1;
			memset(o->route, 0, 0x28);
			g_group->route_from = 0;
			g_group->route_to = 0;
			group_sub_le_range(g_group, 2, 10);
			break;
		}
	} else if (g_trial_lost == 1) {
		nvfile_info(g_nvf_heads, 0x196, 9, &ww, &hh);
		b = new Bmp(ww, hh);
		NVF_HEADS->load(0x196, b->data(), 0, 0, 9, 0, &w, &w, 320);
		g_dlg_box_flag = 0;
		old = dlg_set16(g_dialog, 6);
		dlg_box_bmp(g_dialog, b, " ", g_tev_txt[127], 0);
		ans = dlg_box_bmp(g_dialog, b, " ", g_tev_txt[128], 2, g_tev_txt[129], g_tev_txt[130]);
		if (ans == 1) {
			ans = dlg_box_bmp(g_dialog, b, " ", g_tev_txt[131], 2, g_tev_txt[1], g_tev_txt[2]);
			if (ans == 1) {
				dlg_message(g_dialog, g_tev_txt[132]);
				while (group_count_alive(g_group))
					group_sub_le(g_group, 0x32);
				g_hero_died = 1;
			}
		}
		if (ans == 2) {
			dlg_message(g_dialog, g_tev_txt[76]);
			g_travel_state = -1;
			g_gashok_arrested = 1;
			memset(o->route, 0, 0x28);
			g_group->route_from = 0;
			g_group->route_to = 0;
			group_sub_le_range(g_group, 2, 10);
		}
		dlg_set16(g_dialog, old);
		delete b;
	}
}

void random_encounter(Travel far *o, signed char n, signed char k)
{
	signed char res;

	dlg_message(g_dialog, g_tev_txt[77]);
	g_fight_max_enemies = n;
	travel_leave(o);
	g_fight_flag = 1;
	switch (k) {
	case 1:
		res = start_fight(0x50, 0);
		break;
	case 2:
		res = start_fight(0x51, 0);
		break;
	case 3:
		res = start_fight(0x52, 0);
		break;
#if FIX_0146
	/* Fix #146: k ausserhalb 1..4: kein Kampf, kein fight_result mit Muellwert */
	case 4:
		res = start_fight(0x53, 0);
		break;
	default:
		res = -2;
		break;
	}
	if (res != -2)
		fight_result(o, res);
#else
	case 4:
		res = start_fight(0x53, 0);
		break;
	}
	fight_result(o, res);
#endif
	travel_enter(o);
}

void show_once_anim(Travel far *o)
{
	signed short w;
	signed char r;
	void far *p;

	r = group_find_item(g_group, 0x117);
	if (r >= 0 && g_once_anim == 0) {
		Bmp b(0x40, 0x40);
		NVF_HEADS->load(0x197, b.data(), 0, 0, 0x1b, 0, &w, &w, 320);
		dlg_box_bmp(g_dialog, &b, (char far *)0, g_tev_txt[78], 0);
		g_once_anim = 1;
		p = Anim_ctor(0, 0x193, 0, 0);
		dlg_box_anim(g_dialog, p, (char far *)0, g_tev_txt[79], 0);
		Anim_dtor(p, 3);
		diary_add(g_diary, g_tev_txt[126], 0);
	}
}

void armed_hero_fight(Travel far *o, signed char n)
{
	struct hero far *h1;
	struct hero far *h2;
	signed char r;
	signed short flag;

	g_armed_fight_lost = 1;
	flag = 0;
	h1 = get_random_hero_weapon_type(g_group, 4);
	h2 = get_first_active_hero(g_group);
	if (h1 == 0) {
		h1 = get_random_hero_with_item(g_group, 0x0b, 3);
		if (h1 == 0) {
			h1 = get_random_hero_with_item(g_group, 0x10, 3);
			if (h1 == 0) {
				h1 = get_random_hero_with_item(g_group, 0x21, 3);
				if (h1 == 0)
					h1 = get_random_hero_with_item(g_group, 0x102, 3);
			}
		}
	}
	if (h1 != 0) {
		if (h1 != h2) {
			hero_swap(g_group, h1, h2);
			flag = 1;
		}
		travel_leave(o);
		g_fight_end = 1;
		g_fight_flag = 1;
		if (n == 2 || n == 4)
			r = start_fight(0x33, 1);
		else
			r = start_fight(0x34, 1);
		if (r == 0)
			g_armed_fight_lost = 0;
		if (r == 1)
			r = 0;
		fight_result(o, r);
		travel_enter(o);
		if (flag)
			hero_swap(g_group, h1, h2);
	}
}

void npc6_encounter(Travel far *o)
{
	signed short w;
	signed char ans;
	signed char flag = 0;

	ans = -1;
	if (g_npc6_met == 0) {
		Bmp b1(0x40, 0x40);
		NVF_HEADS->load(0x197, b1.data(), 0, 0, 0x1a, 0, &w, &w, 320);
		dlg_box_bmp(g_dialog, &b1, (char far *)0, g_tev_txt[86], 0);
		while (ans == -1)
			ans = dlg_box_bmp(g_dialog, &b1, (char far *)0, g_tev_txt[87], 3, g_tev_txt[88], g_tev_txt[89], g_tev_txt[90]);
		Bmp b2(0x40, 0x40);
		NVF_HEADS->load(0x198, b2.data(), 0, 0, 2, 0, &w, &w, 320);
		switch (ans) {
		case 1:
			dlg_box_bmp(g_dialog, &b2, (char far *)0, g_tev_txt[91], 0);
			timewarp_minutes(g_date, 30L);
			group_sub_hunger_thirst(g_group, 0x14, 0x14);
			flag = 1;
			break;
		case 2:
			ans = -1;
			while (ans == -1)
				ans = dlg_box_bmp(g_dialog, &b2, (char far *)0, g_tev_txt[92], 2, g_tev_txt[93], g_tev_txt[94]);
			switch (ans) {
			case 1:
				dlg_box_bmp(g_dialog, &b2, (char far *)0, g_tev_txt[95], 0);
				flag = 1;
				break;
			case 2:
				dlg_box_bmp(g_dialog, &b2, (char far *)0, g_tev_txt[96], 0);
				break;
			}
			break;
		case 3:
			dlg_box_bmp(g_dialog, &b2, (char far *)0, g_tev_txt[97], 0);
			break;
		}
		g_npc6_met = 1;
		if (flag == 1)
			add_npc(6);
	} else if (o->from == 0x2a) {
		show_once_anim(o);
	}
}

void spring_event(void)
{
	signed char ans;
	signed char r;
	struct hero far *h;
	signed short n;
	void far *p;
	signed short s;

	p = Anim_ctor(0, 0x199, 0, 0);
	ans = dlg_box_anim(g_dialog, p, (char far *)0, g_tev_txt[98], 3, g_tev_txt[99], g_tev_txt[100], g_tev_txt[101]);
	Anim_dtor(p, 3);
	switch (ans) {
	case 1:
		group_refill_water(g_group);
		group_clr_thirst(g_group);
		break;
	case 3:
		group_refill_water(g_group);
		group_clr_thirst(g_group);
	case 2:
		s = group_find_item(g_group, 0x1c);
		if (s < 0)
			s = group_find_item(g_group, 0x61);
		if (s >= 0) {
			h = get_best_hero_skill(g_group, TA_WILDNISLEBEN);
			n = test_skill(h, TA_WILDNISLEBEN, 0);
			if (n > 0) {
				dlg_message(g_dialog, g_tev_txt[103]);
				r = random_schweif(6) + random_schweif(6);
				if (r <= 10)
					group_give_item(g_group, 0x122, r);
				else
					group_give_item(g_group, 0x121, 1);
			} else {
				dlg_message(g_dialog, g_tev_txt[104]);
			}
		} else {
			dlg_message(g_dialog, g_tev_txt[102]);
		}
		break;
	}
}
