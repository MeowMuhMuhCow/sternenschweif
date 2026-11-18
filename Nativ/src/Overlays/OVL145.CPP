/* OVL145 - Overlay O145 (Segment 0x2241 in SCHWEIF.EXE): Zielfelder und Sichtlinien im Kampf. Kampffeld 24 x 24
 * (Zeilen zu 25 Bytes) als Suchkarte f->search_map bzw. g_fight_walkmap, Sichtlinie f->los_line (struct point).
 * fig_mark_hero_targets(f, mode, min_dist, skip): Helden in Zufallsreihenfolge (random_schweif, Merkfeld 7
 *   Worte); fuer jeden in der Gruppe (group_has_hero) mit Feld (fight_find_cell) alle leeren Felder mit Abstand
 *   >= min_dist (manhattan_dist) und freier Sichtlinie (fig_line_of_sight, Reichweite 99) mit FIG_CELL_HERO_GOAL +
 *   Held markieren; Modi 12/14 ueberspringen Held skip. fig_mark_enemy_targets: dasselbe fuer 20 Gegner (Modi
 *   10/13 fight_check_state, 9/12 enemy_is_renegade_on_field, Marke FIG_CELL_ENEMY_GOAL + Gegner).
 * fig_prepare_search(f, mode): Suchkarte vorbehandeln - Helden tot/bewusstlos -> frei, versteinert ->
 *   FIG_CELL_WALL (in Modus 5 = g_fight_end fremde handlungsfaehige Helden gesperrt), Gegner 10..49 (tot: nur
 *   enemy_blocks sperrt, versteinert); Modus 8 mit umgekehrtem Versteinert-Test.
 * bresenham_line(x0, y0, x1, y1, buf): Bresenham-Linie nach buf, ggf. umgedreht (beginnt bei (x0, y0)), Ende
 *   (-1, -1), Rueckgabe Punktzahl.
 * fig_line_free(f, x, y, buf, range): Linie pruefen (Objekte >= FIG_CELL_OBJ ausser fig_obj_transparent und
 *   FIG_CELL_BLOCKED, Helden ausser hero_is_inactive, Gegner mit enemy_blocks; FIG_CELL_SFIGHT und Feld 10 beim
 *   Sondergegner zaehlen als frei). fig_find_sfight_cell(f, x, y, &tx, &ty, mode): Feld der Sonderfigur
 *   (g_sfight_cells, bis x == -1) mit Sichtlinie suchen, Modus 1 = Nahkampf (Abstand 1).
 * fig_line_of_sight(f, x, y, tx, ty, mode): Sichtlinie (Modus 1: gleiche Zeile/Spalte und Abstand <= 1).
 * fig_cb_select_target(f, &x, &y, n): Zielauswahl mit Cursor (Maus g_mouse_posx/y in Feldkoordinaten mit Abrunden,
 *   g_action Pfeile/KEY_RETURN/KEY_ESC), Bahn als Marken (path_marks, fig_list_add/fig_list_remove), Ziel nach
 *   f->bottom_hero/bottom_enemy; ohne gueltiges Ziel Felder im Umkreis von bp_left mit fig_move_check markieren
 *   (reach_marks), Meldung txt[0x60] bzw. txt[0x61], Rueckgabe 0xff; sonst Feldwert des Ziels (FIG_CELL_SFIGHT ->
 *   10 mit Umsetzen per fight_set_cell). (BE FIG_cb_select_target)
 *
 * BCC -mlarge -3 -Ot -Og -Oi -c -Y OVL145.CPP   -> 6548/6548 Bytes GLEICH (18.10.2026), cobjfix laeuft
 *   (Namensfassung mit schweif.h: 19.10.2026 GLEICH, LINKGLEICH)
 * - fig_prepare_search: "e - 10 - (e >= 30 ? 20 : 0)" zweimal im Quelltext -> -Og-Wort-Zwischenwert [bp-6] (eine eigene
 *   Variable idx rechnet anders: Bedingung vor dem push); Bitbytes +bc als -Og-Bytes [bp-2]/[bp-3]; der Helden-
 *   zeiger muss im inneren Block stehen (liegt hinter den Zwischenwerten); "!(signed char)h->dead" -> cbw, ohne
 *   Cast nur or al,al; g_mouse_locked (unsigned char) als "!(signed char)g_mouse_locked" (cbw).
 * - fig_mark_enemy_targets: "if (A || B) { Rest }" statt "if (!(A || B)) continue;" (jz short / jmp near).
 * - fig_cb_select_target: SI als Zaehler nur mit register-Variable im inneren Block - auf Funktionsebene braucht BCC SI fuer
 *   "f->tmpl.cbx = *px" (les si) und schiebt cnt (auch mit -rd) auf den Stack; Abrundung "t = ..; if (t < 0) t -= 19;
 *   t / 20" mit t im Block (CX); "if (.. != 0xfd) {..} else break;" (jz auf jmp short); Schlussteil als
 *   "if (!ok) {..; return 0xff;} else {..; return e;}" (zwei tote Epiloge); Marken-Loeschschleife und
 *   Cursorsetzen als Makros (je dreimal bzw. zweimal gleich, -Og-Zwischenwert f + i in [bp-1ch]).
 * - bresenham_line: x/y als Tauschvariable wiederverwendet, abs() inline (Kopien [bp-1ch]/[bp-1eh]).
 * Auffaellig (Original): fig_cb_select_target sucht in der Umkreissuche nur sx - r .. sx + r - 1 (Obergrenze exklusiv) und
 *   uebergibt bei Sonderfeldern tx/ty per Zeiger an fig_find_sfight_cell, das sie ueberschreibt (folgende Felder pruefen
 *   gegen das veraenderte Ziel); im Verschiebezweig wird das Ergebnis von fig_list_add(&f->list, f->mark_id2) verworfen
 *   (Aufruf mit char-Argument ueber FIG_LIST_ADD_C).
 */
