/* OVL046 - Overlay O046 (Segment 0x200b in SCHWEIF.EXE): Hauptschleife der 3D-Ansicht fuer Staedte (Gegenstueck zu
 * O014 fuer Dungeons; Stadt typ = group.town, 1..6; Tabelle g_town_view_areas[6] zu 18 Bytes (struct view_area, Index
 * group.town - 1) wie g_dng_view_areas: Karte, Grafik normal/Sparbetrieb, Automap-Notizen/-Kacheln, Palettenbild,
 * Musik).
 * town_main: ein Durchlauf - Musik bei Neuaufbau (g_refresh), bei Stadtwechsel (g_loaded_town) town_cache_files/
 * town_load, Rahmen draw_loc_icons, Zustandswechsel (g_view_swap_pos, g_c6c2), town_update_icons/town_daytime_cells, Tasten
 * 0x4100/0x2004 -> 0x87, 0x4000/0x320d -> 0x84, Menue dlg_choose (Texte g_ttx[427..436]) -> Aktion g_action,
 * town_action, Ereignisse call_handler2 an g_pick_event/g_cell_event (alle 8 Schritte W100 <= 50: Ueberfall
 * town_mugging ausser Stadt 3; W100 <= 2: Zufallsereignis town_random_event statt des Zellereignisses), Position
 * reveal_map(g_map), Blickrichtung group.dir aus g_cam_angle, Begegnungsobjekt Ev (new/delete) bei
 * g_gavron_chase > 0.
 * town_load: Stadt laden (Pic fuer die Palette, g_renderbuf_ptr/g_render_bmp, load_map_all, O096/view_load_objects/
 * mouse_reset, cam_set_from_group, town_make_events, automap_set_town_type). town_automap/automap_view(karte,
 * notizen, kacheln): Automap (O052, Palette g_pal_buf sichern, Schleife bis Taste 0x11b).
 * town_action: Aktionen 0x81..0x87 wie O014 (0x84 Automap, 0x86 Rasten set_loctype(LOC_CAMP_TOWN) bei
 * g_icon_pic_camp == 0x41). town_make_events: Ereignisobjekt Town je Stadt nach g_town_events, Zeiger g_cell_occludes
 * (g_town_occl_04/g_town_occl_05). view_save_frame: vier Rahmenausschnitte sichern. town_update_icons: Anzeigen (andere Gruppe auf
 * dem Feld, g_nr_groups > 1, Feldwert 3/0x17/0x14 je Stadt) -> g_icon_pic_*.
 * cam_set_from_group: Position -> g_view_cell_y/x, g_view_pos_x/g_view_pos_y, Kamera g_cam_x/g_cam_z (can_stand) und
 * g_cam_fx..g_cam_fz (<< 8). town_daytime_cells: Stadt 1/2 Felder je Wochentag/Tageszeit umschalten (town_set_cell,
 * Zustand g_town_cells_state). town_set_cell(x, y, t, flags, rot): Feld der Karte g_cells setzen, cell_forget.
 * town_mugging: Ueberfall - Held nach Geschlecht im Wechsel (g_mugging_state), Bild 0x196/0x0b..0x0d, Texte 0x44,
 * Variante 3 mit Schaden W(min(6, LE-1)) fuer alle (sub_hero_le). town_random_event(n): Zufallsereignisse O239
 * (town_rnd_*) je n, Stadt und Tageszeit. town_cache_files: Dateien je Stadt in den CD-Cache (load_pic_list).
 *
 * BCC -mlarge -3 -Ot -Og -c -Y OVL046.CPP   -> 7022/7022 Bytes GLEICH (18.10.2026), cobjfix laeuft
 *   (Namensfassung mit schweif.h: 19.10.2026 GLEICH, LINKGLEICH)
 * - SEIT WELLE AW ASM-FREI: Adresse ueber typ * 18 + (unsigned)&((struct view_area near *)(unsigned)
 *   (g_town_view_areas - 1))->music (siehe Leitfaden "O014-Muster geloest").
 * - Kein -Oi: memcpy/memset in automap_view sind echte Aufrufe. -Og fuer die Epilogkopien.
 * - town_daytime_cells und town_random_event (zweiter switch): "default: return;" fuer Sprung auf den echten Epilog
 *   plus toten Epilog dahinter, obwohl die Faelle mit break enden (die breaks werden zu Epilogkopien).
 * - town_make_events: case 6 steht im Quelltext vor case 5 (Sprungtabelle). town_mugging: "if (list[i]->sex != 1)
 *   ; else {..}" (jz / jmp short). g_cell_event als long ueber "*(signed long *)&g_cell_event".
 * - Kartenzugriff in town_update_icons/town_set_cell ueber den Bytezeiger "(unsigned char far *)g_cells + (y << 8) +
 *   (x << 2)" wie im Original.
 * - Klasse SaveMenu (s_class.h; 0x6f7 Bytes, erstes Element ein Pic; O107 savegame_menu).
 * - Phase B (19.10.2026): eigene Daten im Quelltext. Stadttabelle g_town_view_areas[6] ab ds:4c37 (der alte Name
 *   g_town_areas, ds:4c25, ist g_town_view_areas - 1; die 18 Bytes davor sind das Ende von g_town_objs_5).
 * Originalfehler/Auffaelliges: town_mugging liest nach den Suchschleifen list[i] auch bei i == n (Eintrag hinter dem
 *   letzten Helden, ggf. Nullzeiger); findet case 0 weder Geschlecht 0 noch 1, bleiben mode und v uninitialisiert.
 */
