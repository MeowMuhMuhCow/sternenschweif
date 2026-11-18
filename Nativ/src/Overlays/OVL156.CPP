/* OVL156 - Overlay O156 (Segment 0x2278 in SCHWEIF.EXE): Kampf-KI der Helden (entspricht DSA1 seg036, Bright Eyes
 * KI_change_hero_weapon / KI_can_attack_neighbour / KI_get_spell / seg036_8cf); Aufrufer O152, O157.
 * ki_change_hero_weapon(f, h): kaputte Waffe tauschen - erste Nahkampfwaffe im Rucksack (Platz 15..30, weapon,
 *   Untertyp nicht 7/8/5, can_use_item) per move_item in die rechte Hand, sonst freier Platz bzw. linke Hand;
 *   Figur obj_pic = h->viewdir, fight_draw, BP -2 (mind. 0). ki_target_valid(f, mode, id): Ziel id angreifbar?
 *   (0: Gegner 10..49 fight_check_state, 1: Held check_hero3 oder abtruenniger Gegner, 3: Held, 4: versteinerter,
 *   lebender Held). ki_best_target(ids, n): staerkstes Ziel (Stufe * (Magier + 1) * 10 + LE + AE bzw. Gegner-
 *   werte). ki_search_target(f, x, y, mode, range, diag): Ziele im Quadrat +-range auf g_fight_walkmap (Abstand
 *   manhattan_dist, Diagonale bei diag 0 ausgeschlossen, Sichtlinie fig_line_of_sight), bestes nach
 *   ki_best_target, Feld nach g_fig_target_x/y (vertauscht, s. u.), Rueckgabe Kennung. ki_range_target(f, h, mode,
 *   x, y): Fernkampfziel suchen bzw. hinlaufen (fight_path_search Modus 0x0d/0x0c, fig_hero_place); 2 = Nachbar,
 *   1 = weiter weg. ki_spell_mode(f, spruch, abtruennig): Zielart des Zaubers (g_spell_descriptions.target_type)
 *   -> -1/0..4. ki_spell_useful(f, h, spruch): lohnt der Zauber auf das Ziel (target_type, enemy_mr_target bzw.
 *   hero_mr_target)? Sonderregeln fuer Blitz, Horriphobus (freies Nachbarfeld noetig), Paralue, Balsam (LE <=
 *   Haelfte oder < 10), Boeser Blick, Plumbumbarum (enemy_attack_value > 6), Fulminictus, Ignifaxius, Axxeleratus/
 *   Duplicatus/Saft-Kraft (fig_count_active_enemies >= 4), Verwandlung beenden. ki_select_spell_target(f, h,
 *   abtruennig, x, y): Zauber waehlen (g_af_spell_list[W16], Zauberwert > -5, nicht zweimal derselbe), bis 5
 *   Versuche; Selbstzauber (target_x/target_y), Beruehrung (Reichweite 0: Ziel neben sich, sonst hinlaufen
 *   fight_path_search Modus 5/6/8/4) oder Fernzauber (ki_search_target Reichweite 99, Modus 0x0d/0x0e/0x0c);
 *   Rueckgabe Zauber oder -1.
 *
 * BCC -mlarge -3 -Ot -Og -c -Y OVL156.CPP   -> 5006/5006 Bytes GLEICH (18.10.2026), cobjfix laeuft.
 *   Mit -Oi ebenfalls GLEICH, ohne -Og 2 Bytes zu lang (Epilog-/Zwischenwerte verschoben).
 *   (Namensfassung mit schweif.h, Welle N11: 19.10.2026 GLEICH, cobjfix LINKGLEICH)
 * - ki_search_target: "if (n == 0) return 0; else { ..; return ids[v]; }" - nur mit else zwei tote Epiloge.
 * - ki_select_spell_target: Zaehler "k++; } while (k < 5);" -> inc word [bp-0ah] / cmp word [bp-0ah],5;
 *   "while (++k < 5)" ergibt inc / mov ax,[bp-0ah] / cmp ax,5.
 * - inline-Setter set_enemy_id/set_target_x/set_target_y -> Parameterkopien [bp-0bh]/[bp-0dh] (ki_range_target)
 *   bzw. [bp-14h]..[bp-1ah] (ki_select_spell_target); zwei Bloecke "{ signed char r = ki_search_target(..); if (r)
 *   h->set_enemy_id(r); }" teilen [bp-19h]/[bp-1ah].
 * - BP auf 0 setzen als Makro BP_CLEAR (wie O157: "bp_left = 0; if (bp_left > 12) bp_left = 12; else ;" -> toter
 *   jmp short; als inline set_bp_left(0) entstuende eine Parameterkopie).
 * - Kampffeld: "*(g_fight_walkmap + cy * 25 + cx - 1)" (les / add bx,[cx] / [bx-1]) neben
 *   "*(g_fight_walkmap + (cy - 1) * 25 + cx)" (mov es / add ax,off / add ax,[cx] / [bx-19h]) - beide aus BCC.
 * - Bitfeldtests: "(unsigned char)h->scared" -> and ax,1 / or al,al; nur th->saftkraft_on ohne Cast (or ax,ax); "!(signed char)"
 *   -> cbw / or ax,ax.
 * Originalfehler/Auffaelliges: ki_spell_useful benutzt fuer Boeser Blick, Plumbumbarum, Fulminictus, Ignifaxius den
 *   Gegnerzeiger e und fuer Axxeleratus, Duplicatus, Saft-Kraft, Verwandlung beenden den Heldenzeiger th, ohne die
 *   Zielart (kind) zu pruefen - bei der anderen Zielart ist der Zeiger uninitialisiert. ki_search_target legt das
 *   Zielfeld vertauscht ab (g_fig_target_x = Zeile/y, g_fig_target_y = Spalte/x; O146 und ki_select_spell_target
 *   schreiben g_fig_target_x = x). ki_change_hero_weapon: Rueckgabe 2 (Waffe abgelegt) wird wie in DSA1 sofort auf
 *   0 gesetzt.
 */
