/* OVL237 - Overlay O237 (Segment 0x242f in SCHWEIF.EXE): Zauberroutinen 72..85 (Verwandlung, Veraenderung), freie
 * Funktionen spell_*(CAST_PTR c) (Klasse Cast, O233). spell_paralue: Ziel versteinert (petrified), nicht sich selbst.
 * spell_salander: Kosten MR * 3 (mind. 25), Gegner mushroom, Sonderfall g_fight_end == 5. spell_in_see_und_fluss: Kosten
 * nach Heldenzahl (nr_heroes * 7), Dauer in Prozent (dlg_input), Probe, group_add_effect(r * 90, EFFECT_SEE_UND_FLUSS).
 * spell_visibili: Eingabe bis 99, EFFECT_INVISIBLE r * 450. spell_abvenenum: Gegenstaende 0x136 -> 0x135.
 * spell_aeolitus/spell_weiches_erstarre/spell_claudibus: Meldungen. spell_brenne_toter_stoff: Fackel (ITEM_TORCH_OFF)
 * oder Laterne (ITEM_LANTERN_OFF, Auswahl bei beiden), Umwandlung in ITEM_TORCH_ON (Brenndauer 10) bzw.
 * ITEM_LANTERN_ON (100, ITEM_OIL -> ITEM_FLASK_BRONZE). spell_dunkelheit/spell_flim_flam: add_effect((Stufe + 3) * 5400).
 * spell_hartes_schmelze: nur in Dungeon 1, Feld x 0..5/y 12..16 bestimmte Stellen, Probe, dng01_melt_ice, g_dng01_ice_state.
 * spell_silentium: einmalig (INGAME_TIMER_SILENTIUM) Schleichen +10 fuer die Gruppe. spell_sturmgebruell: Zeitzaehler.
 *
 * BCC -mlarge -3 -Ot -Og -Oi -c -Y OVL237.CPP   -> 3722/3722 Bytes GLEICH (18.10.2026), cobjfix laeuft
 *   (Namensfassung mit schweif.h: 19.10.2026 GLEICH, LINKGLEICH)
 * - -Oi: strcpy(g_text_output_buf, c->txt[n]) als Intrinsic (repne scasb / rep movsw, daher si/di gesichert); -Og: Epilog-
 *   kopien nach return und an den Enden der if/else-Zweige.
 * - inline hero::set_inv_item_id(i, v)/set_inv_light_timer(i, v): Parameterkopie des Index (abvenenum in DI,
 *   brenne_toter_stoff in [bp-6]/[bp-8] bzw. [bp-0ah]/[bp-0ch]); konstante Werte werden nicht kopiert.
 * - "half = (cost7 = heroes * 7) / 2;" eingebettet (sonst mov ax,[bp-8] nachgeladen).
 * - "c->target != c->caster" -> mov eax / cmp eax,mem; "pct * 90L" -> movsx eax,si / imul eax,eax,5ah.
 * - add_effect/group_add_effect mit Wort-Parametern ueber add_effect_w/group_add_effect_w.
 * Originalfehler: spell_brenne_toter_stoff prueft am Ende sel, das nur gesetzt wird, wenn beide Gegenstaende vorhanden
 *   sind (sonst uninitialisiert -> zufaellig Meldung txt[121] oder Abbruch).
 */
#include <stdio.h>
#include <string.h>
#include "schweif.h"

/* Aufrufe mit Wort-Parametern (das Original uebergibt diese Werte als short) */
#define add_effect_w ((void (*)(struct hero far *, signed short, signed long, signed short, signed short, signed short))add_effect)
#define group_add_effect_w ((void (*)(struct group far *, signed long, signed short, signed short, signed short))group_add_effect)

void spell_paralue(CAST_PTR c)
{
	if (c->target_kind == TARGET_ENEMY || c->target_kind == TARGET_SPECIAL) {
		c->enemy->petrified = 1;
		sprintf(g_text_output_buf, c->txt[103], grammar_article(0x8000, c->enemy->mon_id, 1));
		return;
	}
	if (c->target != c->caster) {
		c->target->petrified = 1;
		sprintf(g_text_output_buf, c->txt[103], c->target->name);
		return;
	}
	strcpy(g_text_output_buf, c->txt[112]);
	c->result = SPELL_RES_ABORT;
}

void spell_salander(CAST_PTR c)
{
	register signed short cost;

	cost = c->enemy->mr * 3;
	if (cost < 25)
		cost = 25;
	if (c->caster->ae >= cost) {
		c->enemy->mushroom = 1;
		sprintf(g_text_output_buf, c->txt[104], grammar_article(0x8000, c->enemy->mon_id, 1));
		c->result = cost;
		c->result += g_spell_cost_mod;
		if (g_fight_end == 5)
			g_fight_end = 0;
		return;
	}
	sprintf(g_text_output_buf, c->txt[140], c->caster->name);
	c->result = SPELL_RES_ABORT;
}

