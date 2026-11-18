/* OVL155 - Overlay O155 (Segment 0x2273 in SCHWEIF.EXE): Unteraktionen des Kampfmenues O154 (vgl. DSA 1 seg033
 * FIG_menu). Kampf f (txt, seq, id), Held h (bp_left, action_id, spell_id, enemy_id, target_x/target_y).
 * fig_menu_guard: Sichern (Aktion FIG_ACTION_GUARD bzw. FIG_ACTION_WAIT). fig_menu_spell(f, h, x, y): Zaubern - nur
 * Typ >= 7, BP >= 5, Zauber per select_spell, Beschreibung g_spell_descriptions (where, target_type, range), Ziel
 * fig_cb_select_target mit Pruefungen (abtruenniger Gegner, tote Helden, fight_check_state), Aktion 4, AE-Kosten
 * hero_spell_cost. fig_menu_use_item: Gegenstand der linken Hand benutzen (Angriffsgegenstaende g_items_attack mit Ziel,
 * Sonderfall 0x61 bei g_fight_end == 3, sonst benutzbar/Nahrung -> hero_set_target auf sich), Aktion 5, BP -3.
 * fig_menu_take_item/fig_menu_drop_item: Rucksack (15..30) als Liste g_menu_texts[16] (Puffer g_text_output_buf +
 * 40 n) -> Gegenstand in die linke Hand nehmen (move_item, BP -2) bzw. ablegen (drop_item(h, slot, -1), BP -1).
 * fig_menu_change_weapon: Waffe wechseln (weapon und can_use_item, einmal "leere Hand"), rechte Hand, bei
 * geaenderter Waffenart fig_stand_pic. fig_menu_show_values: Kampfwerte anzeigen (AT/PA aus O077 bzw. Waffen-
 * tabelle per calc_damage_range, KK-Zuschlag). fig_menu_repeat: letzte Aktion wiederholen (Pruefungen ueber
 * target_type/enemy_mr_target/hero_mr_target, fig_target_in_reach, range_attack_check_ammo, Abstand).
 * fig_target_in_reach: Ziel erreichbar (fig_line_of_sight, Abstand manhattan_dist).
 *
 * BCC -mlarge -3 -Ot -Og -Oi -c -Y OVL155.CPP   -> 7167/7167 Bytes GLEICH (18.10.2026), cobjfix laeuft
 *   (-rd ebenso GLEICH; ohne -Og 5213 Abweichungen).
 *   (Namensfassung mit schweif.h, Welle N11: 19.10.2026 GLEICH, cobjfix LINKGLEICH)
 * - C++ wegen der inline-Elementfunktionen: set_bp_left (Klemmen auf 12), add_bp_left (Klemmen auf 0; -1/-2/-3 ->
 *   dec byte/add byte 0feh/0fdh), set_spell_id/set_enemy_id (char-Kopien), set_target_x/set_target_y (Kopien),
 *   Getter mit unsigned-char-Rueckgabe auf Aufrufergebnis (hero_mr_target(h)->is_dead()/is_scared(),
 *   enemy_mr_target(h)->is_scared()/get_mon_id(), get_name()) -> this-Kopien [bp-4]..[bp-14h] in fig_menu_repeat.
 *   Bitfelder von Held und Gegner im Original unsigned char (or al,al / cmp al,1): hier mit (unsigned char)-Cast.
 * - fig_menu_spell: ok (SI) auf Funktionsebene - im Block deklariert wandern die inline-Kopien des Blocks hinter
 *   die des else-Zweigs. -Og haelt (unsigned)t in DI.
 * - fig_menu_repeat: switch ueber eine Block-Variable "w = rechte Hand" (mov [bp-16h],ax / mov ax,[bp-16h]); x/y
 *   bzw. x1..k in eigenen Bloecken (teilen [bp-16h]..). fig_target_in_reach: "d == 1 || (d == 2 && x2 != x1 &&
 *   y2 != y1)" als if (cmp di,1 direkt; ein switch macht mov ax,di / cmp ax,1).
 * - Listen-Aufrufe dlg_choose mit allen 16 Eintraegen g_menu_texts[0..15] als einzelne far-Zeiger.
 * - ds:6c46 ("%s %s %s") mit vier folgenden Leerstrings (6c4f..6c52) - vermutlich echte Literale des Moduls; hier
 *   extern (cobjfix ordnet _s_6c46+9..+0c zu).
 * Originalfehler/Auffaelliges: fig_menu_guard setzt die BP auf 0 (set_bp_left), BEVOR "BP >= 3" geprueft wird -
 *   Sichern ergibt so immer Aktion FIG_ACTION_WAIT (DSA 1: erst pruefen, dann BP = 0); fig_menu_spell nimmt den
 *   Zauber Eigenschaften (SP_EIGENSCHAFTEN) nur fuer Gegner 10..29 von der Pruefung "abtruennig" aus, nicht fuer
 *   30..49; fig_menu_use_item prueft beim Ziel >= 30 keine Obergrenze 50; fig_menu_repeat ruft target_type(h) bis
 *   zu sechsmal hintereinander auf.
 */
