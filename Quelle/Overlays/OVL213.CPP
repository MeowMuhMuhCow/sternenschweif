/* OVL213 - Overlay O213 (Segment 0x23a7 in SCHWEIF.EXE): Reiseteil - Reiseanimation, Wetterfolgen, Kartenposition
 * (Reiseobjekt Travel, s_class.h). travel_nop_o213: leer. travel_anim_off_leave/travel_anim_off: anim_on = 0,
 * travel_anim_on: anim_on = 1. travel_level: Tempostufe 0..4 aus speed (Grenzen 50/200/330/450). travel_anim:
 * Reiseanimation 72x33 bei (0xf4, 0x89): Tempostufe status[TST_SPEED] (1..4 -> Bild 0xa5..0xa8 und Takt 2/2/1/1,
 * sonst 0xa0/5), zwei Hintergrundebenen aus Bild 0xa3 (Teilbilder 1/2, je doppelt nebeneinander bei bg_x1 bzw. bg_x2,
 * Schrittweite je Stufe, modulo 0x72) in einen Puffer 0x2940 mit Clip-Rechteck g_clip_rect (gesichert/zurueck), Figur
 * Teilbild frame (0..7) darueber (Bmp::over), Maus verstecken bei Ueberdeckung, Zaehler anim_tick/anim_rate.
 * travel_weather_effects: Wetterfolgen fuer alle Helden (Wetter Wx_get(g_weather), Jahreszeit date_season(g_date)):
 * Kaelte hero_disease_test(h, Dumpfschaedel, 6/3/1) nach Kopf/Koerper (inv[0]/inv[9]), Naesse (Frostschaeden 3/1)
 * ohne Mantel 0x60 an Platz 9 bzw. ohne Schuhe inv[14], bei Hitze mit Mantel Meldung (Namen hero_name_gen mit
 * Trennern txt[68]/[69], txt[50], txt[71]/[72], txt[73] aus g_ttxt) und Schaden W2 - 1 (sub_hero_le).
 * travel_ui_load/travel_ui_unload: umrahmen die Orts-/Reiseereignisse (place_event O210, tevent_run O211) -
 * Ereignistexte g_tev_* freigeben, Reisetexte (0xa4/0xad/0x1f1 nach g_ttxt/g_ttxt2/g_ttxt3) und Kompassrose
 * (rose_alloc/rose_free) laden bzw. freigeben, Ereignistexte (0x13e und evtxt_file/evtxt_n) neu laden, wenn vorher
 * vorhanden. travel_set_map_pos(o, noroute): Position auf der Karte - Weg route_first/route_find, Wegpunkte
 * route_load_points, Fortschritt k aus der zurueckgelegten Strecke group.route_pos (Laenge = route.len * 1000),
 * Marke tmap_set_pos, alle Wege travelmap_route/travelmap_draw_path, Route in Farbe 0xfd, Teilstrecken per
 * travelmap_route_part je nach Richtung und Umkehrungen turns.
 *
 * BCC -mlarge -3 -Ot -Og -Oi -c -Y OVL213.CPP   -> 4213/4213 Bytes GLEICH (18.10.2026), cobjfix laeuft
 *   (Namensfassung mit schweif.h, Reiseobjekt Travel: 19.10.2026 GLEICH, LINKGLEICH)
 * - "o->turns = g_group->get_turns()" mit inline "signed long get_turns() { return (signed short)turns; }" -> mov al /
 *   cbw / movsx eax,ax / Store nur al; direkt mit Casts optimiert BCC die Kette weg (445 Abweichungen). Der Test
 *   "g_group->get_turns() == 0" (cbw / movsx / cmp eax,0) geht auch als "!(signed long)(signed short)turns", aber
 *   nicht mit "== 0".
 * - group.route_from/route_to sind signed char, hier unsigned verglichen; group.route_pos unsigned: Casts.
 * - travel_anim_on/travel_anim_off haben im Prototyp einen zweiten (unbenutzten) Parameter wie bei den Aufrufern.
 * - [bp-16h] ist der -Og-Zwischenwert group.route_from (kein eigenes Local); weitere -Og-Zwischenwerte o->to
 *   ([bp-17h]), o->nr_points * 2, o + i und o->route[i].
 * - "step * k / 2" (step long, k unsigned) -> movzx / push eax / mov eax,step / pop edx / imul; "k * step"
 *   ergibt imul eax,[step]. "step = len / o->nr_points * 2" -> div word / shl / movzx.
 * - strcpy(g_text_output_buf, g_ttxt[50]) (Quelle global) erzeugt die zweite Intrinsic-Form (Quelle zweimal
 *   geladen, Ziel per push dx/ax) - von selbst.
 * - Zweites switch: "case 4: o->bg_x2 += 3; default: break;" (Sprung des default an eigener Stelle); Tabellen
 *   der beiden switch hinter travel_anim (die des zweiten zuerst).
 * - "struct rect r; .. r = g_clip_rect;" (wortweise, hinter den Skalaren), Bmp-Bloecke je eigener Block bei
 *   [bp-22h], das bleibende Bmp pic2 auf Funktionsebene; "if (g_travel_state != 2) { .. }" um den ganzen Rumpf.
 * Originalfehler: travel_set_map_pos benutzt len (und step) uninitialisiert, wenn Start und Ziel der Gruppe gleich
 *   sind und danach from != to ("len - o->turn_pos"); travel_anim_off_leave und travel_anim_off sind identisch.
 */
