/* OVL005 - Overlay O005 (Segment 0x1f11 in SCHWEIF.EXE): Zeitgeber-Ereignisse (gerufen aus SEG010 do_timers/
 * heroes_tick und SEG010-Hauptschleife, O086/O093/O095 fuer ap_for_level, O113 fuer check_level_up).
 * timers_hourly(p): Zaehler g_guest_lock[7]/g_guest_lock2[3]/g_guest_lock_t2..g_guest_lock_t24/g_dng13_smoke_timer herunterzaehlen, location_countdown_msgs bei
 *   g_binge_torches_lit, Zaehler g_npc8_days (NSC group_get_npc mit npc_id == 8 und nicht in Haft), bei 30 g_npc_months = 99.
 * timers_4hourly(p): conv_dec_counter(g_dialog2, 0..9). timers_daily(p): Wx_next(g_weather); Monatsanfang -
 *   Tabellen g_temple_desecrated/g_smith_banned/g_shop_banned leeren, Wahrscheinlichkeiten aus Datei g_file_3 (Satz 4..12, Byte +4) nach
 *   g_inn_room_rolls[9], Wuerfe nach g_shop_food_rolls[5]/g_shed_free_today/g_dragan_room_today, Zustaende g_vindaria_state/g_gavron_chase, Tabelle g_market_rolls (11 Bytes, bis
 *   -1) neu
 *   auswuerfeln (am 16.12.17 fest 3), diverse Merker.
 * hero_hunger_thirst(p, h): Hunger und Durst je Stunde - Essen/Trinken aus dem Gepaeck (group_find_food/
 *   group_find_drink + lokales ItemUse-Objekt mit item_consume, eigener Held hero_find_food/hero_find_drink),
 *   Zunahme per inline add_hunger/add_thirst (Klemmen 0..100), Verhungern hunger_thirst_damage, Meldungen
 *   txt[36..46] der Textdatei 0x81 (Merker f4f0 per inline set_f4f0), Meldung txt[42] bei Wechsel von Bit
 *   unconscious. hero_disease_tick(p, h): je Tag disease_effect bei Krankheit, recipe_timer - 1.
 * ap_for_level(lvl): AP-Bedarf fuer Stufe lvl (rekursiv, 100 * (lvl - 1) + ap_for_level(lvl - 1)).
 *   check_level_up(): Stufenanstieg aller Helden (Stufe < 21 und ap_for_level(Stufe + 1) <= AP) ueber Inv_ctor/
 *   Lvl_run, danach prepare_spell_icons.
 * inn_days_tick(): Herbergstage g_inn_days, bei 0 g_inn_room/g_inn_town/g_inn_loc loeschen. npc_pay_tick(): NSC mit
 *   npc_id == 7 - Zahlung 10 aus der Gruppenkasse (group_sub_money) oder Abschied (g_npc_months = 99,
 *   groups_check_npc). jail_tick(): Haft (jail) aller Helden der sechs Gruppen herunterzaehlen, befreite Gruppen
 *   (Liste als Kopie von g_group_ptrs_null) auf Position 12/0x1c setzen (set_x/set_y verschachtelt, screen_x/
 *   screen_y, f1c = 1), Meldung g_ttx[411].
 * hero_fur_tick(h): Pelz an den Haenden aus Dungeon 13 (hero.fur_hands 1..5: Meldungen BINGE.LTX[22..26] (Datei
 *   0x2f9), change_attrib(FF)/
 *   sub_hero_le/screen_redraw, dann Auswahl txt[19] mit Gegenmitteln 0x1e bzw. 0x16/0x41 im Gepaeck der Gruppe;
 *   -1: Erholung bis FF wieder auf dem Grundwert).
 * location_countdown_msgs(): Zaehler g_binge_anvil_hours ab 11 mit Meldungen je Ort (f22). dng13_smoke_damage():
 *   Dungeon 13, Qualm - alle
 *   Helden sub_hero_le(3..5), g_refresh_portraits = 1. show_daily_hint(): Meldung txt[117] der Textdatei 0x21c bei
 *   Gruppe f20 == 4.
 *
 * BCC -mlarge -3 -Ot -Og -c -Y OVL005.CPP   -> 6338/6338 Bytes GLEICH (18.10.2026, erste Fassung), cobjfix laeuft
 *   (g_group_ptrs_null -> _s_00e3+0xd). Ohne -Og 13 Bytes zu kurz.
 *   (Namensfassung mit schweif.h: 19.10.2026 GLEICH)
 * - Gruppe (struct group): inline get_loctype() (cbw / cmp ax,3), set_x/set_y "old_x = x; return x = v;"
 *   verschachtelt aufgerufen: set_x(set_x(12)) -> Zwischenwert [bp-10h] als Parameterkopie des aeusseren Aufrufs
 *   (wie O038 set20).
 * - Lokales ItemUse o(h, q, i, w, f) jeweils in eigenem Block: Ktor O047_0000, Dtor O047_0124(&o, 2) am Blockende;
 *   in hero_fur_tick mit q = 0 (zwei push 0).
 * - Klemm-Addierer add_hunger/add_thirst mit variablem Wert -> Bytekopien [bp-1fh..-22h]; danach steht im
 *   Original noch einmal "if (hunger > 100) hunger = 100;" ausserhalb.
 * - Bitfelder unsigned short: "!(signed char)h->dead" (and / cbw / or ax,ax), "(signed char)h->unconscious" im if
 *   (or al,al), "(unsigned char)h->unconscious != was_unconscious" (cmp al,[bp-11h]); -Og cacht das Byte in
 *   [bp-24h].
 * - "(signed short)h->fur_hands == -1" -> cbw / cmp ax,0ffffh; "!!grp->nr_heroes" und "(signed short)g_group->nr_heroes
 *   > 1" fuer die cbw-Formen.
 * - Verschachtelter ?:-Formatstring in sprintf (msg 1..8 -> txt[36..41,45,46]) -> mov dx/mov ax + jmp.
 * - txt_buf/txt der Textdateien als Locals auf Funktionsebene hinter n (hero_hunger_thirst), die ItemUse-Objekte
 *   als grosse Locals.
 * Auffaelliges im Original: hero_hunger_thirst - die Zunahme 2/(2*g_resting+1) bzw. 1/(..) ist bei g_resting >= 1 immer
 *   0; hero_fur_tick prueft das Gegenmittel 0x16 zweimal (vor und nach dem Ersatzmittel 0x41).
 */
