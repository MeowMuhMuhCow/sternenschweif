/* SEG045 - Bildaufbau-Synchronisation (VGA-Strahlruecklauf) und Linien/Rahmen im Bildschirmpuffer, geclippt
 * an g_clip_rect (seg045 in SCHWEIF.EXE)
 * BCC -mlarge -3 -O -Oe -Ot -c -Y SEG045.C   -> 470/470 Bytes GLEICH
 *   (Namensfassung mit schweif.h: 19.10.2026 GLEICH)
 * - wait_for_vsync: Bit 5 im CRTC-Register 11h loeschen, dann warten, bis kein Ruecklauf mehr laeuft, und
 *   danach auf den Beginn des naechsten (Status 3DAh: Ruecklauf und Anzeige aus).
 * - Ziel ist ein Puffer mit SCREEN_W Bytes je Zeile (g_vga_buf bzw. ein Bildpuffer gleicher Breite); die
 *   Farbe wird als short uebergeben, R34 fill_line/fill_v_line nehmen das Low-Byte.
 * - Zeiger wie in s_proto.h als void far *; das Weiterschalten "(unsigned char far *)dst += ..." nutzt die
 *   Borland-Erweiterung "Cast als lvalue" ("dst = (unsigned char far *)dst + ..." ergibt anderen Code).
 */
#include <dos.h>
#include "schweif.h"

void wait_for_vsync(void)
{
	signed short val;

	outportb(VGA_CRTC_INDEX, CRTC_VRETRACE_END);
	val = inportb(VGA_CRTC_DATA);
	val &= ~CRTC_VRE_BIT5;
	outportb(VGA_CRTC_INDEX, CRTC_VRETRACE_END);
	outportb(VGA_CRTC_DATA, val);

	do {
		val = inportb(VGA_INPUT_STATUS);
	} while ((val & VGA_ST_VRETRACE) || (val & VGA_ST_NO_DISPLAY));

	outportb(VGA_CRTC_INDEX, CRTC_VRETRACE_END);
	val = inportb(VGA_CRTC_DATA);
	val &= ~CRTC_VRE_BIT5;
	outportb(VGA_CRTC_INDEX, CRTC_VRETRACE_END);
	outportb(VGA_CRTC_DATA, val);

	do {
		val = inportb(VGA_INPUT_STATUS);
	} while (!(val & VGA_ST_VRETRACE) || !(val & VGA_ST_NO_DISPLAY));
}

/* n Bildwechsel abwarten (n >= 1) */
void wait_for_vsyncs(signed short n)
{
	do {
		wait_for_vsync();
	} while (--n);
}

/* Rahmen w x h mit der linken oberen Ecke (x, y) */
void do_border(void far *dst, signed short x, signed short y, signed short w, signed short h, signed short color)
{
	do_h_line(dst, x, y, w, color);
	do_v_line(dst, x, y, h, color);
	do_v_line(dst, x + w - 1, y, h, color);
	do_h_line(dst, x, y + h - 1, w, color);
}

void do_h_line(void far *dst, signed short x, signed short y, signed short len, signed short color)
{
	signed short end;

	if (x > g_clip_rect.x1) return;
	if (y < g_clip_rect.y0) return;
	if (y > g_clip_rect.y1) return;

	if (x < g_clip_rect.x0) {
		len = x + len - g_clip_rect.x0;
		x = g_clip_rect.x0;
	}

	if ((end = x + len) - 1 > g_clip_rect.x1) {
		len -= end - g_clip_rect.x1 - 1;
	}

	if (len > 0) {
		(unsigned char far *)dst += y * SCREEN_W + x;
		fill_line(dst, len, color);
	}
}

void do_v_line(void far *dst, signed short x, signed short y, signed short len, signed short color)
{
	signed short end;

	if (y > g_clip_rect.y1) return;
	if (x < g_clip_rect.x0) return;
	if (x > g_clip_rect.x1) return;

	if (y < g_clip_rect.y0) {
		len = y + len - g_clip_rect.y0;
		y = g_clip_rect.y0;
	}

	if ((end = y + len) - 1 > g_clip_rect.y1) {
		len -= end - g_clip_rect.y1 - 1;
	}

	if (len > 0) {
		(unsigned char far *)dst += y * SCREEN_W + x;
		fill_v_line(dst, len, color, SCREEN_W);
	}
}