#include <string.h>
#include <stdlib.h>
#include "schweif.h"

/* fig_list_add mit char-Argument (Original: push ax ohne cbw) */
#define FIG_LIST_ADD_C(l, id) ((signed char (*)(void far *, signed char))fig_list_add)(l, id)

#define CB(f, x, y) (*((f)->search_map + (y) * FIG_MAP_PITCH + (x)))

void fig_mark_hero_targets(FIGHT_PTR f, signed short mode, signed short min_dist, signed short skip)
{
	signed short i;
	signed short x;
	signed short y;
	signed short r;
	signed short k;
	struct hero far *h;
	signed short used[7];
	register signed short c;

	memset(used, 0, sizeof(used));
	for (i = 0; i <= 6; i++) {
		if (g_quick_fight)
			game_idle();
		do {
			k = random_schweif(7) - 1;
		} while (used[k]);
		used[k] = 1;
		if ((mode == 14 || mode == 12) && k == skip)
			continue;
		h = &g_heroes[k];
		if (!group_has_hero(g_group, h))
			continue;
		if (!fight_find_cell(k + FIG_ID_HERO, &x, &y))
			continue;
		if (CB(f, x, y) == 0)
			continue;
		if (CB(f, x, y) == FIG_CELL_WALL)
			continue;
		for (r = 0; r < 24; r++)
			for (c = 0; c < 24; c++)
				if (!(unsigned char)CB(f, c, r) && manhattan_dist(c, r, x, y) >= min_dist
					&& fig_line_of_sight(f, c, r, x, y, 99))
					CB(f, c, r) = k + FIG_CELL_HERO_GOAL;
	}
}

