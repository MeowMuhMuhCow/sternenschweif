/* OVL217 - Overlay O217 (Segment 0x23b9 in SCHWEIF.EXE): Reiseteil - Tempo, Rastmenue, Umkehren (Travel wie O213;
 * Aufrufer O122, O123, O165, O211, O214, O215). travel_slow5_off/_on, travel_slow4_off/_on(o, v): Tempo v * 10 / 7
 * bzw. v * 7 / 10 mit Merkern g_slow5_on/g_slow4_on (travel_slow4_on nur mit Seil 0x79 (group_find_item) oder einem
 * Helden mit Stabzauber 3 (get_hero_staffspell), sonst Ortstext g_tev_txt[12]; Ereignistexte 0x13e bei Bedarf
 * geladen/freigegeben). select_hero_bar(o): Held auswaehlen - Rahmen 34x34 je Held am unteren Rand (Mausbereiche
 * g_hero_bar_areas, 7 x 10 Bytes, lokale Kopie fuer get_mouse_action), Pfeile/Maus/Enter, Enter -> Heldenbogen
 * Inv_ctor/hero_sheet_loop/Inv_dtor, Esc/0x5100 Ende; o->speed gesichert und zurueck, o->status[TST_SPEED] =
 * travel_level. travel_rest_menu(o): Menue unterwegs dlg_choose (g_ttxt[9..12], [18..21], [51]) mit 5 bzw. 6
 * Punkten: 1 travel_stop, 2 umkehren (action = TRAVEL_ACT_TURN), 3 Eilmarsch an/aus (travel_speed_hurry/_normal),
 * 4 Route ab dem Ziel neu, wenn die Route weitergeht (action = TRAVEL_ACT_REROUTE), 5 Tempo drosseln je Gelaende
 * (5: travel_slow5_on/_off, 4: travel_slow4_on/_off). travel_speed_normal(o, v): hurry = 0, v * 10 / 13.
 * travel_speed_hurry(o, v, frage): Eilmarsch - hilflose Helden (tot/bewusstlos einfach, versteinert doppelt; muessen
 * getragen werden) und schwache (LE * 2 <= LEmax und LE < 15) sammeln; zu viele Hilflose (Gruppengroesse <= 3 *
 * Zahl) -> Meldung g_ttxt[40..43] und g_rest_mod = 99, kein Eilmarsch; sonst Frage g_ttxt[44..47] (dlg_yes_no);
 * Eilmarsch -> hurry = 1, v * 13 / 10, g_rest_mod = -3 (wenn nicht 99). travel_reverse(o, &a, &b, d): unterwegs
 * umkehren - Teilstrecken neu zeichnen (travelmap_route_part), Strecke/Start/Ziel tauschen, Route o->route (Ende
 * ROUTE_END) umdrehen, wait_for_vsync, tmap_draw, action = TRAVEL_ACT_TURNED, Gruppendaten (route_pos ..
 * turn_to) zurueckschreiben. travel_stop(o, art): Reise anhalten/rasten - Gruppendaten setzen, bei art 2 g_rest_mod
 * -= 2, loctype = LOC_CAMP_WILD (alter Wert nach old_loctype, wenn Bit 7 frei), g_travel_state = 3, Daten zurueck in
 * o, travel_redraw.
 *
 * BCC -mlarge -3 -Ot -Og -Oi -c -Y OVL217.CPP   -> 5033/5033 Bytes GLEICH (18.10.2026), cobjfix laeuft.
 *   (Namensfassung mit schweif.h: 19.10.2026 GLEICH, LINKGLEICH)
 *   Ohne -Oi 22 Bytes zu lang (strcat-Intrinsic), ohne -Og 151 Bytes zu kurz (Epilogkopien in travel_rest_menu u. a.).
 * - Nur zwei Abweichungen im ersten Entwurf: "w = (width - 1) * g_group->nr_heroes + 3" (nr_heroes zuerst geladen und
 *   gepusht; umgekehrt dx = width - 1) und "set_route_len((unsigned long)o->route_len * 1000)" (movzx eax,byte /
 *   imul eax,eax,3e8h; mit "o->route_len * 1000L" mov al / mov ah,0 / movsx eax,ax).
 * - Gruppen-Setter als inline-Elementfunktionen mit Parameterkopie (set_route_pos long [bp-10h], set_route_from/_to
 *   Byte, set_turns mit signed long -> cbw / movsx eax,ax / Store nur al); get_turns() wie OVL213.
 * - Gruppenmodus: inline set_loctype(v) VERSCHACHTELT als set_loctype(set_loctype(LOC_CAMP_WILD)) - erklaert den
 *   doppelten Test, die Kopie [bp-16h] (Ergebnis des inneren Aufrufs mit cbw) und das lose cbw am Ende; [bp-17h] ist
 *   der -Og-Zwischenwert des Bytes loctype.
 * - travel_rest_menu: switch 1..5 mit break -> Sprungtabelle, jedes break wird mit -Og zur Epilogkopie; leere Schleife
 *   "for (..; o->route[i] != o->to && o->route[i] != 0xff; i++) ;" und "if (A || B) ; else o->action = 2;".
 * - travel_slow5_off .. travel_slow4_off/travel_speed_normal: register-Parameter v ohne Aufrufe -> CX, "v = v * 10 / 7;
 *   return v;" (imul / div).
 * - Ketten "k = ret = o->planned = 0", "cnt = nweak = nhelpless = hurry = 0", "g_key_shift = g_key = g_action = n = 0".
 * - Lokale Kopie der Mausfelder "areas = g_hero_bar_areas" (struct mit Array, rep movsw 28h) liegt hinter den
 *   -Og-Werten.
 * Originalfehler/Auffaelliges: travel_slow4_on gibt die Ereignistexte frei, setzt aber nur g_tev_buf auf 0, g_tev_txt
 *   bleibt als haengender Zeiger stehen. Das Tempo (o->speed, unsigned long) wird an die Tempo-Funktionen nur als
 *   unsigned short uebergeben und dort 16-bittig multipliziert (v * 13 bzw. v * 10 laeuft ab 5042 bzw. 6554 ueber).
 *   travel_speed_hurry: bei zu vielen Hilflosen wird nur gemeldet, das Tempo bleibt unveraendert.
 */
