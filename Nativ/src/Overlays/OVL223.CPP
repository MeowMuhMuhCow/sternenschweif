/* OVL223 - Overlay O223 (Segment 0x23dc in SCHWEIF.EXE): Route abgehen (Travel aus O038, Route route[40], Ende
 * ROUTE_END). Gerufen von O038 travel_step (travel_walk_planned/travel_walk_simple/place_reveal_dirs) und O067
 * (place_reveal_dirs); route_rebuild von O215 (TRAVEL_ACT_REROUTE), travel_reset_pos von O216.
 * travel_walk_planned(p, v): geplante Route (route_plan) abgehen - Startindex aus from, Schleife Weg fuer Weg
 *   (travel_route mit Zeigern auf v/Ort, Weg route_find, unbekannte Wege (g_routes[r].known == 0) -> route_mark_known
 *   bzw. travelmap_route_part/travelmap_draw_path_to zeichnen), Position in die Gruppe (inline set_route_pos/
 *   _from/_to), Ereignisse place_has_event/shrine_donation, Abzweigungsfrage dlg_choose (Texte g_ttxt 27..34), bei
 *   "Nein" route_plan_simple + travel_walk_simple; planned = 1 waehrend des Laufs; Abschluss travel_level nach
 *   status[TST_SPEED], travel_redraw, travelmap_route/tmap_draw.
 * travel_walk_simple(p, v): einfache Route (route_plan_simple) bis zum Ende abgehen (Sonderfall Ort 0x56/0x57 mit
 *   Frage g_ttxt[39], Ende am Ort 0xab).
 * travel_reset_pos(p): route_pos = 0, auch in der Gruppe. route_rebuild(p, v): Route ab Ort v neu aufbauen (Puffer
 *   40 Bytes, solange place_count_all_dirs == 2 und Ort > 0x20), sonst Meldung g_ttxt3[60]. place_reveal_dirs(p, id):
 *   Ort id in g_places: alle Richtungen aus dirs_all nach dirs uebernehmen, Nordwest (d7) fuer Ort 0x56 nur bei
 *   g_artherion_hut_known == 1. place_reveal_dir(p, id, k): dasselbe nur fuer Richtung k (switch 0..7). route_mark_known(p, a, b):
 *   Weg a-b bzw. b-a in g_routes suchen, als bekannt markieren (known = 1), Richtung am Ort b (dir_to bzw. dir_from)
 *   -> place_reveal_dir.
 *
 * BCC -mlarge -3 -Ot -Og -c -Y OVL223.CPP   -> 6267/6267 Bytes GLEICH (18.10.2026), cobjfix laeuft.
 *   (Namensfassung mit schweif.h: 19.10.2026 GLEICH, LINKGLEICH)
 * - Gruppen-Getter get_turns() (wie O213): turns-Store, "& 1" -> test eax,1, "== 0" -> cmp eax,0. Setter
 *   set_route_pos/_from/_to mit Parameterkopien ([bp-13h] ...), set_route_pos mit long-Kopie.
 * - p->route[i - 1] -> es:[bx+0cfh] (unsigned char i: Index zuerst, -1 in die Verschiebung); p->route[39] = +f7.
 * - Zeiger auf v und den Ort als initialisierte Locals (unsigned char far *pv = &v, *pl = &loc) - Reihenfolge
 *   loc, pv, pl kommt aus der Deklarations-Initialisierung, danach i = 0 / st = 1 als Anweisungen.
 * - "place_unloads_ui(p) == 0" -> or al,al (mit ! -> cbw / or ax,ax); "!place_has_event(..)" am Ende von
 *   travel_walk_simple -> cbw.
 * - travel_walk_simple: "jg short / jmp short" bzw. "jz short / jmp short" vor der Meldung g_ttxt[35] = zwei
 *   "if (..) goto next;" (Sprung ueber Sprung bleibt bei goto stehen); als &&-Kette direkt jng/jnz.
 * - place_reveal_dir: id NICHT in CL (in place_reveal_dirs schon) -> "#pragma option -rd" vor der Funktion
 *   (alternativ volatile id); jeder case endet mit return (Epilogkopie pop bp / retf, auch fuer den if-Sprung),
 *   default = eigener Epilog.
 * - route_mark_known: "register unsigned short i = 0;" zwischen found/k (Reihenfolge der Initialisierung), Schleife
 *   "for (; i < NR_ROUTES && (from != a || to != b); i++);" (Test oben), Index*10 als -Og-Zwischenwert [bp-4].
 * - place_reveal_dirs: id in CL, i in DX (keine Aufrufe), Bitfelder "unsigned short b:1" in 1-Byte-struct (PLACES,
 *   struct dir_set; Test and ax,1 / cmp ax,1, Setzen or byte, Loeschen and byte 7fh).
 * - Gruppe: route_from/route_to sind signed char - im Vergleich mit route[i] (unsigned char) Cast (unsigned char);
 *   place_count_dirs/_all_dirs liefern unsigned char, der direkte Vergleich "> 2" braucht (signed char); shrine_donation
 *   hat keinen Parameter, das Original uebergibt p (Aufruf ueber gecasteten Funktionszeiger).
 * Auffaelliges im Original: travel_walk_planned - "else if (route[i] >= 0x14 && .. && get_turns() == 0)" steht im
 *   else-Zweig von "if (get_turns() == 0)" (Bedingung dort nie wahr, Meldung g_ttxt[30] im ersten Zweig unerreichbar);
 *   travel_walk_simple - r aus dem Vorlauf (route_find(p, route[0], route[1])) wird nie benutzt; route_rebuild/
 *   place_reveal_dirs/place_reveal_dir benutzen p nicht.
 */
