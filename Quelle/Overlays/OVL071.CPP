/* OVL071 - Overlay O071 (Segment 0x209d in SCHWEIF.EXE): Rastplatz/Lager - class Camp (s_class.h, 0x19 Bytes; aus
 * O030 fuer die Ortsarten LOC_CAMP_WILD/_TOWN/_DUNGEON als run(0/1/2), aus O173 als new/run(1)/delete).
 * STUFE D1 (wovd1, 19.10.2026): asm-strcat durch echtes strcat(.., "..") ersetzt - Code GLEICH, das Literal
 *   im eigenen _DATA bildet wovd1/cobjfix_d.py auf ds:587e ab (Bytes geprueft).
 * Felder (struct camp): Texte txt_buf/txt (txt_load_list(g_font, 0xae, 0x8c)), searched +8, Auftrag job +0a (1
 * Kraeuter, 2/3 Nahrung, aus rest_jobs), Lagerplatz site +0c (99 = gut), Nahrungssuchen food_tries +0d, Kraeuter
 * gesucht herbs_done +0e, Modus mode +10, Bild pic +11, done +15, Wachen eingeteilt watch_set +17.
 * Camp()/~Camp(): Startwerte, g_refresh = 1 / g_leave_location = 0. run(m): Texte laden, Modus 0 -> Bild 0xaf/0xb0 nach
 * Tageszeit (DATE->ticks), menu_wild(); sonst Bild 0xb1/0xb2, menu_rest(); danach Gruppen g_groups[6]
 * (group_reset_rest), Ortsart per set_loctype(set_loctype(0)) zuruecksetzen, Texte frei, Rueckgabe 1. menu_wild():
 * Hauptmenue (7 bzw. 8 Punkte nach Tageszeit): 0x81 Lagermenue menu_jobs, 0x82 Talent use_skill, 0x83 Zaubern
 * camp_use_magic (O072), 0x84 Spielstand (new SaveMenu / savegame_menu, Erfolg -> Ortsart LOC_CAMP_WILD,
 * g_travel_state = 1), 0x85 Gruppen camp_group_menu (O072), 0x86 Rasten camp_rest (O072), 0x87 Rast mit Auftraegen
 * rest_jobs, 0x88 tagsueber verlassen; Rast nur, wenn nicht zu viele Helden tot/bewusstlos/versteinert (LE-Summen
 * le_max/le) - sonst txt[112]; Abschluss je g_travel_state. menu_rest: Menue fuer Modus 1/2 (5 bzw. 6 Punkte mit
 * NSC group_get_npc: Rueckfrage txt[123] -> groups_check_npc). choose_watch: drei Wachen waehlen (Namen nach
 * g_camp_watch_names[3][16], Zaehler hero.watches; nicht bei Helden mit check_hero3). find_food: Wasser/Jagd
 * suchen (TA_WILDNISLEBEN/TA_FAEHRTENSUCHEN mit Modifikatoren g_camp_water_season/_weather bzw.
 * g_camp_hunt_season/_weather, Wetter Wx_get 3/4 -> Gelingen + hero_disease_test, Gegenstand 0x2d; Namen
 * g_camp_job_names[1]/[2], Bit rest_food). gather_herbs: Kraeuter suchen (Stunden dlg_input bis 6 je Tageszeit,
 * TA_PFLANZENKUNDE, Tabelle g_gather_herbs_table 12 x struct herb_find, Ortskraut g_loc_herb +10 %, Fund
 * give_hero_new_item, Liste mit grammar_article; Name g_camp_job_names[0], Stunden g_camp_herb_hours, Bit
 * rest_herbs). use_skill: Talent anwenden (use_skill). reset_watch(n, keep): Wachzaehler neu aus
 * g_camp_watch_names. eat_drink: Hungrige/Durstige mit Gegenstand aus den Vorraeten (group_find_food/_drink,
 * ItemUse + item_consume) essen/trinken lassen. menu_jobs: Lagermenue (Wachen, Nahrung, Kraeuter, Einteilung
 * loeschen clear_jobs). rest_jobs: Rast ab Mittag (DATE->ticks >= 12 Uhr) - Einteilung pruefen (check_hero),
 * Zusammenfassung, nach Rueckfrage Auftraege ausfuehren (find_food/gather_herbs mit job, Bits rest_* umschalten,
 * Wachen watches), dann camp_rest.
 *
 * BCC -mlarge -3 -Ot -Og -Oi -c -Y -E..\bc\BIN\TASM.EXE OVL071.CPP   -> 9448/9448 Bytes GLEICH (18.10.2026),
 *   cobjfix laeuft (Zeitzaehler 1/3 werden zu _g_246e+4/+0c). Ohne -Og 633 Bytes zu kurz, ohne -Oi falsch.
 *   (Namensfassung mit schweif.h: 19.10.2026 GLEICH, LINKGLEICH)
 * - strcat(g_text_output_buf, ",@") mit DS-Literal ds:587e (= g_gather_herbs_str_found + 6, hinter "%d^%s"),
 *   bekannte Laenge 3 (repne scasb / dec di / mov cx,3 / rep movsb) wie OVL111/OVL238.
 * - "m = 0; m -= (signed short)(t / TICKS_PER_HOUR);" -> xor si,si vor dem idiv, sub si,ax ("0 - t/5400" rechnet
 *   in DX).
 * - eat_drink: target = &g_heroes[v2] als eigene Variable ([bp-12h]) vor new ItemUse(h, target, ..); delete ->
 *   dtor(o, 3).
 * - Bitfelder +ba/+bc (unsigned short b:1): "(signed char)h->b" im if -> and / or al,al; in &&-Kette mit !
 *   -> and / cbw / or ax,ax; Setter set_watches/set_rest_food/set_rest_herbs(signed char) mit Parameterkopie.
 * - menu_wild(): Locals n, a, cnt, n6, n7, h, grp, state, redraw in dieser Reihenfolge; "n6 = n7 = sa = sb = 0"
 *   (DI zuerst gespeichert); group::get_nr_heroes()/get_loctype()/set_loctype() inline (-Og-Byte [bp-15h] geteilt).
 * - choose_watch: Schleife mit i++ nur im letzten Zweig (while, nicht for); rest_jobs: "if (!h[i]) { st = -1;
 *   break; } else if .." ergibt die toten jmp short hinter den breaks.
 * - run() hat einen signed-char-Parameter (mov al,[bp+0ah]).
 * - Tageszeit ueber DATE->ticks (Fixup auf g_date + 1 wie im Original, nicht g_daytime); screen_textbar mit
 *   int-Farbe ueber gecasteten Funktionszeiger TEXTBAR.
 * Originalfehler: rest_jobs prueft "ans == 1" auch nach den Fehlerfaellen st == -1/-2, in denen ans nie gesetzt
 *   wurde (Muellwert -> ggf. Auftraege mit unvollstaendigem h[]). find_food: bei job ausser 0/2/3 bleibt h
 *   uninitialisiert, wird aber benutzt. eat_drink: "a = b = 0;" am Ende wirkungslos.
 */
