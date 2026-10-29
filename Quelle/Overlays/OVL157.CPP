/* OVL157 - Overlay O157 (Segment 0x227d in SCHWEIF.EXE): Kampf - automatischer Zug eines Helden (BE KI_hero, DSA1
 * seg036). ki_hero(f, h, x, y): Vorbereitung je nach npc_id (1: Bogen 0x13 mit Pfeilen in die Haende per
 * get_item_pos/move_item, 6/8: Flucht scared), Sonderkaempfe g_fight_end 5/3 und BP == 1 setzen BP 0; dann
 * Schleife bis fertig/BP verbraucht: Flucht (fight_path_search Modus 7, fig_hero_place, fight_find_cell), Zauber
 * (ki_select_spell_target, Typ >= 7, AE > 10, fb6_b0), Fernkampf (hand_weapon_class / range_attack_check_ammo /
 * ki_range_target / ki_change_hero_weapon), Nahkampf (ki_search_target) oder hinlaufen (fight_path_search
 * Modus 5/4). range_attack_check_ammo(f, h, mode) (BE range_attack_check_ammo): Fernwaffe der rechten Hand
 * pruefen - Munition der linken Hand verbrauchen (drop_item), Wurfwaffen in f->dropped (hoechstens 30) merken,
 * sonst Meldung f->txt[8/9/0x5f] mit Heldenname; mode 0 verbraucht, 1 prueft, 2 prueft ohne Meldung.
 *
 * BCC -mlarge -3 -Ot -Og -c -Y OVL157.CPP   -> 2402/2402 Bytes GLEICH (18.10.2026), cobjfix laeuft
 *   (Namensfassung mit schweif.h, Welle N11: 19.10.2026 GLEICH, cobjfix LINKGLEICH)
 * - BP-Begrenzung "h->bp_left = 0; if (h->bp_left > 12) h->bp_left = 12; else ;" (Makro im Original, hier
 *   BP_CLEAR) -> toter jmp short.
 * - "h->bp_left += (signed char)-5" -> add byte es:[..],0fbh (mit -5 bzw. -= 5: mov al/add/mov).
 * - -Og-Zwischenwert [bp-11h] fuer Byte +bc (Bits renegade/tied als (signed char) -> shr/and/cbw/or ax,ax);
 *   (unsigned char)h->scared / h->fb6_b0 -> and ax,1 / or al,al.
 * - Zwei getrennte Block-Variablen "signed char sc" (je [bp-12h]) - die Parameterkopien der inline-Setter
 *   set_spell_id/set_enemy_id liegen dann beide auf [bp-13h]; eine gemeinsame Variable verschiebt [bp-11h]/[bp-13h].
 * - range_attack_check_ammo: "register" Parameter mode -> SI (Prolog), ret ohne register -> DI; switch-Rumpf-
 *   Reihenfolge wie im Original, "if (ammo != ITEM_ARROWS) .. else" (jz auf den zweiten Teil).
 * - range_attack_check_ammo steht unten -> far call aus ki_hero (TLINK: nop/push cs/call near).
 * - Bisheriger Name von O157_0726 "fight_hero_action" war falsch (prueft nur Fernwaffe und Munition).
 */
#include <stdio.h>
#include "schweif.h"

#define BP_CLEAR(h) { (h)->bp_left = 0; if ((h)->bp_left > 12) (h)->bp_left = 12; else ; }

#define ITEM_ARROWS	10		/* Pfeile (Munition fuer 9, 0x13, 0x100, 0x13d) */
#define ITEM_BOLTS	13		/* Bolzen (Munition fuer 0xc, 0x101) */
#define ITEM_BOW_13	0x13		/* Bogen, den NSC 1 vor dem ersten Zug nimmt */
#define NR_DROPPED	30		/* Fight.dropped[] */

