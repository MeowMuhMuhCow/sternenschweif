/* OVL216 - Overlay O216 (Segment 0x23b3 in SCHWEIF.EXE): Reiseteil - Reise entlang der Route, Tempo, Statusbild
 * (Reiseobjekt Travel). STUFE D1 (wovd1, 19.10.2026): asm-strcat durch echtes strcat(.., "..") ersetzt - Code
 * GLEICH, das Literal im eigenen _DATA bildet cobjfix_d.py auf ds:7202 ab (Bytes geprueft).
 * travel_follow_route(p): Reise entlang der Route route[] (Ende 0xff): je Abschnitt Weg route_find(prev, cur) (0x17 ->
 * 0x1d, 999 = kein Weg: Meldung g_ttxt3[56]), Wegwerte len/road/terrain nach route_len/road/terrain, Strecke len =
 * route_len * 1000, Tempo travel_boat_speed, Wegpunkte route_load_points; Schleife bis route_pos >= len: Uhrzeit
 * g_daytime (+10 je Schritt, timewarp_minutes), alle drei Stunden ab 2 Uhr g_travel_state = 5 (-> travel_set_map_pos),
 * Strecke route_pos += Tempo, Marke auf der Karte (tmap_follow Farbe 0xfd, vorwaerts/rueckwaerts je nach Ortsnummer),
 * nach halber Strecke einmal tevent_run (Wegereignis, route.event > 0), Eingabe travel_redraw/travel_menu. Am Ort:
 * place_reveal_dirs, Orte >= 0xbe per place_event (0xc4 nur bei g_place197_done == 0, 0xc5 beendet einmalig). Danach
 * Abschluss (Ort 0x10 -> rest, Gruppe LE 3..6 und AE 5). travel_road_factor/travel_weather_factor(p, k): Tempofaktor
 * je Wegzustand 0..8 (Gelaende 6: 15) bzw. Wetter 0..4. travel_health_factor(p, mode): Gruppenzustand je Held
 * (get_first_hero/get_next_hero): tot/bewusstlos +1, versteinert +2, LE-Summen, Schuhe (Gegenstaende 0x33/0x32/0x138
 * an Platz 14) bzw. Abnutzung inv[14].f10 >= 10 (Meldung txt[38], drop_item), Zufall -> hero_add_inv_f10;
 * Sammelmeldungen txt[66]/[67] mit Namen (Trenner txt[68]/[69], Schluss "." bzw. txt[70]) und sub_hero_le; Ergebnis
 * 9/12/13 (-1 mit Schuhen). travel_boat_speed: Tempo 600/350 je nach Start-/Zielort (0x1c/0x1e/0x1f).
 * travel_text_speed/weather/terrain/road(p, buf): Texte g_ttxt2[k * 3 + Zufall + 0/15/30/54] fuer status[TST_SPEED/
 * WEATHER/TERRAIN/ROAD]. travel_text_variant: Zufall 0/1/2 (60 %/30 %/10 %). travel_status_update: status[] und shown[]
 * abgleichen, bei Aenderung travel_status_draw: Statusbild 72x77 aus den Bildern 0x97/0x98/0x99 (Teilbilder, Bmp::over)
 * bei (0xf4, 0x38) mit Clip-Rechteck g_clip_rect, Maus verstecken.
 *
 * BCC -mlarge -3 -Ot -Og -Oi -c -Y OVL216.CPP   -> 4848/4848 Bytes GLEICH (18.10.2026), cobjfix laeuft
 *   (Namensfassung mit schweif.h, Reiseobjekt Travel: 19.10.2026 GLEICH, LINKGLEICH)
 * - Ohne -Og 195 Bytes zu kurz, ohne -Oi 3133 Abweichungen (strcat-Intrinsics in travel_health_factor).
 * - C++ wegen inline-Settern mit Parameterkopie: group::set_turns(signed long) (cbw / movsx eax,ax / Store nur al),
 *   set_turn_pos, set_route_from, set_route_to, set_route_pos; Travel::set_route_len/set_road/set_terrain;
 *   group::get_nr_heroes() (signed short aus signed char -> mov al / cbw / mov [bp-3],al).
 * - -Og-Zwischenwerte: (unsigned long)spd ([bp-2eh], an drei Stellen neu gespeichert), g_routes + way ([bp-2ah],
 *   Segment neu aus ds:c7d2), p->nr_points * 2 ([bp-30h]), p + idx ([bp-32h], Segment aus [bp+8]), i * 4 in
 *   travel_health_factor, (int)sick/(int)n am Ende; in travel_status_update sind [bp-1..3] die Zwischenwerte status[].
 * - Ketten: "p->from = prev = p->to = cur", "sick = sa = sb = flag = 0", "n7 = n8 = 0" (rechts nach links;
 *   n8 = n7 = 0 vertauscht die Stores), "g_on_boat = g_travel_state = 0".
 * - Heldenbits als (signed char)h->dead usw. (Original mit unsigned char-Bitfeldern): -> and ax,1 / or al,al;
 *   "!(signed char)h->dead" -> cbw. "(signed short)h->inv[14].f10 >= 10" -> cbw / cmp ax,0ah.
 *   "step * i / 2" -> movzx / push eax / mov eax,step / imul.
 * - travel_status_draw: zweite Sprungtabelle (switch status[TST_ROAD]) liegt vor der ersten; case 2/3 mit gemeinsamem
 *   Rumpf im ersten switch, getrennte gleiche Rumpfe im zweiten; Bmp-Bloecke teilen [bp-22h].
 * Originalfehler: travel_road_factor/travel_weather_factor geben r uninitialisiert zurueck, wenn k ausserhalb 0..8
 *   bzw. 0..4 liegt; travel_boat_speed liefert ein uninitialisiertes Tempo, wenn der Startort nicht 0x1c/0x1e/0x1f
 *   ist. Auffaellig: travel_follow_route verwirft zweimal das Ergebnis von route_load_points (nr_points wird nicht
 *   aktualisiert).
 */
