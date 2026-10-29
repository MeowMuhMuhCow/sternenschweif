/* OVL194 - Overlay O194 (Segment 0x2339 in SCHWEIF.EXE): Zauber der Gegner, Klasse Spell (0x1e Bytes, s_class.h),
 * lokal in enemy_cast_spell (O192); Gegenstueck zu Cast (O233). mon_spell_cost(n, half) (BE MON_get_spell_cost):
 * AE-Kosten aus g_mon_spell_descriptions (-1 = W(5..10) bei halben Kosten). Konstruktor(e): Zaubertexte (Datei 0x1f
 * bzw. g_b690), caster = e, result = SPELL_RES_DEFAULT. Destruktor: Texte freigeben. damage(le) (BE
 * MON_do_spell_damage): Schaden an Held (sub_hero_le) bzw. Gegner (Enemy::sub_le), Kampfliste Lst_add, tot ->
 * g_fig_target_killed. target_pa, target_rs des Ziels. cast(n) (BE MON_cast_spell): Ziel holen (Enemy::target_hero /
 * target_enemy nach ENEMY_TARGET_TYPE), Zauberroutine g_mon_spell_handlers[n] (MSP_*) aufrufen, Klang
 * (arc_exists/snd_play2), Ergebnis SPELL_RES_* bzw. Kosten -> Enemy::sub_ae. Zauberroutinen mon_spell_* (BE
 * mspell_*): Verwandlung beenden .. Paralue (Werte des Ziels aendern, Texte aus txt).
 *
 * BCC -mlarge -3 -Ot -Og -Oi -c -Y -E..\bc\BIN\TASM.EXE OVL194.CPP -> 4769/4769 Bytes GLEICH (18.10.2026),
 *   cobjfix laeuft (g_mon_spell_handlers -> _g_mon_spell_repertoire+0x41). Ohne -Og 3320, ohne -Oi 2067 Abweichungen.
 *   (Namensfassung mit schweif.h: 19.10.2026 GLEICH, LINKGLEICH)
 * - Methodenzeiger wie OVL233: "class Spell;" vor "typedef void (Spell::*SPELL_PMF)();" -> func = g_mon_spell_handlers[n]
 *   per SCOPY@ (cx = 8), (this->*func)() -> call dword ptr es:[bx+0ah].
 * - Zielart als Makro ENEMY_TARGET_TYPE(e) (s_struct.h, wie O138).
 * - Begrenzung "if (x < 0) x = 0; else" (Makro CLAMP0) erzeugt den toten jmp short; am Funktionsende Epilogkopie.
 * - Aenderungen der Gegnerwerte ueber inline add_rs/add_at/add_pa(signed char) (add byte es:[..]; mit Variablen
 *   eigene Parameterkopie [bp-5]..[bp-7] in mon_spell_ignifaxius, [bp-3] in mon_spell_armatrutz); set_attacks(attacks
 *   + 1)/set_bp(bp << 1) in mon_spell_axxeleratus; "saftkraft = saftkraft + 5" steht direkt so da; at++ -> inc byte.
 * - Bitfelder (unsigned short b:1) im Wahrheitstest mit (signed char); Waffenplatz ueber hero::is_inv_breakable(3).
 * - add_effect/set_item_broken/Lst_add mit Wort-Parametern ueber gecastete Funktionszeiger (*_w).
 * - Aufruf ::mon_spell_cost(n, ..) innerhalb der Klasse, push cs / call near.
 * Originalfehler/Auffaelligkeiten:
 * - mon_spell_balsam: result = 7 nur bei Fehlbetrag < 7, sonst bleibt result = SPELL_RES_ABORT (-4) -> das Ziel
 *   bekommt sub_le(4, 0) Schaden statt Heilung, und cast() wertet -4 als "ohne Kosten". Gemeint war wohl mindestens 7.
 * - mon_spell_verwandlung_beenden: reicht die AE nicht, wird die ganze AE verbraucht (result = AE), ohne Wirkung.
 */
#include <stdio.h>
#include <string.h>
#include "schweif.h"