#include <stdio.h>
#include <mem.h>
#include "schweif.h"

/* Phase B: eigene Daten des Moduls (ds:4a3c..4d38, dsana) - initialisierte Daten in Adressreihenfolge */
char g_town_occl_04[30] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00,
	0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00
};
char g_town_occl_05[30] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00
};
struct view_obj g_town_objs_1[] = {	/* Objekte Stadt 1 (Ende -1; -2: Rest nur ohne Sparbetrieb) */
	{ 0, 0x132 }, { 1, 0x133 }, { 2, 0x24b }, { 3, 0x20e }, { 4, 0x136 }, { 5, 0x137 },
	{ 6, 0x138 }, { 7, 0x139 }, { 8, 0x135 }, { 9, 0x13b }, { 10, 0x213 }, { 11, 0x20f },
	{ 12, 0x214 }, { 13, 0x215 }, { 14, 0x348 }, { 15, 0x134 }, { 16, 0x24c }, { 17, 0x24e },
	{ 18, 0x24f }, { 19, 0x250 }, { 20, 0x40a }, { 21, 0x40b }, { 22, 0x40c }, { 23, 0x24d },
	{ -1, 0 }
};
struct view_obj g_town_objs_2[] = {	/* Objekte Stadt 2 */
	{ 0, 0x132 }, { 1, 0x133 }, { 2, 0x24b }, { 3, 0x20e }, { 4, 0x213 }, { 5, 0x136 },
	{ 6, 0x138 }, { 7, 0x215 }, { 8, 0x139 }, { 9, 0x216 }, { 10, 0x137 }, { 11, 0x20f },
	{ 12, 0x214 }, { 13, 0x217 }, { 14, 0x13a }, { 15, 0x348 }, { 16, 0x34c }, { 17, 0x135 },
	{ 18, 0x134 }, { 19, 0x250 }, { 20, 0x24d }, { 21, 0x24e }, { 22, 0x40c }, { 23, 0x24c },
	{ -2, -2 }, { 24, 0x24f }, { -1, 0 }
};
struct view_obj g_town_objs_6[] = {	/* Objekte Stadt 6 */
	{ 0, 0x132 }, { 1, 0x133 }, { 2, 0x24b }, { 3, 0x20e }, { 4, 0x213 }, { 5, 0x136 },
	{ 6, 0x138 }, { 7, 0x215 }, { 8, 0x139 }, { 9, 0x216 }, { 10, 0x137 }, { 11, 0x20f },
	{ 12, 0x214 }, { 13, 0x13b }, { 14, 0x335 }, { 15, 0x32e }, { 16, 0x348 }, { 17, 0x135 },
	{ 18, 0x13a }, { 19, 0x134 }, { 20, 0x24d }, { 21, 0x40a }, { 22, 0x24e }, { -1, 0 }
};
struct view_obj g_town_objs_3[] = {	/* Objekte Stadt 3 */
	{ 0, 0x132 }, { 1, 0x2ed }, { 2, 0x2ee }, { 3, 0x136 }, { 4, 0x216 }, { 5, 0x20f },
	{ 6, 0x138 }, { 7, 0x2ef }, { 8, 0x2f6 }, { 9, 0x2f7 }, { 10, 0x324 }, { 11, 0x4ba },
	{ 12, 0x4bb }, { -1, 0 }
};
struct view_obj g_town_objs_4[] = {	/* Objekte Stadt 4 */
	{ 0, 0x34a }, { 1, 0x133 }, { 2, 0x24b }, { 3, 0x20e }, { 4, 0x13b }, { 5, 0x137 },
	{ 6, 0x138 }, { 7, 0x215 }, { 8, 0x216 }, { 9, 0x136 }, { 10, 0x249 }, { 11, 0x24a },
	{ 12, 0x32a }, { 13, 0x214 }, { 14, 0x213 }, { 15, 0x134 }, { 16, 0x32b }, { 17, 0x32c },
	{ 18, 0x336 }, { 19, 0x139 }, { 20, 0x218 }, { 21, 0x335 }, { 22, 0x32e }, { 23, 0x34c },
	{ -2, -2 }, { 24, 0x329 }, { 25, 0x32d }, { 26, 0x328 }, { -1, 0 }
};
struct view_obj g_town_objs_5[] = {	/* Objekte Stadt 5 */
	{ 0, 0x34a }, { 1, 0x133 }, { 2, 0x24b }, { 3, 0x20e }, { 4, 0x13b }, { 5, 0x32e },
	{ 6, 0x138 }, { 7, 0x137 }, { 8, 0x213 }, { 9, 0x136 }, { 10, 0x216 }, { 11, 0x214 },
	{ 12, 0x215 }, { 13, 0x218 }, { 14, 0x139 }, { 15, 0x20f }, { 16, 0x249 }, { 17, 0x24a },
	{ 18, 0x335 }, { 19, 0x134 }, { 20, 0x135 }, { 21, 0x34b }, { 22, 0x40f }, { 23, 0x40d },
	{ 24, 0x40e }, { 25, 0x34c }, { -2, -2 }, { 26, 0x32f }, { 27, 0x410 }, { -1, 0 }
};
struct view_area g_town_view_areas[6] = {	/* je Stadt 1..6: Karte, Grafik, Grafik Sparbetrieb, Automap-Notizen, Palettenbild,
					   Automap-Kacheln, Musik, Objektliste */
	{ 0x131, 0x130, 0x21f, 0x225, 0x12f, 0x252, 28, g_town_objs_1 },	/* Stadt 1 */
	{ 0x22d, 0x22e, 0x22f, 0x226, 0x12f, 0x252, 29, g_town_objs_2 },	/* Stadt 2 */
	{ 0x230, 0x231, 0x232, 0x227, 0x12f, 0x255, 30, g_town_objs_3 },	/* Stadt 3 */
	{ 0x233, 0x234, 0x235, 0x228, 0x12f, 0x256, 31, g_town_objs_4 },	/* Stadt 4 */
	{ 0x239, 0x23a, 0x23b, 0x22a, 0x12f, 0x256, 32, g_town_objs_5 },	/* Stadt 5 */
	{ 0x23f, 0x240, 0x241, 0x22c, 0x12f, 0x252, 33, g_town_objs_6 } 	/* Stadt 6 */
};
char g_cl_town[34] = {
	0x2f, 0x21, 0x3d, 0x21, 0x06, 0x22, 0xb4, 0x23, 0x05, 0x24, 0x44, 0x20, 0x6c, 0x23, 0x6d, 0x23,
	0x04, 0x22, 0x1e, 0x22, 0x22, 0x22, 0x06, 0x24, 0x11, 0x24, 0xc6, 0x22, 0xa3, 0x21, 0xc7, 0x22,
	0xff, 0xff
};
char g_cl_town_st[6] = "";
char g_cl_town1[14] = { 0x19, 0x22, 0x31, 0x21, 0x1f, 0x22, 0x25, 0x22, 0x52, 0x22, 0x30, 0x21, 0xff, 0xff };
char g_cl_town1_st[2] = "";
char g_cl_town2[16] = { 0x1a, 0x22, 0x2d, 0x22, 0x2f, 0x22, 0x26, 0x22, 0x52, 0x22, 0x2e, 0x22, 0xab, 0x40, 0xff, 0xff };
char g_cl_town2_st[2] = "";
char g_cl_town3[16] = { 0x1b, 0x22, 0x30, 0x22, 0x32, 0x22, 0x27, 0x22, 0x55, 0x22, 0x31, 0x22, 0xcc, 0x21, 0xff, 0xff };
char g_cl_town3_st[2] = "";
char g_cl_town4[20] = {
	0xe4, 0x22, 0x33, 0x22, 0x35, 0x22, 0x28, 0x22, 0x56, 0x22, 0x34, 0x22, 0xc7, 0x21, 0xd3, 0x21,
	0x69, 0x40, 0xff, 0xff
};
char g_cl_town4_st[4] = "";
char g_cl_town5[14] = { 0x1d, 0x22, 0x39, 0x22, 0x3b, 0x22, 0x2a, 0x22, 0x56, 0x22, 0x3a, 0x22, 0xff, 0xff };
char g_cl_town5_st[2] = "";
char g_cl_town6[14] = { 0x1e, 0x22, 0x3f, 0x22, 0x41, 0x22, 0x2c, 0x22, 0x52, 0x22, 0x40, 0x22, 0xff, 0xff };
char g_cl_town6_st[3] = "";
/* Phase B: eigenes BSS (ds:c7ec..c7fe) in Adressreihenfolge */
signed short g_town_steps;
TOWN_PTR g_town_events;
struct compass far *g_compass;
signed short g_view_pos_x;
char g_unused_c7f8[2];   /* ds:c7f8, unbenutzt */
signed short g_view_pos_y;
char g_unused_c7fc[2];   /* ds:c7fc, unbenutzt */

