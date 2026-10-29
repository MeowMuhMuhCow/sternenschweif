/* OVL072 - Overlay O072 (Segment 0x20a4 in SCHWEIF.EXE): Lager/Rast (freie Funktionen camp_* mit dem Lagerobjekt
 * class Camp (s_class.h) als Kontext p, auch aus den Reiselagern O067/O073: Texte p->txt, job/watch_set Wachen
 * angefordert, site Rast erfolgt, mode (0 Wildnis, sonst Rast ohne Wache), pic Bild, penalty Folge; Gruppe g_group
 * (Helden, dungeon = Gebietsart, Geld, Wegstrecke route_pos), Wachen g_camp_watch_names[3]).
 * camp_set_foes(a, b, c): drei Gegnergruppen g_encounters[3] (struct encounter, je 4 Bytes, als Bytes geschrieben) aus g_camp_encounters (8 Bytes je Eintrag),
 *   Anzahl = count * 10 / (W11 + 9) bzw. / (W10 + 10), in Gebiet 7 halbiert. camp_night_encounter(): naechtliche
 *   Begegnung je Gebietsart (switch 1..16 auf group.dungeon, W100) bzw. zufaellig (W4), dann camp_set_foes -> 1.
 * camp_use_magic(p): Magier per get_active_mage(txt[32]) - beim Brauen txt[13]; nur Magier (is_mage): mit Wache
 *   (watches) oder Kraeutersuche (rest_herbs) txt[12], schon gezaubert (rest_magic) Meldung txt[29] mit
 *   hero_name_gen, sonst rest_magic = use_magic(h); kein Magier -> Text g_ttx[338].
 * camp_group_menu(p): Lagermenue (Leiste g_screen, Bild p->pic): 2..4 Punkte (Rasten, Gruppe wechseln bei
 *   g_nr_groups != 1, Gruppen zusammenfuehren camp_other_group_near, NSC group_get_npc -> Zusatzpunkt), Auswahl
 *   ueber g_action bzw. dlg_choose -> grp_split / group_switch_next / groups_merge / NSC entlassen (txt[123],
 *   groups_check_npc) / Ende.
 * camp_other_group_near(p): 1, wenn eine andere Gruppe aus g_groups[6] am selben Weg (route_from/route_to,
 *   Strecke route_pos +-300) bzw. ueber group_same_place erreichbar ist.
 * camp_rest(p): Rast - Stundenabfrage (dlg_input, mind. 3), Wachen der Reihe nach (get_hero_by_file_name auf
 *   g_camp_watch_names[w], Wachprobe test_attrib(h, ATTRIB_KK, mode), bei Misserfolg camp_reset_watch), Zeit
 *   vorruecken timewarp_hours/timewarp_until (g_date), Ueberfall bei Wache sel (W100 <= 25/15 bzw.
 *   g_camp_ambush_watch; nicht mit INGAME_TIMER_TRAVIA_SAFE_REST): camp_wake_up, camp_encounter bzw.
 *   camp_night_encounter, ggf. Diebstahl von Gegenstaenden (group_find_item/drop_item) und Geld (group_sub_money);
 *   danach Regeneration camp_regenerate, Folge p->penalty (-1..-3 Geld, 99 -> group_add_effect); Rueckgabe ret + 1
 *   bzw. 0 (abgebrochen), -1 nach dem Kampf aus camp_night_encounter.
 * camp_regenerate(p, n): Erholung je Held (Wert g_rest_mod, Ausruestung 0x126/0x48, Wetter date_season/g_camp_weather,
 *   Krankheit hero_disease_test, Bits rest_*, watches, Ergebnis per inline set_rest_bonus, hero_sleep(h, Bonus)).
 *   camp_wake_up(p, w): Wachwechsel/Wecken (set_asleep), schlafende Helden per strcat in g_text_output_buf
 *   sammeln und melden (txt[45..48]).
 *
 * BCC -mlarge -3 -Ot -Og -Oi -c -Y OVL072.CPP   -> 8015/8015 Bytes GLEICH (18.10.2026), cobjfix laeuft
 *   (Namensfassung mit schweif.h: 19.10.2026 GLEICH, LINKGLEICH)
 * - -Oi fuer die strcat-Intrinsics in camp_wake_up, -Og fuer die Byte-Zwischenwerte der Bitfelder (+ba, [bp-6]/
 *   [bp-8]), (int)hours in [bp-2ah] und die Epilogkopien in camp_use_magic (ohne -Og 11 Bytes zu kurz, ohne -Oi 70).
 * - Bitfelder +ba/+bc als "unsigned short b:1": Tests als (signed char)h->b (or al,al) bzw. !(signed char)h->b
 *   (and ax,1 / cbw / or ax,ax); inline set_rest_magic(signed char) ergibt mov al,[bp-5] / cbw / and ax,1.
 * - inline group::get_route_pos() (unsigned long): "d = g->get_route_pos()" -> mov eax / mov si,ax; in der Rechnung
 *   "g_route_km * 1000 - g->get_route_pos()" -> mov edx,es:[bx+33h] / sub ax,dx (direkt waere sub ax,es:[..]).
 * - Bedingung "!g_speech_off && g_sound_ready == 1" als zwei freie inline-Funktionen (neg/sbb/inc, mov ax,1/xor ax,ax/or).
 * - camp_regenerate: register nur am Parameter n (SI), Zaehler b ohne register (DI); mit register vertauscht.
 * - drop_item(h, get_item_pos(h, item), 1): add sp,6 nach drei Pushes verraet, dass "push 1" zum aeusseren
 *   Aufruf gehoert. "k = w; for (k += 1; ..)" ergibt mov al,[w] / mov [k],al / mov al,[k] / inc al / mov [k],al.
 * - inline is_mage() { return type >= 7; } -> mov ax,1 / xor ax,ax / or ax,ax; inline set_rest_bonus/set_watches
 *   mit Parameterkopie ([bp-7], [bp-27h]).
 * - Aufrufe mit int-Argumenten (push word) ueber gecastete Funktionszeiger: TEXTBAR, SET_ASLEEP, RESET_WATCH,
 *   GROUP_EFFECT.
 * Originalfehler/Auffaelliges: camp_set_foes wuerfelt W11 fuer die Anzahl zweimal (Vergleich "> 1" und
 *   gespeicherter Wert stammen aus verschiedenen Wuerfen). camp_other_group_near vergleicht d uninitialisiert, wenn
 *   keine der beiden Positionsbedingungen zutrifft. camp_rest: found wird nie zurueckgesetzt (gilt nach einem
 *   Treffer fuer alle weiteren Wachen), mode bleibt bei unbekanntem watches auf -5; Diebstahlschleife ueberspringt
 *   mit "k += (k + 13) / 10" Gegenstaende. camp_wake_up: last ist uninitialisiert, falls kein Held gesammelt wird.
 */
