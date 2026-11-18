/* OVL208 - Overlay O208 (Segment 0x2390 in SCHWEIF.EXE): Legespiel im Phex-Tempel (Dungeon 5, O204 an 0x30b) mit
 * 3 x 5 verdeckten Karten, class Puzzle (s_class.h, 0x29 Bytes, vom Aufrufer mit new angelegt).
 * Puzzle::run: g_pp20_index = -1, Bildschirm 64000 Bytes und Grafikdatei 0x3b3 laden (load_data, Palette = letzte
 *   0x300 Bytes), Karten mischen (shuffle: zwoelf Karten mit den Werten 3..14 zufaellig per random_schweif, die drei
 *   uebrigen Wert 2), Tisch zeichnen (draw_table: Hintergrund und Kartenruecken per NvfMem::unpack in den Puffer, 15
 *   Kopien per Bmp::over), Schleife poll_input/handle_keys (Pfeile bewegen den Rahmen, Enter deckt auf)/handle_mouse
 *   (Position -> Spalte/Zeile, Klick deckt auf)/check_won bis Esc (g_action 0x11b), g_mouse_rclick oder gewonnen;
 *   Rueckgabe won.
 * Aufdecken: moves + 1, die aelteste der drei offenen Karten (shown[0]) wieder umdrehen (unflip_oldest), Liste
 *   schieben (push_shown), neue Karte mit Klang 0xac in fuenf Bildern umdrehen (flip_card). check_won: sind die drei
 *   offenen Karten die drei mit Wert 2 -> gewonnen. draw_cursor (Rahmen do_border: alter Farbe 0, neuer 0xff),
 *   show_table (Puffer zentriert nach g_vga_buf).
 *
 * BCC -mlarge -3 -Ot -Og -Oi -c -Y OVL208.CPP   -> 4081/4081 Bytes GLEICH (18.10.2026), cobjfix laeuft
 *   (Namensfassung mit schweif.h: 19.10.2026 GLEICH, LINKGLEICH; class Puzzle nach s_class.h, Methoden umbenannt)
 * - -Oi fuer memset(grid, 0, 15) (rep stosw + stosb); -Og fuer die Zwischenwerte ([bp-8]/[bp-0ah] Offsets in
 *   shuffle, [bp-2]/[bp-4] = card_h bzw. card_w + 2 in draw_cursor, [bp-24h] = sel in flip_card, [bp-6]/[bp-8] =
 *   320 - table_w bzw. 200 - table_h in handle_mouse).
 * - unflip_oldest: Kartennummer ueber die Schleifenvariable ("i = shown[0]; x = i % 5; y = i / 5;" -> mov si,ax),
 *   mit shown[0] % 5 direkt kaeme ein -Og-Zwischenwert auf dem Stack. check_won: dritte register-Variable c -> CX
 *   (keine Aufrufe), x/y in SI/DI.
 * - Lokale Bmp-Objekte (R42) mitten in der Funktion deklariert (Konstruktor an der Stelle), NvfMem (R41, 0x1a Bytes)
 *   kommt als grosses Objekt ans Ende des Rahmens; Destruktoren rueckwaerts. Bmp::put(dst, x, y) = gb_blit,
 *   put(dst, x, y, mode) = gb_blit_m, over(b, x, y, mode) = gb_put; NvfMem::unpack liefert long (hier verworfen).
 * - "*(grid + y * 5 + x)" (les / add bx,ax / add bx,di), nicht grid[y * 5 + x].
 * - handle_keys: if/else-if-Kette, die Epilogkopien kommen von -Og.
 * Originalfehler/Auffaelliges: handle_mouse liest die Mausposition in beiden Zweigen von "if (g_mouse_ldown == 0)"
 *   gleich (toter Zweig); last_sel wird nie -1 (Pruefung in draw_cursor wirkungslos); mem_alloc-Ergebnisse
 *   ungeprueft.
 */
#include <mem.h>

#include "schweif.h"

signed short Puzzle::run()
{
	g_pp20_index = -1;
	shown[0] = shown[1] = shown[2] = -1;
	sel = 0;
	last_sel = 0;
	won = 0;
	moves = 0;
	saved_handler = g_mouse_handler_on;
	g_mouse_handler_on = 0;
	load_data();
	shuffle();
	draw_table();
	do {
		poll_input();
		handle_keys();
		handle_mouse();
		check_won();
	} while (g_action != 0x11b && g_mouse_rclick == 0 && won == 0);
	delete grid;
	mem_free(file_data);
	mem_free(screen);
	g_mouse_handler_on = saved_handler;
	g_refresh = 1;
	return won;
}

