/* OVL154 - Overlay O154 (Segment 0x2270 in SCHWEIF.EXE): Kampfmenue eines Helden (wie FIG_menu in DSA 1 seg033).
 * fig_menu(f, h, x, y): Schleife solange nicht fertig und h->bp_left > 0: Tasten (fight_check_keys), Bild
 * (fight_draw), Tastenmerker f->keys[7] ggf. leeren, Marker f->mark_id entfernen; Flucht (scared, fight_path_search
 * Modus 7 / fig_hero_place), KI bei renegade/npc_id/fb5 (ki_hero); sonst Menue dlg_choose (10 Eintraege, Farbe
 * code_to_text(0xe7/0xf9) fuer gesperrte Punkte, Lage dlg_set_pos_x/y je x + y < 15) mit Untermenues 5 (Werte/
 * Kampfstil attack_style) und 4 (Gegenstand txt[92]/Ablegen/Nehmen). Menuewahl act: 1 Bewegen fig_menu_move,
 * 2 Angriff fig_menu_attack, 3..9/11 fig_menu_* (O155), 13 Automatik/Rueckzug (all_heroes_down, group_set_fb5/
 * group_set_fb6_b0, fight_quick_start), 10 Kampf abbrechen (g_in_fight = 0, g_game_state = 2). Rueckgabe
 * h->action_id; bei g_fight_end == 5 Ende fuer andere Helden mit Aktion 2/4/15/5.
 * fig_menu_move: Fesselung (tied, Probe MU+2, sonst MU -2 fuer 7 h per add_effect), Bewegung fig_move_hero.
 * fig_menu_attack: Ziel waehlen (fig_cb_select_target, Fernkampf hand_weapon_class/range_attack_check_ammo),
 * Pruefungen (selbst, abtruenniger Gegner, tot, fight_check_state, Abstand manhattan_dist), dann enemy_id/
 * target_x/target_y, BP -3, Aktion 2/15.
 *
 * BCC -mlarge -3 -Ot -Og -Oi -c -Y OVL154.CPP   -> 5269/5269 Bytes GLEICH (18.10.2026), cobjfix laeuft
 *   (Namensfassung mit schweif.h, Welle N11: 19.10.2026 GLEICH, cobjfix LINKGLEICH)
 * - -Oi fuer strcpy (Intrinsic); deshalb bekommen register-Variablen auf Ebene des Menue-Blocks kein Register:
 *   ok/k/changed (SI/DI) in eigenen inneren Bloecken deklarieren, sonst [bp-10h]/[bp-12h].
 * - C++ wegen inline is_mage() (mov ax,1/xor ax,ax/or ax,ax) und der Setter set_attack_style/set_fb6_b0/
 *   set_attack_type/set_enemy_id/set_action_id/set_target_x/set_target_y (Kopien [bp-0fh], [bp-1bh], [bp-0ah]..
 *   [bp-17h]) sowie add_bp_left(-3) (add byte 0fdh, Klemmen auf 0).
 * - -Og: [bp-10h] Byte +bc, [bp-12h] Bitwert g_items[w].weapon, [bp-14h] x + y, [bp-16h]/[bp-18h] &f->reach_marks[k];
 *   in fig_menu_attack haelt -Og (unsigned)t in SI und laedt es nach Aufrufen neu (mov al,[bp-9]/mov ah,0/mov si,ax).
 * - "act = sel = -1" (Kette, sel zuerst); verschachtelte ?: fuer act (mov ax,k / jmp); r im Block ([bp-1ah]).
 * Auffaelliges: die Schleifen "for (i = 0; i < 7; i++) f->keys[i] = 0;" loeschen auch keys[6], das vorher als
 *   Merker (Kampf automatisch) getestet wird; "!weapon || (weapon && broken)" testet weapon doppelt (wie DSA 1).
 */
#include <stdio.h>
#include <string.h>
#include "schweif.h"

