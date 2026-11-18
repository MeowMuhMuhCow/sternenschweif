/* OVL045 - Overlay O045 (seg105 in SCHWEIF.EXE): Huellen zur Spielende-Sequenz - o045_ani_halt haelt die
 * Bildschirm-Animation an (ani_halt(g_screen_pic)), finale_play legt ein lokales Finale (O035, struct finale,
 * 0x22 Bytes) an und spielt es ab (Finale_ctor, Finale_run, Finale_dtor(&f, 2)); dazwischen drei leere Funktionen.
 *
 * BCC -mlarge -3 -Ot -c -Y OVL045.C   -> 81/81 Bytes GLEICH (18.10.2026)
 *   (Namensfassung mit schweif.h, Welle N20: 19.10.2026 GLEICH, LINKGLEICH)
 */
#include "schweif.h"

void o045_ani_halt(void)
{
	ani_halt(g_screen_pic);
}

void o045_stub_1(void)
{
}

void o045_stub_2(void)
{
}

void o045_stub_3(void)
{
}

void finale_play(void)
{
	struct finale f;

	Finale_ctor(&f);
	Finale_run(&f);
	Finale_dtor(&f, 2);
}