#include <stdio.h>
#include <string.h>
#include <mem.h>
#include "schweif.h"

/* Phase B: eigene Daten des Moduls (ds:5826..5882, dsana) - initialisierte Daten in Adressreihenfolge, danach der Literalpool */
struct herb_find g_gather_herbs_table[13] = {
	{ 0x7e, 0x05, 0x01, 15 },
	{ 0x7b, 0x0c, 0x03, 8 },
	{ 0x7c, 0x0a, 0x02, 9 },
	{ 0x3f, 0x0f, 0x03, 8 },
	{ 0x82, 0x0f, 0x01, 10 },
	{ 0x9d, 0x01, 0x01, 18 },
	{ 0x7a, 0x0c, 0x02, 8 },
	{ 0x40, 0x2d, 0x05, 4 },
	{ 0x83, 0x03, 0x01, 15 },
	{ 0x3c, 0x28, 0x04, 8 },
	{ 0x3d, 0x1e, 0x02, 4 },
	{ 0x3e, 0x14, 0x02, 4 },
	{ 0xff, 0x00, 0x00, 0 }
};
signed char g_camp_water_season[4] = { -9, 3, 10, 3 };
signed char g_camp_hunt_season[4] = { 10, 5, 10, 5 };
signed char g_camp_herb_season[4] = { 20, 5, 12, 10 };
signed char g_camp_water_weather[6] = { -2, -2, -15, 5, 0, -3 };
signed char g_camp_hunt_weather[6] = { -2, 0, -7, 5, 6, 2 };
signed char g_camp_herb_weather[6] = { 5, 2, 0, 10, 8, 5 };

