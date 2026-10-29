/* OVL018 - Overlay O018 (Segment 0x1f63 in SCHWEIF.EXE): Kampf-Objekt (0xfb1 Bytes) und Kampfstart.
 * Konstruktor Fight_ctor(id, arg): Liste +35 (O153), Bildschirmpuffer 0xfa00 (+00, ds:8b6a), Speicher-/EMS-Pruefung
 * (R39_0062 <= 1000 -> O010_1385, R39_0082(0xb3 bzw. 0xf1), Ersatz O010_1981 bei ds:c69a), Texte 0x26 (ds:c4f3/
 * c4ef) und 0x25 (+04/+08), Tabellen ds:c953/c957/c961/c806/c99b/c4f7, drei Bilder (new Bmp 22x11, Nvf 0x47/
 * 0x7c/0x7d/0x80), Felder leeren, Heldenkette (O088_01DD/09FF) +93 = -1, 20 Gegner, Zeiger ds:c95b/c965, bei id
 * 0x493 Zusatzpuffer 0x2f44 (ds:8b72/8b6e), O098_08A1(5) -> +1a3, O134_0037. Destruktor Fight_dtor: alles freigeben
 * (Bmp/O016-Objekte per delete), EMS zurueck (O010_1999 bzw. R39_00A5), O134_0000. fight_run run(): Abbruch 3 bei
 * nur einem Helden mit +ac, 1 bei O023_011D == 0 bzw. ds:2793 und id 0x493; Palette, Musik 8/0x2e/0x1a, O022_0000,
 * Kampfschleife solange ds:c4cd (Sonderkaempfe 0x493 O020_0BF9 / 0x495 O021_0539, sonst einmalig O022_16A9,
 * O128_0908, Runde +1ee, Zeit 9, O147_03AC), Ergebnis ueber O143_0000. fight_nop leer. start_fight(id, arg): Kampf
 * starten (aus O212 u.a.) - bei 0xaa < id < 0x493 Speicher ds:984c (0x4000) in Datei ds:0fa0 sichern, Clip-
 * Rechteck ds:330b und ds:8bef/8bf1/8bf5 retten, new Fight(id, arg), run(), delete, alles zurueck.
 *
 * BCC -mlarge -3 -Ot -c -Y -E..\bc\BIN\TASM.EXE OVL018.CPP   -> 3477/3477 Bytes GLEICH (18.10.2026),
 *   cobjfix laeuft (mit -Og ebenso GLEICH).
 *   (Namensfassung mit schweif.h, Welle N7: 19.10.2026 GLEICH, cobjfix LINKGLEICH)
 * - Register: im Destruktor belegt flags SI, deshalb Schleifenzaehler OHNE register (-> DI); in start_fight register-
 *   Parameter id (SI) und fd ohne register (-> DI). Mit register-Zaehler bzw. register fd tauschen SI/DI.
 * - Kleine Strukturkopien aus DS (6-Byte-Dateiname, 8-Byte-Clip-Rechteck) -> mov ax / mov [bp-x],ax je Wort, auch
 *   zurueck. Der initialisierte Dateiname liegt ganz oben ([bp-6]), das Rechteck hinter den Skalaren.
 * - Nulltest von far-Zeigern aus Feldern mit Index ("if (g_c953[i])") -> mov ax,[bx] / or ax,[bx+2]; Zuweisung
 *   "g_c953[i] = 0" -> zwei Wortstores (Segment zuerst); long-Member = 0 -> mov dword.
 * - Ketten "first = cur = O088_01DD(..)" und "g_c952 = g_c95f = f35c = f293 = f1ab = 0" (rechts zuerst, cbw beim
 *   Uebergang char -> int); "f295 = f296 = f345 = f346 = -1".
 * - (sz << 10) + 0x3fff >> 14 als long (movsx/shl/add/sar), R39_0082 liefert long (shl/shrd).
 * - Originalfehler/Auffaelliges: Schleife "i <= 7" leert 8 Bytes ab +3d9, das achte ist +3e0 (wird danach auf
 *   -1 gesetzt; Feldgroesse 7 oder 8 unklar); "free = 0" ueberfluessig; R07_0002-Ergebnisse ungeprueft.
 */
#include <stdio.h>
#include <io.h>
#include "schweif.h"

