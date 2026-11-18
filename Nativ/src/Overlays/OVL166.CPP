/* OVL166 - Overlay O166 (Segment 0x229f in SCHWEIF.EXE): Sonderorte einer Stadt (Klasse Sites, 8 Bytes, s_class.h:
 * Texte 0x21a (0x64 Stueck) per txt_load_list; O030 ruft explore/evening_fight/challenge/rematch fuer die Ereignis-
 * orte 1..4, O178 inn_vision, O258 talk) und der Kartenereignis-Rueckruf dieser Stadt (Town::sites_event, Eintrag 2
 * in g_town_descs; Town aus O183, 0x12 Bytes). Die Stadt ist nicht bestimmt; Deutungen der Szenen sind vermutet.
 * Town::sites_event(ev) (O166_0000): wie port_event in O249 (xy = (y - g_map_x0) << 8 | (x - g_map_y0)): Zelle
 *   betreten -> sites_shock, drei Felder town_enter(LOC_DATA | 0x14..0x16), acht Felder sites_hint; nachts
 *   (g_daytime <= 7 Uhr oder > 19 Uhr), ohne g_sites_night_done, mit g_artherion_hut_visited == 1 und W100 <= 30 lokales
 *   Sites-Objekt mit night_encounter (g_sites_night_done = 1); Feldtyp 3 -> town_swim_check. Aktion 1: switch xy
 *   (20 Felder) -> town_enter(LOC_DATA | 0x01..0x16), sonst LOC_HOUSE. Aktion 100: 0xe0f -> LOC_DATA | 0x0e,
 *   sonst Marktstand 1..3: loc_load_data(0x0e), town_market_stall(k), town_enter(LOC_MERCHANT).
 * Town::sites_shock (O166_0435): einmal (g_sites_shock_done) ein Zauberkundiger (Hexe, Waldelf, Auelf, Firnelf,
 *   Magier, Druide, sonst ein zufaelliger Held), Klang 0xab, Meldung txt[0], 15 LE Schaden. Town::sites_hint
 *   (O166_0578): einmal (g_sites_hint_done) txt[1].
 * Sites::inn_vision (O166_05A5, aus der Herberge O178): Held mit der hoechsten Intuition, Probe Goetter/Kulte+3
 *   des Besten, Text in einen Textbildschirm (struct text_screen: tscr_open/tscr_set_anim 0x220/0x221/tscr_print)
 *   mit Warten auf g_action; g_sites_vision_ok/_failed. talk() (O166_0813, aus O258): Fragefolgen dlg_choose,
 *   Merker g_artherion_heard/g_topic_artherion/g_artherion_hut_known. night_encounter (O166_0A91): Bild 0xc2 (Bmp 32x32 per Nvf g_nvf_heads) mit
 *   Fragen dlg_box_bmp, g_sites_night_met. sites_script_a4 (O166_0D14): conv_run(g_dialog2, 0xa4) -> 30
 *   Minuten. challenge/rematch/evening_fight (O166_0D3B/1313/1687): Folgen mit Kampf 0xa7 (start_fight, Texte
 *   vorher frei und danach neu geladen) bzw. Abfindung (Gruppengeld, group_sub_money), g_sites_challenges/
 *   g_sites_rematch/g_sites_paid/g_sites_evening, g_hero_died/g_guide_avail. explore (O166_1A37): Held mit der
 *   hoechsten Neugier, Probe KL+2 des Klugsten, 3 Stunden, g_sites_explored. sites_guide_limit (O166_1BA7):
 *   g_guide_avail auf hoechstens 1.
 *
 * BCC -mlarge -3 -Ot -Og -Oi -c -Y OVL166.CPP   -> 7096/7096 Bytes GLEICH (18.10.2026), cobjfix laeuft
 *   (-rd ebenso GLEICH; ohne -Og 2 Bytes anders: Epilogkopie hinter dem switch in evening_fight).
 *   (Namensfassung mit schweif.h: 19.10.2026 GLEICH, LINKGLEICH)
 * - -Oi: strcat/strcpy (g_text_output_buf/g_text_buf2/txt[]) als Intrinsics.
 * - C++: Konstruktor/Destruktor wie OVL031 Ev; lokales Sites o in sites_event, lokale Bmp mitten im Rumpf
 *   (Konstruktor nach dlg_set16), in rematch zwei Bmp in Geschwisterbloecken (old erst im zweiten Block).
 * - sites_event: k (Aktion 100) in eigenem Block - teilt sich den Stackplatz mit dem Sites-Objekt ([bp-0ch]).
 * - evening_fight: "else { if (!g_sites_reward) { ..; return; } .. }" - mit "else if (..) {..; return;} else"
 *   toter Epilog; switch (k) mit Rumpfreihenfolge 2, 1, 0; "a = -1;" als Anweisung nach "k = 0" in der
 *   Deklaration.
 * Originalfehler/Auffaelliges: challenge erhoeht g_sites_challenges (1..4) vor der Abfrage "== 1" - txt[0x3e]
 *   ist unerreichbar; sites_event Aktion 100: k bleibt bei unbekanntem Feld uninitialisiert (wie O183);
 *   rematch zeigt bei Antwort 2 und 3 denselben Text txt[0x43].
 */