#include <stdio.h>
#include <string.h>
#include "schweif.h"

/* Phase B: eigene Daten des Moduls (ds:5882..5974, dsana) - initialisierte Daten in Adressreihenfolge */
signed short g_camp_ambush_watch = 0;
struct camp_encounter g_camp_encounters[30] = {
	{ 0, 0, 0, 0 },
	{ 2, 7, 0, 0 },
	{ 27, 5, 0, 0 },
	{ 72, 3, 0, 0 },
	{ 91, 2, 0, 0 },
	{ 33, 6, 0, 0 },
	{ 116, 4, 0, 0 },
	{ 45, 3, 0, 0 },
	{ 16, 4, 0, 0 },
	{ 115, 4, 0, 0 },
	{ 79, 4, 0, 0 },
	{ 9, 5, 0, 0 },
	{ 10, 4, 0, 0 },
	{ 39, 6, 0, 0 },
	{ 11, 4, 0, 0 },
	{ 81, 8, 0, 0 },
	{ 114, 1, 0, 0 },
	{ 66, 5, 0, 0 },
	{ 68, 3, 0, 0 },
	{ 69, 2, 0, 0 },
	{ 31, 4, 0, 0 },
	{ 23, 3, 0, 0 },
	{ 54, 6, 0, 0 },
	{ 108, 3, 0, 0 },
	{ 58, 7, 0, 0 },
	{ 39, 4, 0, 5 },
	{ 27, 4, 0, 8 },
	{ 91, 4, 0, 3 },
	{ 94, 4, 0, 3 },
	{ 116, 4, 0, 4 }
};