#include <stdio.h>
#include <string.h>
#include "schweif.h"

/* Phase B: eigene Daten des Moduls (ds:6c36..6c54, dsana) - initialisierte Daten in Adressreihenfolge, danach der Literalpool */
struct dirs4 g_fig_dirs_ki = { { { 1, 0 }, { 0, -1 }, { -1, 0 }, { 0, 1 } } };	/* Kampffeld-Richtung 0: x+1, 1: y-1, 2: x-1, 3: y+1 */

#define ITEM_SPECIAL_61	0x61		/* Sondergegenstand (g_fight_end == 3: gegen Gegner 10 einsetzen) */
#define NR_MENU_ITEMS	16		/* Rucksackplaetze 15..30 in g_menu_texts */
#define MENU_ITEM_LEN	40		/* Textlaenge je Eintrag in g_text_output_buf */

/* Sichern */
signed short fig_menu_guard(Fight far *f, struct hero far *h)
{
	signed short ret = 1;

#if FIX_0187
	/* Fix #187: erst die BP pruefen, dann auf 0 setzen (wie DSA 1) */
	h->enemy_id = -1;
	if (h->bp_left >= 3)
		h->action_id = FIG_ACTION_GUARD;
	else
		h->action_id = FIG_ACTION_WAIT;
	h->set_bp_left(0);
#else
	h->set_bp_left(0);
	h->enemy_id = -1;
	if (h->bp_left >= 3)
		h->action_id = FIG_ACTION_GUARD;
	else
		h->action_id = FIG_ACTION_WAIT;
#endif
	return ret;
}

