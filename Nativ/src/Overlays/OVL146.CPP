/* OVL146 - Overlay O146 (Segment 0x2246 in SCHWEIF.EXE): Wegsuche im Kampf (entspricht DSA1 seg038,
 * Bright Eyes FIG_find_path_to_target / _backtrack / FIG_count_direction_changes_of_path). Kampffeld 24 x 24
 * (Zeilen zu 25 Bytes, g_fight_walkmap), Kopie f->search_map, Weg nach g_fig_path (Richtungen 0..3, -1 Ende).
 * fig_find_path_backtrack(f, tab, x, y, dist, bp, mode, two, id): Rueckverfolgung in der Distanztabelle mit je
 *   bevorzugter Richtung 0..3 (g_fig_dirs_back, vier Puffer zu 50 Bytes), Weg mit den wenigsten Richtungswechseln
 *   nach g_fig_path, bei zu wenig BP Endmarke FIG_PATH_SHORT. fig_count_dir_changes(f, path): Richtungswechsel
 *   zaehlen (99 bei leerem Weg). fig_path_add_target(f, &n, flag, tx, ty, list, x, y, e): Ziel n eintragen
 *   (Kennung e, FIG_CELL_HERO_GOAL.. -> +0x33, FIG_CELL_ENEMY_GOAL.. -> +0x35), 1 bei 10 Zielen.
 * fight_path_search(f, actor, x, y, mode, arg): Breitensuche (g_fig_dirs_search) bis Distanz 48 (Modi 0..3/9..11
 *   = Gegner; Felder FIG_CELL_BLOCKED Ausgang, FIG_CELL_GOAL/Zielmarken Sonderziele, >= FIG_CELL_OBJ blockiert
 *   (FIG_CELL_WALL), Helden < 10, Gegner 10..49 mit fight_check_state/enemy_is_renegade_on_field), Zielmarken
 *   fig_prepare_search/fig_mark_hero_targets/fig_mark_enemy_targets, bestes Ziel nach Richtungswechseln,
 *   Zielfeld nach g_fig_target_x/y, Rueckgabe Kennung des Ziels (0 = keins).
 *
 * BCC -mlarge -3 -Ot -Og -Oi -c -Y OVL146.CPP   -> 4188/4188 Bytes GLEICH (18.10.2026), cobjfix laeuft
 *   (_g_6bf6/_g_6c06 -> _g_6be6+0x10/+0x20). Namensfassung mit schweif.h: 19.10.2026 GLEICH, LINKGLEICH.
 * - Tabellenzugriffe "*(tab + y * 25 + x)" (les / add bx,ax / add bx,[x]); "tab[y * 25 + x]" rechnet den Index
 *   erst in ax (Hunderte Abweichungen).
 * - "tab[..] == (unsigned char)(dist - 1)" -> mov al / add al,0ffh / cmp es:[bx],al; mit (signed char) oder
 *   (char) wird daraus dec al.
 * - "&g_enemies[e - 10]" (unsigned char e) -> imul / add ax,[c4f7] / add ax,0fd26h; "g_enemies + e - 10" rechnet in dx.
 * - -Og: Zwischenwerte ty * 25 ([bp-28h]), new_y * 25 / Tabellenoffset / tail_y * 25 ([bp-36h]..[bp-3ah]),
 *   in fig_count_dir_changes Offset path + i in SI; i/n ohne register landen ohne Aufrufe in DX/CX. -Oi: memset/memcpy
 *   (memset(tab, 0xff, 600) -> mov ax,0ffh; memset(path, -1, 50) -> mov ax,0ffffh).
 * - "tx[i] = ty[i] = flag[i] = 0" (rechts zuerst), "fresh = cnt = 0"; Strukturkopien "back = g_fig_dirs_back" als
 *   Anweisung (Aggregate liegen hinter den Skalaren); fig_count_dir_changes steht unten -> far call aus fig_find_path_backtrack.
 * - Die lange Tail-Bedingung ist als Makro TAILOK geschrieben (sechsmal gleich), FOUND(e) = fig_path_add_target-Aufruf.
 * Originalfehler: fig_find_path_backtrack benutzt best uninitialisiert, wenn kein Weg gefunden wird (DSA2 begrenzt nur auf
 *   0..3); die Endmarke -2 ueberschreibt die Endmarke -1 (g_fig_path[i - 1], DSA1 schrieb dahinter); der Tail-Test
 *   auch im ersten Rueckverfolgungsschritt ist wie in DSA1 (dort Endlosschleife bei fliehenden Zweifeld-
 *   gegnern; hier prueft fight_path_search bei Ausgaengen TAILOK, ob der Fehler noch auftreten kann, ist offen);
 *   "entry = *(g_fight_walkmap + ..)" wird nie benutzt. fight_path_search gibt f->search_map frei, ohne den Zeiger zu loeschen.
 */
