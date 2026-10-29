/* OVL187 - Overlay O187 (Segment 0x22ff in SCHWEIF.EXE): Kartenobjekt der Reisekarte (class TMap in s_class.h, 0xf8
 * Bytes, global g_tmap ds:c217; Kartenbild ORKLAND.AIF = Datei 0x92, Laden in O038 travel_enter). Kopf (0x1e Bytes aus
 * der Datei: Groesse width/height, Palettenfarben nr_colors ab first_color), Palette pal (eigen bei own_pal), Ausschnitt
 * view_x/view_y (Kachel tile_x/tile_y auf 16/20 gerundet, Feinlage fine_x/fine_y), zuletzt verfolgter Punkt
 * last_x/last_y, Reihenfolge der 12 Kachelzeilen rows[], zwei animierte Kartenpunkte (CIRCLE.NVF = Datei 0x1a3,
 * je 4 Bilder 9 x 9 und Hintergrund; Zaehler g_tmap_mark1_tick/_mark2_tick), Bildpuffer scr. Das Kartenbild (0x230 Pixel
 * breit) liegt im EMS (g_ems_avail, Seitenrahmen g_ems_frame, Handle g_swap_handle2, EMS_map4) bzw. XMS
 * (g_xms_move, xms_move_any).
 * TMap()/~TMap(): Bmp-Mitglieder (_vector_new_/_vector_delete_). new_pal/free_pal: Palette anlegen/freigeben.
 * screen_x/screen_y: Kartenkoordinate -> Bildschirm. read_header/read_pal/read_map: Kopf, Palette (Dateiende - 3n),
 * Kartenbild in 48-KB-Stuecken nach EMS/XMS (g_file_main, g_tmap_loaded = geladen). set_pal: Palette setzen (Farben
 * 0..0x9f der Karte, 0xa0..0xff aus MMASK.NVF 0x94). get_size. load(id): laden. set_pos(x, y): Ausschnitt setzen,
 * 240 Zeilen holen. draw: zeichnen (tmap_blit_rows, Maus R18). load_col/load_row: Kachelspalte/-zeile nachladen.
 * scroll_right/_left/_down/_up: um ein Pixel verschieben. plot(x, y, c): Punkt setzen (alter Wert zurueck).
 * follow(x, y, steps, c): Ausschnitt einem Weg folgen lassen und Punkt setzen. plot_points: Punktliste bis -1.
 * mark1_init/mark2_init, mark1_free/mark2_free, mark1_anim/mark2_anim: Kartenpunkte 1/2 (Travel.map_pts) blinken
 * lassen - nicht unter einem offenen Fenster (dlg_overlaps) und nicht unter der Kompassrose (g_travel_rose_x/_y).
 *
 * BCC -mlarge -3 -Ot -Og -Oi -c -Y OVL187.CPP   -> 6084/6084 Bytes GLEICH (18.10.2026), cobjfix laeuft (s. u.).
 *   (Namensfassung mit schweif.h: 19.10.2026 GLEICH, LINKGLEICH)
 *   Ohne -Og 3446 Abweichungen; -Oi fuer memcpy (EMS-Kopien MK_FP(g_ems_frame, pos & 0x3fff) -> mov esi,eax / pop edi).
 * - Alles Elementfunktionen von TMap in Adressreihenfolge (read_map weiter unten -> far call, sonst push cs/near).
 * - Vorzeichen entscheidet "sub reg,k" gegen "add reg,-k": load_col(a, unsigned b): "(tile_y + b * 20 - 20) *
 *   0x230L" -> sub dx,14h / movzx eax (signed b: add dx,-14h); load_row(): "(unsigned long)(view_y + dy) * 0x230 +
 *   tile_x - 16" -> sub eax,10h (signed: add eax,-10h). Unsigned b ergibt auch rows[b + i] als mov ax,b / add ax,i /
 *   add bx,ax.
 * - huge nur in set_pos (p += 0x120 -> PADA@) und plot ((char huge *)scr + tile_x + off -> zweimal PADD@), sonst
 *   far-Arithmetik (add word [bp-4],120h). EMS_norm_ptr normalisiert einen Zeiger.
 * - Member "pal = 0" -> mov dword; "last_x = last_y = -1" (+2c zuerst); follow(): abs() inline (Kopien [bp-8]/[bp-0ah]).
 * - Destruktorzeiger fuer _vector_delete_: ASM "push seg F_R42_0008 / push 03b3h" (Segment ueber ein anderes
 *   Symbol); cobjfix loest das seit 19.10. selbst auf.
 * Auffaellig (Original): scroll_up zeichnet ab scr + tile_x (auf 16 gerundet), die drei anderen Richtungen ab
 *   scr + view_x - wohl ein Versehen.
 */