#include <stdio.h>
#include <string.h>
#include "schweif.h"

/* Phase B: eigene Daten des Moduls (ds:7204..7254, dsana) - initialisierte Daten in Adressreihenfolge */
struct hero_bar_areas g_hero_bar_areas = {
	{ { 0, 0, 0, 0, 1 },
	{ 0, 0, 0, 0, 2 },
	{ 0, 0, 0, 0, 3 },
	{ 0, 0, 0, 0, 4 },
	{ 0, 0, 0, 0, 5 },
	{ 0, 0, 0, 0, 6 },
	{ 0, 0, 0, 0, 7 },
	{ -1, -1, -1, -1, -1 } }
};

unsigned short travel_slow5_off(Travel far *o, register unsigned short v)
{
	g_slow5_on = 0;
#if FIX_0121
	/* Fix #121: 32-bittig rechnen (kein Ueberlauf bei hohem Tempo) */
	v = (unsigned long)v * 10 / 7;
#else
	v = v * 10 / 7;
#endif
	return v;
}

unsigned short travel_slow5_on(Travel far *o, register unsigned short v)
{
	g_slow5_on = 1;
#if FIX_0121
	/* Fix #121: 32-bittig rechnen (kein Ueberlauf bei hohem Tempo) */
	v = (unsigned long)v * 7 / 10;
#else
	v = v * 7 / 10;
#endif
	return v;
}

unsigned short travel_slow4_off(Travel far *o, register unsigned short v)
{
	g_slow4_on = 0;
#if FIX_0121
	/* Fix #121: 32-bittig rechnen (kein Ueberlauf bei hohem Tempo) */
	v = (unsigned long)v * 10 / 7;
#else
	v = v * 10 / 7;
#endif
	return v;
}

