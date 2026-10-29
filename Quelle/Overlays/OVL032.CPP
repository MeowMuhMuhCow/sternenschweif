/* OVL032 - Overlay O032 (Segment 0x1fac in SCHWEIF.EXE): Programmstart. main(argc, argv): FILES-Pruefung
 * (dos_files_total < MIN_DOS_FILES -> Meldung g_str_files_low, Ende), g_mem_too_low -> Ende, Versionszeile
 * g_version_str, srand(time), init_memory, Grafik (set_video_mode(0x13)), Init (check_speech_file, mouse_init,
 * set_keyboard_isr, timer_init, load_palette_arc3, init_game_state), Klangeinstellungen SndSetup (O043, new/run/delete),
 * Spielstand LASTGAME fehlt -> g_first_start; Vorspann Opening (O027) bei Parameter -x//x oder beim ersten Start;
 * LASTGAME fehlt bzw. Parameter -> Abfrage des Spielmodus g_game_mode (g_ttx[0]); freier Platz auf dem Laufwerk
 * (disk_free) unter g_disk_needed -> Meldung g_ttx[3]; sonst Spielstand: Parameter (ohne -//) als Name (".GAM"
 * abschneiden, savegame_make_name, ".GAM" anhaengen, savegame_load, Fehler g_ttx[473]) bzw. ohne Parameter
 * savegame_read("LASTGAME") ueber ein SaveMenu, dann main_loop, cleanup_game(0).
 * init_memory: Speicher (dos_get_indos_ptr, farcoreleft, g_vga_buf = A000:0000, Puffer g_text_output_buf/g_text_buf2/
 * g_heroes/g_loc_icon_buf/g_portrait_buf, Haupttexte GLOBLTXT.LTX 0x15 (0x1e0 Stueck), Paletten, Grammatikpuffer),
 * unter 0x2a940 Bytes freiem Speicher 7 Meldungen und r31_0020. free_swap_handles: Auslagerungsblocks
 * g_swap_handle/g_swap_handle2 freigeben (ems_free/xms_free_block). game_free_all: alles freigeben. disk_free:
 * freier Platz (getdfree). init_game_state: Startwerte (Schriftfarben, Gruppe g_groups[0] -> g_group, Position/
 * Richtung, Optionen (g_opt_music usw.), Lautstaerken/Geschwindigkeiten). prepare_dirs: Temp-Verzeichnis anlegen/leeren
 * (findfirst/unlink), CHARS1.DAT (0xfa) kopieren (herofile_copy). check_speech_file: Datei 1 in g_file_speech
 * pruefen -> g_speech_on. dos_files_total: Summe der DOS-Dateitabellen (INT 21h/52h, Inline-asm).
 * create_amap_watch_file: AMAPWTCH.DAT (0x407) mit 27 x 4096 Bytes anlegen. globals_ctor/globals_dtor: statische
 * Konstruktoren/Destruktoren der globalen Objekte (Bilder, Dateien, Archive, CD-Cache, Schrift, Klaenge, Gruppenfeld
 * 6 x 0x49 per _vector_new_ mit Group_ctor; dazu der Aufruf der Startpruefung cd_check (SEG005, unveraendert)).
 *
 * BCC -mlarge -3 -Ot -Og -c -Y OVL032.CPP   -> 4413/4413 Bytes GLEICH (18.10.2026); cobjfix laeuft inzwischen (s.u.).
 *   (Namensfassung mit schweif.h, Welle N20: 19.10.2026 GLEICH, LINKGLEICH)
 *   Ohne -Og 6 Bytes zu lang, -Oi falsch (strcpy/strcat sind Aufrufe).
 * - C++: Public _main = "void main(register int argc, char **argv)" (argc in SI, fd ohne register -> DI).
 * - Eigene Funktionen weiter unten -> far call (main zuerst), game_free_all ruft free_swap_handles -> push cs / call near.
 * - globals_ctor/globals_dtor im Original vom Compiler erzeugt; hier normale Funktionen, Konstruktoren als Funktionen mit
 *   this, _vector_new_(g_groups, 0x49, 6UL, 5, Group_ctor) mit Deklaration (void far *, unsigned, unsigned long,
 *   unsigned, ...) -> @_vector_new_$qnvuiuluie; Group_ctor = Funktionszeiger ins Overlay (push seg / push 20h).
 * - Group_ctor als Funktionszeiger zeigt im Original auf den Overlay-Thunk (push seg / push 20h); frueher brach
 *   cobjfix daran ab, die aktuelle Fassung loest den Thunk ueber die Original-EXE auf. MK_FP(FP_SEG(Group_ctor),
 *   0x20) ergaebe push byte 20h statt push word (nicht bytegleich).
 * - g_mem_debug2/g_mem_debug werden als zwei Worte genullt (oberes zuerst): als zwei int geschrieben; "far-Zeiger = 0" ergibt
 *   mit -3 mov dword.
 * - init_game_state: inline group::set_x_v(v) { old_x = x; x = v; } (ebenso set_y_v; die Header-Fassung set_x gibt den
 *   Wert zurueck): set_x_v(g_group->x) -> x als -Og-Zwischenwert in SI, Parameterkopie [bp-2]; set_loctype_c(0x81) mit
 *   [bp-5] und losem cbw (int-Rueckgabe verworfen).
 * - main: Opening ohne Destruktor -> delete = mem_free_v direkt; Spielstandobjekt mit Mitglied-Konstruktor
 *   -> operator new + if (p) Pic_ctor(p), delete -> if (p) { Pic_dtor(p, 2); mem_free_v(p); } (SaveMenu).
 * - main: leerer then-Zweig "if (c == '-' || c == '/') { } else {...}" -> Sprung auf "jmp near" (L_035b).
 * - prepare_dirs: "r = findfirst(..); if (r == 0) do {...} while (r == 0);" (Test oben und unten).
 * - dos_files_total: asm (31 C9 xor cx,cx; cmp bx,0ffffh = 81 FB FF FF), "return _CX;" -> mov ax,cx + toter Epilog.
 * - Phase B (19.10.2026): O032 definiert die globalen Daten des Spiels selbst (DATA 8952 Bytes, BSS 15328 Bytes, am
 *   Dateiende). g_spell_handlers/g_mon_spell_handlers als echte Methodenzeiger (&Cast::spell_..., &Spell::mon_spell_...),
 *   g_forbidden_item_lists[12] (Index type - 1; MAP-Name g_forbidden_items = 4 Bytes davor). globals_ctor/_dtor bleiben
 *   normale Funktionen, ihre _INIT_/_EXIT_-Eintraege stehen weiter im Reststueck DRO032 (Spalte D, nicht DI).
 *   Namen der Kopierschutz-Welle (KS) statt der Kompatibilitaetsmakros: g_indos_flag/dos_get_indos_ptr,
 *   g_copyprot/CopyProt_ctor/_dtor, cleanup_game, cd_check, timer_init.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <io.h>
#include <fcntl.h>
#include <dir.h>
#include <dos.h>
#include <alloc.h>
#include "schweif.h"

/* Die globalen Daten des Spiels (Phase B: DATA ds:1966..3c5e, BSS ds:8b96..c776) stehen am Dateiende. */

#define MIN_DOS_FILES	20		/* FILES= mindestens */
#define CHARS1_DAT	0xfa		/* CHARS1.DAT (Heldendateien, nach ..\TEMP kopiert) */
#define GLOBLTXT	0x15		/* GLOBLTXT.LTX (0x1e0 Texte) -> g_ttx */
#define AMAPWTCH_DAT	0x407		/* AMAPWTCH.DAT (27 x 4096 Bytes in ..\TEMP angelegt) */

void far *_vector_new_(void far *p, unsigned size, unsigned long n, unsigned mode, ...);	/* Laufzeitbibliothek */

void main(register int argc, char **argv)
{
	signed short fd;

	if (dos_files_total() < MIN_DOS_FILES) {
		puts(g_str_files_low);
		chdir(g_start_dir);
		free_swap_handles();
		exit(0);
	}
	if (g_mem_too_low != 0) {
		chdir(g_start_dir);
		free_swap_handles();
		exit(0);
	}
	g_version_str = (char far *)mem_alloc(50L);
	g_version_b = 1;
	g_version_c = 2;
	g_version_a = 1;
	g_version_d = 3;
	strcpy(g_version_str, g_str_version);
	srand(time(0));
	r43_003c(&g_video_state);
	printf(g_version_str);
	g_mem_debug2 = 0;
	g_mem_debug = 0;

	if (init_memory()) {
		set_video_mode(0x13);
		check_speech_file();
		mouse_init();
		set_keyboard_isr();
		timer_init();
		load_palette_arc3();
		init_game_state();
		{
			SndSetup far *p = new SndSetup;
			p->run();
			delete p;
		}
		set_palette(g_pal_buf, 0, 0x100);
		g_music_ready = 1;
		g_sound_ready = 1;
		Speech_set(g_snd_speech, 1);
		sprintf(g_text_output_buf, g_path_fmts[1], g_str_lastgame_3ac4);
		if ((fd = _open(g_text_output_buf, O_RDONLY | O_BINARY)) != -1)
			close(fd);
		else
			g_first_start = 1;

		if ((argc > 1 && (argv[1][0] == '-' || argv[1][0] == '/')) || (g_first_start != 0 && argc <= 1)) {
			Opening far *p = new Opening;
			p->run();
			delete p;
			load_palette_arc3();
		}

		screensaver_reset(&g_saver);
		sprintf(g_text_output_buf, g_path_fmts[1], g_str_lastgame_3acd);
		if ((fd = _open(g_text_output_buf, O_RDONLY | O_BINARY)) != -1)
			close(fd);

		if ((argc <= 1 && fd == -1) || (argc > 1 && (argv[1][0] == '-' || argv[1][0] == '/'))) {
			mouse_show(0);
			/* GLOBLTXT[0] "Bitte waehlen Sie Ihren@Schwierigkeitsgrad!", [1] "Anfaenger", [2] "Fortgeschritten" */
			g_game_mode = -2;
			while (g_game_mode == -2)
				g_game_mode = dlg_choose(g_dialog, g_ttx[0], 2, g_ttx[1], g_ttx[2]) - 1;
			mouse_hide(0);
		}

		g_disk_free = disk_free();
		if (g_disk_free < g_disk_needed) {
			sprintf(g_text_output_buf, g_ttx[3], g_disk_needed - g_disk_free);
			dlg_message(g_dialog, g_text_output_buf);
		} else {
			clear_kbd();
			if (argc > 1) {
				if (argv[1][0] == '-' || argv[1][0] == '/') {
				} else {
					SaveMenu far *p;
					char far *q;
					char name[14];

					strcpy(g_text_buf2, argv[1]);
					str_toupper(g_text_buf2);
					q = strstr(g_text_buf2, g_str_gam_3ad6);
					if (q != 0)
						*q = 0;
					savegame_make_name(g_text_output_buf, g_text_buf2);
					strcpy(name, g_text_output_buf);
					strcat(g_text_output_buf, g_str_gam_3adb);
					p = new SaveMenu;
					if (savegame_load(p, g_text_output_buf) == -1) {
						sprintf(g_text_output_buf, g_ttx[473], name);
						dlg_message(g_dialog, g_text_output_buf);
					}
					delete p;
					g_refresh = 1;
				}
			} else {
				SaveMenu far *p = new SaveMenu;
				savegame_read(p, g_str_lastgame_3ae0);
				delete p;
				g_refresh = 1;
			}
			mouse_show(0);
			g_status_on = 1;
			main_loop();
		}
		cleanup_game(0);
	}
}

signed short init_memory(void)
{
	unsigned long mem;

	g_indos_flag = dos_get_indos_ptr();
	g_mem_min_free = g_mem_free_start = farcoreleft();
	g_vga_buf = MK_FP(0xa000, 0);
	g_text_output_buf = (char far *)mem_alloc(0x5f4L);
	g_text_buf2 = (char far *)mem_alloc(500L);
	g_heroes = (struct hero far *)mem_alloc(0x2989L);
	txt_load_list(g_font, GLOBLTXT, 0x1e0, &g_ttx_buf, &g_ttx);
	g_loc_icon_buf = (unsigned char far *)mem_alloc(0x1440L);
	g_portrait_buf = (unsigned char far *)mem_alloc(0x400L);
	g_pal_buf = (unsigned char far *)mem_alloc_w(0x300);
	g_pal_dark = (unsigned char far *)mem_alloc_w(0x300);
	g_grammar_bufs = (char far *)mem_alloc_w(0xa0);
	g_grammar_name_buf = (char far *)mem_alloc_w(0x28);
	Date_alloc(g_date);
	if ((mem = farcoreleft()) <= 0x2a940L) {
		puts(g_str_mem_low1);
		puts(g_str_mem_low2);
		puts(g_str_mem_low3);
		puts(g_str_mem_low4);
		puts(g_str_mem_low5);
		puts(g_str_mem_low6);
		puts(g_str_mem_low7);
		r31_0020();
	}
	g_palette = (unsigned char far *)mem_alloc_w(0x300);
	return 1;
}

void free_swap_handles(void)
{
	if (g_swap_handle != 0) {
		if (g_ems_avail == 1)
			ems_free(g_swap_handle);
		else
			xms_free_block(g_swap_handle);
	}
	if (g_mem_saving == 0)
		xms_free_block(g_swap_handle2);
}

void game_free_all(void)
{
	mem_free(g_text_output_buf);
	mem_free(g_text_buf2);
	mem_free(g_ttx);
	mem_free(g_heroes);
	mem_free(g_ttx_buf);
	mem_free(g_portrait_buf);
	mem_free(g_loc_icon_buf);
	mem_free_v(g_pal_buf);
	mem_free_v(g_pal_dark);
	mem_free_v(g_grammar_bufs);
	mem_free_v(g_grammar_name_buf);
	if (g_view_active != 0)
		view_leave();
	free_swap_handles();
	if (g_fight_xms != 0) {
		if (g_fight_use_ems == 1)
			ems_free(g_fight_xms);
		else
			xms_free_block(g_fight_xms);
		g_fight_xms = 0;
		g_fight_use_ems = 0;
	}
}

signed long disk_free(void)
{
	struct dfree d;
	signed long r;

	getdfree(getdisk() + 1, &d);
	r = (unsigned long)d.df_avail * d.df_bsec * d.df_sclus;
	return r;
}

void init_game_state(void)
{
	font_set_colors(g_font, 0xf, 0);
	font_set_box(g_font, 10, 100, 10, 0);
	font_set_line_h(g_font, 8);
	font_set_screen(g_font, g_vga_buf);
	font_push_colset(g_font, 0xff, 0xfd, 0xf9, 0xfc, 0xe7);
	dlg_set16(g_dialog, 4);
	g_group = g_groups;
	g_group->old_town = g_group->town;
	g_group->town = 1;
	g_group->screen_x = 0xa2c;
	g_group->set_x_v(15);
	g_group->set_x_v(g_group->x);
	g_group->screen_y = 0x594;
	g_group->set_y_v(8);
	g_group->set_y_v(g_group->y);
	g_group->dir = 3;
	g_exit_dir = 1;
	g_group->set_loctype_c(0x81);
	g_loc_id = 1;
	prepare_dirs();
	create_amap_watch_file();
	g_opt_music = 1;
	g_opt_sound = 1;
	g_cd_speech = 1;
	g_subtitles = 1;
	g_anims_on = 1;
	g_free_move = 0;
	g_win_centered = 0;
	g_print_to_file = 0;
	g_vol_music = 100;
	g_vol_fx = 100;
	g_vol_speech = 100;
	g_print_width = 80;
	g_print_page_len = 64;
	g_text_speed = 2;
	g_anim_speed = 2;
	g_fight_speed = 0;
	g_saver_level = 3;
	g_dng04_rantzen_left = 20;
	clear_input();
	mouse_clear_left();
	mouse_clear_right();
	o109_init_stub();
}

