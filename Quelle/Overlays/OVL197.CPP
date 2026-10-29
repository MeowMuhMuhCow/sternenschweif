/* OVL197 - Overlay O197 (Segment 0x2349 in SCHWEIF.EXE): Zahleneingabe NumIn (0x35 Bytes, s_class.h; lokales Objekt in
 * Win::num_box O205) mit Ziffernfeldern, Pfeilen (auf/ab) und OK-Knopf. NumIn(n, gfx, min, max, val): sieben Bilder
 * (new Bmp 13x16/34x16/13x7) per NvfMem::unpack (R41) aus den Rahmenteilen gfx (Nr. 0x0e..0x14), Stellen = min(n, 5),
 * Startwert in Ziffern zerlegen (min/max/val < 0 -> 0/32000/32000). ~NumIn. value(): Wert aus den Ziffern (long),
 * get_x/get_y/set_x/set_y, draw_frame: Rahmen zeichnen, draw_digits: Ziffern zeichnen (Cursor in Farbe 0xfd ueber
 * font_set_colors/font_restore_colors). run(x, y): Tastenschleife Dlg::num_loop (O190) - Enter/0 pruefen (lo..hi,
 * sonst auf die Grenze setzen), Esc/g_mouse_rclick -> -1, Hoch/Runter Ziffer +-1 (zyklisch), Links/Rechts Cursor,
 * '0'..'9' direkt.
 *
 * BCC -mlarge -3 -Ot -Og -c -Y -E..\bc\BIN\TASM.EXE OVL197.CPP   -> 3364/3364 Bytes GLEICH (18.10.2026),
 *   cobjfix laeuft. Ohne -Og 18 Bytes zu kurz.
 *   (Namensfassung mit schweif.h, Klasse NumIn in s_class.h, NvfMem/Bmp als Klassen: 19.10.2026 GLEICH, LINKGLEICH)
 * - Konstruktor: "register mul = 1" VOR dem lokalen NvfMem-Objekt deklarieren (sonst kommt mov di,1 erst nach dem
 *   Konstruktoraufruf nvf_new: Initialisierungen laufen in Deklarationsreihenfolge).
 * - -Og-Zwischenwert [bp-4] (Konstruktor) bzw. [bp-0ch] (run) = ndig; [bp-0eh] = Offset von &dig[cur].
 * - value(): keine register-Variablen -> mul SI, i CX (mit register-mul kaeme mul nach CX).
 * - "if ((key = DLG->num_loop(this)) == 0)" -> mov [bp-4],ax / or ax,ax.
 * - "x + i * step - 4" mit register i -> push x / imul / pop dx; mit Member cur ("x + cur * step - 4") dagegen
 *   imul zuerst, dann mov dx,x / add dx,ax (gleiche Schreibweise, anderer Code).
 * - Originalfehler/Auffaelliges: Startwert mit mehr Stellen als n ergibt eine erste "Ziffer" > 9 (z.B. Vorgabe
 *   32000 bei val < 0 und n < 5); v = value() nach Hoch/Runter/Ziffer ohne Verwendung.
 */
#include <stdio.h>
#include "schweif.h"

#define MAX_DIGITS	5		/* hoechstens 5 Ziffern */
#define DIGIT_STEP	14		/* Abstand der Ziffernfelder */
#define NUM_MAX		32000		/* Vorgabe fuer max/val < 0 */
#define PIC_FIELD	0x0e		/* Teilbilder der Rahmendatei (Win::frame_gfx): Ziffernfeld 13 x 16 */
#define PIC_UP		0x0f		/* Pfeil auf 13 x 7 */
#define PIC_DOWN	0x10		/* Pfeil ab */
#define PIC_UP_DOWN	0x11		/* Pfeil auf gedrueckt */
#define PIC_DOWN_DOWN	0x12		/* Pfeil ab gedrueckt */
#define PIC_OK		0x13		/* OK-Knopf 34 x 16 */
#define PIC_OK_DOWN	0x14		/* OK-Knopf gedrueckt */
#define CH_ENTER	13		/* ASCII-Ergebnis von Dlg::num_loop */
#define CH_ESC		27
#define COL_CURSOR	0xfd		/* Farbe der Cursorziffer */
#define DLG		((Dlg far *)g_dialog)

