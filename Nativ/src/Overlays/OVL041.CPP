/* OVL041 - Overlay O041 (Segment 0x1fe7 in SCHWEIF.EXE): zwei Abspannfolgen (je 0x24 Bytes, s_class.h; Basis
 * struct text_screen O181 wie O039 Intro/O040 Anim3; O007 und O183 legen sie lokal an).
 * Outro1 (O041_0000/00AB/10DC): Dateiliste g_cl_outro1 (bei g_cd_speech zusaetzlich g_cl_outro1_speech) in den
 * CD-Cache, font = new Font(1) mit Farbsatz 0xdf, tscr_open; ~Outro1: tscr_close(1), delete font. Outro2
 * (O041_00F5/018B/0209): Texte 0xa9 (0x32 Stueck) nach texts_buf/texts, font wie oben (Farbe 0xe2); ~Outro2
 * gibt zusaetzlich die Texte frei und ruft cache_purge. run(): bei g_speech_ext_avail voice(), sonst plain().
 * plain()/voice() (Outro2: O041_0233/08BB, Outro1: O041_1106/16E1, Outro1 laedt die Texte selbst in Locals): lokale
 * Zeilentabelle lines (9 bzw. 10 x {Text, Sprachnummer, Animation, Wechsel}) mit texts[10..17] bzw. tl[0..8] anlegen
 * (BCC-Vorlagen im DS: ds:40a8/4102/415c/41c0, MAP-Namen g_outro2_lines usw.), fuenf Paletten, Bild 0x364 (Pic)
 * einblenden, Animation 0x3b7 bzw. 0x3b5 (ani_load/anim_reset auf g_screen_pic, Ausschnitt 0x38/0x15) per
 * pal_crossfade einblenden; mit Sprachausgabe (g_speech_on == 1 und speech_active) je Zeile Untertitel (bei
 * g_subtitles) und Sprache snd_speech mit Warteschleife snd_speech_busy, sonst 200 Takte (wait_ticks_key) je Zeile
 * (Outro2::plain per switch auf texts[10..17]); voice() wechselt bei gesetztem change die Animation (lines.e[i].ani,
 * Schriftfarbe nach 0x3b7/0x3b5) und spielt wait_ticks(0x10) bei festen Zeilen; Abschluss: ausblenden, Paletten
 * frei, g_refresh = 1, g_clock_update zurueck.
 *
 * BCC -mlarge -3 -Ot -Og -c -Y OVL041.CPP   -> 7904/7904 Bytes GLEICH (18.10.2026), cobjfix laeuft
 *   (Namensfassung mit schweif.h: 19.10.2026 GLEICH, LINKGLEICH)
 * - -Og noetig (ohne 23 Bytes zu lang): Epilogkopien in run() (if/else mit zwei Aufrufen weiter unten im Modul
 *   -> far call, pop bp / retf je Zweig) und der Zwischenwert i * 10 ([bp-2ch] usw.) fuer lines.e[i - 1].
 * - Kein -Oi: memcpy/memset sind echte Aufrufe (wie O039/O040).
 * - Auto-Aggregat mit nicht konstanten Initialisierern (C++-Deklaration mitten im Rumpf): BCC legt eine
 *   unbenannte Vorlage im eigenen _DATA an (in Definitionsreihenfolge), schreibt die Laufzeitwerte (Textzeiger)
 *   hinein und kopiert sie dann als Ganzes auf den Stack (rep movsw). Das " " der letzten Zeile kommt in
 *   Auftretensreihenfolge in den Literalpool - darum wechseln sich im Pool Vorlagen-Literal und die drei
 *   Code-Literale jeder Funktion ab (Phase B, 19.10.2026). Mit Member texts (les this / les es:[bx+20h])
 *   werden die Textzeiger als zwei Wort-Stores geschrieben, mit lokalem tl (Outro1) als mov eax.
 * - lines.e[i - 1].snd: -1 wird in die lea-Basis gefaltet ([bp-90h]), der Feldversatz dagegen als "add ax,4"
 *   addiert; lines.e[i].ani ergibt lea dx,[bp-84h] direkt.
 * - Paletten dark, pic, work, ani, saved in Deklarationsreihenfolge, angelegt dark, pic, saved, ani, work (wie
 *   OVL040); Outro1::voice "signed short change = 0;" zwischen b und pal_dark deklariert (Initialisierung vor
 *   mouse_hide).
 * Originalfehler/Auffaelliges: "s = \" \";" (Leertext) am Schleifenanfang wird sofort ueberschrieben
 *   (alle vier Laeufe). Outro1::plain: k ist immer 0, "k % 2 + 0x3b5" also fest 0x3b5. voice() ohne
 *   Sprachausgabe liest nach dem letzten i++ lines.e[i - 1].ani hinter dem Feld (Outro2: e[9] bei 9 Eintraegen;
 *   folgenlos, weil change des letzten Eintrags 0 ist). Outro2::plain zeigt mit Sprachausgabe nur 8 der 9 Zeilen.
 */
