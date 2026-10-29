/* OVL039 - Overlay O039 (Segment 0x1fdf in SCHWEIF.EXE): Vorspann, Klasse Intro (0x24 Bytes, s_class.h; Basis
 * struct text_screen O181, 0x18 Bytes; O222 legt sie lokal an). pal_crossfade(a, b, n): 64 Ueberblendschritte
 * pal_fade(a, b, n - 1) / wait_for_vsync / set_palette(a, 0, n - 1). Intro(): Musik anhalten, Dateiliste
 * g_cl_intro (bei g_cd_speech zusaetzlich g_cl_intro_speech) in den CD-Cache laden, Texte 0xa9 (0x32 Stueck) nach
 * texts_buf/texts, Musikstueck 0x0e, font = new Font(1) mit Farbsatz 0xe2/0xba/0xff, tscr_open. ~Intro():
 * tscr_close(1), delete font, Musik aus, Texte frei, cache_purge. run() (O039_01D0): Hintergrund 0x347 (Pic per
 * new/delete, Palette einblenden), dann 10 bzw. 11 (g_trial_lost) Szenen (Anim-Objekt je Durchlauf, Tabelle
 * g_intro_scenes {Animation, Bild}), Bild 0x346 per nvfile_load in einen Bmp-Puffer, Ueberblenden
 * (pal_crossfade), Animation abspielen bis Ende oder 24 Marken, dabei je Szene einmal Untertitel texts[20..29]
 * (tscr_print_font, nur bei g_subtitles) bzw. Sprachausgabe 0x3e9..0x3f2 (snd_speech, bei g_cd_speech und
 * g_speech_ext_avail), danach Ausblenden mit leerem Text g_intro_blank; zum Schluss Palette zurueck, Puffer frei.
 *
 * BCC -mlarge -3 -Ot -c -Y OVL039.CPP   -> 2684/2684 Bytes GLEICH (18.10.2026), cobjfix laeuft
 *   (Namensfassung mit schweif.h: 19.10.2026 GLEICH, LINKGLEICH)
 * - C++: Konstruktor (this==0 -> new(0x24), "return this" + toter Epilog), Destruktor (flags in SI, &1 ->
 *   delete), "new Font(1)"/"delete font" -> Font_ctor(0, 1)/Font_dtor(font, 3), "new Pic" -> Pic_ctor(0),
 *   "new Bmp(w, h[, p])" -> gb_new_alloc/gb_new mit this = 0.
 * - Lokale Objekte: bg (Bmp) mitten im Rumpf deklariert (Konstruktor erst nach "count = 10/11"), liegt als
 *   letzte Funktionsebenen-Variable auf [bp-4ah]. Im Schleifenrumpf "struct text_screen tw; Anim a(..);" - tw
 *   (ohne Konstruktor, nur fuer tscr_print_font benutzt) VOR a deklarieren; "Bmp t" (10 Bytes) im
 *   verschachtelten Block liegt direkt unter den inline-Kopien ([bp-58h]), tw (0x18) und a danach
 *   ([bp-70h], [bp-120h]): Objekte ueber 10 Bytes legt BCC ans Ende (siehe OVL011.CPP).
 * - a.set_dst(g_vga_buf) als inline-Setter: Parameterkopie [bp-4eh] (mov eax hin und zurueck).
 * - a.running() (Anim_running) liefert unsigned char -> test al,1 (bei int: test ax,1).
 * - buf = (unsigned char far *)(signed long)mem_alloc(0xfa00L) -> shl/shrd/shld; die anderen Puffer direkt.
 * - mem_free liefert den auf 0 gesetzten Zeiger zurueck: "buf = mem_free(buf); delete buf;" (delete auf NULL).
 * - switch ueber k: Faelle 0..9 ohne 2 (Tabelle zeigt fuer 2 auf default), default als letzter Fall mit break.
 * - Bei k == 11 wird p nicht neu angelegt: es ist das bei k == 10 angelegte (dort nicht freigegebene) Bild.
 */
#include <mem.h>
#include "schweif.h"