#include "schweif.h"

#define PLACES ((struct dir_set far *)g_places)	/* Ortstabelle mit Bitsicht der Richtungen */

void travel_walk_planned(Travel far *p, unsigned char v)
{
	signed char ndirs;
	signed char nall;
	signed char turned;
	signed char ans;
	signed char known;
	unsigned char next;
	unsigned char dest = p->to;
	unsigned char i;
	unsigned char state;
	unsigned char mark;
	unsigned char far *pv = &v;
	unsigned char far *pl = &dest;
	signed short r;

	i = 0;
	state = 1;
	p->planned = 1;
	p->turns = g_group->get_turns();
	if (g_travel_state == 4) {
		if (place_unloads_ui(p)) {
			while (place_event(p) == 1) {
				travel_follow_route(p);
				state = 0xff;
			}
		} else
			g_travel_state = 0;
	}
	if (state != 0xff) {
		while (p->route[i] != p->from)
			i++;
		i += 1;
		if (p->route[i] == ROUTE_END) {
			travel_set_map_pos(p, 0);
			state = 0xff;
		} else if (place_count_all_dirs(p, p->from) == 2 && i == 1) {
			g_group->set_route_from(v);
			g_group->set_route_to(p->route[i]);
			travel_set_map_pos(p, 0);
		}
	}
	p->speed = 0;
	travel_anim_on(p, 1);
	while (state != 0xff && p->planned == 1 && ((!g_travel_state && (v > 0x14 || i == 1)) || g_travel_state == 6)) {
		if (g_travel_state == 6 && !g_group->loctype && g_group->town == 0)
			g_travel_state = 0;
		v = p->route[i - 1];
		dest = p->route[i];
		mark = p->route[39];
		if (i == 1)
			travel_set_map_pos(p, 0);
		turned = travel_route(p, pv, pl);
		if (g_travel_state == 0 || g_travel_state == 6) {
			if (turned == 1) {
				for (i = 0; p->route[i] != (unsigned char)g_group->route_to; i++)
					;
			}
			r = route_find(p, v, dest);
			if (r == ROUTE_NONE) {
				dlg_message(g_dialog, g_ttxt[27]);
				state = 0xff;
			} else {
				known = g_routes[r].known;
				if (known == 0) {
					if (!(g_group->get_turns() & 1)) {
						route_mark_known(p, v, dest);
					} else if (p->to == dest) {
						travelmap_route_part(p, dest, v, 300, p->turn_pos, 0xff, 0);
					} else {
						travelmap_route_part(p, v, dest, 300, p->turn_pos, 0xff, 0);
					}
					travelmap_draw_path_to(p, mark == 0 ? 0xff : mark);
				}
				v = g_group->route_from;
				dest = g_group->route_to;
				p->turn_pos = g_group->turn_pos;
				travel_reset_pos(p);
				g_group->set_route_pos(p->route_pos);
				g_group->set_route_from(p->to);
				g_group->set_route_to(p->to);
				p->prev_place = p->from;
				p->from = p->to;
				v = dest;
				ndirs = place_count_dirs(p, v);
				nall = place_count_all_dirs(p, v);
				next = p->route[i];
				if (g_travel_state == 6) {
					g_travel_state = 0;
				} else {
					if (v == 7 || v == 9 || place_has_event(p, p->route[i]) == 1)
						;
					else if (place_unloads_ui(p)) {
						g_travel_state = 0;
						while (place_event(p) == 1)
							travel_follow_route(p);
					}
					if (g_travel_state > 1 && g_travel_state != 4)
						g_travel_state = 0;
					p->from = g_group->route_from;
					p->to = g_group->route_to;
				}
				if (next <= 0x14 && next != 0x11 && next != 0x12 && next != 7 && next != 9) {
					next = dlg_yes_no(g_dialog, g_ttxt[28]);
					if (next == 1) {
						state = 0xff;
						p->rest = 1;
						travel_reset_pos(p);
					}
				}
				if (place_has_event(p, p->route[i]) == 1) {
					next = ((signed char (*)(void far *))shrine_donation)(p);
					if (next == 1) {
						state = 0xff;
						p->rest = 1;
						travel_reset_pos(p);
					}
				}
				if (p->rest != 1 && g_travel_state != 4) {
					if (nall == ndirs && p->route[i] != ROUTE_END) {
						if (nall > 2 && p->route[i + 1] != ROUTE_END && p->route[i] > 0x20) {
							if (g_group->get_turns() == 0) {
								dlg_message(g_dialog, g_ttxt[29]);
							} else if (p->route[i] >= 0x14 && place_has_event(p, p->route[i]) != 1 &&
								place_unloads_ui(p) == 0 && g_group->get_turns() == 0) {
								dlg_message(g_dialog, g_ttxt[30]);
							}
						}
						i++;
						state = p->route[i];
					} else {
						place_reveal_dirs(p, v);
						if (p->route[i + 1] != ROUTE_END) {
							if ((p->route[i] != 0x56 || g_artherion_hut_known != 0) && (signed char)place_count_all_dirs(p, p->route[i]) > 2) {
								ans = dlg_choose(g_dialog, g_ttxt[31], 2, g_ttxt[32], g_ttxt[33]);
								if (ans == 1) {
									i++;
									state = p->route[i];
								} else {
									state = 0xff;
									route_clear(p);
									travelmap_route(p, -1);
									tmap_draw(g_tmap);
									route_plan_simple(p, p->from);
									travel_walk_simple(p, p->from);
								}
							} else {
								i++;
								state = p->route[i];
							}
						} else {
							if (g_group->get_turns() == 0 && p->route[i] > 0x14 && place_unloads_ui(p) == 0)
								dlg_message(g_dialog, g_ttxt[34]);
							i++;
							state = p->route[i];
						}
					}
				} else if (nall == 1 && p->route[i + 1] == ROUTE_END && p->rest != 1 && g_travel_state != 4) {
					if (p->route[i] >= 0x14 && place_has_event(p, p->route[i]) != 1 &&
						place_unloads_ui(p) == 0 && g_group->get_turns() == 0)
						dlg_message(g_dialog, g_ttxt[30]);
				} else if (g_travel_state != 4) {
					place_reveal_dirs(p, v);
					if (p->route[i + 1] != ROUTE_END) {
						if ((p->route[i] != 0x56 || g_artherion_hut_known != 0) && (signed char)place_count_all_dirs(p, p->route[i]) > 2) {
							ans = dlg_choose(g_dialog, g_ttxt[31], 2, g_ttxt[32], g_ttxt[33]);
							if (ans == 1) {
								i++;
								state = p->route[i];
							} else {
								state = 0xff;
								route_clear(p);
								travelmap_route(p, -1);
								tmap_draw(g_tmap);
								route_plan_simple(p, p->from);
								travel_walk_simple(p, p->from);
							}
						} else {
							i++;
							state = p->route[i];
						}
					} else {
						if (g_group->get_turns() == 0 && p->route[i] > 0x14 && place_unloads_ui(p) == 0)
							dlg_message(g_dialog, g_ttxt[34]);
						i++;
						state = p->route[i];
					}
				}
			}
		} else
			state = 0xff;
	}
	p->speed = 0;
	p->status[3] = travel_level(p);
	if (p->rest == 1)
		route_clear(p);
	if (g_travel_state != 1)
		travel_redraw(p);
	if (!g_travel_state && g_travel_state != 4)
		g_group->turns = 0;
	if (g_travel_state == 6) {
		g_travel_state = 0;
		return;
	}
	if (g_travel_state == 0 || g_travel_state > 4) {
		travelmap_route(p, -1);
		tmap_draw(g_tmap);
		p->planned = 0;
	}
}

