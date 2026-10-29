/* OVL139 - Overlay O139 (Segment 0x2222 in SCHWEIF.EXE): Kampffeld aus der Umgebung der Gruppe erzeugen (Gitter
 * p->pfad->g[24][25] hinter der Kampfstruktur +0xfad, Art +0x14).
 * fight_env_cell(p, x, y): Feldtyp der Karte ds:caf8 (4 Bytes je Feld, 256 je Zeile) ueber Tabelle ds:cafc, Feld der
 * Gruppe = 1, Typ 5 mit Wert -> 6. fight_env_scan(p): 43 Felder um die Gruppe (Richtungstabelle *ds:cadb[ds:caba]) nach
 * ds:cabb[], Nachbarn links/rechts (R32_003A gegen ds:6646/ds:664f) nach ds:cabf[] (+10), Kartenrand -> 0.
 * fight_load_grid(p): Gitter 600 Bytes aus Archiv ds:35c6 Eintrag 0x1fa. fight_wall_shape(p, n): Wandform (0..15, -1) eines Feldes
 * aus den vier Nachbarn in der Tabelle *ds:caf3 [10][9]. fight_path_to_start(p, x, y): Wegsuche (Breitensuche bis 50 Schritte,
 * Richtungen ds:6658) von (x, y) zum Startfeld 0xcb in der Kopie *ds:c99b - 1 = erreichbar. fight_block_unreachable(p): Gitter
 * nach ds:c99b, Startfeld (ds:6851[ds:caba]), Randdiagonalen, unerreichbare Felder sperren (0x8a), Rueckgabe
 * Zahl der gesperrten Felder. fight_is_wall/fight_is_obj(p, x, y): Feld ist Wand (0x32..0x45) bzw. Objekt (0x9e..0xad).
 * fight_build_field(p): Hauptfunktion - Wandstuecke je Feld (ds:cad7, ds:685b, Tabelle *ds:caef [16][12]), Sonderfelder
 * (3, 10, 7, 8, 5 mit Tueren ds:cabf), fight_block_unreachable, Wandecken/-kanten (0x3c..0x45), oberer Rand, Diagonale, Objekte
 * verschieben (-8/-10), Art 1/3 (+20) bzw. 2 (+45).
 *
 * BCC -mlarge -3 -Ot -Og -Oi -c -Y OVL139.CPP   -> 7003/7003 Bytes GLEICH (18.10.2026), cobjfix laeuft
 *   (Namensfassung mit schweif.h, Welle N7: 19.10.2026 GLEICH, cobjfix LINKGLEICH)
 * - -Oi fuer memset/memcpy (fight_path_to_start/fight_block_unreachable, memcpy mit Member-Quelle laedt die Quelle zuerst), -Og fuer die
 *   Zwischenwerte (y*25, x+1/x+2/y+1.. in den &&-Ketten - nur der immer ausgewertete erste Operand wird spaeter
 *   wiederverwendet, die uebrigen neu gerechnet und gespeichert; entsteht von selbst).
 * - Gitter als 2D-Feld "unsigned char g[24][25]": g[y + 1][x + 2] -> y*25 / add bx,si / [bx+30h] (Konstanten
 *   gefaltet); ebenso *ds:c99b und der Puffer in fight_path_to_start als "unsigned char (far *)[25]".
 * - fight_build_field: nur "register signed short x; signed short y;" ergibt x in SI, y in DI; beide oder keine register-
 *   Variable (in jeder Reihenfolge) vertauschen SI/DI.
 * - fight_wall_shape: "a = (t = tab[i - 1][j]) == -1 ? 2 : g_cabb[t];" (eingebettet, cmp ax); i in SI, j in CX, res in DI
 *   (keine Aufrufe). fight_env_cell: Ergebnis als char in CL, Rueckgabe mov al,cl / cbw.
 * Originalfehler/Auffaelliges: fight_build_field prueft fuer die oberste Zeile "!g[1][x - 1]" (Versatz +0x2d, vermutlich
 *   g[1][x] gemeint); in den Diagonalschleifen werden fight_is_wall/fight_is_obj mit vertauschten Koordinaten (y + i, x - i)
 *   aufgerufen, gelesen wird g[y + i][x - i]. fight_is_wall/fight_is_obj lassen y == 24 zu (Zeile hinter dem 600-Byte-Gitter).
 *   fight_wall_shape liest bei nicht gefundenem n hinter die Tabelle (i = 10). Parameter p in fight_env_cell/fight_wall_shape/fight_path_to_start unbenutzt.
 */
