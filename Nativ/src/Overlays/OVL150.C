/* OVL150 - Overlay O150 (Segment 0x2258 in SCHWEIF.EXE): Held im Kampf bewegen (entspricht DSA1 seg034
 * FIG_move_pathlen/FIG_move_hero, in DSA2 umgebaut). fig_move_pathlen(f): Laenge des Wegs g_fig_path bis -1, 99
 * wenn danach FIG_PATH_SHORT folgt. fig_move_check(f, h, sx, sy, px, py, problem, cost): Zielfeld (sx, sy) pruefen -
 * Flucht ueber den Rand (ESCAPE_LEFT/_DOWN/_UP), Feldinhalt aus g_fight_walkmap [y][25], Weg suchen (Zielmarke
 * FIG_CELL_GOAL per fight_set_cell, fight_path_search Modus 15, fig_move_pathlen), Fluchtschritt anhaengen;
 * problem MOVE_NO_PATH, MOVE_TOO_FAR (h->bp_left), MOVE_ESCAPE (fight_cell_blocked), MOVE_BLOCKED (>= 50,
 * lebender Gegner per enemy_blocks, lebender anderer Held).
 * fig_move_hero(f, h, px, py): Auswahlschleife mit Feldcursor (Vorlage f->tmpl mit pic_cursor, Liste f->list, ID
 * f->mark_id2), Pfeiltasten bzw. Maus (g_mouse_posx/y -> Feld mit Abrunden auf 20er, ACTION_MOUSE_MOVED), Anzeige
 * "Ziel: n BP"/Meldung (f->txt) in g_fig_text; KEY_RETURN/KEY_ESC beendet, bei Flucht Rueckfrage (dlg_yes_no ->
 * h->scared = 1), fig_hero_place und fight_find_cell(Nr + 1, px, py).
 *
 * BCC -mlarge -3 -Ot -Og -Oi -c -Y OVL150.C   -> 2677/2677 Bytes GLEICH (18.10.2026), cobjfix laeuft
 *   (Namensfassung mit schweif.h: 19.10.2026 GLEICH, LINKGLEICH; "h->scared = 1" ergibt dasselbe or byte wie
 *   frueher "h->fbd |= 1", g_mouse_locked als (signed char))
 * - Reines C. -Og: Epilogkopien bei den returns in fig_move_check, Zwischenwerte [bp-6] (*sy), [bp-8] (Feldoffset),
 *   [bp-0ah] (c1 als int), [bp-0bh] (Statusbyte des Helden); toter Epilog in fig_move_pathlen.
 * - Aeussere Bedingungen in fig_move_check als "if (..) { .. }" (jmp near ans Funktionsende), nur echte returns
 *   werden mit -Og zu Epilogkopien.
 * - Abrunden der Mausposition: Zwischenwert t im INNEREN Block (ohne Aufrufe) -> CX ("mov cx,dx / or cx,cx /
 *   jnl / sub cx,13h"); auf Funktionsebene landet t anders (1062 Abweichungen); mit oder ohne register gleich.
 * - fight_set_cell hat einen unsigned-char-Parameter: Konstante 0xcb -> push 0cbh (als signed char: push byte -35h).
 * - Heldenstatus "!(signed char)g_heroes[c1 - 1].dead" mit unsigned-short-Bitfeldern (1 Byte) -> and ax,1 /
 *   cbw / or ax,ax, Adresse gefaltet zu [bx-533h]. Auch in C (kein C++ noetig).
 * - get_hero_index/fight_path_search liefern unsigned char (mov ah,0); strcpy als -Oi-Intrinsic (zwei Formen: Quelle aus
 *   Struktur bzw. far-Zeiger-Variable -> far-Zeiger-Variable).
 * - Zuweisungsketten "curr_x = sel_x = *px" (sel_x zuerst gespeichert).
 * Originalfehler: fig_move_check liest c1 (Inhalt des Zielfelds) auch bei Flucht, wo nur c2 gesetzt wurde
 *   (uninitialisiert, wie DSA1 cb_entry_bak); ok ist uninitialisiert, wenn der Held schon am Rand steht und
 *   fliehen will (Zweig ohne Wegsuche). Der Fluchtschritt-Test ist asymmetrisch (links bp > cost, sonst
 *   bp - 1 > cost), wie in DSA1.
 */
#include <stdio.h>
#include <string.h>
#include "schweif.h"

#define WALKMAP ((signed char (far *)[FIG_MAP_PITCH])g_fight_walkmap)	/* Begehbarkeitskarte als [24][25] */

