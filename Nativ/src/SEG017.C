/* SEG017 - Karte 64 x 64 (Objekt struct map_grid, 0x10 Bytes): anlegen, freigeben, laden, speichern, Feld
 * lesen/schreiben, Kacheltabelle laden und daraus die Zellentabelle g_cells (struct map_cell) aufbauen
 * (seg017 in SCHWEIF.EXE)
 *
 * BCC -mlarge -3 -Ot -Oi -Og -c -Y SEG017.C   -> 965/965 Bytes GLEICH (18.10.2026)
 *   (Namensfassung mit schweif.h: 19.10.2026 GLEICH)
 * get_map_cell: if/else mit zwei returns (toter Epilog nach dem ersten), braucht -Og.
 * build_cells: Kacheln als struct tile_info (flags signed char), Index direkt mit nr -
 *   [bp-0Eh] ist der -Og-Zwischenwert (int)nr, keine eigene Variable.
 * - Objektparameter wie in s_proto.h als void far *, Zugriff ueber den Cast-Makro MAP (gleicher Code).
 * - Die Karte wird in der Zellentabelle zentriert: Ursprung g_map_x0/g_map_y0 = (64 - w) / 2 bzw.
 *   (64 - h) / 2 (Namen der beiden Ursprungswerte vermutlich vertauscht, siehe R08).
 */
#include <mem.h>
#include "schweif.h"

#define MAP	((struct map_grid far *)map)

void far *new_map(void far *map)
{
	if (map || (map = mem_alloc_w(sizeof(struct map_grid)))) {
		MAP->w = MAP_SIZE;
		MAP->h = MAP_SIZE;
		MAP->data = (unsigned short far *)mem_alloc(MAP_SIZE * MAP_SIZE * sizeof(short));
	}
	return map;
}

void free_map(void far *map, signed short flags_arg)
{
	register signed short flags = flags_arg;

	if (map) {
		mem_free(MAP->data);
		if (flags & 1)
			mem_free_v(map);
	}
}

void load_map(void far *map, signed short id)
{
	ofile_open(g_ofile, id);
	ofile_read(g_ofile, &MAP->w, 2);
	ofile_read(g_ofile, &MAP->h, 2);
	ofile_read(g_ofile, MAP->data, MAP->w * MAP->h * sizeof(short));
	ofile_close(g_ofile);
}

void save_map(void far *map, signed short id)
{
	ofile_create(g_ofile, id, 4);
	ofile_write(g_ofile, &MAP->w, 2);
	ofile_write(g_ofile, &MAP->h, 2);
	ofile_write(g_ofile, MAP->data, MAP->w * MAP->h * sizeof(short));
	ofile_close(g_ofile);
}

unsigned char get_map_cell(void far *map, signed short x, signed short y)
{
	if (MAP->w > x && MAP->h > y) {
		return *(unsigned char far *)&MAP->data[y * MAP->w + x];
	} else {
		return MAP_CELL_INVALID;
	}
}

void set_map_cell(void far *map, signed short x, signed short y, unsigned char nr)
{
	if (MAP->w > x && MAP->h > y)
		*(unsigned char far *)&MAP->data[y * MAP->w + x] = nr;
}

void load_tiles(void far *map, signed short id)
{
	MAP->tiles = (struct tile_info far *)mem_alloc(file_length(g_file_main, id));
	file_load(g_file_main, id, MAP->tiles);
}

void build_cells(void far *map)
{
	unsigned char nr;
	signed short y;
	unsigned short far *src;
	struct map_cell far *cell;
	register signed short x;

	g_map_x0 = (MAP_SIZE - MAP->w) / 2;
	g_map_y0 = (MAP_SIZE - MAP->h) / 2;
	memset(g_cells, 0, MAP_SIZE * MAP_SIZE * sizeof(struct map_cell));
	src = MAP->data;

	for (y = 0; y < MAP->h; y++) {
		for (x = 0; x < MAP->w; x++) {
			nr = *(unsigned char far *)src;
			src++;
			cell = &g_cells[y + g_map_y0][x + g_map_x0];
			cell->type = MAP->tiles[nr].type;
			cell->flags = !(MAP->tiles[nr].flags & TILE_BLOCKED) ? 0 : CELL_BLOCKED;
			if (MAP->tiles[nr].kind == TILE_KIND_WALL0)
				cell->flags |= CELL_WALL0;
			cell->rot = (MAP->tiles[nr].flags & TILE_ROT_MASK) + 1;
		}
	}
}

void load_map_all(void far *map, signed short map_id, signed short tile_id)
{
	load_map(map, map_id);
	load_tiles(map, tile_id);
	build_cells(map);
	mem_free(MAP->tiles);
}
