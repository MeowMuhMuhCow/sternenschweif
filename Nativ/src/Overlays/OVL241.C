/* OVL241 - Overlay O241 (Segment 0x244d in SCHWEIF.EXE): weitere Methoden der Schenke (struct tavern far *o, Klasse
 * Tavern aus O240, Zeiger g_tavern; BE seg059/seg060). tavern_check_hours(o, t): Oeffnungszeit pruefen (Stufe 6..13:
 * geschlossen zwischen 3 und 11 Uhr, sonst zwischen 3 und 16 Uhr), sonst hours = 1/2; geschlossen -> Ende.
 * tavern_entrance(o): Eingangsmenue - Symbolfelder (3/4 Knoepfe je Andrang g_tavern_crowd), Zeitlimit 180 Takte
 * (date_now/date_since) -> Mahnung, Wahl ACTION_ICON_1..3 -> seat, danach je Schenke tavern_order_dinner /
 * tavern_order_drink bzw. eigenes Menue (Gruppengroesse; 1: Platz nehmen tavern_add_bill/tavern_drink_test,
 * 2: tavern_eat, 3: tavern_round), sonst Meldung und Ende; 10 Minuten. tavern_round(o): Runde ausgeben (Preis W20 +
 * Andrang * 20 - 10, Unterauswahl bezahlen/prellen/...: group_sub_money, Hausverbot g_tavern_banned[g_loc_id],
 * group_sub_le_range/group_sub_le). tavern_pay_bill(o, t): Zeche bezahlen bzw. bei zu wenig Geld alles abgeben
 * (Geldtext money_to_string), Rueckgabe 0/1. tavern_use_skill(o): Talent anwenden (get_active_hero, use_skill ==
 * -2 -> Hausverbot), 30 Minuten. tavern_change_seat(o): Platzwahl (seat_changes: 3/4 Warnungen txt[57]/[56], 5
 * Rauswurf mit Heldennamen). tavern_talk(o): je Platz seat Gespraech (conv_run(g_dialog2, id): 1 Wirt je
 * Schenkengruppe +1/+9/+15/+37/+65/+77; 2 Meldung; 3 Geruechte/Personen 0x8c..0x96/0xa0.. mit Sperrzaehlern
 * g_guest_lock[7]/g_guest_lock2[3]/g_guest_lock_t2..t24 = W5 + 4 Stunden, sonst txt[99]). tavern_guests_left: noch
 * freie Personen? tavern_leave(o): Ende setzen. tavern_talk_event/tavern_talk_event2(state, answer):
 * Gespraechsereignisse (tavern_order bzw. tavern_order_drink, conv_unlock_topic, conv_finish).
 *
 * BCC -mlarge -3 -Ot -Og -c -Y OVL241.C   -> 5665/5665 Bytes GLEICH (18.10.2026), cobjfix laeuft (auch mit -Oi;
 *   ohne -Og 982 Abweichungen - Epilogkopien).
 *   (Namensfassung mit schweif.h: 19.10.2026 GLEICH, LINKGLEICH)
 * - Reines C wie O242; Varianten nach Wirt/Wirtin als Makros FC/FC2/FC3 (o->female ? txt[128] : txt[127] usw.).
 * - tavern_round: "price = g_tavern_crowd * 20 + random_schweif(20) - 10" (Aufruf trotzdem zuerst; push ax / imul /
 *   pop dx / add ax,dx); die Form "random_schweif(20) + g_tavern_crowd * 20 - 10" rechnet in dx. "if (sel != 1)
 *   {zahlen} else {..}" (Zweigreihenfolge im Code); nopt als "(signed long)(price * 10) <= money ? 3 : 1".
 * - tavern_change_seat: "default: ..; break;" im switch am Funktionsende (then-Zweig springt auf die Epilogkopie
 *   des break, danach der echte Epilog); "old = k = o->seat", "(k = o->seat) == old".
 * - tavern_talk: "default: return;" -> Nicht-Treffer springt auf eigenen Epilog, toter Schluss-Epilog.
 * - tavern_pay_bill: "(cost = o->bill) > money" eingebettet, "group_sub_money(g, cost = o->bill)".
 * - tavern_talk_event/2: "if (answer == 1) { ..; return -1; } else if .." -> toter jmp short hinter der
 *   Epilogkopie.
 * - since[8]: Zeitstempel fuer date_now/date_since (8 Bytes auf dem Stack).
 * Originalfehler/Auffaelligkeiten: tavern_entrance Wahl 3 leert g_text_output_buf per sprintf(.., ""),
 *   tavern_round prueft dann "g_text_output_buf != 0" (den Zeiger statt *g_text_output_buf) - immer wahr,
 *   dlg_message faengt den Leertext ab. tavern_round: nach "bezahlen" (Wahl 3 der Unterauswahl) wird rounds bei
 *   negativem Wert um 2 verringert, sonst auf -1 gesetzt.
 */
