/* SEG008 - Mauszeiger je Kartenfeld und Aufdecken der Automap um die Gruppe (seg008 in SCHWEIF.EXE)
 * cursor_for_cell(cell): Feldcode 0x80..0x84 -> Mauszeiger 1/2/4/3/0 (mouse_set_cursor nur bei Wechsel,
 * g_cursor_no). reveal_map(map, x, y): Aufdeckkarte map->seen[64][64] des Kartenobjekts (Klasse Map, O052) um
 * (x, y): in der Stadt (g_area_type == AREA_TOWN) 5 x 5 Felder (Randfelder nur, wenn das Feld
 * get_cell_a(...) in der Durchsichtliste map->see_through steht), Mitte MAP_HERE (0x81); sonst 3 x 3 Felder
 * (Mitte | 0x80), beim Kartenzauber (g_reveal_by_spell, O235) nur bisher unbekannte Felder mit 0x41.
 * Feld 0x81 = Standort schon markiert -> nichts tun.
 *
 * BCC -mlarge -3 -Ot -c -Y SEG008.C   -> 2550/2550 Bytes GLEICH (18.10.2026)
 *   (Namensfassung mit schweif.h: 19.10.2026 GLEICH)
 * Schluessel:
 *  - Das Modul laeuft ohne -Oe, aber vor reveal_map steht #pragma option -Oe: nur dann liegt v
 *    (Markierungswert 1 bzw. 0x41) im Register DL wie im Original, waehrend cursor_for_cell ohne -Oe
 *    sein Argument in DI haelt. (Gleichwertige Alternative: _DL als Pseudoregister statt v, ganz ohne -Oe.)
 *  - reveal_map: Zweige mit return statt else (Epilog im Zweig, kein Sprung), die Zeile y+1 prueft
 *    im Original zweimal "<= 63" (kein ">= 0").
 *  - Parameter wie im Prototyp void far * (Aufrufer uebergeben g_map); Zugriff ueber den Cast-Makro MAP
 *    (kein eigener Zeiger - gleicher Code).
 */
#include "schweif.h"

/* Phase B: eigene Daten des Moduls (ds:531c..5322, dsana) - initialisierte Daten in Adressreihenfolge */
char g_fmt_col4_531c[6] = "\xf4%s\xf0";

/* Werte der Aufdeckkarte */
#define MAP_SEEN	1	/* Feld gesehen */
#define MAP_SPELL	0x40	/* durch den Kartenzauber erkannt */
#define MAP_POS		0x80	/* Standort der Gruppe */
#define MAP_HERE	(MAP_POS | MAP_SEEN)

void cursor_for_cell(signed short cell_arg)
{
	register signed short cursor;
	register signed short cell = cell_arg;

	if (cell) {
		switch (cell) {
		case 0x80: cursor = 1; break;
		case 0x81: cursor = 2; break;
		case 0x82: cursor = 4; break;
		case 0x83: cursor = 3; break;
		case 0x84: cursor = 0; break;
		}
		if (cursor != g_cursor_no) {
			mouse_set_cursor(cursor, 0);
			g_cursor_no = cursor;
		}
	}
}

#pragma option -Oe
#define MAP ((struct map far *)map)
/* Feld (row, x + dx) ist durchsichtig */
/* get_cell_a(y, x): Zeile row + g_map_y0, Spalte x + g_map_x0 (Namen seit N8 richtig) */
#define SEE(dx) (is_in_word_array(get_cell_a(row + g_map_y0, x + g_map_x0 + (dx)), MAP->see_through) != -1)