void prepare_dirs(void)
{
	register signed short r;
	struct ffblk ff;
	char path[40];

	strcpy(path, g_str_drive_root);
	path[0] = getdisk() + 'A';
	getcurdir(0, path + 3);
	strcpy(g_text_output_buf, path);
	strcat(g_text_output_buf, g_str_temp_dir);
	if (chdir(g_text_output_buf) == 0)
		chdir(path);
	else
		mkdir(g_text_output_buf);
	sprintf(g_text_output_buf, g_path_fmts[0], g_str_all_3c59);
	r = findfirst(g_text_output_buf, &ff, 0);
	if (r == 0) {
		do {
			sprintf(g_text_output_buf, g_path_fmts[0], ff.ff_name);
			unlink(g_text_output_buf);
			r = findnext(&ff);
		} while (r == 0);
	}
	sprintf(g_text_output_buf, g_path_fmts[1], file_getname(g_file_main, CHARS1_DAT));
	sprintf(g_text_buf2, g_path_fmts[0], file_getname(g_file_main, CHARS1_DAT));
	herofile_copy(g_text_output_buf, g_text_buf2);
}

void check_speech_file(void)
{
	if (file_open(g_file_speech, 1) >= 0) {
		file_close(g_file_speech);
		g_speech_on = 1;
	} else {
		g_speech_on = 0;
	}
}

signed short dos_files_total(void)
{
	asm mov ax,5200h
	asm int 21h
	asm les bx,es:[bx+4]
	asm xor cx,cx
l:
	asm add cx,es:[bx+4]
	asm les bx,es:[bx]
	asm cmp bx,0ffffh
	asm jnz l
	return _CX;
}

void create_amap_watch_file(void)
{
	register signed short i;
	register signed short fd;
	void far *buf;

	buf = mem_alloc_w(0x1000);
	sprintf(g_text_output_buf, g_path_fmts[0], file_getname(g_file_main, AMAPWTCH_DAT));
	fd = creat(g_text_output_buf, 0x80);
	if (fd != -1) {
		for (i = 0; i < 27; i++)
			write(fd, buf, 0x1000);
		close(fd);
	}
	mem_free_v(buf);
}

void globals_ctor(void)
{
	g_data_on_cd = cd_check();		/* Aufruf der Startpruefung (SEG005, unveraendert) */
	Init_ctor(g_init);
	gb_new_alloc(g_cursor_bmp, 0x10, 0x10);
	gb_new_alloc(g_cursor_bg_bmp, 0x10, 0x10);
	gb_new_alloc(&g_bmp_mark_le, 0x20, 0x20);
	gb_new_alloc(&g_bmp_mark_ae, 0x20, 0x20);
	File_ctor(g_file_main, g_file_names[6], g_patch_star, g_names_star_idx, g_file_names[7], 0x4da);
	Dlg_ctor(g_dialog);
	arc_new(g_file_3, g_file_names[6], g_patch_star, g_names_star_idx, g_file_names[7], 0x4da, 0x41);
	File_ctor(g_file_fx, g_file_names[8], g_patch_fx, g_names_fx_idx, g_file_names[9], 0xb5);
	arc_new(g_arc_fx, g_file_names[8], g_patch_fx, g_names_fx_idx, g_file_names[9], 0xb5, 0x41);
	File_ctor(g_file_speech, g_file_names[10], g_patch_speech, g_names_speech_idx, g_file_names[11], 0xf1);
	File_ctor(g_file_raw, g_file_names[12], g_patch_raw, g_names_raw_idx, g_file_names[13], g_nr_raw_files);
	TempFile_ctor(g_ofile);
	File_ctor(g_file_cd, g_file_names[0], g_patch_starcd, g_names_starcd_idx, g_file_names[1], 0x30);
	File_ctor(g_file_speech_ext, g_file_names[2], g_patch_speechcd, g_names_speechcd_idx, g_file_names[3], 0x3d);
	Cache_ctor(g_cd_cache, 300, 2000000L);
	Nvf_ctor(g_nvf_heads);
	new_map(&g_map_w);
	Font_ctor(g_font, 1);
	Bars_ctor(g_bars);
	Screen_ctor(g_screen);
	ani_ctor(g_screen_pic);
	Sound_ctor_o042(g_snd_music, g_arc_fx, g_file_fx);
	Sound_ctor(g_snd_fx, g_file_fx, g_arc_fx, g_file_speech);
	Items_ctor((void far *)&g_items);
	Date_ctor(g_date);
	Dlg_ctor_o257(g_dialog2);
	Wx_ctor(g_weather);
	Automap_ctor(g_map);
	Diary_ctor(g_diary);
	Travel_ctor((TRAVEL_PTR)g_travel);
	TMap_ctor(g_tmap);
	_vector_new_(g_groups, sizeof(struct group), (unsigned long)NR_GROUPS, 5, Group_ctor);
	Lst_ctor(g_fight_list);
	CopyProt_ctor(g_copyprot);
}

void globals_dtor(void)
{
	CopyProt_dtor(g_copyprot, 2);
	Lst_dtor(g_fight_list, 2);
	TMap_dtor(g_tmap, 2);
	Travel_dtor((TRAVEL_PTR)g_travel, 2);
	Automap_dtor(g_map, 2);
	Wx_dtor(g_weather, 2);
	Dlg_dtor(g_dialog2, 2);
	Items_dtor((void far *)&g_items, 2);
	Sound_dtor(g_snd_fx, 2);
	Sound_dtor(g_snd_music, 2);
	ani_dtor(g_screen_pic, 2);
	Font_dtor(g_font, 2);
	free_map(&g_map_w, 2);
	Nvf_dtor(g_nvf_heads, 2);
	Cache_dtor(g_cd_cache, 2);
	File_dtor(g_file_speech_ext, 2);
	File_dtor(g_file_cd, 2);
	File_dtor(g_file_raw, 2);
	File_dtor(g_file_speech, 2);
	arc_delete(g_arc_fx, 2);
	File_dtor(g_file_fx, 2);
	arc_delete(g_file_3, 2);
	gb_delete(g_bmp_spare, 2);
	Win_dtor(g_dialog, 0);
	File_dtor(g_file_main, 2);
	gb_delete(&g_bmp_mark_ae, 2);
	gb_delete(&g_bmp_mark_le, 2);
	gb_delete(g_cursor_bg_bmp, 2);
	gb_delete(g_cursor_bmp, 2);
	Init_dtor(g_init, 2);
}

/* ===================================================================================================================
 * Globale Daten (Phase B, 19.10.2026): von tools/dsconv.py in Adressreihenfolge erzeugt und von Hand ergaenzt
 * (Zaubertabellen, Verbotslisten). Objekte von Klassen mit Konstruktor (Bilder, Gruppen ...) stehen hier nur als
 * Speicher unter ihrem Adressnamen; angesprochen werden sie ueber die typisierten Namen aus s_data.h, konstruiert
 * von globals_ctor. Auf Zeichenketten zeigen Tabellen wie g_file_names.
 * =================================================================================================================== */