/* Phase B: BCC (C-Modus) legt das BSS in umgekehrter Reihenfolge der ersten Deklaration an - Reihenfolge hier vor schweif.h */
extern char g_unused_cb5c[];
extern signed short g_tavern_crowd;
#include <stdio.h>
#include "schweif.h"

/* Phase B: eigene Daten des Moduls (ds:75d2..75d8, dsana) - initialisierte Daten in Adressreihenfolge, danach der Literalpool */
struct tavern far *g_tavern = 0;
/* Phase B: eigenes BSS (ds:cb5a..cb6c) in Adressreihenfolge (Reihenfolge festgelegt durch die extern-Zeilen vor den #include) */
signed short g_tavern_crowd;		/* Andrang in der Schenke */
char g_unused_cb5c[16];			/* ds:cb5c, unbenutzt */

#define HOURS(n)	((n) * TICKS_PER_HOUR)
#define ENTRANCE_TIMEOUT 0xb4		/* Takte bis zur Mahnung im Eingangsmenue */

void tavern_round(struct tavern far *o);
signed short tavern_guests_left(struct tavern far *o);

#define FC (o->female ? o->txt[128] : o->txt[127])
#define FC2 (o->female ? o->txt[130] : o->txt[129])
#define FC3 (o->female ? o->txt[132] : o->txt[131])

void tavern_check_hours(struct tavern far *o, signed long t)
{
	if (o->quality >= 6 && o->quality <= 13) {
		if (t < HOURS(11) && t > HOURS(3)) {
			screen_textbar(g_screen, o->txt[g_loc_name], 0xff);
			dlg_message(g_dialog, o->txt[11]);
		} else
			o->hours = 1;
	} else {
		if (t < HOURS(16) && t > HOURS(3)) {
			screen_textbar(g_screen, o->txt[g_loc_name], 0xff);
			dlg_message(g_dialog, o->txt[12]);
		} else
			o->hours = 2;
	}
	if (o->hours == 0)
		o->done = 1;
}