void travel_walk_simple(Travel far *p, unsigned char v)
{
	signed char nall;
	signed char turned;
	signed char known;
	signed char ans;
	unsigned char dest = p->to;
	unsigned char i;
	unsigned char j;
	unsigned char state;
	unsigned char next;
	signed short r;
	signed short old;
	unsigned char far *pv = &v;
	unsigned char far *pl = &dest;

	state = 1;
	if (g_travel_state == 4) {
		if (place_unloads_ui(p)) {
			while (place_event(p) == 1) {
				travel_follow_route(p);
				state = 0xff;
			}
		} else
			g_travel_state = 0;
	}
	if (state != 0xff) {
		for (i = 0; p->route[i] != p->from; i++)
			;
		i += 1;
		p->turns = g_group->get_turns();
		if (p->route[i] == ROUTE_END)
			state = 0xff;
		else
			r = route_find(p, p->route[0], p->route[1]);
	}
	p->speed = 0;
	travel_anim_on(p, 1);
	while (state != 0xff && !g_travel_state) {
		v = p->route[i - 1];
		dest = p->route[i];
		turned = travel_route(p, pv, pl);
		if (g_travel_state == 0) {
			if (turned == 1) {
				for (i = 0; p->route[i] != (unsigned char)g_group->route_to; i++)
					;
			}
			r = route_find(p, v, dest);
			if (r != ROUTE_NONE) {
				known = g_routes[r].known;
				if (known == 0) {
					if (!(g_group->get_turns() & 1)) {
						route_mark_known(p, v, dest);
					} else if (p->to == dest) {
						travelmap_route_part(p, dest, v, 300, p->turn_pos, 0xff, 0);
					} else {
						travelmap_route_part(p, v, dest, 300, p->turn_pos, 0xff, 0);
					}
				}
			}
			v = g_group->route_from;
			dest = g_group->route_to;
			p->turn_pos = g_group->turn_pos;
			travel_reset_pos(p);
			g_group->set_route_pos(p->route_pos);
			g_group->set_route_from(p->to);
			g_group->set_route_to(p->to);
			p->prev_place = p->from;
			p->from = p->to;
			v = dest;
			place_reveal_dirs(p, v);
			nall = place_count_all_dirs(p, v);
			if (p->route[i] == 0x56 && g_artherion_hut_known != 0 && g_artherion_heard != 0) {
				old = dlg_set16(g_dialog, 5);
				ans = dlg_yes_no(g_dialog, g_ttxt[39]);
				dlg_set16(g_dialog, old);
				if (ans == 1) {
					route_clear(p);
					p->route[0] = 0x56;
					p->route[1] = 0x57;
					p->route[2] = ROUTE_END;
					i = 0;
				} else {
					for (j = 0; p->route[j] != 0x56 && p->route[j] != ROUTE_END; j++)
						;
					if (p->route[j] == 0x56) {
						next = p->route[j + 1];
						route_clear(p);
						p->route[0] = 0x56;
						p->route[1] = next;
						p->route[2] = ROUTE_END;
						i = 0;
					}
				}
				g_artherion_heard = 0;
			}
			next = p->route[i];
			if (next <= 0x14 && next != 0x11 && next != 0x12 && next != 7 && next != 9) {
				next = dlg_yes_no(g_dialog, g_ttxt[28]);
				if (next == 1) {
					state = 0xff;
					p->rest = 1;
					travel_reset_pos(p);
				}
			}
			if (place_has_event(p, p->route[i]) == 1) {
				next = ((signed char (*)(void far *))shrine_donation)(p);
				if (next == 1) {
					state = 0xff;
					p->rest = 1;
					travel_reset_pos(p);
				}
			}
			if (p->route[i] == 0xab)
				state = 0xff;
			if (p->rest != 1) {
				if (nall == 1 && p->route[i + 1] == ROUTE_END) {
					if (p->route[i] >= 0x14 && place_has_event(p, p->route[i]) != 1 && place_unloads_ui(p) == 0)
						dlg_message(g_dialog, g_ttxt[35]);
					i++;
					state = p->route[i];
				} else if (p->route[i] != ROUTE_END) {
					if (nall <= 2)
						goto next;
					if (p->route[i + 1] != ROUTE_END)
						goto next;
					if (p->route[i] >= 0x14 && place_has_event(p, p->route[i]) != 1 && place_unloads_ui(p) == 0)
						dlg_message(g_dialog, g_ttxt[35]);
next:
					i++;
					state = p->route[i];
				}
			}
		} else
			state = 0xff;
	}
	p->speed = 0;
	p->status[3] = travel_level(p);
	if (g_travel_state != 1)
		travel_redraw(p);
	if (g_travel_state == 0) {
		p->from = g_group->route_from;
		p->to = g_group->route_to;
		g_group->route_pos = 0;
		p->route_pos = 0;
		if (place_unloads_ui(p) && p->rest != 1 && p->from != 7 && p->from != 9 && !place_has_event(p, p->from)) {
			while (place_event(p) == 1)
				travel_follow_route(p);
		}
	}
	if (!g_travel_state && g_travel_state != 4)
		g_group->turns = 0;
	if (g_travel_state != 3 && g_travel_state != 1) {
		travelmap_route(p, -1);
		tmap_draw(g_tmap);
		p->planned = 0;
	}
	if (g_travel_state == 6)
		g_travel_state = 0;
}

