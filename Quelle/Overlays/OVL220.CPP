/* OVL220 - Overlay O220 (Segment 0x23ca in SCHWEIF.EXE): Richtungswahl mit der Kompassrose (51x51) der Reisekarte
 * (Travel: Auswahl dir_sel 1..8, Vorgabe dir_def, Lage rose_x/rose_y, alte Lage old_rose_x/_y, Ort place (struct dir_set, Kopie aus
 * g_places, freie Richtungen place.dirs), Bildpuffer rose_bg_buf/rose_buf, 17 Teilbilder rose_pics, Bmp-Elemente
 * rose_bg (Hintergrund) und rose, sichtbar sub_scene, Kartenpunkte map_pts[0..3]).
 * rose_alloc/rose_free: Puffer anlegen (17 Bilder der Datei 0x95 laden) bzw. freigeben. rose_mouse_area(p, tab):
 * Mausbereiche (struct mouse_area, Ende x0 == -1) relativ zur Rose pruefen (is_mouse_in_rect), Aktion des letzten
 * Treffers. rose_dir_allowed: Richtung dir_sel frei? (Bit in place.dirs, g_action = Richtung bzw. 0). rose_compose: Rose
 * aus Grundbild, freien Richtungen und gewaehlter Richtung zusammensetzen (lokale Bmp, Bmp::over). rose_open(p, ort):
 * Ort uebernehmen, Auswahl g_action bzw. naechste freie (rose_next_dir), zeichnen. rose_draw: zeichnen; beim ersten
 * Mal (g_rose_placed == 0) Lage in einer Ecke (10/0xb4 x 10/0x8c), die die beiden Kartenpunkte
 *   (tmap_screen_x/tmap_screen_y)
 * nicht verdeckt. rose_hide: Hintergrund zurueck. rose_move(p, tasten): Rose mit Pfeiltasten (Umschalt = 8 Pixel)
 * bzw. Maus verschieben, bei Verdecken der Punkte zurueck in die Ecke. rose_move_to: an der neuen Lage neu zeichnen.
 * rose_next_dir: von dir_def aus abwechselnd links/rechts die naechste freie Richtung suchen. Richtungen (Bit k =
 * dir_sel k + 1, struct dir_bits d0..d7): Norden, Nordosten, Osten ... Nordwesten (Lage der Teilbilder in rose_compose).
 *
 * BCC -mlarge -3 -Ot -Og -c -Y OVL220.CPP   -> 4529/4529 Bytes GLEICH (18.10.2026), cobjfix laeuft
 *   (Namensfassung mit schweif.h: 19.10.2026 GLEICH, LINKGLEICH)
 * - -Og noetig: rose_dir_allowed "switch (p->dir_sel)" + "g_action = p->dir_sel" -> gemeinsamer Zwischenwert [bp-2]
 *   (mov [bp-2],ax / dec ax); eigene Variable sel landet in CX bzw. ergibt mov bx,ax / dec bx. HIT-Makros:
 *   p->rose_x + 50 in [bp-0ch], p->rose_x in DI, p->rose_y in SI (je Teilbedingung neu).
 * - C++ wegen lokaler Bmp-Objekte (gb_new / gb_delete(&b, 2)) in eigenen Bloecken (gemeinsamer Platz [bp-0ah]).
 * - rose_compose: zweiter switch mit "return;" je Fall (Epilogkopien) und "default: return;" (sonst fehlt der tote
 *   Epilog am Ende); Fallreihenfolge 1, 5, 7, 3, 8, 2, 4, 6 (Sprungtabelle).
 * - rose_open: zweiter Parameter ist eine 3-Byte-Struktur als Wert ("p->place = v" -> mov ax / mov al mit einem les).
 * - rose_move: letzter case (0x4d00) ohne break in "default: break;" -> nur ein jmp short vor dem Ende.
 * Originalfehler: rose_draw setzt die Rose als letzten Ausweg auf (200, 200) - unterhalb des 200 Zeilen hohen
 *   Bildschirms (Hintergrund-/Zeichenzugriff ausserhalb). rose_alloc uebergibt zweimal dieselbe Dummy-Variable.
 */