#include <string.h>
#include "schweif.h"

#define DLG		((Dlg far *)g_dialog)
#define PIC_TRAVEL_BG	0xa3		/* Hintergrundebenen der Reiseanimation (Teilbilder 0..2) */
#define PIC_WALK	0xa0		/* Figur stehend/gehend, Stufe 1..4: 0xa5..0xa8 */
#define INV_HEAD	0		/* Inventarplaetze: Kopf */
#define INV_BODY	9		/* Koerper */
#define INV_FEET	14		/* Fuesse */
#define ITEM_COAT	0x60		/* warmer Mantel (vermutet: schuetzt vor Kaelte/Naesse, bei Hitze Schaden) */

void travel_nop_o213(Travel far *o)
{
}

void travel_anim_off_leave(Travel far *o)
{
	o->anim_on = 0;
}

signed char travel_level(Travel far *o)
{
	signed char r;

	if (o->speed <= 50)
		r = 0;
	if (o->speed > 450)
		r = 4;
	if (o->speed > 50 && o->speed <= 200)
		r = 1;
	if (o->speed > 200 && o->speed <= 330)
		r = 2;
	if (o->speed > 330 && o->speed <= 450)
		r = 3;
	return r;
}

void travel_anim(Travel far *o)
{
	void far *buf;
	struct rect r;
	signed short dummy;
	register signed short pic;
	register signed short hidden = 0;

	switch (o->status[TST_SPEED]) {
	case 1:
		o->anim_rate = 2;
		pic = 0xa5;
		break;
	case 2:
		o->anim_rate = 2;
		pic = 0xa6;
		break;
	case 3:
		o->anim_rate = 1;
		pic = 0xa7;
		break;
	case 4:
		o->anim_rate = 1;
		pic = 0xa8;
		break;
	default:
		o->anim_rate = 5;
		pic = PIC_WALK;
		break;
	}
	if (o->status[TST_SPEED] != 0) {
		switch (o->status[TST_SPEED]) {
		case 1:
			o->bg_x2++;
			break;
		case 2:
			o->bg_x2++;
			break;
		case 3:
			o->bg_x2 += 2;
			break;
		case 4:
			o->bg_x2 += 3;
		default:
			break;
		}
		if (o->bg_x2 >= 0x72)
			o->bg_x2 %= 0x72;
	}
	r = g_clip_rect;
	g_clip_rect.y0 = 0;
	g_clip_rect.x0 = 0x72;
	g_clip_rect.y1 = 0x20;
	g_clip_rect.x1 = 0xb9;
	buf = mem_alloc(0x2940L);
	{
		Bmp b(0x48, 0x21);

		nvfile_load(g_nvf_heads, PIC_TRAVEL_BG, b.data(), 0, 0, 3, &dummy, &dummy, 0x140);
		b.put(buf, 0x72, 0, 0);
	}
	{
		Bmp b(0x72, 0x21);

		nvfile_load(g_nvf_heads, PIC_TRAVEL_BG, b.data(), 0, 1, 3, &dummy, &dummy, 0x140);
		b.put(buf, o->bg_x1, 0, 4);
		b.put(buf, o->bg_x1 + 0x72, 0, 4);
	}
	{
		Bmp b(0x72, 0x21);

		nvfile_load(g_nvf_heads, PIC_TRAVEL_BG, b.data(), 0, 2, 3, &dummy, &dummy, 0x140);
		b.put(buf, o->bg_x2, 0, 4);
		b.put(buf, o->bg_x2 + 0x72, 0, 4);
	}
	Bmp pic2(0x48, 0x21);
	pic2.get(buf, 0x72, 0);
	buf = mem_free(buf);
	g_clip_rect = r;
	{
		Bmp b(0x48, 0x21);

		nvfile_load(g_nvf_heads, pic, b.data(), 0, o->frame, 3, &dummy, &dummy, 0x140);
		pic2.over(&b, 0, 0, 4);
	}
	if (mouse_overlaps_rect(0xf4, 0x89, 0x13b, 0xa9)) {
		hidden = 1;
		mouse_hide(0);
	}
	pic2.put(g_vga_buf, 0xf4, 0x89);
	if (hidden)
		mouse_show(0);
	o->anim_tick++;
	if (o->anim_tick >= o->anim_rate) {
		o->anim_tick = 0;
		if (o->status[TST_SPEED] != 0) {
			o->bg_x1++;
			if (o->bg_x1 >= 0x72)
				o->bg_x1 = 0;
		}
		o->frame++;
		if (o->frame > 7)
			o->frame = 0;
	}
}