enum {					/* Menuewahl (nicht hero.action_id) */
	MENU_MOVE = 1,
	MENU_ATTACK = 2,
	MENU_GUARD = 3,
	MENU_SPELL = 4,
	MENU_USE_ITEM = 5,
	MENU_DROP_ITEM = 6,
	MENU_CHANGE_WEAPON = 7,
	MENU_TAKE_ITEM = 8,
	MENU_SHOW_VALUES = 9,
	MENU_QUIT = 10,
	MENU_REPEAT = 11,
	MENU_AUTO = 13
};
#define COL_DISABLED	0xe7		/* code_to_text: Farbe gesperrter Menuepunkt */
#define COL_WARN	0xf9		/* code_to_text: Farbe "mit Einschraenkung" */
#define KEY_AUTO	6		/* f->keys[6]: Kampf automatisch */

signed short fig_menu(Fight far *f, struct hero far *h, signed short x, signed short y)
{
	signed short act;
	signed short i;
	signed short sel;
	signed short done;
	signed short dx;
	signed short dy;
	signed short can_flee;

	done = 0;
	while (done == 0 && h->bp_left > 0) {
		if (g_quick_fight)
			game_idle();
		fight_check_keys(f);
		if (f->keys[KEY_AUTO] != 0) {
			register signed short k;

			k = 1;
			if (g_quick_fight) {
				if (h->npc_id != 0)
					k = 0;
				else
					fight_quick_stop(f);
			}
			if (k) {
				group_set_fb5(g_group, 0);
				group_set_fb6_b0(g_group, 0);
				for (i = 0; i < 7; i++)
					f->keys[i] = 0;
			}
		}
		if (f->keys[group_hero_pos(g_group, h)] != 0 && !g_quick_fight) {
			h->fb5 = 0;
			f->keys[group_hero_pos(g_group, h)] = 0;
		}
		if (f->mark_id != -1) {
			fig_list_remove(&f->list, f->mark_id, 0);
			f->mark_id = -1;
		}
		fig_set_mark(f, h->fighter_id);
		fight_draw(f);
		if ((unsigned char)h->scared) {
			/* Flucht */
			if ((unsigned char)h->tied || (unsigned char)h->petrified) {
				h->scared = 0;
			} else if (fight_path_search(f, h, x, y, 7, 1)) {
				fig_hero_place(f, h);
			}
			done = 1;
		} else if ((unsigned char)h->renegade || h->npc_id != 0 || h->fb5 != 0) {
			/* Computer kaempft */
			h->action_id = FIG_ACTION_WAIT;
			if (h->bp_left >= 3)
				ki_hero(f, h, x, y);
			done = 1;
		} else {
			act = sel = -1;
			sprintf(g_text_buf2, f->txt[18], h->bp_left);
			if (h->bp_left >= 3)
				strcpy(g_text_output_buf, f->txt[20]);
			else
				sprintf(g_text_output_buf, code_to_text(COL_DISABLED), f->txt[20]);
			if (h->bp_left >= 5 && h->is_mage() && h->ae != 0)
				strcpy(g_text_output_buf + 50, f->txt[22]);
			else
				sprintf(g_text_output_buf + 50, code_to_text(COL_DISABLED), f->txt[22]);
			if (f->id >= FIGHT_ID_SPECIAL || g_fight_end == 5) {
				sprintf(g_text_output_buf + 100, code_to_text(COL_DISABLED), f->txt[34]);
				can_flee = 0;
			} else {
				can_flee = 1;
				strcpy(g_text_output_buf + 100, f->txt[34]);
			}
			i = h->inv[INV_SLOT_RIGHT_HAND].item_id;
			if (h->bp_left < 2 || !h->nr_items)
				sprintf(g_text_output_buf + 150, code_to_text(COL_DISABLED), f->txt[24]);
			else if (!g_items[i].weapon || (g_items[i].weapon && h->inv[INV_SLOT_RIGHT_HAND].broken))
				sprintf(g_text_output_buf + 150, code_to_text(COL_WARN), f->txt[24]);
			else
				strcpy(g_text_output_buf + 150, f->txt[24]);
			if (h->bp_left >= 3 && h->inv[INV_SLOT_LEFT_HAND].item_id != 0)
				sprintf(g_text_output_buf + 200, f->txt[92],
					grammar_article(0x8002, h->inv[INV_SLOT_LEFT_HAND].item_id, 0));
			else
				sprintf(g_text_output_buf + 200, code_to_text(COL_DISABLED), f->txt[23]);
			if (!h->nr_items)
				sprintf(g_text_output_buf + 250, code_to_text(COL_DISABLED), f->txt[40]);
			else if (h->attrib[ATTRIB_KK].cur * 110 <= h->load)
				sprintf(g_text_output_buf + 250, code_to_text(COL_WARN), f->txt[40]);
			else
				strcpy(g_text_output_buf + 250, f->txt[40]);
			if (h->bp_left >= 2 && h->nr_items != 0)
				strcpy(g_text_output_buf + 300, f->txt[16]);
			else
				sprintf(g_text_output_buf + 300, code_to_text(COL_DISABLED), f->txt[16]);
			dx = x + y < 15 ? 90 : -90;
			dy = x + y < 15 ? -30 : 30;
			dlg_set_pos_x(g_dialog, dx);
			dlg_set_pos_y(g_dialog, dy);
			do {
				sel = dlg_choose(g_dialog, g_text_buf2, 10, f->txt[19], g_text_output_buf,
					g_text_output_buf + 50, f->txt[25], f->txt[94], f->txt[45], g_text_output_buf + 100,
					g_text_output_buf + 150, f->txt[21], f->txt[27]);
				if (sel == 5) {
					/* Untermenue Werte / Kampfstil */
					sel = dlg_choose(g_dialog, g_text_buf2, 2, f->txt[26], f->txt[93]);
					if (sel != -1) {
						if (sel == 1) {
							act = MENU_SHOW_VALUES;
						} else {
							sprintf(g_text_buf2, f->txt[52], h->attack_style == 0 ? f->txt[54] :
								h->attack_style == 1 ? f->txt[53] : f->txt[55]);
							sel = dlg_choose(g_dialog, g_text_buf2, 3, f->txt[54], f->txt[53], f->txt[55]);
							if (sel != -1)
								h->set_attack_style(sel - 1);
							sel = -2;
							sprintf(g_text_buf2, f->txt[18], h->bp_left);
						}
					} else {
						sel = -2;
					}
				} else if (sel == 4) {
					/* Untermenue Gegenstaende */
					sel = dlg_choose(g_dialog, g_text_buf2, 3, g_text_output_buf + 200,
						g_text_output_buf + 250, g_text_output_buf + 300);
					if (sel != -1)
						act = sel == 1 ? MENU_USE_ITEM : sel == 2 ? MENU_DROP_ITEM : MENU_TAKE_ITEM;
					else
						sel = -2;
				} else if (sel != -1) {
					act = sel == 1 ? MENU_MOVE : sel == 2 ? MENU_ATTACK : sel == 3 ? MENU_SPELL :
						sel == 6 ? MENU_QUIT : sel == 7 ? (can_flee ? MENU_AUTO : -1) :
						sel == 8 ? MENU_CHANGE_WEAPON : sel == 9 ? MENU_GUARD : MENU_REPEAT;
					if (act == -1)
						sel = -2;
				}
				g_quick_fight = 0;
			} while (sel < 0);
			if (act != MENU_MOVE) {
				register signed short k;
				register signed short changed;

				changed = 0;
				for (k = 0; f->reach_marks[k] != -1; k++) {
					fig_list_remove(&f->list, f->reach_marks[k], 0);
					f->reach_marks[k] = -1;
					changed = 1;
				}
				if (changed) {
					g_fig_redraw_all = 1;
					fight_draw(f);
				}
			}
			if (act == MENU_MOVE) {
				register signed short k;
				register signed short changed;

				fig_menu_move(f, h, &x, &y);
				changed = 0;
				for (k = 0; f->reach_marks[k] != -1; k++) {
					fig_list_remove(&f->list, f->reach_marks[k], 0);
					f->reach_marks[k] = -1;
					changed = 1;
				}
				if (changed) {
					g_fig_redraw_all = 1;
					fight_draw(f);
				}
			} else if (act == MENU_ATTACK) {
				done = fig_menu_attack(f, h, x, y);
			} else if (act == MENU_GUARD) {
				done = fig_menu_guard(f, h);
			} else if (act == MENU_SPELL) {
				done = fig_menu_spell(f, h, x, y);
			} else if (act == MENU_USE_ITEM) {
				done = fig_menu_use_item(f, h, x, y);
			} else if (act == MENU_TAKE_ITEM) {
				fig_menu_take_item(f, h);
			} else if (act == MENU_CHANGE_WEAPON) {
				fig_menu_change_weapon(f, h);
			} else if (act == MENU_SHOW_VALUES) {
				fig_menu_show_values(f, h);
			} else if (act == MENU_AUTO) {
				g_fight_screen_on = 0;
				act = dlg_choose(g_dialog, f->txt[62], 3, f->txt[63], f->txt[64], f->txt[65]);
				if (act == 1) {
					/* nur dieser Held automatisch */
					signed short r;

					r = 0;
					if (h->is_mage()) {
						r = dlg_yes_no(g_dialog, f->txt[47]);
						if (r != -1)
							h->set_fb6_b0(r);
					} else {
						h->fb6_b0 = 0;
					}
					if (r != -1) {
						h->fb5 = 1;
						if (all_heroes_down(g_group)) {
							g_key_last = 0;
							for (i = 0; i < 7; i++)
								f->keys[i] = 0;
						}
					}
					g_fight_screen_on = 1;
				} else if (act > 1) {
					/* alle Helden automatisch (3: Schnellkampf) */
					signed short r;

					if (get_mage_hero(g_group) != 0)
						r = dlg_yes_no(g_dialog, f->txt[47]);
					else
						r = 0;
					if (r != -1) {
						group_set_fb5(g_group, 1);
						group_set_fb6_b0(g_group, r);
						for (i = 0; i < 7; i++)
							f->keys[i] = 0;
						if (act == 2)
							g_fight_screen_on = 1;
						else
							fight_quick_start(f);
					} else {
						g_fight_screen_on = 1;
					}
				} else {
					g_fight_screen_on = 1;
				}
			} else if (act == MENU_DROP_ITEM) {
				fig_menu_drop_item(f, h);
			} else if (act == MENU_QUIT) {
				g_fight_screen_on = 0;
				if (dlg_yes_no(g_dialog, f->txt[46]) == 1) {
					done = 1;
					g_in_fight = 0;
					g_game_state = 2;
				} else {
					g_fight_screen_on = 1;
				}
			} else if (act == MENU_REPEAT) {
				done = fig_menu_repeat(f, h);
			}
			dlg_set_pos_x(g_dialog, 0);
			dlg_set_pos_y(g_dialog, 0);
		}
	}
	if (g_fight_end == 5 && get_first_active_hero(g_group) != h &&
	    (h->action_id == FIG_ACTION_MELEE_ATTACK || h->action_id == FIG_ACTION_SPELL ||
	     h->action_id == FIG_ACTION_RANGE_ATTACK || h->action_id == FIG_ACTION_USE_ITEM))
		g_fight_end = 0;
	return h->action_id;
}

