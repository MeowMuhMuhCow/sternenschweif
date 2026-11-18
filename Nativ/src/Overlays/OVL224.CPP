/* OVL224 - Overlay O224: Bildschirmschoner (struct saver, 6 Bytes, globales Objekt g_saver ds:cb2e; frueher Klasse
 * Tm). saver::saver (busy = 0), saver::tick (Stufe g_saver_level -> idle_limit = 10/5/2 * 1080; g_idle_ticks ueber der
 * Grenze -> g_saver_due, alle Helden kampfunfaehig -> screensaver_reset), saver::set_level(v) (0..3, alte zurueck),
 * saver::anim (zwei 32x20-Kacheln ueber gb_*, zufaellige Kacheln tauschen bis saver::wait(9) meldet Ende, memcpy 640
 * Bytes), saver::wait(n) (n x poll, solange g_saver_due), saver::poll (Tastatur: Ctrl-Q beendet das Spiel, Maus,
 * Klangdienste). Konstruktor saver::saver (busy = 0) fuer das globale Objekt g_saver.
 *
 * BCC -mlarge -3 -Ot -Og -Oi -c -Y OVL224.CPP   -> 827/827 Bytes GLEICH (18.10.2026), cobjfix laeuft
 *   (Namensfassung mit schweif.h: 19.10.2026 GLEICH, LINKGLEICH; keine Referenz-OBJ: gegen die csrc-Fassung und
 *   lnk/OVL224.OBJ geprueft)
 * Phase B (19.10.2026): echtes globales Objekt "struct saver g_saver" (BSS ds:cb2e) mit Konstruktor saver::saver().
 *   BCC erzeugt den statischen Initialisierer selbst (O224_032B, lokales Public @_STCON_$qv) samt _INIT_-Eintrag
 *   (fern, Prioritaet 32) - genau der Eintrag des Originals (c_module.tsv Spalte DI). Der Code ist unveraendert
 *   827/827 GLEICH. Vorher: Konstruktor als Funktion saver_ctor(s) nachgebildet, saver_init als normale Funktion,
 *   _INIT_-Eintrag im ASM-Datenstueck. cobjfix_d laesst das ASM-Public F_O224_032B weg (sonst zweiter Einsprung
 *   im Overlay-Stub neben dem lokalen Public).
 *   tick(): "if (g_saver_level != 0) { ... }" (jnz + jmp near), -Og kopiert den Epilog ans Ende des then-Zweigs.
 *   memcpy(gb_data(tile), gb_data(tile2), 640) mit -Oi -> rep movsw; "if (wait(9)) { done = 1; break; }".
 */
#include <bios.h>
#include <stdlib.h>
#include <mem.h>
#include "schweif.h"

/* Phase B: eigenes BSS (ds:cb2e..cb34) - das globale Objekt; BCC erzeugt dazu den statischen Initialisierer
 * (O224_032B, ruft den Konstruktor) und den _INIT_-Eintrag (Prioritaet 32) */
struct saver g_saver;

saver::saver()
{
	busy = 0;
}

void saver::tick()
{
	if (g_saver_level != 0) {
		if (all_heroes_down(g_group) == 0) {
			switch (g_saver_level) {
			case 1: idle_limit = 10; break;
			case 2: idle_limit = 5; break;
			case 3: idle_limit = 2; break;
			}
			idle_limit *= 1080;
			if (g_idle_ticks > idle_limit) {
				g_idle_ticks = 0;
				g_saver_due = 1;
			}
		} else {
			screensaver_reset(this);
		}
	}
}

signed short saver::set_level(signed char v)
{
	signed short old = g_saver_level;

	g_saver_level = v;
	if (g_saver_level < 0) g_saver_level = 0;
	if (g_saver_level > 3) g_saver_level = 3;
	return old;
}

void saver::anim()
{
	signed short x, y, old_x, old_y, done;
	void far *tile;
	void far *tile2;

	old_x = 0;
	old_y = 0;
	done = 0;
	tile = gb_new_alloc(0, 32, 20);
	tile2 = gb_new_alloc(0, 32, 20);
	gb_save(tile, g_vga_buf, 0, 0);
	while (done == 0) {
		do {
			x = random_schweif(10) - 1;
			y = random_schweif(10) - 1;
		} while (x == old_x && y == old_y);
		old_x = x;
		old_y = y;
		gb_save(tile2, g_vga_buf, x * 32, y * 20);
		gb_blit(tile, g_vga_buf, x * 32, y * 20);
		memcpy(gb_data(tile), gb_data(tile2), 640);
		if (wait(9)) {
			done = 1;
			break;
		}
	}
	gb_delete(tile, 3);
	gb_delete(tile2, 3);
}

signed short saver::wait(signed short n)
{
	register signed short i;
	register signed short ret = 0;

	for (i = 0; i < n; i++) {
		if (g_saver_due) {
			poll();
		} else {
			ret = 1;
			break;
		}
	}
	return ret;
}

void saver::poll()
{
	wait_for_vsync();
	if (bioskey(1)) {
		if (bioskey(0) == KEY_CTRL_Q) {
			sys_exit(0);
			exit(0);
		} else {
			screensaver_reset(this);
			clear_kbd();
		}
	}
	if (g_mouse_event) {
		screensaver_reset(this);
		g_mouse_event = 0;
	}
	if (g_snd_service_due == 1) {
		snd_service(g_snd_music);
		snd_speech_service(g_snd_speech);
		snd_service(g_snd_fx);
		g_snd_service_due = 0;
	}
}
