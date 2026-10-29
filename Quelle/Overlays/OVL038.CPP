/* OVL038 - Overlay O038 (Segment 0x1fda in SCHWEIF.EXE): Reiseobjekt der Reisekarte, class Travel (s_class.h; 0x11c
 * Bytes, zwei Bmp-Elemente rose_bg/rose +a9/+b3), global ds:c0fb (R01 main_loop ruft travel_main). Die Reisefunktionen
 * O209..O223 und die Ereignisse O164 bekommen das Objekt als erstes Argument (TRAVEL_PTR).
 * Travel(): Konstruktor (Startwerte, Weg from/to = 1/0x5c, weather = Wx_get(g_weather), Route route[40] leeren,
 *   g_tev_buf = 0). ~Travel(): Destruktor (Bmp-Elemente, delete bei Flag 1).
 * travel_enter(p): Reisekarte aufbauen - Musik/Bilder (load_pic_list(g_cd_cache, ..), Klang), Bildspeicher
 *   g_tmap_buf (0x11e30), Texte g_ttxt/g_ttxt2/g_ttxt3 (txt_load_list), Ort- und Wegtabelle 0xa1/0xa2 nach g_places/
 *   g_routes, Startweg route_first/route_find, Wegwerte per inline-Setter nach route_len/road/terrain, Ereignistexte
 *   g_tev_txt/g_tev_loc_txt bei g_tev_reload.
 * travel_leave(p): Reisekarte abbauen - Texte frei, Position in die Gruppe (inline set_route_from/_to/_pos), Ort- und
 *   Wegtabelle zurueckschreiben (ofile_*), alles freigeben.
 * travel_main(p, k): Reise (Gruppe group.loctype/town/dungeon per geschachtelten inline-Settern zuruecksetzen,
 *   Startort k oder Weg aus group.route_from/route_to, Route, Ereignisse place_reveal_dirs/shrine_donation, Schleife
 *   ueber travel_step bis Rueckgabe 1 oder g_travel_state != 0/6, Abschluss route_clear/travel_enter_town/
 *   travel_enter_dungeon, travel_leave, bei g_travel_state < 0 travel_arrive).
 * travel_step(p): ein Schritt - Menue dlg_choose (Texte g_ttxt) oder automatisch, switch 1/2/4/3 (einfache/
 *   geplante Route route_plan_simple/route_plan, travel_walk_simple/_planned, Rast p->rest, Ort eingeben dlg_input), Sonderfall g_travel_state == -7.
 * travel_arrive(p): Ortswechsel je nach g_travel_state (-1..-6: Stadt bzw. Dungeon, Startfeld group_set_screen_pos,
 *   Tagebuch diary_add).
 * route_load_points(p, n): Eintrag n der Datei 0x93 (Offsettabelle) nach p->points laden (arc_*), Laenge in
 *   g_points_len, liefert / 2.
 *
 * BCC -mlarge -3 -Ot -Og -c -Y OVL038.CPP   -> 5548/5548 Bytes GLEICH (18.10.2026), cobjfix laeuft
 *   (mit -Oi ebenfalls gleich, ohne -Og 26 Bytes zu lang).
 *   (Namensfassung mit schweif.h: 19.10.2026 GLEICH, LINKGLEICH)
 * - C++: Konstruktor/Destruktor mit Bmp-Elementen (gb_new0 bzw. gb_delete(.., 2), this == 0 -> mem_alloc_w(0x11c),
 *   delete mem_free_v) erzeugt BCC selbst; Rumpf nur Zuweisungen, Ketten rechts nach links
 *   ("route_len = route_pos = speed = 0" -> dword, dword, al).
 * - inline-Setter mit Parameterkopie: Travel::set_route_len/set_road/set_terrain, group::set_route_from/_to/_pos
 *   (long -> mov eax); geschachtelt set_loctype(set_loctype(0)) / set_town(set_town(0)) / set_dungeon(..) wie
 *   O102/O217 (loses cbw bei set_loctype); "if (g->get_loctype() == 8)" ueber inline int-Getter (-Og-Byte [bp-11h]
 *   mit set_loctype geteilt).
 * - Register: travel_enter Wegnummer e OHNE register (DI), -Og-Offset g_routes[e] in SI; route_load_points
 *   Parameter n MIT register (SI), len ohne (DI) - jeweils umgekehrt vertauscht BCC SI/DI.
 * - travel_step: Sprungtabelle verraet die Rumpfreihenfolge case 1, 2, 4, 3; travel_arrive: case -1 .. -6 absteigend.
 * - travel_step/travel_arrive liegen unter travel_main -> far call (travel_enter/travel_leave darueber -> push cs/
 *   call near). Prototypen aus s_proto.h (TRAVEL_PTR).
 * Originalfehler: travel_step - Rueckgabe ret nur bei rest == 1 gesetzt, sonst uninitialisiert (travel_main prueft
 *   "j != 1" auf diesen Wert); travel_main - "set_route_to(p->from)" statt to (hier harmlos, da from == to == k).
 */
