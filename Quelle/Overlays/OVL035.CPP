/* OVL035 - Overlay O035 (Segment 0x1fc6 in SCHWEIF.EXE): Spielende-Sequenz, Klasse Finale (s_class.h, 0x22 Bytes),
 * von finale_play (O045) lokal angelegt. Konstruktor: drei Paletten zu 0x300 Bytes (pal_saved/pal_from/pal_pic),
 * new Pic, Bild-Member Bmp, gesicherte Palette (get_palette). Destruktor: Palette zurueck, delete Pic, Paletten,
 * Texte frei. run(): bei mehreren Gruppen Meldung txt[9], title() (Titelbild 0x436, Dateilisten g_cl_finale bzw.
 * g_cl_finale_speech in den CD-Cache), Texte 0x3b1 (15 Stueck), Musik 0x28, bei g_speech_ext_avail das Skript
 * finale_play_script (O036) bzw. scenes(), Schrift zuruecksetzen, Meldung txt[10] (Fenster mittig), Spielstand-
 * menue (savegame_menu ueber ein lokales SaveMenu), Spielstand g_str_lastgame_3dbc loeschen.
 * scene1..scene9: Bild 0x3a7..0x3af, Text txt[0..8] an fester Position; fade_wait(n): einblenden, warten (bei
 * laufender Musik ohne CD bis zur Marke g_snd_vol_req == n, sonst 1000 Takte), ausblenden.
 *
 * BCC -mlarge -3 -Ot -c -Y -E..\bc\BIN\TASM.EXE OVL035.CPP   -> 3349/3349 Bytes GLEICH (18.10.2026),
 *   cobjfix laeuft (mit -Og ebenso GLEICH). (Namensfassung mit schweif.h, Welle N20: 19.10.2026 GLEICH, LINKGLEICH)
 * - "new Pic" -> Pic_ctor(0) (Konstruktor mit this = 0), "delete pic" -> Pic_dtor(pic, 3) ohne Nulltest.
 * - SaveMenu (0x6f7 Bytes): Klasse mit Pic als erstem MEMBER (nicht Basisklasse!) und automatischem
 *   Konstruktor/Destruktor: new -> mem_alloc_w(0x6f7) + "if (p) Pic_ctor(p)" mit mov dx/ax in beiden Zweigen;
 *   delete -> "if (p) { Pic_dtor(p, 2); mem_free_v(p); }" mit totem jmp short.
 * - fade_wait: Bedingung ueber zwei verschachtelte inline-Funktionen: cd_off() { return !g_music_off; } -> neg/sbb/inc,
 *   no_cd_music() { return cd_off() && g_music_ready == 1; } -> mov ax,1 / xor ax,ax / or ax,ax. Direkt geschrieben
 *   macht BCC daraus nur cmp/jnz.
 * - sprintf(s, g_path_fmts[1], ..): das Format ist der far-Zeiger (cobjfix: _g_3587+4).
 * - Originalfehler/Auffaelliges: Konstruktor setzt txt_buf/txt nicht, der Destruktor gibt sie aber frei (ohne
 *   run() undefinierte Zeiger); new-Ergebnisse werden nicht geprueft; pal_from bleibt uninitialisiert.
 */
#include <stdio.h>
#include <mem.h>
#include <io.h>
#include "schweif.h"

/* Phase B: eigene Daten des Moduls (ds:3d8e..3dc6, dsana) - initialisierte Daten in Adressreihenfolge, danach der Literalpool */
signed short g_options_shown = 0;
signed short g_cl_finale[11] = { -24557, -24556, -24555, -24554, -24553, -24552, -24551, -24550, -24549, 9137, -1 };
signed short g_cl_finale_speech[9] = { -15339, -15338, -15337, -15336, -15335, -15334, -15333, -15332, -1 };
unsigned char g_cl_finale_st[4] = "";

#define PAL_SIZE	0x300
#define SCREEN_SIZE	0xfa00
#define FINALE_TXT	0x3b1		/* Texte der Spielende-Sequenz (15 Stueck) */
#define MUSIC_FINALE	0x28
#define PIC_TITLE	0x436

#define SRGB(p)		((struct srgb far *)(p))

Finale::Finale()
{
	pal_saved = new unsigned char[PAL_SIZE];
	pal_from = new unsigned char[PAL_SIZE];
	pal_pic = new unsigned char[PAL_SIZE];
	pic = new Pic;
	get_palette(pal_saved, 0, 0x100);
}

Finale::~Finale()
{
	set_palette(pal_saved, 0, 0x100);
	delete pic;
	delete pal_pic;
	delete pal_from;
	delete pal_saved;
	mem_free(txt_buf);
	mem_free(txt);
}