#define DATE ((struct game_date far *)g_date)
/* Text in die Textleiste: Farbe 0xff als int wie im Original */
/* Heldenauswahl vorbelegen (O058 preselect_for_task): Aufgabe als int (push word) wie im Original */
#define PRESELECT(task) ((void (*)(struct group far *, signed short))preselect_for_task)(g_group, task)
#define TEXTBAR(t) ((void (*)(void far *, char far *, signed short))screen_textbar)(g_screen, t, 0xff)

Camp::Camp()
{
	penalty = 0;
	food_tries = 0;
	herbs_done = 0;
	done = 0;
	watch_set = 0;
	searched = 0;
	job = 0;
	g_refresh = 1;
}

Camp::~Camp()
{
	g_leave_location = 0;
}

signed short Camp::run(signed char m)
{
	struct group far *p;
	signed long t;
	register signed short i;

	txt_load_list(g_font, 0xae, 0x8c, &txt_buf, &txt);
	mode = m;
	if (mode == 0)
		group_set_on_route(g_group, 1);
	clear_input();
	mouse_clear_left();
	mouse_clear_right();
	mouse_set_cursor(0, 1);
	nop();
	if (mode == 0) {
		t = DATE->ticks;
		if (t > 0xa8c0L && t < 0x17bb0L)	/* 8..18 Uhr */
			pic = 0xaf;
		else
			pic = 0xb0;
		if (g_travel_state != 4 && g_travel_state != 6)
			g_travel_state = 0;
		menu_wild();
	} else {
		pic = mode == 1 ? 0xb1 : 0xb2;
		menu_rest();
	}
	if (done == 0) {
		for (i = 0; i < 6; i++) {
			p = &g_groups[i];
			if (!!p->nr_heroes)
				group_reset_rest(p);
		}
		if (group_get_on_route(g_group))
			g_group->set_loctype(g_group->set_loctype(0));
	}
	snd_obj_stop(g_snd_fx);
	g_rest_mod = 0;
	mem_free(txt_buf);
	mem_free(txt);
	return 1;
}

