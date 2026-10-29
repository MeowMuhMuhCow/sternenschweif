/* OVL178 - Overlay O178 (Segment 0x22d5 in SCHWEIF.EXE): Herberge LOC_INN (DSA1 seg068/069). class Inn (s_class.h,
 * 0x16 Bytes, global g_inn): Texte txt_buf/txt (txt_load_list(0x1a, 0x50)), Menuegroesse nr_icons (4/7), Merker
 * special (Sonderfall Ort 2..3)/done/redraw_icons, aus Datei 0x200 (g_file_3) Satz g_loc_id * 6 nach info:
 * Preisaufschlag markup (%), Qualitaet quality, f04, Art kind (Bild/Klang 0x189/0x2a, txt[55]/[54]).
 * Konstruktor/Destruktor (new 0x16, mem_free beide Puffer, g_inn = 0). is_open: Herberge geschlossen
 * (g_inn_closed[Ort]) -> Meldung txt[53], 0. run: Menueschleife dlg_choose (4/7 Punkte) -> g_action 0x81..0x86:
 * inn_talk (Gespraech), eat, room, inn_apply_skill/inn_apply_magic, sleep; Sonderfaelle Ort 2..3
 *   (g_gashok_arrested/g_guide_avail) und
 * Hafen (g_port_deal, Town(5, 0x21d, 0x5a) + port_night_event). eat: Ort 4..12 Gruppe essen (Kosten (n+1)*(1+%)*3)
 * bzw. Mahlzeit (6 - q/4)*n*(1+%)*10, Qualitaetstext, group_sub_hunger_thirst_max. room: Zimmer (Ort 4..12
 * g_inn_has_room[Ort], n*2) bzw. 3 Kategorien (5/30/100 + %) mal Personen mal Naechte (dlg_input), Buchung
 * g_inn_room/g_inn_days/g_inn_town/g_inn_loc/g_inn_price. sleep: Ort 4..12 Nacht mit Diebstahlwurf
 * (inn_theft_night/inn_theft_morning), sonst Naechte-Schleife mit wait_midnight/wait_morning (Zeit g_daytime gegen 8
 *   Uhr,
 * Taste/Maus -> Frage txt[73]), group_sleep (Erholung), Sonderereignisse (Sites::inn_vision, Intro +
 * Gefangennahme per group_find_item(0x129), Anim3 + Trinkgelage).
 *
 * BCC -mlarge -3 -Ot -Og -rd -c -Y OVL178.CPP   -> 5600/5600 Bytes GLEICH (18.10.2026), cobjfix laeuft
 *   Ohne -rd 2154 Abweichungen (room), ohne -Og 3 (Epilogkopie).
 *   (Namensfassung mit schweif.h: 19.10.2026 GLEICH, LINKGLEICH)
 * - -rd noetig: in room() haelt "register pct" den Aufschlag in SI ("(pct = info.markup) * 5 / 100 + 5"), die drei
 *   Preise muessen auf dem Stack bleiben (ohne -rd bekommen sie SI/DI).
 * - room(): long cost [bp-0eh] nur eingebettet: "(cost = g_inn_price) > g_group->money", "group_sub_money(g, cost =
 *   g_inn_price)".
 * - sleep(): "if (!wait_midnight()) {...} else done = 1;" (done = 1 steht hinter dem Block); Sites/Anim3 als lokale
 *   Objekte in eigenen Bloecken, Intro und Town per new/delete (Konstruktor(0, ..), Destruktor(p, 3)).
 * - Ketten: "g_refresh = redraw_icons = 1", "g_inn_room = g_inn_days = 1" (280d zuerst), "g_guide_avail = g_gashok_arrested =
 *   0" (2745 zuerst).
 * - Methoden unterhalb von run() -> far call (TLINK: nop/push cs/call near).
 * - screen_textbar mit int-Farbe (push word) ueber gecasteten Funktionszeiger LOC_TEXTBAR.
 * Originalfehler: room() - nach "if (g_inn_room == -1) ... else if (g_inn_room != -1) ..." ist "else if (old_room)
 *   g_inn_room = old_room" unerreichbar (eine stornierte Buchung wird bei Abbruch nicht wiederhergestellt, Geld ist
 *   schon erstattet). eat() - "if (cost < 0) cost = 1" (0 bleibt kostenlos; room prueft <= 0).
 */
