/* OVL234 - Overlay O234 (Segment 0x2414 in SCHWEIF.EXE): Zauberroutinen 9..26 (Beherrschung, Daemonologie,
 * Elementar) als freie Funktionen spell_*(CAST_PTR c) - Methoden der Klasse Cast (O233) ueber g_spell_handlers
 * (Ergebnis c->result SPELL_RES_* bzw. AE-Kosten, Teilkosten c->fail_cost, Probenzuschlag c->mod, Zielart
 * c->target_kind, Zaubernder c->caster, Ziel c->target bzw. c->enemy).
 * spell_grosse_gier .. spell_zwingtanz: Gegnerbits (scared/tame/asleep/dancing), Untote enemy_is_gfx_19_3f immun.
 * spell_magischer_raub: AE vom Ziel-Helden uebertragen (dlg_input, sub_ae_splash/add_hero_ae). spell_respondami: NSC-Text.
 * Beschwoerungen spell_furor_blut/spell_heptagon/spell_elementare_beschwoeren (Proben test_spell, Kosten 0x21/0x25/0x12,
 * Ergebnis g_spell_summon_mon/_renegade; Heptagon-Patzer: Zauberwert -5, AE-Grundwert -2W20 ueber add_hero_ae2),
 * spell_skelettarius. spell_kraehenruf: Stufe + 2 Wuerfe gegen cast_target_pa -> cast_damage.
 * spell_nihilatio_gravitas/spell_solidirid: Stufe waehlen (dlg_input), Kosten 5r bzw. 2r, g_spell_level = r.
 *
 * BCC -mlarge -3 -Ot -Og -Oi -c -Y OVL234.CPP   -> 5986/5986 Bytes GLEICH (18.10.2026), cobjfix laeuft.
 *   (Namensfassung mit schweif.h: 19.10.2026 GLEICH, LINKGLEICH) Ohne -Og 2 Bytes zu kurz (grosse_verwirrung).
 * - -Oi: strcpy(g_text_output_buf, c->txt[..]) als Intrinsic; c->txt[npc->npc_id + 0x118] -> Konstante in der
 *   Verschiebung.
 * - enemy::add_at_min0/add_pa_min0(-2) inline (add byte 0feh, toter jmp short bzw. Epilogkopie am Funktionsende),
 *   hero::set_spell (Kopie [bp-3]). Gegnerbits unsigned short b:1 ("!(signed char)e->dead" -> cbw / or).
 * - test_spell mit signed-char-Parameter: "c->mod + w + 4" -> add al,[bp-2] / add al,4.
 * - Kosten von Herr ueber das Tierreich: Byte +0x22 (enemy.first_ap) unsigned gelesen.
 * Originalfehler/Auffaelliges: spell_nihilatio_gravitas bietet als Hoechststufe das GROESSERE von Stufe und AE/5 an
 *   (gemeint wohl das kleinere - Kosten 5r koennen die AE uebersteigen); spell_kraehenruf speichert das Ergebnis von
 *   cast_damage in hits, ohne es zu benutzen.
 */
#include <stdio.h>
#include <string.h>
#include "schweif.h"

/* set_asleep mit Wort-Parameter (das Original uebergibt 1 als short) */
#define set_asleep_w ((void (*)(struct hero far *, signed short))set_asleep)

void spell_grosse_gier(CAST_PTR c)
{
	if (!g_spell_event) {
		if (c->target_kind == TARGET_ENEMY || c->target_kind == TARGET_SPECIAL) {
			if (c->enemy->is_animal) {
				c->result = SPELL_RES_ABORT;
				sprintf(g_text_output_buf, c->txt[8], grammar_article(0, c->enemy->mon_id, 1));
			} else {
				sprintf(g_text_output_buf, c->txt[0xcf], grammar_article(0x8000, c->enemy->mon_id, 1));
				c->enemy->scared = 1;
			}
			return;
		}
		sprintf(g_text_output_buf, c->txt[0xcf], c->target->name);
		c->target->scared = 1;
	}
}