unsigned short travel_slow4_on(Travel far *o, register unsigned short v)
{
	signed char r;
	signed short loaded = 0;

	if (g_tev_buf == 0) {
		loaded = 1;
		txt_load_list(g_font, 0x13e, 0x8c, &g_tev_buf, &g_tev_txt);
	}
	r = group_find_item(g_group, 0x79);
	if (r >= 0 || get_hero_staffspell(g_group, 3) != 0) {
		g_slow4_on = 1;
#if FIX_0121
		/* Fix #121: 32-bittig rechnen (kein Ueberlauf bei hohem Tempo) */
		v = (unsigned long)v * 7 / 10;
#else
		v = v * 7 / 10;
#endif
	} else
		dlg_message(g_dialog, g_tev_txt[12]);
	if (loaded) {
		g_tev_buf = mem_free(g_tev_buf);
#if FIX_0121
		/* Fix #121: Zeiger nach dem Freigeben nullen */
		g_tev_txt = (char far * far *)mem_free(g_tev_txt);
#else
		mem_free(g_tev_txt);
#endif
	}
	return v;
}

void select_hero_bar(Travel far *o)
{
	signed short sel;
	signed short cur;
	signed short mx;
	signed short my;
	signed short w;
	struct inv_sheet far *sheet;
	signed short width = 0x22;
	signed short top = 0xa4;
	signed short bot = 0xc8;
	signed char done = 0;
	signed char redraw = 0;
	signed short n = 0;
	unsigned long save_speed = o->speed;
	struct hero_bar_areas areas;

	sel = cur = 0;
	mouse_clear_left();
	o->speed = 0;
	travel_anim_off(o, 1);
	g_refresh = 0;
	group_draw_portraits(g_group);
	w = (width - 1) * g_group->nr_heroes + 3;
	g_hero_bar_areas.a[0].x0 = 0xf0 - w + 1;
	g_hero_bar_areas.a[0].y0 = top;
	g_hero_bar_areas.a[0].x1 = 0xf0 - w + width;
	g_hero_bar_areas.a[0].y1 = bot;
	g_hero_bar_areas.a[1].x0 = 0xf0 - w + width;
	g_hero_bar_areas.a[1].y0 = top;
	g_hero_bar_areas.a[1].x1 = 0xf0 - w + width * 2 - 1;
	g_hero_bar_areas.a[1].y1 = bot;
	g_hero_bar_areas.a[2].x0 = 0xf0 - w + width * 2 - 1;
	g_hero_bar_areas.a[2].y0 = top;
	g_hero_bar_areas.a[2].x1 = 0xf0 - w + width * 3 - 2;
	g_hero_bar_areas.a[2].y1 = bot;
	g_hero_bar_areas.a[3].x0 = 0xf0 - w + width * 3 - 2;
	g_hero_bar_areas.a[3].y0 = top;
	g_hero_bar_areas.a[3].x1 = 0xf0 - w + width * 4 - 3;
	g_hero_bar_areas.a[3].y1 = bot;
	g_hero_bar_areas.a[4].x0 = 0xf0 - w + width * 4 - 3;
	g_hero_bar_areas.a[4].y0 = top;
	g_hero_bar_areas.a[4].x1 = 0xf0 - w + width * 5 - 4;
	g_hero_bar_areas.a[4].y1 = bot;
	g_hero_bar_areas.a[5].x0 = 0xf0 - w + width * 5 - 4;
	g_hero_bar_areas.a[5].y0 = top;
	g_hero_bar_areas.a[5].x1 = 0xf0 - w + width * 6 - 5;
	g_hero_bar_areas.a[5].y1 = bot;
	g_hero_bar_areas.a[6].x0 = 0xf0 - w + width * 6 - 5;
	g_hero_bar_areas.a[6].y0 = top;
	g_hero_bar_areas.a[6].x1 = 0xf0 - w + width * 7 - 6;
	g_hero_bar_areas.a[6].y1 = bot;
	areas = g_hero_bar_areas;
	mouse_hide(0);
	do_border(g_vga_buf, 0xf0 - w + 1, 0xa4, width, width, 0xf9);
	mouse_show(0);
	g_key_shift = g_key = g_action = n = 0;

	while (done == 0) {
		mx = g_mouse_posx;
		my = g_mouse_posy;
		travel_redraw(o);
		if (g_mouse_ldclick != 0) {
			g_key = 0x1c0d;
			g_mouse_ldclick = 0;
		}
		if (g_mouse_lclick != 0 && g_mouse_event == 0) {
			g_key = 0x1c0d;
			mouse_clear_left();
		}
		if (g_mouse_rclick != 0 && g_mouse_event == 0) {
			g_key = 0x5100;
			mouse_clear_right();
		}
		if (redraw == 1) {
			redraw = 0;
			mouse_hide(0);
			group_draw_portraits(g_group);
			do_border(g_vga_buf, 0xf0 - w + sel * (width - 1) + 1, 0xa4, width, width, 0xf9);
			mouse_show(0);
		}
		if (g_key == 0x1c0d) {
			o->reenter = 1;
			travel_leave(o);
			sheet = (struct inv_sheet far *)Inv_ctor(0, 0);
			n = 0;
			hero_sheet_loop(sheet, g_group->heroes[cur]);
			mouse_hide(0);
			Inv_dtor(sheet, 3);
			travel_enter(o);
			o->reenter = 0;
			mouse_show(0);
			redraw = 1;
			done = 1;
		}
		if (g_key == 0x5100 || g_key == 0x11b)
			done = 2;
		if (g_mouse_posx != mx || g_mouse_posy != my) {
			n = get_mouse_action(&areas);
			if (n != 0 && g_group->nr_heroes >= n)
				sel = n - 1;
		}
		if (g_key == 0x4d00) {
			sel = cur + 1;
			if (g_group->nr_heroes <= sel)
				sel = 0;
		}
		if (g_key == 0x4b00) {
			sel = cur - 1;
			if (sel < 0)
				sel = g_group->nr_heroes - 1;
		}
		if (cur != sel) {
			mouse_hide(0);
			do_border(g_vga_buf, 0xf0 - w + cur * (width - 1) + 1, 0xa4, width, width, 0);
			cur = sel;
			g_key = n = 0;
			do_border(g_vga_buf, 0xf0 - w + sel * (width - 1) + 1, 0xa4, width, width, 0xf9);
			mouse_show(0);
		}
	}
	o->speed = save_speed;
	o->status[3] = travel_level(o);
	travel_anim_on(o, 1);
}

