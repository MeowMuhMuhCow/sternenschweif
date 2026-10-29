/* OVL235 - Overlay O235 (Segment 0x241c in SCHWEIF.EXE): Zauberroutinen 27..48 (Bewegung, Heilung, Hellsicht,
 * Illusion), freie Funktionen spell_*(CAST_PTR c) auf dem Zauberobjekt aus O233 (Cast). Aufruf ueber g_spell_handlers
 * (Fernaufruf nur O013 -> spell_analues).
 * spell_axxeleratus: AT/PA +2, EFFECT_AXXELERATUS (90 Ticks), bei einer Aktion bp_left = calc_bp, zwei Aktionen.
 * spell_foramen/spell_motoricus/spell_spurlos/spell_sensibar: Texte bzw. nur mit g_spell_event.
 * spell_transversalis: Sprung ueber die Automap des Dungeons (g_dng_view_areas[dungeon - 1], g_amap_teleport/_teleported;
 *   Texte vorher freigegeben). spell_ueber_eis: Strecke eingeben, EFFECT_UEBER_EIS. spell_balsam/spell_hexenspeichel:
 *   LE heilen (add_hero_le). spell_klarum_purum: Gifte heilen (Kosten g_poison_prices). spell_ruhe_koerper,
 *   spell_tiere_besprechen, spell_adleraug (TA_SINNESSCHAERFE +7). spell_analues: Gegenstand pruefen (g_analues_items,
 *   Probe bzw. 99 bei g_spell_by_item, set_inv_magic_known). spell_eigenschaften: Gegnerwerte. spell_exposami:
 *   fig_hidden_enemies_text. spell_odem_arcanum: Magie erkennen. spell_penetrizzel: 5 x 5 Felder reveal_map
 *   (g_reveal_by_spell). spell_chamaelioni, spell_duplicatus, spell_harmlose_gestalt (EFFECT_HARMLOS),
 *   spell_hexenknoten (FIG_ACTION_WAIT).
 *
 * BCC -mlarge -3 -Ot -Og -Oi -c -Y OVL235.CPP   -> 6028/6028 Bytes GLEICH (18.10.2026), cobjfix laeuft
 *   (g_dng_view_areas - 1 -> _g_0b84+0x39). Ohne -Og 4571 Abweichungen (Epilogkopien); -Oi fuer die strcpy-Intrinsics.
 *   (Namensfassung mit schweif.h: 19.10.2026 GLEICH, LINKGLEICH)
 * - "if (!!g_items[id].magic)" (mov al / cbw / or ax,ax), "!= 0" -> cmp byte.
 * - inline hero::set_bp_left(calc_bp(..)) (setzt und begrenzt auf 12; Byte-Parameterkopie [bp-1]) und
 *   hero::set_inv_magic_known(slot) (Parameterkopien [bp-0ch]/[bp-0eh] bzw. [bp-6], getrennt vom -Og-Zwischenwert
 *   slot * 0x1b).
 * - Bit dead als unsigned-short-Bitfeld: "(signed char)c->target->dead" -> and ax,1 / or al,al.
 * - get_free_effect-Ergebnis einmal benutzt -> Stackplatz [bp-2] (adleraug/harmlose_gestalt), dreimal -> SI (axxeleratus).
 * - add_effect mit Wort-Parametern ueber add_effect_w; spell_exposami/spell_penetrizzel haben im Original keinen
 *   Parameter (unbenannter CAST_PTR, gleicher Code).
 * - spell_analues: "break; } else { ..; break; }" erklaert den toten jmp short hinter dem ersten break.
 * Originalfehler: spell_hexenspeichel - "else if (c->result > c->caster->ae)" prueft das eben auf -4 gesetzte
 *   Ergebnis (nie wahr); die AE-Obergrenze greift dort nie (spell_balsam prueft nach der Rechnung richtig).
 */
#include <stdio.h>
#include <string.h>
#include "schweif.h"

