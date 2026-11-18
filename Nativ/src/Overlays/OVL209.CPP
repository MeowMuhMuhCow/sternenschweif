/* OVL209 - Overlay O209 (Segment 0x2396 in SCHWEIF.EXE): Reisekarte (Reiseobjekt Travel; Wegtabelle g_routes mit
 * 0x106 Wegen zu 10 Bytes: from/to Orte, len Laenge, known bekannt; Kartenobjekt g_tmap, Klasse O187).
 * travelmap_draw(p): Kartenbild 0x94 (Bmp 80x200 nach g_vga_buf bei x=0xf0), wenn g_travel_state != 1, die Gruppe
 * unterwegs ist und mode != 2; Datum (Date_draw), Karte (tmap_set_pal/tmap_draw), Statusbild
 * (travel_status_update), Reiseanimation (bei anim_on), travelmap_draw_icons; g_refresh = 0. travelmap_draw_icons: zwei
 * 24x24-Symbole aus dem Archiv g_file_3 (Offsets 0x7747/0x7507, 0x240 Bytes) nach 0xf1/0x10c, 0xaf.
 * travelmap_walk_x/walk_y(p, x, y, ziel, farbe): Marke waagerecht/senkrecht bis ziel bewegen und zeichnen
 *   (tmap_follow).
 * travelmap_show_route(p, ziel): Route route[] bis ziel, Anfangs-/Endpunkt aus den Wegpunkten points (letzter Index
 * nr_points per route_load_points), Verbindungslinie, alle Wege travelmap_route und Route in Farbe 0xbb.
 * travelmap_route(p, ausser): alle bekannten Wege in Farbe 0xff. travelmap_draw_path(p, ausser)/travelmap_draw_path_to(p,
 * ziel): Routenabschnitte in Farbe 0xbb (Wegnummer per route_find_area fuer Orte 2..20 ausser 5/17/18, sonst
 * route_find). travelmap_set_point1/point2(p, ort): Weg zum Ort suchen, Endpunkt nach map_pts[0..1] bzw. [2..3].
 * travelmap_route_part(p, a, b, -, len2, farbe, len1): Weg anteilig zeichnen, Rueckgabe Wegpunktindex + 2.
 *
 * BCC -mlarge -3 -Ot -Og -c -Y OVL209.CPP   -> 3720/3720 Bytes GLEICH (18.10.2026), cobjfix laeuft
 *   (Namensfassung mit schweif.h, Reiseobjekt Travel: 19.10.2026 GLEICH, LINKGLEICH)
 * - -Og noetig: Zeigeroffset p + i als Zwischenwert ([bp-16h] bzw. DI/SI), nr_points * 2 als Zwischenwert ([bp-18h],
 *   [bp-0ch]), g_routes[i] als Offset [bp-6] mit Segment neu aus ds:c7d2.
 * - travelmap_draw_path/_to: idx OHNE register -> DI, Zwischenwert p + i -> SI (mit register vertauscht).
 * - Wegpunkt-Zugriffe "*(p->points + p->nr_points + 1)" / "*(p->points + i + 1)" / "*(p->points + p->nr_points - i)"
 *   (les/add/[bx+2]); p->points[p->nr_points + 1] bzw. p->points[i + 1] ergeben anderen Code (mov ax,cse / add ax,fa
 *   bzw. inc ax / shl).
 * - zwei char-Locals: zuerst deklariertes bei [bp-1]; [bp-2] (step = 1) wird zuerst gesetzt ->
 *   "signed char done; signed char step = 1; done = 0;".
 * - "i += 0xff" (unsigned char) -> add al,0ffh; abs() inline (Kopien [bp-12h]/[bp-14h]); Bmp in eigenem Block.
 * - travelmap_route_part: Farbe als unsigned char (wie bei den Aufrufern O213/O215).
 * Originalfehler: travelmap_set_point1/2 lesen nach erfolgloser Suche g_routes[0x106] (ein Eintrag hinter der
 *   Tabelle) und benutzen b uninitialisiert, wenn der Ort in keiner Spalte vorkommt. travelmap_draw_path_to benutzt
 *   idx uninitialisiert bzw. veraltet, wenn bei einem Ort 2..20 der naechste Routeneintrag 0xff ist.
 *   travelmap_route_part: 4. Parameter unbenutzt.
 */
