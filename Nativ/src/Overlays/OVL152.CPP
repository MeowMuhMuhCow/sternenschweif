/* OVL152 - Overlay O152 (Segment 0x225e in SCHWEIF.EXE): Gegner-KI im Kampf (entspricht DSA1 seg037).
 * test_foe_melee_attack(f, x, y, dx, dy, mode): Nachbarfeld g_fight_walkmap[(y + dy) * 25 + x + dx] pruefen -
 *   mode 1: Held 1..9 (bei g_fight_end == 5 nur der Held get_first_active_hero(g_group), Vergleich
 *   get_hero_index(h) == v - 1, sonst check_hero3) oder abtruenniger Gegner 10..29 (enemy_is_renegade_on_field),
 *   mode 0: feindlicher Gegner (fight_check_state).
 * enemy_spell_attack(f, e, attack, x, y) = BE seg037_0791: Zahl der Sprueche aus g_mon_spell_repertoire[mag_id]
 *   (5 je Gegner, Ende -1), zufaelligen Spruch (random_interval), nicht denselben wie zuletzt (max. 5 Versuche),
 *   get_foe_attack_mode -> Modus (2 = sofort auf sich selbst, enemy_set_target_enemy), sonst je nach
 *   g_mon_spell_descriptions[l].ranged Nah- oder Fernziel suchen (ki_search_target Reichweite 1 bzw. 99), sonst
 *   hinlaufen (fight_path_search Modus 1/2/0 bzw. 10/11/9, fig_enemy_move, Feld neu ueber fight_find_cell);
 *   max. 5 Runden; Ergebnis Spruch oder -1.
 * enemy_range_attack(f, e, attack, x, y) = BE seg037_0b3e: Ziel per ki_search_target(.., 99, 0) ab BP >= 3, Treffer
 *   nur bei Entfernung manhattan_dist >= 2; sonst hinlaufen (fight_path_search 10/9, 2); ohne Erfolg Aktion
 *   FIG_ACTION_WAIT.
 * enemy_turn(f, e, x, y): Schleife solange BP > 0 - Marker mark_id entfernen (fig_list_remove), fig_set_mark/
 *   fight_draw, Flucht (scared) wenn le_flee >= LE, abtruennig (renegade) -> attack 0; Zauber (mag_id != -1,
 *   AE >= 5; Aktion 4, BP -5), Fernkampf (shots/throws > 0; Aktion 15, BP -3), Nahkampf rundum ab Blickrichtung
 *   viewdir mit Richtungstabelle g_fig_dirs_ki (Zweifeldgegner enemy_is_twofielded: Schwanzfeld pruefen),
 *   Aktion 2 / BP -3, sonst auf das Ziel zu laufen (fight_path_search Modus 3/1/0), stehen geblieben -> BP 0.
 *   Ergebnis e->action_id.
 *
 * BCC -mlarge -3 -Ot -c -Y OVL152.CPP   -> 3491/3491 Bytes GLEICH (18.10.2026; mit -Og oder -Ov ebenso),
 *   cobjfix laeuft (_g_6c36 -> _g_6be6+0x50)
 *   (Namensfassung mit schweif.h, Welle N11: 19.10.2026 GLEICH, cobjfix LINKGLEICH)
 * - C++ wegen der inline-Setter e->set_enemy_id(v) / e->set_cur_spell(v): Parameterkopie je Expansion ([bp-15h],
 *   [bp-16h] in enemy_spell_attack; in enemy_turn [bp-13h] auf Funktionsebene, [bp-15h] im Block mit r).
 *   Direktzuweisungen e->enemy_id = -1 gehen ohne Kopie. this wird nicht kopiert (e ist Parameter).
 * - "{ signed char r = ki_search_target(..); if (r) e->set_enemy_id(r); }" als eigener Block: beide Vorkommen in
 *   enemy_spell_attack teilen [bp-17h]/[bp-18h] (Block-Local hinter den Funktions-Kopien, seine Kopie dahinter).
 * - Bitfelder (Original "unsigned char b:1", Header unsigned short): "!(signed char)e->tied" -> shr/and/cbw/or ax,ax,
 *   "if ((unsigned char)e->renegade)" -> shr/and/or al,al (ohne Cast or ax,ax); Setzen e->scared = 1 -> or byte 4.
 * - enemy_turn liefert signed short (cbw im Epilog; Prototyp bisher signed char).
 * - Zuweisungsketten rechts zuerst: "cnt = done = i = 0" (i, DI, cnt), "ret = stop = 0".
 * - Spruchauswahl als do { .. } while (k < 5) (kein Sprung zur Bedingung am Anfang), break im else.
 * - BP-Abzug "e->bp += (signed char)-5" -> add byte 0fbh, Grenze "if (..) e->bp = 0; else ;" -> toter jmp.
 * - Felder als *(g_fight_walkmap + (y + dy) * 25 + x + dx); im Schwanzfeld (y - d.y) * 25 + (x - d.x).
 * Originalfehler/Auffaelligkeiten: enemy_turn prueft "v >= 0xcc || v >= 50 || v >= 30" (die ersten beiden sind
 *   ueberfluessig, DSA1 hatte "< 0 || >= 50 || >= 30"); enemy_spell_attack ruft random_interval(0, cnt - 1) auch mit
 *   cnt == 0 (Gegner mit Magie-Id ohne Sprueche); Parameter f von test_foe_melee_attack unbenutzt.
 */
