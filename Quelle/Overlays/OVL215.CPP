/* OVL215 - Overlay O215 (Segment 0x23b0 in SCHWEIF.EXE): Reisen auf der Karte - ein Weg zwischen zwei Orten
 * (Reiseobjekt Travel, Gruppe g_group, Wegtabelle g_routes, Texte g_ttxt/g_ttxt3).
 * travel_resume(p, pos, speed): Weg aus den Gruppendaten wiederherstellen (route_len/1000, route_from/to, route_pos,
 *   turn_pos, turns), Route neu (route_clear) ausser bei g_travel_state == 3, Weg zeichnen travelmap_route_part (bei
 *   Umkehr zweiteilig), Rueckgabe neue Wegposition. Gerufen von travel_route bei g_travel_state == 2.
 * travel_route(p, &von, &nach): Hauptschleife eines Weges (aus O223): Weg route_find, Laenge route_len * 1000
 *   (-> group.route_len), Tempo aus travel_road/weather/health_factor (/4, Schiff 800), Wegpunkte zeichnen
 *   (tmap_follow),
 *   Uhrzeit g_daytime (Rast- bzw. Nachtwarnungen, Stunde /90/60, g_travel_state = 5), Zufallsbegegnungen
 *   travel_encounter je Gelaende (W200 == 1), Treffen mit anderen Gruppen auf demselben Weg (g_groups), Ereignisse bei
 *   1/2, 1/30, 29/30 (tevent_run), 1/4 (armed_hero_fight), 3/5 (Wasserstelle g_water_routes), 1/3 (Verfolger
 *   ambush_fight, travel_scene_31) der Strecke, Rast/Umkehr ueber action (travel_reverse, route_rebuild, flag_not),
 *   Gruppenzustand neu (tot/bewusstlos/versteinert, LE). Rueckgabe 1, wenn umgekehrt/abgebrochen wurde.
 *
 * BCC -mlarge -3 -Ot -Og -c -Y OVL215.CPP   -> 6791/6791 Bytes GLEICH (18.10.2026), cobjfix laeuft (-Oi ebenso;
 *   ohne -Og 4563 Abweichungen).
 *   (Namensfassung mit schweif.h, Reiseobjekt Travel: 19.10.2026 GLEICH, LINKGLEICH)
 * - C++ nur wegen inline-Elementfunktionen: group::set_route_from/set_route_to/set_route_len/set_route_pos
 *   (Parameterkopien [bp-41h].. und long-Kopien [bp-4ah]/[bp-4eh]/[bp-52h]/[bp-56h]), set_turns(long) [bp-5ah],
 *   Travel::set_route_len/set_road/set_terrain ([bp-43h]..), Getter get_nr_heroes() (mov al / cbw / mov [x],al),
 *   get_route_len()/get_route_pos() (mov eax -> nur ax gespeichert bzw. mov edx / sub ax,dx), get_turns().
 * - group.route_from/route_to (signed char) und route_pos (signed long) werden unsigned gelesen: Casts.
 * - Lange Zuweisungskette am Anfang (rechts zuerst gespeichert), Tabelle "struct route_list42 water =
 *   g_water_routes" (0x54 Bytes, ds:71ae in g_rose_areas), redraw = 1 bei der Deklaration (vor der Tabellenkopie).
 * - "*(p->points + pos + 1)" -> [bx+2]; "p->points[pos + 1]" rechnet inc/shl. travel_reverse/travelmap_route_part
 *   liefern unsigned short (sonst "add ax,0fffeh" statt "sub ax,2" bei "- 2").
 * - switch (p->terrain) mit Sprungtabelle 0..5: Rumpfreihenfolge case 1, 0, 5, 2 (3/4 fehlen).
 * - travel_scene_31 (O164, ohne Parameter) wird mit p aufgerufen: gecasteter Funktionszeiger.
 * Originalfehler/Auffaelliges: "if ((unsigned char)p->hurry == 0xff) ;" (leere Anweisung). Zweimal "if (speed >=
 *   p->route_pos) p->route_pos += speed;" (Vergleich wirkt verdreht). Beim Gruppentreffen bleibt other_pos aus einem
 *   frueheren Durchlauf stehen, wenn keine der beiden Lagen passt, und wird zudem fuer das Ja/Nein von yes_no_box
 *   missbraucht. enc_lvl (Begegnungsstufe) ist bei way == 999 uninitialisiert (dann laeuft die Schleife aber nicht).
 */