#include <bios.h>
#include "schweif.h"

void rose_alloc(Travel far *p)
{
	signed short dummy;
	register signed short i;

	p->rose_bg_buf = mem_alloc(0xa29L);
	p->rose_bg.init(0x33, 0x33, p->rose_bg_buf);
	p->rose_buf = mem_alloc(0xa29L);
	p->rose.init(0x33, 0x33, p->rose_buf);
	p->rose_pics[0] = mem_alloc(0xa29L);
	for (i = 1; i <= 4; i++)
		p->rose_pics[i] = mem_alloc(0x145L);
	for (i = 5; i <= 8; i++)
		p->rose_pics[i] = mem_alloc(0x144L);
	for (i = 9; i <= 12; i++)
		p->rose_pics[i] = mem_alloc(0x145L);
	for (i = 13; i <= 16; i++)
		p->rose_pics[i] = mem_alloc(0x144L);
	for (i = 0; i <= 16; i++)
		nvfile_load(g_nvf_heads, 0x95, p->rose_pics[i], 0, i, 0, &dummy, &dummy, 0x140);
	p->dir_sel = 1;
	p->sub_scene = 0;
}

void rose_free(Travel far *p)
{
	register signed short i;

	for (i = 0; i <= 16; i++)
		p->rose_pics[i] = mem_free(p->rose_pics[i]);
	p->rose_bg.free_lines();
	p->rose_bg_buf = mem_free(p->rose_bg_buf);
	p->rose.free_lines();
	p->rose_buf = mem_free(p->rose_buf);
}

signed short rose_mouse_area(Travel far *p, struct mouse_area far *a)
{
	signed short x1;
	signed short y0;
	signed short y1;
	signed short r;
	register signed short x0;

	r = 0;
	while (a->x0 != -1) {
		x0 = a->x0 + p->rose_x;
		y0 = a->y0 + p->rose_y;
		x1 = a->x1 + p->rose_x;
		y1 = a->y1 + p->rose_y;
		if (is_mouse_in_rect(x0, y0, x1, y1))
			r = a->action;
		a++;
	}
	return r;
}

signed short rose_dir_allowed(Travel far *p)
{
	signed short r = 0;

	switch (p->dir_sel) {
	case 1:
		if (p->place.dirs.d0)
			r = 1;
		break;
	case 2:
		if (p->place.dirs.d1)
			r = 1;
		break;
	case 3:
		if (p->place.dirs.d2)
			r = 1;
		break;
	case 4:
		if (p->place.dirs.d3)
			r = 1;
		break;
	case 5:
		if (p->place.dirs.d4)
			r = 1;
		break;
	case 6:
		if (p->place.dirs.d5)
			r = 1;
		break;
	case 7:
		if (p->place.dirs.d6)
			r = 1;
		break;
	case 8:
		if (p->place.dirs.d7)
			r = 1;
		break;
	}
	if (r)
		g_action = p->dir_sel;
	else
		g_action = 0;
	return r;
}