#include <mem.h>
#include "schweif.h"

/* Phase B: eigene Daten des Moduls (ds:6644..6668, dsana) - initialisierte Daten in Adressreihenfolge */
signed short g_fight_timer = -1;
char g_env_left_idx[9] = { 0x00, 0x03, 0x07, 0x0c, 0x12, 0x19, 0x20, 0x26, 0xff };
char g_env_right_idx[9] = { 0x02, 0x06, 0x0b, 0x11, 0x18, 0x1f, 0x25, 0x2a, 0xff };
struct dirs4 g_fig_dirs4 = { { { 1, 0 }, { 0, -1 }, { -1, 0 }, { 0, 1 } } };	/* Kampffeld-Richtung 0: x+1, 1: y-1, 2: x-1, 3: y+1 */

#define WALK	((unsigned char (far *)[FIG_MAP_PITCH])g_fight_walkmap)	/* Begehbarkeitskarte als [24][25] */
#define G	f->field->cells

signed short fight_env_cell(Fight far *f, register signed short x, register signed short y)
{
	struct map_cell far *cell;
	signed char kind;

	if (g_group->x == x && g_group->y == y)
		kind = 1;
	else {
		cell = (struct map_cell far *)(g_env_cells + ((y + g_map_x0) << 8) + ((x + g_map_y0) << 2));
		kind = g_env_kinds[cell->type];
		if (kind == 5 && cell->flags != 0)
			kind = 6;
	}
	return kind;
}

void fight_env_scan(Fight far *f)
{
	signed short y, i, k;
	signed char far *d;
	register signed short x, clip;

	d = g_env_dirtabs[g_env_side];
	for (i = 0; i <= 0x2a; i++, d += 2) {
		clip = 0;
		x = g_group->x + d[0];
		y = g_group->y + d[1];
		if (x < 0) {
			x = 0;
			clip = 1;
		} else if (g_map_w - 1 < x) {
			x = g_map_w - 1;
			clip = 1;
		}
		if (y < 0) {
			y = 0;
			clip = 1;
		} else if (g_map_h - 1 < y) {
			y = g_map_h - 1;
			clip = 1;
		}
		if (clip)
			g_env_kind[i] = 0;
		else
			g_env_kind[i] = fight_env_cell(f, x, y);
		if ((k = is_in_byte_array(i, g_env_left_idx)) != -1) {
			x--;
			if (!clip && x < 0)
				clip = 1;
			if (clip)
				g_env_side_kind[k] = 0;
			else
				g_env_side_kind[k] = fight_env_cell(f, x, y);
		} else if ((k = is_in_byte_array(i, g_env_right_idx)) != -1) {
			x++;
			if (!clip && g_map_w - 1 < x)
				clip = 1;
			if (clip)
				g_env_side_kind[k + 10] = 0;
			else
				g_env_side_kind[k + 10] = fight_env_cell(f, x, y);
		}
	}
}

void fight_load_grid(Fight far *f)
{
	file_open(g_file_main, 0x1fa);
	file_read(g_file_main, f->field->cells, 0x258);
	file_close(g_file_main);
}

signed short fight_wall_shape(Fight far *f, signed short n)
{
	signed short t, a, b, c, d, found;
	register signed short i, j, res;

	found = 0;
	g_env_wall_flag = 0;
	for (i = 0; i < 10; i++) {
		for (j = 0; j < 9; j++) {
			if (g_env_neighbours[i][j] == n) {
				found = 1;
				break;
			}
		}
		if (found)
			break;
	}
	/* Art der vier Nachbarn (oben, rechts, unten, links; -1 = Rand -> 2) */
	a = (t = g_env_neighbours[i - 1][j]) == -1 ? 2 : g_env_kind[t];
	b = (t = g_env_neighbours[i][j + 1]) == -1 ? 2 : g_env_kind[t];
	c = (t = g_env_neighbours[i + 1][j]) == -1 ? 2 : g_env_kind[t];
	d = (t = g_env_neighbours[i][j - 1]) == -1 ? 2 : g_env_kind[t];
	if (g_env_kind[n] == 2) {
		if (a == 2) {
			if (b == 2) {
				if (c == 2) {
					if (d == 2)
						res = 0;
					else
						res = 11;
				} else {
					if (d == 2)
						res = 13;
					else
						res = 5;
				}
			} else {
				if (c == 2) {
					if (d == 2)
						res = 12;
					else
						res = 6;
				} else {
					if (d == 2)
						res = 7;
					else
						res = 1;
				}
			}
		} else {
			if (b == 2) {
				if (c == 2) {
					if (d == 2)
						res = 14;
					else
						res = 9;
				} else {
					if (d == 2)
						res = 8;
					else
						res = 2;
				}
			} else {
				if (c == 2) {
					if (d == 2)
						res = 10;
					else
						res = 3;
				} else {
					if (d == 2)
						res = 4;
					else
						res = 15;
				}
			}
		}
	} else if (g_env_kind[n] == 0)
		res = 0;
	else
		res = -1;
	return res;
}