void travel_reset_pos(Travel far *p)
{
	p->route_pos = 0;
	g_group->set_route_pos(p->route_pos);
}

void route_rebuild(Travel far *p, unsigned char v)
{
	unsigned char i = 0;
	signed char ndirs = 0;
	signed char n;
	unsigned char buf[40];

	for (n = 0; n < 40; n++)
		buf[n] = 0;
	while (p->route[i] != v && p->route[i] != ROUTE_END)
		i++;
	if (p->route[i] == ROUTE_END) {
		dlg_message(g_dialog, g_ttxt3[60]);
		return;
	}
	n = 0;
	buf[n] = p->from;
	n++;
	ndirs = place_count_all_dirs(p, v);
	while (ndirs == 2 && v > 0x20) {
		buf[n] = v;
		n++;
		i++;
		v = p->route[i];
		ndirs = place_count_dirs(p, v);
	}
	buf[n] = v;
	buf[n + 1] = ROUTE_END;
	for (n = 0; n < 40; n++)
		p->route[n] = buf[n];
}

void place_reveal_dirs(Travel far *p, unsigned char id)
{
	register signed short i;

	for (i = 0; PLACES[i].id != id; i++)
		;
	if (PLACES[i].dirs_all.d0 == 1)
		PLACES[i].dirs.d0 = 1;
	if (PLACES[i].dirs_all.d1 == 1)
		PLACES[i].dirs.d1 = 1;
	if (PLACES[i].dirs_all.d2 == 1)
		PLACES[i].dirs.d2 = 1;
	if (PLACES[i].dirs_all.d3 == 1)
		PLACES[i].dirs.d3 = 1;
	if (PLACES[i].dirs_all.d4 == 1)
		PLACES[i].dirs.d4 = 1;
	if (PLACES[i].dirs_all.d5 == 1)
		PLACES[i].dirs.d5 = 1;
	if (PLACES[i].dirs_all.d6 == 1)
		PLACES[i].dirs.d6 = 1;
	if (PLACES[i].dirs_all.d7 == 1)
		PLACES[i].dirs.d7 = 1;
	if (id == 0x56 && g_artherion_hut_known != 1)
		PLACES[i].dirs.d7 = 0;
}