#include "schweif.h"

#define BP_CLEAR(h) { (h)->bp_left = 0; if ((h)->bp_left > 12) (h)->bp_left = 12; else ; }

enum {					/* Zielart der KI (ki_target_valid, ki_spell_mode) */
	KI_ENEMY = 0,			/* feindlicher Gegner */
	KI_HERO_OR_RENEGADE = 1,	/* Held oder abtruenniger Gegner (Sicht eines Abtruennigen) */
	KI_SELF = 2,			/* auf sich selbst */
	KI_HERO = 3,			/* anderer Held */
	KI_PETRIFIED_HERO = 4		/* versteinerter Held (Verwandlung beenden) */
};

/* zerbrochene Waffe gegen eine Nahkampfwaffe aus dem Rucksack tauschen (BE KI_change_hero_weapon) */
signed short ki_change_hero_weapon(Fight far *f, struct hero far *h)
{
	signed short ret = 0;
	signed short id;
	signed short i;
	struct fig_node far *fig;

	for (i = INV_SLOT_KNAPSACK; i < NR_INV_SLOTS; i++) {
		id = h->inv[i].item_id;
		if (g_items[id].weapon && g_items[id].subtype != 7 && g_items[id].subtype != 8 && g_items[id].subtype != 5 &&
			can_use_item(h, id))
		{
			move_item(h, INV_SLOT_RIGHT_HAND, i);
			ret = 1;
			break;
		}
	}
	if (ret == 0) {
		/* keine Waffe: in einen freien Rucksackplatz bzw. in die linke Hand ablegen */
		for (i = INV_SLOT_KNAPSACK; i < NR_INV_SLOTS; i++) {
			if (h->inv[i].item_id == 0) {
				move_item(h, INV_SLOT_RIGHT_HAND, i);
				ret = 2;
				break;
			}
		}
		if (ret == 0)
			move_item(h, INV_SLOT_RIGHT_HAND, INV_SLOT_LEFT_HAND);
		ret = 0;
	}
	fig = fig_list_find(&f->list, h->fighter_id);
	fig->obj_pic = h->viewdir;
	fight_draw(f);
	h->bp_left += (signed char)-2;
	if (h->bp_left < 0)
		h->bp_left = 0;
	else ;
	return ret;
}