void travel_rest_menu(Travel far *o)
{
	signed char ans;
	signed char i;
	char far *txt_slow;
	char far *txt_hurry;
	signed char n = 5;

	if (!g_on_boat) {
		if (o->terrain == 5) {
			n = 6;
			if (g_slow5_on == 0)
				txt_slow = g_ttxt[18];
			else
				txt_slow = g_ttxt[19];
		}
		if (o->terrain == 4) {
			n = 6;
			if (g_slow4_on == 0)
				txt_slow = g_ttxt[20];
			else
				txt_slow = g_ttxt[21];
		}
		if (o->hurry != 1)
			txt_hurry = g_ttxt[7];
		else
			txt_hurry = g_ttxt[8];
		if (n == 5)
			txt_slow = g_ttxt[51];
		ans = dlg_choose(g_dialog, g_ttxt[9], n, g_ttxt[11], g_ttxt[10], txt_hurry, g_ttxt[12], txt_slow, g_ttxt[51]);
		if (n == 5 && ans == 5)
			ans = 6;
		switch (ans) {
		case 1:
			travel_stop(o, 0);
			break;
		case 2:
			o->action = TRAVEL_ACT_TURN;
			break;
		case 3:
			if (o->hurry != 1)
				o->speed = travel_speed_hurry(o, o->speed, 1);
			else
				o->speed = travel_speed_normal(o, o->speed);
			break;
		case 4:
			for (i = 0; o->route[i] != o->to && o->route[i] != ROUTE_END; i++)
				;
			if (o->route[i] == ROUTE_END || o->route[i + 1] == ROUTE_END)
				;
			else
				o->action = TRAVEL_ACT_REROUTE;
			break;
		case 5:
			if (o->terrain == 5) {
				if (g_slow5_on == 0)
					o->speed = travel_slow5_on(o, o->speed);
				else
					o->speed = travel_slow5_off(o, o->speed);
			}
			if (o->terrain == 4) {
				if (g_slow4_on == 0)
					o->speed = travel_slow4_on(o, o->speed);
				else
					o->speed = travel_slow4_off(o, o->speed);
			}
			break;
		}
	}
}

