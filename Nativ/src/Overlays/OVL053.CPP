/* OVL053 - Overlay O053 (Segment 0x2041 in SCHWEIF.EXE): weitere Methoden der Automap, class Automap (s_class.h; Karte
 * cells +11b (2 Bytes je Feld, struct map_tile), Aufdeckkarte seen +16f (64 Bytes je Zeile, Bit 6), Ziel x/y +177/+179).
 * nop: leer. travel: Reisen per Mausklick - begehbare Feldtypen als Liste (g_walk_default, g_walk_town4/5/3 je nach
 * group.town bzw. g_walk_dng[group.dungeon - 1]), Zielfeld pruefen (is_in_byte_array), bei g_amap_teleport == 0
 * Wegsuche path_search (Breitensuche im Puffer w*h, Werte als signed char, Abbruch bei Treffer oder Wert 0x100) und
 * Weg path_trace (Richtungen 3/1/0/2 rueckwaerts, danach group_set_screen_pos(x, y)); sonst Sprung nur auf bekannte
 * Felder (seen Bit 6), g_amap_teleported = 1. random_cell: zufaelliges Feld in hoechstens 8 Schritten (Breitensuche,
 * Ringpuffer der letzten 10 Felder, Schritte dirs, random_schweif) -> Position setzen, group_set_screen_pos,
 * reveal_map. cell_read/cell_write: Feld (x, y) aus Kartendatei id lesen bzw. schreiben (g_ofile, O243: Kopf w/h,
 * Sprung relativ (y*w + x)*2). init: Texte laden (txt_load_list, 0x21e), x = f17b = -1, load_cells. free_cells:
 * Karte und Texte freigeben, g_refresh = 1.
 *
 * BCC -mlarge -3 -Ot -Og -Oi -c -Y OVL053.CPP   -> 2805/2805 Bytes GLEICH (18.10.2026), cobjfix laeuft
 *   (Namensfassung mit schweif.h: 19.10.2026 GLEICH, LINKGLEICH)
 * - -Og: Zwischenwerte [bp-10h] (= w), [bp-12h]/[bp-14h] (Offset von p -/+ w) in path_search, [bp-2eh]/[bp-30h] in
 *   random_cell.
 * - -Oi: memset(buf, -2, w * h) (mov ax,0fffeh / mov ah,al / rep stosw / adc / rep stosb).
 * - random_cell: lokale Tabellen dirs/q mit Initialisierer (BCC: Vorlage im eigenen _DATA ds:551f/552f -> mov si /
 *   lea di / push ss / pop es / rep movsw). Aggregat-Locals liegen HINTER allen skalaren Locals und -Og-Zwischenwerten
 *   ([bp-40h], [bp-68h]), egal wo sie deklariert sind. Initialisierte Deklarationen in Speicherreihenfolge.
 * - Phase B (19.10.2026): eigene Daten im Quelltext. g_walk_dng[16] (Index dungeon - 1, MAP-Name g_walk_dungeon = 4 Bytes
 *   davor) zeigt auf die bisher namenlosen Listen g_walk_dng_nn; Dungeon 9 benutzt g_walk_town3.
 * - path_search: "if (!!seen[row][col] && ..)" -> mov al/cbw/or ax,ax; "seen[row][col] &&" (auch != 0) -> cmp byte.
 * - Zeigerformen: (cells + y * w)[x].type bzw. *(buf + y * w + x) (Adresse schrittweise), seen als signed char
 *   (far *)[64].
 * - travel: "ret = g_amap_teleported = 1"; "return ret;" im inneren if mit else -> Epilog + toter jmp short.
 * - nop als (leere) Elementfunktion: gleiche Bytes wie die freie Funktion des Originals.
 */
#include <mem.h>
#include "schweif.h"