/* Zaubern: Zauber und Ziel waehlen; 1 = Aktion gewaehlt */
signed short fig_menu_spell(Fight far *f, struct hero far *h, signed short x, signed short y)
{
	signed short range;
	signed short tx;
	signed short ty;
	signed short done = 0;
	struct spell_desc far *desc;
	unsigned char t;
	register signed short ok;

	if (h->type < HERO_TYPE_WITCH) {
		dlg_message(g_dialog, f->txt[0x38]);
	} else if (h->bp_left >= 5) {
		h->set_spell_id(select_spell(h, 0));
		if (h->spell_id > 0) {
			h->action_id = FIG_ACTION_WAIT;
			h->enemy_id = -1;
			desc = &g_spell_descriptions[h->spell_id];
			if (desc->where == -1) {
				/* nicht im Kampf */
				dlg_message(g_dialog, f->txt[0x39]);
			} else if (desc->target_type != 0) {
				tx = x;
				ty = y;
				range = 1;
				if (desc->range > 0)
					range = 99;
				t = fig_cb_select_target(f, &tx, &ty, range);
				ok = 1;
				if (t == TARGET_ID_NONE) {
					ok = 0;
				} else if (t >= 50) {
					dlg_message(g_dialog, f->txt[0x1c]);
					ok = 0;
				} else if (t == TARGET_ID_SELF) {
					if (desc->target_type != 4) {
						dlg_message(g_dialog, f->txt[4]);
						ok = 0;
					}
				} else if (t < TARGET_ID_ENEMY && desc->target_type != 2 && desc->target_type != 3) {
					dlg_message(g_dialog, f->txt[5]);
					ok = 0;
				} else if (t >= TARGET_ID_ENEMY && t < 50 && desc->target_type != 1 && desc->target_type != 3) {
					dlg_message(g_dialog, f->txt[6]);
					ok = 0;
				}
				if (ok) {
					if ((h->spell_id != SP_EIGENSCHAFTEN && t >= TARGET_ID_ENEMY && t < TARGET_ID_SPECIAL &&
					     (unsigned char)g_enemies[t - TARGET_ID_ENEMY].renegade) ||
#if FIX_0188
						/* Fix #188: Ausnahme fuer Zauber Eigenschaften auch bei Sondergegnern 30..49 */
						(h->spell_id != SP_EIGENSCHAFTEN && t >= TARGET_ID_SPECIAL && t < 50 && (unsigned char)g_enemies[t - TARGET_ID_SPECIAL].renegade)) {
#else
						(t >= TARGET_ID_SPECIAL && t < 50 && (unsigned char)g_enemies[t - TARGET_ID_SPECIAL].renegade)) {
#endif
						sprintf(g_text_output_buf, f->txt[0x53], grammar_article(0x8000,
							g_enemies[t - (t < TARGET_ID_SPECIAL ? TARGET_ID_ENEMY : TARGET_ID_SPECIAL)].mon_id, 1));
						dlg_message(g_dialog, g_text_output_buf);
					} else if ((t >= TARGET_ID_HERO && t < TARGET_ID_ENEMY &&
						    (unsigned char)g_heroes[t - TARGET_ID_HERO].dead) ||
						(t >= TARGET_ID_ENEMY && t < TARGET_ID_SPECIAL && h->spell_id != SP_SKELETTARIUS &&
						 !fight_check_state(&g_enemies[t - TARGET_ID_ENEMY])) ||
						(t >= TARGET_ID_SPECIAL && t < 50 && h->spell_id != SP_SKELETTARIUS &&
						 !fight_check_state(&g_enemies[t - TARGET_ID_SPECIAL]))) {
						dlg_message(g_dialog, f->txt[0x1d]);
					} else {
						h->action_id = FIG_ACTION_SPELL;
						h->set_enemy_id(t);
						h->set_target_x(tx);
						h->set_target_y(ty);
						done = 1;
					}
				}
			} else {
				/* ohne Ziel */
				h->action_id = FIG_ACTION_SPELL;
				h->enemy_id = 0;
				h->set_target_x(x);
				h->set_target_y(y);
				done = 1;
			}
		}
		if (h->action_id == FIG_ACTION_SPELL && hero_spell_cost(h->spell_id, 1) > h->ae) {
			dlg_message(g_dialog, f->txt[0x3a]);
			h->action_id = FIG_ACTION_WAIT;
			done = 0;
		}
	} else {
		sprintf(g_text_output_buf, f->txt[0x11], h->name);
		dlg_message(g_dialog, g_text_output_buf);
	}
	return done;
}