/* Aufrufe mit Wort-Parametern (das Original uebergibt diese Werte als short) */
#define add_effect_w ((void (*)(struct hero far *, signed short, signed long, signed short, signed short, signed short))add_effect)
#define set_item_broken_w ((void (*)(struct hero far *, signed short, signed short))set_item_broken)
#define lst_add_w ((void (*)(void far *, signed short, signed short, signed short, void far *, signed short, void far *))Lst_add)
/* Wert nach einer Aenderung nicht negativ werden lassen (das else erzeugt den toten jmp short des Originals) */
#define CLAMP0(x) if ((x) < 0) (x) = 0; else

signed short mon_spell_cost(signed short n, signed short half)
{
	signed char cost;

	cost = g_mon_spell_descriptions[n].ae_cost;
	if (half) {
		if (cost == -1)
			cost = random_interval(5, 10);
		else
			cost = cost / 2;
	}
	return cost;
}

Spell::Spell(struct enemy far *e)
{
	signed short i;

	if (!g_quick_fight) {
		txt_load_list(g_font, 0x1f, 300, &txt_buf, &txt);
	} else {
		txt = (char far * far *)mem_alloc(0x4b4L);
		txt_buf = mem_alloc(10L);
		for (i = 0; i <= 300; i++)
			txt[i] = g_b690;
	}
	caster = (Enemy far *)e;
	result = SPELL_RES_DEFAULT;
}

Spell::~Spell()
{
	mem_free(txt_buf);
	mem_free(txt);
}

void Spell::damage(signed short le)
{
	if (le <= 0)
		return;
	if (ENEMY_TARGET_TYPE(caster) == TARGET_HERO) {
		sub_hero_le(hero, le);
		lst_add_w(g_fight_list, 8, le, 1, caster, 2, hero);
		if ((signed char)hero->dead)
			g_fig_target_killed = 1;
	} else {
		target->sub_le(le, 1);
		lst_add_w(g_fight_list, 0x0b, le, 1, caster, 1, target);
		if ((signed char)target->dead)
			g_fig_target_killed = 1;
	}
}

signed short Spell::target_pa()
{
	if (ENEMY_TARGET_TYPE(caster) == TARGET_HERO)
		return hero_pa(hero);
	else
		return target->pa_value();
}

signed short Spell::target_rs()
{
	if (ENEMY_TARGET_TYPE(caster) == TARGET_HERO)
		return (signed char)(hero->rs_bonus + hero->rs_bonus2);
	else
		return target->rs;
}

signed short Spell::cast(signed short n)
{
	signed short cost;
	signed short retval;

	retval = 1;
	*g_text_output_buf = 0;
	cost = ::mon_spell_cost(n, 0);
	if (ENEMY_TARGET_TYPE(caster) == TARGET_HERO)
		hero = caster->target_hero();
	else
		target = caster->target_enemy();
	func = g_mon_spell_handlers[n];
	(this->*func)();
	if (result != SPELL_RES_NONE && result != SPELL_RES_ABORT && g_spell_flags.no_sound == 0 &&
		g_mon_spell_descriptions[n].snd != 0 && (g_fight_obj == 0 || g_fight_screen_on == 1)) {
		if (arc_exists(g_arc_fx, g_mon_spell_descriptions[n].snd))
			snd_play2(g_snd_fx, g_mon_spell_descriptions[n].snd);
		else
			snd_play2(g_snd_fx, 0x92);
	}
	if (result == SPELL_RES_NONE) {
		retval = -1;
		g_spell_cancelled = 1;
	} else if (result == SPELL_RES_FAIL) {
		caster->sub_ae(::mon_spell_cost(n, 1));
		strcpy(g_text_output_buf, txt[0x109]);
		retval = -1;
	} else if (result == SPELL_RES_ABORT) {
		retval = 0;
		g_spell_cancelled = 1;
	} else if (result == SPELL_RES_DEFAULT) {
		caster->sub_ae(cost);
		g_spell_success = 1;
	} else {
		caster->sub_ae(result);
		g_spell_success = 1;
	}
	memset(&g_spell_flags, 0, 7);
	return retval;
}