/* Phase B: eigene Daten des Moduls (ds:5322..5558, dsana) - initialisierte Daten in Adressreihenfolge */
/* begehbare Feldtypen (Liste, Ende -1) */
char g_walk_default[] = {	/* Standard (Staedte ohne eigene Liste) */
	1, 2, 3, 4, 5, 6, 7, 8, 9, 14, 15, 16, 17, 18, 19, 20, 21, 22, 31, 32, 102, 103, 104, 105, -123, -122, -1
};
char g_walk_town5[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 11, 111, 112, -1 };	/* Stadt 5 */
char g_walk_town4[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 111, 112, -1 };	/* Stadt 4 */
char g_walk_dng_01[] = {	/* Dungeon 1 */
	1, 2, 3, 4, 5, 12, 13, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 48, 49,
	50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 65, 66, 67, 68, 69, 70, 71, -1
};
char g_walk_dng_02[] = { 1, 2, 3, 4, 5, 13, 14, 58, 59, 60, -1 };	/* Dungeon 2 */
char g_walk_dng_03[] = { 1, 2, 3, 4, 5, 9, 10, 24, 25, 28, 29, 30, 54, 55, 56, -1 };	/* Dungeon 3 */
char g_walk_dng_04[] = {	/* Dungeon 4 */
	5, 6, 9, 10, 11, 12, 13, 21, 22, 23, 24, 26, 27, 28, 29, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, -1
};
char g_walk_dng_05[] = {	/* Dungeon 5 */
	1, 2, 3, 4, 17, 18, 19, 20, 25, 26, 27, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, -1
};
char g_walk_dng_06[] = {	/* Dungeon 6 */
	1, 2, 3, 4, 5, 7, 8, 9, 10, 11, 16, 17, 18, 19, 78, 79, 80, 81, 82, 94, 95, 96, 97, 98, 100, 101, 102, 103,
	104, 105, 106, 112, 113, 114, 115, -1
};
char g_walk_dng_07[] = {	/* Dungeon 7 */
	1, 2, 3, 4, 5, 7, 8, 9, 10, 11, 16, 17, 18, 19, 78, 79, 80, 81, 82, 94, 95, 96, 97, 98, 100, 101, 102, 103,
	104, 105, 106, 112, 113, 114, 115, -1
};
char g_walk_dng_08[] = {	/* Dungeon 8 */
	1, 2, 3, 4, 5, 7, 8, 9, 10, 11, 16, 17, 18, 19, 78, 79, 80, 81, 82, 94, 95, 96, 97, 98, 100, 101, 102, 103,
	104, 105, 106, 112, 113, 114, 115, -1
};
char g_walk_town3[] = {	/* Dungeon 9 und Stadt 3 */
	1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 66, 67, 68, 69,
	70, 71, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100, 101, 102, 103, 105, 106, -1
};
char g_walk_dng_10[] = { 1, 2, 3, 4, 5, 20, 27, 28, 31, 37, 38, 39, 40, 41, 43, 44, -1 };	/* Dungeon 10 */
char g_walk_dng_11[] = { 1, 2, 3, 4, 5, 23, 31, 33, 38, 39, 40, 41, 44, 45, 46, -1 };	/* Dungeon 11 */
char g_walk_dng_12[] = {	/* Dungeon 12 */
	1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 39, 40, 44, 47, 58, 59, -1
};
char g_walk_dng_13[] = { 1, 2, 3, 4, 5, 20, 21, 22, 23, 24, 35, 36, 37, 38, 39, 40, 41, -1 };	/* Dungeon 13 */
char g_walk_dng_14[] = {	/* Dungeon 14/15 */
	1, 2, 3, 4, 5, 9, 10, 11, 12, 13, 14, 15, 16, 19, 20, 21, 22, 23, 24, -1
};
char g_walk_dng_16[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 14, 15, 32, 33, 34, 46, -1 };	/* Dungeon 16 */
char far *g_walk_dng[16] = {	/* je Dungeon 1..16 (Index group.dungeon - 1) */
	g_walk_dng_01, g_walk_dng_02, g_walk_dng_03, g_walk_dng_04, g_walk_dng_05, g_walk_dng_06,
	g_walk_dng_07, g_walk_dng_08, g_walk_town3, g_walk_dng_10, g_walk_dng_11, g_walk_dng_12,
	g_walk_dng_13, g_walk_dng_14, g_walk_dng_14, g_walk_dng_16
};
/* ds:551f..5557: BCC-Vorlagen der lokalen dirs und q in Automap::random_cell */