void Camp::menu_wild()
{
	signed short n;
	signed short a;
	signed char cnt;
	signed char n6;
	signed char n7;
	struct hero far *h;
	struct group far *grp;
	signed char state;
	signed short redraw;
	signed short sa;
	signed short sb;

	state = -1;
	redraw = 0;
	grp = g_group;
	g_refresh = 1;
	if (done == 0)
		eat_drink();
	while (state <= 0) {
		if (g_refresh) {
			snd_play(g_snd_music, 0x0b);
			g_refresh = 0;
			n = (DATE->ticks > 0x17bb0L || DATE->ticks < 0xa8c0L) ? 7 : 8;
			draw_loc_icons(g_screen, n, 0x45, 0x19, 0x0b, 0x28, 0x33, 0x11, 0x47, 0x32);
			screen_redraw(g_screen);
			anim_load(g_screen_pic, pic, 0);
			TEXTBAR(txt[0]);
			if (state == -1)
				state = 0;
		}
		if (g_options_shown)
			anim_load(g_screen_pic, pic, 0);
		handle_input(g_screen);
		if (g_key == 0x11b)
			g_action = 0x88;
		if (g_action == 0x4100 || g_action == 0x2004)
			g_action = 0x84;
		if (g_mouse_rclick || g_key == 0x4900) {
			a = dlg_choose(g_dialog, txt[1], n, txt[124], txt[4], txt[5], txt[35], txt[36], txt[7], txt[126],
				txt[8]) - 1;
			if (a == -1)
				g_action = 0x86;
			else if (a != -2)
				g_action = a + 0x81;
		}
		if (g_action == 0x81) {
			menu_jobs();
			redraw = 1;
		} else if (g_action == 0x82) {
			use_skill();
			redraw = 1;
		} else if (g_action == 0x83) {
			camp_use_magic(this);
			redraw = 1;
		} else if (g_action == 0x84) {
			SaveMenu far *s = new SaveMenu;
			if (savegame_menu(s, 0)) {
				g_group->set_loctype(LOC_CAMP_WILD);
				g_travel_state = 1;
				route_clear((Travel far *)g_travel);
				done = 1;
				state = 1;
			}
			delete s;
			g_refresh = 1;
		} else if (g_action == 0x85) {
			if (g_travel_state == 4 || g_travel_state == 6)
				dlg_message(g_dialog, txt[118]);
			else {
				camp_group_menu(this);
				if (g_group->get_loctype() != LOC_CAMP_WILD) {
					done = 1;
					state = 2;
				}
			}
			redraw = 1;
		} else if (g_action == 0x86) {
			state = camp_rest(this);
			if (state >= 1 && g_game_state == 0) {
				TEXTBAR(txt[0]);
				screen_portraits(g_screen);
			}
		} else if (g_action == 0x87) {
			state = rest_jobs();
			if (state >= 1 && g_game_state == 0) {
				TEXTBAR(txt[0]);
				screen_portraits(g_screen);
			}
		} else if (g_action == 0x88) {
			if (DATE->ticks < 0x17bb0L && DATE->ticks > 0xa8c0L)
				state = 1;
		}
		if (!group_count_check1(g_group))
			state = 2;
		if (state == 1 && g_travel_state != 1) {
			cnt = g_group->get_nr_heroes();
			n6 = n7 = sa = sb = 0;
			h = get_first_hero(g_group);
			do {
				if (h) {
					if ((signed char)h->dead || (signed char)h->unconscious) {
						n6 += 1;
						n7 += 1;
					}
					if ((signed char)h->petrified) {
						n6 += 2;
						n7 += 1;
					}
					sa += h->le_max;
					sb += h->le;
				}
				h = get_next_hero(g_group, h);
			} while (get_first_hero(g_group) != h);
			if (cnt - n7 < n6 || n6 * 3 > cnt && sb * 4 < sa) {
				dlg_message(g_dialog, txt[112]);
				redraw = 1;
				state = 0;
			}
		}
		if (redraw) {
			redraw = 0;
			n = (DATE->ticks > 0x17bb0L || DATE->ticks < 0xa8c0L) ? 7 : 8;
			draw_loc_icons(g_screen, n, 0x45, 0x19, 0x0b, 0x28, 0x33, 0x11, 0x47, 0x32);
			site = 1;
			screen_portraits(g_screen);
			anim_reset(g_screen_pic, 2);
		}
	}
	if (g_travel_state != 0 && g_travel_state != 4 && g_travel_state != 6) {
		if (g_travel_state == 5 && grp == g_group) {
			g_travel_state = 0;
			return;
		}
		g_travel_state = 2;
		if (done == 0)
			route_clear((Travel far *)g_travel);
	}
}

void Camp::menu_rest()
{
	signed short a;
	signed short redraw;
	struct hero far *h;
	register signed short n;
	register signed short fin;

	fin = 0;
	redraw = 0;
	if (group_get_npc(g_group))
		n = 6;
	else
		n = 5;
	while (!fin) {
		if (g_refresh) {
			g_refresh = 0;
			if (n == 6)
				draw_loc_icons(g_screen, n, 9, 0x19, 0x0b, 0x11, 0x42, 8);
			else
				draw_loc_icons(g_screen, n, 9, 0x19, 0x0b, 0x11, 8);
			screen_redraw(g_screen);
			anim_load(g_screen_pic, pic, 0);
			TEXTBAR(txt[0]);
			snd_play(g_snd_music, 0x0b);
		}
		if (g_options_shown)
			anim_load(g_screen_pic, pic, 0);
		if (redraw) {
			redraw = 0;
			if (n == 6)
				draw_loc_icons(g_screen, n, 9, 0x19, 0x0b, 0x11, 0x42, 8);
			else
				draw_loc_icons(g_screen, n, 9, 0x19, 0x0b, 0x11, 8);
			screen_portraits(g_screen);
		}
		handle_input(g_screen);
		if (g_mouse_rclick || g_key == 0x4900) {
			if (n == 6)
				a = dlg_choose(g_dialog, txt[1], n, txt[2], txt[4], txt[5], txt[7], txt[119], txt[30]) - 1;
			else
				a = dlg_choose(g_dialog, txt[1], n, txt[2], txt[4], txt[5], txt[7], txt[30]) - 1;
			if (a != -2)
				g_action = a + 0x81;
		}
		if (g_action == 0x81)
			choose_watch();
		else if (g_action == 0x82)
			use_skill();
		else if (g_action == 0x83)
			camp_use_magic(this);
		else if (g_action == 0x84)
			fin = camp_rest(this);
		else if (g_action == 0x85) {
			if (n == 6) {
				h = group_get_npc(g_group);
				sprintf(g_text_output_buf, txt[123], h->name);
				if (dlg_yes_no(g_dialog, g_text_output_buf) == 1) {
					g_npc_months = 0x63;
					groups_check_npc();
					redraw = 1;
					g_refresh = 0;
					n = 5;
				}
			} else
				fin = 1;
		} else if (g_action == 0x86) {
			if (n == 6)
				fin = 1;
		} else if (g_action == 0x11b)
			fin = 1;
	}
	g_refresh = 1;
}

