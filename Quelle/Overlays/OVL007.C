/* OVL007 - Overlay O007 (Segment 0x1f1f in SCHWEIF.EXE): Zwischensequenzen und Entwickler-Testmenue.
 * debug_test_menu: Menueschleife (DEBUG.LTX 0x1f0 nach g_debug_txt_buf/g_debug_txt, dlg_choose mit 10 Punkten):
 * 1 Spielende (g_game_state = 0x63), 2 phex_finale, 3..7 lokale Objekte Anim3 (O040), Outro1/Outro2 (O041),
 * Intro (O039, mit g_trial_lost = 0/1) (je Ktor, run, Dtor(&o, 2)), 8 blackmage_scene, 9/10 ohne Wirkung.
 * blackmage_scene (Schwarzmagier, auch aus dem Dungeon O017): Endbild (Bmp 320x200 auf g_vga_buf), Palette, drei
 * Texte txt[30..32] (SEQUENCE.LTX 0xa9) mit Sprachausgabe (scene_line_speech, wenn g_speech_on == 1 und
 * speech_active) bzw. Textfeld (scene_line_anim), Auswahl scene_choice (2 Punkte), Abschluss txt[36]/txt[37];
 * Rueckgabe 1/-1/0, der Salamanderstein (ITEM_SALAMANDER_STONE) bei der Gruppe entscheidet den Zweig.
 * phex_finale (Phex-Tempel, auch aus dem Dungeon O204): 19 Textseiten (PHEXTEMP.LTX 0x34d) scene_line(txt, Sprache,
 * Dauer), zwei Auswahlen scene_choice (3 bzw. 2 Punkte) -> ggf. Kampf phex_finale_fight und g_port_deal = 1; bei
 * Wahl 2 Ansehen g_god_favor[1..14] -= 20 und [9] -= 15. scene_line_speech/scene_line_anim/scene_line: Textzeile im
 * Bild unten (Bmp 320x28 auf g_vga_buf + 0xd700 retten/zurueck), Sprache snd_speech bzw. ani_play_until bzw.
 * Pause wait_ticks_key. scene_choice(a, b, c, n): Auswahl mit Maus (g_mouse_posy) und Pfeiltasten, Enter; Markierung per
 * Formatstring g_scene_fmt mit Markern g_scene_mark_on/_off. phex_finale_fight: start_fight(0xa5, 1).
 * unused_scene_stub leer.
 *
 * BCC -mlarge -3 -Ot -c -Y OVL007.C   -> 5211/5211 Bytes GLEICH (18.10.2026; -Og ebenso), cobjfix laeuft
 *   (Namensfassung mit schweif.h, Welle N20: 19.10.2026 GLEICH, LINKGLEICH)
 * - Reines C: Bmp-Objekte als gb_*-Aufrufe (new = gb_new(0, ..), delete = gb_delete(p, 3), lokal (&b, 2)),
 *   Anim3/Outro/Intro-Objekte in debug_test_menu als Funktions-Locals (char[0x24]) wie in OVL222.C.
 * - blackmage_scene: lokales Bmp [bp-36h] UEBERLAPPT das 3-Byte-Feld rgb [bp-30h] -> beide in eigenen Geschwister-
 *   Bloecken (sonst Bmp bei [bp-3ah]). save = g_mouse_range (8-Byte-struct) wortweise, Rueckschreiben ebenso.
 * - scene_choice: initialisierte lokale Felder ohne eigene Daten: "struct str2 on = g_scene_mark_on; struct str2 off =
 *   g_scene_mark_off; struct str7 fmt = g_scene_fmt;" (7 Bytes -> drei Wort- und ein Byte-mov), liegen oben ([bp-2],
 *   [bp-4], [bp-0ch]).
 * - memcpy ohne -Oi als echter Aufruf, bioskey aus der Bibliothek; "anim_load_variant(.., g_pp20_index = 0x192, ..)".
 * Originalfehler/Auffaelligkeiten: blackmage_scene, Zweig ohne Salamanderstein, Wahl 0: "if (g_speech_on == 1 &&
 *   speech_active()) ; else ;" - beide Zweige leer (Abschlusstext/Sprache fehlt, Aufruf bleibt). Im selben Zweig
 *   fehlt vor scene_choice das ani_halt(g_screen_pic) des anderen Zweigs. phex_finale gibt Texte per mem_free_v frei,
 *   blackmage_scene dieselben Puffer per mem_free.
 */