signed short fig_move_pathlen(FIGHT_PTR f)
{
	register signed short i = 0;

	while (g_fig_path[i] != -1) i++;
	if (g_fig_path[i + 1] == -2) return 99;
	return i;
}

void fig_move_check(FIGHT_PTR f, struct hero far *h, signed short far *sx, signed short far *sy,
	signed short far *px, signed short far *py, signed short far *problem, signed short far *cost)
{
	unsigned char c1;
	unsigned char c2;
	signed short ok;
	register signed short esc;
	register signed short i;

	*problem = MOVE_OK;
	*cost = 0;
	if (*sx != *px || *sy != *py) {
		esc = ESCAPE_NONE;
		if (*sx < 0 || *sy < 0 || *sy > 23) {
			if (*sx < 0) {
				*sx = 0;
				esc = ESCAPE_LEFT;
			} else if (*sy < 0) {
				*sy = 0;
				esc = ESCAPE_DOWN;
			} else {
				*sy = 23;
				esc = ESCAPE_UP;
			}
			if (WALKMAP[*sy][*sx] != 0 && (*sx != *px || *sy != *py)) {
				*problem = MOVE_BLOCKED;
				if (esc == ESCAPE_LEFT) *sx = -1;
				else if (esc == ESCAPE_DOWN) *sy = -1;
				else *sy = 24;
			} else {
				c2 = WALKMAP[*sy][*sx];
			}
		} else {
			c1 = WALKMAP[*sy][*sx];
		}
		if (*problem != MOVE_BLOCKED) {
			if (esc && *sx == *px && *sy == *py) {
#if FIX_0018
				/* Fix #18: Flucht vom Randfeld: Weg ist vorhanden (ok war uninitialisiert) */
				g_fig_path[0] = -1;
				*cost = 0;
				ok = 1;
#else
				g_fig_path[0] = -1;
				*cost = 0;
#endif
			} else {
				fight_set_cell(f, *sx, *sy, FIG_CELL_GOAL);
				ok = fight_path_search(f, h, *px, *py, 0x0f, 1);
				*cost = fig_move_pathlen(f);
			}
			if (esc) {
				fight_set_cell(f, *sx, *sy, c2);
				for (i = 0; g_fig_path[i] != -1; i++);
				if (esc == ESCAPE_LEFT) {
					*sx = -1;
					if (h->bp_left > *cost) {
						g_fig_path[i] = 2;
						g_fig_path[i + 1] = -1;
					}
				} else if (esc == ESCAPE_DOWN) {
					*sy = -1;
					if (h->bp_left - 1 > *cost) {
						g_fig_path[i] = 1;
						g_fig_path[i + 1] = -1;
					}
				} else {
					*sy = 24;
					if (h->bp_left - 1 > *cost) {
						g_fig_path[i] = 3;
						g_fig_path[i + 1] = -1;
					}
				}
				(*cost)++;
			} else {
				fight_set_cell(f, *sx, *sy, c1);
			}
			if (ok == 0) {
				*problem = MOVE_NO_PATH;
				return;
			}
			if (h->bp_left < *cost) {
				*problem = MOVE_TOO_FAR;
				return;
			}
			if (fight_cell_blocked(*sx, *sy)) {
				*problem = MOVE_ESCAPE;
				return;
			}
			if (c1 >= 50) {
				*problem = MOVE_BLOCKED;
				return;
			}
			if (c1 >= 10) {
				if (enemy_blocks(&g_enemies[c1 - 10 - (c1 >= 30 ? 20 : 0)])) *problem = MOVE_BLOCKED;
				return;
			}
			if (c1 >= 1) {
				if (!(signed char)g_heroes[c1 - 1].dead && !(signed char)g_heroes[c1 - 1].unconscious &&
					c1 != get_hero_index(h) + 1)
					*problem = MOVE_BLOCKED;
			}
		}
	}
}