#include <mem.h>
#include <stdio.h>
#include "schweif.h"

/* Phase B: eigene Daten des Moduls (ds:00f0..0108, dsana) - initialisierte Daten in Adressreihenfolge */
struct group_ptrs6 g_group_ptrs_null = { { 0, 0, 0, 0, 0, 0 } };

void location_countdown_msgs(void);

void timers_hourly(signed short far *p)
{
	register signed short n;
	register signed short i;
	struct hero far *h;

	for (n = 0; n < g_timer_hours; n++) {
		for (i = 0; i < 7; i++)
			if (g_guest_lock[i] > 0)
				g_guest_lock[i]--;
		for (i = 0; i < 3; i++)
			if (g_guest_lock2[i] > 0)
				g_guest_lock2[i]--;
	}
	if (g_guest_lock_t2 > 0)
		g_guest_lock_t2--;
	if (g_guest_lock_t19 > 0)
		g_guest_lock_t19--;
	if (g_guest_lock_t23 > 0)
		g_guest_lock_t23--;
	if (g_guest_lock_t24 > 0)
		g_guest_lock_t24--;
	if (g_binge_torches_lit != 0)
		location_countdown_msgs();
	if (g_dng13_smoke_timer != 0)
		g_dng13_smoke_timer--;
	if (g_npc8_days != 0) {
		h = group_get_npc(g_group);
		if (h != 0 && h->npc_id == 8 && !h->jail)
			g_npc8_days++;
		if (g_npc8_days == 30)
			g_npc_months = 99;
	}
}

void timers_4hourly(signed short far *p)
{
	register signed short n;
	register signed short i;

	for (n = 0; n < g_timer_4hours; n++)
		for (i = 0; i < 10; i++)
			conv_dec_counter(g_dialog2, i);
}

