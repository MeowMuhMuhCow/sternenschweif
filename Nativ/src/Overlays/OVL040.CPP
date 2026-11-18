/* OVL040 - Overlay O040 (Segment 0x1fe3 in SCHWEIF.EXE): Bildfolge Anim3 (0x24 Bytes, s_class.h; Basis struct
 * text_screen O181 wie O039 Intro; von O183 show_anim3 und O222 lokal angelegt) - animierte Bildfolge im
 * Bildausschnitt (0x38, 0x15) mit Untertiteln/Sprache. Anim3(): Dateiliste g_cl_anim3 (bei g_cd_speech
 * zusaetzlich g_cl_anim3_speech) in den CD-Cache, Texte 0x408 (0x14 Stueck) nach texts_buf/texts, font = new
 * Font(1) mit Farbsatz, tscr_open. ~Anim3(): tscr_close(1), delete font, Texte frei, cache_purge.
 * cue_text(n, &snd) (O040_014E): switch 0..31 -> Text texts[0..17] und Sprachnummer 0x12..0x17/0xe4..0xef
 * (1: snd = -1 = Klang 0x84), sonst " ". play(stage, ani, loop, pic, x, y, mode) (O040_03FB): Animation ani
 * (Anim) mit Hintergrundbild 0x330/pic (Bmp 0xd0 x 0x87), Paletten-Ueberblendung (mode 2/3 -> fade,
 * pal_crossfade), Bildwechsel abspielen, je erreichter Marke stage * 4 + cue Untertitel per cue_text und
 * Sprache (snd_speech, warten snd_speech_busy) bzw. Klang 0x84; bei ani 0x338 mit Sprache stattdessen feste
 * Liste g_anim3_lines (texts[3..17] + Sprachnummern, als Strukturkopie auf den Stack). blank(pal) (O040_0BC4):
 * leerer Untertitel, Hintergrund 0x330/0, Palette. run() (O040_0C7E): Bild 0x364 (Pic), Folge der Animationen
 * 0x339/0x242/0x243/0x244/0x245/0x338/0x247/0x246 (Kurzfassung bei Sprachausgabe), Schlussbild 0x36e und
 * Animation 0x36f, Ausblenden, Puffer frei, zum Schluss diary_add(texts[19]).
 *
 * BCC -mlarge -3 -Ot -Og -c -Y OVL040.CPP   -> 4930/4930 Bytes GLEICH (18.10.2026), cobjfix laeuft
 *   (Namensfassung mit schweif.h: 19.10.2026 GLEICH, LINKGLEICH)
 * - Ohne -Og 1474 Abweichungen (Epilog-/Zwischenwerte, Offset idx * 6 in [bp-40h]). Kein -Oi: memcpy/memset sind
 *   Aufrufe.
 * - Aufrufe innerhalb des Moduls (cue_text/play/blank stehen weiter oben) -> push cs / call near.
 * - "running() & 1" (unsigned char) -> test al,1; ohne "& 1" or al,al.
 * - Initialisierte Tabelle g_anim3_lines: Original schreibt die 15 Textzeiger in die DS-Vorlage und kopiert sie
 *   dann als Ganzes (rep movsw, 0x30 Worte) auf den Stack -> "g_anim3_lines.e[i].text = texts[..]; ... struct
 *   cue_lines16 arr = g_anim3_lines;" (BCCs Verfahren fuer ein auto-Aggregat mit nicht konstanten
 *   Initialisierern; so ohne eigene Daten).
 * - Locals in Reihenfolge des ersten Auftretens: Anim a mitten im Rumpf, inline-Kopien set_loop/set_dst
 *   ([bp-2fh], [bp-34h]) vor Bmp bg ([bp-3eh]); a und arr (0x60) am Ende. In run() Paletten in der Reihenfolge
 *   dark, black, pic, work, saved deklarieren (Stackplaetze), zugewiesen dark, saved, work, pic, black.
 * - "if ((ani == 0x36e && mode == 0) || ani == 0x244) ; else {..}" -> jmp short ueber den Rumpf.
 * Originalfehler: play() benutzt fade uninitialisiert, wenn mode <= 1 (Vergleiche fade == 2 / fade == 1);
 *   "ani != 0x36e || ani != 0x244" ist immer wahr (gemeint &&) -> der Zweig mit nvfile_pal/set_palette ist tot;
 *   "delete b" am Ende bei ani 0x36e/0x244 auch dann, wenn b nicht angelegt wurde (mode == 1 ohne fade 2).
 */