#include <mem.h>
#include "schweif.h"

/* Phase B: eigene Daten des Moduls (ds:4072..4244, dsana) - initialisierte Daten in Adressreihenfolge, danach der Literalpool */
char g_cl_outro1[12] = { 0x64, 0x23, 0xb5, 0x23, 0xb6, 0x23, 0xb7, 0x23, 0xb8, 0x23, 0xff, 0xff };
char g_cl_outro1_speech[36] = {
	0xc9, 0x60, 0xca, 0x60, 0xcb, 0x60, 0xcc, 0x60, 0xcd, 0x60, 0xce, 0x60, 0xcf, 0x60, 0xd0, 0x60,
	0xd1, 0x60, 0xd2, 0x60, 0xd3, 0x60, 0xd4, 0x60, 0xd5, 0x60, 0xd6, 0x60, 0xd7, 0x60, 0xd8, 0x60,
	0xd9, 0x60, 0xff, 0xff
};
char g_cl_outro_st[6] = "";

#define OUTRO_TXT	0xa9		/* Textliste des Vor-/Abspanns (0x32 Texte) */
#define OUTRO_TXT_N	0x32
#define PAL_SIZE	0x300
#define VIEW_X		0x38		/* Bildausschnitt der Hintergrund-Animation */
#define VIEW_Y		0x15
#define SUB_H		0x13		/* Untertitelstreifen: Hoehe, x, Breite (tscr_print_font) */
#define SUB_X		0x14
#define SUB_W		0x118

Outro1::Outro1()
{
	load_pic_list(g_cd_cache, g_cl_outro1, g_cl_outro_st, 1, 0);
	if (g_cd_speech)
		load_pic_list(g_cd_cache, g_cl_outro1_speech, g_cl_outro_st, 1, 0);
	font = new Font(1);
	font_push_colset(font, 0xdf, 0xba, 0xff, 0xff, 0xff);
	tscr_open(this);
}

Outro1::~Outro1()
{
	tscr_close(this, 1);
	delete font;
}

Outro2::Outro2()
{
	((Font far *)g_font)->load_list(OUTRO_TXT, OUTRO_TXT_N, &texts_buf, &texts);
	font = new Font(1);
	font_push_colset(font, 0xe2, 0xba, 0xff, 0xff, 0xff);
	tscr_open(this);
}

Outro2::~Outro2()
{
	tscr_close(this, 1);
	delete font;
	mem_free(texts_buf);
	mem_free(texts);
	cache_purge(g_cd_cache, 1);
}

void Outro2::run()
{
	if (g_speech_ext_avail)
		voice();
	else
		plain();
}