/* Text in die Textleiste: Farbe 0xff als int wie im Original */
#define TEXTBAR(t) ((void (*)(void far *, char far *, signed short))screen_textbar)(g_screen, t, 0xff)
/* Aufrufe mit int-Argumenten (push word) wie im Original */
#define SET_ASLEEP(h, v) ((void (*)(struct hero far *, signed short))set_asleep)(h, v)
#define RESET_WATCH(p, n, keep) ((void (*)(Camp far *, signed char, signed short))camp_reset_watch)(p, n, keep)
#define GROUP_EFFECT(t, a, b, c) ((void (*)(struct group far *, signed long, signed short, signed short, \
	signed short))group_add_effect)(g_group, t, a, b, c)

inline signed short no3903() { return !g_speech_off; }
inline signed short cond3905() { return no3903() && g_sound_ready == 1; }

void camp_set_foes(register signed short a, register signed short b, signed short c)
{
	signed char far *p;

	p = (signed char far *)&g_encounters[0];
	p[0] = g_camp_encounters[a].id;
#if FIX_0191
	/* Fix #191: Anzahl einmal wuerfeln (Vergleich und Wert aus demselben Wurf) */
	{
		signed short n_ = g_camp_encounters[a].count * 10 / (random_schweif(11) + 9);

		p[1] = n_ > 1 ? n_ : 1;
	}
#else
	p[1] = g_camp_encounters[a].count * 10 / (random_schweif(11) + 9) > 1 ?
		g_camp_encounters[a].count * 10 / (random_schweif(11) + 9) : 1;
#endif
	p[2] = g_camp_encounters[a].f4;
	p[3] = g_camp_encounters[a].f6;
	p = (signed char far *)&g_encounters[1];
	p[0] = g_camp_encounters[b].id;
	p[1] = g_camp_encounters[b].count * 10 / (random_schweif(10) + 10);
	if (g_group->dungeon == 7)
		p[1] = p[1] / 2;
	p[2] = g_camp_encounters[b].f4;
	p[3] = g_camp_encounters[b].f6;
	p = (signed char far *)&g_encounters[2];
	p[0] = g_camp_encounters[c].id;
	p[1] = g_camp_encounters[c].count * 10 / (random_schweif(10) + 10);
	if (g_group->dungeon == 7)
		p[1] = p[1] / 2;
	p[2] = g_camp_encounters[c].f4;
	p[3] = g_camp_encounters[c].f6;
}

signed short camp_night_encounter(void)
{
	signed short b;
	signed short c;
	signed short ret;
	register signed short a;
	register signed short r;

	a = -1;
	ret = b = c = 0;
	r = random_schweif(100);
	switch (g_group->dungeon) {
	case 1:
		if (r <= 50) {
			a = 0x11;
			b = 0x13;
		} else if (r <= 90)
			a = 0x16;
		break;
	case 2:
	case 3:
		break;
	case 4:
		if (r <= 20)
			a = 0x0f;
		else if (r <= 50) {
			a = 8;
			b = 9;
		} else if (r <= 70)
			a = 10;
		else
			a = 11;
		break;
	case 5:
		if (r <= 30) {
			a = 0x12;
			b = 0x13;
			c = 0x14;
		} else if (r <= 50)
			a = 0x15;
		else if (r <= 70)
			a = 0x18;
		break;
	case 6:
	case 7:
	case 8:
		if (r <= 20) {
			a = 1;
			b = 3;
			c = 0x1a;
		} else if (r <= 40) {
			a = 2;
			b = 4;
			c = 3;
		} else if (r <= 60) {
			a = 2;
			b = 0x1c;
			c = 0x1b;
		} else if (r <= 80) {
			a = 2;
			b = 3;
			c = 0x17;
		}
		break;
	case 12:
		if (r <= 40)
			a = 10;
		else
			a = 0;
		break;
	case 10:
	case 11:
	case 13:
		if (r <= 40) {
			a = 0x0d;
			b = 0x1a;
		} else if (r <= 60)
			a = 0x0c;
		else
			a = 0x0e;
		break;
	case 9:
	case 14:
	case 15:
	case 16:
		a = 0;
	}
	if (a == -1) {
		r = random_schweif(4);
		switch (r) {
		case 1:
			a = 5;
			if (random_schweif(10) <= 5) {
				b = 6;
				if (random_schweif(10) <= 5)
					c = 0x1d;
			}
			break;
		case 2:
			a = 7;
			if (random_schweif(10) <= 5)
				b = 0x1d;
			break;
		case 3:
			a = 8;
			if (random_schweif(10) <= 5) {
				b = 9;
				if (random_schweif(10) <= 5)
					c = 0x1d;
			}
			break;
		case 4:
			a = 10;
			if (random_schweif(10) <= 5)
				b = 0x1d;
			break;
		}
	}
	if (a) {
		camp_set_foes(a, b, c);
		ret = 1;
	}
	return ret;
}