#include <mem.h>
#include "schweif.h"

/* Phase B: eigene Daten des Moduls (ds:3fc0..4072, dsana) - initialisierte Daten in Adressreihenfolge, danach der Literalpool */
char g_cl_anim3[32] = {
	0x64, 0x23, 0x30, 0x23, 0x42, 0x22, 0x43, 0x22, 0x44, 0x22, 0x45, 0x22, 0x46, 0x22, 0x47, 0x22,
	0x38, 0x23, 0x39, 0x23, 0x6e, 0x23, 0x6f, 0x23, 0x08, 0x24, 0x84, 0x40, 0xa8, 0x40, 0xff, 0xff
};
char g_cl_anim3_speech[38] = {
	0x12, 0x60, 0xef, 0x60, 0x13, 0x60, 0x14, 0x60, 0x15, 0x60, 0x16, 0x60, 0x17, 0x60, 0xe4, 0x60,
	0xe5, 0x60, 0xe6, 0x60, 0xe7, 0x60, 0xe8, 0x60, 0xe9, 0x60, 0xea, 0x60, 0xeb, 0x60, 0xec, 0x60,
	0xed, 0x60, 0xee, 0x60, 0xff, 0xff
};
char g_cl_anim3_st[6] = "";
struct cue_lines16 g_anim3_lines = {
	{ { 0, 21 },
	{ 0, 22 },
	{ 0, 23 },
	{ 0, 228 },
	{ 0, 229 },
	{ 0, 230 },
	{ 0, 231 },
	{ 0, 232 },
	{ 0, 233 },
	{ 0, 234 },
	{ 0, 235 },
	{ 0, 236 },
	{ 0, 237 },
	{ 0, 238 },
	{ 0, 239 },
	{ 0, 0 } }
};

#define ANIM3_TXT	0x408		/* Textliste (0x14 Texte) */
#define ANIM3_TXT_N	0x14
#define PAL_SIZE	0x300
#define VIEW_X		0x38		/* Bildausschnitt */
#define VIEW_Y		0x15
#define VIEW_W		0xd0
#define VIEW_H		0x87
#define SUB_H		0x13		/* Untertitelstreifen: Hoehe, x, Breite (tscr_print_font) */
#define SUB_X		0x14
#define SUB_W		0x118

Anim3::Anim3()
{
	load_pic_list(g_cd_cache, g_cl_anim3, g_cl_anim3_st, 1, 0);
	if (g_cd_speech)
		load_pic_list(g_cd_cache, g_cl_anim3_speech, g_cl_anim3_st, 1, 0);
	((Font far *)g_font)->load_list(ANIM3_TXT, ANIM3_TXT_N, &texts_buf, &texts);
	font = new Font(1);
	font_push_colset(font, 0xe2, 0xba, 0xff, 0xff, 0xff);
	tscr_open(this);
}

Anim3::~Anim3()
{
	tscr_close(this, 1);
	delete font;
	mem_free(texts_buf);
	mem_free(texts);
	cache_purge(g_cd_cache, 1);
}

char far *Anim3::cue_text(unsigned short n, signed short far *snd)
{
	char far *s = " ";

	switch (n) {
	case 0: break;
	case 1: *snd = -1; break;
	case 5: s = texts[0]; *snd = 0x12; break;
	case 9: s = texts[1]; *snd = 0x13; break;
	case 11: s = texts[2]; *snd = 0x14; break;
	case 13: s = texts[3]; *snd = 0x15; break;
	case 14: s = texts[4]; *snd = 0x16; break;
	case 15: s = texts[5]; *snd = 0x17; break;
	case 17: s = texts[6]; *snd = 0xe4; break;
	case 18: s = texts[7]; *snd = 0xe5; break;
	case 21: s = texts[8]; *snd = 0xe6; break;
	case 22: s = texts[9]; *snd = 0xe7; break;
	case 23: s = texts[10]; *snd = 0xe8; break;
	case 24: s = texts[11]; *snd = 0xe9; break;
	case 25: s = texts[12]; *snd = 0xea; break;
	case 26: s = texts[13]; *snd = 0xeb; break;
	case 27: s = texts[14]; *snd = 0xec; break;
	case 29: s = texts[15]; *snd = 0xed; break;
	case 30: s = texts[16]; *snd = 0xee; break;
	case 31: s = texts[17]; *snd = 0xef; break;
	}
	return s;
}