void travel_anim_on(Travel far *o, signed short)
{
	o->anim_on = 1;
}

void travel_anim_off(Travel far *o, signed short)
{
	o->anim_on = 0;
}

void travel_weather_effects(Travel far *o)
{
	struct hero far *h;
	signed char weather;
	signed char season;
	signed char n = 0;
	signed char i;
	signed char total;
	struct hero far *list[7];

	for (i = 0; i < 7; i++)
		list[i] = 0;
	weather = Wx_get(g_weather);
	season = date_season(g_date);
	h = get_first_hero(g_group);
	do {
		if (h != 0 && check_hero3(h)) {
			if (weather == 4) {
				if (h->inv[INV_HEAD].item_id == 0 && h->inv[INV_BODY].item_id == 0)
					hero_disease_test(h, ILLNESS_DUMPFSCHAEDEL, 6);
				else if (h->inv[INV_HEAD].item_id == 0 || h->inv[INV_BODY].item_id == 0)
					hero_disease_test(h, ILLNESS_DUMPFSCHAEDEL, 3);
				else
					hero_disease_test(h, ILLNESS_DUMPFSCHAEDEL, 1);
			}
			if (weather == 3) {
				if (h->inv[INV_HEAD].item_id == 0 && h->inv[INV_BODY].item_id == 0)
					hero_disease_test(h, ILLNESS_DUMPFSCHAEDEL, 3);
				else if (h->inv[INV_HEAD].item_id == 0 || h->inv[INV_BODY].item_id == 0)
					hero_disease_test(h, ILLNESS_DUMPFSCHAEDEL, 1);
			}
			if (weather != 0 && (season == 0 || (season != 2 && o->terrain == 4))) {
				if (get_item_pos(h, ITEM_COAT) != INV_BODY && h->inv[INV_FEET].item_id == 0)
					hero_disease_test(h, ILLNESS_FROSTSCHAEDEN, 3);
				else if (get_item_pos(h, ITEM_COAT) != INV_BODY || h->inv[INV_FEET].item_id == 0)
					hero_disease_test(h, ILLNESS_FROSTSCHAEDEN, 1);
			}
			if (weather != 0 && ((season == 2 && o->terrain == 4) || (season != 2 && o->terrain == 5))) {
				if (get_item_pos(h, ITEM_COAT) != INV_BODY && h->inv[INV_FEET].item_id == 0)
					hero_disease_test(h, ILLNESS_FROSTSCHAEDEN, 1);
			}
			if ((season == 2 && weather != 4 && weather != 3) || (season != 0 && weather == 0)) {
				if (get_item_pos(h, ITEM_COAT) == INV_BODY) {
					list[n] = h;
					n++;
				}
			}
		}
		h = get_next_hero(g_group, h);
	} while (get_first_hero(g_group) != h);
	if (n > 0) {
		total = n;
		i = 0;
		strcpy(g_text_output_buf, g_ttxt[50]);
		do {
			strcat(g_text_output_buf, hero_name_gen(list[i], 1));
			n--;
			i++;
			if (n > 1)
				strcat(g_text_output_buf, g_ttxt[68]);
			else if (n == 1)
				strcat(g_text_output_buf, g_ttxt[69]);
		} while (n > 0);
		strcat(g_text_output_buf, total > 1 ? g_ttxt[72] : g_ttxt[71]);
		strcat(g_text_output_buf, g_ttxt[73]);
		DLG->message(g_text_output_buf);
		n = total;
		i = 0;
		do {
			n--;
			sub_hero_le(list[i], random_schweif(2) - 1);
			i++;
		} while (n > 0);
	}
}