#include <stdio.h>
#include <bios.h>
#include "schweif.h"

/* Ortsname in die Textleiste: Farbe 0xff als int wie im Original */
#define LOC_TEXTBAR(t) ((void (*)(void far *, char far *, signed short))screen_textbar)(g_screen, t, 0xff)

Inn::Inn()
{
	g_inn = this;
	txt_load_list(g_font, 0x1a, 0x50, &txt_buf, &txt);
	g_refresh = redraw_icons = 1;
	special = 0;
	arc_open(g_file_3, 0x200);
	arc_seek(g_file_3, g_loc_id * 6L, 0);
	arc_read(g_file_3, &info, 6L);
	arc_close(g_file_3);
}

Inn::~Inn()
{
	mem_free(txt_buf);
	mem_free(txt);
	g_inn = 0;
}

signed short Inn::is_open()
{
	register signed short ok = 1;

	if ((g_gashok_arrested || g_guide_avail == 1) && g_loc_id >= 2 && g_loc_id <= 3) {
	} else if (g_inn_closed[g_loc_id]) {
		if (g_pp20_index == 4 || g_pp20_index == 0x2ae) {
			LOC_TEXTBAR(txt[g_loc_name]);
		}
		sprintf(g_text_output_buf, txt[53], info.kind ? txt[55] : txt[54]);
		dlg_message(g_dialog, g_text_output_buf);
		ok = 0;
	}
	return ok;
}

void Inn::run()
{
	Town far *t;
	register signed short sel;

	if ((g_gashok_arrested || g_guide_avail == 1) && g_loc_id >= 2 && g_loc_id <= 3) {
		special = 1;
	}
	if (special && g_gashok_arrested) {
		screen_redraw(g_screen);
		anim_load(g_screen_pic, info.kind ? 0x189 : 0x2a, 0);
		LOC_TEXTBAR(txt[g_loc_name]);
		snd_play(g_snd_music, 3);
		g_inn_room = g_inn_days = 1;
		sleep();
		special = 0;
		leave();
		return;
	}
	if (g_port_deal && g_group->town == 5 && g_inn_room && g_inn_days > 0) {
		t = new Town(5, 0x21d, 0x5a);
		port_night_event(t);
		delete t;
		leave();
		return;
	}
	done = 0;
	while (done == 0) {
		if (redraw_icons) {
			if (g_inn_room && g_inn_days > 0 && g_group->town == g_inn_town && g_inn_loc == g_loc_id) {
				nr_icons = 7;
			} else {
				nr_icons = 4;
			}
			if (nr_icons == 4) {
				draw_loc_icons(g_screen, 4, 0x15, 0x0d, 0x13, 8);
			} else {
				draw_loc_icons(g_screen, 7, 0x15, 0x0d, 0x13, 0x19, 0x0b, 0x11, 8);
			}
		}
		if (g_refresh) {
			if (redraw_icons == 0) {
				if (nr_icons == 4) {
					draw_loc_icons(g_screen, 4, 0x15, 0x0d, 0x13, 8);
				} else {
					draw_loc_icons(g_screen, 7, 0x15, 0x0d, 0x13, 0x19, 0x0b, 0x11, 8);
				}
			}
			screen_redraw(g_screen);
			anim_load(g_screen_pic, info.kind ? 0x189 : 0x2a, 0);
			snd_play(g_snd_music, 3);
			LOC_TEXTBAR(txt[g_loc_name]);
			g_refresh = 0;
		}
		redraw_icons = 0;
		handle_input(g_screen);
		if (g_mouse_rclick || g_action == 0x4900) {
			if (nr_icons == 4) {
				sel = dlg_choose(g_dialog, txt[0], 4, txt[3], txt[2], txt[4], txt[1]) - 1;
			} else {
				sel = dlg_choose(g_dialog, txt[0], 7, txt[3], txt[2], txt[4], txt[5], txt[7], txt[6],
					txt[1]) - 1;
			}
			if (sel != -2) {
				g_action = sel + 0x81;
			}
		}
		if (g_action == 0x81) {
			inn_talk(this);
		} else if (g_action == 0x82) {
			eat();
		} else if (g_action == 0x83) {
			room();
		} else if (g_action == 0x84 && nr_icons == 7) {
			inn_apply_skill(this);
		} else if (g_action == 0x85 && nr_icons == 7) {
			inn_apply_magic(this);
		} else if (g_action == 0x86 && nr_icons == 7) {
			sleep();
		} else if (nr_icons + 0x80 == g_action || g_key == 0x11b) {
			done = 1;
		}
		if (g_leave_location) {
			done = 1;
		}
	}
}

