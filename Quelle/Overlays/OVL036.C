/* OVL036 - Overlay O036: Spielende-Sequenz, FLI-Fassung - Skriptliste g_finale_script (6-Byte-Schritte
 * struct finale_step {time, op, arg}) gegen einen Zeitgeber-Zaehler abspielen (Far-Variable g_finale_ticks
 * ausserhalb DGROUP, finale_timer_tick als AIL-Timer-Rueckruf, 10 Hz): finale_play_script(f) Hauptschleife
 * (FINALE_OP_FLI = FLI-Animation ueber fli_play mit Rueckruf finale_fli_cb, FINALE_OP_SPEECH = snd_speech,
 * FINALE_OP_TEXT = Text f->txt[arg] zeigen, FINALE_OP_STOP = snd_obj_stop, FINALE_OP_END = Ende; Alt-Q ->
 * Programmende), finale_fli_cb(n, frames) dasselbe als Rueckruf waehrend einer Animation, finale_show_text()
 * setzt je nach Textnummer g_finale_text_nr einen Rahmen (font_set_box) und gibt den Text aus. Aus Finale::run
 * (O035) bei g_speech_ext_avail.
 *
 * BCC -mlarge -3 -Ot -c -Y OVL036.C   -> 930/930 Bytes GLEICH (18.10.2026), cobjfix laeuft (mit -Og ebenso)
 *   (Namensfassung mit schweif.h, Welle N20: 19.10.2026 GLEICH, LINKGLEICH)
 * Schluessel: extern unsigned short far g_finale_ticks -> mov ax,seg / mov es,ax.
 *   switch-Faelle in Quellreihenfolge: finale_fli_cb "case 0: case 1:" zuerst, finale_play_script "case 0" zuletzt.
 *   "quit = done = 1" (mov ax,1 / mov si,ax / mov [bp-4],ax); Rueckrufe als far-Funktionszeiger (Stub 0x2a/0x2f).
 *   g_finale_script (eigener Name ohne Public) ordnet cobjfix als _s_3dbc+0xa zu (Inline-Wert 0x0a).
 */
/* Phase B: BCC (C-Modus) legt das BSS in umgekehrter Reihenfolge der ersten Deklaration an - Reihenfolge hier vor schweif.h */
extern char far * far *g_finale_texts;
extern struct finale_step far *g_finale_step;
extern char far *g_finale_text;
extern signed short g_finale_text_nr;
extern signed short g_finale_key;
#include <bios.h>
#include <stdlib.h>
#include "schweif.h"

/* Phase B: eigene Daten des Moduls (ds:3dc6..3e6e, dsana) - initialisierte Daten in Adressreihenfolge */
struct finale_step g_finale_script[28] = {	/* {Zeit in Takten zu 1/10 s, Befehl, Argument} */
	{    0, FINALE_OP_FLI,     19 },	/*   0.0 s: FLI-Animation 19 */
	{    0, FINALE_OP_SPEECH, 1045 },	/*   0.0 s: Sprachausgabe 1045 */
	{    5, FINALE_OP_TEXT,     0 },	/*   0.5 s: Text txt[0] */
	{  140, FINALE_OP_SPEECH, 1049 },	/*  14.0 s: Sprachausgabe 1049 */
	{  250, FINALE_OP_FLI,     20 },	/*  25.0 s: FLI-Animation 20 */
	{  250, FINALE_OP_SPEECH, 1046 },	/*  25.0 s: Sprachausgabe 1046 */
	{  255, FINALE_OP_TEXT,     1 },	/*  25.5 s: Text txt[1] */
	{  460, FINALE_OP_SPEECH, 1051 },	/*  46.0 s: Sprachausgabe 1051 */
	{  520, FINALE_OP_FLI,     21 },	/*  52.0 s: FLI-Animation 21 */
	{  520, FINALE_OP_SPEECH, 1047 },	/*  52.0 s: Sprachausgabe 1047 */
	{  525, FINALE_OP_TEXT,     2 },	/*  52.5 s: Text txt[2] */
	{  700, FINALE_OP_FLI,     22 },	/*  70.0 s: FLI-Animation 22 */
	{  705, FINALE_OP_TEXT,     3 },	/*  70.5 s: Text txt[3] */
	{  880, FINALE_OP_FLI,     23 },	/*  88.0 s: FLI-Animation 23 */
	{  885, FINALE_OP_TEXT,     4 },	/*  88.5 s: Text txt[4] */
	{ 1000, FINALE_OP_FLI,     24 },	/* 100.0 s: FLI-Animation 24 */
	{ 1005, FINALE_OP_TEXT,     5 },	/* 100.5 s: Text txt[5] */
	{ 1050, FINALE_OP_SPEECH, 1050 },	/* 105.0 s: Sprachausgabe 1050 */
	{ 1260, FINALE_OP_FLI,     25 },	/* 126.0 s: FLI-Animation 25 */
	{ 1260, FINALE_OP_SPEECH, 1048 },	/* 126.0 s: Sprachausgabe 1048 */
	{ 1265, FINALE_OP_TEXT,     6 },	/* 126.5 s: Text txt[6] */
	{ 1495, FINALE_OP_FLI,     26 },	/* 149.5 s: FLI-Animation 26 */
	{ 1500, FINALE_OP_TEXT,     7 },	/* 150.0 s: Text txt[7] */
	{ 1600, FINALE_OP_SPEECH, 1052 },	/* 160.0 s: Sprachausgabe 1052 */
	{ 1765, FINALE_OP_FLI,     27 },	/* 176.5 s: FLI-Animation 27 */
	{ 1770, FINALE_OP_TEXT,     8 },	/* 177.0 s: Text txt[8] */
	{ 1850, FINALE_OP_STOP,     0 },	/* 185.0 s: Musik aus */
	{ 1900, FINALE_OP_END,     -1 }	/* 190.0 s: Ende */
};
/* Phase B: eigenes BSS (ds:c794..c7a4) in Adressreihenfolge (Reihenfolge festgelegt durch die extern-Zeilen vor den #include) */
signed short g_finale_key;
signed short g_finale_text_nr;
char far *g_finale_text;
struct finale_step far *g_finale_step;
char far * far *g_finale_texts;