#define NVF_HEADS ((Nvf far *)g_nvf_heads)	/* NVF-Dateiobjekt (O098) */
#define MAP_GRID ((struct map_grid far *)&g_map_w)	/* Kartenobjekt ds:b656 (struct map_grid; erstes Feld g_map_w) */
/* Tiefe der angeklickten Flaeche (= g_pick_depth); das Original adressiert sie als g_pick_event + 4 */
#define PICK_DEPTH (((signed short *)&g_pick_event)[2])

void town_main(void)
{
	signed short r;
	signed short loaded;
	register signed short typ;
	register signed short redraw;

	redraw = 0;
	typ = g_group->town;
	g_loaded_dungeon = -1;
	if (g_refresh == 1) {
		redraw = 1;
		g_refresh = 0;
		if (g_cd_music == 0)
			{
			snd_play(g_snd_music, *(signed short near *)(typ * 18 + (unsigned)&((struct view_area near *)(unsigned)(g_town_view_areas - 1))->music));
		}
	}
	loaded = 0;
	if (g_loaded_town != typ) {
		if (g_view_active != 0)
			view_leave();
		if (g_cd_music != 0)
			snd_obj_stop(g_snd_music);
		prepare_spell_icons();
		town_cache_files();
		town_load();
		loaded = 1;
		set_palette(g_view_pal, 0, 0x100);
		screen_textbar(g_screen, g_ttx[typ + 355], 0xff);
		cell_event();
	}
	if (g_cd_music != 0)
		{
		snd_play(g_snd_music, *(signed short near *)(typ * 18 + (unsigned)&((struct view_area near *)(unsigned)(g_town_view_areas - 1))->music));
	}
	if (redraw) {
		if (loaded == 0)
			screen_redraw(g_screen);
		draw_loc_icons(g_screen, 7, 0x0c, g_icon_group == 1 ? 0x0f : 0x2d, g_icon_groups == 1 ? 0x1d : 0x44, 0x27, 0x0b,
			g_icon_camp == 1 ? 0x41 : 0x2f, 0x28);
		set_palette(g_view_pal, 0, 0xa0);
		screen_clear_textbar(g_screen);
		screen_textbar(g_screen, g_ttx[typ + 355], 0xff);
		g_view_recam = 1;
		g_cam_angle = (g_group->dir << 8) + 0x100;
		if (g_cd_music != 0)
			{
			snd_play(g_snd_music, *(signed short near *)(typ * 18 + (unsigned)&((struct view_area near *)(unsigned)(g_town_view_areas - 1))->music));
		}
	}
	if (g_view_swap_pos != 0) {
		swap_in_screen();
		if (g_view_swapped) {
			g_area_type = AREA_TOWN;
			g_cam_pitch = 0;
			g_cam_angle = (g_group->dir << 8) + 0x100;
			g_cam_roll = 0;
			g_view_swapped = 0;
			g_view_recam = 1;
		}
		cell_event();
	}
	if (g_c6c2 != 0) {
		g_cam_angle = (g_group->dir << 8) + 0x100;
		g_view_input_on = 1;
		g_c6c2 = 0;
		screen_textbar(g_screen, g_ttx[typ + 355], 0xff);
		install_cursor();
	}
	if (g_view_recam) {
		cam_set_from_group();
		g_view_recam = 0;
	}
	g_view_input_blocked = 0;
	town_update_icons();
	town_daytime_cells();
	handle_input(g_screen);
	if (g_action == 0x4100 || g_action == 0x2004)
		g_action = 0x87;
	else if (g_action == 0x4000 || g_action == 0x320d)
		g_action = 0x84;
	if (g_mouse_rclick != 0 || g_key == 0x4900) {
		signed short sel;

		sel = dlg_choose(g_dialog, g_ttx[427], 7, g_ttx[428], g_ttx[430], g_ttx[429], g_ttx[431],
			g_ttx[432], g_ttx[433], g_ttx[436]) - 1;
		if (sel != -2)
			g_action = sel + 0x81;
	}
	if (!town_action() && g_pp20_index == 4) {
		view_frame();
		if (g_pick_event != -1) {
			if (PICK_DEPTH < 0x96)
				call_handler2(g_town_events, &g_pick_event);
		} else if (*(signed long *)&g_cell_event != -1) {
			struct map_event far *ev = &g_cell_event;

			g_town_steps++;
			if (!(g_town_steps & 7)) {
				r = random_schweif(100);
				if (r <= 50 && typ != 3)
					town_mugging();
			}
			if (random_schweif(100) <= 2) {
				if (get_cell_a(ev->x, ev->y) != 3)
					town_random_event(random_schweif(22));
			} else
				call_handler2(g_town_events, ev);
		}
	}
	if (g_group->town == typ && g_loaded_town != -1) {
		reveal_map(g_map, g_group->x, g_group->y);
		g_group->old_x = g_group->x;
		g_group->old_y = g_group->y;
		g_group->set_dir((g_cam_angle + 0x80) / 0x100 - 1);
	}
	if (g_gavron_chase > 0) {
		Ev far *e = new Ev;

		ev_gavron_chase(e);
		delete e;
	}
}