#include <dos.h>
#include <stdlib.h>
#include <string.h>
#include "schweif.h"

/* Phase B: eigene Daten des Moduls (ds:6e44..6e4c, dsana) - initialisierte Daten in Adressreihenfolge */
INN_PTR g_inn = 0;
unsigned char g_tmap_mark1_tick = 0x00;
unsigned char g_tmap_mark2_tick = 0x00;
signed short g_tmap_last_row = -1;

#define FILE_MMASK	0x94		/* MMASK.NVF: Rahmen der Reisekarte (Palette 0xa0..0xff) */
#define FILE_CIRCLE	0x1a3		/* CIRCLE.NVF: Kartenpunkte */

TMap::TMap()
{
	pal = 0;
	own_pal = 0;
	view_x = view_y = 0;
}

TMap::~TMap()
{
	if (own_pal)
		delete pal;
}

void TMap::new_pal()
{
	pal = new char[0x300];
	own_pal = 1;
}

signed short TMap::screen_x(signed short v)
{
	return v - view_x + 12;
}

signed short TMap::screen_y(signed short v)
{
	return v - view_y + 16;
}

void TMap::free_pal()
{
	if (own_pal) {
		delete pal;
		pal = 0;
		own_pal = 0;
	}
}

signed short TMap::read_header()
{
	if (file_open(g_file_main, file) > 0) {
		file_read(g_file_main, this, 30L);
		file_close(g_file_main);
		return 1;
	}
	return 0;
}

signed short TMap::read_pal()
{
	if (file_open(g_file_main, file) > 0) {
		file_seek(g_file_main, -((signed long)nr_colors * 3), 2);
		file_read(g_file_main, pal, nr_colors * 3);
		file_close(g_file_main);
		return 1;
	}
	return 0;
}

void TMap::set_pal()
{
	char far *p;

	p = (char far *)mem_alloc(0x300L);
	nvfile_pal(g_nvf_heads, FILE_MMASK, p, 0x100);
	set_palette(pal, first_color, 0xa0);
	memcpy(pal + 0x1e0, p + 0x1e0, 0x120);
	set_palette(p + 0x1e0, 0xa0, 0x60);
	mem_free(p);
}

void TMap::get_size(signed short far *a, signed short far *b)
{
	*a = width;
	*b = height;
}

signed short TMap::load(signed short id)
{
	file = id;
	if (read_header()) {
		if (own_pal == 0) {
			pal = new char[nr_colors * 3];
			own_pal = 1;
		}
		if (read_pal() && read_map())
			return 1;
	}
	return 0;
}

signed short TMap::read_map()
{
	signed long n;
	unsigned long pos;

	if (g_mem_saving == 1 || g_tmap_loaded == 0) {
		if (file_open(g_file_main, file) > 0) {
			bytes_left = file_length2(g_file_main) - 30 - (signed long)nr_colors * 3;
			file_seek(g_file_main, 30L, 0);
			pos = 0;
			do {
				n = file_read(g_file_main, scr, 0xc000L);
				if (g_ems_avail == 1 && g_mem_saving == 1) {
					EMS_map4(0, pos >> 14, g_swap_handle2);
					memcpy(MK_FP(g_ems_frame, pos & 0x3fff), scr, (unsigned)n);
				} else {
					g_xms_move.len = n;
					g_xms_move.src_handle = 0;
					g_xms_move.src = (signed long)scr;
					g_xms_move.dst_handle = g_swap_handle2;
					g_xms_move.dst = pos;
					xms_move_any(&g_xms_move);
				}
				pos += n;
				bytes_left = bytes_left - n;
			} while (bytes_left > 0);
			file_close(g_file_main);
			g_tmap_loaded = 1;
			return 1;
		}
	}
	return 0;
}

