/* OVL240 - Overlay O240 (Segment 0x2447 in SCHWEIF.EXE): Schenke, Klasse Tavern (0x1f Bytes, s_class.h: struct tavern
 * mit Methoden; Zeiger g_tavern; weitere Methoden als freie Funktionen in O241/O242; BE seg059 do_tavern).
 * Konstruktor: Texte 0x1e (0xa0 Stueck), 7-Byte-Satz g_loc_id aus der Datei 0x1f4 nach price_mod/quality/female/
 * event_chance, Zaehler = 0, script = -1. Destruktor. admit (O240_014F): Oeffnungszeit (tavern_check_hours),
 * Hausverbot (check_ban), Zustand der Helden (drink_test(0)), Rueckgabe !done. run (O240_01A7): Hauptschleife bis
 * done (Symbolfelder draw_loc_icons mit 5/6 Knoepfen je Andrang g_tavern_crowd und Uhrzeit, stuendlich hourly_event,
 * Sperrstunde check_closing, Hausverbot, tavern_night_event; Wahl ACTION_ICON_1..5 -> tavern_talk / order /
 * tavern_use_skill / tavern_change_seat / tavern_round, letzte Wahl oder ESC = gehen, Meldung bzw.
 * tavern_pay_bill(0x44) bei offener Zeche). set_crowd(t): Andrang 1..3 aus W100 + Stufe und Uhrzeit. check_ban:
 * Hausverbot (g_tavern_banned[g_loc_id]) bzw. Schenke 3 mit Zwerg in der Gruppe -> hinaus. check_closing(t):
 * Sperrstunde. drink_test(drank): je Held Zechprobe test_skill(h, TA_ZECHEN, drinks * 5 - 20), Misserfolg ->
 * hero_get_drunken; alle betrunken -> Ende. hourly_event: Einladung mit Chance event_chance (Ja: seat = 3, sonst
 * seat_changes++). eat: Essen fuer alle bezahlen (wie tavern_pay_meal in O242, Rueckgabe 1/0). order: Bestellen
 * (Schenken 3..5 -> tavern_order_dinner, 9..18 -> tavern_order_drink, sonst Essen/Trinken/eat). add_bill(art):
 * bill += (Gruppengroesse + 1) mit Aufschlag, Art 1/2 * 3, Art 4 / 2.
 *
 * BCC -mlarge -3 -Ot -Og -c -Y OVL240.CPP   -> 4543/4543 Bytes GLEICH (18.10.2026), cobjfix laeuft
 *   (Namensfassung mit schweif.h: 19.10.2026 GLEICH, LINKGLEICH)
 * - C++: Klasse mit Konstruktor (this == 0 -> mem_alloc_w(0x1f), toter Epilog) und Destruktor ((flags & 1) ->
 *   mem_free_v, flags in SI). Mit -rd falsch (r/n landen sonst nicht in SI/DI).
 * - drink_test: register-Parameter drank (SI), ok OHNE register (DI); mit "register ok" vertauscht.
 * - run: "anim_load(g_screen + 1, ..)" - das Original adressiert g_screen_pic hier als g_screen + 1 (Fixup).
 * - order: "case 3: eat();" ohne break in "default: break;" (sonst Epilogkopie bzw. dritter Epilog).
 * - set_crowd: then-Zweig endet mit "return;" (Epilogkopie, Spruenge der inneren Kette dorthin), else-Zweig mit
 *   return je Fall; Andrang als verschachteltes ?: mit gemeinsamem Store.
 * - add_bill: "bill += (signed short)v" (mov ax,[v] / add es:[..],ax); ohne Cast Lesen/Addieren/Schreiben.
 * - "sel = dlg_choose(..) - 1" -> dec ax / mov di,ax; "g_mouse_rclick = g_action = 0" (g_action zuerst);
 *   "!done" -> neg/sbb/inc.
 * Originalfehler/Auffaelliges: set_crowd "t < 12 Uhr || (t >= 2 Uhr && t < 3 Uhr)" (bzw. 17 Uhr) - die zweite
 *   Teilbedingung ist wirkungslos (vermutlich t >= 12 Uhr gemeint). run: "n == 4 ? txt[1] : txt[5]" ist tot
 *   (n ist 5 oder 6).
 */