#include <string.h>
#include "schweif.h"

void fig_find_path_backtrack(FIGHT_PTR f, unsigned char far *tab, signed short x, signed short y, signed short dist,
	signed char bp, signed short mode, signed short two, signed short id)
{
	signed short dup;
	signed short i;
	signed short bx;
	signed short by;
	signed short entry;
	signed short dist_bak;
	signed short at_exit = 0;
	signed short tx;
	signed short ty;
	signed short dir;
	signed short ok;
	signed short lowest;
	signed short nr;
	signed short best;
	signed char far *path;
	signed short x_bak;
	signed short y_bak;
	signed short short_bp;
	struct dirs4 back;
	signed char far *paths[4];

	back = g_fig_dirs_back;
	short_bp = 0;
	if (g_quick_fight != 0)
		game_idle();
#if FIX_0079
	/* Fix #79: best vorbelegen (kein Weg -> Weg 0 statt Zufall) */
	lowest = 99;
	best = -1;
	paths[0] = (signed char far *)mem_alloc(50L);
#else
	lowest = 99;
	paths[0] = (signed char far *)mem_alloc(50L);
#endif
	paths[1] = (signed char far *)mem_alloc(50L);
	paths[2] = (signed char far *)mem_alloc(50L);
	paths[3] = (signed char far *)mem_alloc(50L);
	entry = *(g_fight_walkmap + y * 25 + x);
	if (fight_cell_blocked(x, y))
		at_exit = 1;
	dist_bak = dist;
	x_bak = x;
	y_bak = y;
	for (i = 0; i < 4; i++) {
		dist = dist_bak;
		x = x_bak;
		y = y_bak;
		dir = i;
		path = paths[i];
		dup = dist;
		memset(path, -1, 50);
		path[dup] = -1;
		dup--;
		dist--;
		while (dist >= 0) {
			if (g_quick_fight != 0)
				game_idle();
			ok = 0;
			while (ok == 0) {
				by = y + back.d[dir].dy;
				bx = x + back.d[dir].dx;
				ty = by + back.d[dir].dy;
				tx = bx + back.d[dir].dx;
				if (by < 24 && by >= 0 && bx < 24 && bx >= 0) {
					entry = *(tab + by * 25 + bx);
					if (entry == dist &&
						(!two || (two &&
						(!*(f->search_map + ty * 25 + tx) || *(f->search_map + ty * 25 + tx) == id + 10 || *(f->search_map + ty * 25 + tx) == id + 30) &&
						ty < 24 && ty >= 0 && tx < 24 && tx >= 0)))
					{
						y = by;
						x = bx;
						if (bp <= dist) {
							path[dup] = -1;
							dup--;
							short_bp = 1;
						} else {
							path[dup] = dir;
							dup--;
						}
						dist--;
						ok = 1;
					}
				}
				if (ok == 0) {
					dir++;
					if (dir == 4)
						dir = 0;
				}
			}
		}
		if (bp >= dist_bak && at_exit == 0 && mode < 9)
			path[dist_bak - 1] = -1;
		if (path[0] != -1) {
			nr = fig_count_dir_changes(f, path);
			if (nr < lowest) {
				best = i;
				lowest = nr;
				if (nr == 0)
					break;
			}
		}
	}
	if (best < 0 || best > 3)
		best = 0;
	memcpy(g_fig_path, paths[best], 50);
	if (short_bp != 0) {
		i = 0;
#if FIX_0079
		/* Fix #79: Marke -2 hinter die Endmarke -1 (wie DSA 1), nicht darueber */
		while (g_fig_path[i++] != -1);
		if (i < 50)
			g_fig_path[i] = -2;
#else
		while (g_fig_path[i++] != -1);
		g_fig_path[i - 1] = -2;
#endif
	}
	mem_free(paths[0]);
	mem_free(paths[1]);
	mem_free(paths[2]);
	mem_free(paths[3]);
	if (g_quick_fight != 0)
		game_idle();
}