void timers_daily(signed short far *p)
{
	signed short v;
	struct chance_rec rec;
	register signed short i;

	Wx_next(g_weather);
	memset(g_temple_desecrated, 0, 0x1d);
	memset(g_smith_banned, 0, 0x0c);
	memset(g_shop_banned, 0, 0x1a);
	for (i = 0; i < 9; i++) {
		arc_open(g_file_3, 0x200);
		arc_seek(g_file_3, (signed long)(i + 4) * 6, 0);
		arc_read(g_file_3, &rec, 6);
		arc_close(g_file_3);
		if (random_schweif(100) <= rec.chance)
			g_inn_room_rolls[i] = 1;
		else
			g_inn_room_rolls[i] = 0;
	}
	for (i = 0; i < 5; i++) {
		if (random_schweif(100) <= 20)
			g_shop_food_rolls[i] = 1;
		else
			g_shop_food_rolls[i] = 0;
	}
	if (random_schweif(100) <= 60)
		g_shed_free_today = 1;
	else
		g_shed_free_today = 0;
	if (random_schweif(100) <= 50)
		g_dragan_room_today = 1;
	else
		g_dragan_room_today = 0;
	if (g_vindaria_state == 2)
		g_vindaria_state = 3;
	else if (g_vindaria_state == 3)
		g_vindaria_state = 1;
	if (g_gavron_chase < 0 && g_gavron_chase != -4) {
		g_gavron_chase--;
		if (g_gavron_chase == -3)
			g_gavron_chase = 0;
	}
	g_peddler_banned = 0;
	for (i = 0; g_market_rolls[i].base != -1; i++) {
		if (g_date_day == 16 && g_date_month == 12 && g_date_year == 17)
			v = 3;
		else
			v = random_interval(g_market_rolls[i].base, 20);
		g_market_rolls[i].value = v;
		g_market_rolls[i].flag8 = random_schweif(100) >= 50 ? 1 : 0;
		g_market_rolls[i].flag9 = random_schweif(100) >= 50 ? 1 : 0;
		g_market_rolls[i].flag10 = random_schweif(100) >= 50 ? 1 : 0;
	}
	if (g_altrogolan_absent > 0)
		g_altrogolan_absent = 0;
	if (g_guide_avail > 1)
		g_guide_avail--;
	if (g_group->town == 4 && g_low_stone_days != 0 && (g_low_stone_days += 1) >= 12)
		g_game_state = 4;
	if (g_port_talk == 1)
		g_port_talk = 0;
	g_dng06_beer_drunk = 0;
	if (g_inn_vision_state == 1)
		g_inn_vision_state = 0;
}