void town_load(void)
{
	Pic pic;
	struct view_area far *e;

	e = &g_town_view_areas[g_group->town - 1];
	g_view_indoor = 0;
	g_area_type = AREA_TOWN;
	g_pp20_index = -1;
	screen_redraw(g_screen);
	view_alloc();
	view_init();
	view_timer_on();
	if (g_compass != 0)
		Compass_dtor(g_compass, 3);
	g_compass = Compass_ctor(0, 1);
	pic.load(e->pal_pic, 0, (struct rgb far *)g_view_pal);
	view_save_frame();
	if (g_renderbuf_ptr == 0)
		g_renderbuf_ptr = mem_alloc(0xaa00L);
	if (g_render_bmp == 0)
		g_render_bmp = new Bmp(0x140, 0x87, g_renderbuf_ptr);
	load_map_all(MAP_GRID, e->map, g_mem_saving == 0 ? e->gfx : e->gfx2);
	view_load_vis_order();
	g_cam_speed = 0;
	g_view_pal_n = 0xa0;
	g_view_init_flag = 0x0f;
	mouse_set_cursor(5, 1);
	view_load_objects();
	view_load_palette();
	mouse_reset();
	cam_set_from_group();
	g_cam_pitch = 0;
	g_vis_start = 0;
	r25_06c4(0x5a, 0x20);
	g_cam_angle = (g_group->dir << 8) + 0x100;
	g_cam_roll = 0;
	g_pick_event = -1;
	*(signed long *)&g_cell_event = -1;
	set_palette(g_view_pal, 0, 0x100);
	g_view_input_on = 1;
	g_loaded_town = g_group->town;
	install_cursor();
	if (g_ems_avail == 0) {
		g_tex_base = (signed long)xms_lock(g_swap_handle);
		flat_enter();
	}
	town_make_events();
	automap_set_town_type(g_map, g_loaded_town);
	if (g_cd_music == 0)
		snd_play(g_snd_music, e->music);
}