/* Gegenstand der linken Hand benutzen; 1 = Aktion gewaehlt */
signed short fig_menu_use_item(Fight far *f, struct hero far *h, signed short x, signed short y)
{
	register signed short ret = 0;
	signed short tx;
	signed short ty;
	signed short t;
	signed short ok = 1;
	register signed short id = h->inv[INV_SLOT_LEFT_HAND].item_id;

	if (h->bp_left >= 3) {
		if (id == 0) {
			dlg_message(g_dialog, f->txt[0x47]);
		} else if (is_in_word_array(id, g_items_attack) != -1) {
			/* Angriffsgegenstand: Ziel waehlen */
			tx = x;
			ty = y;
			t = fig_cb_select_target(f, &tx, &ty, 99);
			if (t == TARGET_ID_NONE) {
				ok = 0;
			} else if (t == 0xfe) {
				dlg_message(g_dialog, f->txt[0x1c]);
				ok = 0;
			} else if (t == TARGET_ID_SELF) {
				dlg_message(g_dialog, f->txt[0x4b]);
				ok = 0;
			}
			if (ok) {
				if ((t >= TARGET_ID_ENEMY && t < TARGET_ID_SPECIAL && (unsigned char)g_enemies[t - TARGET_ID_ENEMY].renegade) ||
#if FIX_0188
				    /* Fix #188: Obergrenze 50 fuer Sondergegner */
				    (t >= TARGET_ID_SPECIAL && t < 50 && (unsigned char)g_enemies[t - TARGET_ID_SPECIAL].renegade)) {
#else
				    (t >= TARGET_ID_SPECIAL && (unsigned char)g_enemies[t - TARGET_ID_SPECIAL].renegade)) {
#endif
					sprintf(g_text_output_buf, f->txt[0x53], grammar_article(0x8000,
						g_enemies[t - (t < TARGET_ID_SPECIAL ? TARGET_ID_ENEMY : TARGET_ID_SPECIAL)].mon_id, 1));
					dlg_message(g_dialog, g_text_output_buf);
				} else if ((t >= TARGET_ID_HERO && t < TARGET_ID_ENEMY && (unsigned char)g_heroes[t - TARGET_ID_HERO].dead) ||
					(t >= TARGET_ID_ENEMY && t < TARGET_ID_SPECIAL && !fight_check_state(&g_enemies[t - TARGET_ID_ENEMY])) ||
#if FIX_0188
					/* Fix #188: Obergrenze 50 fuer Sondergegner */
					(t >= TARGET_ID_SPECIAL && t < 50 && !fight_check_state(&g_enemies[t - TARGET_ID_SPECIAL]))) {
#else
					(t >= TARGET_ID_SPECIAL && !fight_check_state(&g_enemies[t - TARGET_ID_SPECIAL]))) {
#endif
					dlg_message(g_dialog, f->txt[0x1d]);
				} else {
					h->action_id = FIG_ACTION_USE_ITEM;
					h->set_enemy_id(t);
					h->set_target_x(tx);
					h->set_target_y(ty);
					h->add_bp_left(-3);
					ret = 1;
				}
			}
		} else {
			signed short done = 0;

			if (g_fight_end == 3 && id == ITEM_SPECIAL_61 && !(signed char)g_enemies->dead) {
				h->enemy_id = TARGET_ID_ENEMY;
				h->action_id = FIG_ACTION_USE_ITEM;
				h->add_bp_left(-3);
				ret = 1;
				done = 1;
			}
			if (!done) {
				if (!g_items[id].useable && !g_items[id].food) {
					/* nicht benutzbar */
					if (is_in_word_array(id, g_items_plural) != -1)
						sprintf(g_text_output_buf, f->txt[0x62], grammar_name(g_item_names[id]));
					else
						sprintf(g_text_output_buf, f->txt[0x63], grammar_article(0, id, 0));
					dlg_message(g_dialog, g_text_output_buf);
				} else {
					hero_set_target(h, h);
					h->action_id = FIG_ACTION_USE_ITEM;
					h->add_bp_left(-3);
					ret = 1;
				}
			}
		}
	} else {
		sprintf(g_text_output_buf, f->txt[0x11], h->name);
		dlg_message(g_dialog, g_text_output_buf);
	}
	return ret;
}

/* Gegenstand aus dem Rucksack in die linke Hand nehmen */
void fig_menu_take_item(Fight far *f, struct hero far *h)
{
	signed short n;
	signed short id;
	signed short old;
	signed short i;
	signed short sel;
	signed short slots[NR_MENU_ITEMS];

	if (h->bp_left >= 2) {
		n = 0;
		for (i = INV_SLOT_KNAPSACK; i < NR_INV_SLOTS; i++) {
			id = h->inv[i].item_id;
			if (id != 0) {
				slots[n] = i;
				g_menu_texts[n] = g_text_output_buf + n * MENU_ITEM_LEN;
				strcpy(g_menu_texts[n], grammar_name(g_item_names[id]));
				n++;
			}
		}
		if (n == 0) {
			sprintf(g_text_output_buf, f->txt[0x3b], h->name);
			dlg_message(g_dialog, g_text_output_buf);
		} else {
			if (h->inv[INV_SLOT_LEFT_HAND].item_id == 0)
				sprintf(g_text_buf2, f->txt[0x30], h->name);
			else
				sprintf(g_text_buf2, f->txt[0x1f], h->name,
					grammar_article(0x8002, h->inv[INV_SLOT_LEFT_HAND].item_id, 0));
			old = dlg_set16(g_dialog, 6);
			sel = dlg_choose(g_dialog, g_text_buf2, n, g_menu_texts[0], g_menu_texts[1], g_menu_texts[2],
				g_menu_texts[3], g_menu_texts[4], g_menu_texts[5], g_menu_texts[6], g_menu_texts[7],
				g_menu_texts[8], g_menu_texts[9], g_menu_texts[10], g_menu_texts[11], g_menu_texts[12],
				g_menu_texts[13], g_menu_texts[14], g_menu_texts[15]);
			dlg_set16(g_dialog, old);
			if (sel != -1) {
				h->add_bp_left(-2);
				move_item(h, INV_SLOT_LEFT_HAND, slots[sel - 1]);
			}
		}
	} else {
		sprintf(g_text_output_buf, f->txt[0x11], h->name);
		dlg_message(g_dialog, g_text_output_buf);
	}
}