#include "schweif.h"

/* Phase B: eigene Daten des Moduls (ds:3e86..3f5e, dsana) - initialisierte Daten in Adressreihenfolge */
char g_tmap_areas[100] = {
	0xf0, 0x00, 0x02, 0x00, 0x3f, 0x01, 0x1a, 0x00, 0xfd, 0x00, 0xf4, 0x00, 0x1e, 0x00, 0x3b, 0x01,
	0x33, 0x00, 0xe3, 0x00, 0xf4, 0x00, 0x38, 0x00, 0x3b, 0x01, 0x55, 0x00, 0xfa, 0x00, 0x0e, 0x01,
	0x74, 0x00, 0x1e, 0x01, 0x84, 0x00, 0xf8, 0x00, 0xf4, 0x00, 0x56, 0x00, 0x3b, 0x01, 0x84, 0x00,
	0xf9, 0x00, 0xf4, 0x00, 0x89, 0x00, 0x3b, 0x01, 0xa9, 0x00, 0xfb, 0x00, 0xf1, 0x00, 0xaf, 0x00,
	0x08, 0x01, 0xc6, 0x00, 0xf5, 0x00, 0x0c, 0x01, 0xaf, 0x00, 0x23, 0x01, 0xc6, 0x00, 0xf6, 0x00,
	0x27, 0x01, 0xaf, 0x00, 0x3e, 0x01, 0xc6, 0x00, 0xf7, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff
};
char g_cl_travel[102] = {
	0x92, 0x20, 0xa1, 0x20, 0xa2, 0x20, 0xa4, 0x20, 0xad, 0x20, 0x93, 0x20, 0x94, 0x20, 0x95, 0x20,
	0x97, 0x20, 0x98, 0x20, 0x99, 0x20, 0xa3, 0x20, 0xa0, 0x20, 0xa5, 0x20, 0xa6, 0x20, 0xa7, 0x20,
	0xa8, 0x20, 0xa3, 0x21, 0x3e, 0x21, 0xab, 0x20, 0x3f, 0x21, 0x40, 0x21, 0x41, 0x21, 0x42, 0x21,
	0x43, 0x21, 0xac, 0x20, 0x44, 0x21, 0x45, 0x21, 0x96, 0x21, 0x98, 0x21, 0x97, 0x21, 0xd5, 0x21,
	0xd0, 0x21, 0xd2, 0x21, 0xd6, 0x21, 0xd7, 0x21, 0xd4, 0x21, 0xc8, 0x21, 0x93, 0x21, 0x99, 0x21,
	0xc9, 0x21, 0xca, 0x21, 0xcb, 0x21, 0xce, 0x21, 0xaa, 0x20, 0x04, 0x22, 0x71, 0x40, 0xa1, 0x40,
	0xad, 0x40, 0xae, 0x40, 0xff, 0xff
};
char g_cl_travel_st[14] = "";
/* Phase B: eigenes BSS (ds:c7a4..c7de) in Adressreihenfolge */
void far *g_ttxt2_buf;
char far * far *g_ttxt2;
void far *g_ttxt3_buf;
char far * far *g_ttxt3;
void far *g_tev_buf;
char far * far *g_tev_txt;
void far *g_tev_loc_buf;
char far * far *g_tev_loc_txt;
void far *g_ttxt_buf;
char far * far *g_ttxt;
struct travel_place far *g_places;
struct route far *g_routes;
struct loot_item far *g_tev049_loot;
signed short g_points_route;
signed short g_tev_reload;
signed short g_points_len;