#include <stdio.h>
#include "schweif.h"

#define TAVERN_TXT	0x1e		/* Texte der Schenken (0xa0 Stueck) */
#define TAVERN_TXT_N	0xa0
#define TAVERN_DATA	0x1f4		/* Datei mit 7-Byte-Saetzen je Schenke */
#define HOURS(n)	((n) * TICKS_PER_HOUR)

Tavern::Tavern()
{
	g_tavern = this;
	txt_load_list(g_font, TAVERN_TXT, TAVERN_TXT_N, &txt_buf, &txt);
	arc_open(g_file_3, TAVERN_DATA);
	arc_seek(g_file_3, (signed long)g_loc_id * 7, 0);
	arc_read(g_file_3, &price_mod, 7L);
	arc_close(g_file_3);
	done = 0;
	g_refresh = 1;
	hours = 0;
	g_tavern_crowd = 0;
	bill = 0;
	seat = 0;
	drinks = 0;
	seat_changes = 0;
	rounds = 0;
	script = -1;
}

Tavern::~Tavern()
{
	mem_free(txt_buf);
	mem_free(txt);
	g_tavern = 0;
	g_tavern_crowd = 0;
}

signed short Tavern::admit()
{
	tavern_check_hours(this, g_daytime);
	if (done == 0)
		check_ban();
	if (done == 0)
		drink_test(0);
	return !done;
}

void Tavern::run()
{
	signed long t;
	struct hero far *h;
	register signed short n;
	register signed short sel;

	tavern_check_hours(this, g_daytime);
	set_crowd(g_daytime);
	tavern_entrance(this);
	g_refresh = 1;
	while (done == 0) {
		handle_input(g_screen);
		if (g_refresh != 0) {
			t = g_daytime;
			set_crowd(t);
			n = g_tavern_crowd > 1 ? 6 : 5;
			if (g_loc_id >= 9 && g_loc_id <= 18 && n == 6)
				n = 5;
			draw_loc_icons(g_screen, n, 0x15, 0x0d, 0x19, 0x39, n == 5 ? 8 : 0x3a, 8);
			screen_redraw(g_screen);
			anim_load(g_screen + 1, hours == 2 ? 0x2c : 0x18f, 0);
			screen_textbar(g_screen, txt[g_loc_name], 0xff);
			snd_play(g_snd_music, 6);
			g_refresh = 0;
		}
		if (t + TICKS_PER_HOUR <= g_daytime) {
			t = g_daytime;
			hourly_event();
		}
		check_closing(g_daytime);
		check_ban();
		tavern_night_event(this);
		if (g_mouse_rclick != 0 || g_action == KEY_PGUP) {
			sel = dlg_choose(g_dialog, txt[0], n, txt[2], txt[0x60], txt[3], n == 4 ? txt[1] : txt[5],
				n == 5 ? txt[1] : txt[4], txt[1]) - 1;
			if (sel != -2)
				g_action = sel + ACTION_ICON_1;
		}
		if (g_action == ACTION_ICON_1) {
			tavern_talk(this);
		} else if (g_action == ACTION_ICON_2) {
			order();
		} else if (g_action == ACTION_ICON_3) {
			tavern_use_skill(this);
		} else if (g_action == ACTION_ICON_4 && n > 4) {
			tavern_change_seat(this);
		} else if (g_action == ACTION_ICON_5 && n > 5) {
			h = get_first_active_hero(g_group);
			sprintf(g_text_output_buf, txt[0x16], h->name);
			tavern_round(this);
		} else if (n + ACTION_ICON_1 - 1 == g_action || g_key == KEY_ESC) {
			done = 1;
			if (bill != 0) {
				tavern_pay_bill(this, 0x44);
			} else {
				sprintf(g_text_output_buf, txt[0x46], female ? txt[0x82] : txt[0x81]);
				dlg_message(g_dialog, g_text_output_buf);
			}
		}
		if (g_leave_location != 0)
			done = 1;
	}
}