/* Waffe wechseln (rechte Hand) */
void fig_menu_change_weapon(Fight far *f, struct hero far *h)
{
	signed short id;
	signed short old;
	signed short i;
	signed short sel;
	signed short wc_old;
	signed short wc_new;
	signed short offer_empty = 1;
	register signed short n;
	signed short slots[NR_MENU_ITEMS];

	if (h->inv[INV_SLOT_RIGHT_HAND].item_id == 0)
		offer_empty = 0;
	if (h->bp_left >= 2) {
		n = 0;
		for (i = INV_SLOT_KNAPSACK; i < NR_INV_SLOTS; i++) {
			id = h->inv[i].item_id;
			if ((g_items[id].weapon && can_use_item(h, id)) || (id == 0 && offer_empty)) {
				if (id == 0)
					offer_empty = 0;
				slots[n] = i;
				g_menu_texts[n] = g_text_output_buf + n * MENU_ITEM_LEN;
				sprintf(g_menu_texts[n], "%s %s %s", grammar_name(g_item_names[id]),
					h->inv[i].broken ? f->txt[0x3c] : (char far *)"",
					h->inv[i].poison_type ? f->txt[0x48] : (char far *)"");
				n++;
			}
		}
		if (n == 0) {
			sprintf(g_text_output_buf, f->txt[1], h->name);
			dlg_message(g_dialog, g_text_output_buf);
		} else {
			sprintf(g_text_buf2, f->txt[2], h->name,
				grammar_article(0x8002, h->inv[INV_SLOT_RIGHT_HAND].item_id, 0));
			old = dlg_set16(g_dialog, 6);
			sel = dlg_choose(g_dialog, g_text_buf2, n, g_menu_texts[0], g_menu_texts[1], g_menu_texts[2],
				g_menu_texts[3], g_menu_texts[4], g_menu_texts[5], g_menu_texts[6], g_menu_texts[7],
				g_menu_texts[8], g_menu_texts[9], g_menu_texts[10], g_menu_texts[11], g_menu_texts[12],
				g_menu_texts[13], g_menu_texts[14], g_menu_texts[15]);
			dlg_set16(g_dialog, old);
			if (sel != -1) {
				h->add_bp_left(-2);
				wc_old = hand_weapon_class(h);
				move_item(h, INV_SLOT_RIGHT_HAND, slots[sel - 1]);
				wc_new = hand_weapon_class(h);
				if (wc_old != wc_new)
					fig_stand_pic(f, h, &f->seq);
			}
		}
	} else {
		sprintf(g_text_output_buf, f->txt[0x11], h->name);
		dlg_message(g_dialog, g_text_output_buf);
	}
}

/* Kampfwerte anzeigen */
void fig_menu_show_values(Fight far *f, struct hero far *h)
{
	signed short wtype;
	signed short dmg_min;
	signed short dmg_max;
	signed short kk_extra;
	struct weapon_desc far *w;
	register signed short at;
	register signed short pa;

	wtype = hero_weapon_type(h);
	if (wtype == -1) {
		wtype = hero_weapon_type2(h);
		if (wtype == -1) {
			/* waffenlos */
			at = hero_at_weapon(h, 0);
			pa = hero_pa_weapon(h, 0);
			dmg_min = 1;
			dmg_max = 6;
		} else {
			at = hero_at(h);
			pa = hero_pa(h);
			w = &g_weapons_table[g_items[h->inv[INV_SLOT_RIGHT_HAND].item_id].table_index];
			calc_damage_range(w->dmg_d6, 6, w->dmg_add, &dmg_min, &dmg_max);
		}
	} else {
		at = hero_at(h);
		pa = hero_pa(h);
		w = &g_weapons_table[g_items[h->inv[INV_SLOT_RIGHT_HAND].item_id].table_index];
		calc_damage_range(w->dmg_d6, 6, w->dmg_add, &dmg_min, &dmg_max);
		kk_extra = (signed char)(h->attrib[ATTRIB_KK].cur + h->attrib[ATTRIB_KK].mod) - w->kk_bonus;
		if (kk_extra > 0) {
			dmg_min += kk_extra;
			dmg_max += kk_extra;
		}
	}
	if (at < 0)
		at = 0;
	if (pa < 0)
		pa = 0;
	sprintf(g_text_output_buf, f->txt[0], h->name, g_ttx[(h->sex ? 0x3e : 0x32) + h->type], h->level, at, pa,
		(signed char)(h->rs_bonus + h->rs_bonus2), grammar_name(g_item_names[h->inv[INV_SLOT_RIGHT_HAND].item_id]),
		dmg_min, dmg_max, h->le, h->le_max, h->ae, h->ae_max, hero_is_poisoned(h) ? f->txt[0x24] : (char far *)"",
		(unsigned char)h->renegade == 1 ? f->txt[0x3d] : (char far *)"");
	dlg_message(g_dialog, g_text_output_buf);
}