unsigned short travel_speed_normal(Travel far *o, register unsigned short v)
{
	o->hurry = 0;
#if FIX_0121
	/* Fix #121: 32-bittig rechnen (kein Ueberlauf bei hohem Tempo) */
	v = (unsigned long)v * 10 / 13;
#else
	v = v * 10 / 13;
#endif
	return v;
}

unsigned short travel_speed_hurry(Travel far *o, unsigned short v, signed char ask)
{
	signed char i;
	signed char nweak;
	signed char nhelpless;
	signed char hurry;
	signed char ans;
	signed short cnt;
	struct hero far *h;
	struct hero far *weak[7];
	struct hero far *helpless[7];

	if (ask == 0)
		hurry = 1;
	else {
		for (i = 0; i < 7; i++) {
			weak[i] = 0;
			helpless[i] = 0;
		}
		cnt = nweak = nhelpless = hurry = 0;
		for (i = 0; i < 7; i++) {
			h = g_group->heroes[i];
			if (h != 0) {
				if ((unsigned char)h->dead || (unsigned char)h->unconscious) {
					cnt++;
					helpless[nhelpless++] = h;
				} else if ((unsigned char)h->petrified) {
					cnt += 2;
					helpless[nhelpless++] = h;
				} else if (h->le * 2 <= h->le_max && h->le < 15) {
					weak[nweak++] = h;
				}
			}
		}
		if (g_group->nr_heroes <= cnt * 3) {
			if (nhelpless == 1)
				sprintf(g_text_output_buf, g_ttxt[40], helpless[0]->name);
			else {
				i = 0;
				sprintf(g_text_output_buf, g_ttxt[41], helpless[i]->name);
				for (i = 1; i < nhelpless - 1; i++) {
					sprintf(g_text_buf2, g_ttxt[42], helpless[i]->name);
					strcat(g_text_output_buf, g_text_buf2);
				}
				sprintf(g_text_buf2, g_ttxt[43], helpless[i]->name);
				strcat(g_text_output_buf, g_text_buf2);
			}
			dlg_message(g_dialog, g_text_output_buf);
			g_rest_mod = 99;
		} else if (nweak != 0) {
			if (nweak == 1)
				sprintf(g_text_output_buf, g_ttxt[44], weak[0]->name);
			else {
				i = 0;
				sprintf(g_text_output_buf, g_ttxt[45], weak[i]->name);
				for (i = 1; i < nweak - 1; i++) {
					sprintf(g_text_buf2, g_ttxt[42], weak[i]->name);
					strcat(g_text_output_buf, g_text_buf2);
				}
				sprintf(g_text_buf2, g_ttxt[46], weak[nweak - 1]->name);
				strcat(g_text_output_buf, g_text_buf2);
			}
			strcat(g_text_output_buf, g_ttxt[47]);
			ans = dlg_yes_no(g_dialog, g_text_output_buf);
			if (ans == 1) {
				hurry = 1;
				g_rest_mod = 99;
			}
		} else
			hurry = 1;
	}
	if (hurry == 1) {
		o->hurry = 1;
#if FIX_0121
		/* Fix #121: 32-bittig rechnen (kein Ueberlauf bei hohem Tempo) */
		v = (unsigned long)v * 13 / 10;
#else
		v = v * 13 / 10;
#endif
		if (g_rest_mod != 99)
			g_rest_mod = -3;
	}
	return v;
}