void travel_ui_load(Travel far *o)
{
	register signed short had = 0;

	if (g_tev_buf != 0) {
		had = 1;
		g_tev_buf = mem_free(g_tev_buf);
		mem_free(g_tev_txt);
		mem_free(g_tev_loc_buf);
		mem_free(g_tev_loc_txt);
	}
	travel_nop_o213(o);
	rose_alloc(o);
	txt_load_list(g_font, 0xa4, 0x55, &g_ttxt_buf, &g_ttxt);
	txt_load_list(g_font, 0xad, 0x51, &g_ttxt2_buf, &g_ttxt2);
	txt_load_list(g_font, 0x1f1, 0x5e, &g_ttxt3_buf, &g_ttxt3);
	if (had) {
		txt_load_list(g_font, 0x13e, 0x8c, &g_tev_buf, &g_tev_txt);
		txt_load_list(g_font, o->evtxt_file, o->evtxt_n, &g_tev_loc_buf, &g_tev_loc_txt);
	}
}

void travel_ui_unload(Travel far *o)
{
	register signed short had = 0;

	if (g_tev_buf != 0) {
		had = 1;
		g_tev_buf = mem_free(g_tev_buf);
		mem_free(g_tev_txt);
		mem_free(g_tev_loc_buf);
		mem_free(g_tev_loc_txt);
	}
	mem_free(g_ttxt2_buf);
	mem_free(g_ttxt2);
	mem_free(g_ttxt_buf);
	mem_free(g_ttxt);
	mem_free(g_ttxt3_buf);
	mem_free(g_ttxt3);
	travel_anim_off_leave(o);
	rose_free(o);
	if (had) {
		txt_load_list(g_font, 0x13e, 0x8c, &g_tev_buf, &g_tev_txt);
		txt_load_list(g_font, o->evtxt_file, o->evtxt_n, &g_tev_loc_buf, &g_tev_loc_txt);
	}
}

void travel_set_map_pos(Travel far *o, signed short noroute)
{
	signed long step;
	signed long dist;
	signed short way;
	signed short px;
	signed short py;
	unsigned short len;
	unsigned short k = 0;
	signed char i;
	unsigned char tx;
	unsigned char ty;
	register signed short w;

#if FIX_0080
	/* Fix #80: len/step vorbelegen (Start == Ziel) */
	len = 0;
	step = 0;
	if (g_travel_state != 2) {
		if ((unsigned char)g_group->route_from == (unsigned char)g_group->route_to) {
#else
	if (g_travel_state != 2) {
		if ((unsigned char)g_group->route_from == (unsigned char)g_group->route_to) {
#endif
			way = route_first(o, g_group->route_from);
			o->nr_points = route_load_points(o, way);
		} else {
			way = route_find(o, g_group->route_from, g_group->route_to);
			o->nr_points = route_load_points(o, way);
			len = g_routes[way].len * 1000;
			step = len / o->nr_points * 2;
			while (step * k / 2 < (unsigned long)g_group->route_pos)
				k += 2;
			if (k >= 2)
				k -= 2;
		}
		if (noroute == 0) {
			if (o->to >= o->from) {
				px = *(o->points + k);
				py = *(o->points + k + 1);
			} else {
				px = *(o->points + o->nr_points - k);
				py = *(o->points + o->nr_points - k + 1);
			}
			tmap_set_pos(g_tmap, px, py);
		}
		travelmap_route(o, -1);
		if (o->planned == 1)
			travelmap_draw_path(o, -1);
		if (o->route[0] != 0 && g_group->get_turns() == 0) {
			for (i = 0; o->route[i] != o->from; i++) {
				w = route_find(o, o->route[i], o->route[i + 1]);
				if (g_on_boat != 0 && w == 0x17)
					w = 0x1d;
				if (g_routes[w].known == 1) {
					route_load_points(o, w);
					tmap_plot_points(g_tmap, o->points, 0xfd);
				}
			}
		}
		route_load_points(o, way);
		o->turn_pos = g_group->turn_pos;
		o->turns = g_group->get_turns();
		tx = g_group->turn_to;
		ty = g_group->turn_from;
		dist = (unsigned long)g_group->route_pos;
		if (o->from != o->to) {
			if (!o->turns) {
				travelmap_route_part(o, o->from, o->to, 0xc8, dist, 0xfd, 0);
				return;
			}
			if (o->turns & 1) {
				if (o->planned == 1)
					travelmap_route_part(o, o->to, o->from, 0xc8, o->turn_pos, 0xbb, 0);
				else
					travelmap_route_part(o, ty, tx, 0xc8, o->turn_pos, 0xff, 0);
				if (o->from == tx && o->to == ty)
					travelmap_route_part(o, o->from, o->to, 0xc8, dist, 0xfd, len - o->turn_pos);
				else
					travelmap_route_part(o, o->from, o->to, 0xc8, dist, 0xfd, 0);
				return;
			}
			travelmap_route_part(o, o->from, o->to, 0xc8, len - o->turn_pos, o->planned == 1 ? 0xbb : 0xff, 0);
			travelmap_route_part(o, o->from, o->to, 0xc8, dist, 0xfd, len - o->turn_pos);
		}
	}
}