void camp_use_magic(Camp far *p)
{
	struct hero far *h;

	h = get_active_mage(g_group, p->txt[32]);
	if (h != 0) {
		if ((signed char)h->brewing) {
			dlg_message(g_dialog, p->txt[13]);
			return;
		}
		if (h->is_mage()) {
			if (h->watches != 0 || (signed char)h->rest_herbs)
				dlg_message(g_dialog, p->txt[12]);
			else if ((signed char)h->rest_magic) {
				sprintf(g_text_output_buf, p->txt[29], hero_name_gen(h, 1));
				dlg_message(g_dialog, g_text_output_buf);
			} else
				h->set_rest_magic(use_magic(h));
		} else
			dlg_message(g_dialog, g_ttx[338]);
	}
}

void camp_group_menu(Camp far *p)
{
	signed short ans;
	signed short done;
	signed short redraw;
	struct hero far *h;
	register signed short n;
	register signed short npc;

	done = 0;
	redraw = 1;
	npc = 1;
	g_refresh = 0;
	screen_reset_icons(g_screen);
	anim_load(g_screen_pic, p->pic, 0);
	TEXTBAR(p->txt[0]);
	while (!done) {
		if (g_refresh) {
			screen_reset_icons(g_screen);
			screen_redraw(g_screen);
			anim_load(g_screen_pic, p->pic, 0);
			TEXTBAR(p->txt[0]);
			snd_play(g_snd_music, 0x0b);
			redraw = 1;
			g_refresh = 0;
		}
		if (redraw) {
			n = 2;
			if (g_nr_groups != 1)
				n++;
			if (camp_other_group_near(p))
				n++;
			if (group_get_npc(g_group))
				npc = 1;
			else
				npc = 0;
			screen_reset_icons(g_screen);
			if (n == 2) {
				if (!npc)
					draw_loc_icons(g_screen, n, 0x0c, 8);
				else
					draw_loc_icons(g_screen, n + 1, 0x0c, 0x42, 8);
			} else if (n == 3) {
				if (!npc)
					draw_loc_icons(g_screen, n, 0x0c, 0x1d, 8);
				else
					draw_loc_icons(g_screen, n + 1, 0x0c, 0x1d, 0x42, 8);
			} else if (n == 4) {
				if (!npc)
					draw_loc_icons(g_screen, n, 0x0c, 0x1d, 0x0f, 8);
				else
					draw_loc_icons(g_screen, n + 1, 0x0c, 0x1d, 0x0f, 0x42, 8);
			}
			screen_portraits(g_screen);
			anim_reset(g_screen_pic, 2);
			redraw = 0;
		}
		handle_input(g_screen);
		if (g_key == 0x11b) {
			g_key = 0;
			g_action = n + 0x81;
		}
		if (g_mouse_rclick || g_key == 0x4900) {
			if (group_get_npc(g_group))
				npc = 1;
			else
				npc = 0;
			if (n == 2) {
				if (!npc)
					ans = dlg_choose(g_dialog, p->txt[36], n, p->txt[37], p->txt[40]) - 1;
				else
					ans = dlg_choose(g_dialog, p->txt[36], n + 1, p->txt[37], p->txt[119], p->txt[40]) - 1;
			} else if (n == 3) {
				if (!npc)
					ans = dlg_choose(g_dialog, p->txt[36], n, p->txt[37], p->txt[39], p->txt[40]) - 1;
				else
					ans = dlg_choose(g_dialog, p->txt[36], n + 1, p->txt[37], p->txt[39], p->txt[119],
						p->txt[40]) - 1;
			} else if (n == 4) {
				if (!npc)
					ans = dlg_choose(g_dialog, p->txt[36], n, p->txt[37], p->txt[39], p->txt[38],
						p->txt[40]) - 1;
				else
					ans = dlg_choose(g_dialog, p->txt[36], n + 1, p->txt[37], p->txt[39], p->txt[38],
						p->txt[119], p->txt[40]) - 1;
			}
			if (ans != -2)
				g_action = ans + 0x81;
		}
		if (g_action == 0x81) {
			grp_split();
			redraw = 1;
			g_travel_state = 3;
		} else if (g_action == 0x82 && n >= 3) {
			group_switch_next(0);
			g_refresh = redraw = 1;
			if ((signed short)g_group->loctype != LOC_CAMP_WILD) {
				g_travel_state = 3;
				done = 1;
			} else
				g_travel_state = 5;
		} else if (g_action == 0x83 && n == 4) {
			groups_merge();
			redraw = 1;
			g_travel_state = 2;
		} else if (n + 0x80 == g_action) {
			if (!npc)
				done = 1;
			else {
				h = group_get_npc(g_group);
				sprintf(g_text_output_buf, p->txt[123], h->name);
				if (dlg_yes_no(g_dialog, g_text_output_buf) == 1) {
					g_npc_months = 0x63;
					groups_check_npc();
					redraw = 1;
				}
			}
		} else if (n + 0x81 == g_action)
			done = 1;
	}
}