void fig_mark_enemy_targets(FIGHT_PTR f, signed short mode, signed short min_dist, signed short skip)
{
	signed short i;
	struct enemy far *e;
	signed short x;
	signed short y;
	signed short r;
	signed short k;
	signed short used[20];
	register signed short c;

	memset(used, 0, sizeof(used));
	for (i = 0; i < 20; i++) {
		if (g_quick_fight)
			game_idle();
		do {
			k = random_schweif(20) - 1;
		} while (used[k]);
		used[k] = 1;
		if ((mode == 10 || mode == 9) && k == skip)
			continue;
		e = &g_enemies[k];
		if (((mode == 10 || mode == 13) && fight_check_state(e)) || ((mode == 9 || mode == 12) && enemy_is_renegade_on_field(e))) {
			if (!fight_find_cell(k + FIG_ID_ENEMY, &x, &y))
				continue;
			if (CB(f, x, y) == 0)
				continue;
			if (CB(f, x, y) == FIG_CELL_WALL)
				continue;
			for (r = 0; r < 24; r++)
				for (c = 0; c < 24; c++)
					if (!(unsigned char)CB(f, c, r) && manhattan_dist(c, r, x, y) >= min_dist
						&& fig_line_of_sight(f, c, r, x, y, 99))
						CB(f, c, r) = k + FIG_CELL_ENEMY_GOAL;
		}
	}
}

#define EIDX(e)	/* Gegnerindex aus Feldwert (Kopf 10.., Schwanz 30..) */ ((e) - 10 - ((e) >= 30 ? 20 : 0))

void fig_prepare_search(FIGHT_PTR f, signed short mode)
{
	unsigned char e;
	register signed short y;
	register signed short x;

	if (mode != 8) {
		for (y = 0; y < 24; y++)
			for (x = 0; x < 24; x++) {
				e = CB(f, x, y);
				if (g_quick_fight)
					game_idle();
				if (e > 0) {
					if (g_fight_end == 5 && e < 10) {
						struct hero far *h;

						h = &g_heroes[e - 1];
						if (get_first_active_hero(g_group) != h) {
							if (!(signed char)h->dead && !(signed char)h->unconscious) {
								CB(f, x, y) = FIG_CELL_WALL;
								e = 50;
							}
						}
					}
					if (e < 10 && ((signed char)g_heroes[e - 1].dead || (signed char)g_heroes[e - 1].unconscious)) {
						CB(f, x, y) = 0;
					} else if (e < 10 && (signed char)g_heroes[e - 1].petrified) {
						CB(f, x, y) = FIG_CELL_WALL;
					} else if (e >= 10 && e < 50) {
						if ((signed char)g_enemies[EIDX(e)].dead) {
							if (enemy_blocks(&g_enemies[EIDX(e)]))
								CB(f, x, y) = FIG_CELL_WALL;
							else
								CB(f, x, y) = 0;
						} else if ((signed char)g_enemies[EIDX(e)].petrified)
							CB(f, x, y) = FIG_CELL_WALL;
					}
				}
			}
	} else {
		for (y = 0; y < 24; y++)
			for (x = 0; x < 24; x++) {
				e = CB(f, x, y);
				if (g_quick_fight)
					game_idle();
				if (e > 0) {
					if (e < 10 && ((signed char)g_heroes[e - 1].dead || (signed char)g_heroes[e - 1].unconscious)) {
						CB(f, x, y) = 0;
					} else if (e < 10 && !(signed char)g_heroes[e - 1].petrified) {
						CB(f, x, y) = FIG_CELL_WALL;
					} else if (e >= 10 && e < 50) {
						if ((signed char)g_enemies[EIDX(e)].dead) {
							if (enemy_blocks(&g_enemies[EIDX(e)]))
								CB(f, x, y) = FIG_CELL_WALL;
							else
								CB(f, x, y) = 0;
						} else
							CB(f, x, y) = FIG_CELL_WALL;
					}
				}
			}
	}
}