void town_automap(void)
{
	struct view_area far *e;

	e = &g_town_view_areas[g_group->town - 1];
	automap_view(e->map, e->notes, e->tiles);
}

void automap_view(signed short map_file, signed short note_file, signed short gfx)
{
	unsigned char far *buf;

	Sound_volume3(g_snd_music, 0, 0x1f4, 0);
	pal_fade_out();
	g_dialog_depth++;
	swap_out_screen();
	clear_screen_buf(g_vga_buf);
	buf = new unsigned char[0x300];
	snd_play(g_snd_music, 0x2b);
	font_set_colors(g_font, 0xff, 0x1f);
	automap_open(g_map, map_file, note_file, gfx);
	memcpy(buf, g_pal_buf, 0x300);
	memset(g_pal_buf, 0, 0x300);
	automap_draw(g_map);
	automap_show(g_map);
	memcpy(g_pal_buf, buf, 0x300);
	pal_fade_in(g_pal_buf);
	do {
		automap_run(g_map);
		if (g_refresh == 1) {
			automap_draw(g_map);
			automap_show(g_map);
			g_refresh = 0;
		}
		if (g_automap_redraw == 1)
			automap_frame(g_map);
	} while (g_action != 0x11b);
	automap_close(g_map);
	Sound_volume3(g_snd_music, 0, 0x1f4, 0);
	pal_fade_out();
	mouse_hide(0);
	clear_screen_buf(g_vga_buf);
	mouse_show(0);
	g_refresh = 1;
	snd_obj_stop(g_snd_music);
	Sound_stop_music(g_snd_music);
	delete buf;
	g_dialog_depth--;
}

signed short town_action(void)
{
	signed short typ;
	struct hero far *h;
	register signed short r = 0;
	register signed short t;

	typ = g_group->town;
	switch (g_action) {
	case 0x81:
		grp_split();
		town_update_icons();
		break;
	case 0x82:
		groups_merge();
		town_update_icons();
		break;
	case 0x83:
		group_switch_next(0);
		t = g_group->town;
		if (typ == t) {
			cam_set_from_group();
			g_cam_angle = (g_group->dir << 8) + 0x100;
		} else {
			snd_obj_stop(g_snd_music);
			swap_out_screen();
			r = 1;
		}
		if (t)
			town_update_icons();
		break;
	case 0x84:
		town_automap();
		r = 1;
		break;
	case 0x85:
		if ((h = get_active_mage(g_group, g_ttx[326])) != 0)
			use_spell(h, 1, 0);
		break;
	case 0x86:
		if (g_icon_pic_camp == 0x41)
			g_group->set_loctype(LOC_CAMP_TOWN);
		else
			dlg_message(g_dialog, g_ttx[445]);
		break;
	case 0x87: {
		SaveMenu far *b;

		swap_out_screen();
		b = new SaveMenu;
		savegame_menu(b, 0);
		delete b;
		if (g_group->town == typ) {
			g_loaded_town = typ;
			cam_set_from_group();
			g_cam_angle = (g_group->dir << 8) + 0x100;
		}
		g_refresh = 1;
		r = 1;
		break;
	}
	}
	return r;
}

void town_make_events(void)
{
	register signed short typ;

	g_cell_occludes = 0;
	switch (typ = g_group->town) {
	case 1:
		g_town_events = new Town(typ, 0x219, 10);
		break;
	case 2:
		g_town_events = new Town(typ, 0x21a, 100);
		break;
	case 3:
		g_town_events = new Town(typ, 0x21b, 0x19);
		break;
	case 4:
		g_town_events = new Town(typ, 0x21e, 0x2d);
		g_cell_occludes = g_town_occl_04;
		break;
	case 6:
		g_town_events = new Town(typ, 0x21e, 0x2d);
		break;
	case 5:
		g_town_events = new Town(typ, 0x21d, 0x5a);
		g_cell_occludes = g_town_occl_05;
		break;
	}
	automap_load_seen(g_map);
}