void TMap::set_pos(signed short x, signed short y)
{
	signed short i;
	signed short r;
	unsigned long pos;
	char huge *p;

	x -= 120;
	y -= 100;
	r = y % 20;
	y -= r;
	if (x < 0)
		x = 0;
	else if (x > 0x120)
		x = 0x120;
	if (y < 0)
		y = 0;
	else if (y > 400)
		y = 400;
	view_x = x + 16;
	view_y = y + 20;
	tile_x = view_x & 0xfff0;
	tile_y = view_y / 20 * 20;
	fine_x = view_x % 16;
	fine_y = view_y % 20;
	p = (char huge *)EMS_norm_ptr(scr + x - fine_x);
	pos = (signed long)y * 0x230 + x - fine_x;
	mouse_hide(0);
	if (g_ems_avail == 1 && g_mem_saving == 1) {
		for (i = 0; i < 0xf0; i++, pos += 0x230, p += 0x120) {
			EMS_map4(0, pos >> 14, g_swap_handle2);
			memcpy(p, MK_FP(g_ems_frame, pos & 0x3fff), 0x120);
		}
	} else {
		g_xms_move.len = 0x120;
		g_xms_move.src_handle = g_swap_handle2;
		g_xms_move.dst_handle = 0;
		for (i = 0; i < 0xf0; i++, pos += 0x230, p += 0x120) {
			g_xms_move.src = pos;
			g_xms_move.dst = (signed long)p;
			xms_move_any(&g_xms_move);
		}
	}
	mouse_show(0);
	for (i = 0; i < 12; i++)
		rows[i] = i;
	cur = scr + view_x + 0x1680;
	last_x = last_y = -1;
}

void TMap::draw()
{
	signed short hide;

	hide = 0;
	if (mouse_overlaps_rect(0, 0, 0xf8, 0xc8)) {
		hide = 1;
		mouse_hide(0);
	}
	tmap_blit_rows(scr + view_x, rows, fine_y);
	if (hide)
		mouse_show(0);
}

void TMap::load_col(signed short a, unsigned short b)
{
	char far *p;
	signed short j;
	signed short i;
	signed short cnt;
	unsigned long pos;
	unsigned short t;

	pos = (tile_y + b * 20 - 20) * 0x230L + tile_x + a;
	cnt = 3;
	if (tile_y < g_tmap_last_row)
		cnt = 4;
	g_tmap_last_row = tile_y;
	if (g_ems_avail == 1 && g_mem_saving == 1) {
		for (i = 0; i < cnt; i++) {
			t = rows[b + i];
			p = (char far *)EMS_norm_ptr(scr + t * 0x1680L + a + tile_x);
			for (j = 0; j < 20; j++, pos += 0x230, p += 0x120) {
				EMS_map4(0, pos >> 14, g_swap_handle2);
				memcpy(p, MK_FP(g_ems_frame, pos & 0x3fff), 16);
			}
		}
	} else {
		g_xms_move.len = 16;
		g_xms_move.src_handle = g_swap_handle2;
		g_xms_move.dst_handle = 0;
		for (i = 0; i < cnt; i++) {
			t = rows[b + i];
			p = (char far *)EMS_norm_ptr(scr + t * 0x1680L + a + tile_x);
			for (j = 0; j < 20; j++, pos += 0x230, p += 0x120) {
				g_xms_move.src = pos;
				g_xms_move.dst = (signed long)p;
				xms_move_any(&g_xms_move);
			}
		}
	}
}

void TMap::load_row(char far *dst, signed short dy)
{
	signed short i;
	unsigned long pos;

	dst = (char far *)EMS_norm_ptr(dst);
	pos = (unsigned long)(view_y + dy) * 0x230 + tile_x - 16;
	if (g_ems_avail == 1 && g_mem_saving == 1) {
		for (i = 0; i < 20; i++, pos += 0x230, dst += 0x120) {
			EMS_map4(0, pos >> 14, g_swap_handle2);
			memcpy(dst, MK_FP(g_ems_frame, pos & 0x3fff), 0x120);
		}
	} else {
		g_xms_move.len = 0x120;
		g_xms_move.src_handle = g_swap_handle2;
		g_xms_move.dst_handle = 0;
		for (i = 0; i < 20; i++, pos += 0x230, dst += 0x120) {
			g_xms_move.src = pos;
			g_xms_move.dst = (signed long)dst;
			xms_move_any(&g_xms_move);
		}
	}
}

signed char TMap::scroll_right(signed short draw)
{
	signed char moved = 0;

	if (view_x < 0x130) {
		view_x++;
		moved = 1;
		if (++fine_x > 15) {
			fine_x = 0;
			cur += 16;
			tile_x += 16;
		}
		if (draw == 1)
			tmap_blit_rows(scr + view_x, rows, fine_y);
		if (fine_x == 0)
			load_col(0x100, 0);
		else if (fine_x == 1)
			load_col(0x100, 3);
		else if (fine_x == 2)
			load_col(0x100, 6);
		else if (fine_x == 3)
			load_col(0x100, 9);
	}
	return moved;
}

