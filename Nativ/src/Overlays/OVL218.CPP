/* OVL218 - Overlay O218 (Segment 0x23bf in SCHWEIF.EXE): Reiseroute auf der Reisekarte zusammenstellen (Travel:
 * Richtung dir_sel/dir_def, Rose sichtbar sub_scene, Orte from/to, Route route[] (Ende ROUTE_END), route[39], Wegpunkte
 * points/nr_points, mode, f10a, Kartenpunkte map_pts[0..5]).
 * route_plan(p, ort): grosse Routenschleife - naechsten Ort per route_choose_dir waehlen, Weg (route_find_area fuer
 * Orte 2..20 ausser 5/17/18, sonst route_find; Tabelle g_routes) und Wegpunkte (route_load_points), Zahl der
 * bekannten Richtungen place_count_dirs; Rueckfragen (Texte g_ttxt/g_ttxt3) an Kreuzungen, Umkehr mit Rueckbau der
 * Route in einen lokalen Puffer (40 Bytes) und Neuzeichnen (travelmap_walk_x/_y, abs()), Sonderfaelle place_next_dest/
 * place_has_event/place_next_dest_all/place_count_all_dirs; am Ende Start-Marke (tmap_set_pos) und bekannte Wege
 * (tmap_plot_points). route_plan_simple(p, ort): eine Richtung waehlen, dann bis zum naechsten Kreuzungsort
 * (place_next_dest_all; Ort 0x56 mit g_artherion_hut_known/g_artherion_heard, place_reveal_dirs). route_choose_dir(p, &ort, flag): Richtung mit
 * der Kompassrose (rose_open mit dem Ortseintrag aus g_places als Wert, Tastenschleife travelmap_select_dir/
 * travel_redraw/travel_menu, Ziel per route_dest_area bzw. route_dest_by_dir), Rueckgabe Zielort bzw. ROUTE_END (Esc/
 * g_travel_state 10). route_dest_by_dir(p, a, k): Weg mit Ort a und Richtung k in g_routes suchen (vorwaerts ueber
 * dir_from, rueckwaerts ueber dir_to), anderer Endort bzw. ROUTE_END.
 *
 * BCC -mlarge -3 -Ot -Og -c -Y OVL218.CPP   -> 6142/6142 Bytes GLEICH (18.10.2026), cobjfix laeuft
 *   (ohne -Og 5010 Abweichungen). route_plan (4635 Bytes) und route_plan_simple gleich im ersten Versuch.
 *   (Namensfassung mit schweif.h: 19.10.2026 GLEICH, LINKGLEICH)
 * - route_plan: Deklarationsreihenfolge der 24 Skalare bestimmt [bp-1]..[bp-25h]; pv = &v, prev = v, run = 1, way = 0
 *   als initialisierte Deklarationen; Setter g_group->set_route_from() inline (Kopie [bp-26h]), abs() inline
 *   ([bp-28h/-2ah]); -Og-Zwischenwerte (unsigned)m/n, p + n (DI), p + k, p + j, &buf[m+1], nr_points*2, g_routes[way]
 *   ergeben sich von selbst.
 * - Ketten "a = b = c = 0" in Speicherreihenfolge (rechts zuerst), "n += 0xff" (add al,0ffh), "k = k + 1".
 * - route_choose_dir: "#pragma option -r-" vor der Funktion: der -Og-Zwischenwert (unsigned)i muss auf dem Stack
 *   [bp-0ch] liegen (sonst SI + push si); danach "#pragma option -r" fuer route_dest_by_dir (CX/SI).
 * - route_dest_by_dir: Suchschleifen mit leerem Rumpf "for (i = 0; i < NR_ROUTES && (a != x || c != y); i++) ;" - mit
 *   break im Rumpf landet der Test am Schleifenende.
 * - place_count_dirs liefert unsigned char; der direkte Vergleich "< 3" braucht hier (signed char) wie im Original.
 * Originalfehler/Auffaelligkeiten (route_plan): (1) beim Umkehren wird fuer den zweiten Weg buf[1] statt buf[m + 1]
 *   gegen 5/0x11/0x12 geprueft (Kopierfehler); (2) "p->route[j] = p->route[n]" bei n == 0 mit veraltetem j;
 *   (3) Fall "unbekannter Weg, Antwort 2": prev = v ohne v = next, place_next_dest_all bekommt immer denselben Ort
 *   (route_plan_simple setzt v = cur); (4) Suchschleife setzt k = 99 und liest dann noch p->route[99] (hinter der
 *   Route).
 */
#include <stdlib.h>
#include "schweif.h"