void Camp::choose_watch()
{
	signed short i;
	struct hero far *h;
	signed short stop;

	stop = 1;
	watch_set = 1;
	reset_watch(0, 0);
	h = get_first_hero(g_group);
	do {
		if (check_hero3(h))
			stop = 0;
		h = get_next_hero(g_group, h);
	} while (get_first_hero(g_group) != h && stop != 0);
	if (stop)
		dlg_message(g_dialog, txt[10]);
	else {
		i = 0;
		while (i < 3 && !stop) {
			sprintf(g_text_output_buf, txt[11], i + 1);
			PRESELECT(0);
			h = get_active_hero(g_group, g_text_output_buf);
			if (!h)
				stop = 1;
			else if ((signed char)h->brewing)
				dlg_message(g_dialog, txt[13]);
			else {
				h->set_watches(h->watches + 1);
				strcpy(g_camp_watch_names[i], h->file_name);
				i++;
			}
		}
	}
	if (stop)
		reset_watch(0, 0);
}

void Camp::find_food()
{
	signed char wx;
	struct hero far *h;
	struct group far *grp;
	signed short mod;
	signed short water;
	signed short hunt;
	signed short wet;
	signed short ok;

	mod = g_camp_food_mod;
	water = 0;
	hunt = 0;
	wet = 0;
	ok = 0;
	grp = g_group;
	if (job != 0) {
#if FIX_0193
		/* Fix #193: ohne gueltigen Helden nicht suchen (h vorbelegt) */
		h = 0;
		if (job == 2)
			h = get_hero_by_file_name(g_group, g_camp_job_names[1]);
		else if (job == 3)
			h = get_hero_by_file_name(g_group, g_camp_job_names[2]);
		ok = h != 0;
#else
		if (job == 2)
			h = get_hero_by_file_name(g_group, g_camp_job_names[1]);
		else if (job == 3)
			h = get_hero_by_file_name(g_group, g_camp_job_names[2]);
		ok = 1;
#endif
	} else if (food_tries < 2) {
		searched = 1;
		if (site == 99)
			mod += 99;
		PRESELECT(1);
		h = get_active_hero(g_group, txt[14]);
		if (h) {
			if ((signed char)h->brewing)
				dlg_message(g_dialog, txt[13]);
			else if ((signed char)h->rest_food) {
				sprintf(g_text_output_buf, txt[15], h->name);
				dlg_message(g_dialog, g_text_output_buf);
			} else if ((signed char)h->rest_magic)
				dlg_message(g_dialog, txt[12]);
			else {
				h->rest_food = 1;
				if (g_camp_job_names[1][0]) {
					if (g_camp_job_names[2][0]) {
						strcpy(g_camp_job_names[1], h->file_name);
						g_camp_job_names[2][0] = 0;
					} else if (strncmp(g_camp_job_names[1], h->file_name, 16))
						strcpy(g_camp_job_names[2], h->file_name);
				} else
					strcpy(g_camp_job_names[1], h->file_name);
				if (mode)
					g_refresh = 1;
				food_tries++;
				ok = 1;
			}
		}
	} else
		dlg_message(g_dialog, txt[20]);

	if (ok == 1) {
		signed short v;

		v = mod + g_camp_water_season[date_season(g_date)] + g_camp_water_weather[g_camp_weather];
		wx = Wx_get(g_weather);
		if (wx == 3 || wx == 4)
			wet = 1;
		if (test_skill(h, TA_WILDNISLEBEN, v) > 0 || g_ingame_timers[INGAME_TIMER_EFFERD_FIND_WATER] != 0 || wet != 0) {
			group_refill_water(g_group);
			group_clr_thirst(g_group);
			if (wet)
				hero_disease_test(h, 2, 8);
			water = 1;
		}
		v = mod + g_camp_hunt_season[date_season(g_date)] + g_camp_hunt_weather[g_camp_weather];
		if (test_skill(h, TA_FAEHRTENSUCHEN, v) > 0 || g_ingame_timers[INGAME_TIMER_FIRUN_HUNT] != 0) {
			group_clr_hunger(g_group);
			if (group_give_item(g_group, 0x2d, random_schweif(4)))
				hunt = 1;
		}
		timewarp_hours(g_date, 1L);
		if (grp == g_group && check_hero3(h)) {
			sprintf(g_text_output_buf, water ? txt[16] : txt[17], h->name);
			if (water && hunt)
				strcat(g_text_output_buf, txt[41]);
			else if (!water && !hunt)
				strcat(g_text_output_buf, txt[42]);
			else
				strcat(g_text_output_buf, txt[43]);
			strcat(g_text_output_buf, hunt ? txt[18] : txt[19]);
			dlg_message(g_dialog, g_text_output_buf);
		}
	}
}