#include "schweif.h"

#define NO_TARGET	(-1)		/* enemy.enemy_id: kein Ziel */

/* Ziel des Nahkampfs auf dem Nachbarfeld (x + dx, y + dy)? mode 1: Held bzw. abtruenniger Gegner, 0: Gegner */
signed short test_foe_melee_attack(Fight far *f, signed short x, signed short y, signed short dx, signed short dy,
	register signed short mode)
{
	unsigned char v;
	register signed short ret = 0;

	if (g_quick_fight) game_idle();
	v = *(g_fight_walkmap + (y + dy) * FIG_MAP_PITCH + x + dx);
	if (mode == 1) {
		if (g_fight_end == 5) {
			if (v >= FIG_ID_HERO && v < FIG_ID_ENEMY &&
			    get_hero_index(get_first_active_hero(g_group)) == v - FIG_ID_HERO)
				ret = 1;
		} else {
			if ((v >= FIG_ID_HERO && v < FIG_ID_ENEMY && check_hero3(&g_heroes[v - FIG_ID_HERO])) ||
			    (v >= FIG_ID_ENEMY && v < FIG_ID_ENEMY_TAIL && enemy_is_renegade_on_field(&g_enemies[v - FIG_ID_ENEMY])))
				ret = 1;
		}
	} else if (mode == 0) {
		if (v >= FIG_ID_ENEMY && v < FIG_ID_ENEMY_TAIL && fight_check_state(&g_enemies[v - FIG_ID_ENEMY]))
			ret = 1;
	}
	return ret;
}