void spell_in_see_und_fluss(CAST_PTR c)
{
	signed short heroes;
	signed short digits = 1;
	signed short half;
	signed short cost7;
	register signed short pct;
	register signed short max;

	heroes = g_group->nr_heroes;
	half = (cost7 = heroes * 7) / 2;
	if (c->caster->ae < half) {
		sprintf(g_text_output_buf, c->txt[140], c->caster->name);
		c->result = SPELL_RES_ABORT;
		return;
	}
	max = c->caster->ae * 10 / cost7;
	sprintf(g_text_output_buf, c->txt[221], max);
	if (max >= 100)
		digits = 3;
	else if (max >= 10)
		digits = 2;
	pct = dlg_input(g_dialog, digits, g_text_output_buf, 0, max, 0);
	*g_text_output_buf = 0;
	if (pct > 0) {
		if (test_spell(c->caster, SP_IN_SEE_UND_FLUSS, c->mod) > 0) {
			if (pct < 5)
				pct = 5;
			group_add_effect_w(g_group, pct * 90L, 0, 1, EFFECT_SEE_UND_FLUSS);
			c->result = pct / 5 * heroes * 7 / 2;
			sprintf(g_text_output_buf, c->txt[222], pct);
		} else {
			if (pct < 5)
				pct = 5;
			c->fail_cost = pct / 5 * heroes * 7 / 4;
			c->result = SPELL_RES_FAIL;
		}
	} else
		c->result = SPELL_RES_ABORT;
}

void spell_visibili(CAST_PTR c)
{
	signed short pos;
	register signed short n;

	if (c->caster->invisible) {
		sprintf(g_text_output_buf, c->txt[223], c->caster->name);
		c->result = SPELL_RES_ABORT;
		return;
	}
	sprintf(g_text_output_buf, c->txt[105], c->caster->name);
	n = dlg_input(g_dialog, 2, g_text_output_buf, 0, 99, 0);
	*g_text_output_buf = 0;
	if (n <= 0) {
		c->result = SPELL_RES_ABORT;
		return;
	}
	if (n * 5 <= c->caster->ae) {
		if (test_spell(c->caster, SP_VISIBILI, c->mod) > 0) {
			c->result = n * 5;
			c->result += g_spell_cost_mod;
			pos = get_free_effect(c->caster);
			add_effect_w(c->caster, pos, n * 450L, 0, 1, EFFECT_INVISIBLE);
			sprintf(g_text_output_buf, c->txt[106], c->caster->name, grammar_pronoun(c->caster->sex, 0, 0));
		} else {
			c->fail_cost = n * 5 / 2;
			c->result = SPELL_RES_FAIL;
		}
	} else {
		sprintf(g_text_output_buf, c->txt[140], c->caster->name);
		c->result = SPELL_RES_ABORT;
	}
}

void spell_abvenenum(CAST_PTR c)
{
	register signed short pos;

	/* Gegenstand 0x136 wird zu 0x135 (vermutet: verdorbene -> gute Nahrung) */
	pos = get_item_pos(c->caster, 0x136);
	if (pos != -1) {
		do {
			c->caster->set_inv_item_id(pos, 0x135);
			sprintf(g_text_output_buf, c->txt[200], c->caster->name);
			pos = get_item_pos(c->caster, 0x136);
		} while (pos != -1);
		return;
	}
	c->result = 2;
	sprintf(g_text_output_buf, c->txt[224], c->caster->name);
}

void spell_aeolitus(CAST_PTR c)
{
	if (g_spell_event == 0)
		sprintf(g_text_output_buf, c->txt[201], c->caster->name);
}

