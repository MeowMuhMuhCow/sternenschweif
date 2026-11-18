/* OVL014 - Overlay O014 (Segment 0x1f4e in SCHWEIF.EXE): Hauptschleife der 3D-Ansicht fuer Dungeons (typ =
 * group.dungeon, 1..16; Gegenstueck O046 fuer Staedte mit fast gleichem Code). Tabelle g_dng_view_areas[16] (struct
 * view_area, 18 Bytes je Dungeon, Index group.dungeon - 1: Karte, Grafik normal/Sparbetrieb,
 * Automap-Notizen/-Kacheln, Palettenbild, Musik).
 * dng_main: ein Durchlauf - Musik bei Neuaufbau (g_refresh), bei Dungeonwechsel (g_loaded_dungeon) Bilderlisten je
 *   Dungeon in den CD-Cache (load_pic_list, g_dng_pics..), dng_load, Rahmen draw_loc_icons mit drei Anzeigen
 *   (g_icon_group/groups/camp), Zustandswechsel (g_view_swap_pos, g_camp_fight_pending -> Kampf 0xab, g_c6c2),
 *   dng_update_icons/dng_set_light, Tasten 0x4100/0x2004 -> 0x87, 0x4000/0x320d -> 0x84, Menue dlg_choose (Texte
 *   g_ttx[428..436]) -> Aktion g_action = 0x81 + Wahl, dng_action, Ereignisse call_handler an g_pick_event/
 *   g_cell_event, Position reveal_map(g_map) und Blickrichtung group.dir aus g_cam_angle.
 * dng_load: Dungeon laden (Pic fuer die Palette, Puffer g_renderbuf_ptr 0xaa00 / Bmp g_render_bmp 320x135,
 *   load_map_all, view_alloc/view_init/view_load_dng_objects/view_load_palette, mouse_reset, cam_set_from_group,
 *   dng_save_frame, dng_make_events). group_set_screen_pos(x, y): Gruppe auf Zelle x/y setzen (screen_x/y = n *
 *   0xa8 + 0x54, vorige Lage merken), Kamera, cell_forget. dng_save_frame/view_free_frame: vier Rahmenausschnitte
 *   g_hud_* sichern bzw. freigeben. dng_automap: automap_view mit den Tabellenwerten. dng_action: Aktionen 0x81..0x87
 *   (Gruppe teilen/vereinen/wechseln, Automap, Zauber use_spell, Rasten set_loctype(LOC_CAMP_DUNGEON), Speichern
 *   savegame_menu mit SaveMenu), Rueckgabe 1 = Ansicht verlassen. dng_make_events: Ereignisobjekt area_events (O129)
 *   je Dungeon nach g_area_events mit Textdatei/-zahl, Helligkeit g_dng_light/g_dng_light2 und Verdeckungstabelle
 *   g_cell_occludes (g_dng_occl_..). dng_update_icons: Anzeigen - andere Gruppe auf demselben Feld (g_groups) bzw.
 *   Feldtyp aus g_cells in Typlisten (is_in_byte_array, g_dng_nogroup_.., g_dng_camp_04, g_dng_nocamp_..), Bilder
 *   g_icon_pic_* (screen_draw_icons). dng_set_light: Helligkeit fade_table nach Tageszeit (g_daytime) bzw.
 *   group_mr_bonus.
 *
 * BCC -mlarge -3 -Ot -Og -c -Y OVL014.CPP   -> 4945/4945 Bytes GLEICH (18.10.2026), cobjfix laeuft
 *   (Namensfassung mit schweif.h, Welle N19: 19.10.2026 GLEICH, cobjfix LINKGLEICH)
 * - SEIT WELLE AW ASM-FREI: Musiknummer ueber typ * 18 + (unsigned)&((struct view_area near *)(unsigned)
 *   (g_dng_view_areas - 1))->music (Basis und Feldversatz ungefaltet im Register, Leitfaden "O014-Muster geloest").
 * - -Og noetig (Epilogkopien, -Og-Zwischenwert [bp-7] in set_loctype). Kein -Oi.
 * - C++ wegen inline-Elementfunktionen von struct group: set_dir(int) (Kopie [bp-4], Store nur al), set_loctype()
 *   wie O183, group_set_screen_pos: go_x(v, pos) { set_screen_x(pos); set_x(set_x(v)); } -> genau die Kopienfolge
 *   [bp-2] = x, [bp-4] = pos, [bp-6] = Ergebnis des inneren set_x.
 * - group_set_screen_pos: cell_forget bekommt g_view_cell_y/x als EIN far-Wert (VIEW_CELL_YX, eine Fixup-Stelle auf
 *   ds:9492; zwei Einzelargumente ergaeben dieselben Bytes, aber ein zweites Extern).
 * - dng_load: "volatile signed short typ" + eingebettete Zuweisung "e = &g_dng_view_areas[(typ = ..) - 1]" (sonst typ in SI bzw.
 *   Nachladen); Pic als lokales Objekt (0xa6 Bytes, am Ende). dng_action: "new SaveMenu" mit implizitem inline-Ktor
 *   (operator new 0x6f7, dann if (p) Pic-Ktor), Zeiger im case-Block (hinter dem -Og-Byte [bp-7]).
 *   dng_make_events: "switch (typ = ..)" eingebettet (mov bx,ax); case 9 steht im Quelltext zwischen 4 und 5
 *   (Sprungtabelle). dng_update_icons: Block-Locals in zwei Bloecken teilen sich [bp-4..-0ch]; "!!g->nr_heroes";
 *   "default: break;" nach dem letzten case.
 * - Phase B (19.10.2026): eigene Daten im Quelltext. Die Dungeontabelle beginnt bei ds:0bcf; der alte Name g_dng_areas
 *   (MAP-Public ds:0bbd = g_dng_view_areas - 1) stammt aus dem Zugriff mit 1-basiertem Index, die 18 Bytes davor
 *   sind das Ende der Objektliste g_dng_objs_16. Die Objektlisten g_dng_objs_nn (struct view_obj) hatten keine Namen.
 * Auffaellig (Original): dng_make_events laesst a/b (-> g_dng_light/g_dng_light2) fuer Dungeon 4 uninitialisiert
 *   (dng_set_light benutzt sie dort nicht); dng_set_light "v += 10; v -= 10;" ohne Wirkung; dng_update_icons prueft
 *   "&g_groups[i] != 0" (immer wahr).
 */