void Tavern::set_crowd(signed long t)
{
	register signed short r;

	r = random_schweif(99) + quality;
	if (hours == 1) {
		if (t < HOURS(12) || (t >= HOURS(2) && t < HOURS(3)))
			g_tavern_crowd = r <= 10 ? 3 : (r <= 20 ? 2 : 1);
		else if (t >= HOURS(15) && t < HOURS(23))
			g_tavern_crowd = r <= 100 ? 3 : (r <= 110 ? 2 : 1);
		else
			g_tavern_crowd = r <= 20 ? 3 : (r <= 100 ? 2 : 1);
		return;
	}
	if (t < HOURS(17) || (t >= HOURS(2) && t < HOURS(3))) {
		g_tavern_crowd = r <= 10 ? 3 : (r <= 20 ? 2 : 1);
		return;
	}
	if (t >= HOURS(19)) {
		g_tavern_crowd = r <= 100 ? 3 : (r <= 110 ? 2 : 1);
		return;
	}
	g_tavern_crowd = r <= 20 ? 3 : (r <= 100 ? 2 : 1);
}

void Tavern::check_ban()
{
	if (g_tavern_banned[g_loc_id]) {
		screen_textbar(g_screen, txt[g_loc_name], 0xff);
		sprintf(g_text_output_buf, txt[0x0d], female ? txt[0x82] : txt[0x81]);
		dlg_message(g_dialog, g_text_output_buf);
		done = 1;
		g_mouse_rclick = g_action = 0;
		group_sub_le(g_group, 1);
		return;
	}
	if (g_loc_id == 3 && group_has_dwarf(g_group)) {
		screen_textbar(g_screen, txt[g_loc_name], 0xff);
		dlg_message(g_dialog, txt[0x18]);
		done = 1;
		g_mouse_rclick = g_action = 0;
	}
}

void Tavern::check_closing(signed long t)
{
	register signed short r = 0;

	if (t > HOURS(3)) {
		if (hours == 1) {
			if (t < HOURS(11))
				r = 1;
		} else if (t < HOURS(16))
			r = 1;
	}
	if (r) {
		done = 1;
		if (bill != 0) {
			tavern_pay_bill(this, 0x40);
		} else {
			sprintf(g_text_output_buf, txt[0x0e], female ? txt[0x80] : txt[0x7f], female ? txt[0x84] : txt[0x83],
				female ? txt[0x84] : txt[0x83]);
			dlg_message(g_dialog, g_text_output_buf);
		}
		g_mouse_rclick = g_action = 0;
	}
}

void Tavern::drink_test(register signed short drank)
{
	struct hero far *h;
	signed short ok = 0;

	if (drank)
		drinks++;
	h = get_first_hero(g_group);
	do {
		if (drank && check_hero3(h) && !h->drunk && test_skill(h, TA_ZECHEN, drinks * 5 - 20) <= 0) {
			sprintf(g_text_output_buf, txt[0x61], h->name);
			dlg_message(g_dialog, g_text_output_buf);
			hero_get_drunken(h);
		}
		if (!h->drunk)
			ok = 1;
		h = get_next_hero(g_group, h);
	} while (get_first_hero(g_group) != h);
	if (!ok) {
		done = 1;
		if (bill != 0) {
			screen_textbar(g_screen, txt[g_loc_name], 0xff);
			tavern_pay_bill(this, 0x42);
		} else {
			screen_textbar(g_screen, txt[g_loc_name], 0xff);
			sprintf(g_text_output_buf, txt[0x3a], female ? txt[0x80] : txt[0x7f]);
			dlg_message(g_dialog, g_text_output_buf);
		}
		g_mouse_rclick = g_action = 0;
	}
}