#include <stdio.h>
#include <string.h>
#include "schweif.h"

#define NVF_HEADS	((Nvf far *)g_nvf_heads)	/* NVF-Dateiobjekt (O098) */
#define SITES_TXT	0x21a		/* Texte der Sonderorte (0x64 Stueck) */
#define SITES_TXT_N	0x64
#define SITES_PICS	0x0a		/* NVF-Datei der Personenbilder (32 x 32) */
#define FIGHT_SITES	0xa7		/* Kampf der Sonderorte */
#define ITEM_REWARD	0x129		/* Gegenstand nach dem Abendkampf */

signed short Town::sites_event(struct map_event far *ev)
{
	signed short loc;
	signed short r;
	register signed short type;
	register signed short xy;

	type = ev->type;
	xy = (ev->y - g_map_x0) << 8;
	xy |= ev->x - g_map_y0;
	r = 0;
	if (type == MAP_EVENT_ENTER) {
		sites_shock();
		if (xy == 0xd01) {
			enter(LOC_DATA | 0x14, ev);
			r = 1;
		} else if (xy == 0x91e) {
			enter(LOC_DATA | 0x15, ev);
			r = 1;
		} else if (xy == 0x1212) {
			enter(LOC_DATA | 0x16, ev);
			r = 1;
		} else if (xy == 0xa0b || xy == 0xe0b || xy == 0xf0c || xy == 0xf0e || xy == 0xe10 ||
			xy == 0xb10 || xy == 0xa0e || xy == 0xc0d) {
			sites_hint();
			r = 1;
		}
		if ((g_daytime <= 7 * TICKS_PER_HOUR || g_daytime > 19 * TICKS_PER_HOUR) && !g_sites_night_done &&
			g_artherion_hut_visited == 1 && random_schweif(100) <= 30) {
			Sites o;

			o.night_encounter();
			g_sites_night_done = 1;
		}
		if ((signed char)get_cell_a(ev->x, ev->y) == 3)
			swim_check();
	} else if (type == 1) {
		loc = 0;
		switch (xy) {
		case 0x80d:
			loc = LOC_DATA | 0x01;
			break;
		case 0x602:
			loc = LOC_DATA | 0x02;
			break;
		case 0xd14:
			loc = LOC_DATA | 0x03;
			break;
		case 0xe0a:
			loc = LOC_DATA | 0x04;
			break;
		case 0x100e:
			loc = LOC_DATA | 0x05;
			break;
		case 0xa11:
			loc = LOC_DATA | 0x06;
			break;
		case 0xb18:
			loc = LOC_DATA | 0x07;
			break;
		case 0xf04:
			loc = LOC_DATA | 0x08;
			break;
		case 0x417:
			loc = LOC_DATA | 0x09;
			break;
		case 0x100d:
			loc = LOC_DATA | 0x0a;
			break;
		case 0x908:
			loc = LOC_DATA | 0x0b;
			break;
		case 0x506:
			loc = LOC_DATA | 0x0c;
			break;
		case 0x121d:
			loc = LOC_DATA | 0x0d;
			break;
		case 0x41d:
			loc = LOC_DATA | 0x10;
			break;
		case 0x80f:
			loc = LOC_DATA | 0x11;
			break;
		case 0xf1a:
			loc = LOC_DATA | 0x12;
			break;
		case 0x1007:
			loc = LOC_DATA | 0x13;
			break;
		case 0xd01:
			loc = LOC_DATA | 0x14;
			break;
		case 0x91e:
			loc = LOC_DATA | 0x15;
			break;
		case 0x1212:
			loc = LOC_DATA | 0x16;
			break;
		default:
			enter(LOC_HOUSE, ev);
			r = 1;
			break;
		}
		if (loc) {
			enter(loc, ev);
			r = 1;
		}
	} else if (type == 100) {
		if (xy == 0xe0f) {
			enter(LOC_DATA | 0x0e, ev);
		} else {
			signed short k;

			switch (xy) {
			case 0xb0c:
				k = 1;
				break;
			case 0xd0c:
				k = 2;
				break;
#if FIX_0186
			/* Fix #186: k fuer unbekannte Felder vorbelegen */
			case 0xd0e:
				k = 3;
				break;
			default:
				k = 1;
				break;
			}
#else
			case 0xd0e:
				k = 3;
				break;
			}
#endif
			loc_load_data(0x0e);
			market_stall(k);
			enter(LOC_MERCHANT, ev);
		}
	}
	return r;
}