void rose_compose(Travel far *p)
{
	{
		Bmp b(0x33, 0x33, p->rose_pics[0]);
		p->rose.over(&b, 0, 0, 4);
	}
	if (p->place.dirs.d0) {
		Bmp b(0x0d, 0x19, p->rose_pics[1]);
		p->rose.over(&b, 0x13, 0, 4);
	}
	if (p->place.dirs.d4) {
		Bmp b(0x0d, 0x19, p->rose_pics[2]);
		p->rose.over(&b, 0x13, 0x1a, 4);
	}
	if (p->place.dirs.d6) {
		Bmp b(0x19, 0x0d, p->rose_pics[3]);
		p->rose.over(&b, 0, 0x13, 4);
	}
	if (p->place.dirs.d2) {
		Bmp b(0x19, 0x0d, p->rose_pics[4]);
		p->rose.over(&b, 0x1a, 0x13, 4);
	}
	if (p->place.dirs.d7) {
		Bmp b(0x12, 0x12, p->rose_pics[5]);
		p->rose.over(&b, 7, 7, 4);
	}
	if (p->place.dirs.d1) {
		Bmp b(0x12, 0x12, p->rose_pics[6]);
		p->rose.over(&b, 0x1a, 7, 4);
	}
	if (p->place.dirs.d3) {
		Bmp b(0x12, 0x12, p->rose_pics[7]);
		p->rose.over(&b, 0x1a, 0x1a, 4);
	}
	if (p->place.dirs.d5) {
		Bmp b(0x12, 0x12, p->rose_pics[8]);
		p->rose.over(&b, 7, 0x1a, 4);
	}
	switch (p->dir_sel) {
	case 1: {
		Bmp b(0x0d, 0x19, p->rose_pics[9]);
		p->rose.over(&b, 0x13, 0, 4);
		return;
	}
	case 5: {
		Bmp b(0x0d, 0x19, p->rose_pics[10]);
		p->rose.over(&b, 0x13, 0x1a, 4);
		return;
	}
	case 7: {
		Bmp b(0x19, 0x0d, p->rose_pics[11]);
		p->rose.over(&b, 0, 0x13, 4);
		return;
	}
	case 3: {
		Bmp b(0x19, 0x0d, p->rose_pics[12]);
		p->rose.over(&b, 0x1a, 0x13, 4);
		return;
	}
	case 8: {
		Bmp b(0x12, 0x12, p->rose_pics[13]);
		p->rose.over(&b, 7, 7, 4);
		return;
	}
	case 2: {
		Bmp b(0x12, 0x12, p->rose_pics[14]);
		p->rose.over(&b, 0x1a, 7, 4);
		return;
	}
	case 4: {
		Bmp b(0x12, 0x12, p->rose_pics[15]);
		p->rose.over(&b, 0x1a, 0x1a, 4);
		return;
	}
	case 6: {
		Bmp b(0x12, 0x12, p->rose_pics[16]);
		p->rose.over(&b, 7, 0x1a, 4);
		return;
	}
	default:
		return;
	}
}

void rose_open(Travel far *p, struct dir_set place)
{
	p->place = place;
	if (g_action >= 1 && g_action <= 8) {
		p->dir_sel = g_action;
		if (rose_dir_allowed(p))
			rose_draw(p);
		return;
	}
	if (!rose_dir_allowed(p))
		rose_next_dir(p);
	rose_draw(p);
}

#define HIT(x, y, n)	(p->rose_x + (n) > (x) && p->rose_x < (x) && p->rose_y + (n) > (y) && p->rose_y < (y))

void rose_draw(Travel far *p)
{
	signed short flag;
	signed short x1;
	signed short y1;
	signed short x2;
	signed short y2;

	flag = 0;
	rose_compose(p);
	if (g_rose_placed == 0) {
		x1 = tmap_screen_x(g_tmap, p->map_pts[0]);
		y1 = tmap_screen_y(g_tmap, p->map_pts[1]);
		x2 = tmap_screen_x(g_tmap, p->map_pts[2]);
		y2 = tmap_screen_y(g_tmap, p->map_pts[3]);
		p->rose_x = p->rose_y = p->old_rose_x = p->old_rose_y = 10;
		g_rose_placed = 1;
		if (HIT(x1, y1, 50) || HIT(x2, y2, 50)) {
			p->rose_x = p->old_rose_x = 0xb4;
			p->rose_y = p->old_rose_y = 10;
			g_rose_placed = 1;
			if (HIT(x1, y1, 50) || HIT(x2, y2, 50)) {
				p->rose_x = p->old_rose_x = 10;
				p->rose_y = p->old_rose_y = 0x8c;
				g_rose_placed = 1;
				if (HIT(x1, y1, 50) || HIT(x2, y2, 50)) {
					p->rose_x = p->old_rose_x = 0xb4;
					p->rose_y = p->old_rose_y = 0x8c;
					g_rose_placed = 1;
					if (HIT(x1, y1, 50) || HIT(x2, y2, 50)) {
#if FIX_0097
						/* Fix #97: letzter Ausweg sichtbar oben links statt unterhalb des Bildes */
						p->rose_x = p->rose_y = p->old_rose_x = p->old_rose_y = 0;
#else
						p->rose_x = p->rose_y = p->old_rose_x = p->old_rose_y = 200;
#endif
						g_rose_placed = 1;
					}
				}
			}
		}
		p->rose_bg.get(g_vga_buf, p->rose_x, p->rose_y);
	}
	if (mouse_overlaps_rect(p->rose_x, p->rose_y, p->rose_x + 0x33, p->rose_y + 0x33)) {
		flag = 1;
		mouse_hide(0);
	}
	p->rose.put(g_vga_buf, p->rose_x, p->rose_y, 4);
	if (flag)
		mouse_show(0);
	p->sub_scene = 1;
}