#include <stdio.h>
#include <mem.h>
#include <bios.h>
#include "schweif.h"

/* Phase B: eigene Daten des Moduls (ds:011a..0172, dsana) - initialisierte Daten in Adressreihenfolge, danach der Literalpool */
signed short g_cl_blackmage[3] = { 8361, 8594, -1 };
signed short g_cl_blackmage_speech[10] = { 24794, 24795, 24796, 24797, 24798, 24800, 24801, 24802, 24803, -1 };
unsigned char g_cl_blackmage_st[4] = "";
signed short g_cl_phex_finale[3] = { 9037, 9061, -1 };
signed short g_cl_phex_finale_speech[16] = {
	-15356, -15355, -15354, -15353, -15352, -15351, -15350, -15349, -15348, -15347, -15346, -15345, -15344, -15343, -15342, -1
};
unsigned char g_cl_phex_finale_st[4] = "";
struct str2 g_scene_mark_on = { "\xf1" };
struct str2 g_scene_mark_off = { "\xf0" };
struct str7 g_scene_fmt = { "%s%s%s" };

#define SEQUENCE_TXT	0xa9		/* SEQUENCE.LTX (0x32 Texte) */
#define PHEXTEMP_TXT	0x34d		/* PHEXTEMP.LTX (0x50 Texte) */
#define DEBUG_TXT	0x1f0		/* DEBUG.LTX (0x1cc Texte) */
#define FIGHT_PHEX_FINALE 0xa5

void scene_line_speech(char far *s, signed short snd)
{
	struct Bmp far *bm;

	bm = gb_new(0, 320, 28, (char far *)g_vga_buf + 0xd700);
	if (g_subtitles != 0) {
		font_set_box(g_font, 10, 300, 0xaf, 1);
		font_print_wrapped(g_font, s);
	}
	snd_speech(g_snd_speech, snd);
	while (snd_speech_busy(g_snd_speech)) {
		game_idle();
		wait_for_vsync();
	}
	mouse_hide(0);
	gb_clear(bm);
	mouse_show(0);
	gb_delete(bm, 3);
}

void scene_line_anim(char far *s, signed char n)
{
	struct Bmp far *bm;

	bm = gb_new(0, 320, 28, (char far *)g_vga_buf + 0xd700);
	font_set_box(g_font, 10, 300, 0xaf, 1);
	font_print_wrapped(g_font, s);
	ani_play_until(g_screen_pic, 5, n);
	mouse_hide(0);
	gb_clear(bm);
	mouse_show(0);
	gb_delete(bm, 3);
}