void Puzzle::load_data()
{
	signed long len;

	screen = (char far *)mem_alloc(64000L);
	len = file_length(g_file_main, 0x3b3);
	file_data = (char far *)mem_alloc(len);
	file_load(g_file_main, 0x3b3, file_data);
	pal = file_data + len - 0x300;
}

void Puzzle::shuffle()
{
	signed short i;
	signed char far *g;
	register signed short r;

	grid = new signed char[15];
	memset(grid, 0, 15);
	for (i = 0, g = grid; i < 12; i++) {
		do
			r = random_schweif(15) - 1;
		while (g[r] != 0);
		g[r] = i + 3;
	}
	for (i = 0; i < 15; i++)
		if (!g[i])
			g[i] = 2;
}

void Puzzle::draw_table()
{
	NvfMem nvf;
	char far *p;
	register signed short x;
	register signed short y;

	nvf.unpack(screen, file_data, 0, 0, &table_w, &table_h, 0x140);
	p = screen + table_h * 0x140;
	nvf.unpack(p, file_data, 1, 0, &card_w, &card_h, 0x140);
	Bmp b1(table_w, table_h, screen);
	Bmp b2(card_w, card_h, p);
	for (y = 0; y < 3; y++)
		for (x = 0; x < 5; x++)
			b1.over(&b2, (card_w + 2) * x + 20, (card_h + 1) * y + 18, 0);
	set_palette(pal, 0, 0x100);
	show_table();
	draw_cursor();
}

void Puzzle::draw_cursor()
{
	register signed short x;
	register signed short y;

	mouse_hide(0);
	if (last_sel != -1 && last_sel != sel) {
		x = last_sel % 5;
		y = last_sel / 5;
		do_border(g_vga_buf, (0x140 - table_w) / 2 + (card_w + 2) * x + 19,
			(200 - table_h) / 2 + (card_h + 1) * y + 17,
			card_w + 2, card_h + 2, 0);
	}
	x = sel % 5;
	y = sel / 5;
	do_border(g_vga_buf, (0x140 - table_w) / 2 + (card_w + 2) * x + 19,
		(200 - table_h) / 2 + (card_h + 1) * y + 17,
		card_w + 2, card_h + 2, 0xff);
	mouse_show(0);
}

void Puzzle::show_table()
{
	Bmp b(table_w, table_h, screen);

	mouse_hide(0);
	b.put(g_vga_buf, (0x140 - table_w) / 2, (200 - table_h) / 2);
	mouse_show(0);
}

void Puzzle::handle_keys()
{
	if (g_action == 0x5000) {
		sel += 5;
		if (sel > 14)
			sel -= 5;
		draw_cursor();
		last_sel = sel;
	} else if (g_action == 0x4800) {
		sel -= 5;
		if (sel < 0)
			sel += 5;
		draw_cursor();
		last_sel = sel;
	} else if (g_action == 0x4b00) {
		sel--;
		if (sel < 0)
			sel = 0;
		draw_cursor();
		last_sel = sel;
	} else if (g_action == 0x4d00) {
		sel++;
		if (sel > 14)
			sel = 14;
		draw_cursor();
		last_sel = sel;
	} else if (g_action == 0x1c0d && shown[0] != sel && shown[1] != sel && shown[2] != sel) {
		moves++;
		if (shown[0] != -1)
			unflip_oldest();
		push_shown();
		flip_card();
	}
}

void Puzzle::flip_card()
{
	signed short y;
	signed short tmp;
	Bmp b1(0x35, 0x2a);
	Bmp b2(card_w, card_h);
	NvfMem nvf;
	register signed short i;
	register signed short x;

	x = sel % 5;
	y = sel / 5;
	mouse_hide(0);
	snd_play(g_snd_fx, 0xac);
	for (i = 15; i < 20; i++) {
		nvf.unpack(b1.data(), file_data, i, 0, &tmp, &tmp, 0x140);
		nvf.unpack(b2.data(), file_data, i == 19 ? *(grid + y * 5 + x) : 1, 0, &tmp, &tmp, 0x140);
		wait_for_vsync();
		b2.put(g_vga_buf, (0x140 - table_w) / 2 + (card_w + 2) * x + 20,
			(200 - table_h) / 2 + (card_h + 1) * y + 18);
		b1.put(g_vga_buf, (0x140 - table_w) / 2 + (card_w + 2) * x + 13,
			(200 - table_h) / 2 + (card_h + 1) * y + 8, 4);
		wait_for_vsync();
		wait_for_vsync();
		wait_for_vsync();
	}
	b2.put(g_vga_buf, (0x140 - table_w) / 2 + (card_w + 2) * x + 20, (200 - table_h) / 2 + (card_h + 1) * y + 17);
	mouse_show(0);
	Bmp b3(table_w, table_h, screen);
	b3.over(&b2, (card_w + 2) * x + 20, (card_h + 1) * y + 18, 0);
	show_table();
	draw_cursor();
}