NumIn::NumIn(signed short n, void far *gfx, signed short min, signed short max, signed short val)
{
	signed short w;
	register signed short i;
	register signed short mul = 1;
	NvfMem nvf;

	redraw = 0;
	field = new Bmp(13, 16);
	ok = new Bmp(34, 16);
	ok_down = new Bmp(34, 16);
	up = new Bmp(13, 7);
	down = new Bmp(13, 7);
	up_down = new Bmp(13, 7);
	down_down = new Bmp(13, 7);
	nvf.unpack(field->data(), gfx, PIC_FIELD, 0, &w, &w, SCREEN_W);
	nvf.unpack(ok->data(), gfx, PIC_OK, 0, &w, &w, SCREEN_W);
	nvf.unpack(ok_down->data(), gfx, PIC_OK_DOWN, 0, &w, &w, SCREEN_W);
	nvf.unpack(up->data(), gfx, PIC_UP, 0, &w, &w, SCREEN_W);
	nvf.unpack(down->data(), gfx, PIC_DOWN, 0, &w, &w, SCREEN_W);
	nvf.unpack(up_down->data(), gfx, PIC_UP_DOWN, 0, &w, &w, SCREEN_W);
	nvf.unpack(down_down->data(), gfx, PIC_DOWN_DOWN, 0, &w, &w, SCREEN_W);
	if (min < 0)
		min = 0;
	if (max < 0)
		max = NUM_MAX;
	if (val < 0)
		val = NUM_MAX;
	ndig = n > MAX_DIGITS ? MAX_DIGITS : n;
	if (val == 0) {
		for (i = 0; i < ndig; i++)
			dig[i] = 0;
	} else {
#if FIX_0044
		/* Fix #44: Startwert auf die darstellbaren Stellen begrenzen (erste Ziffer sonst > 9) */
		for (i = 0; i < ndig - 1; i++)
			mul *= 10;
		if ((signed long)val >= (signed long)mul * 10)
			val = (signed short)((signed long)mul * 10 - 1);
#else
		for (i = 0; i < ndig - 1; i++)
			mul *= 10;
#endif
		for (i = 0; i < ndig; i++) {
			if (i != ndig - 1) {
				dig[i] = val / mul;
				val %= mul;
				mul /= 10;
			} else
				dig[i] = val;
		}
	}
	lo = min;
	hi = max;
	cur = 0;
	step = DIGIT_STEP;
}

NumIn::~NumIn()
{
	delete field;
	delete ok;
	delete ok_down;
	delete up;
	delete down;
	delete up_down;
	delete down_down;
}

signed long NumIn::value()
{
	signed short mul = 1;
	signed long sum = 0;
	signed short i;

	for (i = ndig - 1; i >= 0; i--) {
		sum += (signed long)dig[i] * mul;
		mul *= 10;
	}
	return sum;
}

signed short NumIn::get_x()
{
	return x;
}

signed short NumIn::get_y()
{
	return y;
}

void NumIn::set_x(signed short v)
{
	x = v;
}

void NumIn::set_y(signed short v)
{
	y = v;
}

void NumIn::draw_frame()
{
	register signed short i;

	mouse_hide(0);
	for (i = 0; i < ndig; i++) {
		up->put(g_vga_buf, x + i * step - 4, y - 12, 4);
		field->put(g_vga_buf, x + i * step - 4, y - 4);
		down->put(g_vga_buf, x + i * step - 4, y + 13, 4);
	}
	ok->put(g_vga_buf, x + i * step + 12, y - 4);
	mouse_show(0);
}