/* Feldinhalt id als Ziel der Art mode angreifbar? */
signed short ki_target_valid(Fight far *f, signed short mode, register signed short id)
{
	signed short ret = 0;

	if (mode == KI_ENEMY) {
		if (id >= FIG_ID_ENEMY && id < FIG_ID_ENEMY_TAIL && fight_check_state(&g_enemies[id - FIG_ID_ENEMY]) ||
			id >= FIG_ID_ENEMY_TAIL && id < 50 && fight_check_state(&g_enemies[id - FIG_ID_ENEMY_TAIL]))
			ret = 1;
	} else if (mode == KI_HERO_OR_RENEGADE) {
		if (id >= FIG_ID_HERO && id < FIG_ID_ENEMY && check_hero3(&g_heroes[id - FIG_ID_HERO]) ||
			id >= FIG_ID_ENEMY && id < FIG_ID_ENEMY_TAIL && enemy_is_renegade_on_field(&g_enemies[id - FIG_ID_ENEMY]) ||
			id >= FIG_ID_ENEMY_TAIL && id < 50 && enemy_is_renegade_on_field(&g_enemies[id - FIG_ID_ENEMY_TAIL]))
			ret = 1;
	} else if (mode == KI_HERO) {
		if (id >= FIG_ID_HERO && id < FIG_ID_ENEMY && check_hero3(&g_heroes[id - FIG_ID_HERO]))
			ret = 1;
	} else if (mode == KI_PETRIFIED_HERO) {
		if (id >= FIG_ID_HERO && id < FIG_ID_ENEMY && (unsigned char)g_heroes[id - FIG_ID_HERO].petrified &&
			!(signed char)g_heroes[id - FIG_ID_HERO].dead)
			ret = 1;
	}
	return ret;
}

/* staerkstes der n Ziele ids[] (Stufe, Zauberkunde, LE, AE); Ergebnis Index */
signed short ki_best_target(unsigned char far *ids, signed short n)
{
	signed short max = -1;
	signed short best = 0;
	struct hero far *h;
	struct enemy far *e;
	signed short i;
	signed short v;

	for (i = 0; i < n; i++) {
		if (ids[i] < FIG_ID_ENEMY) {
			h = &g_heroes[ids[i] - FIG_ID_HERO];
			v = h->level * (h->is_mage() + 1) * 10 + h->le + h->ae;
		} else {
			e = &g_enemies[ids[i] - FIG_ID_ENEMY];
			v = e->level * (e->magic + 1) * 10 + e->le + e->ae;
		}
		if (v > max) {
			max = v;
			best = i;
		}
	}
	return best;
}