void rose_hide(Travel far *p)
{
	mouse_hide(0);
	p->rose_bg.put(g_vga_buf, p->rose_x, p->rose_y);
	p->sub_scene = 0;
	g_rose_placed = 0;
	mouse_show(0);
}

#define HIT2(x, y)	(p->rose_x + 52 > (x) && p->rose_x < (x) + 9 && p->rose_y + 52 > (y) && p->rose_y < (y) + 9)

void rose_move(Travel far *p, signed short keys)
{
	signed short y1;
	signed short x2;
	signed short y2;
	register signed short x1;
	register signed short step;

	x1 = tmap_screen_x(g_tmap, p->map_pts[0]);
	y1 = tmap_screen_y(g_tmap, p->map_pts[1]);
	x2 = tmap_screen_x(g_tmap, p->map_pts[2]);
	y2 = tmap_screen_y(g_tmap, p->map_pts[3]);
	if (p->sub_scene != 0) {
		mouse_hide(0);
		if (keys) {
			while (g_key != 0x1c0d && g_key != 0x11b && g_key != 0x5100) {
				if (bioskey(1))
					g_key = bioskey(0);
				else
					g_key = 0;
				g_key_shift = bioskey(2) & 3;
				if (g_key_shift != 0)
					step = 8;
				else
					step = 1;
				switch (g_key) {
				case 0x4800:
					p->rose_y = clamp_min_l((signed long)(p->rose_y - step), 0L);
					break;
				case 0x4b00:
					p->rose_x = clamp_min_l((signed long)(p->rose_x - step), 0L);
					break;
				case 0x5000:
					p->rose_y = clamp_max_l((signed long)(p->rose_y + step), 0x95L);
					break;
				case 0x4d00:
					p->rose_x = clamp_max_l((signed long)(p->rose_x + step), 0xbdL);
				default:
					break;
				}
				rose_move_to(p);
			}
		} else {
			mouse_move(p->rose_x, p->rose_y);
			while (g_mouse_rdown) {
				p->rose_x = g_mouse_posx;
				if (p->rose_x > 0xbd)
					p->rose_x = 0xbd;
				p->rose_y = g_mouse_posy;
				if (p->rose_y > 0x95)
					p->rose_y = 0x95;
				rose_move_to(p);
			}
		}
		if (HIT2(x1, y1) || HIT2(x2, y2)) {
			rose_hide(p);
			p->rose_x = p->rose_y = p->old_rose_x = p->old_rose_y = 10;
			rose_draw(p);
		}
		mouse_show(0);
	}
	clear_input();
	mouse_clear_left();
	mouse_clear_right();
}

void rose_move_to(Travel far *p)
{
	if (p->rose_x != p->old_rose_x || p->rose_y != p->old_rose_y) {
		p->rose_bg.put(g_vga_buf, p->old_rose_x, p->old_rose_y);
		p->rose_bg.get(g_vga_buf, p->rose_x, p->rose_y);
		p->rose.put(g_vga_buf, p->rose_x, p->rose_y, 4);
		p->old_rose_x = p->rose_x;
		p->old_rose_y = p->rose_y;
	}
}

void rose_next_dir(Travel far *p)
{
	signed char s = -1;
	signed char k = 1;

	p->dir_sel = p->dir_def;
	while (!rose_dir_allowed(p)) {
		p->dir_sel -= s * k;
		k += 1;
		s = -s;
		while (p->dir_sel <= 0)
			p->dir_sel += 8;
		p->dir_sel = (p->dir_sel - 1) % 8 + 1;
	}
}