/* Phase B: eigene Daten des Moduls (ds:0f34..0fa6, dsana) - initialisierte Daten in Adressreihenfolge */
signed short g_fight_use_ems = 0;
signed short g_file3_marked = 0;
signed short g_fight_pic_keys[46] = {
	8229, 8230, 8263, 8264, 8265, 8266, 8269, 8267, 8268, 8270, 8330, 8331, 8332, 8333, 8335, 8698,
	8219, 8699, 8700, 8701, 8337, 8346, 8334, 16555, 16474, 16476, 16484, 16471, 16472, 16505, 16506, 16468,
	16473, 16507, 16504, 16467, 16508, 16483, 16461, 16469, 16501, 16513, 16482, 16493, 16470, -1
};
char g_fight_pic_states[12] = "";
struct str6 g_fight_map_file = { "t.$$$" };
/* Phase B: eigenes BSS (ds:8b66..8b76) in Adressreihenfolge */
signed short g_fight_clock_save;
signed short g_fight_rounds;
void far *g_fight_buf;
char far *g_anim_big_buf;
char far *g_anim_big_buf_mem;

/* Mausbereich ds:330b als ein Rechteck (Original: Felder eines Symbols, keine Einzel-Publics) */
#define MOUSE_AREA (*(struct rect *)&g_mouse_min_x)

Fight::Fight(register signed short id, signed short initiative)
{
	struct hero far *first;
	struct hero far *cur;
	struct enemy far *e;
	signed short w;
	signed long free;
	signed short kb;
	register signed short i;

	free = 0;
	kb = 0xb3;
	prepare_spell_icons();
	load_pic_list(g_cd_cache, g_fight_pic_keys, g_fight_pic_states, 1, 0);
	g_fight_clock_save = g_clock_update;
	arc_mark(g_arc_fx);
	if (id == FIGHT_ID_SPECIAL || id == FIGHT_ID_SPECIAL2) {
		g_file3_marked = 1;
		arc_mark(g_file_3);
	}
	if (id == FIGHT_ID_SPECIAL)
		kb = 0xf1;
	buf = mem_alloc(0xfa00L);
	g_fight_buf = buf;
	Lst_set(g_fight_list, buf);
	swapped = 0;
	/* Speicher pruefen, Hintergrund ins XMS bzw. EMS */
	free = xms_free_kb();
	if (free <= 1000) {
		swapped = 1;
		swap_save_file();
		if (g_file3_marked == 0) {
			arc_mark(g_file_3);
			g_file3_marked = 1;
		}
	}
	g_fight_xms = xms_alloc_kb(kb);
	if (g_fight_xms <= 0) {
		if (swapped == 0) {
			swapped = 1;
			swap_save_file();
			if (g_file3_marked == 0) {
				arc_mark(g_file_3);
				g_file3_marked = 1;
			}
		}
		g_fight_xms = xms_alloc_kb(kb);
	}
	if (g_fight_xms <= 0 && g_ems_avail == 1) {
		g_fight_use_ems = 1;
		ems_alloc((((signed long)kb << 10) + 0x3fff) >> 14, &w);
		g_ems_frame = ems_frame_seg();
		g_fight_xms = w;
	}
	g_fight_obj = this;
	txt_load_list(g_font, 0x26, 0x7b, &g_mon_names_buf, &g_mon_names);
	g_fig_obj_bmps = (BMP_PTR far *)mem_alloc(0x258L);
	g_fig_obj_ids = (signed char far *)mem_alloc(0x96L);
	g_fig_extra_anims = (FIGANIM_PTR far *)mem_alloc(0x28L);
	g_mon_genders = (signed char far *)mem_alloc(0x7bL);
	g_fight_walkmap = (unsigned char far *)mem_alloc(0x258L);
	g_enemies = (struct enemy far *)mem_alloc(0x5b4L);
	pic_mark = new Bmp(0x16, 0xb);
	pic_cursor = new Bmp(0x16, 0xb);
	pic_reach = new Bmp(0x16, 0xb);
	nvfile_load(g_nvf_heads, 0x47, pic_cursor->data(), 0, 0x7c, 0, &w, &w, 0x140);
	nvfile_load(g_nvf_heads, 0x47, pic_mark->data(), 0, 0x7d, 0, &w, &w, 0x140);
	nvfile_load(g_nvf_heads, 0x47, pic_reach->data(), 0, 0x80, 0, &w, &w, 0x140);
	txt_load_list(g_font, 0x25, 0x6c, &txt_buf, &txt);
	for (i = 0; i < 150; i++)
		g_fig_obj_bmps[i] = 0;
	for (i = 0; i < 30; i++) {
		anims[i] = 0;
		g_anim_slots[i] = 0;
	}
	for (i = 0; i < 10; i++) {
		g_fig_extra_anims[i] = 0;
		g_anim_slots_extra[i] = 0;
	}
	for (i = 0; i < 30; i++)
		loot[i] = 0;
	for (i = 0; i < 30; i++)
		dropped[i] = 0;
	for (i = 0; i <= 7; i++)		/* Originalfehler: 8 Bytes, das achte ist draw_tab[0].id */
		keys[i] = 0;
	first = cur = get_first_hero(g_group);
	do {
		cur->fighter_id = -1;
		cur = get_next_hero(g_group, cur);
	} while (cur != first);
	for (i = 0; i < NR_ENEMIES; i++) {
		e = &g_enemies[i];
		e->mon_id = 0;
		e->fighter_id = -1;
	}
	for (i = 0; i < 150; i++)
		g_fig_obj_ids[i] = -1;
	mark_id = mark_id2 = f345 = f346 = -1;
	for (i = 0; i < 0x90; i++)
		reach_marks[i] = -1;
	g_quick_fight = g_fig_nr_objs = nr_twofield = nr_anims = nr_dropped = 0;
	this->initiative = initiative;
	this->id = id;
	*(signed long far *)&top_hero = 0;
	*(signed long far *)&top_enemy = 0;
	*(signed long far *)&bottom_hero = 0;
	*(signed long far *)&bottom_enemy = 0;
	g_fig_redraw_top = 1;
	g_fig_redraw_bottom = 1;
	g_fig_redraw_text = 1;
	g_fig_list = &list;
	g_fig_tmpl = &tmpl;
	draw_tab[0].id = 0xff;
	g_fig_redraw_all = 1;
	g_fight_id = this->id;
	pal = (unsigned char far *)mem_alloc(0x300L);
	msg = (char far *)mem_alloc(0xc8L);
	Lst_init(g_fight_list);
	g_fig_text = (char far *)mem_alloc(0x64L);
	*g_fig_text = 0;
	g_anim_small_buf = mem_alloc(0x550L);
	if (id == FIGHT_ID_SPECIAL) {
		g_anim_big_buf_mem = (char far *)mem_alloc(0x2f44L);
		g_anim_big_buf = g_anim_big_buf_mem + 0x64;
	} else
		g_anim_big_buf = 0;
	nvf5_count = nvfile_count(g_nvf_heads, 5);
	g_fig_enemies_escaped = 0;
	g_fight_running = 1;
	fight_timer_start();
}