signed short blackmage_scene(void)
{
	signed short x0;
	signed short y0;
	signed short x1;
	signed short y1;
	unsigned char far *pal;
	struct mouse_range save;
	void far *raw;
	char far * far *txt;
	struct hero far *h1;
	struct hero far *h2;
	struct hero far *h3;
	struct hero far *h4;
	register signed short ret;
	register signed short r;

	g_dialog_depth++;
	ret = 0;
	load_pic_list(g_cd_cache, g_cl_blackmage, g_cl_blackmage_st, 1, 0);
	if (g_cd_speech != 0)
		load_pic_list(g_cd_cache, g_cl_blackmage_speech, g_cl_blackmage_st, 1, 0);
	txt_load_list(g_font, SEQUENCE_TXT, 0x32, &raw, &txt);
	snd_play(g_snd_music, 0xe);
	pal = mem_alloc_w(0x300);
	memcpy(pal, g_palette, 0x300);
	font_get_box(g_font, &x0, &y0, &x1, &y1);
	pal_fade_out();
	mouse_hide(0);
	save = g_mouse_range;
	g_mouse_range.min_y = 0xaa;
	mouse_move(g_mouse_posx, 0xaa);
	save_screen();
	{
		struct Bmp b;

		gb_new(&b, 320, 200, g_vga_buf);
		gb_clear(&b);
		gb_delete(&b, 2);
	}
	ani_set_pos(g_screen_pic, 0, 0xc);
	anim_load_variant(g_screen_pic, g_pp20_index = 0x192, 3, 0);
	g_screen_pic_state = 0;
	{
		unsigned char rgb[3];

		rgb[0] = 0x3f;
		rgb[1] = 0x3f;
		rgb[2] = 0x3f;
		set_color(rgb, 0xff);
	}
	set_palette(g_screen_pic_pal, 0, 0xff);
	mouse_set_cursor(0, 1);
	mouse_show(0);
	font_set_colors(g_font, 0xff, 0);
	font_push_colset(g_font, 0x88, 0x81, 0xff, 0xff, 0xff);
	if (group_find_item(g_group, ITEM_SALAMANDER_STONE) >= 0) {
		if (g_speech_on == 1 && speech_active(g_snd_speech)) {
			scene_line_speech(txt[30], 0xda);
			scene_line_speech(txt[31], 0xdb);
			scene_line_speech(txt[32], 0xdc);
		} else {
			scene_line_anim(txt[30], 1);
			scene_line_anim(txt[31], 2);
			scene_line_anim(txt[32], 3);
		}
		ani_halt(g_screen_pic);
		r = scene_choice(txt[34], txt[35], " ", 2);
		if (r == 0) {
			if (g_speech_on == 1 && speech_active(g_snd_speech)) {
				h1 = get_first_active_hero(g_group);
				if (h1->sex == 0)
					snd_speech(g_snd_speech, 0xe0);
				else
					snd_speech(g_snd_speech, 0xe2);
				while (snd_speech_busy(g_snd_speech)) {
					game_idle();
					wait_for_vsync();
				}
			}
			ani_halt(g_screen_pic);
			g_screen_pic_state = 0;
			anim_select_variant(g_screen_pic, 3, 1);
			if (g_speech_on == 1 && speech_active(g_snd_speech))
				scene_line_speech(txt[36], 0xdd);
			else
				scene_line_anim(txt[36], 5);
		} else {
			if (g_speech_on == 1 && speech_active(g_snd_speech)) {
				h2 = get_first_active_hero(g_group);
				if (h2->sex == 0)
					snd_speech(g_snd_speech, 0xe1);
				else
					snd_speech(g_snd_speech, 0xe3);
				while (snd_speech_busy(g_snd_speech)) {
					game_idle();
					wait_for_vsync();
				}
			}
			ani_halt(g_screen_pic);
			g_screen_pic_state = 0;
			anim_select_variant(g_screen_pic, 3, 2);
			if (g_speech_on == 1 && speech_active(g_snd_speech))
				scene_line_speech(txt[37], 0xde);
			else
				scene_line_anim(txt[37], 6);
			ret = 1;
		}
	} else {
		if (g_speech_on == 1 && speech_active(g_snd_speech)) {
			scene_line_speech(txt[30], 0xda);
			scene_line_speech(txt[31], 0xdb);
			scene_line_speech(txt[32], 0xdc);
		} else {
			scene_line_anim(txt[30], 1);
			scene_line_anim(txt[31], 2);
#if FIX_0134
			/* Fix #134: Animation vor der Auswahl anhalten wie im Zweig mit Salamanderstein (O055_032F fehlte) */
			scene_line_anim(txt[32], 3);
			}
			ani_halt(g_screen_pic);
			r = scene_choice(txt[34], txt[35], " ", 2);
#else
			scene_line_anim(txt[32], 3);
		}
		r = scene_choice(txt[34], txt[35], " ", 2);
#endif
		if (r == 0) {
			if (g_speech_on == 1 && speech_active(g_snd_speech)) {
				h3 = get_first_active_hero(g_group);
				if (h3->sex == 0)
					snd_speech(g_snd_speech, 0xe0);
				else
					snd_speech(g_snd_speech, 0xe2);
				while (snd_speech_busy(g_snd_speech)) {
					game_idle();
					wait_for_vsync();
				}
			}
			ani_halt(g_screen_pic);
			g_screen_pic_state = 0;
			anim_select_variant(g_screen_pic, 3, 1);
			if (g_speech_on == 1 && speech_active(g_snd_speech))
				;
			else
				;
			ret = -1;
		} else {
			if (g_speech_on == 1 && speech_active(g_snd_speech)) {
				h4 = get_first_active_hero(g_group);
				if (h4->sex == 0)
					snd_speech(g_snd_speech, 0xe1);
				else
					snd_speech(g_snd_speech, 0xe3);
				while (snd_speech_busy(g_snd_speech)) {
					game_idle();
					wait_for_vsync();
				}
			}
			ani_halt(g_screen_pic);
			g_screen_pic_state = 0;
			anim_select_variant(g_screen_pic, 3, 2);
			if (g_speech_on == 1 && speech_active(g_snd_speech))
				scene_line_speech(txt[37], 0xde);
			else
				scene_line_anim(txt[37], 6);
			ret = 1;
		}
	}
	pal_fade_out();
	mouse_hide(0);
	ani_stop(g_screen_pic);
	ani_set_pos(g_screen_pic, 0x10, 2);
	restore_screen();
	mouse_reset();
	g_mouse_range = save;
	mouse_show(0);
	pal_fade_in(pal);
	font_pop_colset(g_font);
	font_set_box(g_font, x0, y0, x1, y1);
	mem_free_v(pal);
	snd_obj_stop(g_snd_music);
	mem_free(raw);
	mem_free(txt);
	cache_purge(g_cd_cache, 1);
	g_dialog_depth--;
	return ret;
}

