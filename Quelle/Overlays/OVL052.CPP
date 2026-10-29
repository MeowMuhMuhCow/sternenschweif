/* OVL052 - Overlay O052 (Segment 0x2031 in SCHWEIF.EXE): Automap - Kartenobjekt class Automap (s_class.h; 0x28d Bytes,
 * Instanz g_map ds:be36, weitere Methoden in O053: nop leer, travel = Reisen zum Zielfeld x/y). Benutzt von O014/O032/
 * O046 (Dungeon/Stadt), O053 (init -> load_cells) und O106 (Spielstand, Block "AMAP"). Felder siehe s_class.h:
 *   Texte txt_buf/txt (0x21e), Bild-Element pic (O269 Pic), Kartenbild img (64000 Bytes), Kacheln tiles, Notiztext note
 *   (0x200), Kachelindex tile_idx (256 Bytes: Feldtyp -> laufende Kachelnummer), Schriftfarben old_fg/old_bg, w/h,
 *   ntiles, zoom (1/2/4 = Felder je Kachel), tile_size = tw * th, Ausschnitt vx/vy, Cursor cx/cy, Feldinfo cinfo,
 *   Ausschnitt in Pixeln vw/vh, Bildversatz ox/oy, letzte Cursorposition last_x/last_y, geladene Aufdeckkarte seen_id,
 *   Speicherblock handle (XMS ueber xms_alloc_kb/xms_move_any, EMS ueber ems_alloc/EMS_copy bei ems == 1), is_open,
 *   see_through[5] (set_town_type), Merker swapped (swap_save_file/swap_alloc), arc_marked (arc_mark/arc_release), ems,
 *   Gruppenmarke gx/gy, andere Gruppen other_x/other_y[6], Cursor in Pixeln cur_x/cur_y, Karte cells (2 Bytes je Feld:
 *   Typ, Info), Bmp-Zeiger mark_own[6]/mark_other[6] (Animationsbilder eigene/andere Gruppe, Bild 0x1a3 Nr. 12..17/
 *   18..23), bg_own/bg_other[6] (Hintergrund unter den Marken), bg_cursor (unter dem Cursor), b173, Aufdeckkarte seen
 *   (64 x 64, signed char), Ziel x/y, f17b, Notizdatei note_file, Puffer buf (256), Notizfeld edit (O110/O111),
 *   Notizsatz rec (x, y, Text 0xfc), gesicherte Mausbereichsliste saved_areas.
 * Speicherblock handle: Hintergrundbild (64000 Bytes) und ab 0xfa00 100 Notizsaetze zu 256 Bytes (Kopierauftrag
 *   g_xms_move).
 * Automap(): seen_id = -1, Aufdeckkarte new 0x1000 genullt, Zoom 1. ~Automap(): close bei is_open == 1, delete seen.
 * open(karte, notizen, grafik): Speicher (xms_free_kb <= 90 KB -> auslagern swap_save_file, XMS 90 KB, sonst EMS 6
 *   Seiten bei g_ems_avail == 1), Hintergrund load_bg, Texte, Puffer, Palette aus Archiv 0x406 (Stadt 3: 0x1c0e,
 *   sonst Dungeon * 0x31e, + 0x1e; 160 Farben & 0x3f), Notizen leeren/laden, Karte, Kacheln, Bmps 16/zoom, Mausrueckruf
 *   cursor_for_cell (g_mouse_handler_areas/g_mouse_handler/g_mouse_handler_on), Schriftfarben 0xff/0xcf, Edit(4, 0xc3,
 *   0x4f, 9), Mausbereiche g_mouse_areas = g_amap_areas, mouse_clear_left.
 * close: alles freigeben, Notizen schreiben, Speicher zurueck, g_refresh = 0. load_cells(id): g_ofile-Datei (w, h,
 *   Karte w*h*2). load_tiles(id): Kachelzahl aus dem Archiv (swap_u16 & 0xff), in Staedten (AREA_TOWN) die
 *   vorkommenden Feldtypen markieren, sonst alle; load_tile_imgs. make_bmps. load_tile_imgs(id): Kachelbilder
 *   (nvfile_load, g_nvf_heads), bei Zoom > 1 verkleinert (Bmp::scale) per memcpy, Kachelindex umnummerieren.
 * scroll_up/_down/_right/_left: Ausschnitt verschieben und neue Zeile/Spalte zeichnen. set_town_type(v). center
 *   (Ausschnitt um den Cursor, 18 x 9 Kacheln). show (Palette, blit, Edit). draw (ganze Karte ins Kartenbild).
 *   reveal_all: alle 64 x 64 Felder per reveal_map. load_bg: Bild 0x222 (g_pp20_index) mit Kompassrose (0x411,
 *   Richtung aus g_cam_angle) in den Speicherblock. blit(src, h): Kartenbild mit Durchsicht (0) ueber den Hintergrund
 *   aus dem Speicherblock auf den Schirm g_vga_buf (25 x 0xa00 Bytes).
 * load_seen: Aufdeckkarte sichern/laden (Datei 0x407, je 0x1000 Bytes; Stadt: Nr. bzw. 0x13, Dungeon: Nr. + 10,
 *   0x19 -> 0x18). save / load: Spielstandblock "AMAP", Version 2, Zoom.
 * run: Eingabe (poll_input, handle_options_key, wait_for_vsync), animate, Menue dlg_choose (Texte 0x1e..0x23,
 *   g_fmt_col4_* bei Zoomgrenze), Aktionen g_action: 0x80..0x83 Ausschnitt, g_key_ascii 0x2d/0x2b Zoom -/+ (Bmps neu,
 *   load_tile_imgs), Pfeile Cursor, Tab/Einfg Notiz, Enter Reisen (travel, nur bekannte Felder ausser der eigenen
 *   Position), Entf Notiz loeschen, Mausklick g_mouse_ldown (g_mouse_posx/posy) Cursor setzen, g_mouse_ldclick
 *   (g_mouse_clickx/clicky) Reisen.
 * frame (Kartenbild-Ausschnitt auf den Schirm). draw_tile(x, y, t) (g_mem_debug aus). cursor_right/_down/_left/_up.
 *   update_cursor (Rahmen do_border, Notiztext ins Edit-Feld, cinfo).
 * edit_note (nur bekannte Felder). animate (alle 6 Takte: Marken der eigenen und anderer Gruppen g_groups[6] auf
 *   derselben Karte bzw. Stadt 3 <-> Dungeon 9). erase_marks/erase_cursor: Marken/Cursor loeschen.
 * clear_notes. del_note (nur bei Feldinfo 0). put_note/get_note(satz, i) (XMS/EMS). find_note -> Text oder 0.
 *   find_slot (Notiz am Cursor, sonst freier Satz, sonst -1). save_note (voll: Text 0x24). load_notes/save_notes:
 *   Notizen aus der Datei note_file laden/schreiben.
 *
 * BCC -mlarge -3 -Ot -Og -Oi -c -Y OVL052.CPP   -> 14210/14210 Bytes GLEICH (19.10.2026), cobjfix laeuft
 *   (Namensfassung mit schweif.h: 19.10.2026 GLEICH, LINKGLEICH)
 * Stolpersteine:
 * - Zeiger + Zeilenrechnung: nur "img + (oy + 48) * 320 + ox + 16" (Konstante wird distributiv herausgezogen, der
 *   Index ist dann eine Summe) ergibt imul / add ax,[scr] / push bx / push es. "img + oy * 320 + ox + 0x3c10" laedt
 *   den Zeiger zuerst (push/pop ums imul), mit (unsigned)-Cast mov dx,[img] / add dx,ax.
 * - scroll_down: "seen[(unsigned)(zoom * 9 + vy - 1)]" (und derselbe Cast im Kartenindex): sonst faltet BCC das -1 in die
 *   Verschiebung ([bx-40h]) und -Og legt nur zoom*9 ab; mit Cast Zeile als Zwischenwert [bp-16h], Index zuerst.
 * - "new Bmp(n, n = 16 / zoom)" (mov di,ax / push ax / push di bzw. [bp-4]/[bp-0ch]).
 * - "tile_idx[i] = k++" laedt k als Wort VOR der Zieladresse, inc erst nach dem Store.
 * - "memset(tile_idx, 1, ntiles = n)" -> mov cx,ax. "if ((vw = zoom * 18) > w)", "vw *= tw / zoom" (es:bx bleibt).
 * - "if (!(c = *src++))" (signed char -> cbw). -Og-Zwischenwerte (min-Makro, Zeilen) entstehen von selbst.
 * - font_set_colors(g_font, old_fg, old_bg) ueber gecasteten Funktionszeiger (Prototyp: unsigned char).
 * - Palette g_pal_buf (unsigned char far *) als struct rgb ueber den Makro PAL.
 * Originalfehler/Auffaelliges: open() ohne Fehlerbehandlung, wenn weder XMS noch EMS klappt (Handle <= 0 wird
 *   trotzdem benutzt); swap_save_file kann zweimal laufen (kein Test auf swapped). edit_note/del_note leeren nur 100 der 0xfc
 *   Textbytes (Rest wird mitgespeichert). del_note ohne Notiz am Cursor "loescht" den ersten freien Satz. close() setzt
 *   die freigegebenen Zeiger img/tiles/note/tile_idx/cells nicht zurueck (nur txt_buf/txt).
 */