#include <stdlib.h>
#include "schweif.h"

#define NR_ROUTES	0x106		/* Wege in g_routes */
#define PIC_TRAVELMAP	0x94		/* Kartenbild (rechter Rand 80 x 200) */
#define COL_ROUTE	0xbb		/* geplante Route */
#define COL_KNOWN	0xff		/* bekannte Wege */
#define IS_AREA_PLACE(o) ((o) > 1 && (o) <= 20 && (o) != 5 && (o) != 17 && (o) != 18)	/* Ort mit Gebiet (route_find_area) */

void travelmap_draw(Travel far *p)
{
	signed short dummy;

	g_pp20_index = PIC_TRAVELMAP;
	mouse_hide(0);
	if (g_travel_state != 1 && group_get_on_route(g_group) && p->mode != 2) {
		{
			Bmp b(0x50, 0xc8);

			nvfile_load(g_nvf_heads, PIC_TRAVELMAP, b.data(), 0, 0, 0, &dummy, &dummy, 0x140);
			b.put(g_vga_buf, 0xf0, 0);
		}
		p->shown[3] = -1;
		Date_draw(g_date, 1);
		tmap_set_pal(g_tmap);
		tmap_draw(g_tmap);
		travel_status_update(p);
		if (p->anim_on != 0)
			travel_anim(p);
		travelmap_draw_icons(p);
	}
	mouse_clear_left();
	mouse_clear_right();
	clear_input();
	mouse_show(0);
	g_refresh = 0;
}

void travelmap_draw_icons(Travel far *p)
{
	signed long off = 7;
	Bmp b1(0x18, 0x18);
	Bmp b2(0x18, 0x18);

	mouse_hide(0);
	arc_open(g_file_3, 6);
	arc_seek(g_file_3, off + 0x7740, 0);
	arc_read(g_file_3, b1.data(), 0x240L);
	b1.put(g_vga_buf, 0xf1, 0xaf);
	arc_seek(g_file_3, off + 0x7500, 0);
	arc_read(g_file_3, b2.data(), 0x240L);
	b2.put(g_vga_buf, 0x10c, 0xaf);
	arc_close(g_file_3);
	mouse_show(0);
}

signed short travelmap_walk_x(Travel far *p, register signed short x, signed short y, register signed short end,
	signed char col)
{
	signed char done;
	signed char step = 1;

	done = 0;
	while (done == 0) {
		if (x > end) {
			x -= step;
			if (x <= end)
				done = 1;
		} else {
			x += step;
			if (x >= end)
				done = 1;
		}
		if (tmap_follow(g_tmap, x, y, step, 0)) {
			wait_for_vsync();
			tmap_draw(g_tmap);
		}
	}
	return x;
}

signed short travelmap_walk_y(Travel far *p, signed short x, register signed short y, register signed short end,
	signed char col)
{
	signed char done;
	signed char step = 1;

	done = 0;
	while (done == 0) {
		if (y > end) {
			y -= step;
			if (y <= end)
				done = 1;
		} else {
			y += step;
			if (y >= end)
				done = 1;
		}
		if (tmap_follow(g_tmap, x, y, step, 0)) {
			wait_for_vsync();
			tmap_draw(g_tmap);
		}
	}
	return y;
}