#include <stdio.h>
#include "schweif.h"

/* Phase B: eigene Daten des Moduls (ds:06b4..0ede, dsana) - initialisierte Daten in Adressreihenfolge */
unsigned char g_dng_occl_05[24] = {
	0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00
};
unsigned char g_dng_occl_01[29] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x00
};
unsigned char g_dng_occl_02[24] = {
	0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x00,
	0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
unsigned char g_dng_occl_03[24] = {
	0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00
};
unsigned char g_dng_occl_09[21] = {
	0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01,
	0x01, 0x00, 0x00, 0x00, 0x00
};
unsigned char g_dng_occl_10[24] = {
	0x00, 0x01, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x00, 0x00, 0x01, 0x00,
	0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
unsigned char g_dng_occl_11[24] = {
	0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00,
	0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00
};
unsigned char g_dng_occl_12[24] = {
	0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00
};
unsigned char g_dng_occl_13[24] = {
	0x00, 0x01, 0x00, 0x00, 0x01, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
unsigned char g_dng_occl_14[24] = {
	0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
unsigned char g_dng_occl_16[24] = {
	0x00, 0x00, 0x01, 0x01, 0x01, 0x00, 0x00, 0x01, 0x01, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x00
};
struct view_obj g_dng_objs_01[] = {	/* Objekte Dungeon 1 (Ende -1) */
	{ 0, 0x49b }, { 1, 0x272 }, { 2, 0x276 }, { 3, 0x277 }, { 4, 0x497 }, { 5, 0x498 },
	{ 6, 0x499 }, { 7, 0x49a }, { 8, 0x275 }, { 9, 0x4a9 }, { 10, 0x4aa }, { 11, 0x4ac },
	{ 12, 0x4bc }, { 13, 0x4a0 }, { 14, 0x4a1 }, { 15, 0x274 }, { 16, 0x273 }, { 17, 0x4a2 },
	{ 18, 0x27a }, { 19, 0x278 }, { 20, 0x279 }, { 21, 0x27c }, { 22, 0x4a3 }, { 23, 0x4a4 },
	{ 24, 0x4a5 }, { 25, 0x4a6 }, { 26, 0x4a7 }, { 27, 0x4a8 }, { -1, 0 }
};
struct view_obj g_dng_objs_02[] = {	/* Objekte Dungeon 2 */
	{ 0, 0x28a }, { 1, 0x27e }, { 2, 0x281 }, { 3, 0x285 }, { 4, 0x282 }, { 5, 0x28b },
	{ 6, 0x283 }, { 7, 0x284 }, { 8, 0x286 }, { 9, 0x287 }, { 10, 0x280 }, { 11, 0x289 },
	{ 12, 0x288 }, { 13, 0x27f }, { 14, 0x28c }, { 15, 0x28d }, { 16, 0x28e }, { 17, 0x28f },
	{ 18, 0x290 }, { 19, 0x349 }, { -1, 0 }
};
struct view_obj g_dng_objs_03[] = {	/* Objekte Dungeon 3 */
	{ 0, 0x2a9 }, { 1, 0x2a2 }, { 2, 0x291 }, { 3, 0x292 }, { 4, 0x293 }, { 5, 0x294 },
	{ 6, 0x295 }, { 7, 0x296 }, { 8, 0x29d }, { 9, 0x2a1 }, { 10, 0x2a8 }, { 11, 0x29f },
	{ 12, 0x2a6 }, { 13, 0x2a0 }, { 14, 0x29c }, { 15, 0x2a3 }, { 16, 0x2a4 }, { 17, 0x2a5 },
	{ 18, 0x29e }, { 19, 0x299 }, { 20, 0x2a7 }, { 21, 0x2ab }, { -1, 0 }
};
struct view_obj g_dng_objs_04[] = {	/* Objekte Dungeon 4 */
	{ 0, 0x2cb }, { 1, 0x2cc }, { 2, 0x2cd }, { 3, 0x2ce }, { 4, 0x2cf }, { 5, 0x2d0 },
	{ 6, 0x2d1 }, { 7, 0x2dd }, { 8, 0x2dc }, { 9, 0x2d8 }, { 10, 0x20e }, { 11, 0x2da },
	{ 12, 0x2de }, { 13, 0x2e1 }, { 14, 0x2db }, { 15, 0x2d3 }, { 16, 0x2d4 }, { 17, 0x2d5 },
	{ 18, 0x2d6 }, { 19, 0x2df }, { 20, 0x2e0 }, { 21, 0x2d7 }, { 22, 0x24b }, { 23, 0x4b4 },
	{ 24, 0x4b5 }, { 25, 0x4b6 }, { -1, 0 }
};
struct view_obj g_dng_objs_09[] = {	/* Objekte Dungeon 9 */
	{ 0, 0x31a }, { 1, 0x31b }, { 2, 0x31c }, { 3, 0x31e }, { 4, 0x31f }, { 5, 0x320 },
	{ 6, 0x321 }, { 7, 0x326 }, { 8, 0x325 }, { 9, 0x324 }, { 10, 0x31d }, { 11, 0x327 },
	{ 12, 0x322 }, { 13, 0x323 }, { 14, 0x4ae }, { 15, 0x4af }, { 16, 0x4b1 }, { 17, 0x4b0 },
	{ 18, 0x4b2 }, { 19, 0x4ba }, { 20, 0x4bb }, { -1, 0 }
};
struct view_obj g_dng_objs_05[] = {	/* Objekte Dungeon 5 */
	{ 0, 0x352 }, { 1, 0x353 }, { 2, 0x354 }, { 3, 0x355 }, { 4, 0x356 }, { 5, 0x357 },
	{ 6, 0x358 }, { 7, 0x359 }, { 8, 0x35a }, { 9, 0x35b }, { 10, 0x35c }, { 11, 0x35e },
	{ 12, 0x361 }, { 13, 0x360 }, { 14, 0x370 }, { 15, 0x362 }, { 16, 0x363 }, { 17, 0x35f },
	{ 18, 0x35d }, { 19, 0x371 }, { 20, 0x223 }, { 21, 0x224 }, { -1, 0 }
};
struct view_obj g_dng_objs_06[] = {	/* Objekte Dungeon 6 */
	{ 0, 0x387 }, { 1, 0x388 }, { 2, 0x389 }, { 3, 0x38a }, { 4, 0x38b }, { 5, 0x3d3 },
	{ 6, 0x38e }, { 7, 0x39b }, { 8, 0x39a }, { 9, 0x3a0 }, { 10, 0x39d }, { 11, 0x39e },
	{ 12, 0x398 }, { 13, 0x399 }, { 14, 0x391 }, { 15, 0x392 }, { 16, 0x393 }, { 17, 0x394 },
	{ 18, 0x396 }, { 19, 0x395 }, { 20, 0x390 }, { 21, 0x397 }, { 22, 0x38f }, { 23, 0x3a2 },
	{ 24, 0x38d }, { 25, 0x38c }, { 26, 0x39c }, { 27, 0x3a5 }, { 28, 0x39f }, { -1, 0 }
};
struct view_obj g_dng_objs_07[] = {	/* Objekte Dungeon 7 */
	{ 0, 0x387 }, { 1, 0x388 }, { 2, 0x389 }, { 3, 0x38a }, { 4, 0x38b }, { 5, 0x3d3 },
	{ 6, 0x3c7 }, { 7, 0x3c8 }, { 8, 0x3cb }, { 9, 0x3c1 }, { 10, 0x39e }, { 11, 0x3c5 },
	{ 12, 0x3c4 }, { 13, 0x397 }, { 14, 0x3ce }, { 15, 0x3cf }, { 16, 0x3bc }, { 17, 0x3ba },
	{ 18, 0x3c6 }, { 19, 0x3c9 }, { 20, 0x3b9 }, { 21, 0x3d0 }, { 22, 0x39f }, { -1, 0 }
};
struct view_obj g_dng_objs_08[] = {	/* Objekte Dungeon 8 */
	{ 0, 0x387 }, { 1, 0x388 }, { 2, 0x38b }, { 3, 0x38a }, { 4, 0x3d7 }, { 5, 0x3d3 },
	{ 6, 0x39e }, { 7, 0x3d9 }, { 8, 0x3c1 }, { 9, 0x39b }, { 10, 0x3da }, { 11, 0x3db },
	{ 12, 0x3a5 }, { 13, 0x3dc }, { 14, 0x392 }, { 15, 0x3dd }, { 16, 0x3de }, { 17, 0x3d8 },
	{ 18, 0x413 }, { 19, 0x414 }, { 20, 0x415 }, { 21, 0x416 }, { 22, 0x3a0 }, { 23, 0x39f },
	{ -1, 0 }
};
struct view_obj g_dng_objs_10[] = {	/* Objekte Dungeon 10 */
	{ 0, 0x3df }, { 1, 0x3e0 }, { 2, 0x3e1 }, { 3, 0x3e2 }, { 4, 0x3e3 }, { 5, 0x3e4 },
	{ 6, 0x3e5 }, { 7, 0x3e6 }, { 8, 0x3e7 }, { 9, 0x3e8 }, { 10, 0x3e9 }, { 11, 0x3ea },
	{ 12, 0x3eb }, { 13, 0x3ec }, { 14, 0x3ed }, { 15, 0x3ee }, { 16, 0x3ef }, { 17, 0x3f5 },
	{ -1, 0 }
};
struct view_obj g_dng_objs_11[] = {	/* Objekte Dungeon 11 */
	{ 0, 0x3f0 }, { 1, 0x3f3 }, { 2, 0x3f8 }, { 3, 0x3e2 }, { 4, 0x3e3 }, { 5, 0x3fd },
	{ 6, 0x3fb }, { 7, 0x43d }, { 8, 0x3fc }, { 9, 0x3fa }, { 10, 0x3ff }, { 11, 0x400 },
	{ 12, 0x3fe }, { 13, 0x3f9 }, { 14, 0x3f5 }, { 15, 0x3f1 }, { 16, 0x3f4 }, { 17, 0x3f2 },
	{ 18, 0x43e }, { -1, 0 }
};
struct view_obj g_dng_objs_12[] = {	/* Objekte Dungeon 12 */
	{ 0, 0x43f }, { 1, 0x440 }, { 2, 0x441 }, { 3, 0x442 }, { 4, 0x443 }, { 5, 0x444 },
	{ 6, 0x445 }, { 7, 0x446 }, { 8, 0x447 }, { 9, 0x448 }, { 10, 0x449 }, { 11, 0x44a },
	{ 12, 0x44b }, { 13, 0x44c }, { 14, 0x44d }, { 15, 0x44e }, { 16, 0x44f }, { 17, 0x450 },
	{ 18, 0x451 }, { 19, 0x452 }, { 20, 0x453 }, { 21, 0x454 }, { 22, 0x455 }, { 23, 0x456 },
	{ 24, 0x4b7 }, { -1, 0 }
};
struct view_obj g_dng_objs_13[] = {	/* Objekte Dungeon 13 */
	{ 0, 0x45b }, { 1, 0x45c }, { 2, 0x45d }, { 3, 0x447 }, { 4, 0x45f }, { 5, 0x466 },
	{ 6, 0x460 }, { 7, 0x461 }, { 8, 0x462 }, { 9, 0x46c }, { 10, 0x463 }, { 11, 0x464 },
	{ 12, 0x467 }, { 13, 0x468 }, { 14, 0x469 }, { 15, 0x46a }, { 16, 0x46b }, { 17, 0x465 },
	{ 18, 0x46d }, { 19, 0x45e }, { 20, 0x4b8 }, { -1, 0 }
};
struct view_obj g_dng_objs_14[] = {	/* Objekte Dungeon 14/15 */
	{ 0, 0x46e }, { 1, 0x46f }, { 2, 0x470 }, { 3, 0x448 }, { 4, 0x478 }, { 5, 0x477 },
	{ 6, 0x472 }, { 7, 0x476 }, { 8, 0x473 }, { 9, 0x474 }, { -1, 0 }
};
struct view_obj g_dng_objs_16[] = {	/* Objekte Dungeon 16 */
	{ 0, 0x479 }, { 1, 0x47a }, { 2, 0x47b }, { 3, 0x47c }, { 4, 0x47d }, { 5, 0x47e },
	{ 6, 0x47f }, { 7, 0x480 }, { 8, 0x481 }, { 9, 0x482 }, { 10, 0x483 }, { 11, 0x484 },
	{ 12, 0x485 }, { 13, 0x486 }, { 14, 0x487 }, { 15, 0x488 }, { 16, 0x489 }, { 17, 0x48a },
	{ 18, 0x48b }, { 19, 0x48c }, { 20, 0x48d }, { 21, 0x48e }, { 22, 0x48f }, { 23, 0x490 },
	{ -1, 0 }
};
struct view_area g_dng_view_areas[16] = {	/* je Dungeon 1..16: Karte, Grafik, Grafik Sparbetrieb, Automap-Notizen, Palettenbild,
					   Automap-Kacheln, Musik, Objektliste */
	{ 0x262, 0x263, 0x264, 0x265, 0x25a, 0x253, 41, g_dng_objs_01 },	/* Dungeon 1 */
	{ 0x266, 0x267, 0x268, 0x269, 0x25b, 0x259, 41, g_dng_objs_02 },	/* Dungeon 2 */
	{ 0x26a, 0x26b, 0x26c, 0x26d, 0x25c, 0x3d4, 41, g_dng_objs_03 },	/* Dungeon 3 */
	{ 0x26e, 0x26f, 0x270, 0x271, 0x25d, 0x254, 42, g_dng_objs_04 },	/* Dungeon 4 */
	{ 0x34e, 0x34f, 0x350, 0x351, 0x25f, 0x257, 36, g_dng_objs_05 },	/* Dungeon 5 */
	{ 0x377, 0x378, 0x379, 0x37a, 0x260, 0x258, 37, g_dng_objs_06 },	/* Dungeon 6 */
	{ 0x37b, 0x37c, 0x37d, 0x37e, 0x260, 0x258, 37, g_dng_objs_07 },	/* Dungeon 7 */
	{ 0x37f, 0x380, 0x381, 0x382, 0x260, 0x258, 37, g_dng_objs_08 },	/* Dungeon 8 */
	{ 0x230, 0x2fb, 0x2fc, 0x227, 0x25e, 0x255, 30, g_dng_objs_09 },	/* Dungeon 9 */
	{ 0x2fe, 0x2ff, 0x300, 0x301, 0x401, 0x491, 38, g_dng_objs_10 },	/* Dungeon 10 */
	{ 0x302, 0x303, 0x304, 0x305, 0x401, 0x492, 38, g_dng_objs_11 },	/* Dungeon 11 */
	{ 0x306, 0x307, 0x308, 0x309, 0x401, 0x493, 38, g_dng_objs_12 },	/* Dungeon 12 */
	{ 0x30a, 0x30b, 0x30c, 0x30d, 0x401, 0x494, 38, g_dng_objs_13 },	/* Dungeon 13 */
	{ 0x316, 0x30f, 0x310, 0x311, 0x401, 0x495, 38, g_dng_objs_14 },	/* Dungeon 14 */
	{ 0x317, 0x30f, 0x310, 0x311, 0x401, 0x495, 38, g_dng_objs_14 },	/* Dungeon 15 */
	{ 0x312, 0x313, 0x314, 0x315, 0x401, 0x496, 38, g_dng_objs_16 } 	/* Dungeon 16 */
};
signed short g_dng_pics[17] = {
	0x213d, 0x2206, 0x23b4, 0x2405, 0x221e, 0x2222, 0x2406, 0x2411, 0x209d, 0x209e, 0x22c6, 0x22c7, 0x22c8,
	0x22c9, 0x40a2, 0x21a3, -1
};
char g_dng_pics_st[4] = "";
signed short g_dng_pics_01[11] = {
	0x2262, 0x2264, 0x2253, 0x2265, 0x225a, 0x22ad, 0x2263, 0x40a5, 0x407c, 0x4061, -1
};
char g_dng_pics_01_st[4] = "";
signed short g_dng_pics_02[12] = {
	0x2266, 0x2268, 0x2259, 0x2269, 0x225b, 0x22ad, 0x2267, 0x409f, 0x405b, 0x40ac, 0x4061, -1
};
char g_dng_pics_02_st[4] = "";
signed short g_dng_pics_03[16] = {
	0x226a, 0x226c, 0x23d4, 0x225c, 0x22ad, 0x226d, 0x226b, 0x4055, 0x40ac, 0x4061, 0x409f, 0x600f, 0x6010,
	0x6011, 0x2045, -1
};
char g_dng_pics_03_st[4] = "";
signed short g_dng_pics_04[10] = { 0x226e, 0x2270, 0x2254, 0x2271, 0x225d, 0x22ca, 0x212a, 0x22eb, 0x226f, -1 };
char g_dng_pics_04_st[4] = "";
signed short g_dng_pics_05[19] = {
	0x234e, 0x2350, 0x2257, 0x23b3, 0x225f, 0x234d, 0x234f, 0x2351, 0x4047, 0x409f, 0x405b, 0x404d, 0x40ac,
	0x4061, 0x4060, 0x40a5, 0x40a0, 0x4087, -1
};
char g_dng_pics_05_st[6] = "";
signed short g_dng_pics_06[13] = {
	0x2377, 0x2379, 0x2258, 0x2260, 0x2373, 0x2378, 0x237a, 0x40a3, 0x407f, 0x4070, 0x407c, 0x4061, -1
};
char g_dng_pics_06_st[4] = "";
signed short g_dng_pics_07[11] = {
	0x237b, 0x237d, 0x2258, 0x23d5, 0x2260, 0x2374, 0x237c, 0x237e, 0x23d6, 0x4061, -1
};
char g_dng_pics_07_st[4] = "";
signed short g_dng_pics_08[9] = { 0x237f, 0x2381, 0x2258, 0x2382, 0x2260, 0x2375, 0x2380, 0x4061, -1 };
char g_dng_pics_08_st[2] = "";
signed short g_dng_pics_09[9] = { 0x2230, 0x22fc, 0x4026, 0x2255, 0x225e, 0x2231, 0x2227, 0x221b, -1 };
char g_dng_pics_09_st[2] = "";
signed short g_dng_pics_10[16] = {
	0x22fe, 0x2300, 0x2491, 0x24b9, 0x2401, 0x2402, 0x22ff, 0x2301, 0x4047, 0x4061, 0x24d2, 0x24d3, 0x24d4,
	0x24d5, 0x24d6, -1
};
char g_dng_pics_10_st[4] = "";
signed short g_dng_pics_11[10] = { 0x2302, 0x2304, 0x2492, 0x24b9, 0x2401, 0x2403, 0x2303, 0x2305, 0x4061, -1 };
char g_dng_pics_11_st[4] = "";
signed short g_dng_pics_12[9] = { 0x2306, 0x2308, 0x2493, 0x2309, 0x2401, 0x2457, 0x2307, 0x4061, -1 };
char g_dng_pics_12_st[2] = "";
signed short g_dng_pics_13[9] = { 0x230a, 0x230c, 0x2494, 0x230d, 0x2401, 0x2458, 0x230b, 0x4061, -1 };
char g_dng_pics_13_st[2] = "";
signed short g_dng_pics_14[9] = { 0x2316, 0x2317, 0x2310, 0x2459, 0x2495, 0x2401, 0x230f, 0x2311, -1 };
char g_dng_pics_14_st[2] = "";
signed short g_dng_pics_16[16] = {
	0x2312, 0x2314, 0x2496, 0x2315, 0x2401, 0x245a, 0x2313, 0x4061, 0x4047, (signed short)0xc41d, (signed
	short)0xc41e, (signed short)0xc41f, (signed short)0xc420, (signed short)0xc422, (signed short)0xc423, -1
};
char g_dng_pics_16_st[4] = "";
signed char g_dng_nogroup_01[3] = { 2, -1, 0 };
signed char g_dng_nogroup_02[3] = { 3, -1, 0 };
signed char g_dng_nogroup_03[4] = { 1, 9, -1, 0 };
signed char g_dng_nogroup_05[4] = { 4, 11, -1, 0 };
signed char g_dng_nogroup_06[4] = { 27, 28, -1, 0 };
signed char g_dng_nogroup_07[4] = { 8, 22, -1, 0 };
signed char g_dng_nogroup_08[6] = { 12, 13, 18, 23, -1, 0 };
signed char g_dng_nogroup_12[3] = { 7, -1, 0 };
signed char g_dng_nogroup_16[5] = { 6, 9, 14, -1, 0 };
signed char g_dng_camp_04[4] = { 0, 2, -1, 0 };
signed char g_dng_nocamp_13[3] = { 11, -1, 0 };
signed char g_dng_nocamp_15[4] = { 1, 6, -1, 0 };
/* Phase B: eigenes BSS (ds:8b5c..8b66) in Adressreihenfolge */
void far *g_area_events;
signed char g_dng_light;
signed char g_dng_light2;
char g_unused_8b62[4];   /* ds:8b62, unbenutzt */

#define MAP_GRID ((struct map_grid far *)&g_map_w)	/* Kartenobjekt ds:b656 (struct map_grid; erstes Feld g_map_w) */
/* Tiefe der angeklickten Flaeche (= g_pick_depth); das Original adressiert sie als g_pick_event + 4 */
#define PICK_DEPTH (((signed short *)&g_pick_event)[2])
/* cell_forget(g_view_cell_y, g_view_cell_x): das Original uebergibt beide Worte als ein far-Wert ab g_view_cell_y */
#define VIEW_CELL_YX (*(void far **)&g_view_cell_y)

void dng_main(void)
{
	signed short loaded;
	register signed short typ;
	register signed short redraw;

	typ = g_group->dungeon;
	g_loaded_town = -1;
	redraw = 0;
	if (g_refresh == 1) {
		redraw = 1;
		g_refresh = 0;
		if (g_cd_music == 0)
			{
			snd_play(g_snd_music, *(signed short near *)(typ * 18 + (unsigned)&((struct view_area near *)(unsigned)(g_dng_view_areas - 1))->music));
		}
	}
	loaded = 0;
	if (g_loaded_dungeon != typ) {
		if (g_view_active != 0)
			view_leave();
		g_dng_light = g_dng_light2 = 0;
		if (g_cd_music != 0)
			snd_obj_stop(g_snd_music);
		prepare_spell_icons();
		load_pic_list(g_cd_cache, g_dng_pics, g_dng_pics_st, 1, 0);
		switch (typ) {
		case 1: load_pic_list(g_cd_cache, g_dng_pics_01, g_dng_pics_01_st, 1, 0); break;
		case 2: load_pic_list(g_cd_cache, g_dng_pics_02, g_dng_pics_02_st, 1, 0); break;
		case 3: load_pic_list(g_cd_cache, g_dng_pics_03, g_dng_pics_03_st, 1, 0); break;
		case 4: load_pic_list(g_cd_cache, g_dng_pics_04, g_dng_pics_04_st, 1, 0); break;
		case 5: load_pic_list(g_cd_cache, g_dng_pics_05, g_dng_pics_05_st, 1, 0); break;
		case 6: load_pic_list(g_cd_cache, g_dng_pics_06, g_dng_pics_06_st, 1, 0); break;
		case 7: load_pic_list(g_cd_cache, g_dng_pics_07, g_dng_pics_07_st, 1, 0); break;
		case 8: load_pic_list(g_cd_cache, g_dng_pics_08, g_dng_pics_08_st, 1, 0); break;
		case 9: load_pic_list(g_cd_cache, g_dng_pics_09, g_dng_pics_09_st, 1, 0); break;
		case 10: load_pic_list(g_cd_cache, g_dng_pics_10, g_dng_pics_10_st, 1, 0); break;
		case 11: load_pic_list(g_cd_cache, g_dng_pics_11, g_dng_pics_11_st, 1, 0); break;
		case 12: load_pic_list(g_cd_cache, g_dng_pics_12, g_dng_pics_12_st, 1, 0); break;
		case 13: load_pic_list(g_cd_cache, g_dng_pics_13, g_dng_pics_13_st, 1, 0); break;
		case 14:
		case 15: load_pic_list(g_cd_cache, g_dng_pics_14, g_dng_pics_14_st, 1, 0); break;
		case 16: load_pic_list(g_cd_cache, g_dng_pics_16, g_dng_pics_16_st, 1, 0); break;
		}
		dng_load();
		if (g_cd_music != 0)
			{
			snd_play(g_snd_music, *(signed short near *)(typ * 18 + (unsigned)&((struct view_area near *)(unsigned)(g_dng_view_areas - 1))->music));
		}
		loaded = 1;
		set_palette(g_view_pal, 0, 0x100);
		g_view_swap_pos = 0;
		screen_textbar(g_screen, g_ttx[typ + 455], 0xff);
	}
	if (redraw) {
		if (loaded == 0)
			screen_redraw(g_screen);
		draw_loc_icons(g_screen, 7, 0x0c, g_icon_group == 1 ? 0x0f : 0x2d, g_icon_groups == 1 ? 0x1d : 0x44, 0x27, 0x0b,
			g_icon_camp == 1 ? 0x41 : 0x2f, 0x28);
		set_palette(g_view_pal, 0, 0xa0);
		screen_clear_textbar(g_screen);
		screen_textbar(g_screen, g_ttx[typ + 455], 0xff);
		g_view_recam = 1;
		if (g_cd_music != 0)
			{
			snd_play(g_snd_music, *(signed short near *)(typ * 18 + (unsigned)&((struct view_area near *)(unsigned)(g_dng_view_areas - 1))->music));
		}
	}
	if (g_view_swap_pos != 0) {
		swap_in_screen();
		if (g_view_swapped) {
			g_area_type = AREA_DUNGEON;
			g_cam_pitch = 0;
			g_cam_angle = (g_group->dir << 8) + 0x100;
			g_cam_roll = 0;
			g_view_swapped = 0;
			g_view_recam = 1;
		}
	}
	if (g_camp_fight_pending) {
		start_fight(0xab, 0);
		g_camp_fight_pending = 0;
	}
	if (g_c6c2 != 0) {
		g_cam_angle = (g_group->dir << 8) + 0x100;
		g_view_input_on = 1;
		g_c6c2 = 0;
		screen_textbar(g_screen, g_ttx[typ + 455], 0xff);
		install_cursor();
	}
	if (g_view_recam) {
		cam_set_from_group();
		g_view_recam = 0;
	}
	g_view_input_blocked = 0;
	dng_update_icons();
	dng_set_light();
	handle_input(g_screen);
	if (g_action == 0x4100 || g_action == 0x2004)
		g_action = 0x87;
	else if (g_action == 0x4000 || g_action == 0x320d)
		g_action = 0x84;
	if (g_mouse_rclick != 0 || g_key == 0x4900) {
		signed short sel;

		sel = dlg_choose(g_dialog, g_ttx[434], 7, g_ttx[428], g_ttx[430], g_ttx[429], g_ttx[431],
			g_ttx[432], g_ttx[433], g_ttx[436]) - 1;
		if (sel != -2)
			g_action = sel + 0x81;
	}
	if (!dng_action() && g_pp20_index == 4) {
		view_frame();
		if (g_pick_event != -1) {
			if (PICK_DEPTH < 0x96) {
				if (call_handler(g_area_events, &g_pick_event) == 1) {
					g_cam_speed = 0;
					g_step_flags = 0;
					g_step_active = 0;
				}
			}
		} else if (*(signed long *)&g_cell_event != -1) {
			if (call_handler(g_area_events, &g_cell_event) == 1) {
				g_cam_speed = 0;
				g_step_flags = 0;
				g_step_active = 0;
			}
		}
	}
	if (g_group->dungeon == typ) {
		reveal_map(g_map, g_group->x, g_group->y);
		g_group->old_x = g_group->x;
		g_group->old_y = g_group->y;
		g_group->set_dir((g_cam_angle + 0x80) / 0x100 - 1);
	}
}

void dng_load(void)
{
	Pic pic;
	volatile signed short typ;
	struct view_area far *e;

	e = &g_dng_view_areas[(typ = g_group->dungeon) - 1];
	g_view_indoor = typ == 4 ? 0 : 1;
	g_area_type = AREA_DUNGEON;
	pal_fade_out();
	g_pp20_index = -1;
	screen_redraw(g_screen);
	view_alloc();
	view_init();
	view_timer_on();
	if (g_compass != 0)
		Compass_dtor(g_compass, 3);
	g_compass = Compass_ctor(0, 2);
	pic.load(e->pal_pic, 0, g_view_pal);
	dng_save_frame();
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
	view_load_dng_objects();
	view_load_palette();
	mouse_reset();
	cam_set_from_group();
	g_cam_pitch = 0;
	g_vis_start = 0x10;
	r25_06c4(0x5a, 0x20);
	g_cam_angle = (g_group->dir << 8) + 0x100;
	g_cam_roll = 0;
	set_palette(g_view_pal, 0, 0x100);
	g_view_input_on = 1;
	g_loaded_dungeon = g_group->dungeon;
	install_cursor();
	g_pick_event = -1;
	*(signed long *)&g_cell_event = -1;
	if (g_ems_avail == 0) {
		g_tex_base = (signed long)xms_lock(g_swap_handle);
		flat_enter();
	}
	dng_make_events();
	if (g_cd_music == 0)
		snd_play(g_snd_music, e->music);
}

void group_set_screen_pos(register signed short x, register signed short y)
{
	g_group->go_x(x, x * 0xa8 + 0x54);
	g_group->go_y(y, y * 0xa8 + 0x54);
	cam_set_from_group();
	((void (*)(void far *))cell_forget)(VIEW_CELL_YX);
}

void dng_save_frame(void)
{
	mouse_hide(0);
	screen_clear_view(g_screen);
	if (g_hud_top_left == 0) {
		g_hud_top_left = new Bmp(0x18, 0x0d);
		g_hud_top_left->get(g_vga_buf, 0x0f, 2);
		g_hud_top_right = new Bmp(0x2d, 0x3c);
		g_hud_top_right->get(g_vga_buf, 0xb2, 2);
		g_hud_bottom_right = new Bmp(0x0b, 0x16);
		g_hud_bottom_right->get(g_vga_buf, 0xd4, 0x72);
		g_hud_bottom_left = new Bmp(0x2c, 0x34);
		g_hud_bottom_left->get(g_vga_buf, 0x0f, 0x54);
	}
	mouse_show(0);
}

void view_free_frame(void)
{
	if (g_hud_top_left != 0) {
		delete g_hud_top_left;
		delete g_hud_top_right;
		delete g_hud_bottom_left;
		delete g_hud_bottom_right;
		g_hud_top_left = 0;
	}
}

void dng_automap(void)
{
	struct view_area far *e;

	e = &g_dng_view_areas[g_group->dungeon - 1];
	automap_view(e->map, e->notes, e->tiles);
}

signed short dng_action(void)
{
	signed short typ;
	struct hero far *h;
	register signed short r = 0;
	register signed short t;

	typ = g_group->dungeon;
	switch (g_action) {
	case 0x81:
		grp_split();
		dng_update_icons();
		break;
	case 0x82:
		groups_merge();
		dng_update_icons();
		break;
	case 0x83:
		group_switch_next(0);
		t = g_group->dungeon;
		if (typ == t) {
			cam_set_from_group();
			g_cam_angle = (g_group->dir << 8) + 0x100;
		} else {
			snd_obj_stop(g_snd_music);
			swap_out_screen();
			r = 1;
		}
		if (t)
			dng_update_icons();
		break;
	case 0x84:
		dng_automap();
		r = 1;
		break;
	case 0x85:
		if ((h = get_active_mage(g_group, g_ttx[326])) != 0)
			use_spell(h, 1, 0);
		break;
	case 0x86:
		if (g_icon_camp == 1)
			g_group->set_loctype(LOC_CAMP_DUNGEON);
		else
			dlg_message(g_dialog, g_ttx[445]);
		break;
	case 0x87: {
		SaveMenu far *b;

		swap_out_screen();
		b = new SaveMenu;
		savegame_menu(b, 0);
		delete b;
		g_refresh = 1;
		r = 1;
		break;
	}
	}
	return r;
}

void dng_make_events(void)
{
	signed short a;
	signed short b;
	register signed short typ;

	g_cell_occludes = 0;
	switch (typ = g_group->dungeon) {
	case 1:
		g_area_events = new area_events(typ, 0x2ad, 0x82);
		a = 0x10;
		b = 0x1f;
		g_vis_start = 0;
		g_cell_occludes = g_dng_occl_01;
		break;
	case 2:
		g_area_events = new area_events(typ, 0x2ad, 0x82);
		a = 0x0e;
		b = 0x16;
		g_cell_occludes = g_dng_occl_02;
		break;
	case 3:
		g_area_events = new area_events(typ, 0x2ad, 0x82);
		a = 0x0c;
		b = 0x16;
		g_cell_occludes = g_dng_occl_03;
		break;
	case 4:
		g_area_events = new area_events(typ, 0x2ca, 0x1e);
		g_vis_start = 0;
		break;
	case 9:
		g_area_events = new area_events(typ, 0x21b, 0x19);
		a = 8;
		b = 0x1c;
		g_cell_occludes = g_dng_occl_09;
		break;
	case 5:
		g_area_events = new area_events(typ, 0x34d, 0x50);
		a = 0x10;
		b = 0x1a;
		g_cell_occludes = g_dng_occl_05;
		break;
	case 6:
		g_area_events = new area_events(typ, 0x373, 0x8c);
		a = 0x0e;
		b = 0x16;
		break;
	case 7:
		g_area_events = new area_events(typ, 0x374, 0x78);
		a = 0x0e;
		b = 0x16;
		break;
	case 8:
		g_area_events = new area_events(typ, 0x375, 0x64);
		a = 0x0e;
		b = 0x16;
		break;
	case 10:
		g_area_events = new area_events(typ, 0x402, 0x64);
		a = 0x0c;
		b = 0x1c;
		g_cell_occludes = g_dng_occl_10;
		break;
	case 11:
		g_area_events = new area_events(typ, 0x403, 0x41);
		a = 0x0c;
		b = 0x1c;
		g_cell_occludes = g_dng_occl_11;
		break;
	case 12:
		g_area_events = new area_events(typ, 0x457, 0x50);
		a = 0x0c;
		b = 0x1c;
		g_cell_occludes = g_dng_occl_12;
		break;
	case 13:
		g_area_events = new area_events(typ, 0x458, 0x28);
		a = 0x0c;
		b = 0x1c;
		g_cell_occludes = g_dng_occl_13;
		break;
	case 14:
	case 15:
		g_area_events = new area_events(typ, 0x459, 0x19);
		a = 0x0c;
		b = 0x1c;
		g_cell_occludes = g_dng_occl_14;
		break;
	case 16:
		g_area_events = new area_events(typ, 0x45a, 0x5a);
		a = 0x0c;
		b = 0x1c;
		g_cell_occludes = g_dng_occl_16;
		break;
	}
	g_icon_camp = 1;
	automap_load_seen(g_map);
	g_dng_light = a;
	g_dng_light2 = b;
}

void dng_update_icons(void)
{
	signed short typ;
	register signed short f;
	register signed short i;

	typ = g_group->dungeon;
	f = 0;
	if (typ != 0) {
		struct group far *g;
		signed short y;
		signed short x;
		signed short cell;

		y = g_group->y + g_map_y0;
		x = g_group->x + g_map_x0;
		cell = *((unsigned char far *)g_cells + (y << 8) + (x << 2));
		for (i = 0; i < 6; i++) {
			g = &g_groups[i];
			if (g != 0 && g != g_group && !!g->nr_heroes && g->dungeon == typ && g->x == g_group->x &&
			    g->y == g_group->y) {
				f = 1;
				break;
			}
		}
		switch (typ) {
		case 1:
		case 10:
		case 11:
		case 13:
			if (is_in_byte_array(cell, g_dng_nogroup_01) != -1)
				f = 0;
			break;
		case 2:
			if (is_in_byte_array(cell, g_dng_nogroup_02) != -1)
				f = 0;
			break;
		case 3:
			if (is_in_byte_array(cell, g_dng_nogroup_03) != -1)
				f = 0;
			break;
		case 5:
			if (is_in_byte_array(cell, g_dng_nogroup_05) != -1)
				f = 0;
			break;
		case 6:
			if (is_in_byte_array(cell, g_dng_nogroup_06) != -1)
				f = 0;
			break;
		case 7:
			if (is_in_byte_array(cell, g_dng_nogroup_07) != -1)
				f = 0;
			break;
		case 8:
			if (is_in_byte_array(cell, g_dng_nogroup_08) != -1)
				f = 0;
			break;
		case 12:
			if (is_in_byte_array(cell, g_dng_nogroup_12) != -1)
				f = 0;
			break;
		case 16:
			if (is_in_byte_array(cell, g_dng_nogroup_16) != -1)
				f = 0;
			break;
		default:
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
	if (typ == 4 || typ == 13 || typ == 15) {
		signed short y;
		signed short x;
		signed short cell;

		f = 0;
		y = g_group->y + g_map_y0;
		x = g_group->x + g_map_x0;
		cell = *((unsigned char far *)g_cells + (y << 8) + (x << 2));
		switch (typ) {
		case 4:
			if (is_in_byte_array(cell, g_dng_camp_04) != -1)
				f = 1;
			break;
		case 13:
			if (is_in_byte_array(cell, g_dng_nocamp_13) == -1)
				f = 1;
			break;
		case 15:
			if (is_in_byte_array(cell, g_dng_nocamp_15) == -1)
				f = 1;
			break;
		}
		g_icon_pic_camp = f == 1 ? 0x41 : 0x2f;
		screen_draw_icons(g_screen);
		g_icon_camp = f;
	}
}

void dng_set_light(void)
{
	register signed short v;

	v = g_dng_light;
	if (g_group->dungeon != 4) {
		if (g_group->dungeon >= 10 && g_group->dungeon <= 12 && g_binge_torches_lit) {
			v += 10;
			v -= 10;
		} else {
			v += 10;
			v -= group_mr_bonus(g_group) * 2;
			if (v > 31)
				v = 31;
		}
		fade_table(v, g_dng_light2);
	} else if (g_daytime > 0x93a8L && g_daytime <= 0x190c8L)
		fade_table(0x10, 0);
	else
		fade_table(0x1e - group_mr_bonus(g_group) * 2, 0x1c);
}