Sites::Sites()
{
	txt_load_list(g_font, SITES_TXT, SITES_TXT_N, &txt_buf, &txt);
}

Sites::~Sites()
{
	mem_free(txt_buf);
	mem_free(txt);
}

void Town::sites_shock()
{
	struct hero far *h;

	if (!g_sites_shock_done) {
		h = get_hero_of_type(g_group, HERO_TYPE_WITCH);
		if (h == 0)
			h = get_hero_of_type(g_group, HERO_TYPE_SYLVAN_ELF);
		if (h == 0)
			h = get_hero_of_type(g_group, HERO_TYPE_GREEN_ELF);
		if (h == 0)
			h = get_hero_of_type(g_group, HERO_TYPE_ICE_ELF);
		if (h == 0)
			h = get_hero_of_type(g_group, HERO_TYPE_MAGE);
		if (h == 0)
			h = get_hero_of_type(g_group, HERO_TYPE_DRUID);
		if (h == 0)
			h = get_random_alive_hero(g_group);
		snd_play(g_snd_fx, 0xab);
		sprintf(g_text_output_buf, txt[0], h->name);
		dlg_message(g_dialog, g_text_output_buf);
		sub_hero_le(h, 15);
		g_sites_shock_done = 1;
	}
}

void Town::sites_hint()
{
	if (!g_sites_hint_done) {
		dlg_message(g_dialog, txt[1]);
		g_sites_hint_done = 1;
	}
}

void Sites::inn_vision()
{
	struct hero far *h;
	signed char r;
	struct text_screen ts;

	h = get_hero_attrib_max(g_group, ATTRIB_IN);
	sprintf(g_text_output_buf, txt[2], h->name, grammar_pronoun(h->sex, 0, 0));
	dlg_message(g_dialog, g_text_output_buf);
	h = get_best_hero_skill(g_group, TA_GOETTER_KULTE);
	r = test_skill(h, TA_GOETTER_KULTE, 3);
	sprintf(g_text_output_buf, txt[0x56]);
	if (r > 0) {
		sprintf(g_text_buf2, txt[3], h->name);
		strcat(g_text_output_buf, g_text_buf2);
		g_sites_vision_ok = 1;
	} else {
		strcat(g_text_output_buf, txt[4]);
		g_sites_vision_failed = 1;
	}
	mouse_hide(0);
	tscr_open(&ts);
	tscr_set_anim(&ts, 0x220, 0);
	tscr_print(&ts, g_text_output_buf, 0x32, 0x14, 0x118);
	do {
		poll_input();
	} while (g_action == 0);
	tscr_set_anim(&ts, 0x221, 2);
	tscr_print(&ts, txt[5], 0x32, 0x14, 0x118);
	do {
		poll_input();
	} while (g_action == 0);
	tscr_print(&ts, txt[0x57], 0x32, 0x14, 0x118);
	g_refresh = 0;
	do {
		poll_input();
	} while (g_action == 0);
	tscr_close(&ts, 1);
	ani_stop(g_screen_pic);
	mouse_show(0);
	clear_screen_buf(g_vga_buf);
}