void tavern_entrance(struct tavern far *o)
{
	signed char since[8];
	signed long t;
	signed char kind;
	signed short n;
	signed short nb;
	signed short sel;

	if (g_kvirasim_arrived) {
		o->seat = 2;
		g_kvirasim_arrived = 0;
		return;
	}
	kind = o->quality <= 5 ? 0 : (o->quality <= 15 ? 1 : 2);
	date_now(since);
	do {
		handle_input(g_screen);
		if (g_refresh) {
			t = g_daytime;
			tavern_set_crowd(o, t);
			nb = g_tavern_crowd > 1 ? 4 : 3;
			draw_loc_icons(g_screen, nb, 0x36, 0x37, nb == 3 ? 8 : 0x38, 8);
			screen_redraw(g_screen);
			anim_load(g_screen_pic, o->hours == 2 ? 0x2c : 0x18f, 0);
			screen_textbar(g_screen, o->txt[g_loc_name], 0xff);
			snd_play(g_snd_music, 6);
			g_refresh = 0;
		}
		tavern_check_closing(o, g_daytime);
		if (date_since(since) > ENTRANCE_TIMEOUT) {
			sprintf(g_text_output_buf, o->txt[kind + 41], FC);
			dlg_message(g_dialog, g_text_output_buf);
			if (kind == 2)
				dlg_message(g_dialog, o->txt[44]);
			g_mouse_rclick = 1;
		}
		if (g_mouse_rclick != 0 || g_action == KEY_PGUP) {
			sel = dlg_choose(g_dialog, o->txt[6], nb, o->txt[8], o->txt[9], nb == 3 ? o->txt[7] : o->txt[10],
				o->txt[7]) - 1;
			if (sel != -2)
				g_action = sel + ACTION_ICON_1;
			else
				g_action = nb + ACTION_ICON_1 - 1;
		}
		if (g_action == ACTION_ICON_1)
			o->seat = 1;
		else if (g_action == ACTION_ICON_2)
			o->seat = 2;
		else if (g_action == ACTION_ICON_3 && nb > 3)
			o->seat = 3;
		else if (nb + ACTION_ICON_1 - 1 == g_action || g_key == KEY_ESC)
			o->done = 1;
	} while (o->seat == 0 && o->done == 0);
	if (o->seat != 0) {
		dlg_message(g_dialog, o->txt[o->seat + 14]);
		if (g_loc_id >= 3 && g_loc_id <= 5) {
			if (!tavern_order_dinner(o)) {
				sprintf(g_text_output_buf, o->txt[kind + 52], FC);
				dlg_message(g_dialog, g_text_output_buf);
				o->done = 1;
			}
		} else if (g_loc_id >= 9 && g_loc_id <= 18) {
			if (!tavern_order_drink(o)) {
				sprintf(g_text_output_buf, o->txt[kind + 52], FC);
				dlg_message(g_dialog, g_text_output_buf);
				o->done = 1;
			}
		} else {
			n = group_count_check2b(g_group);
			if (n > 1)
				sprintf(g_text_buf2, o->txt[148], n);
			else
				sprintf(g_text_buf2, o->txt[48]);
			nb = g_tavern_crowd > 1 ? 3 : 2;
			sprintf(g_text_output_buf, o->txt[kind + 45], FC);
			sel = dlg_choose(g_dialog, g_text_output_buf, nb, g_text_buf2, o->txt[49], o->txt[50]);
			switch (sel) {
			case 1:
				if (n > 1)
					sprintf(g_text_output_buf, o->txt[149], FC);
				else
					sprintf(g_text_output_buf, o->txt[51], FC);
				dlg_message(g_dialog, g_text_output_buf);
				tavern_add_bill(o, 0);
				tavern_drink_test(o, 1);
				break;
			case 2:
				if (!tavern_eat(o)) {
					sprintf(g_text_output_buf, o->txt[kind + 52], FC);
					dlg_message(g_dialog, g_text_output_buf);
					o->done = 1;
				}
				break;
			case 3:
				if (g_tavern_crowd > 1) {
					sprintf(g_text_output_buf, "");
					tavern_round(o);
				}
				break;
			default:
				sprintf(g_text_output_buf, o->txt[kind + 52], FC);
				dlg_message(g_dialog, g_text_output_buf);
				o->done = 1;
				break;
			}
		}
	}
	if (o->done == 0)
		timewarp_minutes(g_date, 10L);
}

void tavern_round(struct tavern far *o)
{
	signed short nopt;
	signed short n;
	signed short sel;
	signed short price;

	n = group_count_check2b(g_group);
#if FIX_0158
	/* Fix #158: den Text pruefen statt den Zeiger */
	if (*g_text_output_buf != 0)
#else
	if (g_text_output_buf != 0)
#endif
		dlg_message(g_dialog, g_text_output_buf);
	price = g_tavern_crowd * 20 + random_schweif(20) - 10;
	nopt = (signed long)(price * 10) <= g_group->money ? 3 : 1;
	do {
		if (g_loc_id >= 3 && g_loc_id <= 5) {
			if (n > 1)
				sprintf(g_text_buf2, o->txt[152], n);
			else
				sprintf(g_text_buf2, o->txt[143]);
		} else {
			if (n > 1)
				sprintf(g_text_buf2, o->txt[151], n);
			else
				sprintf(g_text_buf2, o->txt[25]);
		}
		sprintf(g_text_output_buf, o->txt[23], FC2, price, FC);
		sel = dlg_choose(g_dialog, g_text_output_buf, nopt, g_text_buf2, o->txt[26], o->txt[27]);
		if (sel != -1) {
			if (sel != 1) {
				group_sub_money(g_group, (signed long)price * 10);
				o->rounds++;
				if (o->seat_changes > 0)
					o->seat_changes--;
				sprintf(g_text_output_buf, o->txt[28], FC2);
				dlg_message(g_dialog, g_text_output_buf);
				if (g_loc_id < 3 || g_loc_id > 5)
					tavern_drink_test(o, 1);
				timewarp_hours(g_date, 1L);
			} else {
				do {
					sel = dlg_choose(g_dialog, o->txt[29], nopt == 1 ? 2 : 3, o->txt[30], o->txt[31],
						o->txt[32]);
					if (sel != -1) {
						if (sel == 3) {
							sprintf(g_text_output_buf, o->txt[35], FC2, FC3);
							dlg_message(g_dialog, g_text_output_buf);
							group_sub_money(g_group, (signed long)price * 10);
							if (g_loc_id < 3 || g_loc_id > 5)
								tavern_drink_test(o, 1);
							timewarp_hours(g_date, 1L);
							if (o->rounds < 0)
								o->rounds -= 2;
							else
								o->rounds = -1;
						} else {
							if (sel == 2) {
								dlg_message(g_dialog, o->txt[34]);
								group_sub_le_range(g_group, 1, 4);
								g_tavern_banned[g_loc_id] = 1;
							} else {
								dlg_message(g_dialog, o->txt[33]);
								group_sub_le(g_group, 1);
								g_tavern_banned[g_loc_id] = 1;
							}
							o->done = 1;
							g_mouse_rclick = g_action = 0;
						}
					}
				} while (sel == -1);
			}
		}
	} while (sel == -1);
}