void spell_grosse_verwirrung(CAST_PTR c)
{
	if (enemy_is_gfx_19_3f(c->enemy)) {
		c->result = SPELL_RES_FAIL;
		strcpy(g_text_output_buf, c->txt[0xb1]);
		return;
	}
	sprintf(g_text_output_buf, c->txt[0xb2], grammar_article(0x8000, c->enemy->mon_id, 1));
	c->enemy->add_at_min0(-2);
	c->enemy->add_pa_min0(-2);
}

void spell_herr_ueber_das_tierreich(CAST_PTR c)
{
	if (!c->enemy->is_animal) {
		c->result = SPELL_RES_ABORT;
		sprintf(g_text_output_buf, c->txt[0x0b], grammar_article(0, c->enemy->mon_id, 1));
		return;
	}
	/* Kosten: Byte +0x22 des Gegners (first_ap, unsigned gelesen) */
	c->result = (unsigned char)c->enemy->first_ap;
	c->result += g_spell_cost_mod;
	if (c->result > c->caster->ae) {
		c->result = SPELL_RES_FAIL;
		c->fail_cost = c->caster->ae;
		return;
	}
	c->enemy->tame = 1;
	sprintf(g_text_output_buf, c->txt[9], grammar_article(0x8000, c->enemy->mon_id, 1));
}

void spell_horriphobus(CAST_PTR c)
{
	if (enemy_is_gfx_19_3f(c->enemy)) {
		c->result = SPELL_RES_FAIL;
		strcpy(g_text_output_buf, c->txt[0xb8]);
		return;
	}
	if (c->enemy->magic) {
		c->result = SPELL_RES_FAIL;
		strcpy(g_text_output_buf, c->txt[0x2b]);
		return;
	}
	c->enemy->scared = 1;
	c->enemy->renegade = 0;
	sprintf(g_text_output_buf, all_heroes_down(g_group) ? c->txt[0xea] : c->txt[0x0c],
		grammar_article(0x8000, c->enemy->mon_id, 1));
}

void spell_magischer_raub(CAST_PTR c)
{
	signed short ae = 0;
	signed short max;

	if (c->target == c->caster) {
		strcpy(g_text_output_buf, c->txt[0x70]);
		c->result = SPELL_RES_ABORT;
	} else if (c->target->ae < 7) {
		sprintf(g_text_output_buf, c->txt[0x121], c->target->name);
		c->result = SPELL_RES_ABORT;
	} else if (c->caster->ae_max == c->caster->ae) {
		sprintf(g_text_output_buf, c->txt[0x123], c->caster->name);
		c->result = SPELL_RES_ABORT;
	} else {
		max = c->target->ae;
		if (c->caster->ae_max - c->caster->ae < max)
			max = c->caster->ae_max - c->caster->ae;
		sprintf(g_text_output_buf, c->txt[0xb9], c->caster->name, c->target->name, 7, max);
		ae = dlg_input(g_dialog, 2, g_text_output_buf, 7, max, 7);
		*g_text_output_buf = 0;
	}
	if (ae > 0) {
		sub_ae_splash(c->target, ae);
		add_hero_ae(c->caster, ae);
		sprintf(g_text_output_buf, c->txt[0x122], c->caster->name, ae, c->target->name);
		return;
	}
	c->result = SPELL_RES_ABORT;
}

void spell_respondami(CAST_PTR c)
{
	struct hero far *npc;

	npc = group_get_npc(g_group);
	if (npc) {
		strcpy(g_text_output_buf, c->txt[npc->npc_id + 0x118]);
		return;
	}
	if (!g_spell_event) {
		strcpy(g_text_output_buf, c->txt[0xba]);
		c->result = SPELL_RES_NONE;
	}
}

void spell_sanftmut(CAST_PTR c)
{
	spell_herr_ueber_das_tierreich(c);
}

