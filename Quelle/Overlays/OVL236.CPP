/* OVL236 - Overlay O236 (Segment 0x2425 in SCHWEIF.EXE): Zauberroutinen 49..71 (Kampf, Verstaendigung, Verwandlung),
 * freie Funktionen spell_*(CAST_PTR c) (Klasse Cast, O233).
 * spell_blitz (blind = 3), spell_ecliptifactus (Stufe bis (AE - 5) / 2, Kosten 2r + 5), spell_eisenrost (Waffe des
 * Ziels zerbricht: inv_slot::is_breakable, set_item_broken; Gegner broken), spell_fulminictus (3W6 + Stufe,
 * cast_damage), spell_ignifaxius (Stufe waehlen, Stufe W6 bzw. Stufe + 20 bei fb5 == 99, doppelt gegen
 * enemy_is_gfx_1b, halb bei fireban; Ruestung Platz 10 verliert RS (Grenze g_armors_table), AT/PA -Stufe/2),
 * spell_plumbumbarum (AT -3), spell_radau (mit ITEM_WITCHES_BROOM wie spell_kraehenruf), Zeitwirkungen ueber add_effect
 * (spell_saft_kraft, spell_scharfes_auge, spell_adler_wolf, spell_arcano_psychostabilis, spell_feuerbann),
 * spell_nekropathia (get_dead_hero, Antwort aus g_nekropathia_txt), spell_armatrutz (Stufe bis Wurzel AE, max. 9,
 * Kosten Stufe^2), spell_inc_attrib(c, attrib): Eigenschaft + 2 (EFFECT_ATTRIB, 0x2a30 Ticks), nicht wenn schon
 * erhoeht; spell_ch/ff/ge/in/kk/kl/mu_steigern = spell_inc_attrib. spell_hexenblick/spell_mutabili: Texte.
 *
 * BCC -mlarge -3 -Ot -Og -Oi -c -Y OVL236.CPP   -> 5678/5678 Bytes GLEICH (18.10.2026), cobjfix laeuft
 *   (ohne -Og 4261, ohne -Oi 2926 Abweichungen; g_nekropathia_txt -> _g_75b0+4)
 *   (Namensfassung mit schweif.h: 19.10.2026 GLEICH, LINKGLEICH)
 * - Inline-Elementfunktionen: hero::get_weapon_type() { return weapon_type; } liefert das cbw vor dem push an einen
 *   signed-char-Parameter von add_effect (direkt h->weapon_type ohne cbw); enemy::add_rs/at/pa_min0(signed char v)
 *   (Parameterkopie [bp-0dh..-0fh] bzw. add byte imm bei Konstante, toter jmp short); inv_slot::is_breakable()
 *   { return (unsigned char)bf == 0x9d ? 0 : 1; } -> xor ax,ax / mov ax,1 / or ax,ax; hero::set_ecliptifactus (Kopie [bp-3]).
 * - add_effect hier mit signed-char-Parametern 4/5 und Wort-Parameter 6 (add_effect_w; int-Argument n bzw. lvl ->
 *   mov al / push ax).
 * - spell_ignifaxius: strcpy-Intrinsic auf Funktionsebene -> id als register-Variable im inneren Block (SI), Zuweisung
 *   getrennt (mov si / or si,si); -Og-Zwischenwerte Typ * 2 [bp-12h] und -lvl [bp-14h].
 * - spell_nekropathia: "struct txt_idx9 answers = g_nekropathia_txt;" (rep movsw), Index "answers.txt[random_schweif(9) - 1]".
 * - spell_adler_wolf: "(signed long)(signed short)level * 450" (cbw / movsx eax,ax); spell_feuerbann ohne (signed short).
 * Originalfehler/Auffaelligkeiten: spell_armatrutz sucht das kleinste k mit k * k >= AE und bietet es als Hoechststufe
 *   an - die Kosten lvl * lvl koennen damit die vorhandene AE uebersteigen (z. B. AE 5 -> Stufe 3 fuer 9 AE).
 */
#include <stdio.h>
#include <string.h>
#include "schweif.h"