#pragma option -rd
void place_reveal_dir(Travel far *p, unsigned char id, signed char k)
{
	register signed short i;

	for (i = 0; PLACES[i].id != id; i++)
		;
	switch (k) {
	case 0:
		if (PLACES[i].dirs_all.d0 == 1)
			PLACES[i].dirs.d0 = 1;
		return;
	case 1:
		if (PLACES[i].dirs_all.d1 == 1)
			PLACES[i].dirs.d1 = 1;
		return;
	case 2:
		if (PLACES[i].dirs_all.d2 == 1)
			PLACES[i].dirs.d2 = 1;
		return;
	case 3:
		if (PLACES[i].dirs_all.d3 == 1)
			PLACES[i].dirs.d3 = 1;
		return;
	case 4:
		if (PLACES[i].dirs_all.d4 == 1)
			PLACES[i].dirs.d4 = 1;
		return;
	case 5:
		if (PLACES[i].dirs_all.d5 == 1)
			PLACES[i].dirs.d5 = 1;
		return;
	case 6:
		if (PLACES[i].dirs_all.d6 == 1)
			PLACES[i].dirs.d6 = 1;
		return;
	case 7:
		if (PLACES[i].dirs_all.d7 == 1)
			PLACES[i].dirs.d7 = 1;
		return;
	}
}

void route_mark_known(Travel far *p, unsigned char from, unsigned char to)
{
	signed char found = 0;
	register unsigned short i = 0;
	signed char dir = 0;

	for (; i < NR_ROUTES && (g_routes[i].from != from || g_routes[i].to != to); i++)
		;
	if (g_routes[i].from == from && g_routes[i].to == to) {
		found = 1;
		g_routes[i].known = 1;
		dir = g_routes[i].dir_to;
	}
	if (found == 0) {
		for (i = 0; i < NR_ROUTES && (g_routes[i].from != to || g_routes[i].to != from); i++)
			;
		if (g_routes[i].from == to && g_routes[i].to == from) {
			found = 1;
			g_routes[i].known = 1;
			dir = g_routes[i].dir_from;
		}
	}
	place_reveal_dir(p, to, dir);
}