void Automap::nop()
{
}

signed short Automap::travel()
{
	signed short px;
	signed short py;
	char far *walk;
	signed char far *buf;
	char far *path;
	register signed short ret = 0;

	walk = g_walk_default;
	if (g_group->dungeon != 0 && g_group->dungeon != 9) {
		if (g_amap_teleport == 0) {
			dlg_message(g_dialog, txt[0x25]);
			return ret;
		} else
			walk = g_walk_dng[g_group->dungeon - 1];
	} else if (g_group->town == 4)
		walk = g_walk_town4;
	else if (g_group->town == 5)
		walk = g_walk_town5;
	else if (g_group->town == 3 || g_group->dungeon == 9)
		walk = g_walk_town3;
	if (is_in_byte_array((cells + y * w)[x].type, walk) != -1) {
		buf = (signed char far *)mem_alloc_w(w * h);
		path = (char far *)mem_alloc_w(0xfa);
		if (g_amap_teleport == 0) {
			ret = path_search(walk, buf, &px, &py, 0);
			if (ret == 1)
				path_trace(path, buf + py * w + px);
			else
				dlg_message(g_dialog, txt[0x26]);
		} else {
			if (!(seen[y][x] & 0x40)) {
				group_set_screen_pos(x, y);
				ret = g_amap_teleported = 1;
			} else
				dlg_message(g_dialog, txt[0x27]);
		}
		mem_free_v(path);
		mem_free_v(buf);
	} else
		dlg_message(g_dialog, txt[0x28]);
	return ret;
}