void Finale::run()
{
	SaveMenu far *sm;

	if (g_nr_groups > 1)
		dlg_message(g_dialog, txt[9]);
	ani_halt(g_screen_pic);
	g_dialog_depth++;
	g_pp20_index = -1;
	pal_fade_out();
	mouse_hide(0);
	title();
	((Font far *)g_font)->load_list(FINALE_TXT, 0xf, &txt_buf, &txt);
	snd_play(g_snd_music, MUSIC_FINALE);
	font_set_line_h(g_font, 10);
	if (g_speech_ext_avail)
		finale_play_script(this);
	else
		scenes();
	font_set_colors(g_font, 0xf, 0);
	font_set_box(g_font, 10, 100, 10, 0);
	font_set_line_h(g_font, 8);
	font_set_screen(g_font, g_vga_buf);
	font_push_colset(g_font, 0xff, 0xfd, 0xf9, 0xfc, 0xe7);
	swap_out_screen();
	g_finale_done = 1;
	clear_screen_buf(g_vga_buf);
	load_palette_arc3();
	mouse_show(0);
	{
		signed char old = g_win_centered;
		g_win_centered = 1;
		dlg_message(g_dialog, txt[10]);
		g_win_centered = old;
	}
	sm = new SaveMenu;
	savegame_menu(sm, 0);
	delete sm;
	{
		char far *s = new char[0x28];
		sprintf(s, g_path_fmts[1], "LASTGAME");
		unlink(s);
		delete s;
	}
	mouse_hide(0);
	memset(g_vga_buf, 0, SCREEN_SIZE);
	mouse_show(0);
}

void Finale::scenes()
{
	scene1();
	scene2();
	scene3();
	scene4();
	scene5();
	scene6();
	scene7();
	scene8();
	scene9();
}

/* Szene: Bild ID ganz zeigen, Text T im Rahmen x X, Breite W, y Y, dann fade_wait(N) */
#define SCENE(ID, X, W, Y, T, N) \
	memset(g_vga_buf, 0, SCREEN_SIZE); \
	pic_to_bmp(pic, ID, &bmp, SRGB(pal_pic), 0x100); \
	bmp.put(g_vga_buf, 0, 0); \
	bmp_free_data(&bmp); \
	font_set_box(g_font, X, W, Y, 1); \
	font_set_colors(g_font, 0xff, -1); \
	font_print_wrapped(g_font, txt[T]); \
	fade_wait(N);

void Finale::scene1() { SCENE(0x3a7, 0, 0xa0, 5, 0, 1) }
void Finale::scene2() { SCENE(0x3a8, 5, 0x136, 0xf, 1, 2) }
void Finale::scene3() { SCENE(0x3a9, 5, 0x136, 5, 2, 3) }
void Finale::scene4() { SCENE(0x3aa, 0, 0x140, 0xa0, 3, 4) }
void Finale::scene5() { SCENE(0x3ab, 0xa5, 0x9b, 0, 4, 5) }
void Finale::scene6() { SCENE(0x3ac, 5, 0x78, 0xc, 5, 6) }
void Finale::scene7() { SCENE(0x3ad, 5, 0x136, 5, 6, 7) }
void Finale::scene8() { SCENE(0x3ae, 0xc8, 0x78, 7, 7, 8) }
void Finale::scene9() { SCENE(0x3af, 0x5f, 0x8c, 5, 8, 9) }

inline signed short cd_off()
{
	return !g_music_off;
}

inline signed short no_cd_music()
{
	return cd_off() && g_music_ready == 1;
}

void Finale::fade_wait(signed short n)
{
	pal_blend2(SRGB(pal_from), SRGB(pal_pic), 0x20, 0x100, 1);
	if (no_cd_music() && g_opt_music) {
		do {
			g_in_wait = 1;
			poll_input();
			g_in_wait = 0;
			wait_for_vsync();
		} while (g_snd_vol_req != n);
	} else
		wait_ticks_key(1000);
	pal_blend2(SRGB(pal_pic), SRGB(pal_from), 0x20, 0x100, 1);
}

void Finale::title()
{
	Pic p;
	unsigned char far *a = new unsigned char[PAL_SIZE];
	unsigned char far *b = new unsigned char[PAL_SIZE];

	if (g_cd_music)
		snd_obj_stop(g_snd_music);
	set_palette(a, 0, 0x100);
	p.load(PIC_TITLE, g_vga_buf, (struct rgb far *)b);
	pal_blend2(SRGB(a), SRGB(b), 0x1e, 0x100, 1);
	cache_clear(g_cd_cache);
	load_pic_list(g_cd_cache, g_cl_finale, g_cl_finale_st, 0, 0);
	if (g_cd_speech)
		load_pic_list(g_cd_cache, g_cl_finale_speech, g_cl_finale_st, 0, 0);
	pal_blend(SRGB(b), 0x19);
	delete b;
	delete a;
}