signed short fight_path_to_start(Fight far *f, signed short x, signed short y)
{
	signed short step = 0, changed, k, nx, ny, cx, cy, found = 0;
	unsigned char c1, c2;
	unsigned char (far *buf)[FIG_MAP_PITCH];
	struct dirs4 d = g_fig_dirs4;

	buf = (unsigned char (far *)[FIG_MAP_PITCH])mem_alloc(0x258);
	changed = 1;
	memset(buf, 0xff, 600);
	buf[y][x] = 0;
	while (found == 0 && changed != 0 && step < 50) {
		changed = 0;
		step++;
		for (cy = 0; cy < FIG_FIELD_W; cy++) {
			if (found != 0)
				break;
			for (cx = 0; cx < FIG_FIELD_W; cx++) {
				if (found != 0)
					break;
				if (buf[cy][cx] == (unsigned char)(step - 1)) {
					for (k = 0; k < 4; k++) {
						ny = cy + d.d[k].dy;
						nx = cx + d.d[k].dx;
						if (ny < FIG_FIELD_W && ny >= 0 && nx < FIG_FIELD_W && nx >= 0) {
							c1 = buf[ny][nx];
							c2 = WALK[ny][nx];
							if (c1 == 0xff) {
								if (c2 == 0 || (c2 == 0x9c && g_fight_env_build == 1)) {
									buf[ny][nx] = step;
									changed = 1;
								} else if (c2 == FIG_CELL_BLOCKED)
									buf[ny][nx] = 0xcc;
								else if (c2 == FIG_CELL_START) {
									found = 1;
									break;
								} else if (c2 >= 0x32)
									buf[ny][nx] = 0xcc;
							}
						}
					}
				}
			}
		}
	}
	mem_free(buf);
	return found;
}

signed short fight_block_unreachable(Fight far *f)
{
	signed short x, y, sx, sy, i, cnt, open, r;
	unsigned char c;

	cnt = 0;
	memcpy(g_fight_walkmap, f->field->cells, 600);
	sy = g_fig_entry_pos[g_env_side].y;
	sx = g_fig_entry_pos[g_env_side].x;
	WALK[sy][sx] = FIG_CELL_START;
	for (x = 16, y = 0, i = 0; i < 8; i++, x++, y++)
		if (!fight_is_wall(f, x, y))
			WALK[y][x] = 0;
	for (x = 8, y = 23, i = 0; i < 16; i++, x++, y--)
		if (!fight_is_wall(f, x, y))
			WALK[y][x] = 0;
	for (y = 0; y < FIG_FIELD_W; y++) {
		open = 1;
		for (x = 0; x < FIG_FIELD_W; x++) {
			c = G[y][x];
			if (open == 0) {
				if (c != 0)
					open = 1;
			} else if (c != 0x8a) {
				r = 1;
				if (sx != x || sy != y)
					r = fight_path_to_start(f, x, y);
				if (r == 0) {
					/* vom Eintrittsfeld nicht erreichbar */
					open = 1;
					if (c != 0 && !fig_is_cell_89_8c(c)) {
						register signed short ok = 1;
						if (fight_is_wall(f, x, y)) {
							if (G[y][x - 1] == 0xad || G[y][x + 1] == 0xaa || G[y - 1][x] == 0xa9 || G[y + 1][x] == 0xa6)
								ok = 0;
							else
								ok = 1;
						}
						if (ok) {
							G[y][x] = 0x8a;
							cnt++;
						} else {
							if (G[y][x] >= 0x3c && G[y][x] <= 0x3f)
								G[y][x] = 0x3c;
							if (G[y][x] >= 0x40 && G[y][x] <= 0x43)
								G[y][x] = 0x43;
						}
					} else if (!c || c == FIG_CELL_BLOCKED)
						G[y][x] = 0x8a;
				} else if (!c)
					open = 0;
			}
		}
	}
	return cnt;
}