signed char TMap::scroll_left(signed short draw)
{
	signed char moved = 0;

	if (view_x > 16) {
		view_x--;
		moved = 1;
		if (--fine_x < 0) {
			fine_x = 15;
			cur -= 16;
			tile_x -= 16;
		}
		if (draw == 1)
			tmap_blit_rows(scr + view_x, rows, fine_y);
		if (fine_x == 15)
			load_col(-16, 9);
		else if (fine_x == 14)
			load_col(-16, 6);
		else if (fine_x == 13)
			load_col(-16, 3);
		else if (fine_x == 12)
			load_col(-16, 0);
	}
	return moved;
}

signed char TMap::scroll_down(signed short draw)
{
	unsigned char c;
	signed char moved = 0;
	signed short i;

	if (view_y < 0x1a3) {
		moved = 1;
		view_y++;
		if (++fine_y > 19) {
			c = rows[0];
			for (i = 0; i < 11; i++)
				rows[i] = rows[i + 1];
			rows[11] = c;
			fine_y = 0;
			tile_y += 20;
		}
		if (draw == 1)
			tmap_blit_rows(scr + view_x, rows, fine_y);
		if (fine_y == 0)
			load_row(scr + tile_x + c * 0x1680L - 16, 200);
	}
	return moved;
}

signed char TMap::scroll_up(signed short draw)
{
	unsigned char c;
	signed char moved = 0;
	signed short i;

	if (view_y > 20) {
		view_y--;
		moved = 1;
		if (--fine_y < 0) {
			c = rows[11];
			for (i = 11; i > 0; i--)
				rows[i] = rows[i - 1];
			rows[0] = c;
			fine_y = 19;
			tile_y -= 20;
		}
		if (draw == 1)
#if FIX_0176
			/* Fix #176: gleicher Versatz view_x wie in den anderen Richtungen */
			tmap_blit_rows(scr + view_x, rows, fine_y);
#else
			tmap_blit_rows(scr + tile_x, rows, fine_y);
#endif
		if (fine_y == 19)
			load_row(scr + tile_x + c * 0x1680L - 16, -40);
	}
	return moved;
}

signed char TMap::plot(signed short x, signed short y, unsigned char c)
{
	signed long off;
	signed char old = 0;
	char huge *p;
	signed short t;

	y -= tile_y - 20;
	x -= tile_x;
	if (y > 0 && y < 240 && x > -16 && x < 256) {
		t = y / 20;
		t = rows[t];
		y %= 20;
		off = (signed long)t * 0x1680 + (signed long)y * 0x120 + x;
		p = (char huge *)EMS_norm_ptr((char huge *)scr + tile_x + off);
		old = *p;
		*p = c;
	}
	return old;
}

signed char TMap::follow(signed short x, signed short y, signed char steps, unsigned char c)
{
	signed char k;
	signed char r = 0;
	signed short nx;
	signed short ny;

	if (x != -1 && y != -1) {
		x += 16;
		nx = x;
		y += 20;
		ny = y;
		do {
			if (last_x < x) {
				if (view_x + 120 < x)
					for (k = 0; k < steps; k++)
						r = scroll_right(0);
				x--;
			} else if (last_x > x) {
				if (view_x + 120 > x)
					for (k = 0; k < steps; k++)
						r = scroll_left(0);
				x++;
			}
			if (last_y < y) {
				if (view_y + 100 < y)
					for (k = 0; k < steps; k++)
						r = scroll_down(0);
				y--;
			} else if (last_y > y) {
				if (view_y + 100 > y)
					for (k = 0; k < steps; k++)
						r = scroll_up(0);
				y++;
			}
		} while (abs(last_x - x) > 1 && abs(last_y - y) > 1 && last_x != -1 && last_y != -1);
		if (c != 0)
			r = plot(nx, ny, c);
		last_x = nx;
		last_y = ny;
	}
	return r;
}

void TMap::plot_points(signed short far *p, unsigned char c)
{
	while (*p != -1) {
		plot(p[0] + 16, p[1] + 20, c);
		p += 2;
	}
}