void Inn::eat()
{
	signed long cost;
	register signed short q;

	if (g_loc_id >= 4 && g_loc_id <= 12) {
		if (dlg_yes_no(g_dialog, txt[46]) == 1) {
			cost = group_count_check2b(g_group) + 1;
			cost += cost * info.markup / 100;
			cost *= 3;
			if (g_group->money < cost) {
				dlg_message(g_dialog, g_ttx[4]);
			} else {
				dlg_message(g_dialog, txt[47]);
				group_sub_hunger_thirst_max(g_group, 2, (21 - info.quality) * 30);
				group_sub_money(g_group, cost);
				timewarp_minutes(g_date, 30L);
			}
		}
	} else {
		cost = 6 - ((signed long)info.quality >> 2);
		cost *= group_count_alive(g_group);
		cost += cost * info.markup / 100;
		sprintf(g_text_output_buf, txt[19], (signed short)cost);
		if (dlg_yes_no(g_dialog, g_text_output_buf) == 1) {
			cost *= 10;
#if FIX_0155
			/* Fix #155: Essen kostet mindestens 1 (0 war kostenlos) */
			if (cost <= 0) {
				cost = 1;
			}
#else
			if (cost < 0) {
				cost = 1;
			}
#endif
			if (g_group->money < cost) {
				dlg_message(g_dialog, g_ttx[4]);
				return;
			}
			q = 21 - info.quality;
			if (q < 1) {
				q = 1;
			}
			if (g_loc_id >= 2 && g_loc_id <= 3) {
				dlg_message(g_dialog, info.quality < 5 ? txt[68] : info.quality < 15 ? txt[21] : txt[69]);
			} else {
				dlg_message(g_dialog, info.quality < 5 ? txt[20] : info.quality < 15 ? txt[21] : txt[22]);
			}
			group_sub_hunger_thirst_max(g_group, q * 20, q * 30);
			group_sub_money(g_group, cost);
			timewarp_hours(g_date, 1L);
		}
	}
}

