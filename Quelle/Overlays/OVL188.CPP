/* OVL188 - Overlay O188 (SCHWEIF.EXE): Spielbildschirm-Klasse Screen (leer, 1 Byte, s_class.h; Objekt g_screen
 * ds:b77e; Aufrufe aus O014/O069/O070/O076/O082/O251). Konstruktor: zwei Rahmenbilder NVF 0x49 Nr. 3/4 in die
 * Bmp-Objekte g_bmp_mark_le/g_bmp_mark_ae laden (Nvf g_nvf_heads), Symbolnummern auf -1 (reset_icons).
 * draw_base: Grundbild aus einem Pic-Objekt (0x2ae bzw. 4 je nach g_area_type), Palette 0xa0..0xff (pal_fade_out/
 * pal_fade_in), Schrift g_font (Farben, Kasten, Tabulator), g_loc_icons_loaded = -2, g_pp20_index = 4.
 * short_name(d, s): Name bis 7 Zeichen bzw. erstes Leerzeichen kopieren. draw_portraits: Heldenleisten -
 * group_draw_hero_bar fuer die aktive Gruppe und die anderen Gruppen g_groups[6], leere Plaetze bis 7 per
 * clear_portrait (zwei schwarze Bmp 0x20 x 0x20 und 0x2a x 8 an g_portrait_x[n]). load_icon(f, b, c): Bild b
 * (0x240 Bytes ab Satz 7) aus Datei f nach g_loc_icon_buf + c * 0x240 lesen (arc_*), g_loc_icons_loaded[c] merken
 * (nur f == 6). draw_icons: die 9 Symbolfelder (Symbolnummer gegen geladenes Symbol, Datei 6 bzw. 7 mit Nummer i
 * (+9 in Staedten)) an g_gui_buttons_pos[i] zeichnen. redraw: Bildschirm komplett neu (ani_halt, ggf. draw_base,
 * clear_textbar, draw_portraits, draw_icons, g_clock_update = 1, Date_draw). clear_textbar/clear_view: Textleiste
 * 0x133 x 8 bzw. Bild 0xd0 x 0x87. draw_names: Namensleisten der Helden (Rahmen do_border/do_h_line/do_v_line,
 * Farbe 0xfc bei g_portrait_marks[i], Name zentriert per font_center_x/font_print), Platz 6 = NSC
 * (group_get_npc). draw_name(n, c)/draw_name_normal(n): eine Namensleiste in Farbe c bzw. normal. textbar(s, c):
 * Meldung s in der Textleiste (Kasten sichern/setzen/zuruecksetzen, print_text). mark_portrait(n, ae): Rahmenbild
 * LE/AE ueber Portraet n, Zaehler g_portrait_mark_timer[n] = 10. set_icons(n, ...): neue Symbolnummern (variable
 * Argumente), bei Aenderung draw_icons.
 *
 * BCC -mlarge -3 -Ot -Oi -c -Y OVL188.CPP   -> 3286/3286 Bytes GLEICH (18.10.2026), cobjfix laeuft
 *   (mit zusaetzlich -Og ebenfalls GLEICH)
 *   (Namensfassung mit schweif.h, Klasse Screen in s_class.h: 19.10.2026 GLEICH, LINKGLEICH)
 * - C++: Klasse ohne Daten (new(1) im Konstruktor), lokale Bmp-Objekte (gb_new_alloc(w, h), gb_new(w, h, p),
 *   Destruktoren rueckwaerts), lokales Pic-Objekt in draw_base ([bp-0aah], hinter buf).
 * - -Oi: memset/memcpy inline (memset(.., -2, 9) / (.., -1, 9) -> rep stosw + stosb).
 * - Zentrierter Text: "font_print(p, s, font_center_x(p, s, x, 0x2b), 0xbe)" - 0xbe ist das letzte Argument des
 *   aeusseren Aufrufs und wird VOR dem inneren Aufruf gepusht (add sp,0ch / 0ch statt 0eh / 0ah).
 * - draw_portraits: "d = group_draw_hero_bar(g_group, m = d = 0, 1)" - Zuweisungskette mitten in den Argumenten.
 * - draw_portraits: "!!g->nr_heroes" fuer mov al / cbw / or ax,ax in der &&-Kette; "g != g_group" -> mov eax / cmp eax.
 * - clear_portrait: "b1.put(g_vga_buf, x = g_portrait_x[n], 0x9d)" (Zuweisung im Argument).
 * - draw_names: ds:35a7[i] als g_portrait_marks[i - 1] (Konstante in die Verschiebung gefaltet).
 * - Ohne Public: ds:349d = g_portrait_x[6], ds:35b5 = g_portrait_marks[6], ds:3365 = &g_gui_buttons_pos_y[0x22],
 *   ds:b6b8/b74d per #define (g_loc_icons, g_portrait_mark_timer).
 * - font_set_colors (unsigned char) mit Ausdruck/Variable als Farbe: Aufruf ueber gecasteten Funktionszeiger SET_COLORS
 *   (short wie im Original); do_border nimmt die Farbe als short (seit N5).
 * Originalfehler: set_icons prueft nach der Schleife nur old[n] (bei n == 9 hinter dem Array) statt aller
 * restlichen Plaetze; draw_portraits setzt m = 0 nutzlos.
 */