void ki_hero(Fight far *f, struct hero far *h, signed short x, signed short y)
{
	signed short magic;
	signed short x_bak;
	signed short y_bak;
	signed short res;
	signed short x2;
	signed short y2;
	struct hero far *first;
	register signed short done;
	register signed short reach;

	done = 0;
	magic = 1;
	if (g_quick_fight)
		game_idle();
	h->action_id = FIG_ACTION_MOVE;
	h->spell_id = -1;
	if (h->npc_id != 0) {
		/* NSC */
		if (h->npc_id == 1) {
			if (f->round == 0 && h->inv[INV_SLOT_RIGHT_HAND].item_id != ITEM_BOW_13 &&
			    get_item_pos(h, ITEM_ARROWS) != -1 && get_item_pos(h, ITEM_BOW_13) != -1) {
				move_item(h, INV_SLOT_RIGHT_HAND, get_item_pos(h, ITEM_BOW_13));
				if (h->inv[INV_SLOT_LEFT_HAND].item_id != ITEM_ARROWS)
					move_item(h, INV_SLOT_LEFT_HAND, get_item_pos(h, ITEM_ARROWS));
			}
		} else if (h->npc_id == 8) {
			h->scared = 1;
		} else if (h->npc_id == 6 && h->le <= 20) {
			h->scared = 1;
		}
		first = fig_get_first_active_hero(g_group);
		if (first->npc_id != 0)
			h->scared = 1;
		if ((unsigned char)h->scared && fight_find_cell(get_hero_index(h) + FIG_ID_HERO, &x2, &y2))
			h->action_id = FIG_ACTION_MOVE;
		if (g_fight_end == 5) {
			BP_CLEAR(h);
		}
		if (g_fight_end == 3) {
			BP_CLEAR(h);
		}
	}
	if (h->bp_left == 1) {
		BP_CLEAR(h);
	}

	while (!done && h->bp_left > 0) {
		if (g_quick_fight)
			game_idle();
		if ((unsigned char)h->scared && h->bp_left > 0) {
			/* Flucht */
			if (!(signed char)h->tied) {
				reach = fight_path_search(f, h, x, y, 7, 1);
				if (reach) {
					x_bak = x;
					y_bak = y;
					fig_hero_place(f, h);
					h->enemy_id = -1;
					if (fight_find_cell(get_hero_index(h) + FIG_ID_HERO, &x, &y)) {
						h->action_id = FIG_ACTION_MOVE;
						if (x_bak == x && y_bak == y) {
							BP_CLEAR(h);
						}
					} else {
						BP_CLEAR(h);
					}
					if (h->bp_left < 3) {
						BP_CLEAR(h);
					}
				} else {
					BP_CLEAR(h);
				}
			} else {
				BP_CLEAR(h);
			}
		} else {
			if (h->is_mage() && h->ae > 10 && magic && (unsigned char)h->fb6_b0) {
				/* zaubern */
				signed char sc;

				sc = -1;
				sc = ki_select_spell_target(f, h, (signed char)h->renegade, x, y);
				if (sc != -1) {
					h->action_id = FIG_ACTION_SPELL;
					h->set_spell_id(sc);
					h->bp_left += (signed char)-5;
					if (h->bp_left < 0) h->bp_left = 0; else ;
					done = 1;
				} else {
					magic = 0;
					h->action_id = FIG_ACTION_MOVE;
					h->spell_id = -1;
				}
			}
			if (h->action_id == FIG_ACTION_MOVE && h->bp_left > 0) {
				if (hand_weapon_class(h) != -1) {
					/* Fernwaffe */
					if (range_attack_check_ammo(f, h, 2)) {
						res = ki_range_target(f, h, (signed char)h->renegade, x, y);
						if (res != 0) {
							if (res == 2) {
								if (!ki_change_hero_weapon(f, h))
									done = 1;
							} else
								h->action_id = FIG_ACTION_RANGE_ATTACK;
						}
						h->bp_left += (signed char)-3;
						if (h->bp_left < 0) h->bp_left = 0; else ;
						done = 1;
					} else {
						if (!ki_change_hero_weapon(f, h))
							done = 1;
					}
				} else {
					/* Nahkampf */
					h->enemy_id = -1;
					if (h->bp_left >= 3) {
						signed char sc;

						h->enemy_id = -1;
						sc = ki_search_target(f, x, y, (signed char)h->renegade, 1, 1);
						if (sc != 0)
							h->set_enemy_id(sc);
					}
					if (h->enemy_id != -1) {
						h->action_id = FIG_ACTION_MELEE_ATTACK;
						h->bp_left += (signed char)-3;
						if (h->bp_left < 0) h->bp_left = 0; else ;
						done = 1;
					} else if (!(signed char)h->tied) {
						if (!(signed char)h->renegade)
							reach = fight_path_search(f, h, x, y, 5, 1);
						else
							reach = fight_path_search(f, h, x, y, 4, 1);
						if (reach) {
							x_bak = x;
							y_bak = y;
							fig_hero_place(f, h);
							h->action_id = FIG_ACTION_MOVE;
							h->enemy_id = -1;
							fight_find_cell(get_hero_index(h) + FIG_ID_HERO, &x, &y);
							if (x_bak == x && y_bak == y) {
								BP_CLEAR(h);
							}
							if (h->bp_left < 3) {
								BP_CLEAR(h);
							}
						} else {
							BP_CLEAR(h);
						}
					} else {
						BP_CLEAR(h);
					}
				}
			}
		}
		if (g_quick_fight)
			game_idle();
	}
}