#define PLACES ((struct dir_set far *)g_places)	/* Ortstabelle mit Bitsicht (rose_open erwartet struct dir_set) */

void route_plan(Travel far *p, unsigned char v)
{
	unsigned char known;
	signed char ndirs;
	signed char ans;
	unsigned char mode;
	unsigned char has_event;
	signed char k;
	unsigned char n;
	unsigned char j;
	unsigned char m;
	unsigned char next;
	unsigned char area;
	unsigned char cross_from;
	unsigned char cross_to;
	signed short x0;
	signed short y0;
	signed short x1;
	signed short y1;
	signed short adx;
	signed short ady;
	signed short r;
	unsigned char run = 1;
	signed short way = 0;
	unsigned char far *pv = &v;
	unsigned char prev = v;
	unsigned char buf[40];

	for (m = 0; m < 40; m++)
		buf[m] = 0;
	next = 1;
	n = j = m = k = area = 0;
	cross_from = cross_to = p->from;
	ndirs = ans = mode = has_event = 0;
	p->from = v;
	g_group->set_route_from(p->from);
	route_clear(p);
	travelmap_set_point1(p, v);
	p->route[n] = p->from;
	n++;
	while (run != 0xff) {
		if (n == 1) {
			p->map_pts[2] = p->map_pts[4] = p->map_pts[0];
			p->map_pts[3] = p->map_pts[5] = p->map_pts[1];
			mode = area = k = 0;
			p->route[0] = cross_from = cross_to = v = p->from;
			if (p->mode == 1) {
				p->route[n] = next = p->to;
			} else {
				next = route_choose_dir(p, pv, 1);
				if (next == v || next == 0xff) {
					n += 0xff;
					run = 0xff;
				}
				p->route[n] = next;
			}
			cross_from = v;
			n++;
			prev = v;
			v = next;
		}
		if (p->route[n] != ROUTE_END) {
			r = 0;
			p->route[n + 1] = 0;
			ndirs = place_count_dirs(p, v);
			if (prev > 1 && prev <= 0x14 && prev != 5 && prev != 0x11 && prev != 0x12)
				way = route_find_area(p, prev, next);
			else
				way = route_find(p, prev, next);
			p->nr_points = route_load_points(p, way);
			if (way != ROUTE_NONE)
				known = g_routes[way].known;
			else
				dlg_message(g_dialog, g_ttxt3[56]);
		}
		if ((v <= 0x20 || v == 0x2c || ndirs >= 3) && known == 1 && p->route[n] != ROUTE_END) {
			p->route[n] = ROUTE_END;
			if (mode == 0)
				travelmap_show_route(p, cross_to);
			if (mode == 2)
				ans = 1;
			else
				ans = dlg_yes_no(g_dialog, g_ttxt[13]);
			mode = 0;
			if (ans == 1) {
				cross_to = v;
				travelmap_set_point2(p, v);
				p->map_pts[4] = p->map_pts[2];
				p->map_pts[5] = p->map_pts[3];
				next = route_choose_dir(p, &v, 0);
				if (p->from == next && p->route[n - 2] != next && p->route[1] != ROUTE_END)
					k = 1;
				if (k == 0 && p->route[n - 2] != next && p->route[1] != ROUTE_END) {
					while (p->route[k] != 0 && (unsigned char)k < n) {
#if FIX_0170
						/* Fix #170: Suche beenden, ohne p->route[99] zu lesen */
						if (p->route[k] == next) {
							k = 99;
							break;
						}
#else
						if (p->route[k] == next)
							k = 99;
#endif
						else
							k = k + 1;
					}
					if (k != 99)
						k = 0;
				}
				if (k != 0) {
					r = -1;
					while (r == -1) {
						if (k == 1)
							r = dlg_choose(g_dialog, g_ttxt[75], 2, g_ttxt[77], g_ttxt[78]);
						else
							r = dlg_choose(g_dialog, g_ttxt[76], 2, g_ttxt[77], g_ttxt[78]);
					}
					if (r == 1) {
						p->route[n + 1] = ROUTE_END;
						run = 0xff;
					} else {
						travelmap_route(p, -1);
						tmap_draw(g_tmap);
						route_clear(p);
						n = 0;
						p->route[n] = v = next = p->from;
					}
				}
				if (next == 0xff)
					run = 0xff;
				if (next <= 0x14 && next != 5 && next != 6 && next != v)
					area = place_area(p, next);
				else
					area = 0;
				if ((p->route[n - 2] == next || place_area(p, p->route[n - 2]) == area) && !k &&
				    p->route[1] != ROUTE_END) {
					mode = 1;
					m = 0;
					p->route[n] = 0;
					n += 0xff;
					buf[m] = p->route[n];
					m = m + 1;
					p->route[n] = 0;
					n--;
					ndirs = place_count_dirs(p, p->route[n]);
					while (n != 0 && p->route[n] != cross_from && p->route[n] > 0x20 && p->route[n] != 0x2c &&
					       ndirs < 3) {
						buf[m] = p->route[n];
						ndirs = place_count_dirs(p, p->route[n]);
						p->route[n] = 0;
						m++;
						n--;
					}
					buf[m] = p->route[n];
					buf[m + 1] = ROUTE_END;
					p->route[n + 1] = ROUTE_END;
					if (n != 0) {
						for (j = n + 0xff; p->route[j] > 0x20 && (signed char)place_count_dirs(p, p->route[j]) < 3;
						     j += 0xff)
							;
					} else {
#if FIX_0170
						/* Fix #170: j bei n == 0 nicht veraltet benutzen */
						j = n;
						p->route[j] = p->route[n];
#else
						p->route[j] = p->route[n];
#endif
					}
					cross_to = p->route[n];
					cross_from = p->route[j];
					next = v = p->route[n];
					prev = p->route[n - 1];
					if (buf[1] > 1 && buf[1] <= 0x14 && buf[1] != 5 && buf[1] != 0x11 && buf[1] != 0x12)
						way = route_find_area(p, buf[1], buf[0]);
					else
						way = route_find(p, buf[0], buf[1]);
					p->nr_points = route_load_points(p, way);
					if (buf[1] > buf[0]) {
						x0 = *p->points;
						y0 = *(p->points + 1);
					} else {
						x0 = *(p->points + p->nr_points);
						y0 = *(p->points + p->nr_points + 1);
					}
					for (m = 0; buf[m + 1] != 0xff; m++)
						;
					m += 0xff;
#if FIX_0170
					/* Fix #170: zweiten Weg mit buf[m + 1] pruefen (Kopierfehler) */
					if (buf[m + 1] > 1 && buf[m + 1] <= 0x14 && buf[m + 1] != 5 && buf[m + 1] != 0x11 &&
					    buf[m + 1] != 0x12)
#else
					if (buf[m + 1] > 1 && buf[m + 1] <= 0x14 && buf[1] != 5 && buf[1] != 0x11 &&
					    buf[1] != 0x12)
#endif
						way = route_find_area(p, buf[m + 1], buf[m]);
					else
						way = route_find(p, buf[m], buf[m + 1]);
					p->nr_points = route_load_points(p, way);
					if (buf[m + 1] > buf[m]) {
						x1 = *(p->points + p->nr_points);
						y1 = *(p->points + p->nr_points + 1);
					} else {
						x1 = *p->points;
						y1 = *(p->points + 1);
					}
					adx = abs(x0 - x1);
					ady = abs(y0 - y1);
					if (adx > ady) {
						x0 = travelmap_walk_x(p, x0, y0, x1, cross_to);
						y0 = travelmap_walk_y(p, x0, y0, y1, cross_to);
					} else {
						y0 = travelmap_walk_y(p, x0, y0, y1, cross_to);
						x0 = travelmap_walk_x(p, x0, y0, x1, cross_to);
					}
					travelmap_route(p, -1);
					travelmap_draw_path(p, -1);
					wait_for_vsync();
					tmap_draw(g_tmap);
					n++;
					if (n == 1) {
						dlg_message(g_dialog, g_ttxt[24]);
						run = 0xff;
					} else {
						p->route[n] = p->route[n - 1];
						p->route[n + 1] = ROUTE_END;
					}
				} else if (!k && p->route[1] == ROUTE_END) {
					run = 0xff;
				} else {
					p->route[n] = next;
					cross_from = v;
					n++;
					prev = v;
					v = next;
				}
			} else {
				run = 0xff;
				p->route[n] = ROUTE_END;
				n++;
			}
		} else if (ndirs == 2 && known == 1 && ans != 2) {
			next = place_next_dest(p, v, prev);
			if (p->from == next && p->route[n - 2] != next)
				k = 1;
			if (k == 0) {
				while (p->route[k] != 0 && k < 40) {
#if FIX_0170
					/* Fix #170: Suche beenden, ohne p->route[99] zu lesen */
					if (p->route[k] == next) {
						k = 99;
						break;
					}
#else
					if (p->route[k] == next)
						k = 99;
#endif
					else
						k = k + 1;
				}
				if (k != 99)
					k = 0;
			}
			if (k != 0) {
				r = -1;
				while (r == -1) {
					if (k == 1)
						r = dlg_choose(g_dialog, g_ttxt[75], 2, g_ttxt[77], g_ttxt[78]);
					else
						r = dlg_choose(g_dialog, g_ttxt[76], 2, g_ttxt[77], g_ttxt[78]);
				}
				if (r == 1) {
					p->route[n + 1] = ROUTE_END;
					run = 0xff;
				} else {
					travelmap_route(p, -1);
					tmap_draw(g_tmap);
					route_clear(p);
					n = 0;
				}
				k = 0;
			}
			prev = v;
			if (r == 2)
				v = prev = next = cross_to = p->from;
			else
				v = next;
			p->route[n] = next;
			n++;
		} else if (ndirs == 1 && known == 1) {
			has_event = place_has_event(p, v);
			if (has_event == 1) {
				ans = -1;
				while (ans == -1)
					ans = dlg_choose(g_dialog, g_ttxt[14], 2, g_ttxt[78], g_ttxt[79]);
				if (ans == 1) {
					travelmap_route(p, -1);
					tmap_draw(g_tmap);
					route_clear(p);
					n = 0;
					ans = 0;
				}
			} else {
				ans = -1;
				while (ans == -1)
					ans = dlg_choose(g_dialog, g_ttxt3[57], 2, g_ttxt3[58], g_ttxt3[59]);
			}
			has_event = 0;
			switch (ans) {
			case 1:
				n--;
				while (n != 0 && p->route[n] != cross_from) {
					v = p->route[n];
					next = cross_from;
					p->route[n] = 0;
					n--;
				}
				break;
			case 2:
				run = 0xff;
				p->route[n] = ROUTE_END;
				break;
			}
			n++;
			prev = v;
			v = next;
		} else if (known != 1) {
			ans = -1;
			while (ans == -1)
				ans = dlg_choose(g_dialog, g_ttxt[15], 2, g_ttxt[16], g_ttxt[17]);
			switch (ans) {
			case 1:
				n--;
				mode = 2;
				while (n != 0 && p->route[n] != cross_from) {
					v = p->route[n];
					next = cross_from;
					p->route[n] = 0;
					n--;
				}
				next = p->route[n];
				v = p->route[n - 1];
				tmap_draw(g_tmap);
				break;
			case 2:
				p->route[39] = p->route[n - 2];
#if FIX_0170
				/* Fix #170: dem Weg wirklich folgen (v = next), Route nicht ueberlaufen lassen */
				while (n < 38 && ((place_count_all_dirs(p, next) == 2 && next > 0x20 && next != 0x2c) ||
				       (next == 0x56 && (g_artherion_hut_known == 0 || !!g_artherion_heard)))) {
					next = place_next_dest_all(p, v, prev);
					p->route[n] = next;
					prev = v;
					v = next;
					n++;
				}
#else
				while ((place_count_all_dirs(p, next) == 2 && next > 0x20 && next != 0x2c) ||
				       (next == 0x56 && (g_artherion_hut_known == 0 || !!g_artherion_heard))) {
					next = place_next_dest_all(p, v, prev);
					p->route[n] = next;
					prev = v;
					n++;
				}
#endif
				run = 0xff;
				p->route[n] = ROUTE_END;
				break;
			}
			n++;
			prev = v;
			v = next;
		}
	}
	if (p->route[1] != ROUTE_END) {
		way = route_find(p, p->route[0], p->route[1]);
		p->nr_points = route_load_points(p, way);
		if (g_routes[way].from < g_routes[way].to)
			tmap_set_pos(g_tmap, *p->points, *(p->points + 1));
		else
			tmap_set_pos(g_tmap, *(p->points + p->nr_points), *(p->points + p->nr_points + 1));
		travelmap_route(p, -1);
		for (n = 0; p->route[n + 1] != ROUTE_END; n++) {
			if (p->route[n] > 1 && p->route[n] <= 0x14 && p->route[n] != 5 && p->route[n] != 0x11 &&
			    p->route[n] != 0x12)
				way = route_find_area(p, p->route[n], p->route[n + 1]);
			else
				way = route_find(p, p->route[n], p->route[n + 1]);
			route_load_points(p, way);
			if (g_routes[way].known == 1)
				tmap_plot_points(g_tmap, p->points, 0xff);
		}
	}
}