/* Zeigerziele, die erst weiter unten definiert werden */
extern char g_39b5[], g_39c0[], g_39ca[], g_39d7[], g_39e3[], g_39ee[], g_39fa[], g_3a03[], g_3a0b[], g_3a12[], g_3a18[], g_3a23[], g_3a2d[], g_3a35[], g_3a3c[], g_3a47[], g_3a53[];
/* Phase B: eigene Daten des Moduls (ds:1966..3c5e, dsana) - initialisierte Daten in Adressreihenfolge */
signed short g_nr_raw_files = 450;
signed short g_data_on_cd = 0;
struct recipe g_recipes[14] = {
	{ 167, { 42, 89, 122, 122, 126, 126, -1, -1, -1, -1 }, 166, 0, 8, 48 },
	{ 169, { 41, 122, 126, -1, -1, -1, -1, -1, -1, -1 }, 168, 0, 5, 8 },
	{ 202, { 92, 125, 125, 126, 122, -1, -1, -1, -1, -1 }, 180, 10, 5, 24 },
	{ 203, { 92, 92, 89, 41, 41, -1, -1, -1, -1, -1 }, 239, 0, 5, 8 },
	{ 204, { 92, 129, 126, 63, 63, -1, -1, -1, -1, -1 }, 153, 15, 8, 48 },
	{ 205, { 92, 129, 126, 124, 128, -1, -1, -1, -1, -1 }, 147, 12, 7, 48 },
	{ 206, { 92, 126, 126, 157, 157, 131, -1, -1, -1, -1 }, 154, 7, 8, 48 },
	{ 186, { 92, 60, 60, 64, -1, -1, -1, -1, -1, -1 }, 145, 5, 2, 8 },
	{ 240, { 42, 61, 61, 64, 92, 126, -1, -1, -1, -1 }, 146, 12, 5, 24 },
	{ 242, { 91, 63, 64, -1, -1, -1, -1, -1, -1, -1 }, 237, 2, 2, 8 },
	{ 243, { 92, 126, 126, 126, 157, 157, 157, 131, -1, -1 }, 155, 15, 12, 120 },
	{ 244, { 92, 157, 128, 122, 126, -1, -1, -1, -1, -1 }, 238, 0, 6, 48 },
	{ 241, { 92, 61, 61, 124, 123, 126, 130, -1, -1, -1 }, 236, 25, 10, 72 },
	{ 338, { 295, 225, 181, 107, 70, 315, -1, -1, -1, -1 }, 339, 40, 15, -106 }
};
struct skill_attribs g_skill_attribs[NR_SKILLS] = {
	{ 0, 0, 0, 1 },
	{ 0, 0, 0, 1 },
	{ 0, 0, 0, 1 },
	{ 0, 0, 0, 1 },
	{ 0, 0, 0, 1 },
	{ 0, 0, 0, 1 },
	{ 0, 0, 0, 1 },
	{ 0, 0, 0, 1 },
	{ 0, 0, 0, 1 },
	{ 0, 4, 6, 2 },
	{ 0, 4, 6, 2 },
	{ 0, 5, 4, 2 },
	{ 2, 4, 6, 2 },
	{ 0, 5, 4, 2 },
	{ 0, 4, 6, 2 },
	{ 0, 6, 6, 2 },
	{ 0, 5, 4, 2 },
	{ 2, 4, 4, 2 },
	{ 1, 5, 6, 2 },
	{ 1, 5, 2, 2 },
	{ 5, 2, 2, 2 },
	{ 0, 1, 2, 2 },
	{ 1, 5, 2, 2 },
	{ 0, 5, 2, 2 },
	{ 1, 5, 2, 2 },
	{ 1, 5, 5, 2 },
	{ 1, 5, 4, 2 },
	{ 3, 4, 6, 2 },
	{ 1, 5, 3, 2 },
	{ 1, 5, 5, 2 },
	{ 0, 1, 5, 2 },
	{ 5, 3, 4, 2 },
	{ 0, 1, 3, 3 },
	{ 1, 1, 5, 3 },
	{ 1, 1, 5, 3 },
	{ 1, 1, 5, 3 },
	{ 1, 5, 2, 3 },
	{ 0, 1, 2, 3 },
	{ 1, 1, 3, 3 },
	{ 1, 1, 3, 3 },
	{ 1, 5, 2, 3 },
	{ 0, 5, 2, 2 },
	{ 5, 2, 3, 2 },
	{ 0, 2, 3, 2 },
	{ 0, 1, 5, 2 },
	{ 0, 1, 2, 2 },
	{ 1, 2, 3, 2 },
	{ 1, 5, 3, 2 },
	{ 5, 3, 3, 2 },
	{ 0, 5, 3, 2 },
	{ 1, 5, 5, 1 },
	{ 1, 5, 5, 1 }
};
struct range_idx g_skills_index[7] = { { 0, 9 }, { 9, 10 }, { 19, 7 }, { 26, 6 }, { 32, 9 }, { 41, 9 }, { 50, 2 } };
CAST_PMF g_spell_handlers[86] = {	/* Zauberroutine je Zauber SP_* ([0] = keiner) */
	0,
	&Cast::spell_beherrschung_brechen,	/* SP_BEHERRSCHUNG_BRECHEN */
	&Cast::spell_destructibo,	/* SP_DESTRUCTIBO */
	&Cast::spell_gardianum,	/* SP_GARDIANUM */
	&Cast::spell_illusionen_zerstoeren,	/* SP_ILLUSIONEN_ZERSTOEREN */
	&Cast::spell_verwandlung_beenden,	/* SP_VERWANDLUNG_BEENDEN */
	&Cast::spell_band_und_fessel,	/* SP_BAND_UND_FESSEL */
	&Cast::spell_bannbaladin,	/* SP_BANNBALADIN */
	&Cast::spell_boeser_blick,	/* SP_BOESER_BLICK */
	&Cast::spell_grosse_gier,	/* SP_GROSSE_GIER */
	&Cast::spell_grosse_verwirrung,	/* SP_GROSSE_VERWIRRUNG */
	&Cast::spell_herr_ueber_das_tierreich,	/* SP_HERR_UEBER_DAS_TIERREICH */
	&Cast::spell_horriphobus,	/* SP_HORRIPHOBUS */
	&Cast::spell_magischer_raub,	/* SP_MAGISCHER_RAUB */
	&Cast::spell_respondami,	/* SP_RESPONDAMI */
	&Cast::spell_sanftmut,	/* SP_SANFTMUT */
	&Cast::spell_somnigravis,	/* SP_SOMNIGRAVIS */
	&Cast::spell_zwingtanz,	/* SP_ZWINGTANZ */
	&Cast::spell_furor_blut,	/* SP_FUROR_BLUT */
	&Cast::spell_geister_austreiben,	/* SP_GEISTER_AUSTREIBEN */
	&Cast::spell_geister_beschwoeren,	/* SP_GEISTER_BESCHWOEREN */
	&Cast::spell_heptagon,	/* SP_HEPTAGON */
	&Cast::spell_kraehenruf,	/* SP_KRAEHENRUF */
	&Cast::spell_skelettarius,	/* SP_SKELETTARIUS */
	&Cast::spell_elementare_beschwoeren,	/* SP_ELEMENTARE_BESCHWOEREN */
	&Cast::spell_nihilatio_gravitas,	/* SP_NIHILATIO_GRAVITAS */
	&Cast::spell_solidirid,	/* SP_SOLIDIRID */
	&Cast::spell_axxeleratus,	/* SP_AXXELERATUS */
	&Cast::spell_foramen,	/* SP_FORAMEN */
	&Cast::spell_motoricus,	/* SP_MOTORICUS */
	&Cast::spell_spurlos,	/* SP_SPURLOS */
	&Cast::spell_transversalis,	/* SP_TRANSVERSALIS */
	&Cast::spell_ueber_eis,	/* SP_UEBER_EIS */
	&Cast::spell_balsam,	/* SP_BALSAM */
	&Cast::spell_hexenspeichel,	/* SP_HEXENSPEICHEL */
	&Cast::spell_klarum_purum,	/* SP_KLARUM_PURUM */
	&Cast::spell_ruhe_koerper,	/* SP_RUHE_KOERPER */
	&Cast::spell_tiere_besprechen,	/* SP_TIERE_BESPRECHEN */
	&Cast::spell_adleraug,	/* SP_ADLERAUG */
	&Cast::spell_analues,	/* SP_ANALUES */
	&Cast::spell_eigenschaften,	/* SP_EIGENSCHAFTEN */
	&Cast::spell_exposami,	/* SP_EXPOSAMI */
	&Cast::spell_odem_arcanum,	/* SP_ODEM_ARCANUM */
	&Cast::spell_penetrizzel,	/* SP_PENETRIZZEL */
	&Cast::spell_sensibar,	/* SP_SENSIBAR */
	&Cast::spell_chamaelioni,	/* SP_CHAMAELIONI */
	&Cast::spell_duplicatus,	/* SP_DUPLICATUS */
	&Cast::spell_harmlose_gestalt,	/* SP_HARMLOSE_GESTALT */
	&Cast::spell_hexenknoten,	/* SP_HEXENKNOTEN */
	&Cast::spell_blitz,	/* SP_BLITZ */
	&Cast::spell_ecliptifactus,	/* SP_ECLIPTIFACTUS */
	&Cast::spell_eisenrost,	/* SP_EISENROST */
	&Cast::spell_fulminictus,	/* SP_FULMINICTUS */
	&Cast::spell_ignifaxius,	/* SP_IGNIFAXIUS */
	&Cast::spell_plumbumbarum,	/* SP_PLUMBUMBARUM */
	&Cast::spell_radau,	/* SP_RADAU */
	&Cast::spell_saft_kraft,	/* SP_SAFT_KRAFT */
	&Cast::spell_scharfes_auge,	/* SP_SCHARFES_AUGE */
	&Cast::spell_hexenblick,	/* SP_HEXENBLICK */
	&Cast::spell_nekropathia,	/* SP_NEKROPATHIA */
	&Cast::spell_adler_wolf,	/* SP_ADLER_WOLF */
	&Cast::spell_arcano_psychostabilis,	/* SP_ARCANO_PSYCHOSTABILIS */
	&Cast::spell_armatrutz,	/* SP_ARMATRUTZ */
	&Cast::spell_ch_steigern,	/* SP_CH_STEIGERN */
	&Cast::spell_feuerbann,	/* SP_FEUERBANN */
	&Cast::spell_ff_steigern,	/* SP_FF_STEIGERN */
	&Cast::spell_ge_steigern,	/* SP_GE_STEIGERN */
	&Cast::spell_in_steigern,	/* SP_IN_STEIGERN */
	&Cast::spell_kk_steigern,	/* SP_KK_STEIGERN */
	&Cast::spell_kl_steigern,	/* SP_KL_STEIGERN */
	&Cast::spell_mu_steigern,	/* SP_MU_STEIGERN */
	&Cast::spell_mutabili,	/* SP_MUTABILI */
	&Cast::spell_paralue,	/* SP_PARALUE */
	&Cast::spell_salander,	/* SP_SALANDER */
	&Cast::spell_in_see_und_fluss,	/* SP_IN_SEE_UND_FLUSS */
	&Cast::spell_visibili,	/* SP_VISIBILI */
	&Cast::spell_abvenenum,	/* SP_ABVENENUM */
	&Cast::spell_aeolitus,	/* SP_AEOLITUS */
	&Cast::spell_brenne_toter_stoff,	/* SP_BRENNE_TOTER_STOFF */
	&Cast::spell_claudibus,	/* SP_CLAUDIBUS */
	&Cast::spell_dunkelheit,	/* SP_DUNKELHEIT */
	&Cast::spell_weiches_erstarre,	/* SP_WEICHES_ERSTARRE */
	&Cast::spell_flim_flam,	/* SP_FLIM_FLAM */
	&Cast::spell_hartes_schmelze,	/* SP_HARTES_SCHMELZE */
	&Cast::spell_silentium,	/* SP_SILENTIUM */
	&Cast::spell_sturmgebruell	/* SP_STURMGEBRUELL */
};
struct staffspell_desc g_staffspell_descriptions[7] = {
	{ 1, 1, 2, 1, 21, 1 },
	{ 1, 1, 2, 0, 23, 0 },
	{ 1, 2, 2, 3, 19, 0 },
	{ 1, 2, 2, 4, 27, 0 },
	{ 0, 1, 1, 99, 30, 0 },
	{ 0, 2, 2, 99, 26, 0 },
	{ 0, 1, 2, 99, 21, 2 }
};
struct spell_desc g_spell_descriptions[87] = {
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 0, 1, 5, 2, -1, 0, 1, 2, 0, 0, 155 },
	{ 1, 1, 5, 3, -1, -1, 0, 0, 0, 0, 144 },
	{ 1, 5, 2, 6, -1, 1, 1, 0, 0, 0, 155 },
	{ 1, 1, 5, 2, -1, 1, 0, 1, 1, 0, 155 },
	{ 2, 0, 1, 2, -1, 0, 1, 2, 0, 0, 155 },
	{ 5, 1, 2, 6, 12, 1, 0, 3, 0, 1, 146 },
	{ 3, 5, 2, 2, 8, 1, 0, 1, 0, 1, 146 },
	{ 0, 0, 2, 2, 8, 1, 0, 1, 0, 1, 146 },
	{ 2, 1, 5, 2, 8, 1, 0, 1, 0, 1, 146 },
	{ 0, 1, 1, 2, 8, 1, 0, 1, 0, 1, 146 },
	{ 0, 0, 0, 2, -1, 1, 0, 1, 0, 1, 146 },
	{ 1, 0, 5, 2, 7, 1, 0, 1, 0, 1, 101 },
	{ 0, 0, 1, 6, 6, -1, 0, 2, 0, 0, 146 },
	{ 1, 1, 5, 2, 5, -1, 1, 0, 0, 1, 146 },
	{ 2, 0, 2, 2, -1, 1, 1, 1, 0, 1, 146 },
	{ 5, 1, 2, 2, 8, 1, 0, 3, 1, 1, 146 },
	{ 0, 0, 1, 2, 6, 1, 0, 1, 1, 1, 146 },
	{ 1, 0, 0, 2, -1, 1, 7, 4, 0, 0, 147 },
	{ 0, 0, 0, 2, 13, -1, 0, 0, 0, 0, 147 },
	{ 0, 0, 0, 2, 11, -1, 0, 0, 0, 0, 147 },
	{ 1, 0, 0, 2, -1, 1, 7, 4, 0, 0, 147 },
	{ 2, 0, 2, 2, 13, 1, 2, 3, 1, 0, 147 },
	{ 1, 0, 0, 2, 15, 1, 5, 1, 0, 0, 147 },
	{ 0, 0, 1, 2, -1, 1, 8, 4, 0, 0, 156 },
	{ 1, 1, 6, 6, -1, -1, 0, 0, 0, 0, 156 },
	{ 4, 5, 4, 6, -1, -1, 0, 0, 0, 0, 156 },
	{ 5, 1, 4, 6, 7, 1, 1, 2, 0, 0, 145 },
	{ 1, 1, 1, 3, 6, -1, 0, 0, 0, 0, 145 },
	{ 1, 1, 3, 6, -1, -1, 0, 0, 0, 0, 145 },
	{ 5, 5, 4, 4, 7, -1, 0, 0, 0, 0, 145 },
	{ 1, 1, 1, 6, -1, -1, 0, 0, 0, 0, 149 },
	{ 4, 1, 4, 6, 5, -1, 0, 0, 0, 0, 145 },
	{ 3, 1, 5, 2, -1, 0, 1, 2, 0, 0, 153 },
	{ 2, 5, 2, 3, -1, 0, 1, 2, 0, 0, 153 },
	{ 1, 1, 1, 2, -1, 0, 1, 2, 0, 0, 153 },
	{ 4, 1, 2, 6, 7, -1, 1, 2, 0, 0, 153 },
	{ 2, 0, 5, 2, -1, -1, 1, 0, 0, 0, 153 },
	{ 3, 1, 5, 3, 5, 0, 1, 0, 0, 0, 149 },
	{ 1, 1, 1, 5, 10, -1, 0, 0, 0, 0, 149 },
	{ 3, 1, 5, 2, 7, 1, 1, 1, 1, 1, 149 },
	{ 5, 1, 1, 5, 5, 1, 1, 0, 0, 0, 149 },
	{ 3, 1, 5, 2, 5, -1, 1, 0, 0, 0, 149 },
	{ 1, 1, 1, 6, 5, -1, 1, 0, 0, 0, 149 },
	{ 3, 1, 5, 2, 5, -1, 1, 0, 0, 1, 149 },
	{ 5, 5, 2, 4, 5, 1, 1, 0, 0, 0, 151 },
	{ 1, 1, 2, 4, 8, 1, 1, 2, 0, 0, 151 },
	{ 2, 1, 2, 4, 6, -1, 1, 0, 0, 0, 151 },
	{ 2, 1, 5, 2, 4, 1, 3, 4, 0, 0, 151 },
	{ 5, 1, 5, 4, 5, 1, 0, 3, 1, 0, 143 },
	{ 1, 0, 1, 6, -1, 1, 0, 0, 0, 0, 154 },
	{ 1, 1, 2, 4, 5, 1, 0, 3, 0, 0, 154 },
	{ 3, 1, 4, 6, -1, 1, 0, 3, 1, 0, 154 },
	{ 1, 1, 4, 3, -1, 1, 0, 3, 1, 0, 154 },
	{ 1, 2, 4, 6, 5, 1, 0, 3, 1, 1, 154 },
	{ 2, 0, 5, 2, 4, 1, 0, 3, 1, 0, 154 },
	{ 1, 0, 5, 2, -1, 1, 1, 2, 0, 0, 154 },
	{ 5, 5, 4, 3, 5, 1, 1, 2, 0, 0, 154 },
	{ 2, 5, 5, 2, 2, -1, 1, 0, 0, 0, 152 },
	{ 1, 0, 1, 2, 11, -1, 1, 0, 0, 0, 152 },
	{ 5, 0, 5, 4, 7, -1, 1, 0, 0, 0, 148 },
	{ 1, 0, 1, 2, 7, 1, 1, 2, 1, 0, 148 },
	{ 3, 5, 4, 6, -1, 1, 1, 0, 0, 0, 148 },
	{ 1, 1, 2, 2, 7, 0, 1, 2, 0, 0, 148 },
	{ 2, 0, 0, 6, 7, 0, 1, 0, 0, 0, 148 },
	{ 5, 1, 2, 3, 7, 0, 1, 2, 0, 0, 148 },
	{ 3, 1, 2, 4, 7, 0, 1, 2, 0, 0, 148 },
	{ 4, 1, 5, 2, 7, 0, 1, 2, 0, 0, 148 },
	{ 4, 1, 2, 6, 7, 0, 1, 2, 0, 0, 148 },
	{ 1, 1, 1, 2, 7, 0, 1, 2, 0, 0, 148 },
	{ 4, 0, 1, 2, 7, 0, 1, 2, 0, 0, 148 },
	{ 1, 1, 4, 6, -1, -1, 0, 0, 0, 0, 148 },
	{ 1, 5, 2, 6, 13, 1, 0, 3, 1, 1, 148 },
	{ 1, 1, 5, 2, -1, 1, 4, 1, 0, 1, 150 },
	{ 3, 0, 1, 6, -1, -1, 1, 0, 0, 0, 148 },
	{ 3, 1, 1, 4, -1, -1, 1, 0, 0, 0, 148 },
	{ 5, 1, 1, 3, 5, -1, 0, 0, 0, 0, 157 },
	{ 4, 1, 2, 6, 5, -1, 0, 0, 0, 0, 158 },
	{ 1, 0, 1, 6, -1, -1, 0, 0, 0, 0, 157 },
	{ 1, 1, 3, 6, -1, -1, 0, 0, 0, 0, 157 },
	{ 0, 1, 1, 3, 3, 1, 1, 0, 0, 0, 157 },
	{ 1, 0, 1, 6, -1, -1, 0, 0, 0, 0, 157 },
	{ 4, 1, 1, 3, 1, -1, 0, 0, 0, 0, 157 },
	{ 1, 0, 1, 6, -1, -1, 0, 0, 0, 0, 157 },
	{ 3, 1, 1, 2, -1, -1, 0, 0, 0, 0, 157 },
	{ 4, 1, 2, 6, 5, -1, 0, 0, 0, 0, 157 },
	{ -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
};
struct range_idx g_spells_index[8] = { { 1, 5 }, { 6, 12 }, { 18, 6 }, { 24, 3 }, { 27, 6 }, { 33, 5 }, { 38, 7 }, { 45, 4 } };
struct range_idx g_spells_index2[4] = { { 49, 9 }, { 58, 2 }, { 60, 16 }, { 76, 10 } };
struct mon_spell_desc g_mon_spell_descriptions[15] = {
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 20, 1, 0, 0, 1, 2, 0, 1, 148 },
	{ 8, 2, 0, 5, 2, 2, 1, 0, 146 },
	{ 8, 2, 0, 0, 2, 2, 1, 0, 146 },
	{ 7, 2, 0, 0, 5, 2, 1, 0, 101 },
	{ 7, 1, 0, 1, 4, 6, 0, 1, 145 },
	{ -1, 1, 0, 1, 5, 2, 0, 1, 153 },
	{ 5, 3, 1, 1, 5, 4, 1, 0, 143 },
	{ 5, 2, 1, 1, 2, 4, 0, 0, 157 },
	{ -1, 3, 1, 1, 4, 6, 1, 0, 154 },
	{ -1, 3, 1, 1, 4, 3, 1, 0, 154 },
	{ 5, 3, 1, 2, 4, 6, 1, 0, 146 },
	{ -1, 1, 0, 0, 5, 2, 0, 1, 153 },
	{ -1, 0, 0, 5, 4, 6, 0, 1, 154 },
	{ 13, 3, 0, 5, 2, 6, 1, 0, 146 }
};
signed char g_mon_spell_repertoire[65] = {
	1, 7, 8, -1, -1, 1, 7, 3, 4, -1, 1, 7, 4, 2, 9, 9,
	14, 10, -1, -1, 9, 10, 12, 13, -1, 9, 10, 12, 11, 7, 9, 14,
	7, -1, -1, 9, 14, 7, 12, 4, 6, 1, 8, -1, -1, 6, 1, 5,
	7, -1, 6, 14, 5, 12, 9, 7, 9, 6, -1, -1, 7, 9, 6, 4,
	10
};
SPELL_PMF g_mon_spell_handlers[15] = {	/* Zauberroutine je Gegnerzauber MSP_* ([0] = keiner) */
	0,
	&Spell::mon_spell_verwandlung_beenden,	/* MSP_VERWANDLUNG_BEENDEN */
	&Spell::mon_spell_bannbaladin,	/* MSP_BANNBALADIN */
	&Spell::mon_spell_boeser_blick,	/* MSP_BOESER_BLICK */
	&Spell::mon_spell_horriphobus,	/* MSP_HORRIPHOBUS */
	&Spell::mon_spell_axxeleratus,	/* MSP_AXXELERATUS */
	&Spell::mon_spell_balsam,	/* MSP_BALSAM */
	&Spell::mon_spell_blitz,	/* MSP_BLITZ */
	&Spell::mon_spell_eisenrost,	/* MSP_EISENROST */
	&Spell::mon_spell_fulminictus,	/* MSP_FULMINICTUS */
	&Spell::mon_spell_ignifaxius,	/* MSP_IGNIFAXIUS */
	&Spell::mon_spell_plumbumbarum,	/* MSP_PLUMBUMBARUM */
	&Spell::mon_spell_saft_kraft,	/* MSP_SAFT_KRAFT */
	&Spell::mon_spell_armatrutz,	/* MSP_ARMATRUTZ */
	&Spell::mon_spell_paralue	/* MSP_PARALUE */
};
signed short g_spells_own_test[20] = { 3, 33, 34, 37, 50, 53, 25, 26, 30, 62, 74, 75, 78, 79, 8, 21, 24, 18, 49, -1 };
signed short g_spells_no_sfight[16] = { 6, 7, 8, 9, 10, 12, 16, 17, 18, 21, 24, 40, 54, 72, 73, -1 };
char g_save_runv[1] = "";
signed long g_ingame_timers[50] = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0
};
char g_save_runv_end[1] = "";
char g_save_play[1] = "";
signed char g_anims_on = 0;
signed char g_free_move = 0;
char g_playflags_2[1] = { 0 };   /* ds:253a = g_playflags[2] (O011 PSET/PVIEW greift per Index zu) */
signed char g_subtitles = 0;
signed char g_print_to_file = 0;
signed char g_text_speed = 0;
char g_unused_253e[2] = { 0 };   /* ds:253e, unbenutzt */
signed char g_anim_speed = 0;
signed char g_fight_speed = 0;
signed char g_opt_fight_keep_view = 0;
signed char g_win_centered = 0;
signed char g_opt_music = 0;
signed char g_opt_sound = 0;
signed char g_cd_speech = 0;
signed char g_vol_music = 0;
signed char g_vol_fx = 0;
signed char g_vol_speech = 0;
signed char g_game_mode = 0;
signed char g_saver_level = 0;
signed char g_print_width = 0;
signed char g_print_page_len = 0;
char g_unused_254e[1] = { 0 };   /* ds:254e, unbenutzt */
signed char g_inv_show_body = 0;
char g_unused_2550[26] = { 0 };   /* ds:2550, unbenutzt */
signed char g_flags[1] = "";
signed char g_topic_kvirasim = 0;
signed char g_topic_gashok = 0;
signed char g_topic_lowangen = 0;
signed char g_topic_tiefhusen = 0;
signed char g_topic_tjolmar = 0;
signed char g_topic_finsterkoppen = 0;
signed char g_topic_ansvell = 0;
signed char g_topic_salamandersteine = 0;
signed char g_topic_finsterkamm = 0;
signed char g_topic_svellttal = 0;
signed char g_topic_svelltsuempfe = 0;
signed char g_topic_thasch = 0;
signed char g_topic_blutzinnen = 0;
signed char g_topic_rorwhed = 0;
signed char g_tev_peddler_trail = 0;
signed char g_npc6_met = 0;
char g_unused_257b[1] = { 0 };   /* ds:257b, unbenutzt */
signed char g_once_anim = 0;
signed char g_slow5_on = 0;
signed char g_slow4_on = 0;
signed char g_valley_asked = 0;
signed char g_mountain_asked = 0;
signed char g_armed_fight_lost = 0;
signed char g_tev099_rockslide = 0;
signed char g_salamander_reported = 0;
signed char g_tev_avalanche_done = 0;
signed char g_tev147_rockslide = 0;
signed char g_tev148_rockslide = 0;
signed char g_tev149_rockslide = 0;
signed char g_tev151_rockslide = 0;
char g_unused_2589[1] = { 0 };   /* ds:2589, unbenutzt */
signed char g_trial_lost = 0;
signed char g_shrine_state = 0;
signed char g_shrine_paid = 0;
signed char g_artherion_heard = 0;
signed char g_artherion_hut_known = 0;
signed char g_artherion_hut_visited = 0;
signed char g_artherion_done = 0;
char g_unused_2591[1] = { 0 };   /* ds:2591, unbenutzt */
signed char g_ferry_landed = 0;
signed char g_rondra_priestess_met = 0;
signed char g_ferry_robbed = 0;
signed char g_tev000_warrior_cursing = 0;
signed char g_scene31_due = 0;
signed char g_tev111_seen = 0;
char g_unused_2598[1] = { 0 };   /* ds:2598, unbenutzt */
signed char g_tev066_cub_killed = 0;
signed char g_finsterkopp_entered = 0;
signed char g_low_tunnel_known = 0;
signed char g_guest_lock[7] = "";
signed char g_guest_lock2[3] = "";
signed char g_topic_binge = 0;
char g_unused_25a7[1] = { 0 };   /* ds:25a7, unbenutzt */
signed char g_topic_salamander_stone = 0;
signed char g_topic_ingramosch = 0;
signed char g_topic_sternenschweif = 0;
signed char g_topic_siebenquell = 0;
signed char g_topic_egelbronn = 0;
char g_unused_25ad[1] = { 0 };   /* ds:25ad, unbenutzt */
signed char g_topic_artherion = 0;
signed char g_kvirasim_arrived = 0;
signed char g_rose_active = 0;
signed char g_rose_two_marks = 0;
signed char g_binge_key_state = 0;
char g_unused_25b3[1] = { 0 };   /* ds:25b3, unbenutzt */
signed char g_low25_topic59 = 0;
signed char g_lore_told = 0;
signed char g_low34_ch_passed = 0;
signed char g_low_kysira_ref = 0;
signed char g_music_played = 0;
signed char g_sites_shock_done = 0;
signed char g_sites_hint_done = 0;
signed char g_sites_paid = 0;
signed char g_sites_evening = 0;
signed char g_port_talk = 0;
signed char g_port_can_haggle = 0;
signed char g_port_paid = 0;
signed char g_port_poor = 0;
signed char g_port_deal = 0;
signed char g_ingramosch_talked = 0;
signed char g_dream_seen = 0;
signed char g_low_beggars_scared = 0;
signed char g_low_notices_read = 0;
signed char g_eolan_state = 0;
signed char g_low_deringorn_state = 0;
signed char g_low_brooch_state = 0;
signed char g_low_guard_talk = 0;
signed char g_low_gate1_open = 0;
signed char g_low_gate2_open = 0;
signed char g_low_dragan_task1 = 0;
signed char g_altrogolan_absent = 0;
signed char g_low_ailian_state = 0;
signed char g_vindaria_choice = 0;
signed char g_vindaria_state = 0;
signed char g_dragan_tip_jandora = 0;
signed char g_dragan_tip_brin = 0;
signed char g_dragan_tip_raisha = 0;
signed char g_low_falkert_known = 0;
signed char g_dragan_task = 0;
signed char g_low_dragan_news = 0;
signed char g_low_gavron_house = 0;
signed char g_guide_won = 0;
signed char g_dng02_tools_state = 0;
signed char g_dng02_jugs_state = 0;
signed char g_dng02_fishhook_taken = 0;
signed char g_dng02_otter_fur_taken = 0;
signed char g_dng02_dice_money_taken = 0;
signed char g_dng02_recipe_taken = 0;
signed char g_dng02_secret_switch = 0;
signed char g_dng02_keyhole_door = 0;
signed char g_dng02_sun_lock_open = 0;
signed char g_dng03_secret_door1 = 0;
signed char g_dng03_secret_door2 = 0;
signed char g_dng03_password_solved = 0;
signed char g_dng03_voice_heard = 0;
signed char g_dragon_claw_found = 0;
signed char g_coward_stuck = 0;
signed char g_dng03_combination_open = 0;
signed char g_dng03_combination_tries = 0;
signed char g_dng03_skeleton_warriors_171d = 0;
signed char g_dng03_skeletons_191c = 0;
signed char g_dng03_zombies_1519 = 0;
signed char g_dng03_skeleton_warriors_041d = 0;
signed char g_dng03_zombies_1b15 = 0;
signed char g_dng03_skeleton_warriors_0b17 = 0;
signed char g_dng03_zombies_1313 = 0;
signed char g_dng03_heshthot_130e = 0;
signed char g_dng03_sorcerer_080c = 0;
signed char g_dng03_mages_0702 = 0;
signed char g_dng03_mages_0704 = 0;
signed char g_dng03_priests_0d01 = 0;
signed char g_dng03_knight_0e1d = 0;
signed char g_dng03_desk_opened = 0;
signed char g_tavern_dinner_asked = 0;
signed char g_peddler_away = 0;
signed char g_peddler3_away = 0;
signed char g_place197_done = 0;
signed char g_dng04_path_281d = 0;
signed char g_dng04_path_271c = 0;
signed char g_dng04_rantzen_271b = 0;
signed char g_dng04_einbeeren_found = 0;
signed char g_dng04_belmart_found = 0;
signed char g_dng04_snake_0c19 = 0;
signed char g_dng04_snake_1817 = 0;
signed char g_dng04_snake_211a = 0;
signed char g_dng04_snake_2419 = 0;
signed char g_dng04_rantzen_1413 = 0;
signed char g_dng04_rantzen_2701 = 0;
signed char g_dng04_rantzen_left = 0;
signed char g_dng04_gulmond_found = 0;
signed char g_dng04_village_visited = 0;
char g_unused_260a[1] = { 0 };   /* ds:260a, unbenutzt */
signed char g_pic2_seen = 0;
signed char g_price_level = 0;
signed char g_dng04_hedge_cut = 0;
signed char g_dng04_rantzen_0405 = 0;
signed char g_dng04_rantzen_0e16 = 0;
signed char g_dng04_goblins_0109 = 0;
signed char g_dng04_goblins_040d = 0;
signed char g_dng04_rantzen_1e08 = 0;
signed char g_dng04_zombies_1219 = 0;
signed char g_dng04_zombies_2d0e = 0;
signed char g_dng04_zombies_0919 = 0;
signed char g_dng04_orcs_200c = 0;
signed char g_dng04_orcs_2510 = 0;
signed char g_dng04_orcs_0413 = 0;
signed char g_dng04_orcs_0601 = 0;
signed char g_dng04_orcs_1402 = 0;
signed char g_dng04_orcs_1c02 = 0;
signed char g_heather_recipe_read = 0;
signed char g_street_flag1 = 0;
signed char g_street_flag2 = 0;
signed char g_street_guards = 0;
signed char g_street_thugs = 0;
signed char g_camp_juggler_met = 0;
signed char g_camp_druid_met = 0;
signed char g_camp_hunter_met = 0;
signed char g_camp_vagabond_met = 0;
signed char g_tavgas_visits = 0;
signed char g_inn_vision_state = 0;
signed char g_swamp_chief_refused = 0;
signed char g_swamp_chief_offended = 0;
signed char g_dng04_monster_killed = 0;
signed char g_dng04_tower_open = 0;
signed char g_sites_challenges = 0;
signed char g_sites_rematch = 0;
signed char g_guide_avail = 0;
signed char g_finster_anger = 0;
signed char g_game_finished2 = 0;
signed char g_diary_new_items = 0;
signed char g_low_stone_delivered = 0;
signed char g_low_stone_days = 0;
signed char g_voyage_done = 0;
signed char g_elsurion_met = 0;
signed char g_dng13_smoke_timer = 0;
signed char g_dng10_countdown = 0;
signed char g_dng11_floor_collapsed = 0;
signed char g_binge_hole1_looted = 0;
signed char g_binge_hole2_trapped = 0;
signed char g_dng10_tools_found = 0;
signed char g_dng10_parchment_taken = 0;
signed char g_binge_torches_lit = 0;
signed char g_binge_shaft_stone_ap = 0;
signed char g_dng10_flask_taken = 0;
signed char g_dng12_pit_side = 0;
signed char g_port_haggled = 0;
signed char g_port_price = 0;
signed char g_dng12_cart_seen = 0;
signed char g_dng12_lever = 0;
signed char g_dng10_ring_taken = 0;
signed char g_sacrilege = 0;
signed char g_low_burglary_result = 0;
signed char g_dng05_supplies_taken = 0;
signed char g_dng05_recipe_taken = 0;
char g_unused_2649[1] = { 0 };   /* ds:2649, unbenutzt */
signed char g_dng05_pit_seen = 0;
signed char g_dng05_switch_0114 = 0;
signed char g_sites_vision_ok = 0;
signed char g_sites_vision_failed = 0;
signed char g_sites_night_met = 0;
signed char g_trnd_once1_done = 0;
signed char g_trnd_once2_done = 0;
signed char g_trnd_twice_state = 0;
signed char g_dng05_finale_seen = 0;
signed char g_dng05_bolt_trap_armed = 0;
signed char g_dng05_offering_made = 0;
signed char g_dng05_skeletons_beaten = 0;
signed char g_dng05_scroll_taken = 0;
signed char g_dng05_riddle_solved = 0;
signed char g_dng05_puzzle_won = 0;
signed char g_dng05_door_070d_found = 0;
signed char g_dng05_door_0402_found = 0;
signed char g_dng05_door_1106_found = 0;
signed char g_dng06_noise = 0;
signed char g_dng06_alarm = 0;
signed char g_dng07_noise = 0;
signed char g_dng07_alarm = 0;
signed char g_dng08_noise = 0;
signed char g_dng08_alarm = 0;
char g_unused_2662[2] = { 0 };   /* ds:2662, unbenutzt */
signed char g_dng06_rack_done = 0;
signed char g_dng06_orcs_1b04 = 0;
signed char g_dng06_helmet_taken = 0;
signed char g_dng06_statue_looted = 0;
signed char g_dng06_statue_destroyed = 0;
signed char g_dng06_statue_ignored = 0;
signed char g_dng06_hornets_destroyed = 0;
signed char g_dng06_pit_1305 = 0;
signed char g_dng06_pit_1519 = 0;
signed char g_dng06_rockfall1 = 0;
signed char g_dng06_rockfall2 = 0;
signed char g_dng06_skeleton_dug = 0;
signed char g_dng06_beer_drunk = 0;
signed char g_dng06_dagger_taken = 0;
signed char g_dng06_dogs_state = 0;
signed char g_dng06_tripwire_seen = 0;
signed char g_dng06_tripwire_hit = 0;
signed char g_dng06_hay_state = 0;
signed char g_dng06_latrine_seen = 0;
signed char g_dng06_hall_seen = 0;
signed char g_dng06_beer_drained = 0;
signed char g_dng06_ogre_beer_tasted = 0;
signed char g_dng06_stew_eaten = 0;
signed char g_dng06_orcs_0e10 = 0;
signed char g_dng06_bolt_wall_armed = 0;
signed char g_dng06_beetle_pit_armed = 0;
signed char g_dng06_beetle_pit_fallen = 0;
signed char g_dng07_dogs_door_2c = 0;
signed char g_dng07_dogs_door_2e = 0;
signed char g_dng07_kennel1_searched = 0;
signed char g_dng07_kennel2_searched = 0;
signed char g_dng07_guards_beaten = 0;
signed char g_dng07_shelf_searched = 0;
signed char g_dng07_shelf_hole_found = 0;
signed char g_dng07_orcs_1606 = 0;
signed char g_dng07_orcs_0d08 = 0;
signed char g_dng07_orcs_100b = 0;
signed char g_dng07_orcs_140b = 0;
signed char g_dng07_orcs_120f = 0;
signed char g_dng07_store_seen = 0;
signed char g_dng07_statue_scraped = 0;
signed char g_dng07_rockfall1 = 0;
signed char g_dng07_rockfall2 = 0;
signed char g_dng07_pit1_fallen = 0;
signed char g_dng07_maggots_done = 0;
signed char g_dng07_cave_seen = 0;
signed char g_dng07_junk_finds = 0;
signed char g_dng07_beetles_1b15 = 0;
signed char g_dng07_beetles_1d17 = 0;
signed char g_dng07_beetles_181c = 0;
signed char g_dng07_feed_room_seen = 0;
signed char g_dng07_orcs_0a0d = 0;
signed char g_dng07_latrine_seen = 0;
signed char g_dng07_orcs_050f = 0;
signed char g_dng07_orcs_0305 = 0;
signed char g_dng07_pit2_fallen = 0;
signed char g_dng07_pit3_fallen = 0;
signed char g_dng07_orcs_0319 = 0;
signed char g_dng07_stew_eaten = 0;
signed char g_dng07_kitchen_looted = 0;
signed char g_dng07_straw_amulet = 0;
signed char g_dng07_corpse_sword_taken = 0;
signed char g_dng07_web_131a = 0;
signed char g_dng07_web_1321 = 0;
signed char g_dng07_spiders_1321 = 0;
signed char g_dng07_web_1022 = 0;
signed char g_dng07_spiders_1022 = 0;
signed char g_dng07_web_1024 = 0;
signed char g_dng07_spiders_0e25 = 0;
signed char g_dng07_web_0e25 = 0;
signed char g_dng07_web_0922 = 0;
signed char g_dng07_spiders_0922 = 0;
signed char g_dng07_web_0720 = 0;
signed char g_dng07_web_0320 = 0;
signed char g_dng07_spiders_0320 = 0;
signed char g_dng07_web_0927 = 0;
signed char g_dng07_spiders_0927 = 0;
signed char g_dng07_web_032b = 0;
signed char g_dng07_spiders_032b = 0;
signed char g_dng07_nest_destroyed = 0;
signed char g_dng07_rockfall3 = 0;
signed char g_dng07_rockfall4 = 0;
signed char g_dng07_niche_looted = 0;
signed char g_dng07_amulet_found = 0;
signed char g_dng08_smithy_seen = 0;
signed char g_dng08_storeroom_shown = 0;
signed char g_dng08_banner_torn = 0;
signed char g_dng08_hatchet_taken = 0;
signed char g_dng08_orcs_0d04 = 0;
signed char g_dng08_orcs_0e06 = 0;
signed char g_dng08_orcs_0a06 = 0;
signed char g_dng08_ogres_1603 = 0;
char g_unused_26c0[1] = { 0 };   /* ds:26c0, unbenutzt */
signed char g_low_vindaria_known = 0;
char g_unused_26c2[1] = { 0 };   /* ds:26c2, unbenutzt */
signed char g_binge_hole3_looted = 0;
signed char g_dng10_junk_found = 0;
signed char g_dng16_plate_found = 0;
signed char g_dng16_plate_lifted = 0;
signed char g_dng16_pit_trap = 0;
signed char g_dng16_casket_taken = 0;
signed char g_dng16_double_key_state = 0;
signed char g_dng10_vision_ap = 0;
signed char g_dng11_lever_on = 0;
signed char g_dng11_plate_down = 0;
signed char g_dng11_trap1_found = 0;
signed char g_dng11_trap2_found = 0;
signed char g_dng11_block_down = 0;
char g_unused_26d0[1] = { 0 };   /* ds:26d0, unbenutzt */
signed char g_dng11_winch_state = 0;
signed char g_dng13_wheel_turned = 0;
signed char g_dng14_plate_tried = 0;
signed char g_dng11_plate_seen = 0;
signed char g_dng11_trap3_found = 0;
signed char g_dng14_plate_tries = 0;
signed char g_dng11_trap2_fired = 0;
signed char g_dng11_trap3_fired = 0;
signed char g_place083_attacked = 0;
signed char g_binge_anvil_hours = 0;
char g_unused_26db[1] = { 0 };   /* ds:26db, unbenutzt */
signed char g_dng11_wardrobe_seen = 0;
signed char g_dng14_waterskin_found = 0;
signed char g_dng14_plate_lifted = 0;
signed char g_dng11_kobold_met = 0;
signed char g_dng11_riddle_solved = 0;
signed char g_mine_cart_pos = 0;
signed char g_dng16_plates_dir = 0;
signed char g_dng16_grid_moved = 0;
signed char g_dng16_hero_wet = 0;
signed char g_dng12_beams_done = 0;
signed char g_dng12_beams_seen = 0;
signed char g_dng12_debris_ap = 0;
signed char g_ingerimm_verse_read = 0;
signed char g_tev_lowangen_key_noted = 0;
signed char g_ambush_count = 0;
signed char g_dng10_chest_opened = 0;
signed char g_dng12_skeleton_found = 0;
signed char g_parchments_read = 0;
signed char g_topic_gavron = 0;
signed char g_dng16_chest_looted = 0;
signed char g_dng04_village_thanked = 0;
signed char g_dng10_silver_key_in_lock = 0;
signed char g_dng16_sword_done = 0;
signed char g_finale_done = 0;
signed char g_dng10_gold_key_in_lock = 0;
signed char g_quest_anim_seen = 0;
signed char g_guest_lock_t2 = 0;
signed char g_guest_lock_t19 = 0;
signed char g_guest_lock_t23 = 0;
signed char g_guest_lock_t24 = 0;
signed char g_dng14_in_pool = 0;
signed char g_dng12_stash28_empty = 0;
signed char g_dng11_ramp_falls = 0;
signed char g_dng13_ambush_won = 0;
signed char g_sites_night_done = 0;
char g_unused_26ff[1] = { 0 };   /* ds:26ff, unbenutzt */
signed char g_dng16_plate_steps = 0;
signed char g_dng16_sign_read = 0;
signed char g_dng16_sacrilege = 0;
signed char g_dng11_ramp_armed = 0;
signed char g_dng16_stone_taken = 0;
signed char g_dng16_forging = 0;
signed char g_dng16_golem_fought = 0;
signed char g_dng16_medallion_given = 0;
signed char g_dng16_forge_timer = 0;
signed char g_dng16_wall_timer = 0;
signed char g_dng16_artifact_traded = 0;
signed char g_dng16_leather_taken = 0;
signed char g_sites_explored = 0;
char g_unused_270d[2] = { 0 };   /* ds:270d, unbenutzt */
signed char g_low_arrival_due = 0;
signed char g_dng08_shaman_room_shown = 0;
signed char g_dng08_mummy_fought = 0;
signed char g_dng08_sarcophagus_opened = 0;
signed char g_dng08_whirlweed_found = 0;
signed char g_dng08_hall_0c15 = 0;
signed char g_dng08_pit_0e15 = 0;
signed char g_dng08_pit_armed = 0;
signed char g_dng08_collar_131c = 0;
signed char g_dng08_collar_0d10 = 0;
signed char g_dng08_orc_veteran_1907 = 0;
signed char g_dng08_orcs_1e19 = 0;
signed char g_dng08_orcs_1e15 = 0;
signed char g_dng08_orcs_1a11 = 0;
signed char g_dng08_orcs_1516 = 0;
signed char g_dng08_orcs_121b = 0;
signed char g_dng08_ogres_0e19 = 0;
signed char g_dng08_orcs_1113 = 0;
signed char g_dng08_orcs_1110 = 0;
signed char g_dng08_orcs_0d0f = 0;
signed char g_dng08_orcs_0f0c = 0;
signed char g_dng08_grain_1e0e = 0;
signed char g_dng08_grain_1e0f = 0;
signed char g_dng08_grain_1e10 = 0;
signed char g_dng08_grain_1e0d = 0;
signed char g_dng13_stash29_empty = 0;
signed char g_dng08_goulash_eaten = 0;
signed char g_dng08_straw_0612 = 0;
signed char g_dng08_hall_0415 = 0;
signed char g_dng08_trap_0213_defused = 0;
signed char g_dng08_lever = 0;
signed char g_dng08_hall_fight = 0;
signed char g_dng08_orcs_0511 = 0;
signed char g_dng08_orcs_020d = 0;
signed char g_dng08_orcs_0309 = 0;
signed char g_dng02_druids_1c05 = 0;
signed char g_dng02_cultists_0802 = 0;
signed char g_dng02_cultists_0402 = 0;
signed char g_dng02_cultists_020b = 0;
signed char g_dng02_cultists_070d = 0;
signed char g_dng02_cultists_041c = 0;
signed char g_dng02_sabretooth_0b17 = 0;
signed char g_dng02_sabretooth_0f0c = 0;
signed char g_dng02_sabretooth_0f1c = 0;
signed char g_dng02_cave_spiders_150c = 0;
signed char g_dng02_amulet1_timer = 0;
signed char g_dng02_amulet2_timer = 0;
signed char g_dng02_amulet3_timer = 0;
signed char g_dng02_amulet4_timer = 0;
signed char g_dng02_cultists_171d = 0;
signed char g_dng02_cultists_1c1b = 0;
signed char g_dng02_step_toggle = 0;
signed char g_jail_freed = 0;
signed char g_mugging_state = 0;
signed char g_gashok_arrested = 0;
signed char g_blackmage_done = 0;
signed char g_pending_msg = 0;
signed char g_dng05_mummies_0e1d = 0;
signed char g_dng05_mummies_091f = 0;
signed char g_dng05_mummies_021e = 0;
char g_unused_274b[1] = { 0 };   /* ds:274b, unbenutzt */
signed char g_dng05_priests_0819 = 0;
signed char g_dng05_priests_0a0e = 0;
signed char g_dng05_priests_0a0a = 0;
char g_unused_274f[2] = { 0 };   /* ds:274f, unbenutzt */
signed char g_dng05_skeletons_020e = 0;
char g_unused_2752[1] = { 0 };   /* ds:2752, unbenutzt */
signed char g_dng05_priests_1c0d = 0;
signed char g_dng05_priests_1501 = 0;
signed char g_sites_reward = 0;
signed char g_guide_count = 0;
signed char g_npc8_days = 0;
signed char g_gavron_chase = 0;
char g_unused_2759[1] = { 0 };   /* ds:2759, unbenutzt */
signed char g_low_bridge_y = 0;
signed char g_dng04_mage_killed = 0;
signed char g_dng07_prison_event = 0;
signed char g_dng07_prison_days = 0;
signed char g_dng07_talk1_done = 0;
signed char g_dng07_talk2_done = 0;
signed char g_low_dragan_task3 = 0;
signed char g_dng04_monster_fought = 0;
signed char g_finster_refuse_msg = 0;
signed char g_dng01_grate_first = 0;
signed char g_dng01_hooks_held = 0;
signed char g_dng01_skull_state = 0;
signed char g_tjolmar_vault = 0;
signed char g_dng01_thorn_tree_state = 0;
signed char g_dng01_branch_state = 0;
signed char g_dng01_cultists_0806 = 0;
signed char g_dng01_cultists_0e06 = 0;
signed char g_dng01_troll_fought = 0;
signed char g_dng01_ice_state = 0;
signed char g_dng01_hesinde_done = 0;
signed char g_dng01_ghouls_fought = 0;
signed char g_dng01_illusion_wall2 = 0;
signed char g_dng01_illusion_wall1 = 0;
signed char g_dng01_cave_spiders_1808 = 0;
char g_unused_2772[1] = { 0 };   /* ds:2772, unbenutzt */
signed char g_phex_finale_done = 0;
signed char g_dng05_poltergeist = 0;
signed char g_dng05_skeletons_1214 = 0;
signed char g_dng05_skeletons_1a1d = 0;
signed char g_dng05_skeletons_1b1e = 0;
signed char g_dng05_skeletons_1c1d = 0;
signed char g_dng05_skeletons_1b1c = 0;
signed char g_dng05_wall_1a1b_open = 0;
signed char g_lock_trap_off = 0;
signed char g_dng08_copper_disc_taken = 0;
signed char g_dng06_orc_veteran_1d02 = 0;
signed char g_dng06_orcs_1503 = 0;
signed char g_dng06_orcs_0f03 = 0;
signed char g_dng06_orcs_0402 = 0;
signed char g_dng06_orcs_1009 = 0;
signed char g_dng07_brandy_taken = 0;
signed char g_dng08_latrine_armed = 0;
signed char g_dng03_thief_pending = 0;
signed char g_dng03_sorcerer_0e0c = 0;
signed char g_dng03_stone_stolen = 0;
signed char g_final_fight_won = 0;
signed char g_dng01_cave_spiders_0101 = 0;
signed char g_dng06_straw_1502 = 0;
signed char g_dng06_straw_0d02 = 0;
signed char g_dng06_straw_100b = 0;
signed char g_dng06_straw_0a08 = 0;
signed char g_dng06_straw_0b08 = 0;
signed char g_dng06_straw_0f0a = 0;
signed char g_sfight_seen = 0;
signed char g_dng06_ogre_weapons_smashed = 0;
signed char g_tev082_thicket_done = 0;
signed char g_dng07_was_prisoner = 0;
signed char g_sfight_won = 0;
signed char g_low_tunnel_arrival = 0;
signed char g_travel_state = 0;
signed char g_low_brooch_taken = 0;
signed char g_salamander_stolen = 0;
signed char g_dng11_air_counter = 0;
signed char g_reward_given = 0;
signed char g_reward2_given = 0;
char g_unused_279b[39] = { 0 };   /* ds:279b, unbenutzt */
char g_save_play_end[1] = "";
char g_save_glob[1] = "";
signed char g_inn_room = 0;
signed char g_group_sizes[6] = "";
signed long g_god_favor[2] = { 0, 0 };
signed long g_god_favor_rondra = 0;
char g_unused_27d7[8] = { 0 };   /* ds:27d7, unbenutzt */
signed long g_god_favor_boron = 0;
signed long g_god_favor_hesinde = 0;
char g_unused_27e7[4] = { 0 };   /* ds:27e7, unbenutzt */
signed long g_god_favor_tsa = 0;
char g_god_favor_9_10[8] = { 0 };   /* ds:27ef = g_god_favor[9..10] (O007 greift per Index zu) */
signed long g_god_favor_ingerimm = 0;
char g_unused_27fb[12] = { 0 };   /* ds:27fb, unbenutzt */
signed char g_npc_months = 0;
char g_loc_saved_flag[1] = "";   /* ds:2808, nur O030 enter_location (dort g_2808): beim Betreten gesichert und 0, danach zurueck - Bedeutung offen */
signed short g_nr_groups = 1;
signed short g_group_no = 0;
signed short g_inn_days = 0;
signed short g_inn_town = 0;
signed short g_inn_loc = 0;
signed short g_inn_price = 0;
signed short g_camp_food_mod = 5;
signed short g_camp_herb_mod = 0;
signed short g_rest_mod = 0;
char g_absent_hero1[16] = "";
char g_absent_hero2[16] = "";
signed long g_dng06_alarm_delay = 0;
signed short g_loc_id = 0;
signed short g_loc_name = 0;
signed long g_dng10_altar_coins = 3530;
signed long g_dng16_chest_coins = 500000;
char g_unused_284b[20] = { 0 };   /* ds:284b, unbenutzt */
signed short g_exit_dir = 0;
signed short g_house_hint_seen[17] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
signed char g_area_type = 0;
struct hero far *g_dng01_hook_hero = 0;
char g_camp_job_names[6][16] = { "", "", "", "", "", "" };
signed char g_camp_herb_hours = 0;
signed short g_dng05_offered_ducats = 0;
signed short g_dng05_poltergeist_ticks = 0;
char g_save_glob_end[1] = "";
char g_save_mons[1] = "";
signed char g_mon_known[123] = "";
char g_save_mons_end[1] = "";
char g_save_locs[1] = "";
struct smith_repair g_smith_repairs[12] = {
	{ 0, "" },
	{ 0, "" },
	{ 0, "" },
	{ 0, "" },
	{ 0, "" },
	{ 0, "" },
	{ 0, "" },
	{ 0, "" },
	{ 0, "" },
	{ 0, "" },
	{ 0, "" },
	{ 0, "" }
};
signed char g_temple_desecrated[29] = "";
signed char g_inn_closed[27] = "";
signed char g_tavern_banned[26] = "";
signed char g_smith_banned[21] = "";
signed char g_shop_banned[24] = "";
signed char g_peddler_banned = 0;
signed char g_peddler3_banned = 0;
signed char g_lodging_banned[5] = "";
signed char g_inn_has_room[4] = "";
signed char g_inn_room_rolls[1] = "";
signed char g_shop_sells_food[8] = "";
signed char g_shop_food_rolls[5] = "";
signed char g_shed_free_today = 0;
signed char g_dragan_room_today = 0;
char g_market_rolls_head[2] = { 0 };   /* ds:2a72, Anfang von g_market_rolls (O005; g_markets = dieselbe Tabelle ab +2) */
struct market g_markets[3] = { { 0, "", 0, 0, "", { 5, 0 } }, { 0, { 1, 0 }, 5, 0, "", { 8, 0 } }, { 0, { 6, 0 }, 8, 0, "", { -1, -1 } } };
char g_unused_2a95[9] = { 0 };   /* ds:2a95, unbenutzt */
char g_save_locs_end[1] = "";
signed short g_items_plural2[29] = {
	89, 90, 55, 56, 57, 58, 59, 141, 142, 143, 144, 41, 166, 168, 180, 189,
	82, 84, 83, 238, 239, 39, 74, 275, 314, 315, 322, 247, -1
};
signed short g_items_plural[13] = { 35, 50, 51, 52, 90, 189, 250, 288, 287, 192, 221, 312, -1 };
/* je Heldentyp: Gegenstaende, die er nicht benutzen kann (Liste, Ende -1; can_use_item) */
signed short g_forbidden_items_01[] = {	/* HERO_TYPE_JUGGLER */
	0x050, ITEM_BATTLE_AXE, 0x071, 0x067, ITEM_TWO_HANDED_SWORD, 0x014, ITEM_CROSSBOW, 0x065, 0x06f, 0x036, 0x06a,
	0x052, 0x077, 0x072, 0x09e, 0x0b2, 0x0c5, 0x0c6, 0x116, ITEM_HEAVY_CROSSBOW, ITEM_SILVER_MACE, ITEM_MACE,
	0x13d, ITEM_MAGE_STAFF, ITEM_WITCHES_BROOM, 0x069, 0x02b, ITEM_MACE_12A, 0x15c, -1
};
signed short g_forbidden_items_02[] = {	/* HERO_TYPE_HUNTER */
	ITEM_BATTLE_AXE, 0x071, 0x067, 0x050, ITEM_TWO_HANDED_SWORD, 0x014, ITEM_CROSSBOW, 0x065, 0x06f, 0x052, 0x036,
	0x06a, 0x077, 0x072, 0x09e, 0x0b2, 0x0c5, 0x0c6, 0x116, ITEM_HEAVY_CROSSBOW, ITEM_SILVER_MACE, ITEM_MACE,
	0x13d, ITEM_MAGE_STAFF, ITEM_WITCHES_BROOM, 0x069, 0x02b, ITEM_MACE_12A, 0x15c, 0x11b, 0x11d, -1
};
signed short g_forbidden_items_03[] = {	/* HERO_TYPE_WARRIOR */
	ITEM_ANGST_POISON, ITEM_SHURIN_POISON, ITEM_LOTUS_POISON, ITEM_KROETEN_POISON, ITEM_ARAX_POISON,
	ITEM_SLEEP_POISON, ITEM_GOLDLEIM, ITEM_BANNSTAUB, ITEM_KUKRIS, ITEM_WITCHES_BROOM, ITEM_KUKRIS_DAGGER,
	ITEM_KUKRIS_MENGBILAR, 0x13d, ITEM_MAGE_STAFF, 0x11b, 0x11d, -1
};
signed short g_forbidden_items_04[] = {	/* HERO_TYPE_ESTRAY */
	ITEM_BATTLE_AXE, 0x071, 0x067, 0x050, ITEM_TWO_HANDED_SWORD, 0x014, 0x065, 0x06f, 0x052, 0x06a, 0x036, 0x077,
	0x072, 0x09e, 0x0b2, 0x0c5, 0x0c6, 0x116, ITEM_HEAVY_CROSSBOW, ITEM_SILVER_MACE, ITEM_MACE, 0x13d,
	ITEM_MAGE_STAFF, ITEM_WITCHES_BROOM, 0x069, 0x02b, ITEM_MACE_12A, 0x15c, 0x11b, 0x11d, -1
};
signed short g_forbidden_items_05[] = {	/* HERO_TYPE_THORWALIAN */
	0x071, 0x067, ITEM_TWO_HANDED_SWORD, 0x014, 0x065, 0x06f, 0x06a, 0x077, 0x072, 0x09e, 0x0b2, ITEM_BATTLE_AXE,
	ITEM_SILVER_MACE, ITEM_MACE, 0x13d, 0x09e, ITEM_MAGE_STAFF, ITEM_WITCHES_BROOM, 0x069, ITEM_MACE_12A, 0x11b,
	0x11d, -1
};
signed short g_forbidden_items_06[] = {	/* HERO_TYPE_DWARF */
	0x06f, 0x077, ITEM_TWO_HANDED_SWORD, 0x072, ITEM_PIKE, 0x067, 0x06a, 0x068, ITEM_HEAVY_CROSSBOW,
	ITEM_BATTLE_AXE, 0x13d, 0x071, 0x0b2, 0x09e, ITEM_MAGE_STAFF, ITEM_WITCHES_BROOM, 0x069, 0x15c, 0x11b, 0x11d,
	-1
};
signed short g_forbidden_items_07[] = {	/* HERO_TYPE_WITCH */
	ITEM_SPEAR, 0x002, ITEM_SWORD, ITEM_SABRE, 0x070, ITEM_SKRAJA, 0x006, ITEM_MACE, ITEM_WAR_AXE, 0x088,
	ITEM_SHORTBOW, ITEM_BATTLE_AXE, 0x044, 0x043, 0x068, 0x071, 0x089, ITEM_CROSSBOW, ITEM_RAPIER, ITEM_PIKE,
	0x067, 0x010, 0x063, 0x064, ITEM_TWO_HANDED_SWORD, ITEM_LONGBOW, 0x014, 0x065, 0x066, 0x076, 0x06f, 0x06a,
	0x074, 0x073, 0x077, 0x072, 0x069, ITEM_SEAL_KILLER, 0x08b, ITEM_WAR_AXE_8, ITEM_LEATHER_ARMOR, 0x007, 0x051,
	0x00f, 0x050, 0x052, 0x054, 0x053, ITEM_POT_HELMET, 0x04e, 0x02b, 0x036, 0x09e, 0x0a1, 0x09f, 0x0a0, 0x0b2,
	ITEM_SABER_MAGIC, 0x0c7, 0x0c5, 0x0c6, ITEM_SILVER_MACE, ITEM_SPEAR_D8, ITEM_ARTHERION_SWORD,
	ITEM_ARTHERION_BOW, ITEM_HEAVY_CROSSBOW, 0x102, ITEM_ORC_SABRE, 0x114, 0x115, 0x116, 0x11c,
	ITEM_ASTHENIL_SWORD, ITEM_MACE_12A, 0x13d, ITEM_MAGE_STAFF, 0x021, 0x0c4, 0x15c, 0x11b, 0x11d, -1
};
signed short g_forbidden_items_08[] = {	/* HERO_TYPE_DRUID */
	ITEM_SPEAR, ITEM_SWORD, ITEM_SABRE, 0x070, ITEM_SKRAJA, 0x006, ITEM_MACE, ITEM_WAR_AXE, 0x088,
	ITEM_BATTLE_AXE, 0x044, 0x043, 0x071, 0x089, ITEM_CROSSBOW, ITEM_RAPIER, ITEM_PIKE, 0x067, 0x010, 0x063,
	0x064, ITEM_TWO_HANDED_SWORD, 0x014, 0x065, 0x066, 0x076, 0x06f, 0x06a, 0x074, 0x073, 0x077, 0x072, 0x069,
	ITEM_SEAL_KILLER, 0x08b, ITEM_WAR_AXE_8, 0x007, 0x00f, 0x050, 0x06b, 0x06c, 0x052, 0x053, ITEM_POT_HELMET,
	0x02b, ITEM_MENGBILAR, 0x036, 0x09e, 0x09f, 0x0a1, 0x0a0, 0x0b2, ITEM_SABER_MAGIC, 0x0c7, 0x0c5, 0x0c6,
	ITEM_SILVER_MACE, ITEM_SPEAR_D8, ITEM_ARTHERION_SWORD, ITEM_HEAVY_CROSSBOW, 0x102, ITEM_ORC_SABRE, 0x114,
	0x115, 0x116, 0x11c, ITEM_ASTHENIL_SWORD, ITEM_MACE_12A, 0x13d, ITEM_MAGE_STAFF, ITEM_WITCHES_BROOM,
	ITEM_DAGGER, 0x011, 0x021, ITEM_HEAVY_DAGGER, 0x0da, 0x0e0, ITEM_KUKRIS_DAGGER, ITEM_KUKRIS_MENGBILAR, 0x15c,
	0x11b, 0x11d, ITEM_SHORTBOW, ITEM_LONGBOW, ITEM_ORNATE_SWORD, 0x15d, ITEM_ARTHERION_BOW, -1
};
signed short g_forbidden_items_09[] = {	/* HERO_TYPE_MAGE */
	ITEM_SPEAR, 0x002, ITEM_SWORD, ITEM_SABRE, 0x070, ITEM_SKRAJA, 0x006, ITEM_MACE, ITEM_WAR_AXE, 0x088,
	ITEM_SHORTBOW, ITEM_BATTLE_AXE, 0x044, 0x043, 0x068, 0x071, 0x089, ITEM_CROSSBOW, ITEM_RAPIER, ITEM_PIKE,
	0x067, 0x010, 0x063, 0x064, ITEM_TWO_HANDED_SWORD, ITEM_LONGBOW, 0x014, 0x065, 0x066, 0x076, 0x06f, 0x06a,
	0x074, 0x073, 0x077, 0x072, 0x069, ITEM_SEAL_KILLER, 0x08b, ITEM_WAR_AXE_8, ITEM_LEATHER_ARMOR, 0x007, 0x051,
	0x00f, 0x050, 0x052, 0x054, 0x053, ITEM_POT_HELMET, 0x04e, 0x02b, 0x036, 0x09e, 0x09f, 0x0a0, 0x0a1, 0x0b2,
	ITEM_SABER_MAGIC, 0x0c7, 0x0c5, 0x0c6, ITEM_SILVER_MACE, ITEM_SPEAR_D8, ITEM_ARTHERION_SWORD,
	ITEM_HEAVY_CROSSBOW, ITEM_ORC_SABRE, 0x114, 0x115, 0x116, 0x11c, ITEM_ASTHENIL_SWORD, ITEM_MACE_12A, 0x0c4,
	ITEM_ARTHERION_BOW, 0x13d, ITEM_WITCHES_BROOM, 0x021, 0x102, 0x15c, 0x11b, 0x11d, ITEM_HEAVY_DAGGER, 0x15d,
	0x06b, 0x06c, ITEM_MENGBILAR, ITEM_KUKRIS_MENGBILAR, ITEM_ORNATE_SWORD, 0x047, ITEM_FIGHTING_STAFF,
	ITEM_WITCHES_BROOM, -1
};
signed short g_forbidden_items_10[] = {	/* HERO_TYPE_GREEN_ELF */
	ITEM_BATTLE_AXE, 0x071, 0x067, ITEM_TWO_HANDED_SWORD, 0x014, 0x065, 0x06f, 0x06a, 0x052, 0x036, 0x077, 0x072,
	0x09e, 0x050, 0x053, 0x0c5, 0x0c6, 0x0b2, 0x116, 0x13d, ITEM_MAGE_STAFF, ITEM_WITCHES_BROOM, 0x069,
	ITEM_MACE_12A, ITEM_MACE, ITEM_SILVER_MACE, 0x15c, 0x11b, 0x11d, -1
};
signed short g_forbidden_items_11[] = {	/* HERO_TYPE_ICE_ELF */
	ITEM_BATTLE_AXE, 0x071, 0x067, ITEM_TWO_HANDED_SWORD, 0x014, 0x065, 0x06f, 0x06a, 0x052, 0x036, 0x077, 0x072,
	0x050, 0x053, 0x09e, 0x0b2, 0x0c5, 0x0c6, 0x116, 0x13d, ITEM_MAGE_STAFF, ITEM_WITCHES_BROOM, 0x069, 0x02b,
	ITEM_MACE_12A, ITEM_MACE, ITEM_SILVER_MACE, 0x15c, 0x11b, 0x11d, -1
};
signed short g_forbidden_items_12[] = {	/* HERO_TYPE_SYLVAN_ELF */
	ITEM_BATTLE_AXE, 0x071, 0x067, ITEM_TWO_HANDED_SWORD, 0x014, 0x065, 0x06f, 0x06a, 0x052, 0x036, 0x077, 0x072,
	0x050, 0x053, 0x09e, 0x0b2, 0x0c5, 0x0c6, 0x116, 0x13d, ITEM_MAGE_STAFF, ITEM_WITCHES_BROOM, 0x069, 0x02b,
	ITEM_MACE_12A, ITEM_MACE, ITEM_SILVER_MACE, 0x15c, 0x11b, 0x11d, -1
};
signed short far *g_forbidden_item_lists[12] = {	/* je Heldentyp 1..12 (Index type - 1) */
	g_forbidden_items_01, g_forbidden_items_02, g_forbidden_items_03, g_forbidden_items_04,
	g_forbidden_items_05, g_forbidden_items_06, g_forbidden_items_07, g_forbidden_items_08,
	g_forbidden_items_09, g_forbidden_items_10, g_forbidden_items_11, g_forbidden_items_12
};
struct ranged_weapon g_ranged_weapons_table[12] = {
	{ { 1, 0, 0, -1, -99, -99, -99 }, 6 },
	{ { 1, 1, 0, 0, -1, -99, -99 }, 6 },
	{ { 2, 1, 0, 0, 0, -99, -99 }, 7 },
	{ { 1, 1, 0, 0, 0, -1, -99 }, 4 },
	{ { 2, 2, 1, 0, 0, -1, -2 }, 4 },
	{ { 2, 2, 1, 0, -1, -2, -3 }, 3 },
	{ { 2, 2, 1, 1, 0, 0, -1 }, 4 },
	{ { 9, 9, 9, 9, 9, 9, 9 }, 2 },
	{ { 5, 5, 4, 3, 3, 2, 1 }, 4 },
	{ { 3, 3, 2, 1, 0, -1, -2 }, 3 },
	{ { 1, 1, 0, 0, 0, -1, -99 }, 4 },
	{ { -1, -1, -1, -1, -1, -1, -1 }, -1 }
};
struct weapon_desc g_weapons_table[78] = {
	{ 1, 4, 14, 2, -1, 0, 0, 2 },
	{ 1, 1, 14, 6, -1, -1, -3, 1 },
	{ 1, 3, 15, 2, -1, 0, 0, 2 },
	{ 1, 0, 16, 4, -1, -3, -4, 0 },
	{ 1, 3, 99, 5, 1, 0, -3, 6 },
	{ 1, 2, 15, 1, -1, 0, -1, 2 },
	{ 1, 4, 14, 5, -1, 0, -3, 3 },
	{ 1, 3, 99, 0, 3, 0, 0, 4 },
	{ 2, 4, 14, 3, -1, -1, -4, 3 },
	{ 1, 6, 99, 0, 5, 0, 0, 4 },
	{ 1, 1, 15, 3, -1, -2, -3, 0 },
	{ 1, 3, 99, 4, 2, -1, -3, 5 },
	{ 1, 1, 99, 0, 0, 0, 0, 5 },
	{ 2, 4, 14, 3, -1, -2, -3, 2 },
	{ 1, 4, 99, 0, 4, 0, 0, 4 },
	{ 1, 5, 15, 5, -1, -1, -3, 1 },
	{ 1, 0, 16, 6, -1, -2, -3, 0 },
	{ 1, 3, 99, 4, 2, -1, -4, 5 },
	{ 1, 3, 19, 7, -1, -1, -4, 1 },
	{ 1, 4, 13, 1, -1, 0, -2, 1 },
	{ 1, 3, 16, 3, -1, 0, -1, 2 },
	{ 1, 3, 16, 3, -1, 0, -1, 2 },
	{ 1, 1, 15, 5, -1, 0, -1, 1 },
	{ 1, 0, 19, 2, -1, 0, -6, 1 },
	{ 1, 0, 99, 4, 0, -3, -4, 5 },
	{ 1, 2, 17, 5, -1, -3, -4, 2 },
	{ 1, 3, 17, 6, -1, -3, -4, 2 },
	{ 2, 3, 15, 2, -1, -2, -4, 1 },
	{ 1, 3, 15, 3, -1, 0, -3, 2 },
	{ 1, 4, 15, 5, -1, -1, -3, 1 },
	{ 1, 2, 13, 6, -1, -2, -3, 1 },
	{ 1, 3, 18, 4, -1, -1, -1, 2 },
	{ 3, 3, 17, 4, -1, -3, -4, 1 },
	{ 1, 1, 16, 4, -1, -2, -3, 2 },
	{ 1, 2, 15, 4, -1, -2, -3, 2 },
	{ 1, 1, 16, 7, -1, -3, -4, 2 },
	{ 1, 2, 15, 2, -1, -1, -2, 2 },
	{ 2, 2, 15, 3, -1, -2, -2, 2 },
	{ 1, 3, 15, 2, -1, 0, -1, 2 },
	{ 1, 5, 14, 2, -1, -1, -2, 2 },
	{ 1, 6, 15, 1, -1, -2, -2, 2 },
	{ 1, 4, 16, 3, -1, -1, -3, 1 },
	{ 1, 5, 14, 1, -1, -1, -2, 1 },
	{ 1, 3, 16, 4, -1, 0, -1, 2 },
	{ 1, 4, 14, 2, -1, 0, -1, 2 },
	{ 1, 6, 15, 3, -1, -1, -3, 2 },
	{ 1, 2, 99, 0, 1, 0, 0, 5 },
	{ 1, 3, 13, 4, -1, 0, -2, 2 },
	{ 1, 4, 13, 2, -1, -2, -4, 3 },
	{ 1, 5, 14, 2, -1, -1, -3, 3 },
	{ 1, 4, 99, 0, -1, 0, 0, 5 },
	{ 1, 3, 15, 2, -1, 0, 0, 2 },
	{ 1, 3, 15, 2, -1, 0, -1, 0 },
	{ 0, 0, 99, 0, -1, 0, 0, 4 },
	{ 1, 5, 14, -5, -1, -2, -3, 2 },
	{ 1, 10, 14, -99, -1, -2, -8, 3 },
	{ 1, 2, 15, -99, -1, 2, -1, 2 },
	{ 1, 3, 15, 0, -1, 0, 0, 2 },
	{ 2, 4, 14, -99, -1, -3, -4, 2 },
	{ 1, 4, 14, -99, -1, 2, 2, 2 },
	{ 1, 4, 14, 0, -1, 2, 0, 2 },
	{ 1, 3, 99, 5, 6, 1, 1, 6 },
	{ 1, 0, 99, 4, 7, -3, -4, 5 },
	{ 1, 1, 15, -99, -1, 0, 0, 1 },
	{ 1, 5, 14, -5, -1, 3, -1, 2 },
	{ 1, 6, 99, 0, 8, 0, 0, 4 },
	{ 1, 7, 99, 0, 9, 0, 0, 4 },
	{ 1, 3, 99, 2, 2, -1, -4, 5 },
	{ 1, 4, 14, 4, -1, -5, -5, 2 },
	{ 1, 5, 14, 4, -1, -5, -5, 3 },
	{ 1, 6, 15, 2, -1, -2, -4, 3 },
	{ 1, 2, 99, 4, 10, -1, -3, 5 },
	{ 1, 4, 13, -99, -1, 0, -1, 1 },
	{ 1, 0, 16, 0, -1, -3, -4, 0 },
	{ 1, 1, 15, 0, -1, -2, -3, 0 },
	{ 1, 4, 14, 0, -1, 0, 0, 2 },
	{ 1, 3, 15, -99, -1, -2, -4, 2 },
	{ -1, -1, -1, -1, -1, -1, -1, -1 }
};
struct armor_desc g_armors_table[27] = {
	{ 1, 0 },
	{ 2, 1 },
	{ 3, 2 },
	{ 1, 0 },
	{ 0, 0 },
	{ 0, 0 },
	{ 0, 0 },
	{ 0, 0 },
	{ 3, 3 },
	{ 5, 4 },
	{ 0, 0 },
	{ 0, 0 },
	{ 2, 1 },
	{ 1, 0 },
	{ 2, 2 },
	{ 4, 4 },
	{ 3, 3 },
	{ 2, 1 },
	{ 1, 1 },
	{ 1, 0 },
	{ 1, 1 },
	{ 1, 0 },
	{ 2, 4 },
	{ 5, 3 },
	{ 4, 3 },
	{ 3, 0 },
	{ -1, -1 }
};
char g_unused_3221[1] = { 0 };   /* ds:3221, unbenutzt */
struct usable_desc g_usable_table[19] = {
	{ 0, 0, 2 },
	{ 5, 1, 0 },
	{ 1, 2, 0 },
	{ 1, 3, 2 },
	{ 3, 4, 0 },
	{ 6, 5, 0 },
	{ 1, 6, 0 },
	{ 3, 7, 0 },
	{ 1, 8, 0 },
	{ 1, 9, 0 },
	{ 1, 10, 0 },
	{ 1, 11, 0 },
	{ 1, 12, 0 },
	{ 10, 13, 0 },
	{ 10, 14, 0 },
	{ 1, 15, 0 },
	{ 1, 16, 0 },
	{ 1, 17, 0 },
	{ 1, 18, 0 }
};
char g_unused_325b[2] = { 0x01, 0x13 };   /* ds:325b, unbenutzt */
signed short g_poison_potions[10] = { 55, 56, 57, 58, 59, 141, 142, 143, 144, -1 };
signed short g_items_ask_use[7] = { 55, 56, 57, 58, 142, 144, -1 };
signed short g_herbs_toxic[5] = { 122, 126, 132, 62, -1 };
signed short g_herbs_uneatable[7] = { 128, 129, 130, 157, 156, 124, -1 };
signed short g_elixir_potions[8] = { 147, 148, 149, 150, 151, 152, 153, -1 };
signed short g_bad_elixirs[8] = { 226, 227, 228, 229, 230, 231, 232, -1 };
signed short g_items_attack[7] = { 239, 238, 251, 59, 141, 143, -1 };
signed short g_teleport_spin = -1;
signed short g_vis_start = 0;
signed short g_ems_cur_page = 32767;
char far *g_file_names[14] = {
	(char far *)&g_39b5, (char far *)&g_39c0, (char far *)&g_39ca, (char far *)&g_39d7, (char far *)&g_39e3, (char far *)&g_39ee, (char far *)&g_39fa, (char far *)&g_3a03, (char far *)&g_3a0b, (char far *)&g_3a12, (char far *)&g_3a18, (char far *)&g_3a23, (char far *)&g_3a2d, (char far *)&g_3a35
};
struct rect g_clip_rect = { 0, 0, 319, 199 };
struct mouse_range g_mouse_range = { 0, 0, 319, 199 };
char g_cursor_bmp[10] = "";
char g_cursor_bg_bmp[10] = "";
char g_3327[10] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };   /* ds:3327, Speicher von g_bmp_mark_le (struct Bmp, Konstruktor in globals_ctor) */
char g_3331[16] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0x3f, 0x3f };   /* ds:3331, Speicher von g_bmp_mark_ae (struct Bmp, Konstruktor in globals_ctor) */
struct point g_gui_buttons_pos[9] = {
	{ 241, 57 },
	{ 268, 57 },
	{ 295, 57 },
	{ 241, 84 },
	{ 268, 84 },
	{ 295, 84 },
	{ 241, 111 },
	{ 268, 111 },
	{ 295, 111 }
};
char g_gui_buttons_rest[280] = {
	0xf0, 0x00, 0x00, 0x00, 0x40, 0x01, 0x1c, 0x00, 0xfd, 0x00, 0xf0, 0x00, 0x1e, 0x00, 0x40, 0x01,
	0x38, 0x00, 0xe3, 0x00, 0xd0, 0x00, 0x6d, 0x00, 0xe2, 0x00, 0x88, 0x00, 0xe1, 0x00, 0xcc, 0x00,
	0x76, 0x00, 0xcf, 0x00, 0x88, 0x00, 0xe1, 0x00, 0xc8, 0x00, 0x81, 0x00, 0xcb, 0x00, 0x88, 0x00,
	0xe1, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x04, 0x00, 0xe0, 0x00, 0x10, 0x00, 0x24, 0x00,
	0x43, 0x00, 0x60, 0x00, 0x00, 0x4b, 0xac, 0x00, 0x24, 0x00, 0xdf, 0x00, 0x60, 0x00, 0x00, 0x4d,
	0x44, 0x00, 0x04, 0x00, 0xab, 0x00, 0x33, 0x00, 0x00, 0x48, 0x44, 0x00, 0x59, 0x00, 0xab, 0x00,
	0x88, 0x00, 0x00, 0x50, 0xf1, 0x00, 0x39, 0x00, 0x08, 0x01, 0x50, 0x00, 0x81, 0x00, 0x0c, 0x01,
	0x39, 0x00, 0x23, 0x01, 0x50, 0x00, 0x82, 0x00, 0x27, 0x01, 0x39, 0x00, 0x3e, 0x01, 0x50, 0x00,
	0x83, 0x00, 0xf1, 0x00, 0x54, 0x00, 0x08, 0x01, 0x6b, 0x00, 0x84, 0x00, 0x0c, 0x01, 0x54, 0x00,
	0x23, 0x01, 0x6b, 0x00, 0x85, 0x00, 0x27, 0x01, 0x54, 0x00, 0x3e, 0x01, 0x6b, 0x00, 0x86, 0x00,
	0xf1, 0x00, 0x6f, 0x00, 0x08, 0x01, 0x86, 0x00, 0x87, 0x00, 0x0c, 0x01, 0x6f, 0x00, 0x23, 0x01,
	0x86, 0x00, 0x88, 0x00, 0x27, 0x01, 0x6f, 0x00, 0x3e, 0x01, 0x86, 0x00, 0x89, 0x00, 0x03, 0x00,
	0x9d, 0x00, 0x2d, 0x00, 0xc5, 0x00, 0xf1, 0x00, 0x2e, 0x00, 0x9d, 0x00, 0x58, 0x00, 0xc5, 0x00,
	0xf2, 0x00, 0x59, 0x00, 0x9d, 0x00, 0x83, 0x00, 0xc5, 0x00, 0xf3, 0x00, 0x84, 0x00, 0x9d, 0x00,
	0xae, 0x00, 0xc5, 0x00, 0xf4, 0x00, 0xaf, 0x00, 0x9d, 0x00, 0xd9, 0x00, 0xc5, 0x00, 0xf5, 0x00,
	0xda, 0x00, 0x9d, 0x00, 0x04, 0x01, 0xc5, 0x00, 0xf6, 0x00, 0x13, 0x01, 0x9d, 0x00, 0x3d, 0x01,
	0xc5, 0x00, 0xf7, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0x01, 0xc7, 0x00, 0xfe, 0x00, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
};   /* ds:3365 Fortsetzung der Knopftabelle (O188 liest ds:3365 = g_gui_buttons_pos_y + 0x22) */
struct mouse_area g_fig_mouse_areas[2] = { { 0, 0, 319, 199, 254 }, { -1, -1, -1, -1, -1 } };
signed short g_portrait_x[7] = { 3, 46, 89, 132, 175, 218, 277 };
struct mouse_area g_dlg_areas[14] = {
	{ 0, 0, 319, 199, 1 },
	{ 0, 0, 319, 199, 2 },
	{ 0, 0, 319, 199, 3 },
	{ 0, 0, 319, 199, 4 },
	{ 0, 0, 319, 199, 5 },
	{ 0, 0, 319, 199, 6 },
	{ 0, 0, 319, 199, 7 },
	{ 0, 0, 319, 199, 8 },
	{ 0, 0, 319, 199, 9 },
	{ 0, 0, 319, 199, 10 },
	{ 0, 0, 319, 199, 11 },
	{ 0, 0, 319, 199, 12 },
	{ 0, 0, 319, 199, 13 },
	{ -1, -1, -1, -1, -1 }
};
struct mouse_area g_action_table_merchant[2] = { { 16, 4, 223, 138, 144 }, { -1, -1, -1, -1, -1 } };
signed short g_disease_prices[8] = { 0, 6, 4, 9, 8, 8, 7, 10 };
signed short g_disease_delays[8] = { 0, -10, 20, -20, -30, -20, 0, -30 };
signed short g_poison_prices[10] = { 0, 6, 5, 4, 5, 5, 4, 10, 12, 15 };
signed short g_poison_delays[10] = { 0, -30, -25, -20, -25, -25, -20, -50, -60, -75 };
char far *g_path_fmts[3] = { (char far *)&g_3a3c, (char far *)&g_3a47, (char far *)&g_3a53 };
char g_dsa_version_string[18] = "DSA VERSION\0V0.01";
signed char g_version_a = 0;
signed char g_version_b = 1;
signed char g_version_c = 2;
signed char g_version_d = 1;
signed short g_portrait_marks[14] = { 0, 0, 0, 0, 0, 0, 0, 18499, 17221, 17995, 20041, 17732, -13230, 267 };
char g_unused_35c5[1] = { 0 };   /* ds:35c5, unbenutzt */
char g_file_main[56] = "";
char g_file_3[92] = "";
char g_file_fx[56] = "";
char g_arc_fx[92] = "";
char g_file_speech[56] = "";
char g_file_raw[56] = "";
char g_file_cd[56] = "";
char g_file_speech_ext[56] = "";
char g_cd_cache[22] = "";
char g_font[16] = "";
void far *g_font_dst = 0;
char g_font_rest[39] = { 0 };   /* ds:37f8 Rest des Schriftobjekts g_font (SEG020 liest ds:3819) */
signed short g_font_tab0 = 0;
char g_font_tabs_rest[12] = { 0 };   /* ds:3821 g_font.tabs[1..6] hinter g_font_tab0 (O009 Dlg::layout) */
char g_snd_music[36] = "";
char g_unused_3851[41] = { 0 };	/* ds:3851, unbenutzt; zugleich die gefaltete Basis g_fight_defs_ab - 0xab (O140) */
signed short g_music_off = 0;
signed short g_music_ready = 0;
char g_unused_387e[20] = { 0 };   /* ds:387e, unbenutzt */
signed short g_music_id_req = 0;
char g_unused_3894[34] = { 0 };   /* ds:3894, unbenutzt */
char g_snd_fx[77] = "";
signed short g_speech_off = 0;
signed short g_sound_ready = 0;
char g_snd_fx_rest[20] = { 0 };   /* ds:3907 Teil des Klangobjekts (SEG004 liest ds:3919) */
signed short g_speech_pos = 0;
char g_unused_391d[34] = { 0 };   /* ds:391d, unbenutzt */
signed short g_sheet_enabled = 1;
signed short g_portrait_drag = 1;
char g_unused_3943[2] = { 0 };   /* ds:3943, unbenutzt */
signed short g_paused = 0;
signed short g_temple_visited = 1;
signed short g_temple_visited2 = 1;
signed short g_game_state = 0;
char g_unused_394d[2] = { 0xff, 0xff };   /* ds:394d, unbenutzt */
signed long g_fight_xms = 0;
signed short g_input_preset = 0;
signed char g_hero_excluded = -1;
signed char g_sheet_hunger_shown = -1;
signed char g_sheet_thirst_shown = -1;
signed short g_dialog_depth = 0;
signed char g_loaded_dungeon = 0;
signed char g_loaded_town = 0;
char g_loc_exit_mark[1] = "";   /* ds:395c, nur O030 enter_location (dort g_395c) setzt am Ende -1 - Bedeutung offen */
signed char g_fight_screen_on = 0;
signed char g_fight_flag = 0;
signed short g_fight_end = 0;
signed long g_timer_ticks = 0;
signed long g_timer_minutes = 0;
signed long g_timer_5min = 0;
signed long g_timer_15min = 0;
signed long g_timer_hours = 0;
signed long g_timer_4hours = 0;
signed long g_timer_days = 0;
signed long g_timer_months = 0;
signed long g_timer_years = 0;
signed short g_at_1000 = 0;
signed short g_at_0800 = 0;
signed short g_at_1400 = 0;
signed short g_at_2000_2200 = 0;
signed short g_at_0600_0900 = 0;
unsigned short g_idle_ticks = 0x0000;
signed short g_saver_due = 0;
unsigned char g_loc_herb = 0xff;
signed short g_resting = 0;
signed short g_leave_location = 0;
signed short g_speech_ext_avail = 0;
signed short g_cd_music = 0;
signed long g_disk_free = 0;
char g_unused_39a0[4] = { 0x40, 0x1f, 0x08, 0x07 };   /* ds:39a0, unbenutzt */
signed short g_mem_too_low = 0;
signed short g_first_start = 0;
signed long g_disk_needed = 10485760;
char g_str_temp_dir[9] = "\\..\\TEMP";
char g_39b5[11] = "STARCD.DAT";   /* ds:39b5, Zeigerziel */
char g_39c0[10] = "STARCD.FN";   /* ds:39c0, Zeigerziel */
char g_39ca[13] = "SPEECHCD.DAT";   /* ds:39ca, Zeigerziel */
char g_39d7[12] = "SPEECHCD.FN";   /* ds:39d7, Zeigerziel */
char g_39e3[11] = "X:\\DATA\\%s";   /* ds:39e3, Zeigerziel */
char g_39ee[12] = "..\\CTEMP\\%s";   /* ds:39ee, Zeigerziel */
char g_39fa[9] = "STAR.DAT";   /* ds:39fa, Zeigerziel */
char g_3a03[8] = "STAR.FN";   /* ds:3a03, Zeigerziel */
char g_3a0b[7] = "FX.DAT";   /* ds:3a0b, Zeigerziel */
char g_3a12[6] = "FX.FN";   /* ds:3a12, Zeigerziel */
char g_3a18[11] = "SPEECH.DAT";   /* ds:3a18, Zeigerziel */
char g_3a23[10] = "SPEECH.FN";   /* ds:3a23, Zeigerziel */
char g_3a2d[8] = "RAW.DAT";   /* ds:3a2d, Zeigerziel */
char g_3a35[7] = "RAW.FN";   /* ds:3a35, Zeigerziel */
char g_3a3c[11] = "..\\TEMP\\%s";   /* ds:3a3c, Zeigerziel */
char g_3a47[12] = "..\\GAMES\\%s";   /* ds:3a47, Zeigerziel */
char g_3a53[6] = "\\DATA";   /* ds:3a53, Zeigerziel */
char g_str_files_low[76] = "\nBitte erh\x94hen Sie Ihren Files-Eintrag in der CONFIG.SYS auf mindestens 20!";
char g_str_version[31] = "\nSternenschweif Version C1.02\n";
char g_str_lastgame_3ac4[9] = "LASTGAME";
char g_str_lastgame_3acd[9] = "LASTGAME";
char g_str_gam_3ad6[5] = ".GAM";
char g_str_gam_3adb[5] = ".GAM";
char g_str_lastgame_3ae0[9] = "LASTGAME";
char g_str_mem_low1[19] = "Zu wenig Speicher!";
char g_str_mem_low2[37] = "\"Das Schwarze Auge - Sternenschweif\"";
char g_str_mem_low3[55] = "ben\x94tigt mindestens 568320 Bytes freien Hauptspeicher!";
char g_str_mem_low4[67] = "\nDa Ihre momentane Systemkonfiguration einen stabilen Spielverlauf";
char g_str_mem_low5[71] = "von Sternenschweif verhindert, bitten wir Sie vor einem erneuten Start";
char g_str_mem_low6[71] = "die Hinweise im beiliegenden Faltblatt und die Beispielkonfigurationen";
char g_str_mem_low7[44] = "in der Datei CONFIG.BSP zu ber\x81""cksichtigen.";
char g_str_drive_root[4] = "X:\\";
char g_str_all_3c59[5] = "*.*";
/* Phase B: eigenes BSS (ds:8b96..c776) in Adressreihenfolge */
char g_init[1];
struct mouse_bg g_mouse_bg;
signed short g_cursor_nr;
signed short g_cursor_nr_prev;
signed short g_mouse_lclick_time;	/* ds:8ba7 Zeitpunkt des ersten Linksklicks (SEG023 mouse_isr/timer_isr) */
char g_unused_8ba9[2];   /* ds:8ba9, unbenutzt */
signed short g_mouse_lpending;
signed short g_mouse_lclick;
signed short g_mouse_ldclick;
signed short g_mouse_ldown;
signed short g_mouse_rpending;
signed short g_mouse_rclick;
signed short g_mouse_rdclick;
signed short g_mouse_rdown;
signed short g_mouse_event;
unsigned char g_mouse_moved;
signed char g_mouse_busy;
unsigned char g_mouse_irq_init;
unsigned char g_mouse_locked;
signed short g_mouse_posx;
signed short g_mouse_posy;
signed short g_cursor_hot_x;
signed short g_cursor_hot_y;
signed short g_mouse_clickx;
signed short g_mouse_clicky;
signed short g_mouse_posx_bak;
signed short g_mouse_posy_bak;
signed short g_mouse_show_cnt;
signed short g_mouse_flag;
signed short g_key_flag;
unsigned char far *g_cursor_nvf;
void far *g_mouse_areas;
void far *g_mouse_areas2;
signed short g_key_ascii;
signed short g_key_scan;
signed short g_key_shift;
unsigned short g_key;
signed short g_key_buf;
signed short g_action;
signed short g_mouse_handler_on;
void far *g_mouse_handler_areas;
void (far *g_mouse_handler)(short);
void interrupt (*g_mouse_handler_bak)(void);
void interrupt (*g_old_int9)(void);
signed short g_cast_spell_id[1];
signed short g_spell_cancelled;
signed short g_spell_success;
signed char g_spell_level;
char g_unused_8c08[4];   /* ds:8c08, unbenutzt */
signed short g_spell_cost_paid;
signed short g_spell_summon_mon;
signed char g_spell_summon_renegade;
signed short g_spell_by_item;
signed short g_spell_no_sound;
signed short g_spell_event;
signed char g_spell_cost_mod;
signed char far *g_item_demand;
signed short g_cam_x;
signed short g_cam_y;
signed short g_cam_z;
char g_unused_8c22[2];   /* ds:8c22, unbenutzt */
signed short g_cam_rx;
signed short g_cam_ry;
signed short g_cam_rz;
char g_unused_8c2a[2];   /* ds:8c2a, unbenutzt */
signed short g_cam_rot_x;
signed short g_cam_rot_y;
signed short g_cam_angle;
char g_unused_8c32[2];   /* ds:8c32, unbenutzt */
signed short g_cam_eye_x;
signed short g_cam_eye_y;
signed short g_cam_eye_z;
char g_unused_8c3a[2];   /* ds:8c3a, unbenutzt */
signed short g_cam_ang[3];
char g_unused_8c42[2];   /* ds:8c42, unbenutzt */
struct vtx g_cam_vec;
struct vtx g_cam_dir;
signed short g_cam_neg[3];
char g_unused_8c5a[2];   /* ds:8c5a, unbenutzt */
signed long g_cam_fx;
signed long g_cam_fy;
signed long g_cam_fz;
char g_unused_8c68[8];   /* ds:8c68, unbenutzt */
void far *g_shade_buf;
char g_unused_8c74[8];   /* ds:8c74, unbenutzt */
signed short g_nr_vis_faces;
signed short g_nr_clip_faces;
char g_unused_8c80[2];   /* ds:8c80, unbenutzt */
signed short g_nr_vtx;
signed short g_sin_table[1024];
signed short g_view_init_flag;
signed short g_nr_textures;
char (far *g_tex_names)[12];
char g_unused_948c[2];   /* ds:948c, unbenutzt */
signed short g_nr_vtx_paths;
unsigned short g_obj_key;
signed short g_view_cell_y;
signed short g_view_cell_x;
signed short g_cam_cell_y;
signed short g_cam_cell_x;
signed short g_map_x0;
signed short g_map_y0;
signed short g_view_input_on;
signed short g_cam_pitch;
signed short g_cam_speed;
signed short g_cam_roll;
unsigned char far *g_face_depth;
signed short far *g_face_next;
unsigned short far *g_shade_table;
struct obj3d far *g_obj_cur;
struct obj3d far * far *g_cell_objs;
struct vtx far *g_vtx_buf;
struct texture far *g_textures;
struct face_ent far *g_face_list;
struct vtx_path far *g_vtx_paths[32];
signed short g_view_pal_n;
struct rgb g_view_pal[256];
struct vis_order far *g_vis_order;
struct map_cell (far *g_cells)[MAP_SIZE];
unsigned short g_face_seg;
void far *g_face_block;
char g_unused_9856[6];   /* ds:9856, unbenutzt */
BMP_PTR g_hud_top_left;
BMP_PTR g_hud_top_right;
BMP_PTR g_hud_bottom_left;
BMP_PTR g_hud_bottom_right;
BMP_PTR g_render_bmp;
signed long g_pick_event;
signed short g_pick_depth;
signed short g_pick_angle;
struct map_event g_cell_event;
signed short g_nr_click;
signed long g_click_event[8];
signed short g_click_depth[8];
signed short g_click_angle[8];
struct rect g_click_rect[8];
struct cell_xy g_prof_cells[4];
signed short g_prof_zmin[4][6][32];
signed short g_prof_zmax[4][6][32];
signed short g_prof_ymin[4][6][32];
signed short g_prof_ymax[4][6][32];
signed short g_prof_count[4];
char g_unused_b116[4];   /* ds:b116, unbenutzt */
signed short g_prof_next;
signed short g_view_indoor;
signed char g_step_active;
signed char g_step_pending;
signed short g_step_target;
signed short g_step_turn;
signed short g_move_flags;
signed short g_step_flags;
void far *g_renderbuf_ptr;
signed short g_skip_cell_event;
signed short g_view_active;
char g_unused_b130[2];   /* ds:b130, unbenutzt */
signed short g_tex_anim_frame;
signed short g_tex_anim_frames;
signed short g_tex_anim_due;
signed short g_tex_anim_rate;
signed short g_tex_anim_tick;
struct texture far *g_tex_anim_a[4];
struct texture far *g_tex_anim_b[4];
struct texture far *g_tex_anim_c[4];
unsigned short g_ems_frame;
signed short g_swap_kb;
unsigned char g_star_colors[8];
signed short g_stars_reset;
struct vtx g_stars[100];
signed short g_icon_group;
signed short g_icon_groups;
signed short g_icon_camp;
signed short g_view_input_blocked;
void far *g_cell_occludes;
unsigned char g_near_objs[81];
unsigned char g_near_occl[81];
char g_unused_b548[8];   /* ds:b548, unbenutzt */
signed short g_cursor_no;
void far *g_names_star;
void far *g_names_star_idx;
void far *g_names_fx;
void far *g_names_fx_idx;
void far *g_names_speech;
void far *g_names_speech_idx;
void far *g_names_raw;
void far *g_names_raw_idx;
void far *g_names_starcd;
void far *g_names_starcd_idx;
void far *g_names_speechcd;
void far *g_names_speechcd_idx;
void far *g_patch_starcd;
void far *g_patch_speechcd;
void far *g_patch_star;
void far *g_patch_fx;
void far *g_patch_speech;
void far *g_patch_raw;
char g_dialog[16];
signed short g_dialog_sel;
char g_unused_b5ac[14];   /* ds:b5ac, unbenutzt */
signed short g_silent_mode;
signed short g_blink_off;
char g_unused_b5be[30];   /* ds:b5be, unbenutzt */
signed short g_fight_running;
char g_unused_b5de[2];   /* ds:b5de, unbenutzt */
signed short g_silent_mode2;
char g_unused_b5e2[60];   /* ds:b5e2, unbenutzt */
signed char g_dlg_box_flag;
char g_unused_b61f[8];   /* ds:b61f, unbenutzt */
char g_bmp_spare[14];
char g_ofile[4];
void far *g_cd;
char g_nvf_heads[25];
signed short g_map_w;
signed short g_map_h;
char g_unused_b65a[12];   /* ds:b65a, unbenutzt */
signed short g_pp20_index;
void far *g_vga_buf;
unsigned char far *g_loc_icon_buf;
char far *g_text_output_buf;
char far *g_text_buf2;
void far *g_ttx_buf;
char far * far *g_ttx;
unsigned char far *g_pal_buf;
unsigned char far *g_pal_dark;
struct video_state g_video_state;
char g_input_buf[41];
unsigned char g_icon_pic_group;
unsigned char g_icon_pic_groups;
char g_unused_b6bb[2];   /* ds:b6bb, unbenutzt */
unsigned char g_icon_pic_camp;
char g_unused_b6be[3];   /* ds:b6be, unbenutzt */
char far *g_menu_texts[15];
char g_b6fd[2];   /* ds:b6fd, unbenutzt */
signed char g_menu_tail[85];
unsigned char huge *g_snd_buf0;
unsigned char huge *g_snd_buf1;
signed short g_snd_vol_req;
signed short g_snd_vol_req2;
struct hero far *g_heroes;
char g_unused_b764[5];   /* ds:b764, unbenutzt */
char g_bars[4];
signed short g_status_on;
char g_unused_b76f[15];   /* ds:b76f, unbenutzt */
char g_screen[1];
char g_screen_pic[26];
unsigned char far *g_screen_pic_pal;
char g_unused_b79d[23];   /* ds:b79d, unbenutzt */
signed short g_screen_pic_state;
char g_screen_pic_rest[146];   /* ds:b7b6 Rest des Bildobjekts g_screen_pic, darin g_screen_pic_run ds:b7bc (SEG001) */
char g_snd_speech[2];
struct item_desc far *g_items;
char far * far *g_item_names;
char g_unused_b852[4];   /* ds:b852, unbenutzt */
char g_date[1];
signed long g_daytime;
signed char g_date_weekday;
char g_unused_b85c[1];   /* ds:b85c, unbenutzt */
unsigned char g_date_day;
unsigned char g_date_month;
unsigned char g_date_year;
char g_unused_b860[14];   /* ds:b860, unbenutzt */
char g_dialog2[1338];
char g_decomp_work[138];
char g_weather[4];
char g_map[653];
char g_diary[43];
signed short g_market_buy;
signed short g_market_sell;
signed char g_market_type;
signed char g_market_lvl;
char g_unused_c0f4[6];   /* ds:c0f4, unbenutzt */
signed char g_market_female;
char g_travel[78];
signed short g_travel_rose_x;
signed short g_travel_rose_y;
char g_unused_c14d[120];   /* ds:c14d, unbenutzt */
unsigned char g_route_km;
char g_unused_c1c6[4];   /* ds:c1c6, unbenutzt */
signed char g_camp_weather;
char g_tmap_rest[76];   /* ds:c1cb Reisekarten-Daten, darin g_tmap_mark1_x/_y ds:c20b/c20d (SEG001) */
char g_tmap[240];
void far *g_tmap_buf;
void far *g_tmap_ptr;
char g_c30f[438];   /* ds:c30f, Speicher von g_groups (struct group[NR_GROUPS], Konstruktor in globals_ctor) */
struct group far *g_group;
struct hero far *g_sheet_hero;
signed short g_in_fight;
signed char g_fig_enemies_escaped;
signed short g_refresh;
signed short g_refresh_portraits;
signed short g_hero_died;
signed short g_bg_restored;
signed short g_in_wait;
signed char g_on_boat;
struct mem_move g_xms_move;
char g_unused_c4eb[4];   /* ds:c4eb, unbenutzt */
char far * far *g_mon_names;
void far *g_mon_names_buf;
struct enemy far *g_enemies;
void far *g_portrait_buf;
void far *g_indos_flag;
signed char g_hero_slot_aux[4][7];
signed char g_speech_on;
char g_c520[1];   /* ds:c520, nur fremd */
signed char g_ail_timer_users;
signed short g_mem_check_countdown;	/* ds:c522 Zeitgeber-ISR: alle 20 Takte mem_check (SEG023) */
signed long g_mem_min_free;
signed long g_mem_free_start;
void far *g_anim_cur;
signed short g_anim_running;
void far *g_fight_obj;
signed char g_timer_isr_busy;	/* ds:c536 Zeitgeber-ISR laeuft (Wiedereintrittssperre, SEG023/R26) */
signed char g_snd_service_due;
signed short g_anim_timer;
char g_fight_list[218];
signed short g_fig_anim_removed;
char g_unused_c616[114];   /* ds:c616, unbenutzt */
signed char g_fig_target_killed;
signed char g_fig_action_flag;
signed char g_enemy_unmushroomed;
signed short g_fight_max_enemies;
signed short g_save_fd;
signed short g_date_flag;
signed short g_clock_update;
signed short g_is_day;
char g_unused_c695[1];   /* ds:c695, unbenutzt */
signed short g_menu_line;
signed short g_timers_lock;
signed short g_ems_avail;
signed long g_swap_handle;
signed short g_swap_used_kb;
signed long g_tex_base;
signed long g_swap_pos;
void far *g_view_swap_pos;
signed long g_view_swap_end;
signed short g_swap_nr_objs;
char far *g_grammar_bufs;
char far *g_grammar_name_buf;
char g_c6bc[4];   /* ds:c6bc, nur fremd */
signed short g_automap_redraw;
signed short g_c6c2;
signed short g_dialog_xms;
unsigned long g_tamper_count;
char g_copyprot[75];
signed char g_mem_saving;
signed short g_swap_handle2;
signed short g_anim_no_skip;
char far *g_version_str;
unsigned char far *g_palette;
struct mem_block far *g_mem_blocks;
char g_start_dir[3];
char g_start_dir_path[77];