#include <mem.h>
#include <stdarg.h>
#include <io.h>
#include "schweif.h"

#define g_loc_icons		((signed char *)g_b690 + 0x28)	/* ds:b6b8 Symbolnummern der 9 Felder, -1 = Grundsymbol */
#define g_portrait_mark_timer	(g_menu_tail + 0x4e)			/* ds:b74d Anzeigedauer des Rahmens je Portraet */
#define NVF_HEADS		((Nvf far *)g_nvf_heads)
#define SET_COLORS		((void (*)(void far *, signed short, signed short))font_set_colors)
#define NR_ICONS		9
#define ICON_SIZE		0x240				/* 24 x 24 */
#define BAR_Y			0xbd				/* Namensleisten */

Screen::Screen()
{
	signed short tmp;

	NVF_HEADS->load(0x49, g_bmp_mark_le.data(), 0, 3, 0, &tmp, &tmp, SCREEN_W);
	NVF_HEADS->load(0x49, g_bmp_mark_ae.data(), 0, 4, 0, &tmp, &tmp, SCREEN_W);
	reset_icons();
}

void Screen::draw_base()
{
	char far *pal;
	Pic pic;

	pal = (char far *)mem_alloc_w(0x300);
	memset(pal, 0, 0x300);
	pal_fade_out();
	mouse_hide(0);
	pic.loadf(g_area_type == AREA_DUNGEON ? 0x2ae : 4, g_vga_buf);
	g_pp20_index = 4;
	memset(g_loc_icons_loaded, -2, NR_ICONS);
	memcpy(pal + 0x1e0, (char far *)pic.get_palette() + 0x1e0, 0x120);	/* Farben 0xa0..0xff */
	pal_fade_in(pal);
	ani_set_pos(g_screen_pic, 0x10, 2);
	font_set_colors(g_font, 0x1f, 0x1b);
	font_set_box(g_font, 0xc4, 0x71, 0x0c, -1);
	font_set_tab(g_font, 0xcd, 0);
	g_mouse_areas = &g_gui_buttons_pos_y[0x22];
	g_mouse_areas2 = 0;
	mouse_show(0);
	mem_free_v(pal);
}

void Screen::short_name(char far *d, char far *s)
{
	signed short i;

	for (i = 0; i < 7; i++, s++, d++) {
		if (*s == ' ') {
			*d = 0;
			break;
		} else {
			*d = *s;
		}
	}
	*d = 0;
}

void Screen::draw_portraits()
{
	signed short cnt;
	signed short m;
	struct group far *g;
	register signed short i;
	register signed short pos;

	g_refresh_portraits = 0;
	cnt = count_heroes();
	pos = group_draw_hero_bar(g_group, m = pos = 0, 1);
	for (i = 0, g = g_groups; i < NR_GROUPS; i++, g++) {
		if (g != g_group && !!g->nr_heroes)
			pos = group_draw_hero_bar(g, pos, 0);
	}
	m = 7 - cnt;
	for (i = 0; i < m; i++)
		clear_portrait(pos++);
}