/* bestes Ziel der Art mode im Umkreis range um (x, y) mit Sichtlinie; diag 0: kein Diagonal-Nachbar */
signed char ki_search_target(Fight far *f, signed short x, signed short y, signed short mode, signed short range,
	signed short diag)
{
	unsigned char v;
	signed short n = 0;
	signed short x0;
	signed short y0;
	signed short x1;
	signed short y1;
	signed short dist;
	signed short cx;
	signed short cy;
	unsigned char ids[27];
	signed short xs[27];
	signed short ys[27];

	x0 = x - range;
	y0 = y - range;
	x1 = x + range;
	y1 = y + range;
	if (x0 < 0)
		x0 = 0;
	if (y0 < 0)
		y0 = 0;
	if (x1 > FIG_FIELD_W - 1)
		x1 = FIG_FIELD_W - 1;
	if (y1 > FIG_FIELD_W - 1)
		y1 = FIG_FIELD_W - 1;
	for (cy = y0; cy <= y1; cy++) {
		if (g_quick_fight)
			game_idle();
		for (cx = x0; cx <= x1; cx++) {
			v = *(g_fight_walkmap + cy * FIG_MAP_PITCH + cx);
			if (v > 0 && v < 50 && (x != cx || y != cy)) {
				dist = manhattan_dist(x, y, cx, cy);
				if (dist == 2 && diag == 0 && x != cx && y != cy)
					dist = range + 1;
				if (dist <= range && ki_target_valid(f, mode, v) && fig_line_of_sight(f, x, y, cx, cy, 99)) {
					xs[n] = cx;
					ys[n] = cy;
					ids[n] = v;
					n++;
				}
			}
		}
	}
	if (n == 0)
		return 0;
	else {
		v = ki_best_target(ids, n);
#if FIX_0120
		/* Fix #120: Zielfeld nicht vertauschen (x = Spalte wie in O146 und ki_select_spell_target) */
		g_fig_target_x = xs[v];
		g_fig_target_y = ys[v];
#else
		g_fig_target_x = ys[v];
		g_fig_target_y = xs[v];
#endif
		return ids[v];
	}
}

/* Fernkampf: Ziel suchen bzw. hinlaufen; 2 = Ziel auf dem Nachbarfeld, 1 = weiter weg, 0 = keins */
signed short ki_range_target(Fight far *f, struct hero far *h, signed short mode, signed short x, signed short y)
{
	signed short done;
	signed short ret;
	signed short reach;
	signed short tx;
	signed short ty;

	ret = done = 0;
	while (done == 0 && h->bp_left > 0) {
		if (g_quick_fight)
			game_idle();
		h->enemy_id = -1;
		if (h->bp_left >= 3) {
			h->enemy_id = -1;
			{
				signed char r = ki_search_target(f, x, y, mode, 99, 0);
				if (r)
					h->set_enemy_id(r);
			}
		}
		if (h->enemy_id != -1) {
			fight_find_cell(h->enemy_id, &tx, &ty);
			if (manhattan_dist(tx, ty, x, y) < 2)
				ret = 2;
			else
				ret = 1;
			done = 1;
		} else if (!(signed char)h->tied) {
			if (mode == KI_ENEMY)
				reach = fight_path_search(f, h, x, y, 0xd, 2);
			else
				reach = fight_path_search(f, h, x, y, 0xc, 2);
			if (reach) {
				fig_hero_place(f, h);
				if (h->bp_left < 3) {
					BP_CLEAR(h);
					done = 1;
				} else {
					h->set_enemy_id(reach);
					ret = 1;
					done = 1;
				}
			} else {
				BP_CLEAR(h);
				h->enemy_id = -1;
			}
		} else {
			BP_CLEAR(h);
			h->enemy_id = -1;
		}
	}
	return ret;
}

/* Zielart des Zaubers sp fuer die KI (BE KI_get_spell); ren: Held ist abtruennig; -1 = ungeeignet */
signed short ki_spell_mode(Fight far *f, signed short sp, signed short ren)
{
	signed short ret = -1;
	struct spell_desc far *p;

	if (g_quick_fight)
		game_idle();
	p = &g_spell_descriptions[sp];
	if (ren == 0) {
		if (p->target_type == 2) {
			if (sp == SP_VERWANDLUNG_BEENDEN)
				ret = KI_PETRIFIED_HERO;
			else
				ret = KI_HERO;
		} else if (p->target_type == 1 || p->target_type == 3)
			ret = KI_ENEMY;
		else
			ret = KI_SELF;
	} else {
		if (p->target_type == 3)
			ret = KI_HERO_OR_RENEGADE;
		else if (p->target_type == 0)
			ret = KI_SELF;
	}
	return ret;
}