void hero_hunger_thirst(signed short far *p, struct hero far *h)
{
	struct group far *grp;
	struct hero far *partner;
	signed short hero_no;
	signed short slot;
	signed short id;
	signed short msg = 0;
	signed char was_unconscious;
	signed long n;
	void far *txt_buf;
	char far * far *txt;

	partner = h;
	if (check_hero(h) && h->inv[10].item_id == 0xc5)
		sub_hero_le(h, g_timer_hours);
	for (n = 0; n < g_timer_hours; n++) {
		if ((signed short)h->fur_hands == -1)
			hero_fur_tick(h);
		was_unconscious = h->unconscious;
		if (!(signed char)h->dead && !(signed char)h->petrified && !h->jail) {
			grp = &g_groups[h->group_no];
			if (grp->get_loctype() != LOC_INN) {	/* in der Herberge kein Hunger/Durst */
				if (group_find_item(grp, 0xb8) == -1) {
					if (get_item_pos(h, 0xaf) == -1) {
						if (h->hunger > 90) {
							if ((signed char)h->unconscious)
								g_hero_excluded = get_hero_index(h);
							if (group_find_food(grp, h, &hero_no, &slot, &id)) {
								partner = &g_heroes[hero_no];
								{
									ItemUse o(h, partner, slot, id, 0);

									item_consume(&o);
									if (h == partner) {
										if (!hero_find_food(h, &slot, &id))
											msg = 6;
										else if (h->hunger < 100)
											msg = 4;
									} else
										msg = 7;
								}
							}
						}
						if (h->hunger < 100) {
#if FIX_0179
							/* Fix #179: Zunahme bei Rast anteilig per Zufall statt ganzzahlig 0 */
							{
								signed short d_ = g_resting * 2 + 1;
								signed short v_ = h->hunger_timer <= 0 ? 2 : 1;

								h->add_hunger(v_ / d_ + (d_ > 1 && random_schweif(d_) <= v_ % d_ ? 1 : 0));
							}
#else
							if (h->hunger_timer <= 0)
								h->add_hunger(2 / (g_resting * 2 + 1));
							else
								h->add_hunger(1 / (g_resting * 2 + 1));
#endif
							if (h->hunger > 100)
								h->hunger = 100;
						} else if (h->hunger_timer <= 0) {
							if (g_resting == 0)
								hunger_thirst_damage(h);
							msg = 2;
						}
					} else
						h->hunger = 0;
				} else
					h->hunger = 0;
				if (group_find_item(grp, 0xb9) == -1 && !grp->town) {
					if (get_item_pos(h, 0xaf) == -1) {
						if (h->thirst > 90) {
							if ((signed char)h->unconscious)
								g_hero_excluded = get_hero_index(h);
							if (group_find_drink(grp, h, &hero_no, &slot, &id)) {
								partner = &g_heroes[hero_no];
								{
									ItemUse o(h, partner, slot, id, 0);

									item_consume(&o);
									if (h == partner) {
										if (!hero_find_drink(h, &slot, &id))
											msg = 5;
										else if (h->thirst < 100)
											msg = 3;
									} else
										msg = 8;
								}
							}
						}
						if (h->thirst < 100) {
#if FIX_0179
							/* Fix #179: Zunahme bei Rast anteilig per Zufall statt ganzzahlig 0 */
							{
								signed short d_ = g_resting * 2 + 1;
								signed short v_ = h->hunger_timer <= 0 ? 4 : 2;

								h->add_thirst(v_ / d_ + (d_ > 1 && random_schweif(d_) <= v_ % d_ ? 1 : 0));
							}
#else
							if (h->hunger_timer <= 0)
								h->add_thirst(4 / (g_resting * 2 + 1));
							else
								h->add_thirst(2 / (g_resting * 2 + 1));
#endif
							if (h->thirst > 100)
								h->thirst = 100;
						} else if (h->hunger_timer <= 0) {
							if (g_resting == 0)
								hunger_thirst_damage(h);
							msg = 1;
						}
					} else
						h->thirst = 0;
				} else
					h->thirst = 0;
			}
		}
		if (msg != 0 && g_silent_mode == 0 && g_in_fight == 0 && (!g_group->loctype || g_resting == 0)) {
			if (!(signed char)h->dead && !(signed char)h->petrified &&
				(!group_get_on_route(g_group) || h->f4f0 != msg)) {
				txt_load_list(g_font, 0x81, 0x32, &txt_buf, &txt);
				sprintf(g_text_output_buf,
					msg == 1 ? txt[36] : msg == 2 ? txt[37] : msg == 3 ? txt[38] : msg == 4 ? txt[39] :
					msg == 5 ? txt[40] : msg == 6 ? txt[41] : msg == 7 ? txt[45] : txt[46],
					h->name, grammar_pronoun(h->sex, 1, 2), partner->name);
				h->set_f4f0(msg);
				dlg_message(g_dialog, g_text_output_buf);
				mem_free(txt_buf);
				mem_free(txt);
			}
			msg = 0;
		}
		if ((unsigned char)h->unconscious != was_unconscious && g_silent_mode == 0) {
			txt_load_list(g_font, 0x81, 0x32, &txt_buf, &txt);
			sprintf(g_text_output_buf, txt[42], h->name);
			dlg_message(g_dialog, g_text_output_buf);
			mem_free(txt_buf);
			mem_free(txt);
			if (g_pp20_index == 14)
				g_refresh = 1;
		}
	}
}

void hero_disease_tick(signed short far *p, struct hero far *h)
{
	signed long n;

	for (n = 0; n < g_timer_days; n++) {
		if (!(signed char)h->dead && (hero_is_diseased(h) || hero_is_diseased1(h)))
			disease_effect(h);
		if (h->recipe_timer > 0)
			h->recipe_timer--;
		else
			;
	}
}

signed long ap_for_level(register signed short lvl)
{
	signed long ap = 0;
	signed long base = 100;

	if (lvl <= 2)
		ap = base;
	else
		ap = base * (lvl - 1) + ap_for_level(lvl - 1);
	return ap;
}