/* Phase B: eigene Daten des Moduls (ds:7486..75b4, dsana) - initialisierte Daten in Adressreihenfolge */
signed short g_amap_teleport = 0;
signed short g_amap_teleported = 0;
struct analues_item g_analues_items[49] = {
	{ 162, 2, 54 },
	{ 165, 3, 56 },
	{ 171, 4, 57 },
	{ 245, 4, 57 },
	{ 172, 3, 58 },
	{ 173, 6, 59 },
	{ 174, 0, 60 },
	{ 175, 6, 61 },
	{ 178, 0, 62 },
	{ 215, 3, 63 },
	{ 216, 3, 64 },
	{ 217, 4, 65 },
	{ 158, 4, 66 },
	{ 159, 4, 67 },
	{ 160, 4, 68 },
	{ 161, 6, 69 },
	{ 163, 2, 70 },
	{ 188, 0, 71 },
	{ 198, 15, 72 },
	{ 183, 7, 73 },
	{ 199, 5, 74 },
	{ 197, 5, 75 },
	{ 223, 15, 76 },
	{ 220, 5, 77 },
	{ 218, 6, 78 },
	{ 70, 2, 79 },
	{ 264, 2, 31 },
	{ 265, 4, 32 },
	{ 280, 0, 33 },
	{ 286, 7, 146 },
	{ 321, 8, 147 },
	{ 323, 3, 148 },
	{ 324, 3, 149 },
	{ 182, 5, 152 },
	{ 185, 0, 153 },
	{ 184, 0, 154 },
	{ 322, 0, 155 },
	{ 256, 2, 156 },
	{ 255, 2, 157 },
	{ 196, 10, 158 },
	{ 311, 10, 159 },
	{ 313, 10, 159 },
	{ 312, 10, 159 },
	{ 251, 5, 175 },
	{ 298, 5, 176 },
	{ 327, 5, 227 },
	{ 336, 5, 228 },
	{ 340, 5, 266 },
	{ -1, 0, 0 }
};
signed long g_odem_items = 12058809;

/* add_effect mit Wort-Parametern (das Original uebergibt Index, Wert und Art als short) */
#define add_effect_w ((void (*)(struct hero far *, signed short, signed long, signed short, signed short, signed short))add_effect)

void spell_axxeleratus(CAST_PTR c)
{
	signed short pos;

	if (!c->target->axxeleratus) {
		pos = get_free_effect(c->target);
		add_effect_w(c->target, pos, 0x5aL, c->target->weapon_type, 2, EFFECT_AT);
		pos = get_free_effect(c->target);
		add_effect_w(c->target, pos, 0x5aL, c->target->weapon_type, 2, EFFECT_PA);
		pos = get_free_effect(c->target);
		add_effect_w(c->target, pos, 0x5aL, 0, 1, EFFECT_AXXELERATUS);
		if (c->target->actions == 1) {
			c->target->set_bp_left(calc_bp(c->target));
			c->target->actions = 2;
		}
		sprintf(g_text_output_buf, c->txt[17], c->target->name);
	} else {
		sprintf(g_text_output_buf, c->txt[211], c->target->name);
		c->result = SPELL_RES_ABORT;
	}
}

void spell_foramen(CAST_PTR c)
{
	strcpy(g_text_output_buf, c->txt[268]);
	c->result = SPELL_RES_NONE;
}

void spell_motoricus(CAST_PTR c)
{
	if (g_spell_event == 0) {
		strcpy(g_text_output_buf, c->txt[190]);
		c->result = SPELL_RES_NONE;
	} else {
		c->result = 5;
	}
}

void spell_spurlos(CAST_PTR c)
{
	if (g_spell_event == 0)
		sprintf(g_text_output_buf, c->txt[191], c->caster->name);
}

void spell_transversalis(CAST_PTR c)
{
	struct view_area far *area;

	if (!g_group->dungeon) {
		strcpy(g_text_output_buf, c->txt[18]);
		c->result = SPELL_RES_ABORT;
	} else if (g_dng05_poltergeist == 1 && g_group->poltergeist_bound != 0) {
		strcpy(g_text_output_buf, c->txt[292]);
		c->result = SPELL_RES_ABORT;
	} else {
		c->result = 20;
		c->result += g_spell_cost_mod;
		if (c->result > c->caster->ae) {
			c->result = SPELL_RES_FAIL;
		} else if (g_group->dungeon == 10 || g_group->dungeon == 16) {
			sprintf(g_text_output_buf, c->txt[271], c->caster->name);
		} else {
			/* Texte freigeben, Sprung ueber die Automap des Dungeons */
			c->txt_buf = mem_free(c->txt_buf);
			c->txt = (char far * far *)mem_free(c->txt);
			area = &g_dng_view_areas[g_group->dungeon - 1];
			g_amap_teleport = 1;
			g_amap_teleported = 0;
			automap_view(area->map, area->notes, area->tiles);
			g_amap_teleport = 0;
			if (g_amap_teleported == 0)
				c->result = SPELL_RES_ABORT;
		}
	}
}