Fight::~Fight()
{
	signed short i;

	if (g_anim_big_buf)
		mem_free(g_anim_big_buf_mem);
	mem_free(g_anim_small_buf);
	mem_free(g_fig_text);
	mem_free(msg);
	mem_free(pal);
	g_fight_screen_on = 0;
	g_fight_obj = 0;
	for (i = 0; i < 150; i++)
		if (g_fig_obj_bmps[i])
			delete g_fig_obj_bmps[i];
	for (i = 0; i < 30; i++)
		if (anims[i])
			delete anims[i];
	for (i = 0; i < 10; i++)
		if (g_fig_extra_anims[i])
			delete g_fig_extra_anims[i];
	mem_free(txt_buf);
	mem_free(txt);
	delete pic_cursor;
	delete pic_mark;
	delete pic_reach;
	mem_free(g_fight_walkmap);
	mem_free(g_fig_obj_bmps);
	mem_free(g_fig_obj_ids);
	mem_free(g_mon_genders);
	mem_free(g_fig_extra_anims);
	mem_free(g_enemies);
	mem_free(g_mon_names_buf);
	mem_free(g_mon_names);
	mem_free(field);
	mem_free(f1ff);
	mem_free(buf);
	if (g_fight_use_ems == 1)
		ems_free(g_fight_xms);
	else
		xms_free_block(g_fight_xms);
	g_fight_xms = 0;
	g_fight_use_ems = 0;
	if (g_file3_marked == 1) {
		arc_release(g_file_3);
		g_file3_marked = 0;
	}
	arc_release(g_arc_fx);
	if (swapped) {
		swap_alloc();
		swapped = 0;
	}
	g_sfight_phase = 0;
	g_fight_running = 0;
	fight_timer_stop();
	g_clock_update = g_fight_clock_save;
}