void travelmap_show_route(Travel far *p, unsigned char dest)
{
	signed short x1, x2, y1, y2;
	signed short dx, dy;
	unsigned char i;
	unsigned char start;
	signed short idx = 0;

	mouse_hide(0);
	travelmap_draw_path_to(p, dest);
	for (i = 0; p->route[i] != dest; i++)
		;
	start = i;
	if (IS_AREA_PLACE(p->route[start]))
		idx = route_find_area(p, p->route[start], p->route[start + 1]);
	else
		idx = route_find(p, p->route[start], p->route[start + 1]);
	p->nr_points = route_load_points(p, idx);
	if (p->route[start + 1] > p->route[start]) {
		x1 = p->points[0];
		y1 = p->points[1];
	} else {
		x1 = p->points[p->nr_points];
		y1 = *(p->points + p->nr_points + 1);
	}
	while (p->route[i + 1] != 0xff)
		i++;
	i += 0xff;
	if (IS_AREA_PLACE(p->route[i]))
		idx = route_find_area(p, p->route[i], p->route[i + 1]);
	else
		idx = route_find(p, p->route[i], p->route[i + 1]);
	p->nr_points = route_load_points(p, idx);
	if (p->route[i + 1] > p->route[i]) {
		x2 = p->points[p->nr_points];
		y2 = *(p->points + p->nr_points + 1);
	} else {
		x2 = p->points[0];
		y2 = p->points[1];
	}
	dx = abs(x1 - x2);
	dy = abs(y1 - y2);
	if (dx > dy) {
		x1 = travelmap_walk_x(p, x1, y1, x2, dest);
		y1 = travelmap_walk_y(p, x1, y1, y2, dest);
	} else {
		y1 = travelmap_walk_y(p, x1, y1, y2, dest);
		x1 = travelmap_walk_x(p, x1, y1, x2, dest);
	}
	travelmap_route(p, -1);
	for (i = 0; p->route[i + 1] != 0xff; i++) {
		if (IS_AREA_PLACE(p->route[i]))
			idx = route_find_area(p, p->route[i], p->route[i + 1]);
		else
			idx = route_find(p, p->route[i], p->route[i + 1]);
		p->nr_points = route_load_points(p, idx);
		tmap_plot_points(g_tmap, p->points, COL_ROUTE);
	}
	wait_for_vsync();
	tmap_draw(g_tmap);
	mouse_show(0);
}

void travelmap_route(Travel far *p, register signed short skip)
{
	register unsigned short i;

	for (i = 0; i < NR_ROUTES; i++) {
		if (g_routes[i].known == 1 && i != skip) {
			route_load_points(p, i);
			tmap_plot_points(g_tmap, p->points, COL_KNOWN);
		}
	}
}

void travelmap_draw_path(Travel far *p, signed short skip)
{
	unsigned char i;
	signed short idx;

	for (i = 0; p->route[i + 1] != 0xff; i++) {
		if (IS_AREA_PLACE(p->route[i]))
			idx = route_find_area(p, p->route[i], p->route[i + 1]);
		else
			idx = route_find(p, p->route[i], p->route[i + 1]);
		if ((skip == -1 || skip != idx) && g_routes[idx].known == 1) {
			p->nr_points = route_load_points(p, idx);
			tmap_plot_points(g_tmap, p->points, COL_ROUTE);
		}
	}
}

void travelmap_draw_path_to(Travel far *p, unsigned char dest)
{
	signed char i;
	signed short idx;

	for (i = 0; p->route[i] != dest; i++) {
		if (IS_AREA_PLACE(p->route[i])) {
#if FIX_0047
			/* Fix #47: Routenende erreicht: Schleife verlassen (idx nicht veraltet benutzen) */
			if (p->route[i + 1] == 0xff)
				break;
			idx = route_find_area(p, p->route[i], p->route[i + 1]);
#else
			if (p->route[i + 1] != 0xff)
				idx = route_find_area(p, p->route[i], p->route[i + 1]);
#endif
		} else
			idx = route_find(p, p->route[i], p->route[i + 1]);
		if (g_routes[idx].known == 1) {
			p->nr_points = route_load_points(p, idx);
			tmap_plot_points(g_tmap, p->points, COL_ROUTE);
		}
	}
}