signed short tavern_pay_bill(struct tavern far *o, register signed short v)
{
	signed long cost;
	signed short ret = 1;

	if (o->bill) {
		if ((cost = o->bill) > g_group->money) {
			sprintf(g_text_output_buf, o->txt[v + 1], FC, o->txt[133]);
			dlg_message(g_dialog, g_text_output_buf);
			g_tavern_banned[g_loc_id] = 1;
			group_sub_le_range(g_group, 2, 4);
			group_sub_money(g_group, g_group->money);
			ret = 0;
		} else {
			money_to_string(g_text_buf2, cost);
			sprintf(g_text_output_buf, o->txt[v], FC, g_text_buf2);
			dlg_message(g_dialog, g_text_output_buf);
			group_sub_money(g_group, cost = o->bill);
		}
	}
	return ret;
}

void tavern_use_skill(struct tavern far *o)
{
	struct hero far *h;

	h = get_active_hero(g_group, o->txt[21]);
	if (h != 0) {
		if (use_skill(h, 0, -1) == -2) {
			o->done = 1;
			g_tavern_banned[g_loc_id] = 1;
		}
		timewarp_minutes(g_date, 30L);
	}
}

void tavern_change_seat(struct tavern far *o)
{
	signed short nopt;
	signed short old;
	struct hero far *h;
	signed short sel;
	signed short k;

	nopt = g_tavern_crowd > 1 ? 3 : 2;
	old = k = o->seat;
	sel = dlg_choose(g_dialog, o->txt[6], nopt, o->txt[8], o->txt[9], o->txt[10]);
	if (sel != -1) {
		if (sel == 1)
			o->seat = 1;
		else if (sel == 2)
			o->seat = 2;
		else if (sel == 3 && nopt == 3)
			o->seat = 3;
		if (old != 1 || o->seat != 1) {
			if (g_tavern_crowd > 1)
				o->seat_changes++;
			o->script = -1;
		}
		switch (o->seat_changes) {
		case 3:
			dlg_message(g_dialog, o->txt[57]);
			break;
		case 4:
			dlg_message(g_dialog, o->txt[56]);
			break;
		case 5:
			h = get_first_active_hero(g_group);
			sprintf(g_text_output_buf, o->txt[55], h->name);
			dlg_message(g_dialog, g_text_output_buf);
			g_tavern_banned[g_loc_id] = 1;
			group_sub_le(g_group, 1);
			o->done = 1;
			g_mouse_rclick = g_action = 0;
			break;
		default:
			if ((k = o->seat) == old)
				dlg_message(g_dialog, o->txt[k + 17]);
			else
				dlg_message(g_dialog, o->txt[k + 14]);
			break;
		}
	}
}