void Puzzle::unflip_oldest()
{
	signed short y;
	signed short tmp;
	Bmp b1(0x35, 0x2a);
	Bmp b2(card_w, card_h);
	NvfMem nvf;
	register signed short i;
	register signed short x;

	i = shown[0];
	x = i % 5;
	y = i / 5;
	mouse_hide(0);
	for (i = 15; i < 20; i++) {
		nvf.unpack(b1.data(), file_data, i, 0, &tmp, &tmp, 0x140);
		nvf.unpack(b2.data(), file_data, i == 19 ? 1 : *(grid + y * 5 + x), 0, &tmp, &tmp, 0x140);
		wait_for_vsync();
		b2.put(g_vga_buf, (0x140 - table_w) / 2 + (card_w + 2) * x + 20,
			(200 - table_h) / 2 + (card_h + 1) * y + 18);
		b1.put(g_vga_buf, (0x140 - table_w) / 2 + (card_w + 2) * x + 13,
			(200 - table_h) / 2 + (card_h + 1) * y + 8, 4);
		wait_for_vsync();
		wait_for_vsync();
		wait_for_vsync();
	}
	b2.put(g_vga_buf, (0x140 - table_w) / 2 + (card_w + 2) * x + 20, (200 - table_h) / 2 + (card_h + 1) * y + 17);
	mouse_show(0);
	Bmp b3(table_w, table_h, screen);
	b3.over(&b2, (card_w + 2) * x + 20, (card_h + 1) * y + 18, 0);
	show_table();
	draw_cursor();
}

void Puzzle::push_shown()
{
	shown[0] = shown[1];
	shown[1] = shown[2];
	shown[2] = sel;
}

void Puzzle::check_won()
{
	register signed short x;
	register signed short y;
	register signed short c;

	if (shown[0] != -1 && shown[1] != -1 && shown[2] != -1) {
		c = shown[0];
		x = c % 5;
		y = c / 5;
		if (*(grid + y * 5 + x) == 2) {
			c = shown[1];
			x = c % 5;
			y = c / 5;
			if (*(grid + y * 5 + x) == 2) {
				c = shown[2];
				x = c % 5;
				y = c / 5;
				if (*(grid + y * 5 + x) == 2)
					won = 1;
			}
		}
	}
}

void Puzzle::handle_mouse()
{
	signed short col;
	signed short row;
	register signed short x;
	register signed short y;

	if (g_mouse_ldown == 0) {
		x = g_mouse_posx;
		y = g_mouse_posy;
	} else {
		x = g_mouse_posx;
		y = g_mouse_posy;
	}
	if (g_mouse_ldown != 0 || mouse_x != x || mouse_y != y) {
		mouse_x = x;
		mouse_y = y;
		if ((0x140 - table_w) / 2 + card_w + 21 > x)
			col = 0;
		else if ((0x140 - table_w) / 2 + card_w * 2 + 23 > x)
			col = 1;
		else if ((0x140 - table_w) / 2 + card_w * 3 + 25 > x)
			col = 2;
		else if ((0x140 - table_w) / 2 + card_w * 4 + 27 > x)
			col = 3;
		else
			col = 4;
		if ((200 - table_h) / 2 + card_h + 18 > y)
			row = 0;
		else if ((200 - table_h) / 2 + card_h * 2 + 19 > y)
			row = 1;
		else
			row = 2;
		sel = row * 5 + col;
		if (sel != last_sel) {
			draw_cursor();
			last_sel = sel;
		}
		if (g_mouse_ldown != 0 && shown[0] != sel && shown[1] != sel && shown[2] != sel) {
			moves++;
			if (shown[0] != -1)
				unflip_oldest();
			push_shown();
			flip_card();
		}
	}
}