Travel::Travel()
{
	register signed short i;

	reenter = 0;
	turn_pos = 0;
	f100 = 0;
	from = 1;
	to = 0x5c;
	prev_place = 0;
	action = 0;
	turns = 0;
	planned = 0;
	mode = 0;
	rest = 0;
	road = 1;
	terrain = 1;
	route_len = route_pos = speed = 0;
	hurry = 0;
	weather = Wx_get(g_weather);
	danger = 0;
	resume_10 = resume_50 = resume_90 = 0;
	for (i = 0; i < 40; i++)
		route[i] = 0;
	g_tev_buf = 0;
	g_tev_reload = 0;
}

Travel::~Travel()
{
}

void travel_enter(Travel far *p)
{
	signed long len_places;
	signed long len_routes;
	signed short e;

	g_points_route = -1;
	g_pp20_index = PP20_TRAVELMAP;
	load_pic_list(g_cd_cache, g_cl_travel, g_cl_travel_st, 1, 0);
	if (Sound_mtype(g_snd_music) == 1 || Sound_mtype(g_snd_music) == 2)
		snd_play(g_snd_music, 0x23);
	else
		snd_play(g_snd_music, 0x16);
	Sound_volume3(g_snd_music, g_vol_music, 0x7d0, 0);
	if (p->reenter != 3) {
		p->saved_areas = g_mouse_areas;
		g_mouse_areas = g_tmap_areas;
		if (p->reenter == 0)
			p->shown[0] = p->shown[1] = p->shown[2] = p->shown[3] = -1;
		if (p->sub_scene == 0)
			tmap_new_pal(g_tmap);
		g_tmap_ptr = g_tmap_buf = mem_alloc(0x11e30L);
		tmap_load(g_tmap, 0x92);
		if (p->sub_scene == 0) {
			travel_nop_o213(p);
			rose_alloc(p);
			txt_load_list(g_font, 0xa4, 0x55, &g_ttxt_buf, &g_ttxt);
			txt_load_list(g_font, 0xad, 0x51, &g_ttxt2_buf, &g_ttxt2);
			txt_load_list(g_font, 0x1f1, 0x5e, &g_ttxt3_buf, &g_ttxt3);
		}
		p->points = (signed short far *)mem_alloc(0x1f4L);
		len_routes = file_length(g_file_main, 0xa2);
		len_places = file_length(g_file_main, 0xa1);
		g_places = (struct travel_place far *)mem_alloc(len_places);
		ofile_load(g_ofile, 0xa1, g_places);
		g_routes = (struct route far *)mem_alloc(len_routes);
		ofile_load(g_ofile, 0xa2, g_routes);
	}
	if (g_group->route_from != 0 && g_group->route_to != 0) {
		p->from = g_group->route_from;
		p->to = g_group->route_to;
	}
	if (p->from == p->to)
		e = route_first(p, g_group->route_from);
	else
		e = route_find(p, g_group->route_from, g_group->route_to);
	p->set_route_len(g_routes[e].len);
	p->set_road(g_routes[e].road);
	p->set_terrain(g_routes[e].terrain);
	clear_screen_buf(g_vga_buf);
	if (p->reenter != 3) {
		tmap_set_pal(g_tmap);
		travel_set_map_pos(p, 0);
	}
	if (p->rest == 1)
		travelmap_route(p, -1);
	if (g_travel_state != 1) {
		if (p->reenter == 0) {
			g_refresh = 1;
			travelmap_draw(p);
		} else {
			tmap_draw(g_tmap);
		}
	}
	p->rose_x = p->rose_y = p->old_rose_x = p->old_rose_y = 1;
	if (g_tev_reload != 0) {
		txt_load_list(g_font, 0x13e, 0x8c, &g_tev_buf, &g_tev_txt);
		txt_load_list(g_font, p->evtxt_file, p->evtxt_n, &g_tev_loc_buf, &g_tev_loc_txt);
		g_tev_reload = 0;
	}
}