#include <stdio.h>
#include <string.h>
#include "schweif.h"

/* Phase B: eigene Daten des Moduls (ds:7202..7204, dsana) - initialisierte Daten in Adressreihenfolge, danach der Literalpool */

#define DLG		((Dlg far *)g_dialog)
#define INV_FEET	14		/* Inventarplatz Fuesse (Schuhe) */

void travel_follow_route(Travel far *p)
{
	unsigned char cur;
	unsigned char prev;
	signed short way;
	unsigned short spd;
	unsigned short len;
	unsigned short n;
	signed long step;
	signed long t;
	unsigned char next;
	unsigned char idx;
	signed char half;
	signed char hour;
	register unsigned short i;

	next = 1;
	idx = 1;
	half = 0;
	i = 0;
	hour = g_daytime / 90 / 60;
	p->turns = 0;
	p->turn_pos = 0;
	g_group->set_turns(p->turns);
	g_group->set_turn_pos(p->turn_pos);
	cur = p->to = p->route[1];
	prev = p->from;
	p->planned = 0;
	travelmap_route(p, -1);
	while (next != 0xff) {
		g_group->clr_hunger();
		g_group->clr_thirst();
		g_group->refill_water();
		g_group->set_route_from(prev);
		g_group->set_route_to(cur);
		way = route_find(p, prev, cur);
		if (way == 0x17)
			way = 0x1d;
		if (way != 999) {
			p->set_route_len(g_routes[way].len);
			p->set_road(g_routes[way].road);
			p->set_terrain(g_routes[way].terrain);
		} else {
			DLG->message(g_ttxt3[56]);
			route_clear(p);
		}
		p->status[TST_TERRAIN] = 6;
		p->status[TST_ROAD] = 8;
		travelmap_draw(p);
		len = p->route_len * 1000;
		spd = travel_boat_speed(p);
		p->speed = spd;
		p->status[TST_SPEED] = travel_level(p);
		p->nr_points = route_load_points(p, way);
		n = p->nr_points;
		step = len / n * 2;
		while (len > p->route_pos && way != 999) {
			if (g_refresh != 0)
				travelmap_draw(p);
			travel_anim_on(p, 0);
			t = g_daytime;
			if (t < 0x1518L)
				hour = 0;
			if ((t >= 0x2a30L && hour < 2) || (t >= 0x6978L && hour < 5) || (t >= 0xa8c0L && hour < 8) ||
			    (t >= 0xe808L && hour < 11) || (t >= 0x12750L && hour < 14) ||
			    (t >= 0x16698L && hour < 17) || (t >= 0x1a5e0L && hour < 20) ||
			    (t >= 0x1e528L && hour < 23)) {
				hour = t / 90 / 60;
				g_travel_state = 5;
			}
			timewarp_minutes(g_date, 10L);
			p->speed = spd;
			p->route_pos += spd;
			g_group->set_route_pos(p->route_pos);
			if (step * i / 2 < p->route_pos) {
				if (cur > prev) {
					wait_for_vsync();
					tmap_follow(g_tmap, *(p->points + i), *(p->points + i + 1), 1, 0xfd);
					tmap_draw(g_tmap);
				} else {
					wait_for_vsync();
					tmap_follow(g_tmap, *(p->points + p->nr_points - i), *(p->points + p->nr_points - i + 1), 1, 0xfd);
					tmap_draw(g_tmap);
				}
				i += 2;
			}
			if (p->route_pos * 2 > len && half == 0) {
				half = 1;
				if (g_routes[way].event > 0) {
					tevent_run(p, way);
					if (spd != p->speed)
						spd = p->speed;
				}
			}
			if (g_travel_state == 5) {
				g_travel_state = 0;
				travel_set_map_pos(p, 1);
#if FIX_0124
				/* Fix #124: Wegpunktzahl aktualisieren */
				p->nr_points = route_load_points(p, way);
#else
				route_load_points(p, way);
#endif
			}
			travel_redraw(p);
			travel_menu(p);
		}
		travel_reset_pos(p);
		p->from = prev = p->to = cur;
		g_group->set_route_from(prev);
		cur = p->to = p->route[idx + 1];
		place_reveal_dirs(p, prev);
		way = route_find(p, prev, cur);
		if (way == 0x17)
			way = 0x1d;
		if (way == 999)
			next = 0xff;
		else if (p->route[idx] != 0xff) {
#if FIX_0124
			/* Fix #124: Wegpunktzahl aktualisieren */
			p->nr_points = route_load_points(p, way);
#else
			route_load_points(p, way);
#endif
			if (p->route[idx + 1] == 0xff)
				next = p->route[idx];
		}
		idx++;
		if (prev >= 0xbe && (prev != 0xc4 || g_place197_done == 0))
			place_event(p);
		if (prev == 0xc5 && !g_place197_done) {
			next = 0xff;
			g_place197_done = 1;
		}
		i = 0;
	}
	p->speed = 0;
	p->status[TST_SPEED] = travel_level(p);
	g_group->refill_water();
	route_clear(p);
	if (prev == 0x10) {
		p->rest = 1;
		p->from = p->to = 0x10;
	}
	if (prev != 0xc5) {
		g_group->add_le_range(3, 6);
		g_group->add_ae(5);
	}
	g_on_boat = g_travel_state = 0;
	travel_reset_pos(p);
}