signed short fight_is_wall(Fight far *f, signed short x, register signed short y)
{
#if FIX_0183
	/* Fix #183: Gitter hat 24 Zeilen (0..23) */
	if (x < 0 || y < 0 || x > 24 || y > 23)
#else
	if (x < 0 || y < 0 || x > 24 || y > 24)
#endif
		return 0;
	if (G[y][x] >= 0x32 && G[y][x] <= 0x45)
		return 1;
	else
		return 0;
}

signed short fight_is_obj(Fight far *f, signed short x, register signed short y)
{
#if FIX_0183
	/* Fix #183: Gitter hat 24 Zeilen (0..23) */
	if (x < 0 || y < 0 || x > 24 || y > 23)
#else
	if (x < 0 || y < 0 || x > 24 || y > 24)
#endif
		return 0;
	if (G[y][x] >= 0x9e && G[y][x] <= 0xad)
		return 1;
	else
		return 0;
}

void fight_build_field(Fight far *f)
{
	signed short i, k, a6, a8, cnt;
	unsigned char c;
	signed short t, flag;
	register signed short x;
	signed short y;

	cnt = 0;
	for (i = 0; i <= 0x27; i++) {
		if (i > 3 && i != 7 && i != 12 && i != 18 && i != 25 && i != 32 && i < 38) {
			t = fight_wall_shape(f, i);
			if (t != -1) {
				for (k = 0; k < 12; k++) {
					x = g_env_cell_pos[i].x + g_env_cell_offs[k].x;
					y = g_env_cell_pos[i].y + g_env_cell_offs[k].y;
					if (x >= 0 && x < 24 && y >= 0 && y < 24 && (!G[y][x] || G[y][x] == 0xfe)) {
						c = g_env_wall_tab[t][k];
						if (c < 0x57)
							cnt++;
						if (!g_env_wall_flag || c < 0x57)
							G[y][x] = c + 0x32;
					}
				}
				if (!g_env_wall_flag) {
					x = g_env_cell_pos[i].x;
					y = g_env_cell_pos[i].y;
					if (x + 1 >= 0 && x + 1 < 24 && y + 1 >= 0 && y + 1 < 24)
						G[y + 1][x + 1] = 0x8a;
					if (x + 2 >= 0 && x + 2 < 24 && y + 1 >= 0 && y + 1 < 24)
						G[y + 1][x + 2] = 0x8a;
					if (x + 1 >= 0 && x + 1 < 24 && y + 2 >= 0 && y + 2 < 24)
						G[y + 2][x + 1] = 0x8a;
					if (x + 2 >= 0 && x + 2 < 24 && y + 2 >= 0 && y + 2 < 24)
						G[y + 2][x + 2] = 0x8a;
				}
			}
			if (g_env_kind[i] == 3) {
				x = g_env_cell_pos[i].x;
				y = g_env_cell_pos[i].y;
				if (x + 2 >= 0 && x + 2 < 24 && y + 2 >= 0 && y + 2 < 24) {
					G[y + 2][x + 2] = 0x5e;
					cnt++;
				}
			} else if (g_env_kind[i] == 10) {
				x = g_env_cell_pos[i].x;
				y = g_env_cell_pos[i].y;
				if (x + 1 >= 0 && x + 1 < 24 && y + 1 >= 0 && y + 1 < 24) {
					G[y + 1][x + 1] = 0x88;
					cnt++;
				}
			} else if (g_env_kind[i] == 7) {
				for (a8 = 0; a8 < 4; a8++)
					for (a6 = 0; a6 < 4; a6++) {
						x = g_env_cell_pos[i].x + a6;
						y = g_env_cell_pos[i].y + a8;
						if (x >= 0 && x < 24 && y >= 0 && y < 24)
							G[y][x] = 0x9c;
					}
			} else if (g_env_kind[i] == 8) {
				for (a8 = 0; a8 < 4; a8++)
					for (a6 = 0; a6 < 4; a6++) {
						x = g_env_cell_pos[i].x + a6;
						y = g_env_cell_pos[i].y + a8;
						if (x >= 0 && x < 24 && y >= 0 && y < 24)
							G[y][x] = 0x9d;
					}
			} else if (g_env_kind[i] == 5) {
				signed short k2, a, b;
				x = g_env_cell_pos[i].x;
				y = g_env_cell_pos[i].y;
				if ((k2 = is_in_byte_array(i, g_env_left_idx)) != -1)
					a = g_env_side_kind[k2];
				else
					a = g_env_kind[i - 1];
				if ((k2 = is_in_byte_array(i, g_env_right_idx)) != -1)
					b = g_env_side_kind[k2 + 10];
				else
					b = g_env_kind[i + 1];
				if (a == 1 && b == 1) {
					if (x + 1 >= 0 && x + 1 < 24 && y >= 0 && y < 24) {
						G[y][x + 1] = 0xa6;
						cnt++;
					}
					if (x + 1 >= 0 && x + 1 < 24 && y + 1 >= 0 && y + 1 < 24) {
						G[y + 1][x + 1] = 0xa7;
						cnt++;
					}
					if (x + 1 >= 0 && x + 1 < 24 && y + 2 >= 0 && y + 2 < 24) {
						G[y + 2][x + 1] = 0xa8;
						cnt++;
					}
					if (x + 1 >= 0 && x + 1 < 24 && y + 3 >= 0 && y + 3 < 24) {
						G[y + 3][x + 1] = 0xa9;
						cnt++;
					}
				} else {
					if (x >= 0 && x < 24 && y + 1 >= 0 && y + 1 < 24) {
						G[y + 1][x] = 0xaa;
						cnt++;
					}
					if (x + 1 >= 0 && x + 1 < 24 && y + 1 >= 0 && y + 1 < 24) {
						G[y + 1][x + 1] = 0xab;
						cnt++;
					}
					if (x + 2 >= 0 && x + 2 < 24 && y + 1 >= 0 && y + 1 < 24) {
						G[y + 1][x + 2] = 0xac;
						cnt++;
					}
					if (x + 3 >= 0 && x + 3 < 24 && y + 1 >= 0 && y + 1 < 24) {
						G[y + 1][x + 3] = 0xad;
						cnt++;
					}
				}
			}
		}
	}
	cnt -= fight_block_unreachable(f);
	for (y = 0; y < 24; y++)
		for (x = 0; x < 24; x++) {
			if (G[y][x] == 0x8a || G[y][x] == 0x89) {
				if (y > 0 && x > 0 && fight_is_wall(f, x, y - 1) && fight_is_wall(f, x - 1, y) &&
					!fight_is_wall(f, x - 1, y - 1)) {
					G[y][x] = 0x43;
					cnt++;
					G[y][x - 1] = 0x43;
					G[y - 1][x] = 0x3f;
				} else if (y > 0 && x < 25 && fight_is_wall(f, x, y - 1) && fight_is_wall(f, x + 1, y) &&
					!fight_is_wall(f, x + 1, y - 1)) {
					G[y][x] = 0x45;
					cnt++;
					G[y][x + 1] = 0x40;
					G[y - 1][x] = 0x3f;
				} else if (y < 24 && x > 0 && fight_is_wall(f, x, y + 1) && fight_is_wall(f, x - 1, y) &&
					!fight_is_wall(f, x - 1, y + 1)) {
					G[y][x] = 0x3c;
					cnt++;
					G[y][x - 1] = 0x43;
					G[y + 1][x] = 0x3c;
				} else if (y < 24 && x < 25 && fight_is_wall(f, x, y + 1) && fight_is_wall(f, x + 1, y) &&
					!fight_is_wall(f, x + 1, y + 1)) {
					G[y][x] = 0x3c;
					cnt++;
					G[y][x + 1] = 0x40;
					G[y + 1][x] = 0x3c;
				}
			}
		}
	for (x = 1; x < 20; x++)
		if (G[0][x] == 0x8a && !G[1][x - 1])
			G[0][x] = 0x3c;
	for (x = 0; x < 20; x++) {
		if (G[0][x] == 0x8a)
			G[0][x] = 0x89;
		else if (G[0][x] == 0x45)
			G[0][x] = 0x40;
		else if (G[0][x] == 0x3f)
			G[0][x] = 0x3c;
		else if (G[0][x] == 0x42 && (G[0][x + 1] < 0x3c || G[0][x + 1] > 0x45))
			G[0][x] = 0x43;
	}
	for (i = 0; i < 8; i++) {
		if (G[i + 1][i + 16] == 0x8a) {
			G[i + 1][i + 16] = 0x89;
			G[i][i + 16] = 0x89;
		} else if (!G[i][i + 16])
			G[i][i + 16] = 0xfe;
		else if (G[i][i + 16] == 0x8a)
			G[i][i + 16] = 0x89;
		if (G[i][i + 17] == 0xfe && fight_is_wall(f, i + 16, i) && fight_is_wall(f, i + 17, i + 1))
			G[i][i + 17] = 0x3c;
	}
	for (y = 0; y < 24; y++)
		for (x = 0; x < 24; x++) {
			if (fig_is_cell_89_8c(G[y][x]))
				continue;
			if (fight_is_wall(f, x, y) || fight_is_obj(f, x, y)) {
				if (fight_is_wall(f, x, y - 1) || fight_is_wall(f, x, y + 1) || fight_is_wall(f, x + 1, y) ||
					fight_is_obj(f, x, y - 1) || fight_is_obj(f, x, y + 1) || fight_is_obj(f, x + 1, y)) {
					flag = 1;
					if (!G[y + 1][x])
						flag = 0;
					else {
						for (i = 1; x - i >= 0 && y + i < 24 && i <= 4; i++) {
							if (!G[y + i][x - i] || !G[y + i + 1][x - i] ||
#if FIX_0183
								/* Fix #183: Koordinaten (x, y) wie beim gelesenen Feld G[y + i][x - i] */
								(!fig_is_cell_89_8c(G[y + i][x - i]) && !fight_is_wall(f, x - i, y + i) &&
								!fight_is_obj(f, x - i, y + i))) {
#else
								(!fig_is_cell_89_8c(G[y + i][x - i]) && !fight_is_wall(f, y + i, x - i) &&
								!fight_is_obj(f, y + i, x - i))) {
#endif
								flag = 0;
								break;
							}
						}
					}
					if (flag) {
						if (fight_is_obj(f, x, y))
							G[y][x] -= 8;
						else
							G[y][x] -= 10;
					}
				}
				a6 = x + 1;
				while (fight_is_wall(f, a6, y) || fight_is_obj(f, a6, y)) {
					flag = 1;
					for (i = 1; a6 - i >= 0 && y + i < 24 && i <= 4; i++) {
						if (!G[y + i][a6 - i] || !G[y + i + 1][a6 - i] ||
#if FIX_0183
							/* Fix #183: Koordinaten (x, y) wie beim gelesenen Feld G[y + i][a6 - i] */
							(!fig_is_cell_89_8c(G[y + i][a6 - i]) && !fight_is_wall(f, a6 - i, y + i) &&
							!fight_is_obj(f, a6 - i, y + i))) {
#else
							(!fig_is_cell_89_8c(G[y + i][a6 - i]) && !fight_is_wall(f, y + i, a6 - i) &&
							!fight_is_obj(f, y + i, a6 - i))) {
#endif
							flag = 0;
							break;
						}
					}
					if (flag) {
						if (fight_is_obj(f, a6, y))
							G[y][a6] -= 8;
						else
							G[y][a6] -= 10;
					} else
						break;
					a6++;
				}
				break;
			} else
				break;
		}
	if (f->field->type != 0) {
		for (y = 0; y < 24; y++)
			for (x = 0; x < 24; x++) {
				if (fight_is_wall(f, x, y)) {
					if (f->field->type == 1 || f->field->type == 3) {
						G[y][x] += 20;
						if (G[y][x] == 0x1d) {
							G[y][x + 1] = 0x18;
							G[y - 1][x] = 0x17;
						}
						if (G[y][x] == 0x27) {
							G[y][x + 1] = 0x22;
							G[y - 1][x] = 0x21;
						}
					} else if (f->field->type == 2)
						G[y][x] += 0x2d;
				}
			}
	}
}