void route_plan_simple(Travel far *p, unsigned char v)
{
	unsigned char prev;
	unsigned char cur;
	unsigned char n;
	unsigned char far *pv = &v;

	p->from = v;
	g_group->set_route_from(p->from);
	cur = 1;
	n = 0;
	travelmap_set_point1(p, v);
	route_clear(p);
	p->map_pts[2] = p->map_pts[4] = p->map_pts[0];
	p->map_pts[3] = p->map_pts[5] = p->map_pts[1];
	p->route[n] = p->from;
	n++;
	prev = p->from;
	if (p->mode == 1) {
		p->route[n] = cur = v = p->to;
	} else {
		cur = route_choose_dir(p, pv, 1);
		if (cur == v)
			n += 0xff;
		if (cur == 0x56 && g_artherion_hut_known != 0)
			place_reveal_dirs(p, cur);
		p->route[n] = cur;
		v = cur;
	}
	n++;
	if (cur != prev) {
		while ((place_count_all_dirs(p, cur) == 2 && cur > 0x20 && cur != 0x2c && cur != 0x67) ||
		       (cur == 0x56 && (g_artherion_hut_known == 0 || !!g_artherion_heard))) {
			cur = place_next_dest_all(p, v, prev);
			p->route[n] = cur;
			prev = v;
			v = cur;
			n++;
		}
	}
	v = cur;
	p->route[n] = ROUTE_END;
}