/* Bewegen (gefesselt: Probe auf MU + 2, misslungen MU -2 fuer 7 Stunden) */
void fig_menu_move(Fight far *f, struct hero far *h, signed short far *px, signed short far *py)
{
	signed short slot;

	if ((unsigned char)h->tied) {
		if (test_attrib(h, ATTRIB_MU, 2) > 0) {
			h->tied = 0;
		} else if (h->attrib[ATTRIB_MU].cur > 4) {
			slot = get_free_effect(h);
			add_effect(h, slot, 7 * TICKS_PER_HOUR, ATTRIB_MU, -2, EFFECT_ATTRIB);
		}
	}
	if (!(signed char)h->tied) {
		h->action_id = FIG_ACTION_MOVE;
		if (h->bp_left > 0) {
			fig_move_hero(f, h, px, py);
			h->chamaelioni = 0;
		} else {
			sprintf(g_text_output_buf, f->txt[17], h->name);
			dlg_message(g_dialog, g_text_output_buf);
		}
		return;
	}
	sprintf(g_text_output_buf, f->txt[7], h->name);
	dlg_message(g_dialog, g_text_output_buf);
}

/* Angreifen: Ziel waehlen und pruefen; 1 = Aktion gewaehlt */
signed short fig_menu_attack(Fight far *f, struct hero far *h, register signed short x, signed short y)
{
	signed short tx;
	signed short ty;
	signed short wclass;
	signed short ret = 0;
	unsigned char t;

	if (h->bp_left >= 3) {
		tx = x;
		ty = y;
		t = TARGET_ID_NONE;
		if ((wclass = hand_weapon_class(h)) == -1) {
			t = fig_cb_select_target(f, &tx, &ty, 1);
		} else if (range_attack_check_ammo(f, h, 1)) {
			t = fig_cb_select_target(f, &tx, &ty, 99);
		}
		if (t != TARGET_ID_NONE) {
			if (t == 0 || t >= 50) {
				dlg_message(g_dialog, f->txt[28]);
			} else if (t == (unsigned char)get_hero_index(h) + 1) {
				dlg_message(g_dialog, f->txt[3]);
			} else if ((t >= TARGET_ID_ENEMY && t < TARGET_ID_SPECIAL &&
				    (unsigned char)g_enemies[t - TARGET_ID_ENEMY].renegade) ||
				   (t >= TARGET_ID_SPECIAL && (unsigned char)g_enemies[t - TARGET_ID_SPECIAL].renegade)) {
				sprintf(g_text_output_buf, f->txt[83], grammar_article(0x8000,
					g_enemies[t - (t < TARGET_ID_SPECIAL ? TARGET_ID_ENEMY : TARGET_ID_SPECIAL)].mon_id, 1));
				dlg_message(g_dialog, g_text_output_buf);
				h->action_id = FIG_ACTION_WAIT;
				h->enemy_id = -1;
			} else if ((t < TARGET_ID_ENEMY && (unsigned char)g_heroes[t - TARGET_ID_HERO].dead) ||
				   (t >= TARGET_ID_ENEMY && t < TARGET_ID_SPECIAL && !fight_check_state(&g_enemies[t - TARGET_ID_ENEMY])) ||
				   (t >= TARGET_ID_SPECIAL && !fight_check_state(&g_enemies[t - TARGET_ID_SPECIAL]))) {
				dlg_message(g_dialog, f->txt[29]);
				h->action_id = FIG_ACTION_WAIT;
				h->enemy_id = -1;
			} else if (wclass != -1 && (manhattan_dist(x, y, tx, ty) < 2 ||
				   (manhattan_dist(x, y, tx, ty) == 2 && x != tx && y != ty))) {
				/* Fernwaffe gegen den Nachbarn */
				sprintf(g_text_output_buf, f->txt[51], wclass == 4 ? f->txt[81] : f->txt[82]);
				dlg_message(g_dialog, g_text_output_buf);
				h->action_id = FIG_ACTION_WAIT;
				h->enemy_id = -1;
			} else {
				h->attack_type = 0;
				if (wclass == -1) {
					f->style_tmp = h->attack_style;
					h->set_attack_type(f->style_tmp == 1 ? 2 : f->style_tmp == 0 ? 0 : -2);
					h->set_enemy_id(t);
					h->set_target_x(tx);
					h->set_target_y(ty);
					h->add_bp_left(-3);
					h->action_id = FIG_ACTION_MELEE_ATTACK;
					ret = 1;
				} else {
					h->set_enemy_id(t);
					h->set_target_x(tx);
					h->set_target_y(ty);
					h->add_bp_left(-3);
					h->set_action_id(wclass > 0 ? FIG_ACTION_RANGE_ATTACK : FIG_ACTION_MELEE_ATTACK);
					ret = 1;
				}
			}
		}
	} else {
		sprintf(g_text_output_buf, f->txt[17], h->name);
		dlg_message(g_dialog, g_text_output_buf);
	}
	return ret;
}
