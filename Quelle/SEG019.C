/* SEG019 - Zugriff auf die Zellentabelle der 3D-Karte g_cells[64][64] (struct map_cell, 4 Bytes je Zelle;
 * aufgebaut von R17 build_cells): Feldtyp, Zellbits CELL_*, Drehwinkel des Zellobjekts (seg019 in SCHWEIF.EXE)
 * BCC -mlarge -3 -Ot -c -Y SEG019.C   -> 156/156 Bytes GLEICH
 *   (Namensfassung mit schweif.h: 19.10.2026 GLEICH)
 * - Parameter (y, x) wie im Original: erster Index ist die Zeile.
 */
#include "schweif.h"

unsigned char get_cell_a(signed short y, signed short x)
{
	return g_cells[y][x].type;
}

signed short get_cell_b(signed short y, signed short x)
{
	return g_cells[y][x].flags;
}

struct cell_rot get_cell_c(signed short y, signed short x)
{
	struct cell_rot rot;

	rot.ax = rot.ay = 0;
	rot.az = g_cells[y][x].rot << 8;
	return rot;
}
