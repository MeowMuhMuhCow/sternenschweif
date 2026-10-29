/* OVL172 - Overlay O172 (Segment 0x22b8 in SCHWEIF.EXE): Bildauswahl-Fenster - n Bilder (30x30, Nummern ab start
 * aus Archiv ds:b63d Typ 5 geladen, ueber zwei 10-Byte-Bildobjekte 32x32 -> 28x28 skaliert) nebeneinander in
 * einem Fenster (O205_*), Auswahl per Maus (Trefferrechtecke ds:349f, 10 Bytes je Eintrag) oder Cursor links/
 * rechts, Fenster verschiebbar; Rueckgabe start+Auswahl bzw. -1 (ESC/Bild hoch/runter/rechte Maustaste).
 * f_0250 ist nur ein Weiterreicher auf f_0123.
 *
 * BCC -mlarge -3 -Ot -Og -c -Y OVL172.CPP   -> 1614/1614 Bytes GLEICH (18.10.2026), cobjfix laeuft
 * - C++ wegen der lokalen Objekte (Konstruktor R42_00D8(w,h), Destruktor R42_03B3 mit flags 2 in umgekehrter
 *   Reihenfolge); Objektgroesse 10 Bytes (sub sp,18h verraet es: t, b1, y, b2 in Deklarationsreihenfolge).
 * - (rows + 2) << 5 statt * 32 (sonst shl / add 40h); -Og: [bp-18h]/[bp-1Ah] und SI = Index*10-Zwischenwerte.
 * - f_026a: Schleifenzaehler i ohne register (sonst SI, und der -Og-Wert rutscht nach DI).
 * - sel--; if (sel == -1) getrennt (cmp word [bp-10h],-1); --sel == -1 ergibt mov ax / cmp ax.
 * - !g_8bc0 (mov al / cbw / or ax,ax).
 * (Namensfassung mit schweif.h: 19.10.2026 GLEICH, LINKGLEICH) Die vier Funktionen sind Methoden der Fensterklasse
 *   Win (s_class.h): f_0000 draw_pics, f_0123 select_pic_run, f_0250 select_pic (frei: dlg_select_pic), f_026a
 *   select_pic_loop - gleicher Code wie freie Funktionen mit dem Fenster als erstem Argument.
 */
#include "schweif.h"

#define PIC_FILE_TYPE	5		/* Bildtyp in g_nvf_heads */

void Win::draw_pics(signed short n, signed short start)
{
	signed short dummy;
	Bmp small(0x1c, 0x1c);
	signed short py;
	Bmp pic(0x20, 0x20);
	register signed short i;
	register signed short px;

	py = y + ((h - 0x1c) >> 1);
	for (i = 0; i < n; i++) {
		px = x + ((w - n * 30) >> 1) + i * 30;
		nvfile_load(g_nvf_heads, PIC_FILE_TYPE, pic.data(), 0, start + i, 0, &dummy, &dummy, SCREEN_W);
		do_border(g_vga_buf, px, py, 30, 30, 0xfd);
		pic.scale(&small);
		small.put(g_vga_buf, px + 1, py + 1);
	}
	clear_kbd();
}

signed short Win::select_pic_run(register signed short n, register signed short start)
{
	signed short ret;
	signed short old;

	ret = 0;
	is_open = 1;
	old = tiles;
	tiles = n * 30 / 32;
	if (tiles > 8)
		tiles = 8;
	lines = 4;
	h = lines * 8 + 0x1c;
	y = calc_y(h);
	w = (tiles + 2) << 5;
	x = calc_x(w);
	mouse_hide(0);
	draw_frame(lines);
	draw_pics(n, start);
	mouse_show(0);
	ret = select_pic_loop(n, start);
	tiles = old;
	close();
	is_open = 0;
	return ret;
}

signed short Win::select_pic(signed short n, signed short start)
{
	return select_pic_run(n, start);
}