signed short Fight::run()
{
	signed short ret = 0;
	register signed short first = 1;
	struct hero far *h;

	if ((signed short)g_group->nr_heroes == 1) {
		h = get_first_active_hero(g_group);
		if (h->invisible)
			return 3;
	}
	if (fight_count_monsters(this, id) == 0 || (g_sfight_won == 1 && id == FIGHT_ID_SPECIAL)) {
		g_refresh = 1;
		return 1;
	}
	g_dialog_depth++;
	mouse_hide(0);
	clear_screen_buf(g_vga_buf);
	nvfile_pal(g_nvf_heads, 0x4a, pal, 0xa0);
	if (id < FIGHT_ID_SPECIAL)
		set_palette(pal, 0, 0x40);
	set_palette(pal + 0x180, 0x80, 0x20);
	g_pp20_index = -1;
	if (g_cd_music)
		fight_setup(this);
	if (id == FIGHT_ID_SPECIAL)
		snd_play(g_snd_music, 8);
	else if (id == FIGHT_ID_SPECIAL2)
		snd_play(g_snd_music, 0x2e);
	else
		snd_play(g_snd_music, 0x1a);
	if (g_cd_music == 0)
		fight_setup(this);
	g_fight_screen_on = 1;
	/* Kampfschleife */
	while (g_in_fight) {
		if (g_quick_fight)
			game_idle();
		if (g_refresh) {
			if (first) {
				first = 0;
				if (id == FIGHT_ID_SPECIAL)
					sfight_intro(this);
				else if (id == FIGHT_ID_SPECIAL2)
					sfight2_intro(this);
				else if (!intro_done) {
					fight_draw(this);
					fight_intro_text(this);
					intro_done = 1;
				}
			}
			fight_draw(this);
			g_refresh = 0;
		}
		if (g_in_fight)
			fight_do_round(this);
		round++;
		g_dialog_depth--;
		timewarp(g_date, 9L);
		g_dialog_depth++;
		if (g_in_fight)
			fight_latecomers(this);
		if (g_quick_fight)
			game_idle();
	}
	g_fight_screen_on = 0;
	fight_finish(this, &ret);
	mouse_show(0);
	g_fight_rounds = round;
	return ret;
}

void fight_nop(void)
{
}

signed short start_fight(register signed short id, signed short initiative)
{
	struct str6 map_file = g_fight_map_file;
	signed short ret;
	Fight far *f;
	signed short swapped;
	signed short handler_on;
	void (far *handler)(signed short);
	void far *handler_areas;
	struct rect mouse_area;
	signed short fd;

	if (id > FIGHT_ID_MAX_DEF && id < FIGHT_ID_SPECIAL) {
		/* Zellentabelle der 3D-Karte fuer die Kampffeld-Erzeugung sichern (fight_setup_env) */
		unlink(map_file.s);
		fd = _creat(map_file.s, 0);
		write(fd, g_cells, 0x4000);
		close(fd);
	}
	snd_obj_stop(g_snd_music);
	ani_halt(g_screen_pic);
	swapped = swap_out_screen();
	handler_on = g_mouse_handler_on;
	handler = g_mouse_handler;
	handler_areas = g_mouse_handler_areas;
	g_mouse_handler_on = 0;
	mouse_area = MOUSE_AREA;
	MOUSE_AREA.y0 = 0;
	MOUSE_AREA.y1 = 199;
	MOUSE_AREA.x0 = 0;
	MOUSE_AREA.x1 = 319;
	f = new Fight(id, initiative);
	ret = f->run();
	delete f;
	MOUSE_AREA = mouse_area;
	g_mouse_handler_on = handler_on;
	g_mouse_handler = handler;
	g_mouse_handler_areas = handler_areas;
	if (swapped)
		swap_in_screen();
	return ret;
}