void spell_ueber_eis(CAST_PTR c)
{
	signed short pos;
	signed short n;
	signed short max;

	if (!c->target->ueber_eis) {
		max = c->caster->ae / 3 * 5;
		sprintf(g_text_output_buf, c->txt[192], c->caster->name, max);
		n = dlg_input(g_dialog, max < 10 ? 1 : max < 100 ? 2 : 3, g_text_output_buf, 0, max, 0);
		*g_text_output_buf = 0;
		if (n > 0) {
			if (test_spell(c->caster, SP_UEBER_EIS, c->mod) > 0) {
				c->result = n / 5 * 3 + 5;
				c->result += g_spell_cost_mod;
				pos = get_free_effect(c->target);
				add_effect_w(c->target, pos, (signed long)n * 90, 0, 1, EFFECT_UEBER_EIS);
				sprintf(g_text_output_buf, c->txt[213], c->target->name, n);
			} else {
				c->fail_cost = (n / 2 * 3 + 5) / 2;
				c->result = SPELL_RES_FAIL;
			}
		}
	} else {
		sprintf(g_text_output_buf, c->txt[212], c->target->name);
		c->result = SPELL_RES_ABORT;
	}
}

void spell_balsam(CAST_PTR c)
{
	signed short le;

	if ((signed char)c->target->dead) {
		c->result = SPELL_RES_ABORT;
		strcpy(g_text_output_buf, c->txt[179]);
		le = -99;
	} else if (c->caster->fb5 != 0 || (c->caster->npc_id != 0 && g_in_fight != 0)) {
		c->result = SPELL_RES_NONE;
		le = (c->target->le_max - c->target->le) / 2;
	} else {
		c->result = SPELL_RES_ABORT;
		sprintf(g_text_output_buf, c->txt[19], c->caster->name, c->target->name);
		le = dlg_input(g_dialog, 2, g_text_output_buf, 1, c->caster->ae, 0);
		*g_text_output_buf = 0;
	}
	if (le > 0) {
		if (le < 7) {
			c->result = 7;
		} else if (c->target->le_max - c->target->le < le) {
			c->result = c->target->le_max - c->target->le;
			le = c->result;
		} else {
			c->result = le;
		}
		c->result += g_spell_cost_mod;
		if (c->target->le_max == c->target->le) {
			c->result = SPELL_RES_ABORT;
			sprintf(g_text_output_buf, c->txt[180], c->target->name);
		} else {
			if (c->result > c->caster->ae) {
				c->result = c->caster->ae;
				le = c->result;
			}
			if (test_spell(c->caster, SP_BALSAM, c->mod) > 0) {
				add_hero_le(c->target, le);
				sprintf(g_text_output_buf, c->txt[181], c->target->name);
			} else {
				c->fail_cost = c->result / 2;
				c->result = SPELL_RES_FAIL;
			}
		}
	} else if (le != -99 && c->target->le_max == c->target->le) {
		c->result = SPELL_RES_ABORT;
		sprintf(g_text_output_buf, c->txt[180], c->target->name);
	}
}

void spell_hexenspeichel(CAST_PTR c)
{
	signed short ae;

	c->result = SPELL_RES_ABORT;
	if (c->target == c->caster) {
		sprintf(g_text_output_buf, c->txt[20], c->caster->name);
	} else {
		sprintf(g_text_output_buf, c->txt[19], c->caster->name, c->target->name);
		ae = dlg_input(g_dialog, 2, g_text_output_buf, 0, c->caster->ae, 0);
		*g_text_output_buf = 0;
		if (ae > 0) {
#if FIX_0173
			/* Fix #173: AE-Obergrenze nach der Rechnung pruefen (wie spell_balsam) */
			if (c->target->le_max - c->target->le < ae * 2) {
				c->result = (c->target->le_max - c->target->le) * 2;
			} else {
				c->result = ae * 2;
			}
			if (c->result > c->caster->ae)
				c->result = c->caster->ae;
#else
			if (c->target->le_max - c->target->le < ae * 2) {
				c->result = (c->target->le_max - c->target->le) * 2;
			} else if (c->result > c->caster->ae) {
				c->result = c->caster->ae;
			} else {
				c->result = ae * 2;
			}
#endif
			if (c->target->le_max == c->target->le) {
				c->result = SPELL_RES_ABORT;
				sprintf(g_text_output_buf, c->txt[180], c->target->name);
			} else if (test_spell(c->caster, SP_HEXENSPEICHEL, c->mod) > 0) {
				add_hero_le(c->target, c->result / 2);
				sprintf(g_text_output_buf, c->txt[181], c->target->name);
			} else {
				c->fail_cost = c->result / 2;
				c->result = SPELL_RES_FAIL;
			}
		}
	}
}