void spell_brenne_toter_stoff(CAST_PTR c)
{
	signed short oil;
	signed short sel;
	register signed short torch = -1;
	register signed short lantern = -1;

#if FIX_0048
	/* Fix #48: sel vorbelegen: ohne Fackel und Lampe immer die Meldung */
	sel = 0;
	c->result = SPELL_RES_NONE;
	torch = get_item_pos(c->caster, ITEM_TORCH_OFF);
#else
	c->result = SPELL_RES_NONE;
	torch = get_item_pos(c->caster, ITEM_TORCH_OFF);
#endif
	lantern = get_item_pos(c->caster, ITEM_LANTERN_OFF);
	if (torch != -1 && lantern != -1) {
		sprintf(g_text_output_buf, c->txt[107], c->caster->name);
		sel = dlg_choose(g_dialog, g_text_output_buf, 2, grammar_article(0x4000, ITEM_TORCH_OFF, 0),
			grammar_article(0x4000, ITEM_LANTERN_OFF, 0));
		if (sel == -1)
			torch = lantern = -1;
		else if (sel == 1)
			lantern = -1;
		else
			torch = -1;
	}
	if (torch != -1) {
		if (test_spell(c->caster, SP_BRENNE_TOTER_STOFF, c->mod) > 0) {
			c->caster->set_inv_item_id(torch, ITEM_TORCH_ON);
			c->caster->set_inv_light_timer(torch, 10);
			c->result = random_schweif(20);
			c->result += g_spell_cost_mod;
			sprintf(g_text_output_buf, c->txt[108], c->caster->name);
		} else {
			c->fail_cost = random_schweif(20) / 2;
			c->result = SPELL_RES_FAIL;
		}
		return;
	}
	if (lantern != -1) {
		oil = get_item_pos(c->caster, ITEM_OIL);
		if (oil != -1) {
			if (test_spell(c->caster, SP_BRENNE_TOTER_STOFF, c->mod) > 0) {
				c->caster->set_inv_item_id(lantern, ITEM_LANTERN_ON);
				c->caster->set_inv_light_timer(lantern, 100);
				drop_item(c->caster, oil, 1);
				give_item_at(c->caster, ITEM_FLASK_BRONZE, 1, oil);
				c->result = random_schweif(20);
				c->result += g_spell_cost_mod;
				sprintf(g_text_output_buf, c->txt[119], c->caster->name);
			} else {
				c->fail_cost = random_schweif(20) / 2;
				c->result = SPELL_RES_FAIL;
			}
		} else
			sprintf(g_text_output_buf, c->txt[120], c->caster->name);
		return;
	}
	if (sel != -1) {
		sprintf(g_text_output_buf, c->txt[121], c->caster->name);
		return;
	}
	*g_text_output_buf = 0;
	c->result = SPELL_RES_ABORT;
}

void spell_claudibus(CAST_PTR c)
{
	dlg_message(g_dialog, c->txt[270]);
	*g_text_output_buf = 0;
	c->result = SPELL_RES_ABORT;
}

void spell_dunkelheit(CAST_PTR c)
{
	signed short pos;

	pos = get_free_effect(c->caster);
	add_effect_w(c->caster, pos, (c->caster->level + 3) * 5400L, 0, 1, EFFECT_DUNKELHEIT);
	strcpy(g_text_output_buf, c->txt[109]);
}

void spell_weiches_erstarre(CAST_PTR c)
{
	if (g_spell_event == 0) {
		strcpy(g_text_output_buf, c->txt[203]);
		c->result = SPELL_RES_NONE;
	}
}

void spell_flim_flam(CAST_PTR c)
{
	signed short pos;

	pos = get_free_effect(c->caster);
	add_effect_w(c->caster, pos, (c->caster->level + 3) * 5400L, 0, 1, EFFECT_FLIM_FLAM);
	strcpy(g_text_output_buf, c->txt[110]);
}

void spell_hartes_schmelze(CAST_PTR c)
{
	signed short x;
	signed short y;

	if (g_spell_event == 0) {
		/* wirkt nur in Dungeon 1 an bestimmten Wandstuecken (x 0..5, y 12..16) */
		if (g_group->dungeon == 1) {
			x = g_group->x;
			y = g_group->y;
			if (x >= 0 && x <= 5 && y >= 12 && y <= 16) {
				if ((y == 13 && (x == 3 || x == 4)) || (x == 2 && (y == 14 || y == 15))) {
					if (test_spell(c->caster, SP_HARTES_SCHMELZE, c->mod) > 0) {
						dng01_melt_ice(g_area_events);
						c->result = random_schweif(10) + 10;
						c->result += g_spell_cost_mod;
						sprintf(g_text_output_buf, c->txt[269], c->caster->name,
							grammar_pronoun(c->caster->sex, 0, 0));
						g_dng01_ice_state = 1;
					} else {
						c->fail_cost = (random_schweif(10) + 10) / 2;
						c->result = SPELL_RES_FAIL;
					}
				} else
					dlg_message(g_dialog, c->txt[272]);
			} else {
				strcpy(g_text_output_buf, c->txt[204]);
				c->result = SPELL_RES_NONE;
			}
		} else {
			strcpy(g_text_output_buf, c->txt[204]);
			c->result = SPELL_RES_NONE;
		}
	}
}

void spell_silentium(CAST_PTR c)
{
	if (g_ingame_timers[INGAME_TIMER_SILENTIUM] == 0) {
		group_add_effect_w(g_group, 0x1c2L, TA_SCHLEICHEN, 10, EFFECT_SKILL);
		g_ingame_timers[INGAME_TIMER_SILENTIUM] = 0x1c2;
		c->result = 5;
		c->result += g_spell_cost_mod;
		strcpy(g_text_output_buf, c->txt[111]);
		return;
	}
	strcpy(g_text_output_buf, c->txt[226]);
	c->result = SPELL_RES_NONE;
}

void spell_sturmgebruell(CAST_PTR c)
{
	g_ingame_timers[INGAME_TIMER_STURMGEBRUELL] = 0x1c2;
	strcpy(g_text_output_buf, c->txt[205]);
}