void Inn::room()
{
	signed short old_room = 0;
	signed short old_icons = nr_icons;
	signed short c100;
	signed short c30;
	signed short c5;
	signed long cost;
	Town far *t;
	register signed short pct;

	if (g_loc_id >= 4 && g_loc_id <= 12) {
		if (g_inn_room == 0) {
			if (g_inn_has_room[g_loc_id]) {
				if (dlg_yes_no(g_dialog, txt[48]) == 1) {
					g_inn_price = group_count_alive(g_group) * 2;
					if ((cost = g_inn_price) > g_group->money) {
						dlg_message(g_dialog, g_ttx[4]);
						g_inn_room = 0;
					} else {
						g_inn_room = g_inn_days = 1;
						g_inn_town = 4;
						g_inn_loc = g_loc_id;
						sprintf(g_text_output_buf, txt[56], info.kind ? txt[55] : txt[54]);
						dlg_message(g_dialog, g_text_output_buf);
						group_sub_money(g_group, cost = g_inn_price);
						timewarp_minutes(g_date, 10L);
						dlg_message(g_dialog, txt[24]);
					}
				}
			} else {
				dlg_message(g_dialog, txt[49]);
			}
		} else {
			dlg_message(g_dialog, txt[74]);
		}
	} else {
		if (g_inn_room) {
			if (g_group->town == g_inn_town && g_inn_loc == g_loc_id) {
				if (dlg_yes_no(g_dialog, txt[9]) == 1) {
					group_add_money(g_group, cost = g_inn_price);
					old_room = g_inn_room;
					g_inn_room = 0;
				}
			} else {
				g_inn_room = 0;
			}
		}
		if (g_inn_room == 0) {
			c5 = (pct = info.markup) * 5 / 100 + 5;
			c30 = pct * 30 / 100 + 30;
			c100 = pct * 100 / 100 + 100;
			sprintf(g_text_output_buf, txt[11], c5);
			sprintf(g_text_output_buf + 50, txt[12], c30);
			sprintf(g_text_output_buf + 100, txt[13], c100);
			g_inn_room = dlg_choose(g_dialog, txt[10], 3, g_text_output_buf, g_text_output_buf + 50,
				g_text_output_buf + 100);
#if FIX_0155
			/* Fix #155: Abbruch stellt die stornierte Buchung wieder her (erstattetes Geld wird wieder abgezogen) */
			if (g_inn_room == -1 && old_room) {
				g_inn_room = old_room;
				group_sub_money(g_group, g_inn_price);
				old_room = 0;
			} else if (g_inn_room == -1) {
				g_inn_room = old_room = 0;
			} else if (g_inn_room != -1) {
#else
			if (g_inn_room == -1) {
				g_inn_room = old_room = 0;
			} else if (g_inn_room != -1) {
#endif
				if (g_loc_id >= 2 && g_loc_id <= 3 && g_sites_hint_done && !g_inn_vision_state) {
					g_inn_vision_state = 1;
				}
				g_inn_days = dlg_input(g_dialog, 2, txt[57], 0, 0x63, 1);
				if (g_inn_days > 0) {
					g_inn_price = g_inn_room == 1 ? c5 : g_inn_room == 2 ? c30 : c100;
					g_inn_price *= group_count_alive(g_group);
					g_inn_price *= g_inn_days;
					if (g_inn_price <= 0) {
						g_inn_price = 1;
					}
					if ((cost = g_inn_price) > g_group->money) {
						dlg_message(g_dialog, g_ttx[4]);
						g_inn_room = g_inn_days = 0;
					} else {
						g_inn_town = g_group->town;
						g_inn_loc = g_loc_id;
						group_sub_money(g_group, cost);
						timewarp_minutes(g_date, 30L);
						dlg_message(g_dialog, txt[24]);
					}
				} else {
					g_inn_room = 0;
				}
			} else if (old_room) {
				g_inn_room = old_room;
			}
		}
	}
	if (g_inn_room && g_inn_days > 0) {
		if (g_port_deal && g_group->town == 5) {
			t = new Town(5, 0x21d, 0x5a);
			port_night_event(t);
			delete t;
			done = 1;
		} else {
			nr_icons = 7;
		}
	} else {
		nr_icons = 4;
	}
	if (nr_icons != old_icons) {
		redraw_icons = 1;
	}
}

void Inn::sleep()
{
	signed short days;
	signed short nights;
	signed short save_rest;
	signed short wake;
	signed short drink;
	signed short stop;
	register signed short r;
	register signed short quit;

	days = g_inn_room;
	if (g_inn_room && g_inn_days > 0 && (g_gashok_arrested || dlg_yes_no(g_dialog, txt[28]) == 1)) {
		if (info.quality < 8) {
			days++;
		} else if (info.quality > 15) {
			days--;
		}
		if (g_guide_avail == 1 && g_loc_id >= 2 && g_loc_id <= 3) {
			special = 1;
		}
		if (g_loc_id >= 4 && g_loc_id <= 12) {
			r = random_schweif(100);
			if (r <= 20) {
				quit = 0;
				g_resting = 1;
				wait_midnight();
				g_resting = 0;
				quit = inn_theft_night(this);
				g_resting = 1;
				wait_morning();
				g_resting = 0;
				group_sleep(g_group, days);
				bars_refresh(g_bars);
				LOC_TEXTBAR(txt[g_loc_name]);
				if (quit) {
					dlg_message(g_dialog, txt[51]);
				}
			} else {
				g_resting = 1;
				wait_midnight();
				wait_morning();
				g_resting = 0;
				group_sleep(g_group, days);
				bars_refresh(g_bars);
				LOC_TEXTBAR(txt[g_loc_name]);
				if (r <= 50) {
					inn_theft_morning(this);
				}
			}
		} else {
			nights = 1;
			save_rest = g_rest_mod;
			wake = 0;
			r = 0;
			drink = 0;
			stop = 0;
			quit = 0;
			if (g_inn_days > 1) {
				nights = dlg_input(g_dialog, g_inn_days < 10 ? 1 : 2, txt[72], 0, g_inn_days, 1);
			}
			if (g_loc_id >= 2 && g_loc_id <= 3 && special) {
				r = 1;
			} else if (g_loc_id >= 2 && g_loc_id <= 3 && g_inn_vision_state > 0 && g_sites_evening != -1) {
				g_inn_vision_state = 0;
				if (g_daytime >= 0xa8c0) {
					wake = 1;
				}
			} else if (!g_dream_seen) {
				drink = 1;
			}
			g_resting = 1;
			while (nights > 0 && !quit) {
				if (!wait_midnight()) {
					if (g_loc_id >= 2 && g_loc_id <= 3 && !r && g_guide_avail == 1) {
						r = 1;
					}
					if (wake) {
						wake = 0;
						g_rest_mod -= 2;
						ani_halt(g_screen_pic);
						{
							Sites o;
							o.inn_vision();
							screen_redraw(g_screen);
							LOC_TEXTBAR(txt[g_loc_name]);
							anim_reset(g_screen_pic, 2);
							g_inn_vision_state = -1;
						}
					} else if (r) {
						Intro far *p;

						r = 0;
						stop = 1;
						if (group_find_item(g_group, 0x129) == -1) {
							g_trial_lost = 1;
						} else {
							g_trial_lost = 0;
						}
						p = new Intro;
						mouse_hide(0);
						p->run();
						mouse_show(0);
						delete p;
						if (g_trial_lost) {
							while (group_count_alive(g_group)) {
								group_sub_le(g_group, 50);
							}
							g_hero_died = 1;
							done = 1;
							quit = 1;
						} else {
							special = 0;
							g_guide_avail = g_gashok_arrested = 0;
							g_guide_won = 0;
							done = 1;
							diary_add(g_diary, txt[75], 0);
						}
					} else if (drink) {
						drink = 0;
						g_rest_mod -= 2;
						pal_fade_out();
						snd_obj_stop(g_snd_music);
						ani_stop(g_screen_pic);
						{
							Anim3 o;
							o.run();
							g_topic_sternenschweif = 1;
							snd_play(g_snd_music, 3);
							screen_redraw(g_screen);
							LOC_TEXTBAR(txt[g_loc_name]);
							anim_load(g_screen_pic, info.kind ? 0x189 : 0x2a, 0);
						}
					}
					if (!stop) {
						if (!wait_morning()) {
							nights--;
							group_sleep(g_group, days);
							bars_refresh(g_bars);
							LOC_TEXTBAR(txt[g_loc_name]);
							group_reset_rest(g_group);
						} else {
							quit = 1;
						}
					} else {
						quit = 1;
					}
				} else {
					quit = 1;
				}
				g_rest_mod = save_rest;
			}
			g_resting = 0;
		}
	}
	redraw_icons = 1;
}

signed short Inn::wait_midnight()
{
	register signed short quit = 0;
	register signed short key = 0;

	while (g_daytime > 0xa8c0 && !quit) {
		timewarp_hours(g_date, 1L);
		if (g_refresh_portraits) {
			screen_portraits(g_screen);
		}
		if (g_leave_location) {
			quit = 1;
		}
		if (!quit) {
			if (bioskey(1)) {
				key = bioskey(0);
			}
			if (g_mouse_lclick || g_mouse_rclick || key == 0x11b || key == 0x3920 || key == 0x4900 || key == 0x5100) {
				mouse_clear_left();
				mouse_clear_right();
				key = 0;
				if (dlg_yes_no(g_dialog, txt[73]) == 1) {
					quit = 1;
				}
			}
		}
	}
	return quit;
}

signed short Inn::wait_morning()
{
	register signed short quit = 0;
	register signed short key = 0;

	while (g_daytime < 0xa8c0 && !quit) {
		timewarp_hours(g_date, 1L);
		if (g_refresh_portraits) {
			screen_portraits(g_screen);
		}
		if (g_leave_location) {
			quit = 1;
		}
		if (!quit) {
			if (bioskey(1)) {
				key = bioskey(0);
			}
			if (g_mouse_lclick || g_mouse_rclick || key == 0x11b || key == 0x3920 || key == 0x4900 || key == 0x5100) {
				mouse_clear_left();
				mouse_clear_right();
				key = 0;
				if (dlg_yes_no(g_dialog, txt[73]) == 1) {
					quit = 1;
				}
			}
		}
	}
	return quit;
}

void Inn::leave()
{
	done = 1;
}