signed short fig_count_dir_changes(FIGHT_PTR f, signed char far *path)
{
	signed short i = 0;
	signed short n = 0;

	if (path[0] == -1)
		return 99;
	i++;
	while (path[i] != -1) {
		if (path[i - 1] != path[i])
			n++;
		i++;
	}
	return n;
}

signed short fig_path_add_target(FIGHT_PTR f, signed short far *n, signed short far *flag, signed short far *tx,
	signed short far *ty, unsigned char far *list, signed short x, signed short y, unsigned char e)
{
	if (g_quick_fight != 0)
		game_idle();
	flag[*n] = 1;
	tx[*n] = x;
	ty[*n] = y;
	if (e == FIG_CELL_BLOCKED || e == FIG_CELL_GOAL)
		list[*n] = e;
	else if (e >= FIG_CELL_HERO_GOAL && e < FIG_CELL_ENEMY_GOAL)
		list[*n] = e + 0x33;
	else if (e >= FIG_CELL_ENEMY_GOAL && e < (FIG_CELL_ENEMY_GOAL + NR_ENEMIES))
		list[*n] = e + 0x35;
	else if (e < 10)
		list[*n] = e;
	else if (e < 50)
		list[*n] = e;
	(*n)++;
	if (*n == 10)
		return 1;
	return 0;
}

#define TAILOK (!enemy || (enemy && (!two || (two != 0 && \
	(!*(f->search_map + tail_y * 25 + tail_x) || *(f->search_map + tail_y * 25 + tail_x) == id + 10 || *(f->search_map + tail_y * 25 + tail_x) == id + 30) && \
	tail_y < 24 && tail_y >= 0 && tail_x < 24 && tail_x >= 0))))
#define FOUND(e) fig_path_add_target(f, &cnt, flag, tx, ty, list, new_x, new_y, e)