void check_level_up(void)
{
	register signed short again;
	register signed short i;
	struct hero far *h;
	void far *sheet;
	signed short any = 0;

	if (g_dialog_depth == 0) {
		do {
			again = 0;
			for (i = 0, h = g_heroes; i <= 6; i++, h++) {
				if (h->type != 0 && !(signed char)h->dead && h->level < 21 && ap_for_level(h->level + 1) <= h->ap) {
					snd_obj_stop(g_snd_music);
					ani_stop(g_screen_pic);
					swap_out_screen();
					sheet = Inv_ctor(0, 0);
					Lvl_run(sheet, h);
					Inv_dtor(sheet, 3);
					again = 1;
					any = 1;
				}
			}
		} while (again);
	}
	if (any)
		prepare_spell_icons();
}

void inn_days_tick(void)
{
	if (g_inn_days != 0) {
		g_inn_days--;
		if (g_inn_days == 0)
			g_inn_room = g_inn_town = g_inn_loc = 0;
	}
}

void npc_pay_tick(void)
{
	struct hero far *h;
	void far *txt_buf;
	char far * far *txt;

	h = group_get_npc(g_group);
	if (h != 0 && h->npc_id == 7 && check_hero3(h)) {
		txt_load_list(g_font, 0x81, 0x32, &txt_buf, &txt);
		if (g_group->money >= 10) {
			group_sub_money(g_group, 10);
			dlg_message(g_dialog, txt[43]);
		} else {
			dlg_message(g_dialog, txt[48]);
			g_npc_months = 99;
			groups_check_npc();
		}
		mem_free(txt_buf);
		mem_free(txt);
	}
}

void jail_tick(void)
{
	struct group_ptrs6 freed = g_group_ptrs_null;
	signed short i;
	signed short days_left;
	struct hero far *h;
	struct group far *grp;

	for (i = 0, grp = g_groups; i < 6; i++, grp++) {
		if (!!grp->nr_heroes) {
			h = get_first_hero(grp);
			do {
				days_left = h->jail;
				if (days_left != 0) {
					days_left--;
					h->set_jail(days_left);
					if (days_left == 0)
						freed.p[i] = grp;
				}
				h = get_next_hero(grp, h);
			} while (get_first_hero(grp) != h);
		}
	}
	g_jail_freed = 0;
	for (i = 0; i < 6; i++) {
		if (freed.p[i]) {
			freed.p[i]->screen_x = 0x834;
			freed.p[i]->set_x(freed.p[i]->set_x(12));
			freed.p[i]->screen_y = 0x12b4;
			freed.p[i]->set_y(freed.p[i]->set_y(0x1c));
			freed.p[i]->dir = 1;
			g_jail_freed = 1;
		}
	}
	if (g_jail_freed != 0)
		dlg_message(g_dialog, g_ttx[411]);
}