/* Gegenstand aus dem Rucksack ablegen */
void fig_menu_drop_item(Fight far *f, struct hero far *h)
{
	signed short n;
	signed short id;
	signed short old;
	signed short i;
	signed short sel;
	signed short slots[NR_MENU_ITEMS];

	if (h->bp_left >= 1) {
		n = 0;
		for (i = INV_SLOT_KNAPSACK; i < NR_INV_SLOTS; i++) {
			id = h->inv[i].item_id;
			if (id != 0) {
				slots[n] = i;
				g_menu_texts[n] = g_text_output_buf + n * MENU_ITEM_LEN;
				strcpy(g_menu_texts[n], grammar_name(g_item_names[id]));
				n++;
			}
		}
		if (n == 0) {
			sprintf(g_text_output_buf, f->txt[0x3b], h->name);
			dlg_message(g_dialog, g_text_output_buf);
		} else {
			sprintf(g_text_buf2, f->txt[0x29], h->name);
			old = dlg_set16(g_dialog, 6);
			sel = dlg_choose(g_dialog, g_text_buf2, n, g_menu_texts[0], g_menu_texts[1], g_menu_texts[2],
				g_menu_texts[3], g_menu_texts[4], g_menu_texts[5], g_menu_texts[6], g_menu_texts[7],
				g_menu_texts[8], g_menu_texts[9], g_menu_texts[10], g_menu_texts[11], g_menu_texts[12],
				g_menu_texts[13], g_menu_texts[14], g_menu_texts[15]);
			dlg_set16(g_dialog, old);
			if (sel != -1 && drop_item(h, slots[sel - 1], -1))
				h->add_bp_left(-1);
		}
	} else {
		sprintf(g_text_output_buf, f->txt[0x11], h->name);
		dlg_message(g_dialog, g_text_output_buf);
	}
}