void tavern_talk(struct tavern far *o)
{
	register signed short i;

	switch (o->seat) {
	case 1:
		if (g_loc_id == 16)
			g_low25_topic59 = 0;
		if (g_loc_id >= 1 && g_loc_id <= 2)
			o->script = g_loc_id + 1;
		else if (g_loc_id >= 3 && g_loc_id <= 5)
			o->script = g_loc_id + 9;
		else if (g_loc_id >= 6 && g_loc_id <= 8)
			o->script = g_loc_id + 15;
		else if (g_loc_id >= 9 && g_loc_id <= 18)
			o->script = g_loc_id + 37;
		else if (g_loc_id >= 19 && g_loc_id <= 22)
			o->script = g_loc_id + 65;
		else if (g_loc_id >= 23 && g_loc_id <= 25)
			o->script = g_loc_id + 77;
		if (o->script != -1 && conv_run(g_dialog2, o->script))
			timewarp_minutes(g_date, 30L);
		break;
	case 2:
		dlg_message(g_dialog, o->txt[98]);
		break;
	case 3:
		if (o->rounds > 3) {
			dlg_message(g_dialog, o->txt[random_schweif(20) + 104]);
			timewarp_minutes(g_date, 10L);
		} else {
			if (g_loc_id == 2) {
				if (!g_guest_lock_t2) {
					o->script = 0x93;
					g_guest_lock_t2 = random_schweif(5) + 5;
				}
			} else if (g_loc_id >= 19 && g_loc_id <= 22 && !g_port_talk) {
				if (!g_guest_lock_t19) {
					o->script = 0x94;
					g_guest_lock_t19 = random_schweif(5) + 5;
				}
			} else if (g_loc_id == 23) {
				if (!g_guest_lock_t23) {
					o->script = 0x95;
					g_guest_lock_t23 = random_schweif(5) + 5;
				}
			} else if (g_loc_id == 24) {
				if (!g_guest_lock_t24) {
					o->script = 0x96;
					g_guest_lock_t24 = random_schweif(5) + 5;
				}
			}
			if (o->script == -1) {
				if (tavern_guests_left(o)) {
					do {
						if (g_loc_id >= 9 && g_loc_id <= 18) {
							i = random_schweif(7) - 1;
							if (!g_guest_lock[i]) {
								o->script = i + 0x8c;
								g_guest_lock[i] = random_schweif(5) + 5;
							}
						} else if (g_loc_id >= 3 && g_loc_id <= 5) {
							i = random_schweif(3) - 1;
							if (!g_guest_lock2[i]) {
								o->script = i + 0xa0;
								g_guest_lock2[i] = random_schweif(5) + 5;
							}
						}
					} while (o->script == -1);
				}
			}
			if (o->script != -1) {
				if (conv_run(g_dialog2, o->script))
					timewarp_hours(g_date, 1L);
			} else
				dlg_message(g_dialog, o->txt[99]);
		}
		break;
	default:
		return;
	}
}

signed short tavern_guests_left(struct tavern far *o)
{
	signed short ok = 0;
	signed short i;

	if (g_loc_id >= 9 && g_loc_id <= 18) {
		for (i = 0; i < 7; i++)
			if (!g_guest_lock[i])
				ok = 1;
	} else if (g_loc_id >= 3 && g_loc_id <= 5) {
		for (i = 0; i < 3; i++)
			if (!g_guest_lock2[i])
				ok = 1;
	}
	return ok;
}

void tavern_leave(struct tavern far *o)
{
	o->done = 1;
}

signed short tavern_talk_event(signed short state, register signed short answer)
{
	signed short r = 0;

	if (g_tavern != 0 && state == 1) {
		if (answer == 1) {
			tavern_order(g_tavern);
			return -1;
		} else if (answer == 0x60) {
			conv_unlock_topic(g_dialog2, TOPIC_ARTHERION);
			if (!g_artherion_hut_known)
				g_artherion_heard = g_topic_artherion = 1;
			r = -1;
		} else if (answer == 0x63)
			conv_finish(g_dialog2);
	}
	return r;
}

signed short tavern_talk_event2(signed short state, register signed short answer)
{
	if (g_tavern != 0 && state == 1) {
		if (answer == 1) {
			tavern_order_drink(g_tavern);
			return -1;
		} else if (answer == 2 && g_loc_id == 13 && g_topic_siebenquell != -1) {
			g_topic_siebenquell = 1;
			conv_unlock_topic(g_dialog2, TOPIC_SIEBENQUELL);
		}
	}
	return 0;
}