void view_save_frame(void)
{
	mouse_hide(0);
	screen_clear_view(g_screen);
	if (g_hud_top_left == 0) {
		mouse_hide(0);
		g_hud_top_left = new Bmp(0x22, 0x1f);
		g_hud_top_left->get(g_vga_buf, 0x0f, 2);
		g_hud_top_right = new Bmp(0x1a, 0x1e);
		g_hud_top_right->get(g_vga_buf, 0xc6, 2);
		g_hud_bottom_right = new Bmp(0x17, 0x1c);
		g_hud_bottom_right->get(g_vga_buf, 0xc9, 0x6d);
		g_hud_bottom_left = new Bmp(3, 3);
		g_hud_bottom_left->get(g_vga_buf, 0x0f, 0x86);
		mouse_show(0);
	}
	mouse_show(0);
}

void town_update_icons(void)
{
	signed short y;
	signed short x;
	signed short typ;
	signed short cell;
	struct group far *g;
	register signed short f;
	register signed short i;

	typ = g_group->town;
	for (f = 0, i = 0; i < 6; i++) {
		g = &g_groups[i];
		if (g != 0 && g != g_group && !!g->nr_heroes && g->town == typ && g->x == g_group->x &&
		    g->y == g_group->y) {
			f = 1;
			break;
		}
	}
	if (f != g_icon_group) {
		g_icon_pic_group = f == 1 ? 0x0f : 0x2d;
		screen_draw_icons(g_screen);
		g_icon_group = f;
	}
	f = 0;
	if (g_nr_groups > 1)
		f = 1;
	if (f != g_icon_groups) {
		g_icon_pic_groups = f == 1 ? 0x1d : 0x44;
		screen_draw_icons(g_screen);
		g_icon_groups = f;
	}
	y = g_group->y + g_map_y0;
	x = g_group->x + g_map_x0;
	cell = *((unsigned char far *)g_cells + (y << 8) + (x << 2));
	f = 1;
	if (typ != 3 && cell == 3)
		f = 0;
	if (typ == 1 && cell == 0x17)
		f = 0;
	if ((typ == 2 || typ == 6) && cell == 0x14)
		f = 0;
	if (f != g_icon_camp) {
		g_icon_pic_camp = f == 1 ? 0x41 : 0x2f;
		screen_draw_icons(g_screen);
		g_icon_camp = f;
	}
}

void cam_set_from_group(void)
{
	g_view_cell_y = g_group->y + g_map_y0;
	g_view_cell_x = g_group->x + g_map_x0;
	g_view_pos_x = g_group->screen_x;
	g_view_pos_y = g_group->screen_y;
	if (g_view_active != 0 && g_view_swap_pos == 0) {
		g_cam_x = g_view_cell_y * 0xa8 + 0x53;
		g_cam_z = g_view_cell_x * 0xa8 + 0x53;
		if (!can_stand(g_cam_x, g_cam_z)) {
			g_cam_x = g_group->screen_y + g_map_y0 * 0xa8;
			g_cam_z = g_group->screen_x + g_map_x0 * 0xa8;
		}
	} else {
		g_cam_x = g_group->screen_y + g_map_y0 * 0xa8;
		g_cam_z = g_group->screen_x + g_map_x0 * 0xa8;
	}
	g_cam_fx = (signed long)g_cam_x << 8;
	g_cam_fy = (signed long)g_cam_y << 8;
	g_cam_fz = (signed long)g_cam_z << 8;
}

void town_daytime_cells(void)
{
	switch (g_group->town) {
	case 1:
		if (g_date_weekday == 1 && g_daytime >= 0xa8c0L && g_daytime <= 0x190c8L) {
			if (g_town_cells_state != 1) {
				town_set_cell(0x0c, 7, 0x13, 0, 2);
				town_set_cell(0x0e, 7, 0x13, 0, 2);
				town_set_cell(0x0d, 9, 0x13, 0, 0);
				g_town_cells_state = 1;
			}
		} else if (g_town_cells_state != 2) {
			town_set_cell(0x0c, 7, 0x16, 0, 2);
			town_set_cell(0x0e, 7, 0x16, 0, 2);
			town_set_cell(0x0d, 9, 0x16, 0, 0);
			g_town_cells_state = 2;
		}
		break;
	case 2:
		if (g_date_weekday == 6 && g_daytime >= 0xa8c0L && g_daytime <= 0x190c8L) {
			if (g_town_cells_state != 1) {
				town_set_cell(0x0b, 0x0c, 0x13, 0, 2);
				town_set_cell(0x0d, 0x0c, 0x13, 0, 2);
				town_set_cell(0x0d, 0x0e, 0x13, 0, 0);
				g_town_cells_state = 1;
			}
		} else if (g_town_cells_state != 2) {
			town_set_cell(0x0b, 0x0c, 0x16, 0, 2);
			town_set_cell(0x0d, 0x0c, 0x16, 0, 2);
			town_set_cell(0x0d, 0x0e, 0x16, 0, 0);
			g_town_cells_state = 2;
		}
		break;
	default:
		return;
	}
}