void spell_klarum_purum(CAST_PTR c)
{
	signed short i;
	signed short found;

	for (found = 0, i = 0; i <= 9; i++) {
		if (c->target->poison[i].state == ILL_STATE_SICK) {
			found = 1;
			c->result += g_poison_prices[i];
		}
	}
	if (!found) {
		sprintf(g_text_output_buf, c->txt[21], c->target->name);
		c->result = SPELL_RES_ABORT;
	} else {
		c->result += g_spell_cost_mod;
		if (c->result > c->caster->ae) {
			c->fail_cost = c->result / 2;
			c->result = SPELL_RES_FAIL;
		} else {
			do {
				hero_cure_poison(c->target);
			} while (hero_is_poisoned(c->target));
			sprintf(g_text_output_buf, c->txt[22], c->target->name);
		}
	}
}

void spell_ruhe_koerper(CAST_PTR c)
{
	c->target->ruhe_koerper = 10;
	sprintf(g_text_output_buf, c->txt[51], c->target->name);
}

void spell_tiere_besprechen(CAST_PTR c)
{
	signed short ae;

	if (g_spell_event == 0) {
		strcpy(g_text_output_buf, c->txt[235]);
		c->result = SPELL_RES_NONE;
	} else {
		c->result = SPELL_RES_ABORT;
		sprintf(g_text_output_buf, c->txt[23], c->caster->name);
		ae = dlg_input(g_dialog, 2, g_text_output_buf, 0, c->caster->ae, 0);
		*g_text_output_buf = 0;
		if (ae > 0) {
			if (test_spell(c->caster, SP_TIERE_BESPRECHEN, c->mod) > 0) {
				c->result = ae;
				sprintf(g_text_output_buf, c->txt[236], c->caster->name);
			} else {
				c->fail_cost = ae / 2;
				c->result = SPELL_RES_FAIL;
			}
		}
	}
}

void spell_adleraug(CAST_PTR c)
{
	signed short pos;

	pos = get_free_effect(c->caster);
	add_effect_w(c->caster, pos, 0x21cL, TA_SINNESSCHAERFE, 7, EFFECT_SKILL);
	sprintf(g_text_output_buf, c->txt[24], c->caster->name);
}

void spell_analues(CAST_PTR c)
{
	signed short id;
	signed short slot;
	signed short i;
	signed short r;
	signed short found;

	found = 0;
	slot = select_item(c->caster, g_ttx[240], 1);
	if (slot != -1) {
		id = c->caster->inv[slot].item_id;
		if (id != 0) {
			if (g_spell_by_item == 0)
				r = test_spell(c->caster, SP_ANALUES, c->mod);
			else
				r = 99;
			if (r < 0) {
				strcpy(g_text_buf2, c->txt[55]);
				c->result = SPELL_RES_FAIL;
			} else {
				strcpy(g_text_buf2, c->txt[52]);
				if (!!g_items[id].magic)
					strcpy(g_text_buf2, c->txt[150]);
				for (i = 0; g_analues_items[i].item_id != -1; i++) {
					if (g_analues_items[i].item_id == id) {
						found = 1;
						if (g_analues_items[i].min <= r) {
							strcpy(g_text_buf2, c->txt[g_analues_items[i].txt]);
							if (id == 0x150 && c->caster->spells[SP_VERWANDLUNG_BEENDEN] > -5)
								strcpy(g_text_buf2, c->txt[229]);
							c->caster->set_inv_magic_known(slot);
							break;
						} else {
							strcpy(g_text_buf2, c->txt[55]);
							break;
						}
					}
				}
				sprintf(g_text_output_buf, c->txt[53], c->caster->name, g_text_buf2);
				if (!found && r != 0)
					c->caster->set_inv_magic_known(slot);
			}
		} else {
			c->result = SPELL_RES_ABORT;
			*g_text_output_buf = 0;
		}
	} else {
		c->result = SPELL_RES_ABORT;
		*g_text_output_buf = 0;
	}
}

