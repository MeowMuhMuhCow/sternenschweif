/* OVL024 - Overlay O024 (Segment 0x1f7d in SCHWEIF.EXE): FLI-Animation abspielen - Zeitgeber-Rueckruf
 * fli_timer_tick zaehlt Takte (g_fli_ticks, Far-Variable ausserhalb DGROUP), Abspielschleife fli_play mit
 * Abbruch-Rueckruf (fli_cb3: Bild, Bildzahl, Durchlauf), Abbruch per Taste (fli_cb_key: ESC, Strg-Q, rechte
 * Maustaste, sonst nach dem letzten Bild) bzw. Bildzahl (fli_cb_frames); fli_play_file beendet bei Strg-Q das
 * Programm (sys_exit, exit).
 * fli_play(h, speed, cb, show): Datei h aus g_file_cd oeffnen (fli_open), Zeitgeber mit 70 Hz, Rahmen fortlaufend
 *   nach g_fli_screen (fli_read_frame; show = Palette direkt setzen und auf den Strahlruecklauf warten), Takt je
 *   Bild speed bzw. aus dem Dateikopf (speed < 0), endlos wiederholt, bis cb != 0; Klangdienste nachholen.
 *
 * BCC -mlarge -3 -Ot -Og -c -Y OVL024.C   -> 722/722 Bytes GLEICH (18.10.2026)
 * - -Og (oder -Ov) ist noetig, damit am Ende von if (..) return 0; else return 1; das Sprungziel
 *   "Funktionsende" als Epilog kopiert wird (pop bp/retf doppelt statt jmp short); sonst ist alles gleich.
 * - g_far_data: extern unsigned long far -> mov ax,seg / mov es,ax / es:[0].
 * - Registervariablen: ret zuerst deklariert (SI), Bildzaehler n danach (DI).
 * - ret = fli_read_frame(..); if (ret == 0) als zwei Anweisungen (or si,si statt or ax,ax).
 * - Rueckrufe fli_cb_key/fli_cb_frames werden als far-Zeiger uebergeben (im Original Stub-Einsprung 0x34/0x39);
 *   der Prototyp von fli_play (s_proto.h) nennt den Rueckruf mit zwei Parametern (Sicht der Aufrufer O002/O013),
 *   gerufen wird er ueber fli_cb3 mit dreien (gleicher Code).
 *   (Namensfassung mit schweif.h, Welle N19: 19.10.2026 GLEICH, cobjfix LINKGLEICH)
 */
/* Phase B: BCC (C-Modus) legt das BSS in umgekehrter Reihenfolge der ersten Deklaration an - Reihenfolge hier vor schweif.h */
extern unsigned short g_fli_max_frames;
extern signed short g_fli_key;
extern signed short g_fli_error;
#include <stdio.h>
#include <bios.h>
#include <stdlib.h>
#include "schweif.h"

/* Phase B: eigenes BSS (ds:8b88..8b8e) in Adressreihenfolge (Reihenfolge festgelegt durch die extern-Zeilen vor den #include) */
signed short g_fli_error;
signed short g_fli_key;
unsigned short g_fli_max_frames;

void fli_timer_tick(void)
{
	g_fli_ticks++;
}

signed short fli_play(signed short h, signed short speed, fli_cb cb, signed short show)
{
	signed short timer;
	signed long pos;
	unsigned short loops;
	unsigned short i;
	unsigned long now;
	unsigned long t0;
	unsigned long tnext;
	struct fli_header hdr;
	register signed short ret;
	register unsigned short n;

	pos = 0;
	n = 0;
	loops = 0;
	if (fli_open(h, &hdr) < 0) {
		return -1;
	}
	timer = AIL_register_timer(fli_timer_tick);
	g_fli_ticks = 0;
	AIL_set_timer_frequency(timer, 70L);
	AIL_start_timer(timer);
	if (((fli_cb3)cb)(n, hdr.frames, loops)) {
		ret = 0;
	} else {
		ret = fli_read_frame(&g_fli_screen, show);
		if (ret == 0) {
			pos = file_seek(g_file_cd, 0L, SEEK_CUR);
		} else {
			goto done;
		}
		t0 = g_fli_ticks;
		if (speed < 0) {
			speed = hdr.speed;
		}
		for (;;) {
			file_seek(g_file_cd, pos, SEEK_SET);
			for (i = 0; i < hdr.frames; i++) {
				n++;
				tnext = t0 + speed;
				for (;;) {
					if (((fli_cb3)cb)(n, hdr.frames, loops)) {
						ret = 0;
						goto done;
					}
					now = g_fli_ticks;
					if (now >= tnext) {
						t0 = now;
						break;
					}
				}
				ret = fli_read_frame(&g_fli_screen, show);
				if (ret != 0) {
					goto done;
				}
				if (g_snd_service_due == 1) {
					snd_service(g_snd_music);
					snd_speech_service(g_snd_speech);
					snd_service(g_snd_fx);
					g_snd_service_due = 0;
				}
			}
			loops++;
		}
	}
done:
	AIL_stop_timer(timer);
	AIL_release_timer_handle(timer);
	file_close(g_file_cd);
	return ret;
}

/* Abbruch per ESC, Strg-Q oder rechter Maustaste, sonst nach dem letzten Bild */
signed short fli_cb_key(unsigned short n, unsigned short frames, unsigned short loops)
{
	if (bioskey(1)) {
		g_fli_key = bioskey(0);
	}
	if (g_fli_key == KEY_ESC || g_fli_key == KEY_CTRL_Q) {
		return 1;
	}
	if (g_mouse_rclick) {
		g_mouse_rclick = 0;
		g_fli_key = KEY_ESC;
		return 1;
	}
	if (n < frames) {
		return 0;
	} else {
		return 1;
	}
}

/* Abbruch nach g_fli_max_frames Bildern bzw. nach dem letzten Bild */
signed short fli_cb_frames(unsigned short n, unsigned short frames, unsigned short loops)
{
	if (n >= g_fli_max_frames) {
		return 1;
	}
	if (n < frames) {
		return 0;
	} else {
		return 1;
	}
}

/* einmal abspielen (Abbruch per Taste), danach ggf. Bild loeschen; Strg-Q beendet das Programm; liefert den
 * Fehler bzw. die Taste */
signed short fli_play_file(signed short h, signed short clear)
{
	register signed short ret;

	g_fli_key = 0;
	ret = fli_play(h, -1, (fli_cb)fli_cb_key, 1);
	if (clear) {
		fli_clear_screen();
	}
	if (g_fli_key == KEY_CTRL_Q) {
		sys_exit(0);
		exit(0);
	}
	return ret ? ret : g_fli_key;
}

/* die ersten max Bilder abspielen */
signed short fli_play_frames(signed short h, unsigned short max, signed short show)
{
	g_fli_max_frames = max;
	return fli_play(h, -1, (fli_cb)fli_cb_frames, show);
}