void Spell::mon_spell_verwandlung_beenden()
{
	if ((signed char)target->petrified) {
		result = random_schweif(10) * 5;
		if (result > caster->ae)
			result = caster->ae;
		else {
			target->petrified = 0;
			sprintf(g_text_output_buf, txt[0x72], grammar_article(0x8000, target->mon_id, 1));
		}
	} else if ((signed char)target->mushroom) {
		result = random_schweif(10) * 5;
		if (result > caster->ae)
			result = caster->ae;
		else {
			target->mushroom = 0;
			g_enemy_unmushroomed = 1;
		}
	} else
		result = 2;
}

void Spell::mon_spell_bannbaladin()
{
	if (get_item_pos(hero, 0x108) == -1) {
		hero->tame = 1;
		sprintf(g_text_output_buf, txt[0x73], hero->name);
	} else
		sprintf(g_text_output_buf, all_heroes_down(g_group) ? txt[0x101] : txt[0x91], hero->name);
}

void Spell::mon_spell_boeser_blick()
{
	if (get_item_pos(hero, 0x108) == -1) {
		hero->renegade = 1;
		g_hero_died = 1;
		if (hero->is_mage())
			hero->fb6_b0 = 1;
		sprintf(g_text_output_buf, all_heroes_down(g_group) ? txt[0xee] : txt[0x74], hero->name);
	} else
		sprintf(g_text_output_buf, all_heroes_down(g_group) ? txt[0x101] : txt[0x91], hero->name);
}

void Spell::mon_spell_horriphobus()
{
	if (get_item_pos(hero, 0x108) == -1) {
		hero->scared = 1;
		hero->renegade = 0;
		sprintf(g_text_output_buf, all_heroes_down(g_group) ? txt[0xea] : txt[0x75], hero->name);
	} else
		sprintf(g_text_output_buf, all_heroes_down(g_group) ? txt[0x101] : txt[0x91], hero->name);
}

void Spell::mon_spell_axxeleratus()
{
	target->set_attacks(target->attacks + 1);
	target->at++;
	CLAMP0(target->at);
	target->pa++;
	CLAMP0(target->pa);
	target->set_bp(target->bp << 1);
}

void Spell::mon_spell_balsam()
{
	signed short le;
	signed char heal;

	result = SPELL_RES_ABORT;
	le = (target->le_max - target->le) / 2;
	if (le != 0) {
#if FIX_0111
		/* Fix #111: Heilung mindestens 7, sonst der Fehlbetrag (nicht SPELL_RES_ABORT als Schaden) */
		result = le < 7 ? 7 : le;
#else
		if (le < 7)
			result = 7;
#endif
		if (result > caster->ae)
			result = caster->ae;
		heal = result;
		target->sub_le(-heal, 0);
	}
}

void Spell::mon_spell_blitz()
{
	if (ENEMY_TARGET_TYPE(caster) == TARGET_HERO) {
		hero->blind = 3;
		sprintf(g_text_output_buf, txt[0x56], hero->name);
	} else {
		target->blind = 3;
		sprintf(g_text_output_buf, txt[0x55], grammar_article(0x8000, target->mon_id, 1));
	}
}

void Spell::mon_spell_eisenrost()
{
	signed short id;

	if (ENEMY_TARGET_TYPE(caster) == TARGET_HERO) {
		id = hero->inv[INV_SLOT_RIGHT_HAND].item_id;
		if (id == 0)
			result = 2;
		else if (!hero->inv[INV_SLOT_RIGHT_HAND].broken) {
			if (hero->is_inv_breakable(INV_SLOT_RIGHT_HAND)) {
				set_item_broken_w(hero, INV_SLOT_RIGHT_HAND, 1);
				sprintf(g_text_output_buf, txt[0x5c], grammar_article(0x8000, id, 0), hero->name);
			} else
				result = SPELL_RES_FAIL;
		}
	} else {
		if (!target->broken) {
			target->broken = 1;
			sprintf(g_text_output_buf, txt[0x5b], grammar_article(0x8000, target->mon_id, 1));
		}
	}
}