/* Phase B: eigene Daten des Moduls (ds:75b4..75c6, dsana) - initialisierte Daten in Adressreihenfolge */
struct txt_idx9 g_nekropathia_txt = { { 44, 45, 46, 47, 48, 49, 50, 230, 231 } };

/* Aufrufe mit Wort-Parametern (das Original uebergibt diese Werte als short) */
#define add_effect_w ((void (*)(struct hero far *, signed short, signed long, signed char, signed char, signed short))add_effect)
#define set_item_broken_w ((void (*)(struct hero far *, signed short, signed short))set_item_broken)

void spell_blitz(CAST_PTR c)
{
	if (c->target_kind == TARGET_HERO || c->target_kind == TARGET_SELF) {
		if (c->target == c->caster) {
			c->result = SPELL_RES_ABORT;
			strcpy(g_text_output_buf, c->txt[112]);
		} else {
			c->target->blind = 3;
			sprintf(g_text_output_buf, c->txt[86], c->target->name);
		}
		return;
	}
	c->enemy->blind = 3;
	sprintf(g_text_output_buf, c->txt[85], grammar_article(0x8000, c->enemy->mon_id, 1));
}

void spell_ecliptifactus(CAST_PTR c)
{
	signed short lvl;
	signed short max;
	signed short cost;

	if (c->caster->ae > 6) {
		max = (c->caster->ae - 5) / 2;
		sprintf(g_text_output_buf, c->txt[87], max);
		if (max < 10)
			lvl = dlg_input(g_dialog, 1, g_text_output_buf, 0, max, 0);
		else
			lvl = dlg_input(g_dialog, 2, g_text_output_buf, 0, max, 0);
		*g_text_output_buf = 0;
		if (lvl > 0) {
			cost = lvl * 2 + 5;
			if (test_spell(c->caster, SP_ECLIPTIFACTUS, c->mod) > 0) {
				c->result = cost;
				c->result += g_spell_cost_mod;
				c->caster->set_ecliptifactus(lvl + 1);
				sprintf(g_text_output_buf, c->txt[88], hero_name_gen(c->caster, 1),
					grammar_pronoun(c->caster->sex, 2, 0), lvl);
			} else {
				c->fail_cost = cost / 2;
				c->result = SPELL_RES_FAIL;
			}
		} else
			c->result = SPELL_RES_ABORT;
		return;
	}
	sprintf(g_text_output_buf, c->txt[140], c->caster->name);
	c->result = SPELL_RES_ABORT;
}

void spell_eisenrost(CAST_PTR c)
{
	signed short id;
	signed char kind;

	kind = target_type(c->caster);
	if (kind == TARGET_HERO || kind == TARGET_SELF) {
		if (c->target == c->caster) {
			c->result = SPELL_RES_ABORT;
			strcpy(g_text_output_buf, c->txt[112]);
		} else {
			id = c->target->inv[INV_SLOT_RIGHT_HAND].item_id;
			if (id == 0)
				c->result = SPELL_RES_FAIL;
			else if (c->target->inv[INV_SLOT_RIGHT_HAND].broken)
				strcpy(g_text_output_buf, c->txt[90]);
			else if (c->target->inv[INV_SLOT_RIGHT_HAND].is_breakable()) {
				set_item_broken_w(c->target, INV_SLOT_RIGHT_HAND, 1);
				sprintf(g_text_output_buf, c->txt[92], grammar_article(0x8000, id, 0), c->target->name);
			} else
				c->result = SPELL_RES_FAIL;
		}
		return;
	}
	if (!c->enemy->f03) {
		sprintf(g_text_output_buf, c->txt[89]);
		return;
	}
	if (c->enemy->broken != 0) {
		strcpy(g_text_output_buf, c->txt[90]);
		return;
	}
	c->enemy->broken = 1;
	sprintf(g_text_output_buf, c->txt[91], grammar_article(0x8000, c->enemy->mon_id, 1));
}