/* Fernwaffe und Munition pruefen (mode 1/2) bzw. verbrauchen (mode 0); 1 = Schuss/Wurf moeglich */
signed short range_attack_check_ammo(Fight far *f, struct hero far *h, register signed short mode)
{
	signed short weapon;
	signed short ammo;
	signed short ret;

	ret = 0;
	if (g_quick_fight)
		game_idle();
	weapon = h->inv[INV_SLOT_RIGHT_HAND].item_id;
	ammo = h->inv[INV_SLOT_LEFT_HAND].item_id;
	switch (weapon) {
	case 5:
	case 0x10:
	case 0x11:
	case 0x21:
	case 0x62:
	case 0x89:
	case 0xd8:
	case 0xda:
	case 0x102:
	case 0x11c:
		/* Wurfwaffen: nach dem Kampf zurueck */
		if (!mode) {
			if (f->nr_dropped < NR_DROPPED)
				f->dropped[f->nr_dropped++] = weapon;
			drop_item(h, INV_SLOT_RIGHT_HAND, 1);
			if (ammo == weapon)
				move_item(h, INV_SLOT_RIGHT_HAND, INV_SLOT_LEFT_HAND);
		}
		ret = 1;
		break;
	case 9:
	case 0x13:
	case 0x100:
	case 0x13d:
		/* Boegen */
		if (ammo != ITEM_ARROWS) {
			if (mode != 2) {
				sprintf(g_text_output_buf, f->txt[8], (char far *)h->name);
				dlg_message(g_dialog, g_text_output_buf);
			}
		} else {
			if (!mode)
				drop_item(h, INV_SLOT_LEFT_HAND, 1);
			ret = 1;
		}
		break;
	case 12:
	case 0x101:
		/* Armbrueste */
		if (ammo != ITEM_BOLTS) {
			if (mode != 2) {
				sprintf(g_text_output_buf, f->txt[9], (char far *)h->name);
				dlg_message(g_dialog, g_text_output_buf);
			}
		} else {
			if (!mode)
				drop_item(h, INV_SLOT_LEFT_HAND, 1);
			ret = 1;
		}
		break;
	case ITEM_ARROWS:
	case ITEM_BOLTS:
		/* Munition in der Waffenhand */
		if (mode != 2) {
			sprintf(g_text_output_buf, f->txt[0x5f], (char far *)h->name);
			dlg_message(g_dialog, g_text_output_buf);
		}
		break;
	}
	return ret;
}