void reveal_map(void far *map, signed short x_arg, signed short y)
{
	register signed short x = x_arg;
	register signed short row;
	signed char v;

	if (MAP->seen[y][x] != (signed char)MAP_HERE) {
		if (g_area_type == AREA_TOWN) {
		row = y - 2;
		if (row >= 0 && row <= 63) {
			if (x - 2 >= 0 && SEE(-2)) MAP->seen[row][x - 2] = MAP_SEEN;
			if (x - 1 >= 0 && SEE(-1)) MAP->seen[row][x - 1] = MAP_SEEN;
			if (SEE(0)) MAP->seen[row][x] = MAP_SEEN;
			if (x + 1 <= 63 && SEE(1)) MAP->seen[row][x + 1] = MAP_SEEN;
			if (x + 2 <= 63 && SEE(2)) MAP->seen[row][x + 2] = MAP_SEEN;
		}
		row = y;
		if (row >= 0 && row <= 63) {
			if (x - 2 >= 0 && SEE(-2)) MAP->seen[row][x - 2] = MAP_SEEN;
			if (x - 1 >= 0) MAP->seen[row][x - 1] = MAP_SEEN;
			MAP->seen[row][x] = MAP_HERE;
			if (x + 1 <= 63) MAP->seen[row][x + 1] = MAP_SEEN;
			if (x + 2 <= 63 && SEE(2)) MAP->seen[row][x + 2] = MAP_SEEN;
		}
		row = y - 1;
		if (row >= 0 && row <= 63) {
			if (x - 2 >= 0 && SEE(-2)) MAP->seen[row][x - 2] = MAP_SEEN;
			if (x - 1 >= 0) MAP->seen[row][x - 1] = MAP_SEEN;
			MAP->seen[row][x] = MAP_SEEN;
			if (x + 1 <= 63) MAP->seen[row][x + 1] = MAP_SEEN;
			if (x + 2 <= 63 && SEE(2)) MAP->seen[row][x + 2] = MAP_SEEN;
		}
		row = y + 1;
		if (row >= 0 && row <= 63) {
			if (x - 2 >= 0 && SEE(-2)) MAP->seen[row][x - 2] = MAP_SEEN;
			if (x - 1 >= 0) MAP->seen[row][x - 1] = MAP_SEEN;
			MAP->seen[row][x] = MAP_SEEN;
			if (x + 1 <= 63) MAP->seen[row][x + 1] = MAP_SEEN;
			if (x + 2 <= 63 && SEE(2)) MAP->seen[row][x + 2] = MAP_SEEN;
		}
		row = y + 2;
		if (row >= 0 && row <= 63) {
			if (x - 2 >= 0 && SEE(-2)) MAP->seen[row][x - 2] = MAP_SEEN;
			if (x - 1 >= 0 && SEE(-1)) MAP->seen[row][x - 1] = MAP_SEEN;
			if (SEE(0)) MAP->seen[row][x] = MAP_SEEN;
			if (x + 1 <= 63 && SEE(1)) MAP->seen[row][x + 1] = MAP_SEEN;
			if (x + 2 <= 63 && SEE(2)) MAP->seen[row][x + 2] = MAP_SEEN;
		}
		return;
		}
		v = MAP_SEEN;
		row = y;
		if (g_reveal_by_spell) {
			v |= MAP_SPELL;
			if (row >= 0 && row <= 63) {
				if (x - 1 >= 0 && !MAP->seen[row][x - 1]) MAP->seen[row][x - 1] = v;
				if (!MAP->seen[row][x]) MAP->seen[row][x] = v;
				if (x + 1 <= 63 && !MAP->seen[row][x + 1]) MAP->seen[row][x + 1] = v;
			}
			row = y - 1;
			if (row >= 0 && row <= 63) {
				if (x - 1 >= 0 && !MAP->seen[row][x - 1]) MAP->seen[row][x - 1] = v;
				if (!MAP->seen[row][x]) MAP->seen[row][x] = v;
				if (x + 1 <= 63 && !MAP->seen[row][x + 1]) MAP->seen[row][x + 1] = v;
			}
			row = y + 1;
#if FIX_0007
			/* Fix #7: Untergrenze wie in den Nachbarzeilen */
			if (row >= 0 && row <= 63) {
#else
			if (row <= 63 && row <= 63) {
#endif
				if (x - 1 >= 0 && !MAP->seen[row][x - 1]) MAP->seen[row][x - 1] = v;
				if (!MAP->seen[row][x]) MAP->seen[row][x] = v;
				if (x + 1 <= 63 && !MAP->seen[row][x + 1]) MAP->seen[row][x + 1] = v;
			}
			return;
		}
			if (row >= 0 && row <= 63) {
				if (x - 1 >= 0) MAP->seen[row][x - 1] = v;
				MAP->seen[row][x] = v | MAP_POS;
				if (x + 1 <= 63) MAP->seen[row][x + 1] = v;
			}
			row = y - 1;
			if (row >= 0 && row <= 63) {
				if (x - 1 >= 0) MAP->seen[row][x - 1] = v;
				MAP->seen[row][x] = v;
				if (x + 1 <= 63) MAP->seen[row][x + 1] = v;
			}
			row = y + 1;
#if FIX_0007
			/* Fix #7: Untergrenze wie in den Nachbarzeilen */
			if (row >= 0 && row <= 63) {
#else
			if (row <= 63 && row <= 63) {
#endif
				if (x - 1 >= 0) MAP->seen[row][x - 1] = v;
				MAP->seen[row][x] = v;
				if (x + 1 <= 63) MAP->seen[row][x + 1] = v;
			}
	}
}