void TMap::mark1_init(signed short x, signed short y)
{
	signed short w;
	signed char i;

	for (i = 0; i < 4; i++) {
		mark1_bufs[i] = mem_alloc(0x51L);
		nvfile_load(g_nvf_heads, FILE_CIRCLE, mark1_bufs[i], (void far *)0, i, 0, &w, &w, 0x140);
		mark1_pics[i].init(9, 9, mark1_bufs[i]);
	}
	mark1_bg_buf = mem_alloc(0x51L);
	mark1_bg.init(9, 9, mark1_bg_buf);
	mark1_bg.get(g_vga_buf, x, y);
}

void TMap::mark2_init(signed short x, signed short y)
{
	signed short w;
	signed char i;

	for (i = 0; i < 4; i++) {
		mark2_bufs[i] = mem_alloc(0x51L);
		nvfile_load(g_nvf_heads, FILE_CIRCLE, mark2_bufs[i], (void far *)0, i + 5, 0, &w, &w, 0x140);
		mark2_pics[i].init(9, 9, mark2_bufs[i]);
	}
	mark2_bg_buf = mem_alloc(0x51L);
	mark2_bg.init(9, 9, mark2_bg_buf);
	mark2_bg.get(g_vga_buf, x, y);
}

void TMap::mark1_free()
{
	signed char i;

	for (i = 0; i < 4; i++) {
		mark1_pics[i].free_lines();
		mem_free(mark1_bufs[i]);
	}
	mark1_bg.free_lines();
	mem_free(mark1_bg_buf);
	g_tmap_mark1_tick = 0;
}

void TMap::mark2_free()
{
	signed char i;

	for (i = 0; i < 4; i++) {
		mark2_pics[i].free_lines();
		mem_free(mark2_bufs[i]);
	}
	mark2_bg.free_lines();
	mem_free(mark2_bg_buf);
	g_tmap_mark2_tick = 0;
}

void TMap::mark1_anim(signed short x, signed short y)
{
	signed short k;
	signed short hide = 0;
	signed char d = 2;
	signed short mx = g_travel_rose_x;
	signed short my = g_travel_rose_y;
	signed short sx;
	signed short sy;

	sx = x - view_x + 12;
	sy = y - view_y + 16;
	if (mouse_overlaps_rect(sx - 5, sy - 5, sx + 5, sy + 5)) {
		hide = 1;
		mouse_hide(0);
	}
	if (g_tmap_mark1_tick == 0)
		mark1_init(sx, sy);
	if (g_tmap_mark1_tick % d == 0)
		mark1_bg.put(g_vga_buf, sx, sy);
	if (g_tmap_mark1_tick >= 120)
		g_tmap_mark1_tick = 20;
	k = g_tmap_mark1_tick % (d * 4);
	k = k / d;
	if (g_blink_off == 0 && !dlg_overlaps(g_dialog, sx - 5, sy - 5, sx + 5, sy + 5) &&
	    (sx - 5 <= mx || sx + 5 >= mx + 50 || sy - 5 <= my || sy + 5 >= my + 50) && sx < 0xee)
		mark1_pics[k].put(g_vga_buf, sx, sy, BLIT_MASKED);
	if (hide)
		mouse_show(0);
	g_tmap_mark1_tick++;
}

void TMap::mark2_anim(signed short x, signed short y)
{
	signed short k;
	signed short hide = 0;
	signed char d = 2;
	signed short mx = g_travel_rose_x;
	signed short my = g_travel_rose_y;
	signed short sx;
	signed short sy;

	sx = x - view_x + 12;
	sy = y - view_y + 16;
	if (mouse_overlaps_rect(sx - 5, sy - 5, sx + 5, sy + 5)) {
		hide = 1;
		mouse_hide(0);
	}
	if (g_tmap_mark2_tick == 0)
		mark2_init(sx, sy);
	if (g_tmap_mark2_tick % d == 0)
		mark2_bg.put(g_vga_buf, sx, sy);
	if (g_tmap_mark2_tick >= 120)
		g_tmap_mark2_tick = 20;
	k = g_tmap_mark2_tick % (d * 4);
	k = k / d;
	if (g_blink_off == 0 && !dlg_overlaps(g_dialog, sx - 5, sy - 5, sx + 5, sy + 5) &&
	    (sx - 5 <= mx || sx + 5 >= mx + 50 || sy - 5 <= my || sy + 5 >= my + 50) && sx < 0xee)
		mark2_pics[k].put(g_vga_buf, sx, sy, BLIT_MASKED);
	if (hide)
		mouse_show(0);
	g_tmap_mark2_tick++;
}