/* lohnt sich der Zauber sp auf das gewaehlte Ziel? */
signed short ki_spell_useful(Fight far *f, struct hero far *h, signed short sp)
{
	struct enemy far *e;
	struct hero far *th;
	signed short kind;
	signed short cx;
	signed short cy;
	signed char t;
	signed short ok;

	if (g_quick_fight)
		game_idle();
	ok = 1;
	t = target_type(h);
	if (t == TARGET_ENEMY || t == TARGET_SPECIAL) {
		e = enemy_mr_target(h);
		kind = 1;
	} else if (t == TARGET_SELF || t == TARGET_HERO) {
		th = hero_mr_target(h);
		kind = 2;
	} else
		return 0;

	if (sp == SP_BLITZ) {
		if (kind == 1) {
			if (e->blind != 0 || (unsigned char)e->scared || (unsigned char)e->petrified)
				ok = 0;
		} else {
			if (th->blind != 0 || (unsigned char)th->scared || (unsigned char)th->petrified)
				ok = 0;
		}
	} else if (sp == SP_HORRIPHOBUS) {
		if (kind == 1) {
			if ((unsigned char)e->petrified || e->magic != 0 || (unsigned char)e->scared || e->le < e->le_max / 3)
				ok = 0;
			else {
				/* Fluchtweg: ein Nachbarfeld muss frei sein (oder ausserhalb) */
				ok = 0;
				fight_find_cell(fight_obj_index(e) + FIG_ID_ENEMY, &cx, &cy);
				if (fight_cell_blocked(cx - 1, cy) || !*(g_fight_walkmap + cy * FIG_MAP_PITCH + cx - 1))
					ok = 1;
				if (fight_cell_blocked(cx + 1, cy) || !*(g_fight_walkmap + cy * FIG_MAP_PITCH + cx + 1))
					ok = 1;
				if (fight_cell_blocked(cx, cy - 1) || !*(g_fight_walkmap + (cy - 1) * FIG_MAP_PITCH + cx))
					ok = 1;
				if (fight_cell_blocked(cx, cy + 1) || !*(g_fight_walkmap + (cy + 1) * FIG_MAP_PITCH + cx))
					ok = 1;
			}
		} else {
			if ((unsigned char)th->scared || (unsigned char)th->petrified)
				ok = 0;
		}
	} else if (sp == SP_PARALUE) {
		if (kind == 1) {
			if ((unsigned char)e->petrified || (unsigned char)e->scared)
				ok = 0;
		} else {
			if ((unsigned char)th->petrified || (unsigned char)th->scared)
				ok = 0;
		}
	} else if (sp == SP_BALSAM) {
		ok = 0;
		if (kind == 1) {
			if (e->le <= e->le_max / 2 || e->le < 10)
				ok = 1;
		} else {
			if (th->le <= th->le_max / 2 || th->le < 10)
				ok = 1;
		}
	} else if (sp == SP_BOESER_BLICK) {
#if FIX_0120
		/* Fix #120: Gegnerzauber nur mit Gegner als Ziel (sonst e uninitialisiert) */
		if (kind != 1 || (unsigned char)e->petrified || (unsigned char)e->scared || e->le < e->le_max / 3)
			ok = 0;
#else
		if ((unsigned char)e->petrified || (unsigned char)e->scared || e->le < e->le_max / 3)
			ok = 0;
#endif
	} else if (sp == SP_PLUMBUMBARUM) {
#if FIX_0120
		/* Fix #120: Gegnerzauber nur mit Gegner als Ziel */
		if (kind != 1 || (unsigned char)e->scared || (unsigned char)e->petrified || enemy_attack_value(e) <= 6)
			ok = 0;
#else
		if ((unsigned char)e->scared || (unsigned char)e->petrified || enemy_attack_value(e) <= 6)
			ok = 0;
#endif
	} else if (sp == SP_FULMINICTUS) {
#if FIX_0120
		/* Fix #120: Gegnerzauber nur mit Gegner als Ziel */
		if (kind != 1 || (unsigned char)e->scared || e->le < e->le_max / 5)
			ok = 0;
#else
		if ((unsigned char)e->scared || e->le < e->le_max / 5)
			ok = 0;
#endif
#if FIX_0120
	/* Fix #120: Gegnerzauber nur mit Gegner als Ziel */
	} else if (sp == SP_IGNIFAXIUS) {
		if (kind != 1 || (unsigned char)e->scared)
			ok = 0;
#else
	} else if (sp == SP_IGNIFAXIUS) {
		if ((unsigned char)e->scared)
			ok = 0;
#endif
	} else if (sp == SP_AXXELERATUS) {
#if FIX_0120
		/* Fix #120: Heldenzauber nur mit Held als Ziel (sonst th uninitialisiert) */
		if (kind != 2 || th->axxeleratus != 0 || (unsigned char)th->scared || fig_count_active_enemies(f) < 4 ||
#else
		if (th->axxeleratus != 0 || (unsigned char)th->scared || fig_count_active_enemies(f) < 4 ||
#endif
		    (unsigned char)th->petrified)
			ok = 0;
	} else if (sp == SP_DUPLICATUS) {
#if FIX_0120
		/* Fix #120: Heldenzauber nur mit Held als Ziel */
		if (kind != 2 || th->duplicatus >= 1 || (unsigned char)th->scared || fig_count_active_enemies(f) < 4 ||
#else
		if (th->duplicatus >= 1 || (unsigned char)th->scared || fig_count_active_enemies(f) < 4 ||
#endif
		    (unsigned char)th->petrified)
			ok = 0;
	} else if (sp == SP_SAFT_KRAFT) {
#if FIX_0120
		/* Fix #120: Heldenzauber nur mit Held als Ziel */
		if (kind != 2 || th->saftkraft_on || (unsigned char)th->scared || fig_count_active_enemies(f) < 4 || (unsigned char)th->petrified)
#else
		if (th->saftkraft_on || (unsigned char)th->scared || fig_count_active_enemies(f) < 4 || (unsigned char)th->petrified)
#endif
			ok = 0;
	} else if (sp == SP_VERWANDLUNG_BEENDEN) {
#if FIX_0120
		/* Fix #120: Heldenzauber nur mit Held als Ziel */
		if (kind != 2 || !(signed char)th->petrified)
			ok = 0;
#else
		if (!(signed char)th->petrified)
			ok = 0;
#endif
	}
	return ok;
}

/* Zauber waehlen und Ziel suchen bzw. hinlaufen (BE KI_select_spell_target); Ergebnis Zauber oder -1 */
signed char ki_select_spell_target(Fight far *f, struct hero far *h, signed short ren, signed short x, signed short y)
{
	signed char sp = -1;
	signed short mode;
	signed short reach;
	signed short found;
	signed short k;
	signed short last = -2;
	signed short tries = 0;
	signed short again = 1;
	signed short moved;
	signed short done;

	h->enemy_id = -1;
	while (again && h->bp_left > 0 && tries < 5) {
		again = 0;
		tries++;
		for (done = 0; !done && h->bp_left > 0; ) {
			if (g_quick_fight)
				game_idle();
			k = 0;
			do {
				if (g_quick_fight)
					game_idle();
				sp = g_af_spell_list[random_interval(0, 15)];
				found = 0;
				if (h->spells[sp] > -5) {
					found = 1;
					if (sp == last)
						found = 0;
					else {
						last = sp;
						break;
					}
				}
				k++;
			} while (k < 5);
			h->enemy_id = -1;
			if (found != 0) {
				if ((mode = ki_spell_mode(f, sp, ren)) != -1) {
					if (mode == KI_SELF) {
						hero_set_target(h, h);
						if (ki_spell_useful(f, h, sp)) {
							h->set_target_x(x);
							h->set_target_y(y);
							g_fig_target_x = x;
							g_fig_target_y = y;
							return sp;
						} else {
							again = 1;
							h->enemy_id = -1;
						}
						done = 1;
					} else if (!g_spell_descriptions[sp].range) {
						/* Beruehrungszauber */
						moved = 0;
						while (h->bp_left > 0 && !done) {
							if (g_quick_fight)
								game_idle();
							if (moved == 0) {
								h->enemy_id = -1;
								{
									signed char r = ki_search_target(f, x, y, mode, 1, 1);
									if (r)
										h->set_enemy_id(r);
								}
							}
							if (h->enemy_id != -1) {
								if (h->bp_left >= 5) {
									if (ki_spell_useful(f, h, sp))
										return sp;
									else {
										again = 1;
										h->enemy_id = -1;
									}
								} else {
									BP_CLEAR(h);
									h->enemy_id = -1;
								}
								done = 1;
							} else if (!(signed char)h->tied) {
								if (mode == KI_ENEMY)
									reach = fight_path_search(f, h, x, y, 5, 1);
								else if (mode == KI_HERO)
									reach = fight_path_search(f, h, x, y, 6, 1);
								else if (mode == KI_PETRIFIED_HERO)
									reach = fight_path_search(f, h, x, y, 8, 1);
								else
									reach = fight_path_search(f, h, x, y, 4, 1);
								if (reach) {
									if (g_quick_fight)
										game_idle();
									h->set_enemy_id(reach);
									if (ki_spell_useful(f, h, sp)) {
										fig_hero_place(f, h);
										fight_find_cell(get_hero_index(h) + FIG_ID_HERO, &x, &y);
										moved = 1;
										if (h->bp_left < 5) {
											h->enemy_id = -1;
											BP_CLEAR(h);
											moved = 0;
											done = 1;
										}
									} else {
										again = 1;
										done = 1;
										h->enemy_id = -1;
									}
								} else {
									h->enemy_id = -1;
									again = 1;
									done = 1;
								}
							} else {
								BP_CLEAR(h);
								h->enemy_id = -1;
							}
						}
					} else {
						/* Fernzauber */
						moved = 0;
						while (!done && h->bp_left > 0) {
							if (g_quick_fight)
								game_idle();
							if (moved == 0) {
								h->enemy_id = -1;
								{
									signed char r = ki_search_target(f, x, y, mode, 99, 1);
									if (r)
										h->set_enemy_id(r);
								}
							}
							if (h->enemy_id != -1) {
								if (h->bp_left >= 5) {
									if (ki_spell_useful(f, h, sp))
										return sp;
									else {
										again = 1;
										h->enemy_id = -1;
									}
								} else {
									BP_CLEAR(h);
									h->enemy_id = -1;
								}
								done = 1;
							} else if (!(signed char)h->tied) {
								if (mode == KI_ENEMY)
									reach = fight_path_search(f, h, x, y, 0xd, 1);
								else if (mode == KI_HERO)
									reach = fight_path_search(f, h, x, y, 0xe, 1);
								else
									reach = fight_path_search(f, h, x, y, 0xc, 1);
								if (reach) {
									if (g_quick_fight)
										game_idle();
									h->set_enemy_id(reach);
									if (ki_spell_useful(f, h, sp)) {
										fig_hero_place(f, h);
										fight_find_cell(get_hero_index(h) + FIG_ID_HERO, &x, &y);
										moved = 1;
										if (h->bp_left < 5) {
											BP_CLEAR(h);
											done = 1;
											h->enemy_id = -1;
											moved = 0;
										}
									} else {
										again = 1;
										done = 1;
										h->enemy_id = -1;
									}
								} else {
									h->enemy_id = -1;
									again = 1;
									done = 1;
								}
							} else {
								BP_CLEAR(h);
								h->enemy_id = -1;
								done = 1;
							}
						}
					}
				}
			} else
				done = 1;
		}
	}
	return -1;
}