/* Phase B: eigene Daten des Moduls (ds:3f5e..3fc0, dsana) - initialisierte Daten in Adressreihenfolge, danach der Literalpool */
char g_cl_intro[30] = {
	0x3a, 0x23, 0x3b, 0x23, 0x3c, 0x23, 0x3d, 0x23, 0x3e, 0x23, 0x3f, 0x23, 0x40, 0x23, 0x41, 0x23,
	0x42, 0x23, 0x43, 0x23, 0x44, 0x23, 0x45, 0x23, 0x46, 0x23, 0x47, 0x23, 0xff, 0xff
};
char g_cl_intro_speech[14] = { 0xe9, 0xc3, 0xee, 0xc3, 0xef, 0xc3, 0xf0, 0xc3, 0xf1, 0xc3, 0xf2, 0xc3, 0xff, 0xff };
char g_cl_intro_st[4] = "";
struct intro_scene g_intro_scenes[12] = {
	{ 826, 0 },
	{ 827, 1 },
	{ 828, 1 },
	{ 829, 9 },
	{ 830, 2 },
	{ 831, 3 },
	{ 832, 4 },
	{ 833, 5 },
	{ 834, 6 },
	{ 835, 7 },
	{ 836, 8 },
	{ 837, 9 }
};

#define INTRO_TXT	0xa9		/* Textliste des Vor-/Abspanns (0x32 Texte) */
#define INTRO_TXT_N	0x32
#define INTRO_MUSIC	0x0e		/* Musikstueck */
#define INTRO_BG	0x347		/* Hintergrundbild */
#define INTRO_PICS	0x346		/* NVF-Datei der Szenenbilder */
#define INTRO_SPEECH	0x3e9		/* Sprachnummer der ersten Szene */
#define INTRO_CUES	0x18		/* Szene endet nach so vielen Marken */
#define PAL_SIZE	0x300

void pal_crossfade(unsigned char far *pal, unsigned char far *dst, register signed short n)
{
	unsigned char i;

	for (i = 0; i < 0x40; i++) {
		pal_fade(pal, dst, n - 1);
		wait_for_vsync();
		set_palette(pal, 0, n - 1);
	}
}

Intro::Intro()
{
	snd_obj_stop(g_snd_music);
	load_pic_list(g_cd_cache, g_cl_intro, g_cl_intro_st, 1, 0);
	if (g_cd_speech)
		load_pic_list(g_cd_cache, g_cl_intro_speech, g_cl_intro_st, 1, 0);
	((Font far *)g_font)->load_list(INTRO_TXT, INTRO_TXT_N, &texts_buf, &texts);
	snd_play(g_snd_music, INTRO_MUSIC);
	font = new Font(1);
	font_push_colset(font, 0xe2, 0xba, 0xff, 0xff, 0xff);
	tscr_open(this);
}

Intro::~Intro()
{
	tscr_close(this, 1);
	delete font;
	snd_obj_stop(g_snd_music);
	Sound_stop_music(g_snd_music);
	mem_free(texts_buf);
	mem_free(texts);
	cache_purge(g_cd_cache, 1);
}