void fig_move_hero(FIGHT_PTR f, struct hero far *h, signed short far *px, signed short far *py)
{
	signed short sel_x;
	signed short sel_y;
	signed short curr_x;
	signed short curr_y;
	signed short problem;
	signed short cost;
	signed short mx;
	signed short my;
	signed short mbx;
	signed short mby;

	mouse_clear_left();
	mouse_clear_right();
	curr_x = sel_x = *px;
	curr_y = sel_y = *py;
	mouse_show(0);
	mbx = g_mouse_posx;
	mby = g_mouse_posy;
	if (f->mark_id2 != -1) {
		fig_list_remove(&f->list, f->mark_id2, 0);
		f->mark_id2 = -1;
	}
	f->tmpl.obj_pic = 0;
	f->tmpl.cbx = sel_x;
	f->tmpl.cby = sel_y;
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
		curr_x = sel_x;
		curr_y = sel_y;
		if (!(signed char)g_mouse_locked) {
			if (mbx != g_mouse_posx || mby != g_mouse_posy) {
				mbx = g_mouse_posx;
				mby = g_mouse_posy;
				{
					signed short t;

					t = mbx + mby * 2 - 0xd5;
					if (t < 0) t -= 19;
					mx = t / 20 - 1;
					t = mbx - mby * 2 + 0xfb;
					if (t < 0) t -= 19;
					my = t / 20 - 1;
				}
				if (mx < -1) mx = -1;
				if (mx > 24) mx = 24;
				if (my < -1) my = -1;
				if (my > 24) my = 24;
				if (mx != sel_x || my != sel_y) g_action = ACTION_MOUSE_MOVED;
			}
		}
		if (g_mouse_lclick) {
			mouse_clear_left();
			g_action = KEY_RETURN;
		}
		if (g_action == KEY_RIGHT && sel_x < 23) sel_x++;
		else if (g_action == KEY_LEFT && sel_x >= 0) sel_x--;
		else if (g_action == KEY_UP && sel_y <= 23) sel_y++;
		else if (g_action == KEY_DOWN && sel_y >= 0) sel_y--;
		else if (g_action == ACTION_MOUSE_MOVED) {
			sel_x = mx;
			sel_y = my;
		}
		if (sel_x < 0 && sel_y < 0) sel_x = 0;
		if (sel_x < 0 && sel_y > 23) sel_x = 0;
		if (sel_x != curr_x || sel_y != curr_y) {
			fig_move_check(f, h, &sel_x, &sel_y, px, py, &problem, &cost);
			if (problem == MOVE_OK) sprintf(g_text_output_buf, f->txt[0x0c], cost);
			else if (problem == MOVE_ESCAPE) strcpy(g_text_output_buf, f->txt[0x0d]);
			else if (problem == MOVE_BLOCKED) strcpy(g_text_output_buf, f->txt[0x1e]);
			else if (problem == MOVE_TOO_FAR) strcpy(g_text_output_buf, f->txt[0x20]);
			else if (problem == MOVE_NO_PATH) strcpy(g_text_output_buf, f->txt[0x2a]);
			str_pad(g_text_output_buf, 0x16, 0x20);
			strcpy(g_fig_text, g_text_output_buf);
			g_fig_redraw_text = 1;
			fig_list_remove(&f->list, f->mark_id2, 0);
			f->tmpl.cbx = sel_x;
			f->tmpl.cby = sel_y;
			f->tmpl.gfx = f->pic_cursor;
			f->tmpl.twofielded = -1;
			f->mark_id2 = fig_list_add(&f->list, -1);
			fight_draw(f);
		}
		if (g_mouse_rclick || g_action == KEY_ESC) {
			g_mouse_rclick = 0;
			g_action = KEY_RETURN;
			problem = MOVE_CANCEL;
		}
	} while (g_action != KEY_RETURN);
	clear_input();
	mouse_clear_left();
	mouse_clear_right();
	if (*px != sel_x || *py != sel_y) {
		if (problem == MOVE_OK || problem == MOVE_ESCAPE || problem == MOVE_TOO_FAR) {
			if (problem == MOVE_ESCAPE) {
				mouse_hide(0);
				if (dlg_yes_no(g_dialog, f->txt[0x23]) == 1) {
					h->scared = 1;
					problem = MOVE_OK;
				} else {
					fig_list_remove(&f->list, f->mark_id2, 0);
					f->mark_id2 = -1;
				}
				mouse_show(0);
			}
			if (problem == MOVE_OK || problem == MOVE_TOO_FAR) {
				fig_list_remove(&f->list, f->mark_id2, 0);
				f->mark_id2 = -1;
				mouse_hide(0);
				fig_hero_place(f, h);
				mouse_show(0);
				fight_find_cell(get_hero_index(h) + 1, px, py);
			}
		} else if (problem == MOVE_BLOCKED) {
			mouse_hide(0);
			dlg_message(g_dialog, f->txt[0x2b]);
			mouse_show(0);
		} else if (problem == MOVE_NO_PATH) {
			mouse_hide(0);
			dlg_message(g_dialog, f->txt[0x2c]);
			mouse_show(0);
		}
	}
	if (f->mark_id2 != -1) {
		fig_list_remove(&f->list, f->mark_id2, 0);
		f->mark_id2 = -1;
	}
	mouse_hide(0);
	*g_fig_text = 0;
	g_fig_redraw_text = 1;
}