void Camp::gather_herbs()
{
	signed short i;
	signed short cnt;
	signed short hours;
	signed short mod;
	struct hero far *h;
	struct herb_find far *tab;
	struct group far *grp;
	signed short ok;
	signed char found[12];

	mod = g_camp_herb_mod;
	ok = 0;
	grp = g_group;
	if (job != 0) {
		h = get_hero_by_file_name(g_group, g_camp_job_names[0]);
		hours = g_camp_herb_hours;
		ok = 1;
	} else {
		searched = 1;
		if (site == 99)
			mod += 99;
		if (herbs_done == 0) {
			PRESELECT(2);
			h = get_active_hero(g_group, txt[21]);
			if (h) {
				if ((signed char)h->brewing)
					dlg_message(g_dialog, txt[13]);
				else if ((signed char)h->rest_herbs) {
					sprintf(g_text_output_buf, txt[22], h->name);
					dlg_message(g_dialog, g_text_output_buf);
				} else if ((signed char)h->rest_magic)
					dlg_message(g_dialog, txt[12]);
				else {
					register signed short m;

					m = 0;
					m -= (signed short)(DATE->ticks / TICKS_PER_HOUR);
					m += m > -8 ? 8 : 0x20;
					m -= 5;
					if (m <= 0)
						m = 1;
					if (m > 6)
						m = 6;
					sprintf(g_text_output_buf, txt[23], m, h->name);
					hours = dlg_input(g_dialog, 1, g_text_output_buf, 0, m, 1);
					ok = 1;
				}
			}
		} else
			dlg_message(g_dialog, txt[26]);
	}
	if (ok == 1 && hours > 0) {
		h->rest_herbs = 1;
		strcpy(g_camp_job_names[0], h->file_name);
		g_camp_herb_hours = hours;
		herbs_done = 1;
		if (mode)
			g_refresh = 1;
		memset(found, 0, 12);
		timewarp_hours(g_date, (signed long)hours);
		if (grp == g_group && check_hero3(h)) {
			hours--;
			for (tab = g_gather_herbs_table, cnt = i = 0; i < 12; i++, tab++) {
				if (tab->id == g_loc_herb) {
					tab->chance += 10;
					tab->max++;
				}
				if (random_schweif(100) <= tab->chance) {
					signed short v;

					v = mod + g_camp_herb_season[date_season(g_date)] + g_camp_herb_weather[g_camp_weather];
					if (test_skill(h, TA_PFLANZENKUNDE, tab->diff - hours + v) > 0) {
						found[i] = give_hero_new_item(h, tab->id, 0, random_schweif(tab->max));
						if (found[i])
							cnt++;
					}
				}
				if (tab->id == g_loc_herb) {
					tab->chance -= 10;
					tab->max--;
				}
			}
			if (cnt) {
				sprintf(g_text_output_buf, txt[24], h->name);
				strcat(g_text_output_buf, g_ttx[450]);
				for (i = 0; i < 12; i++) {
					if (found[i]) {
						sprintf(g_text_buf2, "%d^%s", found[i],
							grammar_article((found[i] > 1 ? 4 : 0) + 0x4002, g_gather_herbs_table[i].id, 0));
						strcat(g_text_output_buf, g_text_buf2);
						if (--cnt > 1) {
							strcat(g_text_output_buf, ",@");
						} else if (cnt == 1) {
							strcat(g_text_output_buf, g_ttx[421]);
							strcat(g_text_output_buf, g_ttx[451]);
						}
					}
				}
				strcat(g_text_output_buf, g_ttx[423]);
			} else
				sprintf(g_text_output_buf, txt[25], h->name);
			dlg_message(g_dialog, g_text_output_buf);
		}
	}
}