#define KEY_ALT_Q	0x1011

void finale_show_text(void);

void finale_timer_tick(void)
{
	g_finale_ticks++;
}

signed short finale_fli_cb(unsigned short n, unsigned short frames)
{
	register signed short ret = 0;

	if (bioskey(1)) {
		g_finale_key = bioskey(0);
		if (g_finale_key == KEY_ALT_Q) ret = 1;
	}
	if (g_finale_ticks >= g_finale_step->time) {
		switch (g_finale_step->op) {
		case FINALE_OP_END:
		case FINALE_OP_FLI:
			ret = 1;
			break;
		case FINALE_OP_SPEECH:
			if (g_cd_speech) snd_speech(g_snd_speech, g_finale_step->arg);
			g_finale_step++;
			break;
		case FINALE_OP_TEXT:
			if (g_subtitles) {
				g_finale_text_nr = g_finale_step->arg;
				g_finale_text = g_finale_texts[g_finale_text_nr];
				finale_show_text();
			}
			g_finale_step++;
			break;
		case FINALE_OP_STOP:
			snd_obj_stop(g_snd_music);
			g_finale_step++;
			break;
		}
	}
	if (n >= frames) ret = 1;
	return ret;
}

void finale_play_script(struct finale far *f)
{
	register signed short done = 0;
	register signed short fli;
	signed short timer;
	signed short quit = 0;

	timer = AIL_register_timer(finale_timer_tick);
	g_finale_ticks = 0;
	AIL_set_timer_frequency(timer, 10L);
	AIL_start_timer(timer);
	g_finale_step = g_finale_script;
	g_finale_texts = f->txt;
	do {
		if (g_finale_ticks >= g_finale_step->time) {
			switch (g_finale_step->op) {
			case FINALE_OP_FLI:
				fli = g_finale_step->arg;
				g_finale_step++;
				fli_clear_screen();
				fli_play(fli, -1, finale_fli_cb, 1);
				if (g_finale_key == KEY_ALT_Q) quit = done = 1;
				break;
			case FINALE_OP_SPEECH:
				if (g_cd_speech) snd_speech(g_snd_speech, g_finale_step->arg);
				g_finale_step++;
				break;
			case FINALE_OP_TEXT:
				if (g_subtitles) {
					g_finale_text_nr = g_finale_step->arg;
					g_finale_text = f->txt[g_finale_text_nr];
					finale_show_text();
				}
				g_finale_step++;
				break;
			case FINALE_OP_STOP:
				snd_obj_stop(g_snd_music);
				g_finale_step++;
				break;
			case FINALE_OP_END:
				done = 1;
				break;
			}
		}
	} while (done != 1);
	AIL_stop_timer(timer);
	AIL_release_timer_handle(timer);
	if (quit == 1) {
		sys_exit(0);
		exit(0);
	}
	Sound_volume2(g_snd_music, g_vol_music, 0);
}

void finale_show_text(void)
{
	switch (g_finale_text_nr) {
	case 0: font_set_box(g_font, 0, 160, 5, 1); break;
	case 1: font_set_box(g_font, 5, 310, 15, 1); break;
	case 2: font_set_box(g_font, 100, 210, 5, 1); break;
	case 3: font_set_box(g_font, 5, 310, 5, 1); break;
	case 4: font_set_box(g_font, 165, 155, 5, 1); break;
	case 5: font_set_box(g_font, 5, 200, 5, 1); break;
	case 6: font_set_box(g_font, 5, 310, 5, 1); break;
	case 7: font_set_box(g_font, 170, 140, 7, 1); break;
	case 8: font_set_box(g_font, 95, 140, 5, 1); break;
	}
	font_set_colors(g_font, 255, -1);
	font_print_wrapped(g_font, g_finale_text);
}