signed short camp_other_group_near(Camp far *p)
{
	struct group far *g;
	signed short found;
	register signed short i;
	register unsigned short d;

	found = 0;
	if (g_nr_groups != 1) {
		for (i = 0, g = g_groups; i < 6; i++, g++) {
			if (group_get_on_route(g) && group_get_on_route(g_group)) {
#if FIX_0191
				/* Fix #191: Abstand nur vergleichen, wenn d fuer diesen Weg gesetzt wurde */
				if (g != g_group && !!g->nr_heroes) {
					signed short have = 0;

					if (g->route_from == g_group->route_from && g->route_to == g_group->route_to) {
						d = g->get_route_pos();
						have = 1;
					}
					if (g->route_from == g_group->route_to && g->route_to == g_group->route_from) {
						d = g_route_km * 1000 - g->get_route_pos();
						have = 1;
					}
					if (have && d - 300 < g_group->get_route_pos() && d + 300 > g_group->get_route_pos())
						found = 1;
				}
#else
				if (g != g_group && !!g->nr_heroes) {
					if (g->route_from == g_group->route_from && g->route_to == g_group->route_to)
						d = g->get_route_pos();
					if (g->route_from == g_group->route_to && g->route_to == g_group->route_from)
						d = g_route_km * 1000 - g->get_route_pos();
					if (d - 300 < g_group->get_route_pos() && d + 300 > g_group->get_route_pos())
						found = 1;
				}
#endif
			} else {
				if (g != g_group && !!g->nr_heroes && group_same_place(g_group, g))
					found = 1;
			}
		}
	}
	return found;
}