void spell_somnigravis(CAST_PTR c)
{
	if (c->target_kind == TARGET_ENEMY || c->target_kind == TARGET_SPECIAL) {
		if (enemy_is_gfx_19_3f(c->enemy)) {
			c->result = SPELL_RES_FAIL;
			strcpy(g_text_output_buf, c->txt[0xb8]);
		} else {
			c->enemy->asleep = 1;
			sprintf(g_text_output_buf, c->txt[0x0d], grammar_article(0x8000, c->enemy->mon_id, 1));
		}
		return;
	}
	if (c->target == c->caster) {
		c->result = SPELL_RES_ABORT;
		strcpy(g_text_output_buf, c->txt[0x70]);
		return;
	}
	set_asleep_w(c->target, 1);
	sprintf(g_text_output_buf, c->txt[0x0d], c->target->name);
}

void spell_zwingtanz(CAST_PTR c)
{
	if (enemy_is_gfx_19_3f(c->enemy)) {
		c->result = SPELL_RES_FAIL;
		strcpy(g_text_output_buf, c->txt[0xb8]);
		return;
	}
	c->enemy->dancing = 1;
	sprintf(g_text_output_buf, c->txt[0x0e], grammar_article(0x8000, c->enemy->mon_id, 1));
}

void spell_furor_blut(CAST_PTR c)
{
	signed short w;
	register signed short t;

	w = random_schweif(2);
	c->caster->action_id = FIG_ACTION_WAIT;
	if (c->caster->ae < 13) {
		sprintf(g_text_output_buf, c->txt[0x8c], c->caster->name);
		c->result = SPELL_RES_FAIL;
		c->fail_cost = 13;
		return;
	}
	if (g_nr_enemies == 20) {
		sprintf(g_text_output_buf, c->txt[0xf0], hero_name_gen(c->caster, 1));
		c->result = SPELL_RES_FAIL;
		c->fail_cost = 13;
		return;
	}
	if (test_spell(c->caster, SP_FUROR_BLUT, c->mod + w + 4) > 0) {
		if (test_spell(c->caster, SP_FUROR_BLUT, c->mod + w - 2) > 0) {
			c->result = 0x21;
			if (c->caster->ae < c->result) {
				sprintf(g_text_output_buf, c->txt[0xf5], hero_name_gen(c->caster, 1), hero_name_gen(c->caster, 1));
				c->result = SPELL_RES_FAIL;
				c->fail_cost = 13;
			} else {
				sprintf(g_text_output_buf, c->txt[0xf3], hero_name_gen(c->caster, 1));
				g_spell_summon_mon = w == 1 ? 4 : 0x11;
				g_spell_summon_renegade = 1;
			}
		} else {
			c->result = 13;
			t = random_schweif(6);
			if (t < 4) {
				sprintf(g_text_output_buf, c->txt[0xf1], c->caster->name);
				c->result = SPELL_RES_FAIL;
				c->fail_cost = 13;
			} else {
				sprintf(g_text_output_buf, c->txt[0xf2], c->caster->name);
				g_spell_summon_mon = w == 1 ? 4 : 0x11;
				g_spell_summon_renegade = 0;
			}
		}
		return;
	}
	t = random_schweif(6);
	if (t < 5) {
		sprintf(g_text_output_buf, c->txt[0xf0], hero_name_gen(c->caster, 1));
		c->result = SPELL_RES_FAIL;
		c->fail_cost = 7;
		return;
	}
	sprintf(g_text_output_buf, c->txt[0xf4], hero_name_gen(c->caster, 1));
	g_spell_summon_mon = 0x50;
	g_spell_summon_renegade = 0;
	c->result = 13;
}

void spell_geister_austreiben(CAST_PTR c)
{
	if (g_dng05_poltergeist == 1) {
		g_dng05_poltergeist = 3;
		return;
	}
	if (!g_spell_event) {
		strcpy(g_text_output_buf, c->txt[0xbb]);
		c->result = SPELL_RES_NONE;
	}
}

void spell_geister_beschwoeren(CAST_PTR c)
{
	if (!g_spell_event) {
		strcpy(g_text_output_buf, c->txt[0xef]);
		c->result = SPELL_RES_NONE;
	}
}