#include "schweif.h"

#define DLG		((Dlg far *)g_dialog)

signed short travel_resume(Travel far *p, register signed short pos, register signed short speed)
{
	signed short d;

	p->route_len = (unsigned long)g_group->route_len / 1000;
	d = g_group->get_route_len();
	p->from = g_group->route_from;
	p->to = g_group->route_to;
	p->route_pos = (unsigned long)g_group->route_pos;
	p->turn_pos = g_group->turn_pos;
	p->turns = g_group->get_turns();
	if (g_travel_state != 3) {
		route_clear(p);
		p->route[0] = p->from;
		p->route[1] = p->to;
		p->route[2] = 0xff;
	}
	if (p->from != p->to) {
		travelmap_route(p, -1);
		if (!p->turns)
			pos = travelmap_route_part(p, p->from, p->to, speed, p->route_pos, 0xfd, 0);
		else if (p->turns & 1) {
			travelmap_route_part(p, p->to, p->from, speed, p->turn_pos, 0xff, 0);
			pos = travelmap_route_part(p, p->from, p->to, speed, p->route_pos, 0xfd, d - p->turn_pos);
		} else {
			travelmap_route_part(p, p->from, p->to, speed, p->route_pos, 0xff, 0);
			pos = travelmap_route_part(p, p->from, p->to, speed, p->route_pos, 0xfd, d - p->turn_pos);
		}
	}
	p->reenter = 0;
	tmap_draw(g_tmap);
	g_travel_state = 0;
	return pos;
}