/* Zauber waehlen und Ziel suchen bzw. hinlaufen (BE seg037_0791); Ergebnis Spruch oder -1 */
signed short enemy_spell_attack(Fight far *f, struct enemy far *e, signed short attack, signed short x, signed short y)
{
	signed short cnt;
	signed short i;
	signed short mode;
	signed short k;
	signed short ok;
	signed char reach;
	signed short last;
	signed short tries;
	signed short again;
	signed short moved;
	register signed short spell = -1;
	register signed short done;

	last = -2;
	tries = 0;
	again = 1;
	while (again && e->bp > 0 && tries < 5) {
		again = 0;
		moved = 0;
		tries++;
		if (g_quick_fight) game_idle();
		cnt = done = i = 0;
		while (i < 5 && g_mon_spell_repertoire[e->mag_id * 5 + i++] != -1)
			cnt++;
#if FIX_0054
		/* Fix #54: ohne Zauber keine Zufallswahl mit Index -1 */
		while (cnt > 0 && !done && e->bp > 0) {
			if (g_quick_fight) game_idle();
			k = 0;
#else
		while (!done && e->bp > 0) {
			if (g_quick_fight) game_idle();
			k = 0;
#endif
			do {
				spell = g_mon_spell_repertoire[e->mag_id * 5 + random_interval(0, cnt - 1)];
				ok = 1;
				if (spell == last) {
					ok = 0;
				} else {
					last = spell;
					break;
				}
				k++;
			} while (k < 5);
			e->enemy_id = NO_TARGET;
			if (ok) {
				if ((mode = get_foe_attack_mode(f, spell, attack)) > 0) {
					if (mode == 2) {
						/* auf sich selbst */
						enemy_set_target_enemy(e, e);
						if (enemy_spell_target_ok(e, spell)) {
							return spell;
						} else {
							again = 1;
							e->enemy_id = NO_TARGET;
						}
						done = 1;
					} else if (!g_mon_spell_descriptions[spell].ranged) {
						/* Beruehrungszauber: Ziel auf dem Nachbarfeld */
						while (e->bp != 0 && !done) {
							if (!moved) {
								e->enemy_id = NO_TARGET;
								{
									signed char r = ki_search_target(f, x, y, mode, 1, 1);
									if (r)
										e->set_enemy_id(r);
								}
							}
							if (e->enemy_id != NO_TARGET) {
								if (enemy_spell_target_ok(e, spell)) {
									return spell;
								} else {
									again = 1;
									e->enemy_id = NO_TARGET;
								}
								done = 1;
							} else if (e->bp > 0) {
								if (!(signed char)e->tied) {
									if (mode == 0)
										reach = fight_path_search(f, e, x, y, 1, 1);
									else if (mode == 3)
										reach = fight_path_search(f, e, x, y, 2, 1);
									else
										reach = fight_path_search(f, e, x, y, 0, 1);
									if (reach) {
										e->set_enemy_id(reach);
										if (enemy_spell_target_ok(e, spell)) {
											fig_enemy_move(f, e);
											fight_find_cell(fight_obj_index(e) + FIG_ID_ENEMY, &x, &y);
											moved = 1;
											if (e->bp < 5) {
												e->bp = 0;
												e->enemy_id = NO_TARGET;
											}
										} else {
											again = 1;
											done = 1;
											e->enemy_id = NO_TARGET;
										}
									} else {
										again = 1;
										done = 1;
										e->enemy_id = NO_TARGET;
									}
								} else {
									e->bp = 0;
									e->enemy_id = NO_TARGET;
								}
							}
						}
					} else {
						/* Fernzauber */
						while (!done && e->bp > 0) {
							if (g_quick_fight) game_idle();
							if (!moved) {
								e->enemy_id = NO_TARGET;
								{
									signed char r = ki_search_target(f, x, y, mode, 99, 1);
									if (r)
										e->set_enemy_id(r);
								}
							}
							if (e->enemy_id != NO_TARGET) {
								if (enemy_spell_target_ok(e, spell)) {
									return spell;
								} else {
									again = 1;
									e->enemy_id = NO_TARGET;
								}
								done = 1;
							} else if (e->bp > 0) {
								if (!(signed char)e->tied) {
									if (mode == 0)
										reach = fight_path_search(f, e, x, y, 10, 1);
									else if (mode == 3)
										reach = fight_path_search(f, e, x, y, 11, 1);
									else
										reach = fight_path_search(f, e, x, y, 9, 1);
									if (reach) {
										e->set_enemy_id(reach);
										if (enemy_spell_target_ok(e, spell)) {
											fig_enemy_move(f, e);
											fight_find_cell(fight_obj_index(e) + FIG_ID_ENEMY, &x, &y);
											moved = 1;
											if (e->bp < 5) {
												e->bp = 0;
												e->enemy_id = NO_TARGET;
												moved = 0;
												done = 1;
											}
										} else {
											again = 1;
											done = 1;
											e->enemy_id = NO_TARGET;
										}
									} else {
										again = 1;
										done = 1;
										e->enemy_id = NO_TARGET;
									}
								} else {
									e->bp = 0;
									e->enemy_id = NO_TARGET;
									done = 1;
								}
							}
						}
					}
				}
			} else {
				done = 1;
			}
		}
	}
	return -1;
}

/* Fernkampfziel suchen bzw. hinlaufen (BE seg037_0b3e); 1 = Ziel gefunden (nicht auf dem Nachbarfeld) */
signed short enemy_range_attack(Fight far *f, struct enemy far *e, signed short attack, signed short x, signed short y)
{
	signed short stop;
	signed short tx;
	signed short ty;
	register signed short reach;
	register signed short ret;

	ret = stop = 0;
	while (!stop && e->bp > 0) {
		if (g_quick_fight) game_idle();
		while (!stop && e->bp > 0) {
			if (g_quick_fight) game_idle();
			e->enemy_id = NO_TARGET;
			if (e->bp >= 3) {
				e->enemy_id = NO_TARGET;
				{
					signed char r = ki_search_target(f, x, y, attack, 99, 0);
					if (r)
						e->set_enemy_id(r);
				}
			}
			if (e->enemy_id != NO_TARGET) {
				fight_find_cell(e->enemy_id, &tx, &ty);
				if (manhattan_dist(tx, ty, x, y) < 2)
					ret = 0;
				else
					ret = 1;
				stop = 1;
			} else if (e->bp > 0) {
				if (!(signed char)e->tied) {
					if (attack == 0)
						reach = fight_path_search(f, e, x, y, 10, 2);
					else
						reach = fight_path_search(f, e, x, y, 9, 2);
					if (reach) {
						fig_enemy_move(f, e);
						fight_find_cell(fight_obj_index(e) + FIG_ID_ENEMY, &x, &y);
						if (e->bp < 3)
							e->bp = 0;
					} else {
						e->bp = 0;
					}
				} else {
					e->bp = 0;
				}
			}
		}
	}
	if (!ret) {
		e->enemy_id = NO_TARGET;
		e->action_id = FIG_ACTION_WAIT;
	}
	return ret;
}

/* Zug eines Gegners planen (BE enemy_turn); Ergebnis Kampfaktion */
signed short enemy_turn(Fight far *f, struct enemy far *e, signed short x, signed short y)
{
	signed short reach;
	signed short attack;
	signed short dir;
	signed short cnt;
	signed short done;
	signed short ok;
	signed short v;
	signed short ox;
	signed short oy;
	struct dirs4 d;

	done = 0;
	d = g_fig_dirs_ki;
	e->action_id = FIG_ACTION_WAIT;
	e->enemy_id = NO_TARGET;
	while (!done && e->bp > 0) {
		if (g_quick_fight) game_idle();
		if (f->mark_id != -1) {
			fig_list_remove(&f->list, f->mark_id, 0);
			f->mark_id = -1;
		}
		fig_set_mark(f, e->fighter_id);
		fight_draw(f);
		e->enemy_id = NO_TARGET;
		e->action_id = FIG_ACTION_WAIT;
		if (e->le_flee >= e->le)
			e->scared = 1;
		if (!(signed char)e->scared) {
			attack = 1;
			if ((unsigned char)e->renegade)
				attack = 0;
			if (e->mag_id != -1 && e->ae >= 5) {
				signed char r;

				r = -1;
				r = enemy_spell_attack(f, e, attack, x, y);
				if (r != -1) {
					e->action_id = FIG_ACTION_SPELL;
					e->bp += (signed char)-5;
					if (e->bp < 0)
						e->bp = 0;
					else ;
					e->set_cur_spell(r);
					return e->action_id;
				} else {
					e->action_id = FIG_ACTION_WAIT;
					e->cur_spell = -1;
					e->enemy_id = NO_TARGET;
				}
			}
			if (e->shots > 0 || e->throws > 0) {
				if (enemy_range_attack(f, e, attack, x, y)) {
					e->action_id = FIG_ACTION_RANGE_ATTACK;
					e->bp += (signed char)-3;
					if (e->bp < 0)
						e->bp = 0;
					else ;
					return e->action_id;
				}
			}
			/* Nahkampf: Nachbarfelder ab der Blickrichtung */
			e->enemy_id = NO_TARGET;
			dir = e->viewdir;
			cnt = 0;
			while (e->enemy_id == NO_TARGET && cnt < 4) {
				if (test_foe_melee_attack(f, x, y, d.d[dir].dx, d.d[dir].dy, attack)) {
					ok = 1;
					if (enemy_is_twofielded(e)) {
						/* Schwanzfeld muss frei sein (oder ein Toter darauf liegen) */
						v = *(g_fight_walkmap + (y - d.d[dir].dy) * FIG_MAP_PITCH + (x - d.d[dir].dx));
						if (v != 0) {
							if (fight_obj_index(e) + FIG_ID_ENEMY_TAIL != v) {
								if (v >= 0xcc || v >= 50 || v >= FIG_ID_ENEMY_TAIL ||
								    (v >= FIG_ID_HERO && v < FIG_ID_ENEMY && !(signed char)g_heroes[v - FIG_ID_HERO].dead) ||
								    (v >= FIG_ID_ENEMY && v < FIG_ID_ENEMY_TAIL && !(signed char)g_enemies[v - FIG_ID_ENEMY].dead))
									ok = 0;
							}
						}
					}
					if (ok)
						e->set_enemy_id(*(g_fight_walkmap + (y + d.d[dir].dy) * FIG_MAP_PITCH + x + d.d[dir].dx));
				}
				cnt++;
				if (++dir == 4)
					dir = 0;
			}
		}
		if (e->enemy_id != NO_TARGET) {
			e->action_id = FIG_ACTION_MELEE_ATTACK;
			e->bp += (signed char)-3;
			if (e->bp < 0)
				e->bp = 0;
			else ;
			done = 1;
		} else if (e->bp > 0) {
			/* kein Ziel: hinlaufen bzw. fliehen */
			if (fight_find_cell(fight_obj_index(e) + FIG_ID_ENEMY, &x, &y)) {
				if (!(signed char)e->tied) {
					if ((unsigned char)e->scared)
						reach = fight_path_search(f, e, x, y, 3, 1);
					else if ((unsigned char)e->renegade)
						reach = fight_path_search(f, e, x, y, 1, 1);
					else
						reach = fight_path_search(f, e, x, y, 0, 1);
					if (reach) {
						ox = x;
						oy = y;
						fig_enemy_move(f, e);
						fight_find_cell(fight_obj_index(e) + FIG_ID_ENEMY, &x, &y);
						if (ox == x && oy == y)
							e->bp = 0;
						if (e->bp < 3)
							e->bp = 0;
					} else {
						e->bp = 0;
					}
				} else {
					e->bp = 0;
				}
				e->action_id = FIG_ACTION_WAIT;
			}
		}
		if (g_quick_fight) game_idle();
	}
	return e->action_id;
}