void spell_fulminictus(CAST_PTR c)
{
	signed short dmg;

	if (c->target_kind == TARGET_SELF) {
		c->result = SPELL_RES_ABORT;
		strcpy(g_text_output_buf, c->txt[112]);
		return;
	}
	dmg = dice_roll(3, 6, 0);
	dmg += c->caster->level;
	if (c->caster->ae < dmg)
		dmg = c->caster->ae;
	dmg = cast_damage(c, dmg);
	c->result = dmg;
	c->result += g_spell_cost_mod;
}

void spell_ignifaxius(CAST_PTR c)
{
	signed short dmg;
	signed short lvl;
	signed short rs_lost;
	signed short pos;
	signed short doubled = 0;
	signed short by_item = 0;

	if (c->caster->fb5 == 99)
		by_item = 1;
	if (c->target_kind == TARGET_SELF) {
		c->result = SPELL_RES_ABORT;
		strcpy(g_text_output_buf, c->txt[112]);
		return;
	}
	if (!c->caster->fb5 && !c->caster->npc_id && !(signed char)c->caster->renegade) {
		sprintf(g_text_output_buf, c->txt[93], c->caster->level + 1);
		lvl = dlg_input(g_dialog, 2, g_text_output_buf, 0, c->caster->level + 1, 1);
		*g_text_output_buf = 0;
		if (lvl <= 0) {
			c->result = SPELL_RES_ABORT;
			return;
		}
	} else
		lvl = c->caster->level + 1;
	if (test_spell(c->caster, SP_IGNIFAXIUS, c->mod) > 0) {
		if (by_item)
			dmg = lvl + 20;
		else {
			dmg = dice_roll(lvl, 6, 0);
			if (c->caster->ae < dmg)
				dmg = c->caster->ae;
		}
		if (c->target_kind == TARGET_ENEMY && enemy_is_gfx_1b(c->enemy)) {
			dmg *= 2;
			doubled = 1;
		}
		if (c->target_kind == TARGET_HERO && c->target->fireban)
			dmg /= 2;
		dmg = cast_damage(c, dmg);
		rs_lost = dmg / 10;
		if (c->target_kind == TARGET_HERO) {
			/* Ruestung (Platz 10) verliert RS, hoechstens bis zu ihrem RS */
			register signed short id;

			id = c->target->inv[10].item_id;
			if (id != 0 && rs_lost != 0) {
				if (c->target->inv[10].rs_lost + rs_lost > g_armors_table[g_items[id].table_index].rs)
					rs_lost = g_armors_table[g_items[id].table_index].rs - c->target->inv[10].rs_lost;
				hero_add_rs_lost(c->target, 10, rs_lost);
			}
			pos = get_free_effect(c->target);
			add_effect_w(c->target, pos, 0x1518L, c->target->get_weapon_type(), -lvl / 2, EFFECT_AT);
			pos = get_free_effect(c->target);
			add_effect_w(c->target, pos, 0x1518L, c->target->get_weapon_type(), -lvl / 2, EFFECT_PA);
			*g_text_output_buf = 0;
		} else {
			c->enemy->add_rs_min0(-rs_lost);
			c->enemy->add_at_min0(-lvl / 2);
			c->enemy->add_pa_min0(-lvl / 2);
			*g_text_output_buf = 0;
		}
		if (doubled)
			dmg /= 2;
		c->result = dmg;
		if (g_spell_cost_mod != -1)
			c->result += g_spell_cost_mod;
		return;
	}
	c->fail_cost = lvl / 2;
	c->result = SPELL_RES_FAIL;
}

void spell_plumbumbarum(CAST_PTR c)
{
	signed short pos;

	if (c->target_kind == TARGET_HERO || c->target_kind == TARGET_SELF) {
		if (c->target == c->caster) {
			c->result = SPELL_RES_ABORT;
			strcpy(g_text_output_buf, c->txt[112]);
		} else {
			pos = get_free_effect(c->target);
			add_effect_w(c->target, pos, 45L, c->target->get_weapon_type(), -3, EFFECT_AT);
			sprintf(g_text_output_buf, all_heroes_down(g_group) ? c->txt[233] : c->txt[94],
				hero_name_gen(c->target, 1));
		}
		return;
	}
	c->enemy->add_at_min0(-3);
	sprintf(g_text_output_buf, all_heroes_down(g_group) ? c->txt[233] : c->txt[95],
		grammar_article(0x8001, c->enemy->mon_id, 1));
}

