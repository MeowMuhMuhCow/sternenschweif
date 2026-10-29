/* SEG015 - Bild mit Lamellen-Effekt einblenden (seg015 in SCHWEIF.EXE)
 * fade_in_pic zeichnet das neue Bild (Breite w, Hoehe h, zentriert) in 8 Durchgaengen ueber das alte
 * (Zeilenlaenge auf 8 aufgerundet): je 8er-Block wachsen die Pixel aus dem neuen Bild (w_new 0..8) auf
 * Kosten des alten (w_old 8..0); gerade und ungerade Zeilen laufen mit gegenlaeufigem Versatz. Zum
 * Schluss wird das neue Bild ganz geblittet. Zustand im Objekt struct fade_state (12 Bytes).
 * BCC -mlarge -3 -Ot -c -Y SEG015.C   -> 612/612 Bytes GLEICH
 *   (Namensfassung mit schweif.h: 19.10.2026 GLEICH)
 * - Objektparameter wie in s_proto.h als void far *, Zugriff ueber den Cast-Makro FADE (gleicher Code);
 *   das Bildobjekt ist ein lokales Bmp (C-Fassung aus s_class.h).
 */
#include "schweif.h"

#define FADE	((struct fade_state far *)fade)

void fade_in_pic(void far *fade, void far *pic, void far *old, signed short w, signed short h)
{
	unsigned char far *src_new;
	unsigned char far *dst;
	unsigned char far *src_old;
	signed short x0;
	signed short y0;
	signed long offs;
	struct Bmp bmp;
	signed short i;

	gb_new(&bmp, w, h, pic);
	x0 = (SCREEN_W - w) >> 1;
	y0 = (SCREEN_H - h) >> 1;
	mouse_hide(0);
	offs = (signed long)(y0 * SCREEN_W + x0);

	FADE->tick = 0;
	FADE->w_new = 0;
	FADE->w_old = 8;
	FADE->off_even = 0;
	FADE->off_odd = 7;
	FADE->step = 0;

	while (FADE->step < 8) {
		FADE->off_even &= 7;
		if (FADE->off_odd < 0)
			FADE->off_odd = 7;

		if (++FADE->tick == 1) {
			FADE->tick = 0;
			if (++FADE->w_new > 8)
				FADE->w_new = 8;
			if (--FADE->w_old < 0)
				FADE->w_old = 0;
		}

		dst = (unsigned char far *)g_vga_buf + offs;
		src_new = pic;
		src_old = old;

		for (i = 0; i < h - 1; i++) {
			src_new += w;
			src_old += (w + 8) & 0xfff8;
			dst += SCREEN_W;
			fade_line(fade, dst, src_new, src_old, (i & 1) == 0 ? FADE->off_even : FADE->off_odd, w);
		}

		FADE->off_even++;
		FADE->off_odd--;
		FADE->step++;
	}

	gb_blit(&bmp, g_vga_buf, x0, y0);
	mouse_show(0);
	gb_delete(&bmp, 2);
}

/* eine Zeile: offs Pixel neu, dann w_old alt, danach je 8er-Block w_new neu + w_old alt */
void fade_line(void far *fade, unsigned char far *dst, unsigned char far *src_new,
		unsigned char far *src_old, signed short offs, signed short width)
{
	signed short i;
	signed short x;

	for (i = 0; i < offs; i++) {
		*dst = *src_new;
		src_new++;
		dst++;
		src_old++;
	}
	for (i = 0; i < FADE->w_old; i++) {
		*dst = *src_old;
		src_old++;
		dst++;
		src_new++;
	}
	for (x = offs + 1; x < width - 7; x += 8) {
		for (i = 0; i < FADE->w_new; i++) {
			*dst = *src_new;
			src_new++;
			dst++;
			src_old++;
		}
		for (i = 0; i < FADE->w_old; i++) {
			*dst = *src_old;
			src_old++;
			dst++;
			src_new++;
		}
	}
}