void travelmap_set_point1(Travel far *p, unsigned char a)
{
	signed short idx;
	unsigned char b;
	register signed short i;

	for (i = 0; i < NR_ROUTES && g_routes[i].from != a; i++)
		;
	if (g_routes[i].from == a)
		b = g_routes[i].to;
	if (i == NR_ROUTES) {
		for (i = 0; i < NR_ROUTES && g_routes[i].to != a; i++)
			;
		if (g_routes[i].to == a)
			b = g_routes[i].from;
	}
#if FIX_0046
	/* Fix #46: Ort ohne Weg: nichts tun statt hinter der Tabelle zu lesen */
	if (i == NR_ROUTES)
		return;
	idx = route_find(p, a, b);
#else
	idx = route_find(p, a, b);
#endif
	p->nr_points = route_load_points(p, idx);
	if (b > a) {
		p->map_pts[0] = p->points[0];
		p->map_pts[1] = p->points[1];
		return;
	}
	p->map_pts[0] = p->points[p->nr_points];
	p->map_pts[1] = *(p->points + p->nr_points + 1);
}

void travelmap_set_point2(Travel far *p, unsigned char a)
{
	signed short idx;
	unsigned char b;
	register signed short i;

	for (i = 0; i < NR_ROUTES && g_routes[i].from != a; i++)
		;
	if (g_routes[i].from == a)
		b = g_routes[i].to;
	if (i == NR_ROUTES) {
		for (i = 0; i < NR_ROUTES && g_routes[i].to != a; i++)
			;
		if (g_routes[i].to == a)
			b = g_routes[i].from;
	}
#if FIX_0046
	/* Fix #46: Ort ohne Weg: nichts tun statt hinter der Tabelle zu lesen */
	if (i == NR_ROUTES)
		return;
	idx = route_find(p, a, b);
#else
	idx = route_find(p, a, b);
#endif
	p->nr_points = route_load_points(p, idx);
	if (b > a) {
		p->map_pts[2] = p->points[0];
		p->map_pts[3] = p->points[1];
		return;
	}
	p->map_pts[2] = p->points[p->nr_points];
	p->map_pts[3] = *(p->points + p->nr_points + 1);
}

unsigned short travelmap_route_part(Travel far *p, unsigned char a, unsigned char b, signed short unused,
	unsigned short len2, unsigned char col, unsigned short len1)
{
	unsigned short n;
	signed short idx;
	unsigned short lim;
	unsigned short t;
	unsigned short step;
	register unsigned short i;

	n = i = lim = 0;
	idx = route_find(p, a, b);
	if (g_on_boat && idx == 0x17)
		idx = 0x1d;
	p->nr_points = route_load_points(p, idx);
	t = g_routes[idx].len * 1000;
	n = p->nr_points;
	step = t / n * 2;
	if (len1) {
		lim = len1 / step * 2;
		for (; i <= lim; i += 2) {
			if (b > a) {
				wait_for_vsync();
				tmap_follow(g_tmap, p->points[i], *(p->points + i + 1), 1, 0);
			} else {
				wait_for_vsync();
				tmap_follow(g_tmap, *(p->points + p->nr_points - i), *(p->points + p->nr_points - i + 1), 1, 0);
			}
		}
	}
	lim = len2 / step * 2;
	for (; i <= lim; i += 2) {
		if (b > a) {
			wait_for_vsync();
			tmap_follow(g_tmap, p->points[i], *(p->points + i + 1), 1, col);
		} else {
			wait_for_vsync();
			tmap_follow(g_tmap, *(p->points + p->nr_points - i), *(p->points + p->nr_points - i + 1), 1, col);
		}
	}
	return lim + 2;
}
