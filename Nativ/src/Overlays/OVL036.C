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
struct finale_step g_finale_script[28] = { 0 };
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
		} else {
			wait_for_vsync();
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