void Sites::talk()
{
	signed char c;
	signed char r;
	signed short old;
	register signed short done = 0;

	old = dlg_set16(g_dialog, 5);
	r = -1;
	do {
		c = dlg_choose(g_dialog, txt[6], 3, txt[7], txt[8], txt[9]);
	} while (c == -1);
	switch (c) {
	case 1:
		dlg_message(g_dialog, txt[10]);
		break;
	case 2:
		r = 1;
		break;
	case 3:
		c = dlg_choose(g_dialog, txt[0xb], 3, txt[0xc], txt[0xd], txt[0xe]);
		switch (c) {
		case 2:
			r = 1;
			break;
		case 3:
			dlg_message(g_dialog, txt[0x15]);
			done = 1;
			break;
		}
		break;
	}
	if (r == 1) {
		done = 1;
		do {
			r = dlg_choose(g_dialog, txt[0x10], 3, txt[0x11], txt[0x12], txt[0x13]);
		} while (r == -1);
		if (r == 1) {
			dlg_message(g_dialog, txt[0x51]);
			r = dlg_choose(g_dialog, txt[0x14], 3, txt[0x59], txt[0x5a], txt[0x13]);
		}
		switch (r) {
		case 1:
			break;
		case 2:
			dlg_message(g_dialog, txt[0xf]);
			break;
		case 3:
			dlg_message(g_dialog, txt[0x16]);
			break;
		}
	}
	if (done && !g_artherion_hut_known)
		g_artherion_heard = g_topic_artherion = g_artherion_hut_known = 1;
	dlg_set16(g_dialog, old);
}

void Sites::night_encounter()
{
	signed short w;
	signed char r;
	signed short old;

	old = dlg_set16(g_dialog, 5);
	Bmp b(0x20, 0x20);
	NVF_HEADS->load(SITES_PICS, b.data(), 0, 0xc2, 0, &w, &w, 0x140);
	dlg_box_bmp(g_dialog, &b, (char far *)0, txt[0x17], 0);
	r = dlg_box_bmp(g_dialog, &b, (char far *)0, txt[0x18], 3, txt[0x19], txt[0x1a], txt[0x1b]);
	if (r == 2) {
		r = dlg_box_bmp(g_dialog, &b, (char far *)0, txt[0x1c], 3, txt[0x1d], txt[0x1e], txt[0x1f]);
		if (r == 3)
			dlg_box_bmp(g_dialog, &b, (char far *)0, txt[0x20], 0);
		if (r == 2) {
			r = dlg_box_bmp(g_dialog, &b, (char far *)0, txt[0x21], 3, txt[0x22], txt[0x23], txt[0x24]);
			switch (r) {
			case 1:
				dlg_box_bmp(g_dialog, &b, (char far *)0, txt[0x25], 0);
				break;
			case 2:
				dlg_box_bmp(g_dialog, &b, (char far *)0, txt[0x26], 0);
				break;
			case 3:
				dlg_box_bmp(g_dialog, &b, (char far *)0, txt[0x27], 0);
				break;
			}
		}
		g_sites_night_met = 1;
	}
	dlg_set16(g_dialog, old);
}

void sites_script_a4()
{
	if (conv_run(g_dialog2, 0xa4))
		timewarp_minutes(g_date, 30L);
}