signed short bresenham_line(signed short x0, signed short y0, signed short x1, signed short y1, struct point far *buf)
{
	signed short adx;
	signed short ady;
	signed short err;
	signed short d2;
	signed short stepx;
	signed short stepy;
	signed short inc1;
	signed short inc2;
	signed short endx;
	signed short endy;
	signed short k;
	signed short ox;
	signed short oy;
	register signed short n;
	register signed short x;
	register signed short y;

	n = 0;
	ox = x1;
	oy = y1;
	adx = abs(x1 - x0);
	ady = abs(y1 - y0);
	endx = x1;
	endy = y1;
	if (adx == 0) {
		if (y0 > y1) {
			y = y0;
			y0 = y1;
			y1 = y;
			endy = y1;
		}
		for (y = y0; y < y1; y++, n++) {
			buf[n].x = x0;
			buf[n].y = y;
		}
	} else if (ady == 0) {
		if (x0 > x1) {
			x = x0;
			x0 = x1;
			x1 = x;
			endx = x1;
		}
		for (x = x0; x < x1; n++) {
			buf[n].x = x++;
			buf[n].y = y0;
		}
	} else if (adx >= ady) {
		if (x0 > x1) {
			endx = x0;
			endy = y0;
			x = x1;
			y = y1;
			if (y1 > y0)
				stepy = -1;
			else
				stepy = 1;
		} else {
			x = x0;
			y = y0;
			if (y1 > y0)
				stepy = 1;
			else
				stepy = -1;
		}
		d2 = ady * 2;
		err = d2 - adx;
		inc1 = d2;
		inc2 = (ady - adx) * 2;
		for (; adx > 0; adx--) {
			buf[n].x = x;
			buf[n].y = y;
			n++;
			x++;
			if (err >= 0) {
				err += inc2;
				y += stepy;
			} else
				err += inc1;
		}
	} else {
		if (y0 > y1) {
			endx = x0;
			endy = y0;
			x = x1;
			y = y1;
			if (x1 > x0)
				stepx = -1;
			else
				stepx = 1;
		} else {
			x = x0;
			y = y0;
			if (x1 > x0)
				stepx = 1;
			else
				stepx = -1;
		}
		d2 = adx * 2;
		err = d2 - ady;
		inc1 = d2;
		inc2 = (adx - ady) * 2;
		for (; ady > 0; ady--) {
			buf[n].x = x;
			buf[n].y = y;
			n++;
			y++;
			if (err >= 0) {
				err += inc2;
				x += stepx;
			} else
				err += inc1;
		}
	}
	buf[n].x = endx;
	buf[n].y = endy;
	if (buf[0].x == ox && buf[0].y == oy) {
		for (k = 0; n / 2 > k; k++) {
			adx = buf[n - k].x;
			ady = buf[n - k].y;
			buf[n - k].x = buf[k].x;
			buf[n - k].y = buf[k].y;
			buf[k].x = adx;
			buf[k].y = ady;
		}
	}
	n++;
	buf[n].x = -1;
	buf[n].y = -1;
	return n;
}

signed short fig_line_free(FIGHT_PTR f, signed short x, signed short y, struct point far *buf, signed short range)
{
	signed short ok;
	signed short d;
	register signed short e;
	register signed short i;

	ok = 1;
	for (i = 1; ok == 1 && buf[i + 1].x != -1; i++) {
		if (g_quick_fight)
			game_idle();
		d = manhattan_dist(x, y, buf[i].x, buf[i].y);
		e = *(g_fight_walkmap + buf[i].y * 25 + buf[i].x);
		if (e == FIG_CELL_SFIGHT || (e == 10 && enemy_type_check(g_enemies)))
			e = 0;
		if (d > 0 && d <= range) {
			if ((e >= 50 && !fig_obj_transparent(f, e - 50) && e != 0xfe)
				|| (e > 0 && e < 10 && !hero_is_inactive(&g_heroes[e - 1]))
				|| (e >= 10 && e < 30 && enemy_blocks(&g_enemies[e - 10]))
				|| (e >= 30 && e < 50 && enemy_blocks(&g_enemies[e - 30]))) {
				ok = 0;
				break;
			}
		} else if (d != 0) {
			ok = 0;
			break;
		}
	}
	return ok;
}