void Spell::mon_spell_fulminictus()
{
	signed short dmg;

	dmg = dice_roll(3, 6, 0);
	dmg += caster->level;
	if (ENEMY_TARGET_TYPE(caster) == TARGET_HERO) {
		/* Gegenstaende 0x11e/0xdc halbieren den Schaden */
		if (get_item_pos(hero, 0x11e) != -1 || get_item_pos(hero, 0xdc) != -1)
			dmg /= 2;
	}
	if (caster->ae < dmg)
		dmg = caster->ae;
	damage(dmg);
	result = dmg;
}

void Spell::mon_spell_ignifaxius()
{
	signed short dmg;
	signed short lvl;
	signed short rs_lost;
	signed short pos;

	lvl = caster->level;
	dmg = dice_roll(lvl, 6, 0);
	if (ENEMY_TARGET_TYPE(caster) == TARGET_HERO) {
		if (get_item_pos(hero, 0x11e) != -1 || get_item_pos(hero, 0xdc) != -1)
			dmg /= 2;
		if (hero->fireban != 0)
			dmg /= 2;
	}
	if (caster->ae < dmg)
		dmg = caster->ae;
	damage(dmg);
	rs_lost = dmg / 10;
	if (ENEMY_TARGET_TYPE(caster) == TARGET_HERO) {
		/* Ruestung (Platz 10) verliert RS, hoechstens bis zu ihrem RS */
		if (hero->inv[10].item_id != 0 && rs_lost != 0) {
			if (hero->inv[10].rs_lost + rs_lost > g_armors_table[g_items[hero->inv[10].item_id].table_index].rs)
				rs_lost = g_armors_table[g_items[hero->inv[10].item_id].table_index].rs - hero->inv[10].rs_lost;
			hero_add_rs_lost(hero, 10, rs_lost);
		}
		pos = get_free_effect(hero);
		add_effect_w(hero, pos, 0x1518L, hero->weapon_type, -lvl / 2, EFFECT_AT);
		pos = get_free_effect(hero);
		add_effect_w(hero, pos, 0x1518L, hero->weapon_type, -lvl / 2, EFFECT_PA);
		*g_text_output_buf = 0;
	} else {
		target->add_rs(-rs_lost);
		CLAMP0(target->rs);
		target->add_at(-lvl / 2);
		CLAMP0(target->at);
		target->add_pa(-lvl / 2);
		CLAMP0(target->pa);
		*g_text_output_buf = 0;
	}
	result = dmg;
}

void Spell::mon_spell_plumbumbarum()
{
	signed short pos;

	if (ENEMY_TARGET_TYPE(caster) == TARGET_HERO) {
		pos = get_free_effect(hero);
		add_effect_w(hero, pos, 0x1518L, hero->weapon_type, -3, EFFECT_AT);
		sprintf(g_text_output_buf, txt[0x5e], hero_name_gen(hero, 1));
	} else {
		target->add_at(-3);
		CLAMP0(target->at);
		sprintf(g_text_output_buf, txt[0x5f], grammar_article(0x8001, target->mon_id, 1));
	}
}

void Spell::mon_spell_saft_kraft()
{
	target->add_at(5);
	CLAMP0(target->at);
	target->add_pa(-5);
	CLAMP0(target->pa);
	target->saftkraft = target->saftkraft + 5;
	CLAMP0(target->saftkraft);
	target->f39_b4 = 1;
	result = random_schweif(20);
}

void Spell::mon_spell_armatrutz()
{
	signed short i;
	signed short rs;

	for (i = 0; i * i < caster->ae; i++)
		;
	rs = random_interval(1, i);
	result = rs * rs;
	caster->add_rs(rs);
	CLAMP0(caster->rs);
}

void Spell::mon_spell_paralue()
{
	if (ENEMY_TARGET_TYPE(caster) == TARGET_ENEMY) {
		target->petrified = 1;
		sprintf(g_text_output_buf, txt[0x67], grammar_article(0x8000, target->mon_id, 1));
	} else {
		hero->petrified = 1;
		sprintf(g_text_output_buf, txt[0x67], hero->name);
	}
}