signed short Win::select_pic_loop(signed short n, signed short start)
{
	signed short px;
	signed short px2;
	signed short py;
	signed short cnt;
	signed short mx;
	signed short my;
	signed short ret;
	signed short sel;
	signed short old;
	signed short done;
	signed short redraw;
	signed short i;

	cnt = 0;
	mx = -1;
	my = -1;
	sel = 0;
	old = -1;
	done = 0;
	redraw = 1;
	clear_input();
	mouse_clear_left();
	mouse_clear_right();
	do {
		if (redraw) {
			redraw = 0;
			cnt = 0;
			g_dlg_areas[cnt].x0 = x;
			g_dlg_areas[cnt].x1 = x + w;
			g_dlg_areas[cnt].y0 = y;
			g_dlg_areas[cnt].y1 = y + 0x0f;
			cnt++;
			py = y + ((h - 0x1c) >> 1);
			for (i = 0; i < n; i++) {
				px = x + ((w - n * 30) >> 1) + i * 30;
				px2 = px + 29;
				g_dlg_areas[cnt].x0 = px;
				g_dlg_areas[cnt].x1 = px2;
				g_dlg_areas[cnt].y0 = py;
				g_dlg_areas[cnt].y1 = g_dlg_areas[cnt].y0 + 30;
				cnt++;
			}
			g_dlg_areas[cnt].x0 = 0;
			g_dlg_areas[cnt].x1 = 0xc7;
			g_dlg_areas[cnt].y0 = 0;
			g_dlg_areas[cnt].y1 = 0x13f;
		}
		g_mouse_areas2 = g_dlg_areas;
		poll_input();
		g_mouse_areas2 = 0;
		if (!(signed char)g_mouse_locked && (g_mouse_posx != mx || g_mouse_posy != my) && g_mouse_lclick == 0 && g_mouse_lpending == 0) {
			mx = g_mouse_posx;
			my = g_mouse_posy;
			for (i = n; g_dlg_areas[i].x0 > g_mouse_posx && i > 0; i--);
			sel = i - 1;
			if (sel < 0)
				sel = 0;
		}
		if (g_action == KEY_RIGHT) {
			sel++;
			if (sel == n)
				sel = 0;
		} else if (g_action == KEY_LEFT) {
			sel--;
			if (sel == -1)
				sel = n - 1;
		}
		if (sel != old) {
			mouse_hide(0);
			if (old != -1)
				do_border(g_vga_buf, g_dlg_areas[old + 1].x0, g_dlg_areas[old + 1].y0, 30, 30, 0xfd);
			do_border(g_vga_buf, g_dlg_areas[sel + 1].x0, g_dlg_areas[sel + 1].y0, 30, 30, 0xfc);
			mouse_show(0);
			old = sel;
		}
		if (g_action == 1 || g_key_shift != 0) {
			if (g_mouse_ldown != 0 || g_key_shift != 0) {
				drag(x, y, w, h);
				if (new_x != x || new_y != y) {
					mouse_hide(0);
					restore_bg();
					x = new_x;
					y = new_y;
					draw_frame(lines);
					draw_pics(n, start);
					old = -1;
					mouse_show(0);
					redraw = 1;
				}
			}
			clear_input();
			mouse_clear_left();
			mouse_clear_right();
		}
		if (g_action == KEY_RETURN || g_mouse_lclick != 0) {
			ret = start + sel;
			done = 1;
			mouse_clear_left();
		}
		if (g_key == KEY_PGUP || g_key == KEY_PGDN || g_mouse_rclick != 0 || g_action == KEY_ESC) {
			done = 1;
			ret = -1;
			mouse_clear_right();
		}
	} while (done == 0);
	g_dlg_areas[1].x0 = 0;
	g_dlg_areas[1].x1 = 0xc7;
	g_dlg_areas[1].y0 = 0;
	g_dlg_areas[1].y1 = 0x13f;
	g_dlg_areas[1].action = 2;
	return ret;
}