void Sites::challenge()
{
	signed long money;
	signed long cost;
	signed short w;
	signed char c;
	signed char res;
	signed short old;

	if (g_sites_challenges <= 0 || g_sites_challenges >= 5) {
		dlg_message(g_dialog, txt[0x45]);
		return;
	}
	old = dlg_set16(g_dialog, 5);
	if (g_sites_challenges == 1)
		strcpy(g_text_buf2, txt[0x28]);
	else
		strcpy(g_text_buf2, txt[0x5b]);
	g_sites_challenges += 1;
	c = -1;
	res = 0;
	money = g_group->money;
	cost = g_sites_challenges * 200 - 200;
	Bmp b(0x20, 0x20);
	NVF_HEADS->load(SITES_PICS, b.data(), 0, 0x9d, 0, &w, &w, 0x140);
	while (c == -1)
		c = dlg_box_bmp(g_dialog, &b, (char far *)0, g_text_buf2, 3, txt[0x29], txt[0x2a], txt[0x2b]);
	if (c == 1) {
		mem_free(txt_buf);
		mem_free(txt);
		g_fight_flag = 1;
		res = start_fight(FIGHT_SITES, 0);
		txt_load_list(g_font, SITES_TXT, SITES_TXT_N, &txt_buf, &txt);
		if (res == 2) {
			g_hero_died = 1;
		} else {
			g_sites_challenges = -1;
			if (res == 0) {
				g_guide_avail = 3;
				c = dlg_yes_no(g_dialog, txt[0x2c]);
				if (c == 1) {
					timewarp_hours(g_date, 5L);
					dlg_message(g_dialog, txt[0x2d]);
				}
			}
		}
	} else {
		c = -1;
		while (c == -1)
			c = dlg_box_bmp(g_dialog, &b, (char far *)0, txt[0x2e], 3, txt[0x2f], txt[0x30], txt[0x31]);
		switch (c) {
		case 1:
			c = -1;
			while (c == -1)
				c = dlg_box_bmp(g_dialog, &b, (char far *)0, txt[0x32], 3, txt[0x33], txt[0x34], txt[0x35]);
			if (c == 1) {
				if (money >= cost)
					group_sub_money(g_group, cost);
				else
					group_sub_money(g_group, money);
				sprintf(g_text_output_buf, txt[0x36], (signed short)(cost / 100));
				dlg_box_bmp(g_dialog, &b, (char far *)0, g_text_output_buf, 0);
				g_sites_paid = g_sites_rematch = 1;
				g_sites_challenges = -1;
			}
			if (c == 2)
				dlg_box_bmp(g_dialog, &b, (char far *)0, txt[0x37], 0);
			break;
		case 2:
			dlg_box_bmp(g_dialog, &b, (char far *)0, txt[0x38], 0);
			break;
		case 3:
			c = -1;
			while (c == -1)
				c = dlg_box_bmp(g_dialog, &b, (char far *)0, txt[0x39], 3, txt[0x3a], txt[0x3b], txt[0x3c]);
			if (c == 1)
				dlg_box_bmp(g_dialog, &b, (char far *)0, txt[0x3d], 0);
			if (c == 3) {
#if FIX_0186
				/* Fix #186: erste Herausforderung: Zaehler ist hier schon erhoeht (2) */
				if (g_sites_challenges == 2) {
					dlg_box_bmp(g_dialog, &b, (char far *)0, txt[0x3e], 0);
#else
				if (g_sites_challenges == 1) {
					dlg_box_bmp(g_dialog, &b, (char far *)0, txt[0x3e], 0);
#endif
				} else {
					if (money >= cost)
						group_sub_money(g_group, cost);
					else
						group_sub_money(g_group, money);
					sprintf(g_text_output_buf, txt[0x36], (signed short)(cost / 100));
					dlg_box_bmp(g_dialog, &b, (char far *)0, g_text_output_buf, 0);
				}
				g_sites_paid = g_sites_rematch = 1;
				g_sites_challenges = -1;
			}
			break;
		}
	}
	dlg_set16(g_dialog, old);
}

void Sites::rematch()
{
	signed short w;
	register signed short r = 0;
	signed char ok = 1;

	if (!g_sites_rematch) {
		ok = 0;
		dlg_message(g_dialog, txt[0x45]);
	} else if (g_sites_rematch < 0) {
		if (g_sites_evening == -1) {
			Bmp b(0x20, 0x20);
			NVF_HEADS->load(SITES_PICS, b.data(), 0, 0xbe, 0, &w, &w, 0x140);
			dlg_box_bmp(g_dialog, &b, (char far *)0, txt[0x5e], 0);
			ok = 0;
		} else {
			r = dlg_yes_no(g_dialog, txt[0x4e]);
			if (r != 1)
				ok = 0;
		}
	}
	if (ok == 1) {
		signed short old;

		old = dlg_set16(g_dialog, 5);
		if (r == 0) {
			g_sites_rematch = -1;
			r = -1;
			ok = 0;
			Bmp b(0x20, 0x20);
			NVF_HEADS->load(SITES_PICS, b.data(), 0, 0xbe, 0, &w, &w, 0x140);
			while (r == -1)
				r = dlg_box_bmp(g_dialog, &b, (char far *)0, txt[0x3f], 3, txt[0x40], txt[0x41], txt[0x42]);
			switch (r) {
			case 1:
				ok = 1;
				break;
			case 2:
				dlg_box_bmp(g_dialog, &b, (char far *)0, txt[0x43], 0);
				ok = dlg_yes_no(g_dialog, txt[0x44]);
				break;
			case 3:
				dlg_box_bmp(g_dialog, &b, (char far *)0, txt[0x43], 0);
				break;
			}
		}
		if (ok == 1) {
			if (g_sites_rematch != -2) {
				mem_free(txt_buf);
				mem_free(txt);
				g_fight_flag = 1;
				ok = start_fight(FIGHT_SITES, 0);
				txt_load_list(g_font, SITES_TXT, SITES_TXT_N, &txt_buf, &txt);
				if (ok == 2) {
					g_hero_died = 1;
				} else if (ok == 0) {
					g_guide_avail = 3;
					g_sites_rematch = -2;
					r = dlg_yes_no(g_dialog, txt[0x2c]);
					if (r == 1) {
						timewarp_hours(g_date, 5L);
						dlg_message(g_dialog, txt[0x2d]);
					}
				}
			} else {
				dlg_message(g_dialog, txt[0x5c]);
			}
		}
		dlg_set16(g_dialog, old);
	}
}

void Sites::evening_fight()
{
	signed short w;
	signed char a;
	signed char k = 0;
	signed short old;

	a = -1;
	if (!g_sites_paid) {
		dlg_message(g_dialog, txt[0x45]);
		return;
	}
	old = dlg_set16(g_dialog, 5);
	if (g_daytime >= 18 * TICKS_PER_HOUR && g_daytime <= 22 * TICKS_PER_HOUR && !g_sites_evening) {
		g_sites_evening = 1;
		Bmp b(0x20, 0x20);
		NVF_HEADS->load(SITES_PICS, b.data(), 0, 0xbf, 0, &w, &w, 0x140);
		while (a == -1)
			a = dlg_box_bmp(g_dialog, &b, (char far *)0, txt[0x46], 3, txt[0x47], txt[0x48], txt[0x49]);
		switch (a) {
		case 1:
			k = 2;
			break;
		case 2:
			a = dlg_box_bmp(g_dialog, &b, (char far *)0, txt[0x4a], 3, txt[0x4b], txt[0x4c], txt[0x4d]);
			if (a == 2 || a == 3)
				k = 2;
			break;
		}
	} else {
		k = dlg_yes_no(g_dialog, txt[0x4e]);
		if (k == 1)
			a = 1;
		else
			a = 0;
	}
	dlg_set16(g_dialog, old);
	if (k == 2)
		a = dlg_yes_no(g_dialog, txt[0x4f]);
	if (k == 3)
		a = 1;
	if (k >= 1 && k < 4 && a == 1) {
		if (g_sites_evening != -1) {
			mem_free(txt_buf);
			mem_free(txt);
			g_fight_flag = 1;
			k = start_fight(FIGHT_SITES, 1);
			txt_load_list(g_font, SITES_TXT, SITES_TXT_N, &txt_buf, &txt);
			switch (k) {
			case 2:
				g_hero_died = 1;
				break;
			case 1:
				g_sites_evening = -2;
				break;
			case 0:
				g_guide_avail = 3;
				g_sites_evening = -1;
				a = dlg_yes_no(g_dialog, txt[0x2c]);
				if (a == 1) {
					timewarp_hours(g_date, 5L);
					dlg_message(g_dialog, txt[0x50]);
					g_sites_reward = 1;
					group_give_item(g_group, ITEM_REWARD, 1);
				}
				break;
			}
		} else {
			if (!g_sites_reward) {
				timewarp_hours(g_date, 5L);
				dlg_message(g_dialog, txt[0x50]);
				g_sites_reward = 1;
				group_give_item(g_group, ITEM_REWARD, 1);
				return;
			}
			dlg_message(g_dialog, txt[0x5c]);
		}
	}
}

void Sites::explore()
{
	struct hero far *h1;
	struct hero far *h2;
	signed short old;

	old = dlg_set16(g_dialog, 5);
	if (dlg_choose(g_dialog, txt[0x52], 2, txt[0x53], txt[0x54]) == 1) {
		h1 = get_hero_attrib_max(g_group, ATTRIB_NG);
		h2 = get_hero_attrib_max(g_group, ATTRIB_KL);
		sprintf(g_text_output_buf, txt[0x55], h1->name);
		if (test_attrib(h2, ATTRIB_KL, 2)) {
			sprintf(g_text_buf2, txt[0x5d], h2->name);
			strcat(g_text_output_buf, g_text_buf2);
		}
		dlg_message(g_dialog, g_text_output_buf);
		timewarp_hours(g_date, 3L);
		g_sites_explored = 1;
	}
	old = dlg_set16(g_dialog, old);
}

void sites_guide_limit()
{
	if (g_guide_avail > 1)
		g_guide_avail = 1;
}