signed short fig_find_sfight_cell(FIGHT_PTR f, signed short x, signed short y, signed short far *px, signed short far *py,
	signed short mode)
{
	signed short ok;
	signed short ty;
	register signed short i;
	register signed short tx;

	if (mode == 1) {
		tx = *px;
		ty = *py;
		if (x != tx && y != ty)
			ok = 0;
		else if (manhattan_dist(x, y, tx, ty) != 1)
			ok = 0;
		else
			ok = 1;
	} else {
		ok = 0;
		for (i = 0; g_sfight_cells[i].x != -1; i++) {
			tx = g_sfight_cells[i].x;
			ty = g_sfight_cells[i].y;
			if (fig_line_of_sight(f, x, y, tx, ty, mode)) {
				ok = 1;
				break;
			}
		}
	}
	if (ok == 1) {
		*px = tx;
		*py = ty;
	}
	return ok;
}

signed short fig_line_of_sight(FIGHT_PTR f, register signed short x, register signed short y, signed short tx,
	signed short ty, signed short mode)
{
	signed short r;

	if (mode == 1) {
		if (x != tx && y != ty)
			r = 0;
		else if (manhattan_dist(x, y, tx, ty) > 1)
			r = 0;
		else
			r = 1;
	} else {
		bresenham_line(x, y, tx, ty, f->los_line);
		r = fig_line_free(f, x, y, f->los_line, mode);
	}
	return r;
}

#define CLEARMARKS() \
	for (i = 0; f->path_marks[i] != -1; i++) { \
		fig_list_remove(&f->list, f->path_marks[i], 0); \
		f->path_marks[i] = -1; \
	}

#define SETCURSOR() \
	if (special) { \
		if (f->mark_id2 != -1) \
			fig_list_remove(&f->list, f->mark_id2, 0); \
		f->tmpl.cbx = tx; \
		f->tmpl.cby = ty; \
		f->mark_id2 = -1; \
	} else if (f->mark_id2 != -1) { \
		fig_list_remove(&f->list, f->mark_id2, 1); \
		f->tmpl.cbx = tx; \
		f->tmpl.cby = ty; \
		FIG_LIST_ADD_C(&f->list, f->mark_id2); \
	} else { \
		f->tmpl.cbx = tx; \
		f->tmpl.cby = ty; \
		f->mark_id2 = fig_list_add(&f->list, -1); \
	}