signed char travel_route(Travel far *p, unsigned char far *a, unsigned char far *b)
{
	signed char turned;
	signed char night;
	signed char enc_lvl;
	signed char gi;
	signed char ri;
	signed char n_heroes;
	signed char sick_pts;
	signed char n_sick;
	signed char meet_wait;
	signed char f_weather;
	signed char f_road;
	signed char f_health;
	signed char ev_quarter;
	signed char water_due;
	signed char ev_third;
	signed char hour;
	signed char swap;
	signed char meet;
	signed long step;
	signed long t;
	signed short le_max_sum;
	signed short le_sum;
	signed short awake;
	signed short water_idx;
	unsigned short speed;
	unsigned short dist;
	signed short other_pos;
	unsigned short last_pt;
	unsigned short pos;
	unsigned short old_pos;
	signed short way;
	signed short way2;
	struct hero far *h;
	struct group far *g;
	signed short px;
	signed short py;
	signed short redraw = 1;
	struct route_list42 water = g_water_routes;

	pos = old_pos = other_pos = turned = night = p->action = p->ev_half = p->ev_start = p->ev_end = ev_quarter = water_due = meet = ev_third = meet_wait = hour = 0;
	g_guide_won = 0;
	way = 999;
	t = g_daytime;
	if (*b == 0)
		*b = (unsigned char)g_group->route_to;
	p->from = *a;
	p->to = *b;
	g_group->set_route_from(*a);
	g_group->set_route_to(*b);
	way = route_find(p, *a, *b);
	if (way != 999) {
		p->nr_points = route_load_points(p, way);
		p->set_route_len(g_routes[way].len);
		p->set_road(g_routes[way].road);
		p->set_terrain(g_routes[way].terrain);
		if (g_routes[way].from == p->from)
			enc_lvl = encounter_level(p, g_routes[way].dir_from);
		else
			enc_lvl = encounter_level(p, g_routes[way].dir_to);
	} else {
		DLG->message( g_ttxt3[56]);
		route_clear(p);
	}
	dist = p->route_len * 1000;
	g_group->set_route_len(dist);
	p->weather = Wx_get(g_weather);
	f_road = travel_road_factor(p, p->road);
	f_weather = travel_weather_factor(p, p->weather);
	f_health = travel_health_factor(p, 0);
	awake = g_group->count_check1();
	if (awake == 0) {
		awake = g_group->count_check3();
		if (awake == 0)
			g_travel_state = 1;
		else {
			h = get_first_hero(g_group);
			do {
				set_asleep(h, 0);
				h = get_next_hero(g_group, h);
			} while (get_first_hero(g_group) != h);
		}
	}
	if (p->mode == 2)
		p->speed = speed = 800;
	else
		p->speed = speed = f_weather * f_road * f_health / 4;
	p->status[TST_SPEED] = travel_level(p);
	travel_anim_on(p, 0);
	travel_status_update(p);
	if (g_once_anim == 1 && group_find_item(g_group, 0x117) >= 0)
		p->danger += g_routes[way].danger;
	last_pt = p->nr_points;
	step = dist / last_pt * 2;
	if (t > 0x17bb0L && t < 0x1a5e0L)
		night = 2;
	p->route_pos = (unsigned long)g_group->route_pos;
	pos = 0;
	if (p->mode == 1 || p->mode == 0x0d) {
		while (step * pos / 2 < (unsigned long)g_group->route_pos)
			pos += 2;
		if (pos >= 2)
			pos -= 2;
		hour = g_daytime / 90 / 60;
		p->mode = 0;
	}
	if (g_nr_groups != 1) {
		for (gi = 0, g = g_groups; gi < 6; gi++, g++) {
			if (group_get_on_route(g) && group_get_on_route(g_group) && g != g_group && !!g->nr_heroes) {
				if ((unsigned char)g->route_from == (unsigned char)g_group->route_from && (unsigned char)g->route_to == (unsigned char)g_group->route_to)
					meet = 1;
				if ((unsigned char)g->route_from == (unsigned char)g_group->route_to && (unsigned char)g->route_to == (unsigned char)g_group->route_from)
					meet = 1;
			}
		}
	}
	if (p->route_pos * 2 > dist)
		p->ev_half = 1;
	if (p->route_pos * 30 > dist)
		p->ev_start = 1;
	if (p->route_pos * 30 / 29 > dist)
		p->ev_end = 1;
	if (p->route_pos * 4 > dist)
		ev_quarter = 1;
	water_idx = is_in_word_array(way, &water);
	if (water_idx >= 0 && water_idx < 42 && p->route_pos * 5 / 3 <= dist && group_find_item(g_group, 0x1e) != -1 && !turned)
		water_due = 1;
	while (dist > p->route_pos && way != 999 && g_travel_state == 0) {
		if (g_group->count_check1() != awake)
			redraw = 1;
		if (g_refresh != 0)
			travelmap_draw(p);
		t = g_daytime;
		if (t < 0xbdd8L)
			hour = 0;
		if (t >= 0xfd20L && t <= 0x11238L && hour < 12) {
			hour = 12;
			travel_weather_effects(p);
			g_travel_state = 5;
		}
		if ((t >= 0xd2f0L && hour < 10) || (t >= 0x13c68L && hour < 15) || (t >= 0x17bb0L && hour < 18)) {
			hour = t / 90 / 60;
			g_travel_state = 5;
		}
		timewarp_minutes(g_date, 10);
		t = g_daytime;
		p->speed = speed;
		p->route_pos += speed;
		g_group->set_route_pos(p->route_pos);
		while (step * pos / 2 < p->route_pos && dist > p->route_pos && g_travel_state != 4) {
			if (p->to > p->from) {
				wait_for_vsync();
				px = p->points[pos];
				py = *(p->points + pos + 1);
			} else {
				wait_for_vsync();
				px = *(p->points + p->nr_points - pos);
				py = *(p->points + p->nr_points - pos + 1);
			}
			tmap_follow(g_tmap, px, py, 1, 0xfd);
			tmap_draw(g_tmap);
			pos += 2;
			if (meet_wait > 0)
				meet_wait -= 1;
			if (random_schweif(200) == 1) {
				if (g_routes[way].event == 0) {
					travel_anim_off(p, 1);
					switch (p->terrain) {
					case 1: travel_encounter(p, 1, enc_lvl); break;
					case 0: travel_encounter(p, 0, enc_lvl); break;
					case 5: travel_encounter(p, 5, enc_lvl); break;
					case 2: travel_encounter(p, 2, enc_lvl); break;
					}
					travel_anim_on(p, 1);
					redraw = 1;
				}
			}
		}
		if (meet == 1 && g_travel_state != 1) {
			for (gi = 0, g = g_groups; gi < 6; gi++, g++) {
				if (group_get_on_route(g) && group_get_on_route(g_group) && g != g_group && !!g->nr_heroes) {
#if FIX_0196
					/* Fix #196: Lage der anderen Gruppe nur verwenden, wenn sie in diesem Durchlauf auf demselben Weg gefunden wurde; Ja/Nein in eigener Variable */
					{
						signed short found = 0;
						if ((unsigned char)g->route_from == (unsigned char)g_group->route_from && (unsigned char)g->route_to == (unsigned char)g_group->route_to) {
							other_pos = g->get_route_pos();
							found = 1;
						}
						if ((unsigned char)g->route_from == (unsigned char)g_group->route_to && (unsigned char)g->route_to == (unsigned char)g_group->route_from) {
							other_pos = g_routes[way].len * 1000 - g->get_route_pos();
							found = 1;
						}
						if (found) {
							if (other_pos - speed / 2 < (unsigned long)g_group->route_pos && other_pos + speed / 2 > (unsigned long)g_group->route_pos) {
								if (meet_wait == 0) {
									if (g->count_check1())
										found = DLG->yes_no_box( g_ttxt[36]);
									else
										found = DLG->yes_no_box( g_ttxt[80]);
									if (found == 1)
										travel_stop(p, 1);
									meet_wait = 2;
								}
							} else
								meet_wait = 0;
						}
					}
#else
					if ((unsigned char)g->route_from == (unsigned char)g_group->route_from && (unsigned char)g->route_to == (unsigned char)g_group->route_to)
						other_pos = g->get_route_pos();
					if ((unsigned char)g->route_from == (unsigned char)g_group->route_to && (unsigned char)g->route_to == (unsigned char)g_group->route_from)
						other_pos = g_routes[way].len * 1000 - g->get_route_pos();
					if (other_pos - speed / 2 < (unsigned long)g_group->route_pos && other_pos + speed / 2 > (unsigned long)g_group->route_pos) {
						if (meet_wait == 0) {
							if (g->count_check1())
								other_pos = DLG->yes_no_box( g_ttxt[36]);
							else
								other_pos = DLG->yes_no_box( g_ttxt[80]);
							if (other_pos == 1)
								travel_stop(p, 1);
							meet_wait = 2;
						}
					} else
						meet_wait = 0;
#endif
				}
			}
		}
		if (g_travel_state != 1)
			travel_redraw(p);
		if (ev_quarter == 0 && g_armed_fight_lost == 1 && !g_travel_state && p->route_pos * 4 > dist) {
			ev_quarter = 1;
			if (p->terrain == 1) {
				travel_anim_off(p, 1);
				armed_hero_fight(p, enc_lvl);
				redraw = 1;
				travel_anim_on(p, 1);
			}
		}
		if (water_due != 0 && p->route_pos * 5 / 3 > dist && !turned) {
			DLG->message( g_ttxt[83]);
			g_group->refill_water();
			timewarp_minutes(g_date, 30);
			water_due = 0;
		}
		if (ev_third == 0 && p->route_pos * 3 > dist && !g_travel_state) {
			ev_third = 1;
			travel_anim_off(p, 1);
			if (g_ambush_count < 5) {
				if (p->danger >= 18)
					DLG->message( g_ttxt[23]);
				if (p->danger >= 20) {
					DLG->message( g_ttxt3[14]);
					ambush_fight(p, enc_lvl);
					p->danger -= 18;
					p->danger /= 2;
				}
			}
			if (g_scene31_due == 1 && g_rondra_priestess_met == 1 && group_find_item(g_group, 0x117) >= 0) {
				g_scene31_due = 0;
				((void (*)(Travel far *))travel_scene_31)(p);
			}
			redraw = 1;
			travel_anim_on(p, 1);
		}
		if (!g_travel_state) {
			if (p->ev_half == 0 && p->action == 0 && p->route_pos * 2 > dist) {
				p->ev_half = 1;
				travel_health_factor(p, 2);
				if (g_routes[way].event > 0 && g_routes[way].event < 50) {
					travel_anim_off(p, 1);
					tevent_run(p, way);
					redraw = 1;
					if (speed != p->speed)
						speed = p->speed;
					travel_anim_on(p, 1);
				}
			}
			if (p->ev_start == 0 && p->action == 0 && p->route_pos * 30 > dist) {
				p->ev_start = 1;
				travel_health_factor(p, 1);
				if (g_routes[way].event >= 50) {
					travel_anim_off(p, 1);
					tevent_run(p, way);
					redraw = 1;
					if (speed != p->speed)
						speed = p->speed;
					travel_anim_on(p, 1);
				}
			}
			if (p->ev_end == 0 && p->action == 0 && p->route_pos * 30 / 29 > dist) {
				p->ev_end = 1;
				travel_health_factor(p, 1);
				if (g_routes[way].event >= 50) {
					travel_anim_off(p, 1);
					tevent_run(p, way);
					redraw = 1;
					if (speed != p->speed)
						speed = p->speed;
					travel_anim_on(p, 1);
				}
			}
		} else
			p->route_pos -= speed;
		if (g_travel_state == 1)
			redraw = 0;
		else if (redraw == 1)
			travel_menu(p);
		else
			redraw = travel_menu(p);
		if (g_travel_state != 1) {
			if (t >= 0x17bb0L && t < 0x1a5e0L && night == 0)
				night = DLG->choose( g_ttxt[3], 2, g_ttxt[4], g_ttxt[5]);
			if (night == 1) {
				if (speed >= p->route_pos)
					p->route_pos += speed;
				g_group->set_route_pos(p->route_pos);
				travel_stop(p, 1);
				night = 3;
			}
			if ((t >= 0x1a5e0L || t <= 0x7e90L) && p->mode != 2) {
				DLG->message( g_ttxt[6]);
				if (speed >= p->route_pos)
					p->route_pos += speed;
				g_group->set_route_pos(p->route_pos);
				travel_stop(p, 2);
				night = 3;
			}
		}
		if (night == 3) {
			night = 0;
			if (p->hurry == 1)
				travel_speed_normal(p, speed);
			if ((unsigned char)p->hurry == 0xff)
				;
			p->weather = Wx_get(g_weather);
			redraw = 1;
		}
		if (redraw != 0) {
			n_heroes = g_group->get_nr_heroes();
			sick_pts = n_sick = le_max_sum = le_sum = 0;
			awake = g_group->count_check1();
			h = get_first_hero(g_group);
			do {
				if (h != 0) {
					if ((signed char)h->dead || (signed char)h->unconscious) {
						sick_pts += 1;
						n_sick += 1;
					}
					if ((signed char)h->petrified) {
						sick_pts += 2;
						n_sick += 1;
					}
					le_max_sum += h->le_max;
					le_sum += h->le;
				}
				h = get_next_hero(g_group, h);
			} while (get_first_hero(g_group) != h);
			if (g_group->count_check1() == 0)
				g_travel_state = 1;
			else if (n_heroes - n_sick < sick_pts || (sick_pts * 3 > n_heroes && le_sum * 4 < le_max_sum)) {
				DLG->message( g_ttxt[22]);
				travel_stop(p, 1);
			}
			f_road = travel_road_factor(p, p->road);
			f_weather = travel_weather_factor(p, p->weather);
			f_health = travel_health_factor(p, 0);
			speed = f_weather * f_road * f_health / 4;
			if (p->hurry == 1)
				speed = travel_speed_hurry(p, speed, 0);
			p->speed = speed;
			p->status[TST_SPEED] = travel_level(p);
			redraw = 0;
		}
		if (g_travel_state == 5) {
			g_travel_state = 0;
			if (snd_busy(g_snd_fx, 0) == 0 && Wx_get(g_weather) == 4)
				snd_play(g_snd_fx, 0xad);
			travelmap_route(p, way);
			if (p->planned == 1)
				travelmap_draw_path(p, way);
			if (p->route[0] != 0 && g_group->get_turns() == 0) {
				for (ri = 0; p->route[ri] != p->from; ri++) {
					way2 = route_find(p, p->route[ri], p->route[ri + 1]);
					if (g_on_boat != 0 && way2 == 0x17)
						way2 = 0x1d;
					if (g_routes[way2].known == 1) {
						route_load_points(p, way2);
						tmap_plot_points(g_tmap, p->points, 0xfd);
					}
				}
			}
			p->nr_points = route_load_points(p, way);
		}
		if (g_travel_state == 2) {
			pos = travel_resume(p, pos, speed);
			meet_wait = 2;
		}
		if (speed != p->speed)
			speed = p->speed;
		if (p->action == 1) {
			old_pos = pos;
			pos = travel_reverse(p, &p->from, &p->to, speed) - 2;
			if (pos <= 0)
				pos = old_pos;
		}
		if (p->action == 2) {
			route_rebuild(p, *b);
			turned = 1;
			p->action = 0;
			travelmap_route(p, way);
			travelmap_draw_path(p, way);
		}
		if (p->action == 3) {
			pos = last_pt - pos;
			p->ev_half = flag_not(p, p->ev_half);
			swap = flag_not(p, p->ev_end);
			p->ev_end = flag_not(p, p->ev_start);
			p->ev_start = swap;
			turned = 1;
			p->turns += 1;
			g_group->set_turns(p->turns);
			travelmap_route(p, -1);
			p->planned = 0;
			p->action = 0;
			p->nr_points = route_load_points(p, way);
		}
		if (g_group->count_check1() == 0)
			g_travel_state = 1;
	}
	if (!g_travel_state && !p->ev_end && g_routes[way].event >= 50) {
		travel_anim_off(p, 1);
		tevent_run(p, way);
		travel_anim_on(p, 1);
	}
	if (!g_travel_state) {
		old_pos = dist / step * 2;
		if (pos <= old_pos) {
			if (p->to > p->from) {
				wait_for_vsync();
				tmap_follow(g_tmap, p->points[pos], *(p->points + pos + 1), 1, 0xfd);
				tmap_draw(g_tmap);
			} else {
				wait_for_vsync();
				px = *(p->points + p->nr_points - pos);
				py = *(p->points + p->nr_points - pos + 1);
				tmap_follow(g_tmap, px, py, 1, 0xfd);
				tmap_draw(g_tmap);
			}
			pos += 2;
		}
	}
	if (g_routes[way].from == (unsigned char)g_group->route_from)
		p->dir_def = g_routes[way].dir_from;
	else if (g_routes[way].to == (unsigned char)g_group->route_from)
		p->dir_def = g_routes[way].dir_to;
	else
		p->dir_def = 1;
	if (g_slow5_on == 1)
		travel_slow5_off(p, p->speed);
	if (g_slow4_on == 1)
		travel_slow4_off(p, p->speed);
	p->speed = 0;
	p->status[TST_SPEED] = travel_level(p);
	return turned;
}