signed short camp_rest(Camp far *p)
{
	signed char k;
	signed char per;
	signed char w;
	signed char ret;
	signed char sel;
	signed char hours;
	signed char asked;
	signed char cnt;
	signed char res;
	signed short ok;
	signed short intr;
	signed short found;
	signed short save;
	signed long money;
	struct hero far *h1;
	struct hero far *hw;
	struct hero far *leader;
	struct hero far *other;
	register signed short mode;
	register signed short item;

	ret = 0;
	mode = -5;
	sel = -1;
	hours = 0;
	asked = 0;
	cnt = 0;
	res = 0;
	item = 0;
	ok = 0;
	intr = 0;
	found = 0;
	save = 0;
	leader = group_get_npc(g_group);
	other = get_hero_max_le(g_group);
	if (p->job == 0)
		ok = dlg_yes_no(g_dialog, p->txt[27]);
	else
		ok = 1;
	if (ok == 1) {
		hours -= g_daytime / TICKS_PER_HOUR;
		hours += hours > -8 ? 8 : 0x20;
		if (p->mode) {
			if (hours < 3)
				hours = 3;
			sprintf(g_text_output_buf, p->txt[31], 3, (signed short)hours);
			hours = asked = dlg_input(g_dialog, 2, g_text_output_buf, 3, (signed short)hours, (signed short)hours);
		}
		if (hours > 0) {
			per = hours / 3;
			w = 0;
			if (g_ingame_timers[INGAME_TIMER_TRAVIA_SAFE_REST] == 0) {
				if (g_camp_ambush_watch == 0) {
					if (random_schweif(100) <= (g_camp_watch_names[0][0] == 0 ? 0x19 : 0x0f))
						sel = random_schweif(3) - 1;
				} else
					sel = g_camp_ambush_watch - 1;
				if (sel != -1 && hours < 3)
					sel = 0;
			}
			do {
				if (p->job || p->watch_set)
					hw = get_hero_by_file_name(g_group, g_camp_watch_names[w]);
				else
					hw = 0;
				if (hw && check_hero3(hw)) {
					sprintf(g_text_output_buf, p->txt[28], hw->name);
					TEXTBAR(g_text_output_buf);
					switch (hw->watches) {
					case 1:
						mode = -8;
						break;
#if FIX_0191
					/* Fix #191: found je Wache zuruecksetzen */
					case 2:
						found = 0;
						for (k = 0; k < w; k++)
#else
					case 2:
						for (k = 0; k < w; k++)
#endif
							if (get_hero_by_file_name(g_group, g_camp_watch_names[k]) == hw)
								found = 1;
						if (found == 1)
							mode = 0;
						else
							mode = -8;
						break;
					case 3:
						switch (w) {
						case 0:
							mode = -7;
							break;
						case 1:
							mode = 0;
							break;
						case 2:
							mode = 8;
							break;
						}
					}
					if (check_hero3(hw) && test_attrib(hw, ATTRIB_KK, mode) <= 0) {
						wait_ticks_key(0x28);
						sprintf(g_text_output_buf, p->txt[44], hw->name);
						TEXTBAR(g_text_output_buf);
						k = w;
						for (k += 1; k <= 2; k++) {
							hw = get_hero_by_file_name(g_group, g_camp_watch_names[k]);
							hw->set_watches(hw->watches - 1);
						}
						RESET_WATCH(p, w, 1);
						p->job = p->watch_set = 0;
					}
				} else
					TEXTBAR(p->txt[0]);
				g_resting = 1;
				cnt = 0;
				if (w != sel) {
					do {
						timewarp_hours(g_date, 1L);
						cnt += 1;
						if (g_refresh_portraits)
							screen_portraits(g_screen);
					} while (cnt < per);
					if (w == 1 && leader->npc_id == 3) {
						save = g_opt_sound;
						g_opt_sound = 0;
						while (leader->le != leader->le_max && other->le >= 7) {
							other = get_hero_max_le(g_group);
							sub_hero_le(other, 1);
							add_hero_le(leader, 1);
						}
						g_opt_sound = save;
						screen_portraits(g_screen);
					}
					hours -= per;
				} else {
					do {
						timewarp_hours(g_date, 1L);
						cnt += 1;
						if (g_refresh_portraits)
							screen_portraits(g_screen);
					} while (cnt < per >> 1);
					hours -= cnt;
					intr = 1;
				}
				g_resting = 0;
				if (!intr) {
					if (w < 2)
						w++;
					else if (hours > 0) {
						if (p->mode == 0) {
							hours = 0;
							if (g_daytime < 0xa8c0L)
								timewarp_until(g_date, 0xa8c0L);
						} else {
							timewarp_hours(g_date, (signed long)(signed short)hours);
							hours = 0;
						}
					}
				}
			} while (hours > 0 && !intr);
			if (intr) {
				if (p->mode == 1) {
					h1 = get_random_hero2(g_group);
					if (!check_hero(h1))
						loose_random_item(h1, 100, p->txt[34], p->txt[122]);
					else
						loose_random_item(h1, 100, p->txt[33], p->txt[121]);
				} else {
					g_camp_ambush_watch = 0;
					ret = 1;
					g_fight_flag = 1;
					camp_wake_up(p, w);
					if (p->mode == 0)
						res = camp_encounter(p, g_camp_weather, get_hero_by_file_name(g_group, g_camp_watch_names[w]));
					else if (camp_night_encounter()) {
						g_camp_fight_pending = 1;
						return -1;
					}
					if (res == 2)
						g_travel_state = 1;
					if (g_travel_state != 1 && g_game_state == 0) {
						screen_redraw(g_screen);
						anim_load(g_screen_pic, p->pic, 0);
						TEXTBAR(p->txt[0]);
						snd_play(g_snd_music, 0x0b);
						g_refresh = 0;
						if (res == 1) {
							if (group_count_check1(g_group) > 1) {
								dlg_message(g_dialog, p->txt[114]);
								dlg_message(g_dialog, p->txt[115]);
							} else {
								h1 = get_first_active_hero(g_group);
								sprintf(g_text_output_buf, p->txt[116], h1->name, grammar_pronoun(h1->sex, 0, 0),
									grammar_pronoun(h1->sex, 0, 0), grammar_pronoun(h1->sex, 0, 0));
								dlg_message(g_dialog, g_text_output_buf);
							}
							dlg_message(g_dialog, p->txt[117]);
							for (k = 0; k < 0x46; k++) {
								if (k < 0x32)
									item = 0x2d;
								if (k < 0x3c && k >= 0x32)
									item = random_schweif(5) + 0x3b;
								if (k < 0x46 && k >= 0x3c)
									item = random_schweif(10) + 0x79;
								res = group_find_item(g_group, item);
								if (res == -1)
									h1 = 0;
								else
									h1 = g_group->heroes[res];
								if (h1)
									drop_item(h1, get_item_pos(h1, item), 1);
#if FIX_0191
								/* Fix #191: fehlende Gegenstaende nicht ueberspringen (k laeuft normal weiter) */
								else
									;
#else
								else
									k += (k + 13) / 10;
#endif
							}
							group_sub_money(g_group, g_group->money / 100);
							hours -= 1;
							timewarp_hours(g_date, 1L);
						}
					}
				}
			}
			if (hours > 0) {
				g_resting = 1;
				if (p->mode == 0)
					timewarp_until(g_date, 0xa8c0L);
				else
					timewarp_hours(g_date, (signed long)(signed short)hours);
				g_resting = 0;
			}
			p->site = 1;
		}
		if (g_travel_state <= 0 && g_travel_state > -7) {
			if (!intr) {
				if (asked >= 0)
					camp_regenerate(p, asked);
			} else if (p->mode == 0)
				camp_regenerate(p, 3);
			intr = 0;
			if (cond3905()) {
				h1 = get_first_active_hero(g_group);
				SET_ASLEEP(h1, 1);
				SET_ASLEEP(h1, 0);
				Sound_wait(g_snd_fx);
			}
		}
		save = g_opt_sound;
		g_opt_sound = 0;
		h1 = get_first_hero(g_group);
		do {
			SET_ASLEEP(h1, 0);
			h1 = get_next_hero(g_group, h1);
		} while (get_first_hero(g_group) != h1);
		g_opt_sound = save;
		if (p->penalty == 0x63) {
			dlg_message(g_dialog, p->txt[83]);
			GROUP_EFFECT(0x5eec0L, 6, -2, 0);
			p->penalty = 0;
		}
		if (p->penalty < 0) {
			money = g_group->money;
			switch (p->penalty) {
			case -1:
				if (money > 10000)
					money = 5000;
				else
					money = money / 2;
				break;
			case -2:
				if (money > 9000)
					money = 3000;
				else
					money = money / 3;
				break;
			case -3:
				if (money > 8000)
					money = 2000;
				else
					money = money / 4;
				break;
			}
			p->penalty = 0;
			if (money > 200) {
				sprintf(g_text_output_buf, p->txt[98], money / 100);
				dlg_message(g_dialog, g_text_output_buf);
				group_sub_money(g_group, money);
			}
		}
		return ret + 1;
	}
	return 0;
}