void spell_radau(CAST_PTR c)
{
	/* wirkt nur mit Hexenbesen, dann wie Kraehenruf */
	if (get_item_pos(c->caster, ITEM_WITCHES_BROOM) != -1) {
		spell_kraehenruf(c);
		if (c->result != SPELL_RES_ABORT)
			sprintf(g_text_output_buf, c->txt[216], hero_name_gen(c->caster, 1));
		return;
	}
	strcpy(g_text_output_buf, c->txt[217]);
	c->result = SPELL_RES_FAIL;
}

void spell_saft_kraft(CAST_PTR c)
{
	signed short pos;
	signed short n;

	if (c->target->saftkraft_on) {
		sprintf(g_text_output_buf, c->txt[218], c->target->name, grammar_pronoun(c->target->sex, 0, 0));
		c->result = SPELL_RES_ABORT;
		return;
	}
	n = random_schweif(20) + 2;
	pos = get_free_effect(c->target);
	add_effect_w(c->target, pos, n * 9L, c->target->get_weapon_type(), 5, EFFECT_AT);
	pos = get_free_effect(c->target);
	add_effect_w(c->target, pos, n * 9L, c->target->get_weapon_type(), -5, EFFECT_PA);
	pos = get_free_effect(c->target);
	add_effect_w(c->target, pos, n * 9L, 0, 5, EFFECT_SAFTKRAFT);
	pos = get_free_effect(c->target);
	add_effect_w(c->target, pos, n * 9L, 0, 1, EFFECT_SAFTKRAFT_ON);
	c->result = n;
	c->result += g_spell_cost_mod;
	sprintf(g_text_output_buf, all_heroes_down(g_group) ? c->txt[256] : c->txt[96], c->target->name, n);
}

void spell_scharfes_auge(CAST_PTR c)
{
	signed short pos;

	if (c->target->scharfes_auge) {
		sprintf(g_text_output_buf, c->txt[219], c->target->name);
		c->result = SPELL_RES_NONE;
		return;
	}
	pos = get_free_effect(c->target);
	add_effect_w(c->target, pos, 45L, TA_WURFWAFFEN, 5, EFFECT_SKILL);
	pos = get_free_effect(c->target);
	add_effect_w(c->target, pos, 45L, TA_SCHUSSWAFFEN, 5, EFFECT_SKILL);
	pos = get_free_effect(c->target);
	add_effect_w(c->target, pos, 45L, 0, 1, EFFECT_SCHARFES_AUGE);
	sprintf(g_text_output_buf, c->txt[97], c->target->name);
}

void spell_hexenblick(CAST_PTR c)
{
	if (g_spell_event == 0) {
		strcpy(g_text_output_buf, c->txt[195]);
		c->result = SPELL_RES_NONE;
	}
}

void spell_nekropathia(CAST_PTR c)
{
	struct txt_idx9 answers = g_nekropathia_txt;

	if (g_spell_event == 0) {
		c->target = get_dead_hero(g_group);
		if (c->target == 0) {
			strcpy(g_text_output_buf, c->txt[196]);
			c->result = SPELL_RES_NONE;
			return;
		}
		sprintf(g_text_output_buf, c->txt[232], c->caster->name, hero_name_gen(c->target, 1),
			c->txt[answers.txt[random_schweif(9) - 1]]);
	}
}

void spell_adler_wolf(CAST_PTR c)
{
	signed short pos;

	if (c->caster->adler_wolf) {
		sprintf(g_text_output_buf, c->txt[267], c->caster->name);
		c->result = SPELL_RES_ABORT;
		return;
	}
	pos = get_free_effect(c->caster);
	add_effect_w(c->caster, pos, (signed long)(signed short)c->caster->level * 450, 0, 1, EFFECT_ADLER_WOLF);
	sprintf(g_text_output_buf, c->txt[197], c->caster->name, c->caster->level * 5);
}

