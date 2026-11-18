/* OVL057 - Overlay O057 (Segment 0x2056 in SCHWEIF.EXE): Balkenanzeigen Bars (struct bars, 0x15 Bytes, Objekt
 * g_bars) - Konstruktor setzt vier Farben und die Blinkzaehler, draw(idx, slot, val, max, mode, dst): senkrechter
 * LE- (idx 0) bzw. AE-Balken (idx 1, 3 Pixel breit, Hoehe h*val/max, per clamp_long auf 1..h) samt Loeschen des
 * Rests und Abschlusslinie; Modus 0 = Portraetleiste im Bildschirm g_vga_buf (Maus aus/an), 1 = Kampf, 2 =
 * Kampf-Heldenliste (O148), 3 = Kampf unten (O144) im Puffer dst; tick(): Hunger- und Durstbalken des Helden
 * g_sheet_hero im Heldenbogen (Breite Wert/2, ab 100 alle 25 Aufrufe blinkend).
 *
 * BCC -mlarge -3 -Ot -Og -c -Y OVL057.CPP   -> 1127/1127 Bytes GLEICH (18.10.2026), cobjfix laeuft
 * - -Og noetig: Ende des if-Zweigs (t2++ == 25) wird als Epilog kopiert statt jmp ans Funktionsende.
 * - Konstruktor ohne jmp near (Rumpf kurz genug): normaler C++-Konstruktor; t1 = b1 = t2 = b2 = 0 als Kette.
 * - len = h; len *= val; len /= max; als drei Anweisungen (mov di,ax / mov ax,di dazwischen).
 * - w = g >> 1 (sar) statt / 2; g_sheet_hunger_shown/g_sheet_thirst_shown signed char (movsx/sar).
 *   (Namensfassung mit schweif.h, Welle N19: 19.10.2026 GLEICH, cobjfix LINKGLEICH)
 */
#include "schweif.h"

Bars::Bars()
{
	col[0] = 0xfc;
	col[1] = 0xfd;
	col2[0] = 0xd6;
	col2[1] = 0xbb;
	blink1 = phase1 = blink2 = phase2 = 0;
}

void Bars::draw(signed short idx, signed short slot, signed short val, signed short max, signed short mode, void far *dst)
{
	signed short x;
	signed short y;
	signed short h;
	void far *d;
	signed short i;
	signed short len;

	h = 0x20;
	drawn[idx] = 0;
	if (mode == 0)
		mouse_hide(0);
	if (mode == 0) {
		x = g_portrait_x[slot] + idx * 4 + 0x22;
		y = 0xbd;
		d = g_vga_buf;
	} else if (mode == 2) {
		x = idx * 4 + 0x13;
		y = 0x25;
		d = dst;
		h = 0x24;
	} else {
		x = idx * 4 + 0x24;
		if (mode == 3)
			y = 0xbe;
		else
			y = 0x2a;
		d = dst;
	}
	if (val != 0) {
		len = h;
		len *= val;
		len /= max;
		len = clamp_long((signed long)len, 1L, (signed long)h);
		for (i = 0; i < 3; i++)
			do_v_line(d, x + i, y - len, len, col[idx]);
		if (mode != 2)
			for (i = 0; i < 3; i++)
				do_v_line(d, x + i, y - h, h - len, 0);
		do_h_line(d, x, y - len, 3, col2[idx]);
	} else {
		if (mode != 2)
			for (i = 0; i < 3; i++)
				do_v_line(d, x + i, y - h, h, 0);
	}
	if (mode == 0)
		mouse_show(0);
}

void Bars::tick()
{
	struct hero far *h;
	signed short x;
	signed short y1;
	signed short y2;
	signed short i;
	signed short w;

	x = 0x82;
	y1 = 0x41;
	y2 = 0x48;
	h = g_sheet_hero;
	if (h->hunger >= 100)
		g_sheet_hunger_shown = 100;
	if (h->thirst >= 100)
		g_sheet_thirst_shown = 100;
	if (g_sheet_hunger_shown == 100 && g_sheet_thirst_shown == 100) {
		blink1 = blink2;
		phase1 = phase2;
	}
	if (g_sheet_hunger_shown == 100) {
		if (blink1++ == 25) {
			phase1 ^= 1;
			mouse_hide(0);
			for (i = 0; i < 6; i++)
				do_h_line(g_vga_buf, x, y1 + i, 50, phase1 == 1 ? 0xfd : 0xbb);
			mouse_show(0);
			blink1 = 0;
		}
	} else if (h->hunger != g_sheet_hunger_shown) {
		g_sheet_hunger_shown = h->hunger;
		w = g_sheet_hunger_shown >> 1;
		mouse_hide(0);
		for (i = 0; i < 6; i++) {
			do_h_line(g_vga_buf, x, y1 + i, w, 0xfd);
			do_h_line(g_vga_buf, x + w, y1 + i, 50 - w, 0xbb);
		}
		mouse_show(0);
	}
	if (g_sheet_thirst_shown == 100) {
		if (blink2++ == 25) {
			phase2 ^= 1;
			mouse_hide(0);
			for (i = 0; i < 6; i++)
				do_h_line(g_vga_buf, x, y2 + i, 50, phase2 == 1 ? 0xfb : 0xf4);
			mouse_show(0);
			blink2 = 0;
		}
	} else if (h->thirst != g_sheet_thirst_shown) {
		g_sheet_thirst_shown = h->thirst;
		w = g_sheet_thirst_shown >> 1;
		mouse_hide(0);
		for (i = 0; i < 6; i++) {
			do_h_line(g_vga_buf, x, y2 + i, w, 0xfb);
			do_h_line(g_vga_buf, x + w, y2 + i, 50 - w, 0xf4);
		}
		mouse_show(0);
	}
}