void town_set_cell(register signed short x, register signed short y, signed char type, signed short flags, signed short rot)
{
	struct map_cell far *q;

	x += g_map_x0;
	y += g_map_y0;
	q = (struct map_cell far *)((unsigned char far *)g_cells + (y << 8) + (x << 2));
	q->type = type;
	q->flags = flags;
	if (rot != -1)
		q->rot = ++rot;
	cell_forget(x, y);
}

void town_mugging(void)
{
	void far *txt_buf;
	char far * far *txt;
	struct hero far *h;
	struct hero far *v;
	Bmp far *b1;
	Bmp far *b2;
	Bmp far *b3;
	signed short dmg, dummy, w, hh, save, mode;
	struct hero far *list[7];
	register signed short i;
	register signed short n = 0;

	save = dlg_set16(g_dialog, 5);
	for (i = 0; i < 7; i++)
		list[i] = 0;
	h = get_first_hero(g_group);
	do {
		if (h != 0 && check_hero3(h) && h->inv[5].item_id == 0)
			list[n++] = h;
		h = get_next_hero(g_group, h);
	} while (get_first_hero(g_group) != h);
	if (n > 0) {
#if FIX_0184
		/* Fix #184: nur gefundene Helden benutzen (i < n), mode/v vorbelegen */
		mode = 3;
		v = list[0];
		switch (g_mugging_state) {
		case 0:
			for (i = 0; i < n && list[i]->sex != 0; i++)
				;
			if (i == n) {
				for (i = 0; i < n && list[i]->sex != 1; i++)
					;
				if (i < n) {
					mode = 2;
					v = list[i];
					g_mugging_state = 2;
				}
			} else {
				mode = 1;
				v = list[i];
				g_mugging_state = 1;
			}
			break;
		case 1:
			for (i = 0; i < n && list[i]->sex != 1; i++)
				;
			if (i == n)
				mode = 3;
			else {
				mode = 2;
				v = list[i];
				g_mugging_state = 3;
			}
			break;
		case 2:
			for (i = 0; i < n && list[i]->sex != 0; i++)
				;
			if (i == n)
				mode = 3;
			else {
				mode = 1;
				v = list[i];
				g_mugging_state = 3;
			}
			break;
		case 3:
			mode = 3;
			break;
		}
#else
		switch (g_mugging_state) {
		case 0:
			for (i = 0; i < n && list[i]->sex != 0; i++)
				;
			if (list[i]->sex != 0) {
				for (i = 0; i < n && list[i]->sex != 1; i++)
					;
				if (list[i]->sex != 1)
					;
				else {
					mode = 2;
					v = list[i];
					g_mugging_state = 2;
				}
			} else {
				mode = 1;
				v = list[i];
				g_mugging_state = 1;
			}
			break;
		case 1:
			for (i = 0; i < n && list[i]->sex != 1; i++)
				;
			if (list[i]->sex != 1)
				mode = 3;
			else {
				mode = 2;
				v = list[i];
				g_mugging_state = 3;
			}
			break;
		case 2:
			for (i = 0; i < n && list[i]->sex != 0; i++)
				;
			if (list[i]->sex != 0)
				mode = 3;
			else {
				mode = 1;
				v = list[i];
				g_mugging_state = 3;
			}
			break;
		case 3:
			mode = 3;
			break;
		}
#endif
		txt_load_list(g_font, 0x44, 0x2d, &txt_buf, &txt);
		switch (mode) {
		case 1:
			nvfile_info(g_nvf_heads, 0x196, 0x0b, &w, &hh);
			b1 = new Bmp(w, hh);
			NVF_HEADS->load(0x196, b1->data(), 0, 0, 0x0b, 0, &dummy, &dummy, 0x140);
			g_dlg_box_flag = 0;
			sprintf(g_text_output_buf, txt[0], v->name);
			dlg_box_bmp(g_dialog, b1, (char far *)0, g_text_output_buf, 0);
			delete b1;
			break;
		case 2:
			nvfile_info(g_nvf_heads, 0x196, 0x0c, &w, &hh);
			b2 = new Bmp(w, hh);
			NVF_HEADS->load(0x196, b2->data(), 0, 0, 0x0c, 0, &dummy, &dummy, 0x140);
			g_dlg_box_flag = 0;
			sprintf(g_text_output_buf, txt[1], v->name);
			dlg_box_bmp(g_dialog, b2, (char far *)0, g_text_output_buf, 0);
			g_dlg_box_flag = 0;
			sprintf(g_text_output_buf, txt[2], v->name);
			dlg_box_bmp(g_dialog, b2, (char far *)0, g_text_output_buf, 0);
			delete b2;
			break;
		case 3:
			nvfile_info(g_nvf_heads, 0x196, 0x0d, &w, &hh);
			b3 = new Bmp(w, hh);
			NVF_HEADS->load(0x196, b3->data(), 0, 0, 0x0d, 0, &dummy, &dummy, 0x140);
			g_dlg_box_flag = 0;
			sprintf(g_text_output_buf, txt[3]);
			dlg_box_bmp(g_dialog, b3, (char far *)0, g_text_output_buf, 0);
			delete b3;
			h = get_first_hero(g_group);
			do {
				if (h != 0 && check_hero3(h)) {
					dmg = random_schweif(h->le > 6 ? 6 : h->le - 1);
					sub_hero_le(h, dmg);
					sprintf(g_text_output_buf, txt[4], h->name, dmg);
					dlg_message(g_dialog, g_text_output_buf);
				}
				h = get_next_hero(g_group, h);
			} while (get_first_hero(g_group) != h);
			break;
		}
		mem_free(txt_buf);
		mem_free(txt);
	}
	dlg_set16(g_dialog, save);
}