signed char travel_road_factor(Travel far *p, signed char k)
{
	signed char r;

#if FIX_0124
	/* Fix #124: Vorgabe fuer unbekannte Wegart (mittleres Tempo) */
	r = 10;
	switch (k) {
	case 0: r = 14; break;
#else
	switch (k) {
	case 0: r = 14; break;
#endif
	case 1: r = 13; break;
	case 2: r = 12; break;
	case 3: r = 11; break;
	case 4: r = 10; break;
	case 5: r = 9; break;
	case 6: r = 8; break;
	case 7: r = 7; break;
	case 8:
		if (p->terrain == 6)
			r = 15;
		else
			r = 6;
		break;
	}
	return r;
}

signed char travel_weather_factor(Travel far *p, signed char k)
{
	signed char r;

#if FIX_0124
	/* Fix #124: Vorgabe fuer unbekanntes Wetter (normal) */
	r = 10;
	switch (k) {
	case 0: r = 10; break;
#else
	switch (k) {
	case 0: r = 10; break;
#endif
	case 1: r = 12; break;
	case 2: r = 11; break;
	case 3: r = 9; break;
	case 4: r = 7; break;
	}
	return r;
}

signed char travel_health_factor(Travel far *p, signed char mode)
{
	signed char r;
	signed char flag;
	signed char n;
	signed char sick;
	signed char i;
	signed char save;
	signed char n7;
	signed char n8;
	signed short sa;
	signed short sb;
	struct hero far *h;
	struct hero far *a[7];
	struct hero far *b[7];

	n = g_group->get_nr_heroes();
	sick = sa = sb = flag = 0;
	n7 = n8 = 0;
	for (i = 0; i < 7; i++) {
		b[i] = 0;
		a[i] = 0;
	}
	h = get_first_hero(g_group);
	do {
		if (h != 0) {
			if ((signed char)h->dead || (signed char)h->unconscious)
				sick += 1;
			if ((signed char)h->petrified)
				sick += 2;
			sa += h->le_max;
			sb += h->le;
			if (get_item_pos(h, 0x33) != 0xe && get_item_pos(h, 0x32) != 0xe && get_item_pos(h, 0x138) != 0xe &&
			    check_hero3(h)) {
				flag = 1;
				if (mode == 1 && check_hero(h)) {
					a[n7] = h;
					n7++;
				}
			} else if ((signed short)h->inv[INV_FEET].f10 >= 10) {
				sprintf(g_text_output_buf, g_ttxt[38], hero_name_gen(h, 1));
				DLG->message(g_text_output_buf);
				drop_item(h, INV_FEET, 1);
			} else if (get_item_pos(h, 0x32) == 0xe) {
				if (random_schweif(0x50) == 1 && check_hero3(h))
					hero_add_inv_f10(h, INV_FEET, 1);
			} else if (random_schweif(0x78) == 1 && check_hero3(h))
				hero_add_inv_f10(h, INV_FEET, 1);
			if (p->hurry == 1 && mode != 0) {
				if ((h->le * 2 <= h->le_max && h->le < 15) || h->le * 3 <= h->le_max) {
					if (!(signed char)h->dead && !(signed char)h->unconscious && !(signed char)h->petrified) {
						b[n8] = h;
						n8++;
					}
				}
			}
		}
		h = get_next_hero(g_group, h);
	} while (get_first_hero(g_group) != h);
	if (n7 > 0) {
		save = n7;
		i = 0;
		sprintf(g_text_output_buf, g_ttxt[66]);
		do {
			strcat(g_text_output_buf, a[i]->name);
			n7--;
			i++;
			if (n7 > 1)
				strcat(g_text_output_buf, g_ttxt[68]);
			else if (n7 == 1)
				strcat(g_text_output_buf, g_ttxt[69]);
		} while (n7 > 0);
		strcat(g_text_output_buf, ".");
		DLG->message(g_text_output_buf);
		n7 = save;
		i = 0;
		do {
			n7--;
			sub_hero_le(a[i], 1);
			i++;
		} while (n7 > 0);
	}
	if (n8 > 0) {
		save = n8;
		i = 0;
		sprintf(g_text_output_buf, g_ttxt[67]);
		do {
			strcat(g_text_output_buf, hero_name_gen(b[i], 1));
			n8--;
			i++;
			if (n8 > 1)
				strcat(g_text_output_buf, g_ttxt[68]);
			else if (n8 == 1)
				strcat(g_text_output_buf, g_ttxt[69]);
		} while (n8 > 0);
		strcat(g_text_output_buf, g_ttxt[70]);
		DLG->message(g_text_output_buf);
		n8 = save;
		i = 0;
		do {
			n8--;
			sub_hero_le(b[i], mode);
			i++;
		} while (n8 > 0);
	}
	if (sick * 3 > n || (sick * 4 > n && sb * 3 < sa) || sb * 8 < sa)
		r = 9;
	else if ((sick == 0 && sb * 2 > sa) || (sick == 1 && sb * 4 > sa * 3))
		r = 13;
	else
		r = 12;
	if (flag == 1)
		r -= 1;
	return r;
}

signed short travel_boat_speed(Travel far *p)
{
	signed short r;

#if FIX_0124
	/* Fix #124: Vorgabe fuer andere Startorte (langsames Schiffstempo) */
	r = 350;
	if (p->route[0] == 0x1c)
		r = 600;
#else
	if (p->route[0] == 0x1c)
		r = 600;
#endif
	if (p->route[0] == 0x1f)
		r = 350;
	if (p->route[0] == 0x1e) {
		if (p->route[1] == 0x1f)
			r = 600;
		else
			r = 350;
	}
	return r;
}

void travel_text_speed(Travel far *p, char far *buf)
{
	sprintf(buf, g_ttxt2[p->status[TST_SPEED] * 3 + travel_text_variant(p)]);
}

void travel_text_weather(Travel far *p, char far *buf)
{
	sprintf(buf, g_ttxt2[p->status[TST_WEATHER] * 3 + travel_text_variant(p) + 15]);
}

void travel_text_terrain(Travel far *p, char far *buf)
{
	sprintf(buf, g_ttxt2[p->status[TST_TERRAIN] * 3 + travel_text_variant(p) + 30]);
}

void travel_text_road(Travel far *p, char far *buf)
{
	sprintf(buf, g_ttxt2[p->status[TST_ROAD] * 3 + travel_text_variant(p) + 54]);
}

signed short travel_text_variant(Travel far *p)
{
	register signed short r;

	r = random_schweif(100);
	return r < 60 ? 0 : r < 90 ? 1 : 2;
}

void travel_status_update(Travel far *p)
{
	register signed short changed = 0;

	p->status[TST_ROAD] = p->road;
	if (p->status[TST_ROAD] != p->shown[TST_ROAD]) {
		changed = 1;
		p->shown[TST_ROAD] = p->status[TST_ROAD];
	}
	p->status[TST_TERRAIN] = p->terrain;
	if (p->status[TST_TERRAIN] != p->shown[TST_TERRAIN]) {
		changed = 1;
		p->shown[TST_TERRAIN] = p->status[TST_TERRAIN];
	}
	p->status[TST_WEATHER] = Wx_get(g_weather);
	if (p->status[TST_WEATHER] != p->shown[TST_WEATHER]) {
		changed = 1;
		p->shown[TST_WEATHER] = p->status[TST_WEATHER];
	}
	if (p->shown[TSH_DAY] != g_is_day) {
		changed = 1;
		p->shown[TSH_DAY] = g_is_day;
	}
	if (changed)
		travel_status_draw(p);
}

void travel_status_draw(Travel far *p)
{
	signed short n;
	signed short dummy;
	signed short hidden = 0;
	struct rect r;
	register signed short j;
	register signed short k;

	n = p->status[TST_WEATHER];
	if (g_is_day == 0)
		n += 5;
	Bmp pic(0x48, 0x4d);
	nvfile_load(g_nvf_heads, 0x97, pic.data(), 0, n, 3, &dummy, &dummy, 0x140);
	switch (p->status[TST_TERRAIN]) {
	case 0: j = 0; break;
	case 1: j = 1; break;
	case 2:
	case 3: j = 2; break;
	case 4: j = 3; break;
	case 5: j = 4; break;
	default: j = 5; break;
	}
	{
		Bmp b(0x48, 0x4d);

		nvfile_load(g_nvf_heads, 0x98, b.data(), 0, j, 3, &dummy, &dummy, 0x140);
		pic.over(&b, 0, 0, 4);
	}
	if (j != 5) {
		switch (p->status[TST_ROAD]) {
		case 0: k = 0; break;
		case 1: k = 0; break;
		case 2: k = 1; break;
		case 3: k = 1; break;
		case 4: k = 2; break;
		case 5: k = 2; break;
		default: k = -1; break;
		}
		if (k != -1) {
			Bmp b(0x48, 0x4d);

			nvfile_load(g_nvf_heads, 0x99, b.data(), 0, k, 3, &dummy, &dummy, 0x140);
			pic.over(&b, 0, 0, 4);
		}
	}
	if (mouse_overlaps_rect(0xf4, 0x38, 0x13b, 0x84)) {
		hidden = 1;
		mouse_hide(0);
	}
	r = g_clip_rect;
	g_clip_rect.y0 = 0x38;
	g_clip_rect.x0 = 0xf4;
	g_clip_rect.y1 = 0x84;
	g_clip_rect.x1 = 0x13b;
	pic.put(g_vga_buf, 0xf4, 0x38);
	g_clip_rect = r;
	if (hidden)
		mouse_show(0);
}