void Anim3::play(signed short stage, signed short ani, unsigned char loop, signed short pic, signed short x, signed short y,
	signed short mode)
{
	char far *s;
	signed char fade;
	signed short snd;
	signed short x1;
	signed short y1;
	signed short w1;
	signed short h1;
	signed short num;
	signed short cue;
	signed short max;
	signed short w;
	signed short h;
	signed short dummy;
	Bmp far *b;
	Bmp far *b2;
	unsigned char far *pal_pic;
	unsigned char far *pal_saved;
	signed short voice = 0;

#if FIX_0125
	/* Fix #125: fade und b vorbelegen */
	fade = 0;
	b = 0;
	pal_pic = (unsigned char far *)mem_alloc(PAL_SIZE);
	pal_saved = (unsigned char far *)mem_alloc(PAL_SIZE);
	poll_input();
#else
	pal_pic = (unsigned char far *)mem_alloc(PAL_SIZE);
	pal_saved = (unsigned char far *)mem_alloc(PAL_SIZE);
	poll_input();
#endif
	if (mode > 1) {
		if (mode == 2)
			fade = 2;
		else
			fade = 1;
		mode = fade - 1;
		memcpy(pal_saved, g_palette, PAL_SIZE);
	}
	Anim a(ani, mode, 0);
	if (ani == 0x36e) {
		if (fade == 2)
			a.rate = 2;
		else
			a.rate = 3;
	}
	a.set_loop(loop);
	a.set_dst(g_vga_buf);
	a.size(&w, &h);
	Bmp bg(w, h);
	if (mode != 1 || (mode == 1 && fade == 2)) {
		b = new Bmp(VIEW_W, VIEW_H);
		b->clear();
		nvfile_load(g_nvf_heads, 0x330, b->data(), 0, pic, 0, &dummy, &dummy, 0x140);
#if FIX_0125
		/* Fix #125: && statt || (Palettenzweig fuer 0x36e/0x244) */
		if (ani != 0x36e && ani != 0x244)
#else
		if (ani != 0x36e || ani != 0x244)
#endif
			a.pal_set(1, 0xef);
		else {
			nvfile_pal(g_nvf_heads, 0x330, pal_pic, 0xf0);
			set_palette(pal_pic, 0, 0xf0);
		}
		get_palette(pal_pic, 0, 0xf0);
		if (fade == 1) {
			set_palette(pal_saved, 0, 0x100);
			b->put(g_vga_buf, VIEW_X, VIEW_Y, 4);
			pal_crossfade(pal_saved, pal_pic, 0xf0);
			wait_ticks(0xfc);
			snd_play(g_snd_fx, 0xa8);
			wait_ticks(0xd8);
		} else {
			if (ani != 0x247 && fade != 2) {
				if (ani == 0x36e || ani == 0x244)
					a.setbg(b);
				else {
					a.rect(&x1, &y1, &w1, &h1);
					b2 = new Bmp(w1, h1);
					a.unpack(b2);
					b->over(b2, x + x1 - 0x38, y + y1 - 0x15, 4);
					delete b2;
				}
			}
			if (ani != 0x36e && ani != 0x244)
				b->put(g_vga_buf, VIEW_X, VIEW_Y, 4);
		}
		if (ani != 0x36e && ani != 0x244)
			delete b;
	}
	if ((ani == 0x36e && mode == 0) || ani == 0x244)
		;
	else {
		bg.get(g_vga_buf, x, y);
		a.setbg(&bg);
	}
	max = 0;
	a.start(x, y, 0);
	if (ani != 0x338)
		voice = 1;
	else if (g_speech_on == 1 && speech_active(g_snd_speech))
		voice = 0;
	else
		voice = 1;
	if (voice != 0) {
		do {
			a.wait_cue();
			num = 0;
			snd = 0;
			cue = a.cue;
			if (cue >= 3 && a.rounds >= 1)
				num = cue + stage * 4;
			else if (cue > 0 && cue <= 2 && a.rounds < 1)
				num = cue + stage * 4;
			if (num > max) {
				max = num;
				s = cue_text(num, &snd);
			} else
				s = 0;
			if (s != 0 && g_subtitles)
				tscr_print_font(this, s, font, SUB_H, SUB_X, SUB_W);
			if (snd == -1)
				snd_play(g_snd_fx, 0x84);
			else if (g_speech_on == 1 && speech_active(g_snd_speech) && snd != 0) {
				snd_speech(g_snd_speech, snd);
				snd = 0;
				while (snd_speech_busy(g_snd_speech)) {
					game_idle();
					wait_for_vsync();
				}
			}
			if (cue != 0)
				a.next();
		} while ((a.running() & 1) && cue != 0x15 && a.rounds < 2);
	} else {
		signed short idx = 0;

		g_anim3_lines.e[0].text = texts[3];
		g_anim3_lines.e[1].text = texts[4];
		g_anim3_lines.e[2].text = texts[5];
		g_anim3_lines.e[3].text = texts[6];
		g_anim3_lines.e[4].text = texts[7];
		g_anim3_lines.e[5].text = texts[8];
		g_anim3_lines.e[6].text = texts[9];
		g_anim3_lines.e[7].text = texts[10];
		g_anim3_lines.e[8].text = texts[11];
		g_anim3_lines.e[9].text = texts[12];
		g_anim3_lines.e[10].text = texts[13];
		g_anim3_lines.e[11].text = texts[14];
		g_anim3_lines.e[12].text = texts[15];
		g_anim3_lines.e[13].text = texts[16];
		g_anim3_lines.e[14].text = texts[17];
		struct cue_lines16 arr = g_anim3_lines;
		do {
			num = 0;
			snd = arr.e[idx].snd;
			s = arr.e[idx].text;
			idx++;
			if (s != 0 && g_subtitles)
				tscr_print_font(this, s, font, SUB_H, SUB_X, SUB_W);
			if (g_speech_on == 1 && speech_active(g_snd_speech) && snd != 0) {
				snd_speech(g_snd_speech, snd);
				while (snd_speech_busy(g_snd_speech)) {
					game_idle();
					wait_for_vsync();
				}
			}
		} while (snd != 0);
	}
	if (fade == 2)
		pal_crossfade(pal_pic, pal_saved, 0xf0);
#if FIX_0125
	/* Fix #125: nur ein angelegtes Bild loeschen */
	if ((ani == 0x36e || ani == 0x244) && b != 0)
		delete b;
	mem_free(pal_saved);
#else
	if (ani == 0x36e || ani == 0x244)
		delete b;
	mem_free(pal_saved);
#endif
	mem_free(pal_pic);
}