void Outro2::plain()
{
	char far *s;
	signed short old;
	signed short i;
	signed short snd;
	Pic far *pic;
	unsigned char far *buf;
	Bmp far *b;
	unsigned char far *pal_dark;
	unsigned char far *pal_pic;
	unsigned char far *pal_work;
	unsigned char far *pal_ani;
	unsigned char far *pal_saved;

	mouse_hide(0);
	snd_obj_stop(g_snd_fx);
	Speech_stop(g_snd_speech);
	old = g_clock_update;
	g_clock_update = 0;
	g_dialog_depth++;
	i = 1;
	struct outro_lines9 lines = { {		/* Text, Sprachnummer, Animation, Wechsel zur naechsten Animation */
		{ texts[10], 210, 951, 1 },
		{ texts[11], 211, 951, 1 },
		{ texts[12], 212, 952, 1 },
		{ texts[13], 213, 952, 1 },
		{ texts[14], 214, 952, 1 },
		{ texts[15], 215, 951, 0 },
		{ texts[16], 216, 951, 1 },
		{ texts[17], 217, 952, 0 },
		{ " ", 0, 0, 0 }
	} };
	pal_dark = (unsigned char far *)mem_alloc(PAL_SIZE);
	pal_pic = (unsigned char far *)mem_alloc(PAL_SIZE);
	pal_saved = (unsigned char far *)mem_alloc(PAL_SIZE);
	pal_ani = (unsigned char far *)mem_alloc(PAL_SIZE);
	pal_work = (unsigned char far *)mem_alloc(PAL_SIZE);
	memcpy(pal_saved, g_palette, PAL_SIZE);
	arc_mark(g_file_3);
	memcpy(pal_saved, g_palette, PAL_SIZE);
	buf = (unsigned char far *)mem_alloc(0xfa00L);
	pic = new Pic;
	pic->load(0x364, buf);
	memcpy(pal_pic, pic->get_palette(), PAL_SIZE);
	set_palette(pal_pic, 0, 0x100);
	delete pic;
	memset(pal_work, 0, PAL_SIZE);
	set_palette(pal_work, 0, 0xf0);
	memcpy(pal_dark, g_palette, PAL_SIZE);
	b = new Bmp(0x140, 0xc8, buf);
	b->put(g_vga_buf, 0, 0);
	mem_free(buf);
	delete b;
	ani_load(g_screen_pic, 0x3b7);
	ani_set_pos(g_screen_pic, VIEW_X, VIEW_Y);
	memcpy(pal_ani, g_screen_pic_pal, PAL_SIZE);
	get_palette(pal_ani + 0x2d0, 0xf0, 0x10);
	g_screen_pic_state = 0;
	memcpy(pal_work, pal_dark, PAL_SIZE);
	pal_crossfade(pal_work, pal_ani, 0x100);
	ani_play_until(g_screen_pic, 3, 1);
	anim_reset(g_screen_pic, 5);
	if (g_speech_on == 1 && speech_active(g_snd_speech)) {
		for (i = 1; i <= 8; i++) {
			s = " ";
			anim_reset(g_screen_pic, 5);
			s = lines.e[i - 1].text;
			snd = lines.e[i - 1].snd;
			if (g_subtitles)
				tscr_print_font(this, s, font, SUB_H, SUB_X, SUB_W);
			if (g_speech_on == 1 && speech_active(g_snd_speech) && snd) {
				snd_speech(g_snd_speech, snd);
				snd = 0;
				while (snd_speech_busy(g_snd_speech)) {
					game_idle();
					wait_for_vsync();
				}
			} else
				wait_ticks_key(200);
		}
		tscr_print_font(this, " ", font, SUB_H, SUB_X, SUB_W);
	} else {
		while (i < 10) {
			switch (i) {
			case 1:
				s = texts[10];
				break;
			case 2:
				s = texts[11];
				break;
			case 3:
				s = texts[12];
				break;
			case 4:
				s = texts[13];
				break;
			case 5:
				s = texts[14];
				break;
			case 6:
				s = texts[15];
				break;
			case 7:
				s = texts[16];
				break;
			case 8:
				s = texts[17];
				break;
			case 9:
				s = " ";
				break;
			}
			if (i <= 9)
				tscr_print_font(this, s, font, SUB_H, SUB_X, SUB_W);
			i++;
			if (i < 10)
				wait_ticks_key(200);
		}
	}
	memcpy(pal_work, g_palette, PAL_SIZE);
	pal_crossfade(pal_work, pal_dark, 0x100);
	ani_stop(g_screen_pic);
	g_screen_pic_state = 5;
	clear_screen_buf(g_vga_buf);
	set_palette(pal_saved, 0, 0x100);
	mem_free(pal_saved);
	mem_free(pal_ani);
	mem_free(pal_work);
	mem_free(pal_dark);
	mem_free(pal_pic);
	arc_release(g_file_3);
	g_refresh = 1;
	mouse_show(0);
	g_dialog_depth--;
	g_clock_update = old;
}