void camp_regenerate(Camp far *p, register signed short n)
{
	signed short v;
	struct hero far *h;
	signed short b;

	b = 1;
	h = get_first_hero(g_group);
	snd_obj_stop(g_snd_fx);
	poll_input();
	if (group_find_item(g_group, 0x57) >= 0)
		b++;
	if (group_find_item(g_group, 0x58) >= 0 || group_find_item(g_group, 0x127) >= 0)
		b++;
	if (group_find_item(g_group, 0x55) < 0 && group_find_item(g_group, ITEM_TORCH_ON) < 0 &&
	    group_find_item(g_group, ITEM_LANTERN_ON) < 0)
		b -= 2;
	do {
		if (p->mode == 0) {
			if (g_rest_mod > 0x5a) {
				v = g_rest_mod - 0x63;
				if (h->le * 2 <= h->le_max)
					v -= random_schweif(5);
			} else
				v = g_rest_mod;
			if (n > 0)
				v -= n;
			if (get_item_pos(h, 0x126) >= 0 || get_item_pos(h, 0x48) >= 0)
				v++;
			if ((date_season(g_date) == 0 && get_item_pos(h, 0x126) < 0) ||
			    (g_camp_weather == 4 && get_item_pos(h, 0x48) < 0) ||
			    (g_camp_weather == 5 && get_item_pos(h, 0x48) < 0))
				v -= 2;
			if (date_season(g_date) == 0 || g_camp_weather == 4) {
				if (get_item_pos(h, 0x126) < 0) {
					if (get_item_pos(h, 0x48) < 0)
						hero_disease_test(h, 6, 5);
					else
						hero_disease_test(h, 6, 2);
				}
			} else {
				if ((date_season(g_date) != 2 || g_camp_weather == 5) && get_item_pos(h, 0x126) < 0 &&
				    get_item_pos(h, 0x48) < 0)
					hero_disease_test(h, 6, 2);
			}
		} else {
			if (n > 10)
				n = 10;
			if (date_season(g_date) == 0 && get_item_pos(h, 0x126) < 0)
				v = (n - 10) / 2 + 3;
			else
				v = (n - 8) / 2 + 3;
		}
		if (h->hunger >= 100 || h->thirst >= 100)
			v -= 5;
		if (h->watches != 0) {
			v -= h->watches * 2;
			h->watches = 0;
		}
		if ((signed char)h->rest_herbs) {
			v -= 2;
			h->rest_herbs = 0;
		}
		if ((signed char)h->rest_magic || (signed char)h->rest_food) {
			v--;
			h->rest_magic = 0;
			h->rest_food = 0;
		}
		h->set_rest_bonus(v);
		hero_sleep(h, b);
		h = get_next_hero(g_group, h);
	} while (get_first_hero(g_group) != h);
	if (p->mode == 0)
		g_rest_mod = 0;
	TEXTBAR(p->txt[0]);
}