void phex_finale(void)
{
	void far *raw;
	char far * far *txt;
	signed short x0;
	signed short y0;
	signed short x1;
	signed short y1;
	unsigned char far *pal;
	struct mouse_range save;
	register signed short i;
	register signed short r;

	load_pic_list(g_cd_cache, g_cl_phex_finale, g_cl_phex_finale_st, 1, 0);
	if (g_cd_speech != 0)
		load_pic_list(g_cd_cache, g_cl_phex_finale_speech, g_cl_phex_finale_st, 1, 0);
	snd_play(g_snd_music, 0xe);
	txt_load_list(g_font, PHEXTEMP_TXT, 0x50, &raw, &txt);
	g_dialog_depth++;
	pal = mem_alloc_w(0x300);
	memcpy(pal, g_palette, 0x300);
	font_get_box(g_font, &x0, &y0, &x1, &y1);
	pal_fade_out();
	mouse_hide(0);
	save = g_mouse_range;
	g_mouse_range.min_y = 0xaa;
	mouse_move(g_mouse_posx, 0xaa);
	clear_screen_buf(g_vga_buf);
	ani_set_pos(g_screen_pic, 0, 0);
	anim_load(g_screen_pic, g_pp20_index = 0x365, 0);
	g_screen_pic_state = 0;
	set_palette(g_screen_pic_pal, 0, 0xff);
	mouse_set_cursor(0, 1);
	mouse_show(0);
	font_set_colors(g_font, 0xff, 0);
	font_push_colset(g_font, 0x88, 0x81, 1, 0xff, 0xff);
	scene_line(txt[5], 0, 300);
	scene_line(txt[6], 0, 300);
	scene_line(txt[7], 0x404, 300);
	r = scene_choice(txt[8], txt[9], txt[10], 3);
	if (r == 0) {
		pal_fade_out();
		ani_halt(g_screen_pic);
		clear_screen_buf(g_vga_buf);
		set_palette(pal, 0, 0x100);
		g_fight_end = 4;
		phex_finale_fight();
		g_port_deal = 1;
	} else {
		if (r == 2) {
			scene_line(txt[11], 0, 200);
			for (i = 1; i <= 14; i++)
				g_god_favor[i] -= 20;
			g_god_favor[9] -= 15;
		}
		scene_line(txt[12], 0x405, 0x271);
		scene_line(txt[13], 0x406, 0x113);
		scene_line(txt[14], 0x407, 0x1b8);
		scene_line(txt[15], 0x408, 0x122);
		scene_line(txt[16], 0x409, 0x16d);
		scene_line(txt[17], 0x40a, 0xf0);
		scene_line(txt[18], 0x40b, 0x1c2);
		scene_line(txt[19], 0x40c, 0x249);
		scene_line(txt[20], 0x40d, 0x190);
		scene_line(txt[21], 0x40e, 0x172);
		scene_line(txt[22], 0x40f, 0x18b);
		scene_line(txt[23], 0x410, 0x12c);
		scene_line(txt[24], 0x411, 0x14a);
		scene_line(txt[25], 0x412, 0x226);
		r = scene_choice(txt[26], txt[27], 0, 2);
		if (r == 1) {
			pal_fade_out();
			ani_halt(g_screen_pic);
			clear_screen_buf(g_vga_buf);
			set_palette(pal, 0, 0x100);
			g_fight_end = 4;
			phex_finale_fight();
			g_port_deal = 1;
		} else
			pal_fade_out();
	}
	mouse_hide(0);
	ani_stop(g_screen_pic);
	ani_set_pos(g_screen_pic, 0x10, 2);
	clear_screen_buf(g_vga_buf);
	mouse_reset();
	g_mouse_range = save;
	mouse_show(0);
	pal_fade_in(pal);
	font_pop_colset(g_font);
	font_push_colset(g_font, 0xff, 0xfd, 0xf9, 0xfc, 0xe7);
	font_set_box(g_font, x0, y0, x1, y1);
	g_refresh = 1;
	mem_free_v(pal);
	g_phex_finale_done = 1;
	if (!g_mem_saving) {
		struct obj3d far *o;

		o = g_cell_objs[27];
		o->nfaces -= 2;
	}
	g_dialog_depth--;
	mem_free_v(raw);
	mem_free_v(txt);
	cache_purge(g_cd_cache, 1);
}