unsigned char fig_cb_select_target(FIGHT_PTR f, signed short far *px, signed short far *py, signed short n)
{
	signed short sx;
	signed short sy;
	signed short cx;
	signed short cy;
	signed short tx;
	signed short ty;
	unsigned char e;
	signed short mx;
	signed short my;
	signed short quit;
	signed short ok;
	signed short special;
	signed short first;
	register signed short i;

	quit = 0;
	ok = 0;
	special = 0;
	first = 1;
	mouse_show(0);
	mouse_clear_left();
	mouse_clear_right();
	mx = g_mouse_posx;
	my = g_mouse_posy;
	sx = *px;
	sy = *py;
	if (f->mark_id2 != -1) {
		fig_list_remove(&f->list, f->mark_id2, 0);
		f->mark_id2 = -1;
	}
	for (i = 0; i < 30; i++)
		f->path_marks[i] = -1;
	f->tmpl.obj_pic = 0;
	f->tmpl.cbx = *px;
	f->tmpl.cby = *py;
	f->tmpl.offsetx = 0;
	f->tmpl.offsety = 6;
	f->tmpl.kind = 0;
	f->tmpl.anim = 0;
	f->tmpl.gfx = f->pic_cursor;
	f->tmpl.z = 2;
	f->tmpl.visible = 1;
	f->tmpl.twofielded = -1;
	f->mark_id2 = fig_list_add(&f->list, -1);
	fight_draw(f);
	do {
		fight_check_keys(f);
		if (g_action == KEY_ESC || g_mouse_rclick) {
			g_mouse_rclick = 0;
			quit = 1;
			ok = 0;
		} else {
			if (g_mouse_lclick) {
				g_action = KEY_RETURN;
				mouse_clear_left();
			}
			if (g_action == 0 && !(signed char)g_mouse_locked && (mx != g_mouse_posx || my != g_mouse_posy)) {
				mx = g_mouse_posx;
				my = g_mouse_posy;
				{
					signed short t;

					t = mx + my * 2 - 0xd5;
					if (t < 0)
						t -= 19;
					cx = t / 20 - 1;
				}
				{
					signed short t;

					t = mx - my * 2 + 0xfb;
					if (t < 0)
						t -= 19;
					cy = t / 20 - 1;
				}
				if (cx < 0)
					cx = 0;
				if (cx > 23)
					cx = 23;
				if (cy < 0)
					cy = 0;
				if (cy > 23)
					cy = 23;
				if (*px != cx || *py != cy) {
					*px = cx;
					*py = cy;
				}
			}
			if (g_action == KEY_RIGHT)
				(*px)++;
			else if (g_action == KEY_LEFT)
				(*px)--;
			else if (g_action == KEY_UP)
				(*py)++;
			else if (g_action == KEY_DOWN)
				(*py)--;
			if (*px < 0)
				*px = 0;
			if (*px > 23)
				*px = 23;
			if (*py < 0)
				*py = 0;
			if (*py > 23)
				*py = 23;
			if (f->tmpl.cbx != *px || f->tmpl.cby != *py || first == 1) {
				first = 0;
				special = 0;
				e = *(g_fight_walkmap + *py * 25 + *px);
				if (e == FIG_CELL_SFIGHT || (e == 10 && enemy_type_check(g_enemies))) {
					e = 10;
					special = 1;
				}
				f->bottom_enemy = 0;
				f->bottom_hero = 0;
				g_fig_redraw_bottom = 1;
				if (e > 0 && e < 50) {
					if (e < 10)
						f->bottom_hero = &g_heroes[e - 1];
					else if (e >= 30)
						f->bottom_enemy = &g_enemies[e - 30];
					else
						f->bottom_enemy = &g_enemies[e - 10];
				}
				ok = 0;
				tx = *px;
				ty = *py;
				if (special) {
					if (fig_find_sfight_cell(f, sx, sy, &tx, &ty, n))
						ok = 1;
				} else {
					if (fig_line_of_sight(f, sx, sy, tx, ty, n))
						ok = 1;
				}
				if (ok == 1) {
					CLEARMARKS();
					fight_restore_bg(f);
					if (f->bottom_enemy == 0 && f->bottom_hero == 0)
						fight_add_rect(0, 0x91, 0x64, 0xc7);
					if (n > 1) {
						register signed short cnt;

						cnt = 0;
						if (sx != tx || sy != ty) {
							for (i = 1; f->los_line[i + 1].x != -1; i++) {
								if (*(g_fight_walkmap + f->los_line[i].y * 25 + f->los_line[i].x) != FIG_CELL_SFIGHT) {
									f->tmpl.cbx = f->los_line[i].x;
									f->tmpl.cby = f->los_line[i].y;
									f->path_marks[cnt] = fig_list_add(&f->list, -1);
									cnt++;
								} else
									break;
							}
						}
					}
					SETCURSOR();
					fight_draw_figures(f);
					fight_draw_flush(f);
				} else {
					fight_restore_bg(f);
					if (f->bottom_enemy == 0 && f->bottom_hero == 0)
						fight_add_rect(0, 0x91, 0x64, 0xc7);
					CLEARMARKS();
					SETCURSOR();
					fight_draw_figures(f);
					fight_draw_flush(f);
				}
			}
		}
	} while (g_action != KEY_RETURN && !quit);
	if (f->mark_id2 != -1)
		fig_list_remove(&f->list, f->mark_id2, 0);
	f->mark_id2 = -1;
	CLEARMARKS();
	f->bottom_hero = 0;
	f->bottom_enemy = 0;
	fight_restore_bg(f);
	fight_add_rect(0, 0x91, 0x64, 0xc7);
	fight_draw_figures(f);
	fight_draw_flush(f);
	mouse_hide(0);
	if (!ok) {
		if (!quit) {
			signed short res;
			signed short res2;
			signed short r;
			signed short x0;
			signed short y0;
			signed short x1;
			signed short y1;
			struct hero far *h;
			register signed short cnt;

			special = 0;
			e = *(g_fight_walkmap + ty * 25 + tx);
			if (e == FIG_CELL_SFIGHT || (e == 10 && enemy_type_check(g_enemies))) {
				e = 10;
				special = 1;
			}
			if (e > 0 && e < 50) {
				cnt = 0;
				h = &g_heroes[*(g_fight_walkmap + sy * 25 + sx) - 1];
				r = h->bp_left;
				x0 = sx - r;
				y0 = sy - r;
				x1 = sx + r;
				y1 = sy + r;
				if (x0 < 0)
					x0 = 0;
				if (y0 < 0)
					y0 = 0;
				if (x1 > 24)
					x1 = 24;
#if FIX_0200
				/* Fix #200: Zeilen nur bis 23 */
				if (y1 > 23)
					y1 = 23;
#else
				if (y1 > 24)
					y1 = 24;
#endif
				f->tmpl.obj_pic = 0;
				f->tmpl.offsetx = 0;
				f->tmpl.offsety = 6;
				f->tmpl.kind = 0;
				f->tmpl.anim = 0;
				f->tmpl.gfx = f->pic_reach;
				f->tmpl.z = 1;
				f->tmpl.visible = 1;
				f->tmpl.twofielded = -1;
#if FIX_0200
				/* Fix #200: Umkreis symmetrisch bis sx + r bzw. sy + r */
				for (cy = y0; cy <= y1; cy++)
					for (cx = x0; cx <= x1; cx++) {
#else
				for (cy = y0; cy < y1; cy++)
					for (cx = x0; cx < x1; cx++) {
#endif
						if (!(unsigned char)*(g_fight_walkmap + cy * 25 + cx) && manhattan_dist(sx, sy, cx, cy) <= r) {
							ok = 0;
							if (special) {
#if FIX_0200
								/* Fix #200: Ziel tx/ty nicht fuer die folgenden Felder ueberschreiben (Kopie) */
								{
									signed short tx2 = tx;
									signed short ty2 = ty;

									if (fig_find_sfight_cell(f, cx, cy, &tx2, &ty2, n))
										ok = 1;
								}
#else
								if (fig_find_sfight_cell(f, cx, cy, &tx, &ty, n))
									ok = 1;
#endif
							} else {
								if (fig_line_of_sight(f, cx, cy, tx, ty, n))
									ok = 1;
							}
							if (ok) {
								fig_move_check(f, h, &cx, &cy, &sx, &sy, &res, &res2);
								if (res == 0) {
									f->tmpl.cbx = cx;
									f->tmpl.cby = cy;
									f->reach_marks[cnt] = fig_list_add(&f->list, -1);
									cnt++;
								}
							}
						}
					}
				if (cnt > 0)
					fight_status_text(f->txt[0x60]);
				else
					fight_status_text(f->txt[0x61]);
			}
		}
		return 0xff;
	} else {
		e = *(g_fight_walkmap + ty * 25 + tx);
		if (e == FIG_CELL_SFIGHT) {
			fight_find_cell(10, &cx, &cy);
			fight_set_cell(f, cx, cy, FIG_CELL_SFIGHT);
			fight_set_cell(f, tx, ty, 10);
			e = 10;
		}
		*px = tx;
		*py = ty;
		return e;
	}
}