void NumIn::draw_digits()
{
	register signed short i;
	register signed short cx = x;
	char c;
	signed short w;

	mouse_hide(0);
	for (i = 0; i < ndig; i++) {
		if (cur == i || redraw) {
			field->put(g_vga_buf, x + i * step - 4, y - 4);
			if (cur == i)
				font_set_colors(g_font, COL_CURSOR, -1);
		}
		c = dig[i] + '0';
		w = font_char_width(g_font, c);
		font_put_char(g_font, c, cx + (5 - w) / 2, y);
		if (cur == i)
			font_restore_colors(g_font);
		cx += step;
	}
	mouse_show(0);
	redraw = 0;
}

signed short NumIn::run(signed short px, signed short py)
{
	register signed short i;
	register signed short mul;
	signed short ret;
	signed short key;
	signed long v;
	signed short t;

	active = 1;
	x = px;
	y = py;
	ret = 0;
	mouse_hide(0);
	draw_frame();
	draw_digits();
	mouse_show(0);
	if (g_in_fight == 0)
		wait_ticks(7);
	clear_input();
	mouse_clear_left();
	mouse_clear_right();
	g_mouse_event = 0;
	clear_kbd();
	key = 0;
	while (key != CH_ENTER) {
		if ((key = DLG->num_loop(this)) == 0)
			key = CH_ENTER;
		if (key == CH_ENTER) {
			mouse_hide(0);
			ok_down->put(g_vga_buf, x + ndig * step + 12, y - 4);
			mouse_show(0);
			v = value();
			if (hi >= v && lo <= v) {
				ret = v;
			} else if (lo > v) {
				t = lo;
				key = 0;
			} else {
				t = hi;
				key = 0;
			}
			wait_ticks(5);
			mouse_hide(0);
			ok->put(g_vga_buf, x + ndig * step + 12, y - 4);
			mouse_show(0);
			if (key == 0) {
				/* Wert auf die Grenze t setzen und in Ziffern zerlegen */
				mul = 1;
				if (t == 0) {
					for (i = 0; i < ndig; i++)
						dig[i] = 0;
				} else {
					for (i = 0; i < ndig - 1; i++)
						mul *= 10;
					for (i = 0; i < ndig; i++) {
						if (i != ndig - 1) {
							dig[i] = t / mul;
							t %= mul;
							mul /= 10;
						} else
							dig[i] = t;
					}
				}
				redraw = 1;
				draw_digits();
			}
		} else if (key == CH_ESC || g_mouse_rclick) {
			g_mouse_rclick = g_action = 0;
			ret = -1;
			break;
		} else if (g_action == KEY_UP) {
			mouse_hide(0);
			up_down->put(g_vga_buf, x + cur * step - 4, y - 12, 4);
			mouse_show(0);
			dig[cur]++;
			if (dig[cur] == 10)
				dig[cur] = 0;
			v = value();
			wait_ticks(5);
			mouse_hide(0);
			up->put(g_vga_buf, x + cur * step - 4, y - 12, 4);
			mouse_show(0);
			draw_digits();
		} else if (g_action == KEY_DOWN) {
			mouse_hide(0);
			down_down->put(g_vga_buf, x + cur * step - 4, y + 13, 4);
			mouse_show(0);
			dig[cur]--;
			if (dig[cur] == -1)
				dig[cur] = 9;
			v = value();
			wait_ticks(5);
			mouse_hide(0);
			down->put(g_vga_buf, x + cur * step - 4, y + 13, 4);
			mouse_show(0);
			draw_digits();
		} else if (g_action == KEY_RIGHT) {
			cur++;
			if (cur == ndig)
				cur = 0;
			draw_digits();
		} else if (g_action == KEY_LEFT) {
			cur--;
			if (cur == -1)
				cur = ndig - 1;
			draw_digits();
		} else if (key >= '0' && key <= '9') {
			dig[cur] = key - '0';
			v = value();
			draw_digits();
			cur++;
			if (cur == ndig)
				cur = 0;
			draw_digits();
		}
	}
	active = 0;
	return ret;
}