void Screen::clear_portrait(register signed short pos)
{
	signed short x;
	void far *p;
	Bmp b1(0x20, 0x20);
	Bmp b2(0x2a, 8);

	p = b1.data();
	memset(p, 0, 0x400);
	p = b2.data();
	memset(p, 0, 0x150);
	mouse_hide(0);
	b1.put(g_vga_buf, x = g_portrait_x[pos], 0x9d);
	b1.put_part(g_vga_buf, x + 0x22, 0x9d, 3, 0x20);
	b1.put_part(g_vga_buf, x + 0x26, 0x9d, 3, 0x20);
	b2.put(g_vga_buf, g_portrait_x[pos], 0xbe);
	mouse_show(0);
}

void Screen::load_icon(register signed short file, signed short nr, register signed short field)
{
	signed short k;

	arc_open(g_file_3, file);
	k = 7;
	arc_seek(g_file_3, (signed long)k + (signed long)nr * ICON_SIZE, SEEK_SET);
	arc_read(g_file_3, g_loc_icon_buf + field * ICON_SIZE, ICON_SIZE);
	arc_close(g_file_3);
	g_loc_icons_loaded[field] = file == 6 ? nr : 0xff;
}

void Screen::draw_icons()
{
	Bmp b(0x18, 0x18, 0);
	register signed short i;

	mouse_hide(0);
	for (i = 0; i < NR_ICONS; i++) {
		b.set_base(g_loc_icon_buf + i * ICON_SIZE);
		if (g_loc_icons[i] != -1) {
			if (g_loc_icons_loaded[i] != g_loc_icons[i])
				load_icon(6, g_loc_icons[i], i);
		} else if (g_loc_icons_loaded[i] != -1) {
			load_icon(7, g_area_type == AREA_DUNGEON ? i + 9 : i, i);
		}
		b.put(g_vga_buf, g_gui_buttons_pos[i].x, g_gui_buttons_pos[i].y);
	}
	mouse_show(0);
}

void Screen::redraw()
{
	ani_halt(g_screen_pic);
	if (g_pp20_index != 4)
		draw_base();
	clear_textbar();
	draw_portraits();
	draw_icons();
	g_clock_update = 1;
	Date_draw(g_date, 1);
	font_set_colors(g_font, 0x1f, 0x1b);
}

void Screen::clear_textbar()
{
	Bmp b(0x133, 8);

	b.fill(g_area_type == AREA_DUNGEON ? 0xe8 : 0);
	mouse_hide(0);
	b.put(g_vga_buf, 5, 0x8e);
	mouse_show(0);
}

void Screen::clear_view()
{
	Bmp b(0xd0, 0x87);

	b.clear();
	mouse_hide(0);
	b.put(g_vga_buf, 0x10, 2, 5);
	mouse_show(0);
}

void Screen::draw_names()
{
	struct hero far *h;
	register signed short i;
	register signed short n;

	n = g_group->nr_heroes;
	h = group_get_npc(g_group);
	if (h) {
		do_border(g_vga_buf, g_portrait_x[6] - 1, BAR_Y, 0x2c, 0x0b, g_portrait_marks[6] ? 0xfc : 0xc6);
		short_name(g_text_output_buf, h->name);
		SET_COLORS(g_font, g_portrait_marks[6] ? 0xfc : 0xff, 0);
		font_print(g_font, g_text_output_buf, font_center_x(g_font, g_text_output_buf, g_portrait_x[6], 0x2b), 0xbe);
		font_restore_colors(g_font);
		n--;
	}
	for (i = 0; i < n; i++) {
		h = g_group->heroes[i];
		if (h) {
			mouse_hide(0);
			if (g_portrait_marks[i]) {
				do_border(g_vga_buf, g_portrait_x[i] - 1, BAR_Y, 0x2c, 0x0b, 0xfc);
			} else {
				do_h_line(g_vga_buf, g_portrait_x[i], BAR_Y, 0x2a, 0xc6);
				do_h_line(g_vga_buf, g_portrait_x[i], 0xc7, 0x2a, 0xc6);
				if (i == 0)
					do_v_line(g_vga_buf, g_portrait_x[i] - 1, BAR_Y, 0x0b, 0xc6);
				else if (!g_portrait_marks[i - 1])
					do_v_line(g_vga_buf, g_portrait_x[i] - 1, BAR_Y, 0x0b, 0xc6);
				if (n - 1 == i && !g_portrait_marks[i])
					do_v_line(g_vga_buf, g_portrait_x[i] + 0x2a, BAR_Y, 0x0b, 0xc6);
			}
			short_name(g_text_output_buf, h->name);
			SET_COLORS(g_font, g_portrait_marks[i] ? 0xfc : 0xff, 0);
			font_print(g_font, g_text_output_buf, font_center_x(g_font, g_text_output_buf, g_portrait_x[i], 0x2b), 0xbe);
			font_restore_colors(g_font);
			mouse_show(0);
		}
	}
}