void Intro::run()
{
	char far *s;
	signed short sum;
	signed short cue;
	signed short i;
	signed short cnt;
	signed short w;
	signed short h;
	signed short count;
	signed short dummy;
	signed short x;
	signed short y;
	signed short x0;
	signed short y0;
	Bmp far *b;
	Bmp far *p;
	Pic far *pic;
	unsigned char far *pal_dark;
	unsigned char far *pal_work;
	unsigned char far *pal_target;
	unsigned char far *pal_saved;
	unsigned char far *pal_fadeout;
	unsigned char far *buf;
	register signed short snd;
	register signed short k;

	mouse_hide(0);
	ani_stop(g_screen_pic);
	g_dialog_depth++;
	buf = (unsigned char far *)(signed long)mem_alloc(0xfa00L);
	pal_saved = (unsigned char far *)mem_alloc(PAL_SIZE);
	pal_dark = (unsigned char far *)mem_alloc(PAL_SIZE);
	pal_target = (unsigned char far *)mem_alloc(PAL_SIZE);
	pal_work = (unsigned char far *)mem_alloc(PAL_SIZE);
	pal_fadeout = (unsigned char far *)mem_alloc(PAL_SIZE);
	memcpy(pal_saved, g_palette, PAL_SIZE);
	w = 0x117;
	h = 0xbf;
	x0 = 0;
	y0 = -15;
	x = x0 + 0x39;
	y = y0 + 0x21;
	arc_mark(g_file_3);
	pic = new Pic;
	pic->load(INTRO_BG, buf);
	memcpy(pal_work, pic->get_palette(), PAL_SIZE);
	set_palette(pal_work, 0, 0x100);
	memset(pal_work, 0, PAL_SIZE);
	set_palette(pal_work, 0, 0xf0);
	memcpy(pal_dark, g_palette, PAL_SIZE);
	memcpy(pal_fadeout, g_palette, PAL_SIZE);
	memset(pal_fadeout + 0x2fa, 0, 3);
	b = new Bmp(0x140, 0xc8, buf);
	b->put(g_vga_buf, x0, y0);
	delete b;
	delete pic;
	buf = (unsigned char far *)mem_free(buf);
	delete buf;
	if (g_trial_lost)
		count = 11;
	else
		count = 10;
	Bmp bg(0xd1, 0x88);
	bg.get(g_vga_buf, x, y);
	for (i = 0; i < count; i++) {
		if (i < 9)
			k = i;
		if (i == 9) {
			if (count == 10)
				k = 9;
			else
				k = 10;
		}
		if (i == 10)
			k = 11;
		struct text_screen tw;
		Anim a(g_intro_scenes[k].ani, 0, 0);
		if (k == 0 || k == 4 || k == 6 || k == 7)
			a.loop = 3;
		else
			a.loop = 0;
		if (k == 1 || k == 2 || k == 3 || k == 10 || k == 11)
			a.rate = 3;
		else if (k == 8)
			a.rate = 5;
		else
			a.rate = 4;
		a.set_dst(g_vga_buf);
		a.size(&w, &h);
		if (k != 2 && k != 3 && k != 11) {
			bg.put(g_vga_buf, x, y);
			Bmp t(0xd1, 0x88);
			nvfile_load(g_nvf_heads, INTRO_PICS, t.data(), 0, g_intro_scenes[k].pic, 0, &dummy, &dummy,
				0x140);
			if (k == 1) {
				a.pal_set(1, 0x1d);
				a.pal_set(0x1f, 0xd1);
			} else {
				a.pal_set(1, 0xef);
			}
			memcpy(pal_target, g_palette, PAL_SIZE);
			set_palette(pal_dark, 0, 0x100);
			t.put(g_vga_buf, x, y, 4);
			memcpy(pal_work, pal_dark, PAL_SIZE);
			pal_crossfade(pal_work, pal_target, 0x100);
		}
		if (k == 3) {
			p = new Bmp(0xd1, 0x88);
			nvfile_load(g_nvf_heads, INTRO_PICS, p->data(), 0, 9, 0, &dummy, &dummy, 0x140);
		} else if (k != 11) {
			p = new Bmp(w, h);
			p->get(g_vga_buf, x, y);
		}
		a.setbg(p);
		a.start(x, y, 0);
		sum = 0;
		cnt = k;
		do {
			if (k == cnt) {
				switch (k) {
				case 0:
					s = texts[20];
					snd = INTRO_SPEECH;
					break;
				case 1:
					s = texts[21];
					snd = INTRO_SPEECH + 1;
					s = 0;
					snd = 0;
					break;
				case 3:
					s = texts[22];
					snd = INTRO_SPEECH + 2;
					s = 0;
					snd = 0;
					break;
				case 4:
					s = texts[23];
					snd = INTRO_SPEECH + 3;
					snd = 0;
					break;
				case 5:
					s = texts[24];
					snd = INTRO_SPEECH + 4;
					snd = 0;
					break;
				case 6:
					s = texts[25];
					snd = INTRO_SPEECH + 5;
					break;
				case 7:
					s = texts[26];
					snd = INTRO_SPEECH + 6;
					break;
				case 8:
					if (g_trial_lost) {
						s = texts[27];
						snd = INTRO_SPEECH + 7;
					} else {
						s = texts[28];
						snd = INTRO_SPEECH + 8;
					}
					break;
				case 9:
					s = texts[29];
					snd = INTRO_SPEECH + 9;
					break;
				default:
					s = "";
					break;
				}
				if (s != 0 && k == cnt && g_subtitles) {
					tscr_print_font(&tw, s, font, 0x13, 0x14, 0x118);
					cnt--;
				}
				if (g_cd_speech && snd && g_speech_ext_avail) {
					snd_speech(g_snd_speech, snd);
					snd = 0;
					if (k == cnt && !g_subtitles)
						cnt--;
				}
			}
			a.wait_cue();
			cue = a.cue;
			if (a.rounds && cue)
				sum = cue + a.rounds;
			if (cue)
				a.next();
		} while ((a.running() & 1) && sum < INTRO_CUES);
		if (k != 1 && k != 2 && k != 10) {
			memcpy(pal_work, g_palette, PAL_SIZE);
			s = "";
			tscr_print_font(&tw, s, font, 0x13, 0x14, 0x118);
			pal_crossfade(pal_work, pal_fadeout, 0xf0);
		}
		if (k != 10)
			delete p;
	}
	g_refresh = 0;
	clear_screen_buf(g_vga_buf);
	arc_release(g_file_3);
	g_anim_running = 0;
	set_palette(pal_saved, 0, 0x100);
	mem_free(pal_saved);
	mem_free(pal_target);
	mem_free(pal_work);
	mem_free(pal_dark);
	mem_free(pal_fadeout);
	g_dialog_depth--;
	mouse_show(0);
}
