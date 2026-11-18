/* OVL028 - Overlay O028 (Segment 0x1f93 in SCHWEIF.EXE): Vorspann, FLI-Fassung (opening_play_fli, aus
 * Opening::run O027 bei g_speech_ext_avail): FLI-Sequenzen 2..11 nacheinander (fli_play_frames/
 * fli_play_file), dazwischen Paletten ein-/ausblenden (O034 pal_blend_*), Wellen-Szene opening_scene3 und
 * Einblenden opening_fade_in aus O027; Abbruch ueber op->done (struct opening, s_class.h).
 *
 * BCC -mlarge -3 -Ot -c -Y OVL028.C   -> 773/773 Bytes GLEICH (18.10.2026; -O und -Og ebenso, -Z nicht)
 *   (Namensfassung mit schweif.h, Welle N20: 19.10.2026 GLEICH, LINKGLEICH)
 * - Farbe als 3-Byte-struct srgb by value (Kopie ax+al, Uebergabe per SPUSH@ mit cx=4).
 * - opening_fli_part: Parameter n als register (mov si,[bp+0Ah] am Anfang), FLI-Bildschirmzeiger g_fli_vga
 *   voruebergehend auf den eigenen Puffer umgebogen (32-Bit-Kopien mit -3).
 * - opening_fli_part steht hinter opening_play_fli -> far call (im Original TLINK-nop/push cs/call near).
 */
#include "schweif.h"

/* Phase B: eigene Daten des Moduls (ds:1868..186c, dsana) - initialisierte Daten in Adressreihenfolge */
struct srgb g_opening_white = { 63, 63, 63 };

#define FLI_FIRST	2		/* FLI-Sequenzen des Vorspanns: 2..11 */
#define PAL_STEPS	0x19		/* Schritte je Palettenblende */

void opening_play_fli(struct opening far *op)
{
	struct srgb white;
	unsigned char far *buf;

	white = g_opening_white;
	clear_screen_buf(g_vga_buf);
	set_palette(g_pal_dark, 0, 0x100);
	if (op->done == 0) {
		fli_play_frames(FLI_FIRST, 1, 0);
		pal_blend_from_dark(g_fli_pal, PAL_STEPS);
		if (fli_play_file(FLI_FIRST, 0)) {
			op->done = 1;
		} else {
			pal_blend_to_color(g_fli_pal, white, PAL_STEPS);
		}
	}
	if (op->done == 0) {
		fli_play_frames(3, 1, 0);
		pal_blend_from_color(white, g_fli_pal, PAL_STEPS);
		if (fli_play_file(3, 0)) {
			op->done = 1;
		} else {
			pal_blend(g_fli_pal, PAL_STEPS);
		}
	}
	if (op->done == 0) {
		clear_screen_buf(g_vga_buf);
		opening_scene3(op);
		pal_blend((struct srgb far *)0, PAL_STEPS);
	}
	if (op->done == 0) {
		if (fli_play_file(4, 1)) {
			op->done = 1;
		}
	}
	if (op->done == 0) {
		set_palette(g_pal_dark, 0, 0x100);
		fli_play_frames(5, 1, 0);
		pal_blend2((struct srgb far *)0, g_fli_pal, PAL_STEPS, 0x100, 1);
		if (fli_play_file(5, 0)) {
			op->done = 1;
		}
	}
	if (op->done == 0) {
		if (fli_play_file(6, 0)) {
			op->done = 1;
		}
	}
	buf = mem_alloc_w(0xfa00);
	if (op->done == 0) {
		opening_fli_part(op, 7, buf);
	}
	if (op->done == 0) {
		opening_fli_part(op, 8, buf);
	}
	if (op->done == 0) {
		opening_fli_part(op, 9, buf);
	}
	if (op->done == 0) {
		opening_fli_part(op, 10, buf);
	}
	if (op->done == 0) {
		opening_fli_part(op, 11, buf);
	}
	if (op->done == 0) {
		pal_blend(g_fli_pal, PAL_STEPS);
	}
	mem_free_v(buf);
}

void opening_fli_part(struct opening far *op, register signed short n, unsigned char far *buf)
{
	unsigned char far *save;

	save = g_fli_vga;
	g_fli_vga = buf;
	fli_play_frames(n, 1, 0);
	g_fli_vga = save;
	opening_fade_in(op, buf);
	if (fli_play_file(n, 0)) {
		op->done = 1;
	}
}