void travel_leave(Travel far *p)
{
	signed long len;

	g_mouse_areas = p->saved_areas;
	if (g_tev_buf != 0) {
		g_tev_reload = 1;
		g_tev_buf = mem_free(g_tev_buf);
		mem_free(g_tev_txt);
		mem_free(g_tev_loc_buf);
		mem_free(g_tev_loc_txt);
	}
	if (g_travel_state == 1 && !group_get_on_route(g_group)) {
		p->mode = 1;
		g_group->set_route_from(p->from);
		g_group->set_route_to(p->to);
		g_group->set_route_pos(p->route_pos);
	}
	if (p->reenter != 3) {
		if (p->sub_scene == 0)
			tmap_free_pal(g_tmap);
		mem_free(g_tmap_buf);
		len = ofile_length_id(g_ofile, 0xa1);
		ofile_create(g_ofile, 0xa1, 4);
		ofile_write(g_ofile, g_places, len);
		ofile_close(g_ofile);
		len = ofile_length_id(g_ofile, 0xa2);
		ofile_create(g_ofile, 0xa2, 4);
		ofile_write(g_ofile, g_routes, len);
		ofile_close(g_ofile);
		mem_free(g_places);
		mem_free(g_routes);
		mem_free(p->points);
		if (p->sub_scene == 0) {
			mem_free(g_ttxt2_buf);
			mem_free(g_ttxt2);
			mem_free(g_ttxt_buf);
			mem_free(g_ttxt);
			mem_free(g_ttxt3_buf);
			mem_free(g_ttxt3);
			travel_anim_off_leave(p);
			rose_free(p);
		}
		Sound_volume3(g_snd_music, 0, 0x7d0, 0);
		g_refresh = 1;
	}
	snd_obj_stop(g_snd_music);
	snd_obj_stop(g_snd_fx);
	Speech_stop(g_snd_speech);
}