void spell_eigenschaften(CAST_PTR c)
{
	signed short lo;
	signed short hi;

	enemy_damage_range(c->enemy, 0, &lo, &hi);
	sprintf(g_text_output_buf, c->txt[25], enemy_name(c->enemy), c->enemy->level, enemy_attack_value(c->enemy),
		enemy_parade_value(c->enemy), c->enemy->rs, c->enemy->mr, c->enemy->attacks,
		c->enemy->attacks > 1 ? c->txt[26] : c->txt[27], lo, hi,
		c->enemy->le, c->enemy->le_max, c->enemy->ae, c->enemy->ae_max);
}

void spell_exposami(CAST_PTR)
{
	fig_hidden_enemies_text((FIGHT_PTR)g_fight_obj);
}

void spell_odem_arcanum(CAST_PTR c)
{
	signed long list = g_odem_items;
	signed short id;
	signed short slot;

	slot = select_item(c->caster, g_ttx[240], 1);
	if (slot != -1) {
		id = c->caster->inv[slot].item_id;
		if (id != 0) {
			if (c->caster->inv[slot].magic) {
				if (is_in_word_array(id, &list) != -1)
					sprintf(g_text_output_buf, c->txt[151]);
				else
					sprintf(g_text_output_buf, c->txt[81], grammar_article(0x8000, id, 0));
				c->caster->set_inv_magic_known(slot);
			} else if ((c->caster->inv[slot].quantity > 1 && g_items[id].stackable) ||
				is_in_word_array(id, g_items_plural) != -1) {
				sprintf(g_text_output_buf, c->txt[174], grammar_article(0x8004, id, 0));
			} else {
				sprintf(g_text_output_buf, c->txt[82], grammar_article(0x8000, id, 0));
			}
		} else {
			c->result = SPELL_RES_ABORT;
			*g_text_output_buf = 0;
		}
	} else {
		c->result = SPELL_RES_ABORT;
		*g_text_output_buf = 0;
	}
}

void spell_penetrizzel(CAST_PTR)
{
	signed short x;
	signed short y;

	g_reveal_by_spell = 1;
	for (y = -2; y <= 2; y++)
		for (x = -2; x <= 2; x++)
			reveal_map(g_map, g_group->x + x, g_group->y + y);
	g_reveal_by_spell = 0;
}

void spell_sensibar(CAST_PTR c)
{
	struct hero far *npc;

	npc = group_get_npc(g_group);
	if (npc != 0) {
		strcpy(g_text_output_buf, c->txt[npc->npc_id + 272]);
	} else if (g_spell_event == 0) {
		strcpy(g_text_output_buf, c->txt[193]);
		c->result = SPELL_RES_NONE;
	}
}

void spell_chamaelioni(CAST_PTR c)
{
	c->caster->chamaelioni = 1;
	sprintf(g_text_output_buf, c->txt[83], c->caster->name, grammar_pronoun(c->caster->sex, 0, 0));
}

void spell_duplicatus(CAST_PTR c)
{
	if (c->target->duplicatus == 2) {
		sprintf(g_text_output_buf, c->txt[214], c->target->name);
		c->result = SPELL_RES_ABORT;
	} else {
		if (g_quick_fight != 0) {
			if (c->target->duplicatus != 0)
				c->target->duplicatus = 2;
			else
				c->target->duplicatus = 1;
		}
		sprintf(g_text_output_buf, all_heroes_down(g_group) ? c->txt[237] : c->txt[84], c->target->name,
			grammar_pronoun(c->target->sex, 0, 0));
	}
}

void spell_harmlose_gestalt(CAST_PTR c)
{
	signed short pos;

	if (c->caster->harmless) {
		sprintf(g_text_output_buf, c->txt[215], c->caster->name);
		c->result = SPELL_RES_ABORT;
	} else {
		pos = get_free_effect(c->caster);
		add_effect_w(c->caster, pos, (signed long)(signed short)c->caster->level * 450, 0, 1, EFFECT_HARMLOS);
		sprintf(g_text_output_buf, c->txt[194], c->caster->name, c->caster->level * 5);
	}
}

void spell_hexenknoten(CAST_PTR c)
{
	c->caster->action_id = FIG_ACTION_WAIT;
}