void Camp::use_skill()
{
	struct hero far *h;

	h = get_active_hero(g_group, txt[9]);
	::use_skill(h, 0, -1);
}

void Camp::reset_watch(signed char n, signed char keep)
{
	struct hero far *h;
	register signed short i = 0;

	h = get_first_hero(g_group);
	do {
		h->watches = 0;
		h = get_next_hero(g_group, h);
	} while (get_first_hero(g_group) != h);
	for (; n > i; i++) {
		h = get_hero_by_file_name(g_group, g_camp_watch_names[i]);
		if (h)
			h->set_watches(h->watches + 1);
	}
	if (keep == 0)
		for (i = 0; i < 3; i++)
			g_camp_watch_names[i][0] = 0;
}

void Camp::nop()
{
}

void Camp::eat_drink()
{
	signed short v2;
	signed short v4;
	signed short v6;
	ItemUse far *o;
	struct hero far *h;
	struct hero far *target;
	register signed short ate = 0;
	register signed short drank = 0;

	h = get_first_hero(g_group);
	do {
		if (h && !(signed char)h->unconscious && !(signed char)h->dead && !(signed char)h->petrified) {
			if (group_find_item(g_group, 0xb8) == -1 && get_item_pos(h, 0xaf) == -1 && h->hunger > 60 &&
				group_find_food(g_group, h, &v2, &v4, &v6)) {
				target = &g_heroes[v2];
				o = new ItemUse(h, target, v4, v6, 0);
				item_consume(o);
				delete o;
				ate = 1;
			}
			if (group_find_item(g_group, 0xb9) == -1 && get_item_pos(h, 0xaf) == -1 && h->thirst > 70 &&
				group_find_drink(g_group, h, &v2, &v4, &v6)) {
				target = &g_heroes[v2];
				o = new ItemUse(h, target, v4, v6, 0);
				item_consume(o);
				delete o;
				drank = 1;
			}
		}
		h = get_next_hero(g_group, h);
	} while (get_first_hero(g_group) != h);
	if (ate && drank)
		dlg_message(g_dialog, txt[109]);
	else if (ate)
		dlg_message(g_dialog, txt[110]);
	else if (drank)
		dlg_message(g_dialog, txt[111]);
	ate = drank = 0;
}

void Camp::menu_jobs()
{
	signed short quit;
	register signed short redraw;
	register signed short a;

	quit = 0;
	redraw = 1;
	g_refresh = 0;
	screen_reset_icons(g_screen);
	anim_load(g_screen_pic, pic, 0);
	TEXTBAR(txt[0]);
	while (!quit) {
		if (g_refresh) {
			snd_play(g_snd_music, 0x0b);
			screen_reset_icons(g_screen);
			screen_redraw(g_screen);
			anim_load(g_screen_pic, pic, 0);
			TEXTBAR(txt[0]);
			redraw = 1;
			g_refresh = 0;
		}
		if (g_options_shown)
			anim_load(g_screen_pic, pic, 0);
		if (redraw) {
			screen_reset_icons(g_screen);
			draw_loc_icons(g_screen, 5, 9, 0x10, 0x14, 0x46, 8);
			screen_portraits(g_screen);
			anim_reset(g_screen_pic, 2);
			redraw = 0;
		}
		handle_input(g_screen);
		if (g_key == 0x11b) {
			g_key = 0;
			g_action = 0x85;
		}
		if (g_mouse_rclick || g_key == 0x4900) {
			a = dlg_choose(g_dialog, txt[127], 5, txt[2], txt[3], txt[6], txt[125], txt[40]) - 1;
			if (a != -2)
				g_action = a + 0x81;
		}
		if (g_action == 0x81) {
			choose_watch();
			redraw = 1;
		} else if (g_action == 0x82) {
			find_food();
			redraw = 1;
		} else if (g_action == 0x83) {
			gather_herbs();
			redraw = 1;
		} else if (g_action == 0x84) {
			clear_jobs();
			redraw = 1;
		} else if (g_action == 0x85)
			quit = 1;
	}
}