void hero_fur_tick(struct hero far *h)
{
	void far *txt_buf;
	char far * far *txt;
	register signed short stage;
	register signed short answer;

	if (!(signed char)h->dead) {
		stage = h->fur_hands;
		if (stage >= 1 && stage <= 5) {
			txt_load_list(g_font, 0x2f9, 0x3c, &txt_buf, &txt);
			switch (stage) {
			case 1:
				sprintf(g_text_output_buf, txt[22], h->name, grammar_pronoun(h->sex, 1, 1));
				dlg_message(g_dialog, g_text_output_buf);
				change_attrib(h, ATTRIB_FF, -1);
				sub_hero_le(h, 1);
				screen_redraw(g_screen);
				h->fur_hands = 2;
				break;
			case 2:
				sprintf(g_text_output_buf, txt[23], hero_name_gen(h, 1));
				dlg_message(g_dialog, g_text_output_buf);
				change_attrib(h, ATTRIB_FF, -1);
				sub_hero_le(h, 1);
				screen_redraw(g_screen);
				h->fur_hands = 3;
				break;
			case 3:
				sprintf(g_text_output_buf, txt[24], hero_name_gen(h, 1));
				dlg_message(g_dialog, g_text_output_buf);
				change_attrib(h, ATTRIB_FF, -1);
				sub_hero_le(h, 1);
				screen_redraw(g_screen);
				h->fur_hands = 4;
				break;
			case 4:
				sprintf(g_text_output_buf, txt[25], h->name, grammar_pronoun(h->sex, 1, 1));
				dlg_message(g_dialog, g_text_output_buf);
				change_attrib(h, ATTRIB_FF, -1);
				sub_hero_le(h, 1);
				screen_redraw(g_screen);
				if (h->attrib[ATTRIB_FF].cur == 0)
					h->fur_hands = 5;
				break;
			case 5:
				sprintf(g_text_output_buf, txt[26], h->name, grammar_pronoun(h->sex, 0, 0));
				dlg_message(g_dialog, g_text_output_buf);
				sub_hero_le(h, 1);
				screen_redraw(g_screen);
				break;
			}
			answer = dlg_choose(g_dialog, txt[19], 3, txt[27], txt[28], txt[29]);
			if (answer == 1) {
				dlg_message(g_dialog, txt[30]);
				sub_hero_le(h, 1);
				screen_redraw(g_screen);
			} else if (answer == 2) {
				signed short k;
				signed short slot;

				k = group_find_item(g_group, 0x1e);
				if (k >= 0) {
					dlg_message(g_dialog, txt[31]);
					sub_hero_le(h, 1);
					screen_redraw(g_screen);
					h = g_group->heroes[k];
					slot = get_item_pos(h, 0x1e);
					h->inv[slot].quantity--;
				} else
					dlg_message(g_dialog, txt[21]);
			} else if (answer == 3) {
				if (group_find_item(g_group, 0x16) < 0) {
					signed short k;
					struct hero far *helper;

					k = group_find_item(g_group, 0x41);
					if (k < 0)
						dlg_message(g_dialog, txt[20]);
					else {
						helper = g_group->heroes[k];
						{
							ItemUse o(helper, 0, k, 0x41, 1);

							o.use();
						}
					}
				}
				if (group_find_item(g_group, 0x16) >= 0) {
					struct hero far *helper;

					if ((signed short)g_group->nr_heroes > 1) {
						helper = get_first_hero(g_group);
						if (helper == h)
							helper = get_second_active_hero(g_group);
						sprintf(g_text_output_buf, txt[32], helper->name, h->name);
					} else
						sprintf(g_text_output_buf, txt[32], h->name, grammar_pronoun(h->sex, 0, 0));
					dlg_message(g_dialog, g_text_output_buf);
					sub_hero_le(h, random_schweif(4) + 4);
					screen_redraw(g_screen);
					h->fur_hands = -1;
				}
			}
			mem_free(txt_buf);
			mem_free(txt);
		} else if (stage == -1) {
			if (h->attrib[ATTRIB_FF].cur < h->attrib[ATTRIB_FF].normal)
				change_attrib(h, ATTRIB_FF, 1);
			else
				h->fur_hands = 0;
		}
	}
}

void location_countdown_msgs(void)
{
	void far *txt_buf;
	char far * far *txt;

	if (g_binge_anvil_hours >= 11) {
		txt_load_list(g_font, 0x2f9, 0x3c, &txt_buf, &txt);
		if (g_binge_anvil_hours == 11) {
			if (g_group->dungeon == 10)
				dlg_message(g_dialog, txt[0]);
			else if (g_group->dungeon == 11)
				dlg_message(g_dialog, txt[1]);
		} else {
			if (g_group->dungeon == 11)
				dlg_message(g_dialog, txt[2]);
			g_binge_torches_lit = 0;
			g_binge_anvil_hours = -1;
		}
		mem_free(txt_buf);
		mem_free(txt);
	}
	g_binge_anvil_hours++;
}

void dng13_smoke_damage(void)
{
	struct hero far *h;
	void far *txt_buf;
	char far * far *txt;

	if (g_group->dungeon == 13 && g_group->x >= 6 && g_group->y <= 5) {
		h = get_first_hero(g_group);
		txt_load_list(g_font, 0x2f9, 0x3c, &txt_buf, &txt);
		dlg_message(g_dialog, txt[33]);
		mem_free(txt_buf);
		mem_free(txt);
		do {
			sub_hero_le(h, random_schweif(3) + 3);
			h = get_next_hero(g_group, h);
		} while (get_first_hero(g_group) != h);
		g_refresh_portraits = 1;
	}
}

void show_daily_hint(void)
{
	void far *txt_buf;
	char far * far *txt;
	struct hero far *h;

	if (g_group->town == 4) {
		txt_load_list(g_font, 0x21c, 0x78, &txt_buf, &txt);
		h = get_first_hero(g_group);
		sprintf(g_text_output_buf, txt[117], h->name);
		dlg_message(g_dialog, g_text_output_buf);
		mem_free(txt_buf);
		mem_free(txt);
	}
}