/* letzte Aktion wiederholen; 1 = Aktion steht */
signed short fig_menu_repeat(Fight far *f, struct hero far *h)
{
	register signed short ok = 1;

	if ((h->action_id == FIG_ACTION_SPELL || h->action_id == FIG_ACTION_MELEE_ATTACK ||
	     h->action_id == FIG_ACTION_RANGE_ATTACK) && target_type(h) != TARGET_SELF) {
		{
			signed short x;
			signed short y;

			if (f->id == FIGHT_ID_SPECIAL && h->enemy_id == TARGET_ID_ENEMY && fight_find_cell(TARGET_ID_ENEMY, &x, &y)) {
				/* Sonderkampf: Sondergegner wieder auf das Zielfeld */
				fight_set_cell(f, x, y, 0xfd);
				fight_set_cell(f, h->target_x, h->target_y, TARGET_ID_ENEMY);
			}
		}
		if (((target_type(h) == TARGET_ENEMY || target_type(h) == TARGET_SPECIAL) && !fight_check_state(enemy_mr_target(h))) ||
			(target_type(h) == TARGET_HERO && hero_mr_target(h)->is_dead())) {
			/* Ziel ausser Gefecht */
			dlg_message(g_dialog, f->txt[0x1d]);
			h->action_id = FIG_ACTION_WAIT;
			h->enemy_id = -1;
			ok = 0;
		} else if (((target_type(h) == TARGET_ENEMY || target_type(h) == TARGET_SPECIAL) && enemy_mr_target(h)->is_scared()) ||
			(target_type(h) == TARGET_HERO && hero_mr_target(h)->is_scared())) {
			/* Ziel flieht */
			if (target_type(h) == TARGET_ENEMY || target_type(h) == TARGET_SPECIAL)
				sprintf(g_text_output_buf, f->txt[0x66], grammar_article(0x8000, enemy_mr_target(h)->get_mon_id(), 1));
			else
				sprintf(g_text_output_buf, f->txt[0x66], hero_mr_target(h)->get_name());
			dlg_message(g_dialog, g_text_output_buf);
			h->action_id = FIG_ACTION_WAIT;
			h->enemy_id = -1;
			ok = 0;
		} else if ((h->action_id == FIG_ACTION_SPELL || h->action_id == FIG_ACTION_RANGE_ATTACK) &&
			   !fig_target_in_reach(f, h)) {
			h->action_id = FIG_ACTION_WAIT;
			h->enemy_id = -1;
			ok = 0;
		} else if (h->action_id == FIG_ACTION_RANGE_ATTACK && !range_attack_check_ammo(f, h, 1)) {
			signed short w;

			/* keine Munition */
			w = h->inv[INV_SLOT_RIGHT_HAND].item_id;
			switch (w) {
			case 9:
			case 0x13:
			case 0x100:
			case 0x13d:
				sprintf(g_text_output_buf, f->txt[8], h->name);
				dlg_message(g_dialog, g_text_output_buf);
				break;
			case 0xc:
			case 0x101:
				sprintf(g_text_output_buf, f->txt[9], h->name);
				dlg_message(g_dialog, g_text_output_buf);
				break;
			default:
				break;
			}
			h->action_id = FIG_ACTION_WAIT;
			h->enemy_id = -1;
			ok = 0;
		} else if (h->action_id == FIG_ACTION_MELEE_ATTACK) {
			signed short tx;
			signed short ty;
			signed short hx;
			signed short hy;
			signed char k;

			/* Nahkampf: Ziel muss Nachbar sein */
			k = get_hero_index(h);
			fight_find_cell(h->enemy_id, &tx, &ty);
			fight_find_cell(k + 1, &hx, &hy);
			if (manhattan_dist(hx, hy, tx, ty) > 1) {
				dlg_message(g_dialog, f->txt[0x27]);
				h->action_id = FIG_ACTION_WAIT;
				h->enemy_id = -1;
				ok = 0;
			}
		}
		if (ok) {
			if ((h->action_id == FIG_ACTION_SPELL && h->bp_left < 5) ||
			    ((h->action_id == FIG_ACTION_RANGE_ATTACK || h->action_id == FIG_ACTION_MELEE_ATTACK) && h->bp_left < 3)) {
				h->action_id = FIG_ACTION_WAIT;
				h->enemy_id = -1;
				ok = 0;
				sprintf(g_text_output_buf, f->txt[0x11], h->name);
				dlg_message(g_dialog, g_text_output_buf);
			}
		}
	} else if (h->action_id == FIG_ACTION_WAIT || h->action_id == FIG_ACTION_GUARD) {
		sprintf(g_text_output_buf, f->txt[0x67], h->name);
		dlg_message(g_dialog, g_text_output_buf);
		h->action_id = FIG_ACTION_GUARD;
		h->set_bp_left(0);
		h->enemy_id = -1;
	} else if (h->action_id == FIG_ACTION_MOVE || h->action_id == FIG_ACTION_USE_ITEM) {
		ok = 0;
	}
	return ok;
}

/* Ziel fuer Zauber/Fernkampf erreichbar (Sichtlinie, nicht auf dem Nachbarfeld mit Fernwaffe)? */
signed short fig_target_in_reach(Fight far *f, struct hero far *h)
{
	signed short tx;
	signed short ty;
	signed short hx;
	signed short hy;
	signed char k;
	signed short range;
	signed short wclass;
	register signed short ok = 1;
	register signed short d;

	if (g_quick_fight != 0)
		game_idle();
	k = get_hero_index(h);
	fight_find_cell(h->enemy_id, &tx, &ty);
	fight_find_cell(k + 1, &hx, &hy);
	range = 99;
	if (h->action_id == FIG_ACTION_SPELL && g_spell_descriptions[h->spell_id].range == 0)
		range = 1;
	if (!fig_line_of_sight(f, hx, hy, tx, ty, range)) {
		dlg_message(g_dialog, f->txt[0x27]);
		ok = 0;
	} else {
		d = manhattan_dist(hx, hy, tx, ty);
		if (d == 1 || (d == 2 && hx != tx && hy != ty)) {
			if (h->action_id == FIG_ACTION_RANGE_ATTACK) {
				wclass = hand_weapon_class(h);
				sprintf(g_text_output_buf, f->txt[0x33], wclass == 4 ? f->txt[0x51] : f->txt[0x52]);
				dlg_message(g_dialog, g_text_output_buf);
				ok = 0;
			}
		}
	}
	return ok;
}