void Screen::draw_name(register signed short pos, register signed short color)
{
	struct hero far *h;

	if (pos == 6) {
		h = group_get_npc(g_group);
		short_name(g_text_output_buf, h->name);
	} else {
		short_name(g_text_output_buf, g_group->heroes[pos]->name);
	}
	mouse_hide(0);
	do_border(g_vga_buf, g_portrait_x[pos] - 1, BAR_Y, 0x2c, 0x0b, color);
	mouse_show(0);
	SET_COLORS(g_font, color, 0);
	font_print(g_font, g_text_output_buf, font_center_x(g_font, g_text_output_buf, g_portrait_x[pos], 0x2b), 0xbe);
	font_restore_colors(g_font);
}

void Screen::draw_name_normal(register signed short pos)
{
	struct hero far *h;

	if (pos == 6) {
		h = group_get_npc(g_group);
		short_name(g_text_output_buf, h->name);
	} else {
		short_name(g_text_output_buf, g_group->heroes[pos]->name);
	}
	mouse_hide(0);
	do_border(g_vga_buf, g_portrait_x[pos] - 1, BAR_Y, 0x2c, 0x0b, 0xc6);
	mouse_show(0);
	font_set_colors(g_font, 0xff, 0);
	font_print(g_font, g_text_output_buf, font_center_x(g_font, g_text_output_buf, g_portrait_x[pos], 0x2b), 0xbe);
	font_restore_colors(g_font);
}

void Screen::textbar(char far *s, signed short color)
{
	signed short x0;
	signed short w;
	signed short y0;
	signed short h;

	SET_COLORS(g_font, color, g_area_type == AREA_DUNGEON ? 0xe8 : 0);
	font_get_box(g_font, &x0, &w, &y0, &h);
	font_set_box(g_font, 6, 0x133, 0x8e, -1);
	clear_textbar();
	print_text(s);
	font_restore_colors(g_font);
	font_set_box(g_font, x0, w, y0, h);
}

void Screen::mark_portrait(register signed short pos, signed short ae)
{
	Bmp far *b;

	if (g_pp20_index == 4) {
		b = ae == 0 ? &g_bmp_mark_le : &g_bmp_mark_ae;
		mouse_hide(0);
		b->put(g_vga_buf, g_portrait_x[pos], 0x9d, 4);
		mouse_show(0);
		g_portrait_mark_timer[pos] = 10;
	}
}

void Screen::set_icons(signed short n, ...)
{
	va_list ap;
	signed short old[NR_ICONS];
	register signed short i;
	register signed short changed;

	changed = 0;
	for (i = 0; i < NR_ICONS; i++) {
		old[i] = g_loc_icons[i];
		g_loc_icons[i] = -1;
	}
	va_start(ap, n);
	for (i = 0; i < n; i++) {
		g_loc_icons[i] = va_arg(ap, signed short);
		if (g_loc_icons[i] != old[i])
			changed = 1;
	}
#if FIX_0058
	/* Fix #58: alle restlichen Plaetze pruefen (nicht old[n], bei n == 9 hinter dem Feld) */
	for (; i < NR_ICONS; i++)
		if (old[i] != -1)
			changed = 1;
#else
	if (old[i] != -1)
		changed = 1;
#endif
	if (changed && g_pp20_index == 4)
		draw_icons();
}

void Screen::reset_icons()
{
	memset(g_loc_icons, -1, NR_ICONS);
}