unsigned short travel_reverse(Travel far *o, unsigned char far *pfrom, unsigned char far *pto, register signed short speed)
{
	unsigned char tmp;
	unsigned char tfrom;
	unsigned char tto;
	unsigned char k;
	unsigned short len;
	signed short way;
	signed short ret;
	signed char i;
	unsigned char buf[40];

	k = ret = o->planned = 0;
	way = route_find(o, *pfrom, *pto);
	len = g_routes[way].len * 1000;
	o->route_pos = g_group->route_pos;
	o->from = g_group->route_from;
	o->to = g_group->route_to;
	o->route_len = g_group->get_route_len() / 1000;
	o->turn_pos = g_group->turn_pos;
	o->turns = g_group->get_turns();
	tfrom = g_group->turn_from;
	tto = g_group->turn_to;
	if (o->turns >= 1) {
		if (o->to == tfrom && o->from == tto)
			ret = travelmap_route_part(o, *pfrom, *pto, speed, o->route_pos, 0xff, len - o->turn_pos);
		else if (o->to == tto && o->from == tfrom)
			ret = travelmap_route_part(o, *pfrom, *pto, speed, o->route_pos, 0xff, len - o->turn_pos);
		else {
			travelmap_route_part(o, tfrom, tto, speed, o->turn_pos, 0xff, 0);
			ret = travelmap_route_part(o, *pfrom, *pto, speed, o->route_pos, 0xff, 0);
		}
	} else
		ret = travelmap_route_part(o, *pfrom, *pto, speed, o->route_pos, 0xff, 0);
	o->turn_pos = o->route_pos;
	o->route_pos = len - o->route_pos;
	for (i = 0; i < 40; i++)
		buf[i] = 0;
	tmp = o->to;
	o->to = o->from;
	o->from = tmp;
	*pfrom = o->from;
	*pto = o->to;
	for (k = 0; o->route[k] != ROUTE_END; k++)
		;
	for (i = 0; k > 0; i++) {
		k--;
		buf[i] = o->route[k];
	}
	buf[i] = ROUTE_END;
	for (i = 0; i < 40; i++)
		o->route[i] = buf[i];
	wait_for_vsync();
	tmap_draw(g_tmap);
	o->action = TRAVEL_ACT_TURNED;
	g_group->set_route_pos(o->route_pos);
	g_group->set_route_from(o->from);
	g_group->set_route_to(o->to);
	g_group->set_route_len((unsigned long)o->route_len * 1000);
	g_group->set_turn_pos(o->turn_pos);
	g_group->set_turns(o->turns);
	g_group->set_turn_from(o->to);
	g_group->set_turn_to(o->from);
	return ret;
}

void travel_stop(Travel far *o, signed char mode)
{
	unsigned long save_speed = o->speed;

	o->speed = 0;
	g_group->set_route_pos(o->route_pos);
	g_group->set_route_from(o->from);
	g_group->set_route_to(o->to);
	g_group->set_route_len((unsigned long)o->route_len * 1000);
	g_group->set_turn_pos(o->turn_pos);
	g_group->set_turns(o->turns);
	if (mode == 2)
		g_rest_mod -= 2;
	if (o->mode != 2) {
		g_group->set_loctype(g_group->set_loctype(LOC_CAMP_WILD));
		g_travel_state = 3;
	}
	o->route_pos = g_group->route_pos;
	o->from = g_group->route_from;
	o->to = g_group->route_to;
	o->route_len = g_group->get_route_len() / 1000;
	o->turn_pos = g_group->turn_pos;
	o->turns = g_group->get_turns();
	travel_redraw(o);
	o->speed = save_speed;
}