void Camp::clear_jobs()
{
	signed short i;

	for (i = 0; i < 6; i++)
		g_camp_job_names[i][0] = 0;
	g_camp_herb_hours = 0;
}

signed short Camp::rest_jobs()
{
	signed short ans;
	signed short i;
	signed short ret;
	signed short st;
	struct hero far *h[6];

#if FIX_0193
	/* Fix #193: ans vorbelegen (Fehlerfaelle st == -1/-2) */
	ret = 0;
	st = 0;
	ans = 0;
	if (searched != 0 || DATE->ticks < 0xa8c0L)
#else
	ret = 0;
	st = 0;
	if (searched != 0 || DATE->ticks < 0xa8c0L)
#endif
		dlg_message(g_dialog, txt[128]);
	else {
		for (i = 0; i < 6; i++) {
			if (g_camp_job_names[i][0]) {
				h[i] = get_hero_by_file_name(g_group, g_camp_job_names[i]);
				if (!h[i]) {
					st = -1;
					break;
				} else if (!check_hero(h[i])) {
					st = -2;
					break;
				} else
					st = 1;
			} else
				h[i] = 0;
		}
		if (st == -1) {
			dlg_message(g_dialog, txt[135]);
			clear_jobs();
		} else if (st == -2) {
			dlg_message(g_dialog, txt[136]);
			clear_jobs();
		} else if (st == 1) {
			strcpy(g_text_output_buf, txt[129]);
			if (g_camp_job_names[0][0]) {
				sprintf(g_text_buf2, txt[130], h[0]->name);
				strcat(g_text_output_buf, g_text_buf2);
			}
			if (g_camp_job_names[1][0]) {
				sprintf(g_text_buf2, txt[131], h[1]->name);
				strcat(g_text_output_buf, g_text_buf2);
			}
			if (g_camp_job_names[2][0]) {
				sprintf(g_text_buf2, txt[132], h[2]->name);
				strcat(g_text_output_buf, g_text_buf2);
			}
			if (g_camp_job_names[1][0] || g_camp_job_names[2][0])
				strcat(g_text_output_buf, g_ttx[450]);
			if (g_camp_watch_names[0][0]) {
				sprintf(g_text_buf2, txt[133], h[3]->name, h[4]->name, h[5]->name);
				strcat(g_text_output_buf, g_text_buf2);
			}
			ans = dlg_yes_no(g_dialog, g_text_output_buf);
		} else {
			dlg_message(g_dialog, txt[134]);
			ans = 0;
		}
		if (ans == 1) {
			if (g_camp_job_names[1][0]) {
				job = 2;
				find_food();
#if FIX_0370
				/* Fix #370: 1-Bit-Merker setzen statt +1 */
				h[1]->set_rest_food(1);
#else
				h[1]->set_rest_food(h[1]->rest_food + 1);
#endif
			}
			if (g_camp_job_names[2][0]) {
				job = 3;
				find_food();
#if FIX_0370
				/* Fix #370: 1-Bit-Merker setzen statt +1 */
				h[2]->set_rest_food(1);
#else
				h[2]->set_rest_food(h[2]->rest_food + 1);
#endif
			}
			if (g_camp_job_names[0][0]) {
				job = 1;
				gather_herbs();
#if FIX_0370
				/* Fix #370: 1-Bit-Merker setzen statt +1 (1 + 1 ergab 0) */
				h[0]->set_rest_herbs(1);
#else
				h[0]->set_rest_herbs(h[0]->rest_herbs + 1);
#endif
			}
			if (g_camp_watch_names[0][0]) {
				job = 1;
				h[3]->set_watches(h[3]->watches + 1);
				h[4]->set_watches(h[4]->watches + 1);
				h[5]->set_watches(h[5]->watches + 1);
			}
			ret = camp_rest(this);
		}
	}
	job = 0;
	return ret;
}