void spell_arcano_psychostabilis(CAST_PTR c)
{
	signed short pos;

	pos = get_free_effect(c->target);
	add_effect_w(c->target, pos, 0x1518L, 0, 2, EFFECT_MR);
	*g_text_output_buf = 0;
}

void spell_armatrutz(CAST_PTR c)
{
	signed short lvl;
	signed short pos;
	signed short max;

	if (c->caster->fb5 == 99)
		lvl = 5;
	else {
		/* Hoechststufe: kleinstes k mit k * k >= AE, hoechstens 9 */
#if FIX_0159
		/* Fix #159: groesstes k mit k * k <= AE */
		for (max = 0; (max + 1) * (max + 1) <= c->caster->ae; max++)
			;
#else
		for (max = 0; max * max < c->caster->ae; max++)
			;
#endif
		if (max > 9)
			max = 9;
		if (max) {
			sprintf(g_text_output_buf, c->txt[99], max);
			lvl = dlg_input(g_dialog, 1, g_text_output_buf, 0, max, 0);
		} else
			lvl = 0;
	}
	if (lvl > 0) {
		if (test_spell(c->caster, SP_ARMATRUTZ, c->mod) > 0) {
			c->result = lvl * lvl;
			if (g_spell_cost_mod != -1)
				c->result += g_spell_cost_mod;
			pos = get_free_effect(c->target);
			add_effect_w(c->target, pos, 450L, 0, lvl, EFFECT_RS_BONUS);
			sprintf(g_text_output_buf, c->txt[100], hero_name_gen(c->target, 1), lvl);
		} else {
			c->fail_cost = lvl * lvl / 2;
			c->result = SPELL_RES_FAIL;
			*g_text_output_buf = 0;
		}
		return;
	}
	c->result = SPELL_RES_ABORT;
	*g_text_output_buf = 0;
}

void spell_inc_attrib(CAST_PTR c, signed short attrib)
{
	signed short pos;

	if (c->target == c->caster) {
		c->result = SPELL_RES_ABORT;
		strcpy(g_text_output_buf, c->txt[112]);
		return;
	}
	if (c->target->attrib[attrib].cur > c->target->attrib[attrib].normal) {
		sprintf(g_text_output_buf, c->txt[113], c->target->name, g_ttx[attrib + 308]);
		return;
	}
	pos = get_free_effect(c->target);
	add_effect_w(c->target, pos, 0x2a30L, attrib, 2, EFFECT_ATTRIB);
	*g_text_output_buf = 0;
}

void spell_ch_steigern(CAST_PTR c)
{
	spell_inc_attrib(c, ATTRIB_CH);
}

void spell_feuerbann(CAST_PTR c)
{
	signed short pos;

	if (!c->caster->fireban) {
		pos = get_free_effect(c->caster);
		add_effect_w(c->caster, pos, (signed long)c->caster->level * 450, 0, 1, EFFECT_FIREBAN);
		sprintf(g_text_output_buf, c->txt[102], c->caster->name);
		return;
	}
	sprintf(g_text_output_buf, c->txt[220], c->caster->name);
	c->result = SPELL_RES_ABORT;
}

void spell_ff_steigern(CAST_PTR c)
{
	spell_inc_attrib(c, ATTRIB_FF);
}

void spell_ge_steigern(CAST_PTR c)
{
	spell_inc_attrib(c, ATTRIB_GE);
}

void spell_in_steigern(CAST_PTR c)
{
	spell_inc_attrib(c, ATTRIB_IN);
}

void spell_kk_steigern(CAST_PTR c)
{
	spell_inc_attrib(c, ATTRIB_KK);
}

void spell_kl_steigern(CAST_PTR c)
{
	spell_inc_attrib(c, ATTRIB_KL);
}

void spell_mu_steigern(CAST_PTR c)
{
	spell_inc_attrib(c, ATTRIB_MU);
}

void spell_mutabili(CAST_PTR c)
{
	if (g_spell_event == 0) {
		strcpy(g_text_output_buf, c->txt[198]);
		c->result = SPELL_RES_NONE;
	}
}
