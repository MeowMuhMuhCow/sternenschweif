/* SEG019 - Zugriff auf die Zellentabelle der 3D-Karte g_cells[64][64] (struct map_cell, 4 Bytes je Zelle;
 * aufgebaut von R17 build_cells): Feldtyp, Zellbits CELL_*, Drehwinkel des Zellobjekts (seg019 in SCHWEIF.EXE)
 * BCC -mlarge -3 -Ot -c -Y SEG019.C   -> 156/156 Bytes GLEICH
 *   (Namensfassung mit schweif.h: 19.10.2026 GLEICH)
 * - Parameter (y, x) wie im Original: erster Index ist die Zeile.
 */
#include "schweif.h"

/* 32-Bit: freigegebene oder abgeschnittene 16-Bit-Koordinaten (unsigned char aus einem
   negativen Kurzindex, z. B. 254) duerfen nicht in die 64x64-Tabelle zeigen. */
static int cell_in_map(signed short y, signed short x)
{
	return g_cells && (unsigned short)y < 64 && (unsigned short)x < 64;
}

unsigned char get_cell_a(signed short y, signed short x)
{
	if (!cell_in_map(y, x))
		return 0;
	return g_cells[y][x].type;
}

signed short get_cell_b(signed short y, signed short x)
{
	if (!cell_in_map(y, x))
		return 0;
	return g_cells[y][x].flags;
}

struct cell_rot get_cell_c(signed short y, signed short x)
{
	struct cell_rot rot;

	rot.ax = rot.ay = rot.az = rot.aw = 0;
	if (!cell_in_map(y, x))
		return rot;
	rot.az = g_cells[y][x].rot << 8;
	return rot;
}