void travel_main(Travel far *p, signed char k)
{
	signed char j = 0;
	signed long t;

	if (p->route[0] == 0)
		p->planned = 0;
	ani_stop(g_screen_pic);
	g_group->loctype = g_group->old_loctype;
	g_group->old_loctype = LOC_NONE;
	if (g_group->get_loctype() == LOC_CAMP_WILD)
		g_travel_state = 3;
	g_group->set_loctype(g_group->set_loctype(LOC_NONE));
	g_group->set_town(g_group->set_town(0));
	g_group->set_dungeon(g_group->set_dungeon(0));
	p->status[TST_SPEED] = p->anim_rate = p->anim_tick = p->frame = p->bg_x1 = p->bg_x2 = 0;
	t = g_daytime;
	if (k != 0) {
		p->from = p->to = k;
		g_group->set_route_from(p->from);
		g_group->set_route_to(p->from);
		g_group->route_pos = 0;
		if ((unsigned char)g_group->route_from <= 0x14 && t < 0x1a5e0L && t > 0x93a8L)
			p->rest = 1;
		else
			p->rest = 0;
	} else {
		if ((unsigned char)g_group->route_to == 0xff)
			g_group->set_route_to(g_group->route_from);
		if ((unsigned char)g_group->route_from == 0xff)
			g_group->set_route_from(g_group->route_to);
		p->from = k = g_group->route_from;
		p->to = g_group->route_to;
		for (j = 0; j < 40; j++) {
			if (p->route[j] == p->from)
				break;
		}
		if (p->route[0] == 0 || j >= 39 ||
		    (g_travel_state != 4 && (p->route[j] != p->from || p->route[j + 1] != p->to))) {
			p->route[0] = g_group->route_from;
			p->route[1] = g_group->route_to;
			p->route[2] = 0xff;
		}
		p->mode = 0x0d;
		p->rest = j = 0;
	}
	group_set_on_route(g_group, 1);
	if (g_travel_state == 4)
		g_travel_state = 6;
	else
		g_travel_state = 0;
	mouse_hide(0);
	travel_enter(p);
	mouse_show(0);
	if (k != 0)
		place_reveal_dirs(p, k);
	if (g_group->route_from == g_group->route_to && place_has_event(p, p->from))
		((void (*)(void far *))shrine_donation)(p)	/* Original uebergibt p (Rumpf ohne Parameter) */;
	travel_anim_on(p, 0);
	while (j != 1 && (g_travel_state == 0 || g_travel_state == 6)) {
		if (g_travel_state == 6)
			g_travel_state = 4;
		j = travel_step(p);
	}
	travel_anim_off(p, 0);
	g_group->set_route_from(p->from);
	g_group->set_route_to(p->to);
	if (p->route[1] == 0xff)
		route_clear(p);
	if (!g_travel_state && g_group->route_from == g_group->route_to) {
		group_set_on_route(g_group, 0);
		route_clear(p);
		if ((unsigned char)g_group->route_from <= 0x14 || (unsigned char)g_group->route_from == 0x67)
			travel_enter_town(p, g_group->route_from);
		if (place_has_event(p, g_group->route_from))
			travel_enter_dungeon(p, g_group->route_from);
	}
	mouse_hide(0);
	travel_leave(p);
	mouse_show(0);
	if (g_game_state == 2 || g_travel_state == 1)
		route_clear(p);
	if (g_game_state == 1 || g_travel_state == 1)
		g_hero_died = 1;
	if (!g_travel_state || g_travel_state == 1) {
		if (group_count_check1(g_group))
			group_set_on_route(g_group, 0);
		g_travel_state = 0;
	}
	if (g_travel_state < 0)
		travel_arrive(p);
	p->mode = 0;
}