void camp_wake_up(Camp far *p, signed short w)
{
	struct hero far *h;
	struct hero far *last;
	signed short fail;
	signed short first;
	struct hero far *hw;
	struct hero far *hw2;

#if FIX_0191
	/* Fix #191: last vorbelegen */
	h = get_first_hero(g_group);
	last = 0;
	fail = 0;
	first = 1;
#else
	h = get_first_hero(g_group);
	fail = 0;
	first = 1;
#endif
	do {
		if (g_camp_watch_names[w][0] && (p->job || p->watch_set)) {
			if (get_hero_by_file_name(g_group, g_camp_watch_names[w]) != h) {
				if (random_schweif(100) <= 70 || !check_hero(h)) {
					if (!(signed char)h->petrified)
						SET_ASLEEP(h, 1);
				} else {
					SET_ASLEEP(h, 0);
					fail = 1;
				}
			}
		} else
			SET_ASLEEP(h, 1);
		h = get_next_hero(g_group, h);
	} while (get_first_hero(g_group) != h);
	if (group_all_asleep(g_group) && fail) {
		h = get_random_asleep_hero(g_group);
		SET_ASLEEP(h, 0);
	}
	if (fail) {
		hw = get_hero_by_file_name(g_group, g_camp_watch_names[w]);
		sprintf(g_text_output_buf, p->txt[45], hw->name);
		h = get_first_hero(g_group);
		do {
			if (get_hero_by_file_name(g_group, g_camp_watch_names[w]) != h && !(signed char)h->asleep) {
				if (first)
					first = 0;
				else
					strcat(g_text_output_buf, p->txt[46]);
				strcat(g_text_output_buf, h->name);
				last = h;
			}
			h = get_next_hero(g_group, h);
		} while (get_first_hero(g_group) != h);
		dlg_message(g_dialog, g_text_output_buf);
#if FIX_0191
		/* Fix #191: Meldung nur mit einem gefundenen Helden */
		if (last != 0) {
			sprintf(g_text_output_buf, p->txt[47], last->name);
			dlg_message(g_dialog, g_text_output_buf);
		}
#else
		sprintf(g_text_output_buf, p->txt[47], last->name);
		dlg_message(g_dialog, g_text_output_buf);
#endif
		hw2 = get_hero_by_file_name(g_group, g_camp_watch_names[w]);
		sprintf(g_text_output_buf, p->txt[48], hw2->name);
		dlg_message(g_dialog, g_text_output_buf);
	}
}