void spell_heptagon(CAST_PTR c)
{
	signed short t;
	register signed short d;

	c->caster->action_id = FIG_ACTION_WAIT;
	if (c->caster->ae < 13) {
		sprintf(g_text_output_buf, c->txt[0x8c], c->caster->name);
		c->result = SPELL_RES_FAIL;
		c->fail_cost = 13;
		return;
	}
	if (g_nr_enemies == 20) {
		sprintf(g_text_output_buf, c->txt[0xf0], hero_name_gen(c->caster, 1));
		c->result = SPELL_RES_FAIL;
		c->fail_cost = 13;
		return;
	}
	if (test_spell(c->caster, SP_HEPTAGON, c->mod + 6) > 0) {
		if (test_spell(c->caster, SP_HEPTAGON, c->mod) > 0) {
			c->result = 0x25;
			if (c->caster->ae < c->result) {
				sprintf(g_text_output_buf, c->txt[0xfa], hero_name_gen(c->caster, 1), hero_name_gen(c->caster, 1));
				c->result = SPELL_RES_FAIL;
				c->fail_cost = 13;
			} else {
				sprintf(g_text_output_buf, c->txt[0xf8], hero_name_gen(c->caster, 1));
				g_spell_summon_mon = 0x50;
				g_spell_summon_renegade = 1;
			}
		} else {
			c->result = 13;
			t = random_schweif(6);
			if (t < 4) {
				sprintf(g_text_output_buf, c->txt[0xf6], c->caster->name);
				c->result = SPELL_RES_FAIL;
				c->fail_cost = 13;
			} else {
				sprintf(g_text_output_buf, c->txt[0xf7], c->caster->name);
				g_spell_summon_mon = 0x50;
				g_spell_summon_renegade = 0;
			}
		}
		return;
	}
	t = random_schweif(6);
	if (t < 4) {
		sprintf(g_text_output_buf, c->txt[0xf0], hero_name_gen(c->caster, 1));
		c->fail_cost = 13;
		c->result = SPELL_RES_FAIL;
		return;
	}
	if (t == 6) {
		/* Patzer: Heptagon-Wert -5 (hoechstens -18), AE-Grundwert -2W20 */
		c->fail_cost = 0x1a;
		c->result = SPELL_RES_FAIL;
		t = c->caster->spells[SP_HEPTAGON];
		t -= 5;
		if (t < -18)
			t = -18;
		c->caster->set_spell(SP_HEPTAGON, t);
		d = random_interval(2, 40);
		if (c->caster->ae_max <= d)
			d = c->caster->ae_max - 1;
		add_hero_ae2(c->caster, -d);
		sprintf(g_text_output_buf, c->txt[0xfe], hero_name_gen(c->caster, 1), c->caster->name,
			grammar_pronoun(c->caster->sex, 3, 0), d, grammar_pronoun(c->caster->sex, 3, 0));
		return;
	}
	sprintf(g_text_output_buf, c->txt[0xf9], hero_name_gen(c->caster, 1));
	g_spell_summon_mon = 0x50;
	g_spell_summon_renegade = 0;
	c->result = 13;
}

void spell_kraehenruf(CAST_PTR c)
{
	signed short n;
	signed short i;
	signed short hits;
	signed short pa;

	if (c->target == c->caster) {
		strcpy(g_text_output_buf, c->txt[0x70]);
		c->result = SPELL_RES_ABORT;
		return;
	}
	n = c->caster->level + 2;
	pa = cast_target_pa(c);
	hits = 1;
	sprintf(g_text_output_buf, c->txt[0xd0], n);
	for (i = 0; i < n; i++) {
		if (random_schweif(20) <= 8 && random_schweif(20) > pa)
			hits++;
	}
	hits = cast_damage(c, hits);
}