void Tavern::hourly_event()
{
	signed short r;

	if (seat == 2 && random_schweif(100) <= event_chance) {
		r = dlg_choose(g_dialog, txt[0x64], 2, txt[0x65], txt[0x66]);
		if (r == 1) {
			seat = 3;
			dlg_message(g_dialog, txt[0x67]);
			return;
		}
		dlg_message(g_dialog, txt[0x68]);
		seat_changes++;
	}
}

signed short Tavern::eat()
{
	signed long cost;
	register signed short d;

	cost = group_count_alive(g_group) * (6 - (quality >> 2));
	cost += cost * price_mod / 100;
	sprintf(g_text_output_buf, txt[0x24], (signed short)cost, female ? txt[0x80] : txt[0x7f]);
	if (dlg_yes_no(g_dialog, g_text_output_buf) == 1) {
		d = 21 - quality;
		if (d < 1)
			d = 1;
		group_sub_hunger_thirst_max(g_group, d * 20, d * 30);
		dlg_message(g_dialog, quality < 5 ? txt[0x25] : (quality < 15 ? txt[0x26] : txt[0x27]));
		timewarp_hours(g_date, 1L);
		if (drinks <= 5)
			drinks--;
		if (drinks < 0)
			drinks = 0;
		cost *= 10;
		if (g_group->money < cost) {
			sprintf(g_text_output_buf, txt[0x28], female ? txt[0x80] : txt[0x7f], female ? txt[0x80] : txt[0x7f]);
			dlg_message(g_dialog, g_text_output_buf);
			g_tavern_banned[g_loc_id] = 1;
			group_sub_money(g_group, g_group->money);
			group_sub_le_range(g_group, 1, 3);
			done = 1;
		} else {
			group_sub_money(g_group, cost);
		}
		return 1;
	} else
		return 0;
}

void Tavern::order()
{
	signed short r;
	register signed short n;

	if (g_loc_id >= 3 && g_loc_id <= 5) {
		tavern_order_dinner(this);
		return;
	}
	if (g_loc_id >= 9 && g_loc_id <= 18) {
		tavern_order_drink(this);
		return;
	}
	n = group_count_check2b(g_group);
	if (n > 1)
		sprintf(g_text_output_buf, txt[0x96], n);
	else
		sprintf(g_text_output_buf, txt[0x3c]);
	r = dlg_choose(g_dialog, txt[0x3b], 3, g_text_output_buf, txt[0x3d], txt[0x3e]);
	switch (r) {
	case 1:
		group_sub_hunger_thirst_max(g_group, 5, (21 - quality) * 30);
		if (n > 1)
			sprintf(g_text_output_buf, txt[0x95], female ? txt[0x80] : txt[0x7f]);
		else
			sprintf(g_text_output_buf, txt[0x33], female ? txt[0x80] : txt[0x7f]);
		dlg_message(g_dialog, g_text_output_buf);
		timewarp_minutes(g_date, 30L);
		add_bill(0);
		drink_test(1);
		return;
	case 2:
		group_sub_hunger_thirst_max(g_group, 4, (21 - quality) * 30);
		sprintf(g_text_output_buf, txt[0x3f], female ? txt[0x82] : txt[0x81]);
		dlg_message(g_dialog, g_text_output_buf);
		timewarp_minutes(g_date, 30L);
		add_bill(1);
		drinks++;
		drink_test(1);
		return;
	case 3:
		eat();
	default:
		break;
	}
}

void Tavern::add_bill(register signed short k)
{
	signed long v;

	v = group_count_check2b(g_group) + 1;
	v += v * price_mod / 100;
	if (k == 1 || k == 2)
		v *= 3;
	else if (k == 4)
		v /= 2;
	bill += (signed short)v;
}