void scene_line(char far *s, register signed short snd, register signed short delay)
{
	struct Bmp far *bm;

	bm = gb_new(0, 320, 28, (char far *)g_vga_buf + 0xd700);
	if (g_subtitles != 0 && snd) {
		font_set_box(g_font, 10, 300, 0xaf, 1);
		font_print_wrapped(g_font, s);
		if (!g_cd_speech)
			wait_ticks_key(delay);
	}
	if (g_speech_ext_avail != 0) {
		if (g_cd_speech != 0 && snd) {
			snd_speech(g_snd_speech, snd);
			do
				poll_input();
			while (g_key != 0);
			while (snd_speech_busy(g_snd_speech)) {
				game_idle();
				wait_for_vsync();
			}
		} else if (!snd && s != 0) {
			font_set_box(g_font, 10, 300, 0xaf, 1);
			font_print_wrapped(g_font, s);
			wait_ticks_key(delay);
		}
	}
	mouse_hide(0);
	gb_clear(bm);
	mouse_show(0);
	gb_delete(bm, 3);
}

signed short scene_choice(char far *a, char far *b, char far *c, signed short n)
{
	struct str2 on = g_scene_mark_on;
	struct str2 off = g_scene_mark_off;
	struct str7 fmt = g_scene_fmt;
	signed short last = -1;
	register signed short sel = 0;
	struct Bmp far *bm;
	register signed short redraw;

	bm = gb_new(0, 320, 28, (char far *)g_vga_buf + 0xd700);
	redraw = 1;
	while (bioskey(1))
		bioskey(0);
	do {
		poll_input();
		if (last != g_mouse_posy) {
			if (g_mouse_posy < 0xb7)
				sel = 0;
			else if (g_mouse_posy < 0xbf)
				sel = 1;
			else
				sel = n == 3 ? 2 : 1;
			last = g_mouse_posy;
			redraw = 1;
		}
		if (g_mouse_lclick != 0)
			g_action = 0x1c0d;
		if (g_action == 0x4800) {
			if (--sel < 0)
				sel = n - 1;
			redraw = 1;
		}
		if (g_action == 0x5000) {
			if (++sel > n - 1)
				sel = 0;
			redraw = 1;
		}
		if (redraw == 1) {
			redraw = 0;
			font_set_box(g_font, 10, 300, 0xaf, 1);
			sprintf(g_text_output_buf, fmt.s, !sel ? on.s : off.s, a, off.s);
			font_print_wrapped(g_font, g_text_output_buf);
			font_set_box(g_font, 10, 300, 0xb7, 1);
			sprintf(g_text_output_buf, fmt.s, sel == 1 ? on.s : off.s, b, off.s);
			font_print_wrapped(g_font, g_text_output_buf);
			if (n == 3) {
				font_set_box(g_font, 10, 300, 0xbf, 1);
				sprintf(g_text_output_buf, fmt.s, sel == 2 ? on.s : off.s, c, off.s);
				font_print_wrapped(g_font, g_text_output_buf);
			}
		}
	} while (g_action != 0x1c0d);
	mouse_clear_left();
	mouse_hide(0);
	gb_clear(bm);
	mouse_show(0);
	gb_delete(bm, 3);
	return sel;
}