void Outro2::voice()
{
	char far *s;
	signed short old;
	signed short i;
	signed short snd;
	signed short ani;
	Pic far *pic;
	unsigned char far *buf;
	Bmp far *b;
	unsigned char far *pal_dark;
	unsigned char far *pal_pic;
	unsigned char far *pal_work;
	unsigned char far *pal_ani;
	unsigned char far *pal_saved;
	signed short change;

	mouse_hide(0);
	snd_obj_stop(g_snd_fx);
	Speech_stop(g_snd_speech);
	old = g_clock_update;
	g_clock_update = 0;
	g_dialog_depth++;
	i = 1;
	struct outro_lines9 lines = { {		/* Text, Sprachnummer, Animation, Wechsel zur naechsten Animation */
		{ texts[10], 210, 951, 1 },
		{ texts[11], 211, 952, 1 },
		{ texts[12], 212, 951, 1 },
		{ texts[13], 213, 952, 1 },
		{ texts[14], 214, 951, 1 },
		{ texts[15], 215, 952, 0 },
		{ texts[16], 216, 952, 1 },
		{ texts[17], 217, 951, 0 },
		{ " ", 0, 0, 0 }
	} };
	pal_dark = (unsigned char far *)mem_alloc(PAL_SIZE);
	pal_pic = (unsigned char far *)mem_alloc(PAL_SIZE);
	pal_saved = (unsigned char far *)mem_alloc(PAL_SIZE);
	pal_ani = (unsigned char far *)mem_alloc(PAL_SIZE);
	pal_work = (unsigned char far *)mem_alloc(PAL_SIZE);
	memcpy(pal_saved, g_palette, PAL_SIZE);
	arc_mark(g_file_3);
	memcpy(pal_saved, g_palette, PAL_SIZE);
	buf = (unsigned char far *)mem_alloc(0xfa00L);
	pic = new Pic;
	pic->load(0x364, buf);
	memcpy(pal_pic, pic->get_palette(), PAL_SIZE);
	set_palette(pal_pic, 0, 0x100);
	delete pic;
	memset(pal_work, 0, PAL_SIZE);
	set_palette(pal_work, 0, 0xf0);
	memcpy(pal_dark, g_palette, PAL_SIZE);
	b = new Bmp(0x140, 0xc8, buf);
	b->put(g_vga_buf, 0, 0);
	mem_free(buf);
	delete b;
	ani_load(g_screen_pic, 0x3b7);
	ani_set_pos(g_screen_pic, VIEW_X, VIEW_Y);
	memcpy(pal_ani, g_screen_pic_pal, PAL_SIZE);
	get_palette(pal_ani + 0x2d0, 0xf0, 0x10);
	g_screen_pic_state = 0;
	memcpy(pal_work, pal_dark, PAL_SIZE);
	pal_crossfade(pal_work, pal_ani, 0x100);
	anim_reset(g_screen_pic, 3);
	change = 0;
	if (g_speech_on == 1 && speech_active(g_snd_speech)) {
		for (i = 1; i <= 9; i++) {
			s = " ";
			anim_reset(g_screen_pic, 5);
			s = lines.e[i - 1].text;
			snd = lines.e[i - 1].snd;
			change = lines.e[i - 1].change;
			if (i == 1 || i == 3 || i == 5 || i == 8)
				wait_ticks(0x10);
			if (g_subtitles)
				tscr_print_font(this, s, font, SUB_H, SUB_X, SUB_W);
			if (g_speech_on == 1 && speech_active(g_snd_speech) && snd) {
				snd_speech(g_snd_speech, snd);
				snd = 0;
				while (snd_speech_busy(g_snd_speech)) {
					game_idle();
					wait_for_vsync();
				}
			}
			if (change == 1) {
				change = 0;
				tscr_print_font(this, " ", font, SUB_H, SUB_X, SUB_W);
				wait_ticks(0x10);
				ani = lines.e[i].ani;
				if (ani == 0x3b7)
					font_push_colset(font, 0xe2, 0xba, 0xff, 0xff, 0xff);
				else
					font_push_colset(font, 0xbf, 0xba, 0xff, 0xff, 0xff);
				set_palette(pal_dark, 0, 0x100);
				ani_load(g_screen_pic, ani);
				ani_set_pos(g_screen_pic, VIEW_X, VIEW_Y);
				memcpy(pal_ani, g_screen_pic_pal, PAL_SIZE);
				g_screen_pic_state = 0;
				anim_reset(g_screen_pic, 3);
				set_palette(pal_ani, 0, 0xf0);
			}
		}
	} else {
		anim_reset(g_screen_pic, 5);
		poll_input();
		while (i <= 9) {
			s = lines.e[i - 1].text;
			change = lines.e[i - 1].change;
			if (i <= 8)
				tscr_print_font(this, s, font, SUB_H, SUB_X, SUB_W);
			i++;
			if (i <= 9)
				wait_ticks_key(200);
#if FIX_0190
			/* Fix #190: nach dem letzten Eintrag nicht hinter das Feld lesen */
			ani = i <= 9 ? lines.e[i - 1].ani : 0;
			if (change == 1) {
#else
			ani = lines.e[i - 1].ani;
			if (change == 1) {
#endif
				change = 0;
				ani_play_until(g_screen_pic, 5, 0x63);
				tscr_print_font(this, " ", font, SUB_H, SUB_X, SUB_W);
				if (i == 9)
					i = 0x63;
				if (i != 0x63) {
					if (ani == 0x3b7)
						font_push_colset(font, 0xe2, 0xba, 0xff, 0xff, 0xff);
					else
						font_push_colset(font, 0xbf, 0xba, 0xff, 0xff, 0xff);
					set_palette(pal_dark, 0, 0x100);
					ani_load(g_screen_pic, ani);
					ani_set_pos(g_screen_pic, VIEW_X, VIEW_Y);
					memcpy(pal_ani, g_screen_pic_pal, PAL_SIZE);
					g_screen_pic_state = 0;
					anim_reset(g_screen_pic, 3);
					set_palette(pal_ani, 0, 0xf0);
				}
			}
		}
	}
	memcpy(pal_work, g_palette, PAL_SIZE);
	pal_crossfade(pal_work, pal_dark, 0x100);
	ani_stop(g_screen_pic);
	g_screen_pic_state = 5;
	clear_screen_buf(g_vga_buf);
	set_palette(pal_saved, 0, 0x100);
	mem_free(pal_saved);
	mem_free(pal_ani);
	mem_free(pal_work);
	mem_free(pal_dark);
	mem_free(pal_pic);
	arc_release(g_file_3);
	g_refresh = 1;
	mouse_show(0);
	g_dialog_depth--;
	g_clock_update = old;
}

void Outro1::run()
{
	if (g_speech_ext_avail)
		voice();
	else
		plain();
}

void Outro1::plain()
{
	void far *tl_buf;
	char far *s;
	char far * far *tl;
	signed short old;
	signed short snd;
	signed short i;
	signed short k;
	Pic far *pic;
	unsigned char far *buf;
	Bmp far *b;
	unsigned char far *pal_dark;
	unsigned char far *pal_pic;
	unsigned char far *pal_work;
	unsigned char far *pal_ani;
	unsigned char far *pal_saved;

	mouse_hide(0);
	((Font far *)g_font)->load_list(OUTRO_TXT, OUTRO_TXT_N, &tl_buf, &tl);
	struct outro_lines10 lines = { {		/* Text, Sprachnummer, Animation, Wechsel zur naechsten Animation */
		{ tl[0], 201, 949, 0 },
		{ tl[1], 202, 949, 1 },
		{ tl[2], 203, 950, 0 },
		{ tl[3], 204, 950, 0 },
		{ tl[4], 205, 950, 1 },
		{ tl[5], 206, 949, 0 },
		{ tl[6], 207, 949, 1 },
		{ tl[7], 208, 950, 0 },
		{ tl[8], 209, 950, 0 },
		{ " ", 0, 0, 0 }
	} };
	snd_obj_stop(g_snd_fx);
	Speech_stop(g_snd_speech);
	old = g_clock_update;
	g_clock_update = 0;
	g_dialog_depth++;
	i = 1;
	k = 0;
	snd = 0;
	pal_dark = (unsigned char far *)mem_alloc(PAL_SIZE);
	pal_pic = (unsigned char far *)mem_alloc(PAL_SIZE);
	pal_saved = (unsigned char far *)mem_alloc(PAL_SIZE);
	pal_ani = (unsigned char far *)mem_alloc(PAL_SIZE);
	pal_work = (unsigned char far *)mem_alloc(PAL_SIZE);
	memcpy(pal_saved, g_palette, PAL_SIZE);
	arc_mark(g_file_3);
	buf = (unsigned char far *)mem_alloc(0xfa00L);
	pic = new Pic;
	pic->load(0x364, buf);
	memcpy(pal_pic, pic->get_palette(), PAL_SIZE);
	set_palette(pal_pic, 0, 0x100);
	delete pic;
	memset(pal_work, 0, PAL_SIZE);
	set_palette(pal_work, 0, 0xf0);
	memcpy(pal_dark, g_palette, PAL_SIZE);
	b = new Bmp(0x140, 0xc8, buf);
	b->put(g_vga_buf, 0, 0);
	mem_free(buf);
	delete b;
	ani_load(g_screen_pic, k % 2 + 0x3b5);
	ani_set_pos(g_screen_pic, VIEW_X, VIEW_Y);
	memcpy(pal_ani, g_screen_pic_pal, PAL_SIZE);
	get_palette(pal_ani + 0x2d0, 0xf0, 0x10);
	g_screen_pic_state = 0;
	memcpy(pal_work, pal_dark, PAL_SIZE);
	pal_crossfade(pal_work, pal_ani, 0x100);
	anim_reset(g_screen_pic, 3);
	if (g_speech_on == 1 && speech_active(g_snd_speech)) {
		for (i = 1; i <= 9; i++) {
			s = " ";
			anim_reset(g_screen_pic, 5);
			s = lines.e[i - 1].text;
			snd = lines.e[i - 1].snd;
			if (g_subtitles)
				tscr_print_font(this, s, font, SUB_H, SUB_X, SUB_W);
			if (g_speech_on == 1 && speech_active(g_snd_speech) && snd) {
				snd_speech(g_snd_speech, snd);
				snd = 0;
				while (snd_speech_busy(g_snd_speech)) {
					game_idle();
					wait_for_vsync();
				}
			} else
				wait_ticks_key(200);
		}
		tscr_print_font(this, " ", font, SUB_H, SUB_X, SUB_W);
	} else {
		ani_play_until(g_screen_pic, 5, 1);
		anim_reset(g_screen_pic, 5);
		while (i <= 10) {
			s = lines.e[i - 1].text;
			if (i <= 9)
				tscr_print_font(this, s, font, SUB_H, SUB_X, SUB_W);
			i++;
			if (i <= 10)
				wait_ticks_key(200);
		}
		tscr_print_font(this, " ", font, SUB_H, SUB_X, SUB_W);
	}
	memcpy(pal_work, g_palette, PAL_SIZE);
	pal_crossfade(pal_work, pal_dark, 0x100);
	ani_stop(g_screen_pic);
	g_screen_pic_state = 5;
	clear_screen_buf(g_vga_buf);
	set_palette(pal_saved, 0, 0x100);
	mem_free(pal_saved);
	mem_free(pal_ani);
	mem_free(pal_work);
	mem_free(pal_dark);
	mem_free(pal_pic);
	mem_free(tl_buf);
	mem_free(tl);
	arc_release(g_file_3);
	g_refresh = 1;
	mouse_show(0);
	g_dialog_depth--;
	g_clock_update = old;
}

void Outro1::voice()
{
	void far *tl_buf;
	char far *s;
	char far * far *tl;
	signed short old;
	signed short snd;
	signed short i;
	signed short ani;
	Pic far *pic;
	unsigned char far *buf;
	Bmp far *b;
	signed short change = 0;
	unsigned char far *pal_dark;
	unsigned char far *pal_pic;
	unsigned char far *pal_work;
	unsigned char far *pal_ani;
	unsigned char far *pal_saved;

	mouse_hide(0);
	((Font far *)g_font)->load_list(OUTRO_TXT, OUTRO_TXT_N, &tl_buf, &tl);
	struct outro_lines10 lines = { {		/* Text, Sprachnummer, Animation, Wechsel zur naechsten Animation */
		{ tl[0], 201, 949, 0 },
		{ tl[1], 202, 949, 1 },
		{ tl[2], 203, 950, 0 },
		{ tl[3], 204, 950, 0 },
		{ tl[4], 205, 950, 1 },
		{ tl[5], 206, 949, 0 },
		{ tl[6], 207, 949, 1 },
		{ tl[7], 208, 950, 0 },
		{ tl[8], 209, 950, 0 },
		{ " ", 0, 0, 0 }
	} };
	snd_obj_stop(g_snd_fx);
	Speech_stop(g_snd_speech);
	old = g_clock_update;
	g_clock_update = 0;
	g_dialog_depth++;
	i = 1;
	snd = 0;
	pal_dark = (unsigned char far *)mem_alloc(PAL_SIZE);
	pal_pic = (unsigned char far *)mem_alloc(PAL_SIZE);
	pal_saved = (unsigned char far *)mem_alloc(PAL_SIZE);
	pal_ani = (unsigned char far *)mem_alloc(PAL_SIZE);
	pal_work = (unsigned char far *)mem_alloc(PAL_SIZE);
	memcpy(pal_saved, g_palette, PAL_SIZE);
	arc_mark(g_file_3);
	buf = (unsigned char far *)mem_alloc(0xfa00L);
	pic = new Pic;
	pic->load(0x364, buf);
	memcpy(pal_pic, pic->get_palette(), PAL_SIZE);
	set_palette(pal_pic, 0, 0x100);
	delete pic;
	memset(pal_work, 0, PAL_SIZE);
	set_palette(pal_work, 0, 0xf0);
	memcpy(pal_dark, g_palette, PAL_SIZE);
	b = new Bmp(0x140, 0xc8, buf);
	b->put(g_vga_buf, 0, 0);
	mem_free(buf);
	delete b;
	ani_load(g_screen_pic, 0x3b5);
	ani_set_pos(g_screen_pic, VIEW_X, VIEW_Y);
	memcpy(pal_ani, g_screen_pic_pal, PAL_SIZE);
	get_palette(pal_ani + 0x2d0, 0xf0, 0x10);
	g_screen_pic_state = 0;
	memcpy(pal_work, pal_dark, PAL_SIZE);
	pal_crossfade(pal_work, pal_ani, 0x100);
	anim_reset(g_screen_pic, 3);
	if (g_speech_on == 1 && speech_active(g_snd_speech)) {
		for (i = 1; i <= 9; i++) {
			s = " ";
			anim_reset(g_screen_pic, 5);
			s = lines.e[i - 1].text;
			snd = lines.e[i - 1].snd;
			change = lines.e[i - 1].change;
			if (i == 1 || i == 3 || i == 6 || i == 8)
				wait_ticks(0x10);
			if (g_subtitles)
				tscr_print_font(this, s, font, SUB_H, SUB_X, SUB_W);
			if (g_speech_on == 1 && speech_active(g_snd_speech) && snd) {
				snd_speech(g_snd_speech, snd);
				snd = 0;
				while (snd_speech_busy(g_snd_speech)) {
					game_idle();
					wait_for_vsync();
				}
			}
			if (change == 1) {
				change = 0;
				tscr_print_font(this, " ", font, SUB_H, SUB_X, SUB_W);
				wait_ticks(0x10);
				ani = lines.e[i].ani;
				if (ani == 0x3b5)
					font_push_colset(font, 0xdf, 0xba, 0xff, 0xff, 0xff);
				else
					font_push_colset(font, 0xbf, 0xba, 0xff, 0xff, 0xff);
				set_palette(pal_dark, 0, 0x100);
				ani_load(g_screen_pic, ani);
				ani_set_pos(g_screen_pic, VIEW_X, VIEW_Y);
				memcpy(pal_ani, g_screen_pic_pal, PAL_SIZE);
				g_screen_pic_state = 0;
				anim_reset(g_screen_pic, 3);
				set_palette(pal_ani, 0, 0xf0);
			}
		}
	} else {
		anim_reset(g_screen_pic, 5);
		poll_input();
		while (i <= 10) {
			s = lines.e[i - 1].text;
			change = lines.e[i - 1].change;
			if (i <= 9)
				tscr_print_font(this, s, font, SUB_H, SUB_X, SUB_W);
			i++;
			if (i <= 10)
				wait_ticks_key(200);
#if FIX_0190
			/* Fix #190: nach dem letzten Eintrag nicht hinter das Feld lesen */
			ani = i <= 10 ? lines.e[i - 1].ani : 0;
			if (change == 1) {
#else
			ani = lines.e[i - 1].ani;
			if (change == 1) {
#endif
				change = 0;
				ani_play_until(g_screen_pic, 5, 0x63);
				tscr_print_font(this, " ", font, SUB_H, SUB_X, SUB_W);
				if (i == 9)
					i = 0x63;
				if (i != 0x63) {
					if (ani == 0x3b5)
						font_push_colset(font, 0xdf, 0xba, 0xff, 0xff, 0xff);
					else
						font_push_colset(font, 0xbf, 0xba, 0xff, 0xff, 0xff);
					set_palette(pal_dark, 0, 0x100);
					ani_load(g_screen_pic, ani);
					ani_set_pos(g_screen_pic, VIEW_X, VIEW_Y);
					memcpy(pal_ani, g_screen_pic_pal, PAL_SIZE);
					g_screen_pic_state = 0;
					anim_reset(g_screen_pic, 3);
					set_palette(pal_ani, 0, 0xf0);
				}
			}
		}
	}
	memcpy(pal_work, g_palette, PAL_SIZE);
	pal_crossfade(pal_work, pal_dark, 0x100);
	ani_stop(g_screen_pic);
	g_screen_pic_state = 5;
	clear_screen_buf(g_vga_buf);
	set_palette(pal_saved, 0, 0x100);
	mem_free(pal_saved);
	mem_free(pal_ani);
	mem_free(pal_work);
	mem_free(pal_dark);
	mem_free(pal_pic);
	mem_free(tl_buf);
	mem_free(tl);
	arc_release(g_file_3);
	g_refresh = 1;
	mouse_show(0);
	g_dialog_depth--;
	g_clock_update = old;
}