void town_random_event(register signed short n)
{
	if (g_group->town == 3) {
		switch (n) {
		case 1:
			if (g_daytime > 0x93a8L && g_daytime <= 0x190c8L)
				town_rnd_tallest_hero();
			break;
		case 2:
			if (g_daytime > 0x93a8L && g_daytime <= 0x190c8L)
				town_rnd_magic_user();
			break;
		case 3:
			town_rnd_best_weapon();
			break;
		case 4:
			town_rnd_note_17();
			break;
		default:
			break;
		}
	} else {
		switch (n) {
		case 1:
			if (g_group->town == 2)
				town_rnd_note_00();
			break;
		case 2:
			if (g_group->town == 2)
				town_rnd_once_01();
			break;
		case 3:
			if (g_group->town == 2 && g_daytime > 0x93a8L && g_daytime <= 0x190c8L)
				town_rnd_once_02();
			break;
		case 4:
			if (g_daytime > 0x93a8L && g_daytime <= 0x190c8L)
				town_rnd_meeting_0b();
			break;
		case 5:
			town_rnd_twice_0c();
			break;
		case 6:
			if (g_daytime > 0x93a8L && g_daytime <= 0x190c8L)
				town_rnd_note_0e();
			break;
		case 7:
			if (g_group->town != 2)
				town_rnd_donation(random_schweif(3) - 1);
			break;
		case 8:
			if (g_group->town != 2 && g_daytime > 0x93a8L && g_daytime <= 0x190c8L)
				town_rnd_donation_amount();
			break;
		case 9:
			if (g_group->town != 2)
				town_rnd_paid_service(random_schweif(2) - 1);
			break;
		case 10:
			if (g_group->town != 5 && g_group->town != 4)
				town_rnd_meeting_0d();
			break;
		case 11:
			if (g_group->town != 4 && g_daytime > 0x93a8L && g_daytime <= 0x190c8L)
				town_rnd_offer_1000();
			break;
		case 12:
			if (g_group->town != 2 && g_daytime > 0x93a8L && g_daytime <= 0x190c8L)
				town_rnd_find_money();
			break;
		case 13:
			if (g_group->town != 2 && g_daytime > 0x93a8L && g_daytime <= 0x190c8L)
				town_rnd_mage_arrest();
			break;
		case 14:
			if (g_group->town == 5)
				town_rnd_extortion();
			break;
		case 15:
			if (g_group->town == 5)
				town_rnd_note_1a();
			break;
		case 16:
			if (g_group->town == 5)
				town_rnd_find_item_0e();
			break;
		case 17:
			if (g_group->town == 5 && g_daytime > 0x93a8L && g_daytime <= 0x190c8L)
				town_rnd_choice_11();
			break;
		case 18:
			if (g_group->town == 6)
				town_rnd_dwarf_test();
			break;
		case 19:
			if (g_group->town == 6 && g_daytime > 0x93a8L && g_daytime <= 0x190c8L)
				town_rnd_gift_7e();
			break;
		case 20:
			if (g_daytime > 0x93a8L && g_daytime <= 0x190c8L)
				town_rnd_trader_800();
			break;
		case 21:
			if (g_group->town == 4)
				town_rnd_item83_arrest();
			break;
		case 22:
			if (g_group->town == 2 && g_daytime > 0x93a8L && g_daytime <= 0x190c8L)
				town_rnd_lured_away();
			break;
		default:
			return;
		}
	}
}

void town_cache_files(void)
{
	load_pic_list(g_cd_cache, g_cl_town, g_cl_town_st, 1, 0);
	switch (g_group->town) {
	case 1:
		load_pic_list(g_cd_cache, g_cl_town1, g_cl_town1_st, 1, 0);
		break;
	case 2:
		load_pic_list(g_cd_cache, g_cl_town2, g_cl_town2_st, 1, 0);
		break;
	case 3:
		load_pic_list(g_cd_cache, g_cl_town3, g_cl_town3_st, 1, 0);
		break;
	case 4:
		load_pic_list(g_cd_cache, g_cl_town4, g_cl_town4_st, 1, 0);
		break;
	case 5:
		load_pic_list(g_cd_cache, g_cl_town5, g_cl_town5_st, 1, 0);
		break;
	case 6:
		load_pic_list(g_cd_cache, g_cl_town6, g_cl_town6_st, 1, 0);
		break;
	}
}