void spell_skelettarius(CAST_PTR c)
{
	c->caster->action_id = FIG_ACTION_WAIT;
	if (!(signed char)c->enemy->dead) {
		sprintf(g_text_output_buf, c->txt[0x0f], grammar_article(0x8000, c->enemy->mon_id, 1));
		c->result = SPELL_RES_ABORT;
		return;
	}
	sprintf(g_text_output_buf, c->txt[0x10], grammar_article(0x8000, c->enemy->mon_id, 1));
	g_spell_summon_mon = 0x10;
	g_spell_summon_renegade = 1;
}

void spell_elementare_beschwoeren(CAST_PTR c)
{
	c->caster->action_id = FIG_ACTION_WAIT;
	if (c->caster->ae < 13) {
		sprintf(g_text_output_buf, c->txt[0x8c], c->caster->name);
		c->result = SPELL_RES_FAIL;
		c->fail_cost = 13;
		return;
	}
	if (g_nr_enemies == 20) {
		sprintf(g_text_output_buf, c->txt[0xfb], hero_name_gen(c->caster, 1));
		c->result = SPELL_RES_FAIL;
		c->fail_cost = 13;
		return;
	}
	if (test_spell(c->caster, SP_ELEMENTARE_BESCHWOEREN, c->mod + 2) > 0) {
		c->result = 0x12;
		if (c->caster->ae < c->result) {
			sprintf(g_text_output_buf, c->txt[0xfd], hero_name_gen(c->caster, 1));
			c->result = SPELL_RES_FAIL;
			c->fail_cost = 13;
		} else {
			sprintf(g_text_output_buf, c->txt[0xfc], c->caster->name);
			g_spell_summon_mon = 0x52;
			g_spell_summon_renegade = 1;
		}
		return;
	}
	sprintf(g_text_output_buf, c->txt[0xfb], hero_name_gen(c->caster, 1));
	c->result = SPELL_RES_FAIL;
	c->fail_cost = 13;
}

void spell_nihilatio_gravitas(CAST_PTR c)
{
	signed short lvl;
	register signed short max;

#if FIX_0166
	/* Fix #166: Hoechststufe = kleinerer Wert von Stufe und AE/5 */
	max = c->caster->level < c->caster->ae / 5 ? c->caster->level : c->caster->ae / 5;
#else
	max = c->caster->level < c->caster->ae / 5 ? c->caster->ae / 5 : c->caster->level;
#endif
	sprintf(g_text_output_buf, c->txt[0xbc], c->caster->name, max);
	if (max < 10)
		lvl = dlg_input(g_dialog, 1, g_text_output_buf, 0, max, 0);
	else
		lvl = dlg_input(g_dialog, 2, g_text_output_buf, 0, max, 0);
	*g_text_output_buf = 0;
	if (lvl > 0) {
		if (test_spell(c->caster, SP_NIHILATIO_GRAVITAS, c->mod) > 0) {
			c->result = lvl * 5;
			c->result += g_spell_cost_mod;
			sprintf(g_text_output_buf, c->txt[0xd1], c->caster->name, lvl * 2);
			g_spell_level = lvl;
		} else {
			c->fail_cost = lvl * 5 / 2;
			c->result = SPELL_RES_FAIL;
		}
		return;
	}
	c->result = SPELL_RES_ABORT;
}

void spell_solidirid(CAST_PTR c)
{
	signed short lvl;
	register signed short max;

	max = c->caster->ae / 2;
	sprintf(g_text_output_buf, c->txt[0xbd], c->caster->name, max);
	if (max < 10)
		lvl = dlg_input(g_dialog, 1, g_text_output_buf, 0, max, 0);
	else
		lvl = dlg_input(g_dialog, 2, g_text_output_buf, 0, max, 0);
	*g_text_output_buf = 0;
	if (lvl > 0) {
		if (test_spell(c->caster, SP_SOLIDIRID, c->mod) > 0) {
			c->result = lvl * 2;
			c->result += g_spell_cost_mod;
			sprintf(g_text_output_buf, c->txt[0xd2], c->caster->name, lvl);
			g_spell_level = lvl;
		} else {
			c->fail_cost = lvl;
			c->result = SPELL_RES_FAIL;
		}
		return;
	}
	c->result = SPELL_RES_ABORT;
}