#include <mem.h>
#include <stdio.h>
#include <io.h>
#include "schweif.h"

/* Phase B: eigene Daten des Moduls (ds:5284..531c, dsana) - initialisierte Daten in Adressreihenfolge, danach der Literalpool */
char g_amap_areas[70] = {
	0x00, 0x00, 0x14, 0x00, 0x0f, 0x00, 0xbb, 0x00, 0x80, 0x00, 0x30, 0x01, 0x14, 0x00, 0x3f, 0x01,
	0xbb, 0x00, 0x81, 0x00, 0x10, 0x00, 0x20, 0x00, 0x30, 0x01, 0x30, 0x00, 0x82, 0x00, 0x10, 0x00,
	0x00, 0x00, 0x30, 0x01, 0x0a, 0x00, 0x82, 0x00, 0x10, 0x00, 0xc1, 0x00, 0x30, 0x01, 0xc7, 0x00,
	0x83, 0x00, 0x4c, 0x00, 0x08, 0x00, 0x19, 0x01, 0x2a, 0x00, 0x09, 0x0f, 0xff, 0xff, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
char g_amap_handler_areas[70] = {
	0x00, 0x00, 0x14, 0x00, 0x0f, 0x00, 0xbb, 0x00, 0x80, 0x00, 0x30, 0x01, 0x14, 0x00, 0x3f, 0x01,
	0xbb, 0x00, 0x81, 0x00, 0x10, 0x00, 0x20, 0x00, 0x30, 0x01, 0x30, 0x00, 0x82, 0x00, 0x10, 0x00,
	0x00, 0x00, 0x30, 0x01, 0x0a, 0x00, 0x82, 0x00, 0x10, 0x00, 0xc1, 0x00, 0x30, 0x01, 0xc7, 0x00,
	0x83, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0x01, 0xc7, 0x00, 0x84, 0x00, 0xff, 0xff, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
signed short g_reveal_by_spell = 0;
/* Phase B: eigenes BSS (ds:c800..c806) in Adressreihenfolge */
signed short g_amap_gfx;
signed short g_amap_tick;
signed short g_amap_frame;

#define PAL ((struct rgb far *)g_pal_buf)	/* Palettenpuffer als RGB-Tripel */
#define NVF_HEADS ((Nvf far *)g_nvf_heads)	/* NVF-Dateiobjekt (O098) */
#define MAP_GRID ((struct map_grid far *)&g_map_w)	/* Kartenobjekt ds:b656 (struct map_grid; erstes Feld g_map_w) */
#define MIN(a, b) ((a) < (b) ? (a) : (b))

Automap::Automap()
{
	seen_id = -1;
	seen = new signed char[64][64];
	memset(seen, 0, 0x1000);
	txt_buf = 0;
	txt = 0;
	zoom = 1;
}

Automap::~Automap()
{
	if (is_open == 1)
		close();
	delete seen;
}

void Automap::open(signed short mapid, signed short nid, signed short gfx)
{
	signed short hd;
	signed long kb = 0;
	signed long off;
	signed short i;
	signed short n;

	swapped = 0;
	kb = xms_free_kb();
	if (kb <= 0x5a) {
		swapped = 1;
		swap_save_file();
		if (arc_marked == 0) {
			arc_mark(g_file_3);
			arc_marked = 1;
		}
	}
	handle = xms_alloc_kb(0x5a);
#if FIX_0208
	/* Fix #208: Archiv nicht ein zweites Mal auslagern */
	if (handle <= 0) {
		if (!swapped)
			swap_save_file();
		swapped = 1;
#else
	if (handle <= 0) {
		swapped = 1;
		swap_save_file();
#endif
		if (arc_marked == 0) {
			arc_mark(g_file_3);
			arc_marked = 1;
		}
		handle = xms_alloc_kb(0x5a);
	}
	if (handle <= 0 && g_ems_avail == 1) {
		ems = 1;
		ems_alloc(6, &hd);
		g_ems_frame = ems_frame_seg();
		handle = hd;
	}
	load_bg();
	g_amap_gfx = gfx;
	if (txt_buf == 0)
		txt_load_list(g_font, 0x21e, 0x2d, &txt_buf, &txt);
	img = (unsigned char far *)mem_alloc(0xfa00L);
	note = (char far *)mem_alloc(0x200L);
	tile_idx = (unsigned char far *)mem_alloc(0x100L);
	tiles = 0;
	last_x = last_y = g_cursor_no = -1;
	is_open = 0;
	note_file = nid;
	arc_open(g_file_3, 0x406);
	if (g_area_type == AREA_TOWN) {
		if (g_group->town == 3)
			off = 0x1c0e;
		else
			off = 0;
	} else {
		i = g_group->dungeon;
		off = i * 0x31eL;
	}
	arc_seek(g_file_3, off + 0x1e, 0);
	arc_read(g_file_3, g_pal_buf, 0x1e0L);
	arc_close(g_file_3);
	for (i = 0; i < 0xa0; i++) {
		PAL[i].r = PAL[i].r & 0x3f;
		PAL[i].g = PAL[i].g & 0x3f;
		PAL[i].b = PAL[i].b & 0x3f;
	}
	clear_notes();
	load_notes();
	gx = cur_x = x = f17b = -1;
	for (i = 0; i < 6; i++)
		other_x[i] = -1;
	load_cells(mapid);
	load_tiles(gfx);
	make_bmps();
	cx = g_group->x;
	cy = g_group->y;
	center();
	bg_cursor = new Bmp(n, n = 16 / zoom);
	b173 = new Bmp(n, n = 16 / zoom);
	g_mouse_handler_areas = g_amap_handler_areas;
	g_mouse_handler = cursor_for_cell;
	g_mouse_handler_on = 1;
	font_get_colors(g_font, &old_fg, &old_bg);
	font_set_colors(g_font, 0xff, 0xcf);
	edit = new Edit(4, 0xc3, 0x4f, 9, 0, 0, 0);
	saved_areas = g_mouse_areas;
	g_mouse_areas = g_amap_areas;
	g_mouse_ldclick = 0;
	mouse_clear_left();
	buf = new char[0x100];
	is_open = 1;
}

void Automap::close()
{
	signed short i;

	g_mouse_handler_on = 0;
	for (i = 0; i < 6; i++) {
		delete mark_own[i];
		delete mark_other[i];
	}
	delete bg_own;
	delete bg_cursor;
	delete b173;
	for (i = 0; i < 6; i++)
		delete bg_other[i];
	delete edit;
	delete buf;
	save_notes();
#if FIX_0208
	/* Fix #208: freigegebene Zeiger zuruecksetzen (mem_free liefert 0) */
	tile_idx = (unsigned char far *)mem_free(tile_idx);
	note = (char far *)mem_free(note);
	img = (unsigned char far *)mem_free(img);
	cells = (struct map_tile far *)mem_free(cells);
	tiles = (unsigned char far *)mem_free(tiles);
#else
	mem_free(tile_idx);
	mem_free(note);
	mem_free(img);
	mem_free(cells);
	mem_free(tiles);
#endif
	if (txt_buf != 0) {
		txt_buf = mem_free(txt_buf);
		txt = (char far * far *)mem_free(txt);
	}
	if (ems == 1)
		ems_free(handle);
	else
		xms_free_block(handle);
	handle = 0;
	ems = 0;
	if (arc_marked == 1) {
		arc_release(g_file_3);
		arc_marked = 0;
	}
	if (swapped != 0) {
		swap_alloc();
		swapped = 0;
	}
	((void (*)(void far *, signed short, signed short))font_set_colors)(g_font, old_fg, old_bg);
	mouse_set_cursor(0, 0);
	g_mouse_areas = saved_areas;
	is_open = 0;
	g_refresh = 0;
}

void Automap::load_cells(signed short id)
{
	signed long n;

	ofile_open(g_ofile, id);
	ofile_read(g_ofile, &w, 2L);
	ofile_read(g_ofile, &h, 2L);
	n = w * h * sizeof(struct map_tile);
	cells = (struct map_tile far *)mem_alloc(n);
	ofile_read(g_ofile, cells, n);
	ofile_close(g_ofile);
}

void Automap::load_tiles(signed short id)
{
	signed short n;
	unsigned char t;
	signed short i;

	arc_open(g_file_3, id);
	arc_read(g_file_3, &n, 2L);
	n = swap_u16(n);
	n &= 0xff;
	arc_close(g_file_3);
	if (g_area_type == AREA_TOWN) {
		*tile_idx = 1;
		ntiles = 1;
		for (i = 0; w * h > i; i++) {
			t = cells[i].type;
			if (!tile_idx[t])
				ntiles++;
			tile_idx[t] = 1;
		}
	} else {
		memset(tile_idx, 1, ntiles = n);
	}
	load_tile_imgs(id);
}

void Automap::make_bmps()
{
	signed short dummy;
	signed short n;
	signed short i;

	bg_own = new Bmp(n, n = 16 / zoom);
	for (i = 0; i < 6; i++)
		bg_other[i] = new Bmp(n, n = 16 / zoom);
	for (i = 0; i < 6; i++) {
		mark_own[i] = new Bmp(16, 16);
		NVF_HEADS->load(0x1a3, mark_own[i]->data(), 0, 0, i + 12, 0, &dummy, &dummy, 0x140);
		mark_other[i] = new Bmp(16, 16);
		NVF_HEADS->load(0x1a3, mark_other[i]->data(), 0, 0, i + 18, 0, &dummy, &dummy, 0x140);
	}
}

void Automap::load_tile_imgs(signed short id)
{
	signed short i;
	signed short k;
	signed short dummy;
	unsigned char far *p;
	signed short sz;

	*tile_idx = 1;
	nvfile_dims(g_nvf_heads, id, &tw, &th);
	tile_size = tw * th;
	if (tiles == 0)
		tiles = (unsigned char far *)mem_alloc((signed long)ntiles * (signed long)tile_size);
	if (zoom == 1) {
		for (i = k = 0, p = tiles; i < 0x100; i++) {
			if (tile_idx[i] != 0) {
				NVF_HEADS->load(id, p, 0, 0, i, 0, &dummy, &dummy, 0x140);
				p += tile_size;
				tile_idx[i] = k++;
			}
		}
	} else {
		sz = (tw / zoom) * (th / zoom);
		Bmp big(tw, th);
		Bmp small(tw / zoom, th / zoom);
		for (i = k = 0, p = tiles; i < 0x100; i++) {
			if (tile_idx[i] != 0) {
				NVF_HEADS->load(id, big.data(), 0, 0, i, 0, &dummy, &dummy, 0x140);
				big.scale(&small);
				memcpy(p, small.data(), sz);
				p += tile_size;
				tile_idx[i] = k++;
			}
		}
	}
}

void Automap::scroll_up()
{
	signed short n;
	signed short hh;
	struct map_tile far *p;
	signed short i;

	erase_marks();
	erase_cursor();
	mouse_hide(0);
	hh = th / zoom;
	Bmp b(0x140, 1, 0);
	for (i = th * 9 - hh + 0x2f; i >= 0x30; i--) {
		b.set_base(img + i * 0x140);
		b.put(img, 0, i + hh);
	}
	mouse_show(0);
	n = MIN(w, zoom * 18);
	for (i = 0; i < n; i++) {
		p = cells + vy * w + vx + i;
		draw_tile(i * tw, 0, *(seen[vy] + vx + i) ? p->type : 0);
	}
	show();
	last_x = cur_x = -1;
}

void Automap::scroll_down()
{
	signed short n;
	struct map_tile far *p;
	signed short i;

	Bmp b(0x140, 0xc8, img + (oy + th / zoom + 48) * 320 + ox + 16);
	erase_marks();
	erase_cursor();
	mouse_hide(0);
	b.put_part(img, ox + 16, oy + 48, vw, vh - th / zoom);
	mouse_show(0);
	n = MIN(w, zoom * 18);
	for (i = 0; i < n; i++) {
		p = cells + (unsigned)(zoom * 9 + vy - 1) * w + vx + i;
		draw_tile(i * tw, (zoom * 9 - 1) * th, *(seen[(unsigned)(zoom * 9 + vy - 1)] + vx + i) ? p->type : 0);
	}
	show();
	last_x = cur_x = -1;
}

void Automap::scroll_right()
{
	signed short n;
	struct map_tile far *p;
	signed short i;

	Bmp b(0x140, 0xc8, img + (oy + 48) * 320 + ox + tw / zoom + 16);
	erase_marks();
	erase_cursor();
	mouse_hide(0);
	b.put_part(img, ox + 16, oy + 48, vw - tw / zoom, vh);
	mouse_show(0);
	n = MIN(h, zoom * 9);
	for (i = 0; i < n; i++) {
		p = cells + (vy + i) * w + vx + zoom * 18 - 1;
		draw_tile((zoom * 18 - 1) * tw, i * th, *(seen[vy + i] + vx + zoom * 18 - 1) ? p->type : 0);
	}
	show();
	last_x = cur_x = -1;
}

void Automap::scroll_left()
{
	signed short n;
	struct map_tile far *p;
	signed short i;

	Bmp b(0x140, 0xc8, img + (oy + 48) * 320 + ox + 12);
	erase_marks();
	erase_cursor();
	mouse_hide(0);
	b.put_part(img, ox + tw / zoom + 12, oy + 48, vw - tw / zoom, vh, 0, 0, 1);
	mouse_show(0);
	n = MIN(h, zoom * 9);
	for (i = 0; i < n; i++) {
		p = cells + (vy + i) * w + vx;
		draw_tile(0, i * th, *(seen[vy + i] + vx) ? p->type : 0);
	}
	show();
	last_x = cur_x = -1;
}

void Automap::set_town_type(signed short v)
{
	memset(see_through, -1, sizeof(see_through));
	if (v == 1)
		see_through[0] = 14;
	else if (v == 2)
		see_through[0] = 15;
	else if (v == 6)
		see_through[0] = 16;
	else if (v == 5) {
		see_through[0] = 16;
		see_through[1] = 0x16;
		see_through[2] = 0x17;
		see_through[3] = 0x18;
	}
}

void Automap::center()
{
	vx = cx - zoom * 19 / 2;
	if (vx + zoom * 18 > w)
		vx = w - zoom * 18;
	if (vx < 0)
		vx = 0;
	if ((vw = zoom * 18) > w)
		vw = w;
	vw *= tw / zoom;
	vy = cy - zoom * 10 / 2;
	if (vy + zoom * 9 > h)
		vy = h - zoom * 9;
	if (vy < 0)
		vy = 0;
	if ((vh = zoom * 9) > h)
		vh = h;
	vh *= th / zoom;
}

void Automap::show()
{
	set_palette(g_pal_buf, 0, 0xa0);
	mouse_hide(0);
	blit((char far *)img, handle);
	edit->settext(note);
	mouse_show(0);
	g_automap_redraw = 0;
}

void Automap::draw()
{
	signed short yy;
	signed short xx;
	signed short nh;
	signed short nw;
	struct map_tile far *p;

	gx = cur_x = -1;
	for (yy = 0; yy < 6; yy++)
		other_x[yy] = -1;
	memset(img, 0, 64000U);
	nh = MIN(h, zoom * 9);
	nw = MIN(w, zoom * 18);
	ox = (0x120 - nw * tw / zoom) >> 1;
	oy = (0x90 - nh * th / zoom) >> 1;
	for (yy = 0; yy < nh; yy++)
		for (xx = 0; xx < nw; xx++) {
			p = cells + (vy + yy) * w + vx + xx;
			draw_tile(xx * tw, yy * th, *(seen[vy + yy] + vx + xx) ? p->type : 0);
		}
}

void Automap::reveal_all()
{
	signed short xx;
	signed short yy;

	for (yy = 0; yy < 64; yy++)
		for (xx = 0; xx < 64; xx++)
			reveal_map(this, xx, yy);
}

void Automap::load_bg()
{
	signed short dummy;
	signed short k;
	void far *buf;

	Bmp b(0x2d, 0x29);
	buf = mem_alloc(0xfa00L);
	pic.loadf(g_pp20_index = 0x222, buf, PAL);
	k = ((g_cam_angle + 0x100) / 0x80) & 7;
	NVF_HEADS->load(0x411, b.data(), 0, 0, k, 0, &dummy, &dummy, 0x140);
	b.put(buf, 5, 4, 4);
	g_xms_move.len = 0xfa00L;
	g_xms_move.src_handle = 0;
	g_xms_move.src = (unsigned long)buf;
	g_xms_move.dst_handle = handle;
	g_xms_move.dst = 0;
	if (!ems)
		xms_move_any(&g_xms_move);
	else
		EMS_copy(&g_xms_move);
	mem_free(buf);
}

void Automap::blit(char far *src, signed long hd)
{
	signed char c;
	signed long off;
	unsigned char far *buf;
	unsigned char far *d = (unsigned char far *)g_vga_buf;
	signed short i;
	signed short j;

	buf = new unsigned char[0xa00];
	for (i = 0, off = 0; i < 25; i++, off += 0xa00) {
		g_xms_move.len = 0xa00;
		g_xms_move.src_handle = hd;
		g_xms_move.src = off;
		g_xms_move.dst_handle = 0;
		g_xms_move.dst = (unsigned long)buf;
		if (!ems)
			xms_move_any(&g_xms_move);
		else
			EMS_copy(&g_xms_move);
		for (j = 0; j < 0xa00; j++, d++) {
			if (!(c = *src++))
				*d = buf[j];
			else
				*d = c;
		}
	}
	delete buf;
}

void Automap::load_seen()
{
	signed long off;
	signed short n;

	if (seen_id != -1) {
		off = (signed long)seen_id << 12;
		ofile_create(g_ofile, 0x407, 0x8002);
		ofile_seek(g_ofile, off, 0);
		ofile_write(g_ofile, seen, 0x1000L);
		ofile_close(g_ofile);
		seen_id = -1;
	}
	if (g_area_type == AREA_TOWN) {
		if (g_group->town == 3)
			n = 0x13;
		else
			n = g_group->town;
	} else
		n = g_group->dungeon + 10;
	if (n == 0x19)
		n = 0x18;
	off = (signed long)n << 12;
	ofile_open(g_ofile, 0x407);
	ofile_seek(g_ofile, off, 0);
	ofile_read(g_ofile, seen, 0x1000L);
	ofile_close(g_ofile);
	seen_id = n;
}

signed short Automap::save()
{
	signed short v = 2;
	signed short z;

	_write(g_save_fd, "AMAP", 4);
	_write(g_save_fd, &v, 2);
	z = zoom;
	_write(g_save_fd, &z, 2);
	return 1;
}

signed short Automap::load()
{
	signed long magic;
	signed short v;
	signed short z;

	_read(g_save_fd, &magic, 4);
	if (magic == 0x50414d41L) {
		_read(g_save_fd, &v, 2);
		_read(g_save_fd, &z, 2);
		zoom = z;
		seen_id = -1;
		return 1;
	} else
		return 0;
}

void Automap::run()
{
	char far *s2;
	char far *s1;
	signed short r;
	signed short n;
	signed short i;

	poll_input();
	handle_options_key();
	wait_for_vsync();
	animate();
	if (g_mouse_rclick != 0 || g_key == 0x4900) {
		if (zoom == 4) {
			sprintf(g_text_output_buf, "\xf4%s\xf0", txt[0x21]);
			s1 = g_text_output_buf;
		} else
			s1 = txt[0x21];
		if (zoom == 1) {
			sprintf(g_text_output_buf, g_fmt_col4_531c, txt[0x20]);
			s2 = g_text_output_buf;
		} else
			s2 = txt[0x20];
		r = dlg_choose(g_dialog, txt[0x1e], 5, txt[0x1f], s2, s1, txt[0x23], txt[0x22]);
		if (r == 1)
			g_action = 0xf09;
		else if (r == 2)
			g_action = g_key_ascii = 0x2b;
		else if (r == 3)
			g_action = g_key_ascii = 0x2d;
		else if (r == 4)
			g_action = 0x1c0d;
		else if (r == 5)
			g_action = 0x11b;
	}
	if (g_action == 0x80) {
		if (--vx < 0)
			vx = 0;
		else
			scroll_left();
	} else if (g_action == 0x81) {
		if (w - zoom * 18 < ++vx)
			vx--;
		else
			scroll_right();
	} else if (g_action == 0x82) {
		if (--vy < 0)
			vy = 0;
		else
			scroll_up();
	} else if (g_action == 0x83) {
		if (h - zoom * 9 < ++vy)
			vy--;
		else
			scroll_down();
	}
	if (g_key_ascii == 0x2d) {
		zoom += zoom;
		if (zoom > 4)
			zoom = 4;
		else {
			g_refresh = 1;
			center();
			delete bg_cursor;
			bg_cursor = new Bmp(n, n = 16 / zoom);
			delete b173;
			b173 = new Bmp(n, n = 16 / zoom);
			delete bg_own;
			bg_own = new Bmp(n, n = 16 / zoom);
			for (i = 0; i < 6; i++) {
				delete bg_other[i];
				bg_other[i] = new Bmp(n, n = 16 / zoom);
			}
			f17b = -1;
			load_tile_imgs(g_amap_gfx);
		}
	} else if (g_key_ascii == 0x2b) {
		signed short old = zoom;
		zoom -= zoom >> 1;
		if (zoom == old)
			zoom = 1;
		else {
			g_refresh = 1;
			center();
			delete bg_cursor;
			bg_cursor = new Bmp(n, n = 16 / zoom);
			delete b173;
			b173 = new Bmp(n, n = 16 / zoom);
			delete bg_own;
			bg_own = new Bmp(n, n = 16 / zoom);
			for (i = 0; i < 6; i++) {
				delete bg_other[i];
				bg_other[i] = new Bmp(n, n = 16 / zoom);
			}
			f17b = -1;
			load_tile_imgs(g_amap_gfx);
		}
	} else if ((g_action & 0xff00) == 0x4d00)
		cursor_right();
	else if ((g_action & 0xff00) == 0x4b00)
		cursor_left();
	else if ((g_action & 0xff00) == 0x5000)
		cursor_down();
	else if ((g_action & 0xff00) == 0x4800)
		cursor_up();
	else if (g_action == 0xf09 || g_action == 0x5200)
		edit_note();
	else if (g_action == 0x1c0d) {
		x = cx;
		y = cy;
		if (!!seen[y][x] && (x != g_group->x || y != g_group->y))
			travel();
		else
			x = y = -1;
	} else if (g_action == 0x5300)
		del_note();
	else if (g_mouse_ldown != 0 && is_mouse_in_rect(0x10, 0x30, 0x12f, 0xba)) {
		cx = (g_mouse_posx - ox - 16) / (tw / zoom) + vx;
		if (cx < 0)
			cx = 0;
		else if (cx >= w)
			cx = w - 1;
		cy = (g_mouse_posy - oy - 48) / (th / zoom) + vy;
		if (cy < 0)
			cy = 0;
		else if (cy >= h)
			cy = h - 1;
	} else if (g_mouse_ldclick != 0) {
		if (is_mouse_in_rect(0x10, 0x30, 0x12f, 0xba)) {
			x = (g_mouse_clickx - ox - 16) / (tw / zoom) + vx;
			if (x < 0)
				x = 0;
			else if (x >= w)
				x = w - 1;
			y = (g_mouse_clicky - oy - 48) / (th / zoom) + vy;
			if (y < 0)
				y = 0;
			else if (y >= h)
				y = h - 1;
			if (!!seen[y][x] && (x != g_group->x || y != g_group->y))
				travel();
			else
				x = y = -1;
		}
		g_mouse_ldclick = 0;
	}
}

void Automap::frame()
{
	signed short mh = 0;

	Bmp b(0x140, 0x90, img + 48 * 320 + 16);
	if (is_mouse_in_rect(0, 0x20, 0x130, 0xc0)) {
		mh = 1;
		mouse_hide(0);
	}
	b.put(g_vga_buf, 0x10, 0x30);
	if (mh)
		mouse_show(0);
	g_automap_redraw = 0;
}

void Automap::draw_tile(signed short xx, signed short yy, signed short t)
{
	signed short old = g_mem_debug;

	g_mem_debug = 0;
	Bmp b(tw / zoom, th / zoom, tiles + tile_idx[t] * tile_size);
	xx /= zoom;
	yy /= zoom;
	b.put(img, xx + ox + 16, yy + oy + 48);
	g_mem_debug = old;
}

void Automap::cursor_right()
{
	if (cx + 1 < w) {
		cx++;
		if (cx - vx >= zoom * 18) {
			vx++;
			scroll_right();
		}
	}
}

void Automap::cursor_down()
{
	if (cy + 1 < h) {
		cy++;
		if (cy - vy >= zoom * 9) {
			vy++;
			scroll_down();
		}
	}
}

void Automap::cursor_left()
{
	if (cx != 0) {
		if (cx == vx) {
			vx--;
			scroll_left();
		}
		cx--;
	}
}

void Automap::cursor_up()
{
	if (cy != 0) {
		if (cy == vy) {
			vy--;
			scroll_up();
		}
		cy--;
	}
}

void Automap::update_cursor()
{
	char far *s;
	signed short mh = 0;

	if (is_mouse_in_rect(cur_x - 16, cur_y - 16, cur_x + 16, cur_y + 16)) {
		mh = 1;
		mouse_hide(0);
	}
	erase_cursor();
	cur_x = (cx - vx) * tw / zoom + ox + 16;
	cur_y = (cy - vy) * th / zoom + oy + 48;
	if (is_mouse_in_rect(cur_x - 16, cur_y - 16, cur_x + 16, cur_y + 16) && !mh) {
		mh = 1;
		mouse_hide(0);
	}
	if (cur_x >= 16 && cur_y >= 48 && cur_x < tw * 18 + 16 && cur_y < th * 9 + 48) {
		bg_cursor->get(*(seen[cy] + cx) && get_map_cell(MAP_GRID, cx, cy) ? img : g_vga_buf, cur_x, cur_y);
		do_border(g_vga_buf, cur_x, cur_y, tw / zoom, th / zoom, 0xff);
	} else
		cur_x = -1;
	if (mh)
		mouse_show(0);
	if (last_x != cur_x || last_y != cur_y) {
		edit->settext((s = find_note()) != 0 && *(seen[cy] + cx) ? s : note);
		last_x = cur_x;
		last_y = cur_y;
	}
	cinfo = (cells + cy * w)[cx].info;
}

void Automap::edit_note()
{
	if (!!seen[cy][cx]) {
		edit->run(note);
#if FIX_0208
		/* Fix #208: ganzen Notiztext (0xfc Bytes) leeren */
		save_note();
		memset(note, 0, 0xfc);
#else
		save_note();
		memset(note, 0, 100);
#endif
	}
	g_action = 0;
}

void Automap::animate()
{
	signed short i;
	signed short xx;
	signed short yy;
	signed short mh;

	Bmp b(tw / zoom, th / zoom);
	if (--g_amap_tick < 0) {
		g_amap_tick = 5;
		if (++g_amap_frame > 5)
			g_amap_frame = 0;
		erase_marks();
		nop();
		update_cursor();
		gx = (g_group->x - vx) * tw / zoom + ox + 16;
		gy = (g_group->y - vy) * th / zoom + oy + 48;
		if (gx >= 16 && gy >= 48 && gx < tw * 18 + 16 && gy < th * 9 + 48) {
			mh = 0;
			if (is_mouse_in_rect(gx - 16, gy - 16, gx + 16, gy + 16)) {
				mh = 1;
				mouse_hide(0);
			}
			bg_own->get(img, gx, gy);
			mark_own[g_amap_frame]->scale(&b);
			b.put(g_vga_buf, gx, gy, 4);
			if (mh != 0)
				mouse_show(0);
		} else
			gx = -1;
		for (i = 0; i < 6; i++) {
			if (!!g_groups[i].nr_heroes && &g_groups[i] != g_group &&
				((g_groups[i].town == g_group->town && g_groups[i].dungeon == g_group->dungeon) ||
				(g_groups[i].town == 3 && g_group->dungeon == 9) || (g_groups[i].dungeon == 9 && g_group->town == 3))) {
				xx = other_x[i] = (g_groups[i].x - vx) * tw / zoom + ox + 16;
				yy = other_y[i] = (g_groups[i].y - vy) * th / zoom + oy + 48;
				if (xx >= 16 && yy >= 48 && tw * 18 + 16 > xx && th * 9 + 48 > yy) {
					mh = 0;
					if (is_mouse_in_rect(xx - 16, yy - 16, xx + 16, yy + 16)) {
						mh = 1;
						mouse_hide(0);
					}
					bg_other[i]->get(img, xx, yy);
					mark_other[g_amap_frame]->scale(&b);
					b.put(g_vga_buf, xx, yy, 4);
					if (mh != 0)
						mouse_show(0);
				} else
					other_x[i] = -1;
			}
		}
	}
}

void Automap::erase_marks()
{
	signed short mh;
	signed short yy;
	signed short i;
	signed short xx;

	if (gx != -1) {
		mh = 0;
		if (is_mouse_in_rect(gx - 16, gy - 16, gx + 16, gy + 16)) {
			mh = 1;
			mouse_hide(0);
		}
		bg_own->put(g_vga_buf, gx, gy);
		gx = -1;
		if (mh != 0)
			mouse_show(0);
	}
	for (i = 0; i < 6; i++) {
		xx = other_x[i];
		yy = other_y[i];
		if (xx != -1) {
			mh = 0;
			if (is_mouse_in_rect(xx - 16, yy - 16, xx + 16, yy + 16)) {
				mh = 1;
				mouse_hide(0);
			}
			bg_other[i]->put(g_vga_buf, xx, yy);
			other_x[i] = -1;
			if (mh != 0)
				mouse_show(0);
		}
	}
}

void Automap::erase_cursor()
{
	signed short mh;

	if (cur_x != -1) {
		mh = 0;
		if (is_mouse_in_rect(cur_x - 16, cur_y - 16, cur_x + 16, cur_y + 16)) {
			mh = 1;
			mouse_hide(0);
		}
		bg_cursor->put(g_vga_buf, cur_x, cur_y);
		cur_x = -1;
		if (mh)
			mouse_show(0);
	}
}

void Automap::clear_notes()
{
	struct map_note far *r;
	signed short i;

	r = &rec;
	r->x = r->y = -1;
	memset(r->text, 0, 0xfc);
	for (i = 0; i < 100; i++)
		put_note(r, i);
}

void Automap::del_note()
{
	signed short i;
	struct map_note far *r;

	if (cinfo == 0) {
		if ((i = find_slot()) != -1) {
			r = &rec;
			r->x = r->y = -1;
#if FIX_0208
			/* Fix #208: ganzen Notiztext (0xfc Bytes) leeren */
			memset(r->text, 0, 0xfc);
			memset(note, 0, 0xfc);
#else
			memset(r->text, 0, 0xfc);
			memset(note, 0, 100);
#endif
			edit->settext(note);
			put_note(r, i);
		}
	}
}

void Automap::put_note(struct map_note far *r, signed short i)
{
	unsigned long off = (unsigned short)(i << 8) + 0xfa00L;

	g_xms_move.len = 0x100;
	g_xms_move.src_handle = 0;
	g_xms_move.src = (unsigned long)r;
	g_xms_move.dst_handle = handle;
	g_xms_move.dst = off;
	if (!ems)
		xms_move_any(&g_xms_move);
	else
		EMS_copy(&g_xms_move);
}

void Automap::get_note(struct map_note far *r, signed short i)
{
	unsigned long off = (unsigned short)(i << 8) + 0xfa00L;

	g_xms_move.len = 0x100;
	g_xms_move.src_handle = handle;
	g_xms_move.src = off;
	g_xms_move.dst_handle = 0;
	g_xms_move.dst = (unsigned long)r;
	if (!ems)
		xms_move_any(&g_xms_move);
	else
		EMS_copy(&g_xms_move);
}

char far *Automap::find_note()
{
	struct map_note far *r;
	signed short i;

	r = &rec;
	for (i = 0; i < 100; i++) {
		get_note(r, i);
		if (cx == r->x && cy == r->y)
			return r->text;
	}
	return 0;
}

signed short Automap::find_slot()
{
	struct map_note far *r;
	signed short i;

	r = &rec;
	for (i = 0; i < 100; i++) {
		get_note(r, i);
		if (cx == r->x && cy == r->y)
			return i;
	}
	r = &rec;
	for (i = 0; i < 100; i++) {
		get_note(r, i);
		if (r->x == -1)
			return i;
	}
	return -1;
}

void Automap::save_note()
{
	signed short i;
	struct map_note far *r;

	if ((i = find_slot()) != -1) {
		r = &rec;
		r->x = cx;
		r->y = cy;
		memcpy(r->text, note, 0xfc);
		put_note(r, i);
	} else
		dlg_message(g_dialog, txt[0x24]);
}

void Automap::load_notes()
{
	struct map_note far *r;
	signed short i;

	ofile_open(g_ofile, note_file);
	r = &rec;
	for (i = 0; i < 100; i++) {
		ofile_read(g_ofile, r, 0x100L);
		put_note(r, i);
	}
	ofile_close(g_ofile);
}

void Automap::save_notes()
{
	struct map_note far *r;
	signed short i;

	ofile_create(g_ofile, note_file, 0x8002);
	r = &rec;
	for (i = 0; i < 100; i++) {
		get_note(r, i);
		ofile_write(g_ofile, r, 0x100L);
	}
	ofile_close(g_ofile);
}