unsigned char fight_path_search(FIGHT_PTR f, void far *actor, signed short x0, signed short y0, signed short mode,
	signed short arg)
{
	signed short id;
	signed short reached = 0;
	signed short dist = 0;
	signed short fresh;
	signed short i;
	signed short new_x;
	signed short new_y;
	signed short x;
	signed short y;
	signed short tail_x;
	signed short tail_y;
	unsigned char list[10];
	signed short nr;
	signed short lowest;
	signed short cnt;
	signed short best;
	signed char d;
	unsigned char e;
	signed char far *tab;
	struct enemy far *enemy;
	signed char two;
	signed short tx[10];
	signed short ty[10];
	signed short flag[10];
	struct dirs4 dirs;

	dirs = g_fig_dirs_search;
	enemy = 0;
	two = 0;
	if (g_quick_fight != 0)
		game_idle();
	if ((mode >= 0 && mode <= 3) || (mode >= 9 && mode <= 11)) {
		enemy = (struct enemy far *)actor;
		id = fight_obj_index(enemy);
		if (enemy_is_twofielded(enemy))
			two = 1;
	} else {
		id = get_hero_index((struct hero far *)actor);
	}
	tab = (signed char far *)mem_alloc(600L);
	f->search_map = (unsigned char far *)mem_alloc(600L);
	fresh = 1;
	memset(tab, 0xff, 600);
	*(tab + y0 * 25 + x0) = 0;
	memcpy(f->search_map, g_fight_walkmap, 600);
	for (i = 0; i < 10; i++)
		tx[i] = ty[i] = flag[i] = 0;
	fig_prepare_search(f, mode);
	if (mode == 11 || mode == 9 || mode == 14 || mode == 12)
		fig_mark_hero_targets(f, mode, arg, id);
	if (mode == 10 || mode == 9 || mode == 13 || mode == 12)
		fig_mark_enemy_targets(f, mode, arg, id);
	while (reached == 0 && fresh != 0 && dist < 49) {
		dist++;
		fresh = cnt = 0;
		for (y = 0; y < 24; y++) {
			if (cnt == 10)
				break;
			for (x = 0; x < 24; x++) {
				if (g_quick_fight != 0)
					game_idle();
				if (cnt == 10)
					break;
				if (*(tab + y * 25 + x) == (unsigned char)(dist - 1)) {
					for (i = 0; i < 4; i++) {
						new_y = y + dirs.d[i].dy;
						new_x = x + dirs.d[i].dx;
						tail_y = y - dirs.d[i].dy;
						tail_x = x - dirs.d[i].dx;
						if (new_y < 24 && new_y >= 0 && new_x < 24 && new_x >= 0) {
							d = *(tab + new_y * 25 + new_x);
							e = *(f->search_map + new_y * 25 + new_x);
							if (d == -1) {
								if (e == 0) {
									if (TAILOK) {
										*(tab + new_y * 25 + new_x) = dist;
										fresh = 1;
									}
								} else if (e == FIG_CELL_BLOCKED) {
									if (mode == 3 || mode == 7) {
										if (TAILOK) {
											if (FOUND(e))
												break;
										}
									} else {
										*(tab + new_y * 25 + new_x) = FIG_CELL_WALL;
									}
								} else if (e == FIG_CELL_GOAL) {
									if (FOUND(e))
										break;
								} else if (e >= FIG_CELL_HERO_GOAL && e < FIG_CELL_ENEMY_GOAL) {
									if (FOUND(e))
										break;
								} else if (e >= FIG_CELL_ENEMY_GOAL && e < (FIG_CELL_ENEMY_GOAL + NR_ENEMIES)) {
									if (FOUND(e))
										break;
								} else if (e >= 50) {
									*(tab + new_y * 25 + new_x) = FIG_CELL_WALL;
								} else if (e < 10) {
									if (mode == 0 || mode == 6 || mode == 2 || mode == 4) {
										if (TAILOK) {
											if (FOUND(e))
												break;
										}
									}
								} else if (e < 50) {
									if ((mode == 0 && e < 30) || mode == 5 || (mode == 1 && e < 30) || mode == 4) {
										if (TAILOK) {
											if (mode == 1 || mode == 5) {
												if ((e >= 10 && e < 30 && fight_check_state(&g_enemies[e - 10])) ||
													(e >= 30 && e < 50 && fight_check_state(&g_enemies[e - 30])))
												{
													if (FOUND(e))
														break;
												}
											} else {
												if ((e >= 10 && e < 30 && enemy_is_renegade_on_field(&g_enemies[e - 10])) ||
													(e >= 30 && e < 50 && enemy_is_renegade_on_field(&g_enemies[e - 30])))
												{
													if (FOUND(e))
														break;
												}
											}
										}
									}
								}
							}
						} else {
							if (mode == 3 || mode == 7) {
								if (TAILOK) {
									if (FOUND(FIG_CELL_BLOCKED))
										break;
								}
							}
						}
					}
				}
			}
		}
		if (cnt != 0) {
			reached = 1;
			best = 0;
			lowest = 99;
			if (g_quick_fight != 0)
				game_idle();
			for (i = 0; i < cnt; i++) {
				if (enemy != 0)
					fig_find_path_backtrack(f, (unsigned char far *)tab, tx[i], ty[i], dist, ((struct enemy far *)actor)->bp, mode, two, id);
				else
					fig_find_path_backtrack(f, (unsigned char far *)tab, tx[i], ty[i], dist, ((struct hero far *)actor)->bp_left, mode, two, id);
				nr = fig_count_dir_changes(f, g_fig_path);
				if (nr == 0) {
					best = i;
					break;
				}
				if (nr < lowest) {
					lowest = nr;
					best = i;
				}
			}
			if (enemy != 0)
				fig_find_path_backtrack(f, (unsigned char far *)tab, tx[best], ty[best], dist, ((struct enemy far *)actor)->bp, mode, two, id);
			else
				fig_find_path_backtrack(f, (unsigned char far *)tab, tx[best], ty[best], dist, ((struct hero far *)actor)->bp_left, mode, two, id);
		}
	}
#if FIX_0079
	/* Fix #79: Zeiger nach dem Freigeben nullen */
	mem_free(tab);
	f->search_map = (unsigned char far *)mem_free(f->search_map);
#else
	mem_free(tab);
	mem_free(f->search_map);
#endif
	if (reached != 0) {
		g_fig_target_x = tx[best];
		g_fig_target_y = ty[best];
		return list[best];
	} else
		return 0;
}