void Anim3::blank(unsigned char far *pal)
{
	signed short dummy;
	Bmp far *b;

	tscr_print_font(this, " ", font, SUB_H, SUB_X, SUB_W);
	poll_input();
	b = new Bmp(VIEW_W, VIEW_H);
	nvfile_load(g_nvf_heads, 0x330, b->data(), 0, 0, 0, &dummy, &dummy, 0x140);
	set_palette(pal, 0, 0x100);
	b->put(g_vga_buf, VIEW_X, VIEW_Y);
	delete b;
}

void Anim3::run()
{
	signed short old;
	signed short w;
	signed short h;
	signed short dummy;
	Pic far *pic;
	unsigned char far *buf;
	Bmp far *b;
	Bmp far *b2;
	Anim far *an;
	unsigned char far *pal_dark;
	unsigned char far *pal_black;
	unsigned char far *pal_pic;
	unsigned char far *pal_work;
	unsigned char far *pal_saved;
	register signed short y;
	register signed short voice = 0;

	mouse_hide(0);
	old = g_clock_update;
	g_clock_update = 0;
	g_dialog_depth++;
	g_dream_seen = 1;
	y = 0x15;
	pal_dark = (unsigned char far *)mem_alloc(PAL_SIZE);
	pal_saved = (unsigned char far *)mem_alloc(PAL_SIZE);
	pal_work = (unsigned char far *)mem_alloc(PAL_SIZE);
	pal_pic = (unsigned char far *)mem_alloc(PAL_SIZE);
	pal_black = (unsigned char far *)mem_alloc(PAL_SIZE);
	memcpy(pal_saved, g_palette, PAL_SIZE);
	arc_mark(g_file_3);
	snd_play(g_snd_music, 0x1b);
	buf = (unsigned char far *)mem_alloc(0xfa00L);
	pic = new Pic;
	pic->load(0x364, buf);
	memcpy(pal_pic, pic->get_palette(), PAL_SIZE);
	set_palette(pal_pic, 0, 0x100);
	delete pic;
	memset(pal_black, 0, PAL_SIZE);
	set_palette(pal_black, 0, 0xf0);
	memcpy(pal_dark, g_palette, PAL_SIZE);
	b = new Bmp(0x140, 0xc8, buf);
	b->put(g_vga_buf, 0, 0);
	mem_free(buf);
	delete b;
	play(0, 0x339, 0, 1, 0x38, y, 3);
	blank(pal_dark);
	play(0, 0x242, 0, 2, 0x38, y, 0);
	blank(pal_dark);
	play(0, 0x243, 0, 3, 0xbd, y + 0x67, 0);
	play(0, 0x243, 0, 3, 0x9f, y + 0x17, 1);
	blank(pal_dark);
	play(1, 0x244, 0, 8, 0x38, y, 0);
	blank(pal_dark);
	play(2, 0x245, 3, 4, 0x4c, y + 0x14, 0);
	blank(pal_dark);
	if (g_speech_on == 1 && speech_active(g_snd_speech))
		voice = 1;
	if (voice)
		play(3, 0x338, 3, 5, 0x76, y + 0x2d, 0);
	else {
		play(3, 0x338, 3, 5, 0x76, y + 0x2d, 0);
		play(4, 0x247, 0, 5, 0x38, y - 3, 0);
		play(5, 0x246, 3, 5, 0x76, y + 0x2d, 0);
		play(6, 0x338, 3, 5, 0x76, y + 0x2d, 0);
		play(7, 0x338, 3, 5, 0x76, y + 0x2d, 0);
	}
	tscr_print_font(this, " ", font, SUB_H, SUB_X, SUB_W);
	poll_input();
	b = new Bmp(VIEW_W, VIEW_H);
	nvfile_load(g_nvf_heads, 0x330, b->data(), 0, 0, 0, &dummy, &dummy, 0x140);
	play(9, 0x36e, 0, 8, 0x38, y, 0);
	set_palette(pal_dark, 0, 0x100);
	poll_input();
	memcpy(pal_black, g_palette, PAL_SIZE);
	an = new Anim(0x36f, 0, 0);
	an->loop = 0;
	an->set_dst(g_vga_buf);
	an->size(&w, &h);
	b2 = new Bmp(VIEW_W, VIEW_H);
	b2->clear();
	nvfile_load(g_nvf_heads, 0x330, b2->data(), 0, 7, 0, &dummy, &dummy, 0x140);
	an->setbg(b2);
	b->put(g_vga_buf, VIEW_X, VIEW_Y);
	an->pal_set(1, 0xef);
	an->start(0x38, y, 0);
	do
		an->wait_cue();
	while (an->running() & 1);
	get_palette(pal_work, 0, 0xf0);
	delete b2;
	pal_crossfade(pal_work, pal_black, 0xf0);
	delete an;
	delete b;
	g_refresh = 0;
	clear_screen_buf(g_vga_buf);
	arc_release(g_file_3);
	set_palette(pal_saved, 0, 0x100);
	mem_free(pal_saved);
	mem_free(pal_work);
	mem_free(pal_dark);
	mem_free(pal_black);
	mem_free(pal_pic);
	g_anim_running = 0;
	mouse_show(0);
	snd_obj_stop(g_snd_music);
	g_dialog_depth--;
	g_clock_update = old;
	diary_add(g_diary, texts[19], 0);
}