signed short Automap::path_search(char far *walk, signed char far *buf, signed short far *px, signed short far *py,
	signed short all)
{
	signed short k;
	signed char far *p;
	signed char c;
	signed short idx;
	signed short val;
	signed short found;
	register signed short col;
	register signed short row;

	idx = 0;
	val = 0;
	found = 0;
	for (row = 0; row < h; row++) {
		for (col = 0; col < w; idx++, col++) {
			c = cells[idx].type;
			p = buf + idx;
			*p = -2;
			if (c > 0) {
				if (all == 0) {
					if (!!seen[row][col] && is_in_byte_array(c, walk) != -1)
						*p = -1;
				} else if (is_in_byte_array(c, walk) != -1)
					*p = -1;
			}
		}
	}
	*(buf + y * w + x) = 0;
	*px = g_group->x;
	*py = g_group->y;
#if FIX_0022
	/* Fix #22: Puffer ist signed char: Suche bei 127 beenden statt leer bis 0x100 weiterzulaufen */
	while (found == 0 && val < 0x7f) {
#else
	while (found == 0 && val < 0x100) {
#endif
		p = buf;
		for (k = col = row = 0; k < w * h && found == 0; col++, k++, p++) {
			if (col == w) {
				row++;
				col = 0;
			}
			if (*p == val) {
				if (col > 0 && p[-1] == -1) {
					p[-1] = val + 1;
					if (col - 1 == *px && *py == row)
						found = 1;
				}
				if (col < w - 1 && p[1] == -1) {
					p[1] = val + 1;
					if (col + 1 == *px && *py == row)
						found = 1;
				}
				if (row > 0 && *(p - w) == -1) {
					*(p - w) = val + 1;
					if (*px == col && row - 1 == *py)
						found = 1;
				}
				if (row < h - 1 && *(p + w) == -1) {
					*(p + w) = val + 1;
					if (*px == col && row + 1 == *py)
						found = 1;
				}
			}
		}
		val++;
	}
	return found;
}

void Automap::path_trace(char far *out, signed char far *p)
{
	signed char v;
	register signed short n = 0;

	for (v = *p - 1; v > -1; v--) {
		if (p[-1] == v) {
			out[n] = 3;
			n++;
			p--;
		} else if (p[1] == v) {
			out[n] = 1;
			n++;
			p++;
		} else if (*(p - w) == v) {
			out[n] = 0;
			n++;
			p -= w;
		} else if (*(p + w) == v) {
			out[n] = 2;
			n++;
			p += w;
		}
	}
	group_set_screen_pos(x, y);
}

void Automap::random_cell()
{
	signed char far *buf = (signed char far *)mem_alloc_w(w * h);
	char far *walk = g_walk_default;
	signed short gx = g_group->x;
	signed short gy = g_group->y;
	signed short col;
	signed short col2;
	signed short nx;
	signed short d;
	signed short row;
	signed short row2;
	signed short ny;
	signed char c;
	signed short idx = 0;
	signed char far *p;
	signed short val;
	struct dir_steps dirs = { { { 1, 0 }, { 0, -1 }, { -1, 0 }, { 0, 1 } } };	/* rechts, oben, links, unten */
	signed short maxv = 8;
	signed short cnt = 0;
	signed short qi = -1;
	signed short changed;
	struct point10 q = { 0 };				/* Ringpuffer */

	if (g_group->dungeon != 0)
		walk = g_walk_dng[g_group->dungeon - 1];
	else if (g_group->town == 4)
		walk = g_walk_town4;
	else if (g_group->town == 5)
		walk = g_walk_town5;
	memset(buf, -2, w * h);
	for (row = 0; row < h; row++) {
		for (col = 0; col < w; idx++, col++) {
			c = cells[idx].type;
			p = buf + idx;
			*p = -2;
			if (c > 0 && is_in_byte_array(c, walk) != -1)
				*p = -1;
		}
	}
	*(buf + gy * w + gx) = 0;
	changed = val = 0;
	while (changed == 0 && val < maxv) {
		val++;
		changed = 1;
		for (row2 = 0; row2 < h; row2++) {
			for (col2 = 0; col2 < w; col2++) {
				if (*(buf + row2 * w + col2) == val - 1) {
					for (d = 0; d < 4; d++) {
						ny = row2 + dirs.d[d].y;
						nx = col2 + dirs.d[d].x;
						if (ny < h && ny >= 0 && nx < w && nx >= 0 && *(buf + ny * w + nx) == -1) {
							*(buf + ny * w + nx) = val;
							cnt++;
							qi++;
							if (qi == 10)
								qi = 0;
							q.p[qi].x = nx;
							q.p[qi].y = ny;
							changed = 0;
						}
					}
				}
			}
		}
	}
	if (cnt > 0) {
		c = random_schweif(cnt > 9 ? 10 : cnt) - 1;
		x = q.p[c].x;
		y = q.p[c].y;
		group_set_screen_pos(x, y);
		reveal_map(this, x, y);
	}
	mem_free_v(buf);
}

signed char Automap::cell_read(signed short id, signed short cx, signed short cy)
{
	signed short fw;
	signed short fh;
	unsigned long pos;
	signed char b[2];

	ofile_create(g_ofile, id, 0x8004);
	ofile_read(g_ofile, &fw, 2);
	ofile_read(g_ofile, &fh, 2);
	pos = (unsigned short)((cy * fw + cx) * 2);
	ofile_seek(g_ofile, pos, 1);
	ofile_read(g_ofile, b, 2);
	ofile_close(g_ofile);
	return b[0];
}

void Automap::cell_write(signed short id, signed short cx, signed short cy, signed char v)
{
	signed short fw;
	signed short fh;
	unsigned long pos;
	signed char b[2];

	b[0] = v;
	b[1] = 0;
	ofile_create(g_ofile, id, 0x8004);
	ofile_read(g_ofile, &fw, 2);
	ofile_read(g_ofile, &fh, 2);
	pos = (unsigned short)((cy * fw + cx) * 2);
	ofile_seek(g_ofile, pos, 1);
	ofile_write(g_ofile, b, 2);
	ofile_close(g_ofile);
}

void Automap::init(signed short id)
{
	swap_out_screen();
	if (txt_buf == 0)
		txt_load_list(g_font, 0x21e, 0x2d, &txt_buf, &txt);
	x = f17b = -1;
	load_cells(id);
}

void Automap::free_cells()
{
	mem_free(cells);
	if (txt_buf) {
		txt_buf = mem_free(txt_buf);
		txt = (char far * far *)mem_free(txt);
	}
	g_refresh = 1;
}