signed char travel_step(Travel far *p)
{
	signed char r;
	signed char n;
	signed char ret;
	signed char pos;

#if FIX_0153
	/* Fix #153: Rueckgabe vorbelegen (nur rest == 1 setzt sie) */
	ret = 0;
	pos = p->from;
	if (p->action != 4 && g_group->route_pos == 0 && g_travel_state != 4) {
#else
	pos = p->from;
	if (p->action != 4 && g_group->route_pos == 0 && g_travel_state != 4) {
#endif
		do {
			n = 2;
			if (p->rest == 1) {
				n += 1;
				travelmap_route(p, -1);
			}
			r = dlg_choose(g_dialog, g_ttxt[0], n, g_ttxt[2], g_ttxt[1], g_ttxt[0x25]);
		} while (r == -1);
		p->rest = 0;
	} else {
		p->from = pos = g_group->route_from;
		p->to = g_group->route_to;
		if (p->route[0] == 0) {
			p->route[0] = p->from;
			p->route[1] = p->to;
			p->route[2] = 0xff;
		}
		if (p->planned == 1)
			r = 2;
		else
			r = 1;
	}
	switch (r) {
	case 1:
		if (p->action != 4 && g_travel_state == 0 && g_group->route_pos == 0)
			route_plan_simple(p, pos);
		travel_walk_simple(p, pos);
		break;
	case 2:
		if (p->action != 4 && g_travel_state == 0 && g_group->route_pos == 0)
			route_plan(p, pos);
		travel_walk_planned(p, pos);
		break;
	case 4:
		p->rest = 1;
		break;
	case 3:
		p->from = p->to = dlg_input(g_dialog, 3, g_ttxt3[7], 1, 200, p->from);
		g_group->set_route_from(p->from);
		g_group->set_route_to(p->to);
		place_reveal_dirs(p, p->from);
		travel_set_map_pos(p, 0);
		tmap_draw(g_tmap);
		break;
	}
	if (p->from == p->to && (p->from == 7 || p->from == 9))
		place_event(p);
	if (g_travel_state == -7) {
		route_clear(p);
		travelmap_route(p, -1);
		g_group->set_town(0);
		g_group->set_loctype(0);
		g_group->dir = 0;
		g_group->set_dungeon(0);
		g_group->route_from = 0x56;
		g_group->route_to = 0x56;
		g_group->route_pos = 0;
		p->from = pos = p->to = g_group->route_from;
		g_travel_state = 0;
	}
	if (p->rest == 1)
		ret = 1;
	return ret;
}

signed char travel_arrive(Travel far *p)
{
	switch (g_travel_state) {
	case -1:
		g_group->set_town(2);
		g_group->set_loctype(LOC_DATA | 4);
		g_exit_dir = 1;
		g_group->dir = 1;
		group_set_screen_pos(0x0e, 0x0b);
		break;
	case -2:
		g_dng07_was_prisoner = 1;
		g_dng07_prison_event = 1;
		g_group->set_town(0);
		g_group->set_loctype(0);
		g_exit_dir = 1;
		g_group->dir = 1;
		g_group->set_dungeon(7);
		group_set_screen_pos(6, 0x17);
		break;
	case -3:
		g_group->set_town(0);
		g_group->set_loctype(0);
		g_exit_dir = 1;
		g_group->dir = 1;
		g_group->set_dungeon(6);
		group_set_screen_pos(0x1e, 4);
		diary_add(g_diary, g_ttx[0x17e], 0);
		break;
	case -4:
		g_group->set_town(0);
		g_group->set_loctype(0);
		g_exit_dir = 1;
		g_group->dir = 1;
		g_group->set_dungeon(8);
		group_set_screen_pos(2, 0x10);
		diary_add(g_diary, g_ttx[0x17e], 0);
		break;
	case -5:
		g_group->set_town(4);
		g_group->dir = 2;
		group_set_screen_pos(0x1d, 4);
		g_exit_dir = 2;
		g_low_tunnel_arrival = 1;
		if (g_eolan_state >= 3)
			g_group->set_loctype(LOC_SPECIAL);
		loc_load_data(0x2f);
		break;
	case -6:
		g_group->set_town(0);
		g_group->set_loctype(0);
		g_group->dir = 1;
		g_exit_dir = 1;
		g_group->set_dungeon(0x10);
		group_set_screen_pos(1, 3);
		diary_add(g_diary, g_ttx[0x17f], 0);
		break;
	}
	if (g_mem_saving == 1) {
		g_loaded_dungeon = -1;
		g_loaded_town = -1;
	}
	if (g_travel_state > -7)
		group_set_on_route(g_group, 0);
	else
		group_set_on_route(g_group, 1);
	clear_screen_buf(g_vga_buf);
	return p->from;
}

signed short route_load_points(Travel far *p, register signed short n)
{
	signed long a;
	signed long b;
	signed long ra;
	signed long rb;
	signed short len;

	if (g_points_route != n) {
		g_points_route = n;
		arc_open(g_file_3, 0x93);
		arc_seek(g_file_3, (unsigned short)(n * 4), 0);
		arc_read(g_file_3, &ra, 4L);
		arc_read(g_file_3, &rb, 4L);
		a = ra / 2 + 0x41c;
		b = rb / 2 + 0x41c;
		len = b - a;
		arc_seek(g_file_3, a, 0);
		arc_read(g_file_3, p->points, len);
		arc_close(g_file_3);
		g_points_len = len - 8;
	}
	return g_points_len / 2;
}