#pragma option -r-
unsigned char route_choose_dir(Travel far *p, unsigned char far *pv, signed char first)
{
	unsigned char far *q = pv;
	unsigned char dest;
	signed char dir;
	signed char old_dir;
	signed char done = 0;
	unsigned char i = 0;

	if (*pv > 1 && *pv <= 0x14 && *pv != 0x11 && *pv != 0x12 && first == 0)
		*pv = place_area(p, *pv);
	for (; PLACES[i].id != *pv; i++)
		;
	p->dir_sel = old_dir = 0;
	rose_open(p, PLACES[i]);
	g_rose_active = 1;
	if (p->map_pts[0] != p->map_pts[2] || p->map_pts[1] != p->map_pts[3])
		g_rose_two_marks = 1;
	while (!done) {
		if (p->sub_scene) {
			travelmap_select_dir(p);
			if (p->dir_sel == 0) {
				if (p->f10a == 0) {
					rose_dir_allowed(p);
				} else {
					p->f10a = 0;
					dlg_message(g_dialog, g_ttxt[26]);
					rose_dir_allowed(p);
				}
			}
			if (g_key == 0x11b) {
				travel_redraw(p);
				if (g_travel_state == 9 || g_travel_state == 10) {
					if (g_travel_state == 9)
						dest = p->from;
					else
						dest = 0xff;
					done = 1;
					g_travel_state = 0;
				}
			}
			if (!(g_key == 0 || g_key == 0x4b00 || g_key == 0x4d00) || g_mouse_lclick != 0) {
				if ((g_mouse_lclick != 0 && g_action < 9) || g_key == 0x1c0d) {
					p->dir_def = dir = p->dir_sel - 1;
					if (*pv >= 0xe6)
						dest = route_dest_area(p, q, dir);
					else
						dest = route_dest_by_dir(p, *pv, dir);
					if (*pv == dest || dest == 0xff) {
						p->dir_sel = old_dir;
						dlg_message(g_dialog, g_ttxt[26]);
					} else {
						done = 1;
					}
					rose_draw(p);
					wait_ticks(12);
				} else if (g_action != 0 && g_action != 0xf5 && g_action != 0xf6 && g_key != 0x4900) {
					travel_menu(p);
				}
				clear_input();
				mouse_clear_left();
				mouse_clear_right();
				p->f10a = 0;
			}
			old_dir = p->dir_sel;
		}
	}
	rose_hide(p);
	g_rose_active = 0;
	g_rose_two_marks = 0;
	if (p->map_pts[0] != p->map_pts[2] || p->map_pts[1] != p->map_pts[3])
		tmap_mark2_free(g_tmap);
	tmap_mark1_free(g_tmap);
	return dest;
}

#pragma option -r
unsigned char route_dest_by_dir(Travel far *p, unsigned char a, unsigned char dir)
{
	unsigned char dest = 0xff;
	unsigned short i;

	for (i = 0; i < NR_ROUTES && (g_routes[i].from != a || g_routes[i].dir_from != dir); i++)
		;
	if (g_routes[i].from == a && g_routes[i].dir_from == dir)
		dest = g_routes[i].to;
	if (dest == 0xff) {
		for (i = 0; i < NR_ROUTES && (g_routes[i].to != a || g_routes[i].dir_to != dir); i++)
			;
		if (g_routes[i].to == a && g_routes[i].dir_to == dir)
			dest = g_routes[i].from;
	}
	return dest;
}