void debug_test_menu(void)
{
	char anim3[0x24];			/* Objekt Anim3 (O040) */
	char outro1[0x24];			/* Objekt Outro1 (O041) */
	char outro2[0x24];			/* Objekt Outro2 (O041) */
	char intro1[0x24];			/* Objekt Intro (O039) */
	char intro2[0x24];
	register signed short c;

	anim_free_all(g_screen_pic);
	Sound_stop_music(g_snd_music);
	do {
		txt_load_list(g_font, DEBUG_TXT, 0x1cc, &g_debug_txt_buf, &g_debug_txt);
		c = dlg_choose(g_dialog, g_debug_txt[53], 10, g_debug_txt[430], g_debug_txt[354], g_debug_txt[352], g_debug_txt[447],
			g_debug_txt[448], g_debug_txt[451], g_debug_txt[452], g_debug_txt[453], g_debug_txt[454], g_debug_txt[455]);
		g_debug_txt_buf = mem_free(g_debug_txt_buf);
		g_debug_txt = mem_free(g_debug_txt);
		switch (c) {
		case 1:
			g_game_state = 0x63;
			c = -1;
			break;
		case 2:
			phex_finale();
			c = -1;
			break;
		case 3:
			Anim3_ctor(anim3);
			Anim3_run(anim3);
			c = -1;
			Anim3_dtor(anim3, 2);
			break;
		case 4:
			Outro1_ctor(outro1);
			Outro1_run(outro1);
			Outro1_dtor(outro1, 2);
			break;
		case 5:
			Outro2_ctor(outro2);
			Outro2_run(outro2);
			Outro2_dtor(outro2, 2);
			break;
		case 6:
			Intro_ctor_o039(intro1);
			g_trial_lost = 0;
			Intro_run_o039(intro1);
			Intro_dtor_o039(intro1, 2);
			break;
		case 7:
			Intro_ctor_o039(intro2);
			g_trial_lost = 1;
			Intro_run_o039(intro2);
			Intro_dtor_o039(intro2, 2);
			break;
		case 8:
			blackmage_scene();
			break;
		case 9:
			break;
		case 10:
			break;
		}
	} while (c != -1);
	g_refresh = 1;
}

void phex_finale_fight(void)
{
	start_fight(FIGHT_PHEX_FINALE, 1);
}

void unused_scene_stub(void)
{
}
